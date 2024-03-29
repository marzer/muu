// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "muu/thread_pool.h"
#include "muu/apply_alignment.h"
#include "muu/span.h"
#include "muu/blob.h"
#include "muu/emplacement_array.h"
#include "muu/strings.h"
#include "muu/scope_guard.h"
#include "muu/math.h"
#include "muu/thread_name.h"
#include "muu/impl/std_string.h"
#include "muu/pause.h"
#include "os.h"
#if !MUU_HAS_EXCEPTIONS
	#include "muu/impl/std_exception.h" // std::terminate()
#endif

MUU_DISABLE_WARNINGS;
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <optional>
MUU_ENABLE_WARNINGS;

#include "source_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(warning(disable : 26110)) // core guidelines: Caller failing to hold lock (false-positive)
MUU_PRAGMA_MSVC(warning(disable : 26495)) // core guidelines: uninitialized member
MUU_PRAGMA_MSVC(warning(disable : 4305))  // truncation from size_t to bool (false-positive)

using namespace std::chrono_literals;
using namespace std::string_literals;
using namespace std::string_view_literals;

//----------------------------------------------------------------------------------------------------------------------
// thread_pool internal implementation
//----------------------------------------------------------------------------------------------------------------------

namespace
{
	using thread_pool_byte_span = aligned_byte_span<impl::thread_pool_alignment>;

#if defined(__cpp_lib_atomic_wait) && __cpp_lib_atomic_wait >= 201907 && 0

	class thread_pool_monitor
	{
	  private:
	#if defined(__cpp_lib_atomic_lock_free_type_aliases) && __cpp_lib_atomic_lock_free_type_aliases >= 201907L
		std::atomic_unsigned_lock_free count_ = 0u;
	#else
		std::atomic_uint_fast32_t count_ = 0u;
	#endif

	  public:
		void wait() noexcept
		{
			auto val = count_.load();
			while (val)
			{
				count_.wait(val);
				val = count_.load();
			}
		}

		void increment(size_t i = 1u) noexcept
		{
			count_ += i;
			count_.notify_all();
		}

		void decrement(size_t i = 1u) noexcept
		{
			count_ -= i;
			count_.notify_all();
		}
	};

#else

	class thread_pool_monitor
	{
	  private:
		size_t busy = {};
		mutable std::mutex mutex;
		mutable std::condition_variable cv;

	  public:
		void wait() noexcept
		{
			std::unique_lock lock{ mutex };
			while (busy)
				cv.wait(lock);
		}

		void increment(size_t i = 1u) noexcept
		{
			std::lock_guard lock{ mutex };
			busy += i;
		}

		void decrement(size_t i = 1u) noexcept
		{
			bool notify = false;
			{
				std::lock_guard lock{ mutex };
				busy -= i;
				notify = !busy;
			}
			if (notify)
				cv.notify_all();
		}
	};

#endif

	class thread_pool_queue
	{
	  private:
		thread_pool_byte_span pool;
		thread_pool_monitor& monitor;
		size_t capacity, front = {}, back = {};
		size_t enqueues_ = {};
		mutable std::mutex mutex;
		mutable std::condition_variable cv;
		std::atomic_bool terminated_ = false;

		using task = impl::thread_pool_task;

		MUU_PURE_GETTER
		MUU_ATTR(returns_nonnull)
		MUU_ATTR(assume_aligned(impl::thread_pool_alignment))
		task* get_task(size_t i) noexcept
		{
			return MUU_LAUNDER(reinterpret_cast<task*>(muu::assume_aligned<impl::thread_pool_alignment>(pool.data())
													   + impl::thread_pool_alignment * ((front + i) % capacity)));
		}

		MUU_NODISCARD
		MUU_ATTR(returns_nonnull)
		MUU_ATTR(assume_aligned(impl::thread_pool_alignment))
		task* pop_front_task() noexcept
		{
			MUU_ASSERT(back > front);

			auto t = get_task({});
			MUU_ASSERT(t->action_invoker);

			front++;
			return t;
		}

		MUU_NODISCARD
		MUU_ATTR(nonnull)
		MUU_ATTR(returns_nonnull)
		MUU_ATTR(assume_aligned(impl::thread_pool_alignment))
		task* pop_front_task(void* buf) noexcept
		{
			MUU_ASSERT(back > front);

			auto t = pop_front_task();
			MUU_ASSERT(t->action_invoker);

			auto result = ::new (muu::assume_aligned<impl::thread_pool_alignment>(buf)) task{ MUU_MOVE(*t) };
			MUU_ASSERT(result->action_invoker);

			t->~task();
			return result;
		}

	  public:
		MUU_NODISCARD_CTOR
		thread_pool_queue(thread_pool_byte_span tp, thread_pool_monitor& mon) noexcept //
			: pool{ tp },
			  monitor{ mon },
			  capacity{ pool.size() / impl::thread_pool_alignment }
		{
			MUU_ASSERT(!pool.empty());
			MUU_ASSERT(capacity);
		}

		~thread_pool_queue() noexcept
		{
			std::lock_guard lock{ mutex };
			if (const auto remaining = size())
			{
				while (!empty())
					pop_front_task()->~task();
				monitor.decrement(remaining);
			}
		}

		MUU_DELETE_COPY(thread_pool_queue);
		MUU_DELETE_MOVE(thread_pool_queue);

		void terminate() noexcept
		{
			terminated_ = true;
			cv.notify_all();
		}

		MUU_PURE_INLINE_GETTER
		bool terminated() const noexcept
		{
			return terminated_;
		}

		MUU_PURE_INLINE_GETTER
		size_t size() const noexcept
		{
			return back - front;
		}

		MUU_PURE_INLINE_GETTER
		bool full() const noexcept
		{
			return size() >= capacity;
		}

		MUU_PURE_INLINE_GETTER
		bool empty() const noexcept
		{
			return back == front;
		}

		MUU_PURE_INLINE_GETTER
		size_t available() const noexcept
		{
			return capacity - size();
		}

		MUU_NODISCARD
		bool try_lock() noexcept
		{
			if (!mutex.try_lock())
				return false;

			enqueues_ = {};

			return true;
		}

		// void lock() noexcept
		//{
		//	mutex.lock();
		//	enqueues_ = {};
		//}

		MUU_NODISCARD
		MUU_ATTR(returns_nonnull)
		MUU_ATTR(assume_aligned(impl::thread_pool_alignment))
		void* acquire() noexcept
		{
			MUU_ASSERT(!full());

			enqueues_++;

			return muu::assume_aligned<impl::thread_pool_alignment>(pool.data())
				 + impl::thread_pool_alignment * (back++ % capacity);
		}

		void unlock() noexcept
		{
			const auto enq = enqueues_;
			if (enq)
			{
				monitor.increment(enq);
				MUU_ASSERT(get_task(back - front - 1u)->action_invoker);
			}

			mutex.unlock();

			if (enq)
				cv.notify_all();
		}

		MUU_NODISCARD
		MUU_ATTR(nonnull)
		MUU_ATTR(assume_aligned(impl::thread_pool_alignment))
		task* try_pop(void* buf) noexcept
		{
			std::unique_lock lock{ mutex, std::try_to_lock };
			if (!lock || empty() || terminated())
				return nullptr;

			return pop_front_task(muu::assume_aligned<impl::thread_pool_alignment>(buf));
		}

		MUU_NODISCARD
		MUU_ATTR(nonnull)
		MUU_ATTR(assume_aligned(impl::thread_pool_alignment))
		task* pop(void* buf, std::chrono::milliseconds timeoout) noexcept
		{
			std::unique_lock lock{ mutex };
			cv.wait_for(lock, timeoout, [this]() noexcept { return !empty() || terminated(); });

			if (empty() || terminated())
				return nullptr;

			return pop_front_task(muu::assume_aligned<impl::thread_pool_alignment>(buf));
		}
	};

	static constexpr size_t thread_pool_spin_wait_iterations_per_queue = 100;

	class thread_pool_worker
	{
	  private:
		std::thread thread;
		std::atomic_bool terminated_ = false;
		using task					 = impl::thread_pool_task;

	  public:
		MUU_ALWAYS_INLINE
		void terminate() noexcept
		{
			terminated_ = true;
		}

		MUU_PURE_INLINE_GETTER
		bool terminated() const noexcept
		{
			return terminated_;
		}

		MUU_NODISCARD_CTOR
		thread_pool_worker(size_t worker_index,
						   std::string&& worker_name,
						   span<thread_pool_queue> queues,
						   thread_pool_monitor& monitor_)
		{
			thread =
				std::thread{ [this, worker_index, name = MUU_MOVE(worker_name), queues, monitor = &monitor_]() noexcept
							 {
								 MUU_ASSUME(monitor != nullptr);

#if MUU_WINDOWS
								 MUU_UNUSED(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
								 auto at_exit = scope_guard{ []() noexcept { CoUninitialize(); } };
#endif

								 set_thread_name(name);

								 alignas(impl::thread_pool_alignment) std::byte pop_buffer[impl::thread_pool_alignment];

								 const size_t tries = queues.size() * thread_pool_spin_wait_iterations_per_queue;

								 while (!terminated())
								 {
									 task* t = nullptr;
									 while (!t && !terminated())
									 {
										 for (size_t i = worker_index, e = i + tries; i < e && !t; i++)
										 {
											 MUU_PAUSE();
											 t = queues[i % queues.size()].try_pop(pop_buffer);
										 }
										 if (!t)
											 t = queues[worker_index].pop(pop_buffer, 100ms); // blocks until timeout
									 }
									 if (t)
									 {
										 (*t)(worker_index);
										 monitor->decrement();
										 t->~task();
									 }
								 }
							 } };
		}

		~thread_pool_worker() noexcept
		{
			if (thread.joinable())
				thread.join();
		}

		MUU_DELETE_COPY(thread_pool_worker);
		MUU_DELETE_MOVE(thread_pool_worker);
	};

	MUU_PURE_GETTER
	static size_t calc_thread_pool_workers(size_t worker_count) noexcept
	{
		static constexpr auto absolute_max_workers = 1024_sz;
		static const auto concurrency			= max(static_cast<size_t>(std::thread::hardware_concurrency()), 1_sz);
		static const auto effective_max_workers = min(concurrency * 64_sz, absolute_max_workers);

		return min(worker_count ? worker_count : concurrency, effective_max_workers);
	}

	MUU_PURE_GETTER
	static constexpr size_t calc_thread_pool_worker_queue_size(size_t worker_count, size_t task_queue_size) noexcept
	{
		constexpr size_t max_buffer_size		 = 256_mb; // 4M tasks on x64
		constexpr size_t default_buffer_size	 = 64_kb;  // 1024 tasks on x64
		constexpr size_t max_task_queue_size	 = max_buffer_size / impl::thread_pool_alignment;
		constexpr size_t default_task_queue_size = default_buffer_size / impl::thread_pool_alignment;
		static_assert(max_task_queue_size > 0);
		static_assert(default_task_queue_size > 0);
		MUU_ASSUME(worker_count > 0);

		if (!task_queue_size)
			task_queue_size = default_task_queue_size;

		return min(
			static_cast<size_t>(std::ceil(static_cast<double>(task_queue_size) / static_cast<double>(worker_count))),
			max_task_queue_size / worker_count);
	}

	struct thread_pool_impl
	{
		thread_pool_byte_span queue_buffer;
		thread_pool_byte_span worker_buffer;
		thread_pool_byte_span task_buffer;
		size_t worker_count{}; // also the queue count
		size_t worker_queue_size{};
		std::atomic<size_t> next_queue = 0_sz;
		mutable thread_pool_monitor monitor;

		MUU_PURE_INLINE_GETTER
		thread_pool_queue& queue(size_t idx) noexcept
		{
			MUU_ASSERT(idx < worker_count);
			return *MUU_LAUNDER(
				reinterpret_cast<thread_pool_queue*>(queue_buffer.data() + sizeof(thread_pool_queue) * idx));
		}

		MUU_PURE_INLINE_GETTER
		thread_pool_worker& worker(size_t idx) noexcept
		{
			MUU_ASSERT(idx < worker_count);
			return *MUU_LAUNDER(
				reinterpret_cast<thread_pool_worker*>(worker_buffer.data() + sizeof(thread_pool_worker) * idx));
		}

		MUU_NODISCARD_CTOR
		thread_pool_impl(string_param&& name,
						 thread_pool_byte_span queue_buffer_,
						 thread_pool_byte_span worker_buffer_,
						 thread_pool_byte_span task_buffer_)
			: queue_buffer{ queue_buffer_ },
			  worker_buffer{ worker_buffer_ },
			  task_buffer{ task_buffer_ }
		{
			MUU_ASSERT(!queue_buffer_.empty());
			MUU_ASSERT(!worker_buffer_.empty());
			MUU_ASSERT(!task_buffer_.empty());
			MUU_ASSERT(queue_buffer_.size() % sizeof(thread_pool_queue) == 0_sz);
			MUU_ASSERT(worker_buffer_.size() % sizeof(thread_pool_worker) == 0_sz);
			MUU_ASSERT(task_buffer_.size() % impl::thread_pool_alignment == 0_sz);
			MUU_ASSERT(reinterpret_cast<uintptr_t>(task_buffer_.data()) % impl::thread_pool_alignment == 0_sz);

			worker_count	  = worker_buffer_.size() / sizeof(thread_pool_worker);
			worker_queue_size = task_buffer.size() / impl::thread_pool_alignment / worker_count;
			MUU_ASSERT(queue_buffer_.size() / sizeof(thread_pool_queue) == worker_count);

			for (size_t i = 0; i < worker_count; i++)
			{
				thread_pool_byte_span pool{ task_buffer.data() + impl::thread_pool_alignment * worker_queue_size * i,
											impl::thread_pool_alignment * worker_queue_size };
				::new (static_cast<void*>(queue_buffer.data() + sizeof(thread_pool_queue) * i))
					thread_pool_queue{ pool, monitor };
			}
			auto unwind_queues = scope_guard{ [&]() noexcept
											  {
												  for (size_t i = worker_count; i-- > 0_sz;)
													  queue(i).~thread_pool_queue();
											  } };

			std::string_view worker_name = name ? std::string_view{ name } : "muu::thread_pool"sv;
			size_t constructed_workers	 = {};
			auto unwind_workers			 = scope_guard{ [&]() noexcept
												{
												   for (size_t i = constructed_workers; i-- > 0_sz;)
													   worker(i).~thread_pool_worker();
											   } };
			for (size_t i = 0; i < worker_count; i++)
			{
				std::string n;
				n.reserve(worker_name.length() + 5u);
				n.append(worker_name);
				n.append(" ["sv);
				n.append(std::to_string(i));
				n += ']';

				::new (static_cast<void*>(worker_buffer.data() + sizeof(thread_pool_worker) * i))
					thread_pool_worker{ i, // worker_index
										MUU_MOVE(n),
										span<thread_pool_queue>{ &queue(0), worker_count }, // queues
										monitor };
				constructed_workers++;
			}

			unwind_queues.dismiss();
			unwind_workers.dismiss();
		}

		~thread_pool_impl() noexcept
		{
			for (size_t i = worker_count; i-- > 0_sz;)
				queue(i).terminate();
			for (size_t i = worker_count; i-- > 0_sz;)
			{
				worker(i).terminate();
				worker(i).~thread_pool_worker();
			}
			for (size_t i = worker_count; i-- > 0_sz;)
				queue(i).~thread_pool_queue();
		}

	  private:
		template <typename Action, typename Delay>
		static constexpr auto repeat_with_delay(Action&& action, Delay, size_t max_attempts) noexcept
		{
			using result_type = decltype(static_cast<Action&&>(action)());

			if constexpr (Delay::value > 0)
			{
				for (size_t i = 0; i < max_attempts; i++)
				{
					if (auto q = static_cast<decltype(action)&&>(action)())
						return q;
					std::this_thread::sleep_for(std::chrono::milliseconds{ Delay::value });
				}
			}
			else
			{
				for (size_t i = 0; i < max_attempts; i++)
				{
					if (auto q = static_cast<decltype(action)&&>(action)())
						return q;

					MUU_PAUSE();
				}
			}

			return result_type{};
		}

	  public:
		MUU_NODISCARD
		std::optional<size_t> lock_multiple(size_t required) noexcept
		{
			MUU_ASSUME(required >= 1);

			if (required > worker_queue_size)
				return {};

			const auto find_queue = [queue_count = worker_count,
									 iterations	 = worker_count * thread_pool_spin_wait_iterations_per_queue,
									 required,
									 this]() noexcept
			{
				const auto start = next_queue++;
				for (size_t i = start, e = i + iterations; i < e; i++)
				{
					const auto queue_index = i % queue_count;
					auto& q				   = queue(queue_index);
					if (q.try_lock())
					{
						if (q.available() >= required)
							return std::optional<size_t>{ queue_index };
						q.unlock();
					}
					MUU_PAUSE();
				}
				return std::optional<size_t>{};
			};

			using millis = std::chrono::milliseconds::rep;
			auto qindex	 = find_queue();
			if (!qindex)
				qindex = repeat_with_delay(find_queue, std::integral_constant<millis, 0>{}, 100);

			return qindex;
		}

		MUU_NODISCARD
		size_t lock() noexcept
		{
			const auto find_queue = [queue_count = worker_count,
									 iterations	 = worker_count * thread_pool_spin_wait_iterations_per_queue,
									 this]() noexcept
			{
				const auto start = next_queue++;
				for (size_t i = start, e = i + iterations; i < e; i++)
				{
					const auto queue_index = i % queue_count;
					auto& q				   = queue(queue_index);
					if (q.try_lock())
					{
						if (!q.full())
							return std::optional<size_t>{ queue_index };
						q.unlock();
					}
					MUU_PAUSE();
				}
				return std::optional<size_t>{};
			};

			using millis = std::chrono::milliseconds::rep;
			auto qindex	 = find_queue();
			if (!qindex)
				qindex = repeat_with_delay(find_queue, std::integral_constant<millis, 0>{}, 100);
			if (!qindex)
				qindex = repeat_with_delay(find_queue, std::integral_constant<millis, 10>{}, 10);
			if (!qindex)
				qindex =
					repeat_with_delay(find_queue, std::integral_constant<millis, 100>{}, constants<size_t>::highest);

			MUU_ASSERT(qindex);
			return *qindex;
		}

		MUU_NODISCARD
		MUU_ALWAYS_INLINE
		MUU_ATTR(returns_nonnull)
		MUU_ATTR(assume_aligned(muu::impl::thread_pool_alignment))
		void* acquire(size_t queue_index) noexcept
		{
			return queue(queue_index).acquire();
		}

		MUU_ALWAYS_INLINE
		void unlock(size_t queue_index) noexcept
		{
			queue(queue_index).unlock();
		}

		MUU_ALWAYS_INLINE
		void wait() const noexcept
		{
			monitor.wait();
		}
	};

	struct thread_pool_storage
	{
		thread_pool_byte_span buffer;
		thread_pool_impl impl;
	};

	MUU_CONST_INLINE_GETTER
	MUU_ATTR(nonnull)
	static thread_pool_storage& storage_cast(void* ptr) noexcept
	{
		MUU_ASSUME(ptr != nullptr);

		return *static_cast<thread_pool_storage*>(ptr);
	}
}

//----------------------------------------------------------------------------------------------------------------------
// thread_pool
//----------------------------------------------------------------------------------------------------------------------

extern "C" //
{
	void* MUU_CALLCONV muu_impl_thread_pool_create(size_t worker_count,
												   size_t task_queue_size,
												   string_param* name,
												   void* /*unused - future proofing*/)
	{
		MUU_ASSUME(name != nullptr);

		worker_count				 = calc_thread_pool_workers(worker_count);
		const auto worker_queue_size = calc_thread_pool_worker_queue_size(worker_count, task_queue_size);
		task_queue_size				 = worker_count * worker_queue_size;

		static_assert(impl::thread_pool_alignment >= sizeof(impl::thread_pool_task));

		constexpr auto storage_start = 0_sz;
		const auto queues_start =
			apply_alignment<impl::thread_pool_alignment>(storage_start + sizeof(thread_pool_storage));
		const auto queues_end		= queues_start + sizeof(thread_pool_queue) * worker_count;
		const auto workers_start	= apply_alignment<impl::thread_pool_alignment>(queues_end);
		const auto workers_end		= workers_start + sizeof(thread_pool_worker) * worker_count;
		const auto tasks_start		= apply_alignment<impl::thread_pool_alignment>(workers_end);
		const auto tasks_end		= tasks_start + impl::thread_pool_alignment * task_queue_size;
		const auto total_allocation = tasks_end - storage_start;

		static_assert(alignof(thread_pool_storage) <= impl::thread_pool_alignment);
		auto buffer_ptr = muu::assume_aligned<impl::thread_pool_alignment>(
			muu::aligned_alloc(total_allocation, impl::thread_pool_alignment));

#if !MUU_HAS_EXCEPTIONS
		{
			MUU_ASSERT(buffer_ptr && "allocate() failed!");
			if (!buffer_ptr)
				std::terminate();
		}
#endif

		const auto unwind = scope_fail{ [=]() noexcept { muu::aligned_free(buffer_ptr); } };

		thread_pool_byte_span buffer{ static_cast<std::byte*>(buffer_ptr), total_allocation };
		thread_pool_byte_span queue_buffer{ buffer.data() + queues_start, queues_end - queues_start };
		thread_pool_byte_span worker_buffer{ buffer.data() + workers_start, workers_end - workers_start };
		thread_pool_byte_span task_buffer{ buffer.data() + tasks_start, tasks_end - tasks_start };

		return ::new (buffer_ptr)
			thread_pool_storage{ buffer,
								 thread_pool_impl{ MUU_MOVE(*name), queue_buffer, worker_buffer, task_buffer } };
	}

	void MUU_CALLCONV muu_impl_thread_pool_destroy(void* storage_) noexcept
	{
		MUU_ASSUME(storage_ != nullptr);

		auto& storage = storage_cast(storage_);
		auto buffer	  = storage.buffer;
		storage.~thread_pool_storage();
		muu::aligned_free(buffer.data());
	}

	size_t MUU_CALLCONV muu_impl_thread_pool_lock_multiple(void* storage_, size_t required) noexcept
	{
		MUU_ASSUME(storage_ != nullptr);
		MUU_ASSUME(required >= 1u);

		const auto queue = storage_cast(storage_).impl.lock_multiple(required);
		if (queue)
			return queue.value();
		return static_cast<size_t>(-1);
	}

	size_t MUU_CALLCONV muu_impl_thread_pool_lock(void* storage_) noexcept
	{
		MUU_ASSUME(storage_ != nullptr);

		return storage_cast(storage_).impl.lock();
	}

	void* MUU_CALLCONV muu_impl_thread_pool_acquire(void* storage_, size_t qindex) noexcept
	{
		MUU_ASSUME(storage_ != nullptr);

		return storage_cast(storage_).impl.acquire(qindex);
	}

	void MUU_CALLCONV muu_impl_thread_pool_unlock(void* storage_, size_t qindex) noexcept
	{
		MUU_ASSUME(storage_ != nullptr);

		storage_cast(storage_).impl.unlock(qindex);
	}

	void MUU_CALLCONV muu_impl_thread_pool_wait(void* storage_) noexcept
	{
		MUU_ASSUME(storage_ != nullptr);

		storage_cast(storage_).impl.wait();
	}

	MUU_PURE_GETTER
	size_t MUU_CALLCONV muu_impl_thread_pool_workers(void* storage_) noexcept
	{
		return storage_ ? storage_cast(storage_).impl.worker_count : 0_sz;
	}

	MUU_PURE_GETTER
	size_t MUU_CALLCONV muu_impl_thread_pool_capacity(void* storage_) noexcept
	{
		return storage_ ? storage_cast(storage_).impl.worker_count * storage_cast(storage_).impl.worker_queue_size
						: 0_sz;
	}
}
