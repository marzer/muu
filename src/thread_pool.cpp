// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "muu/thread_pool.h"
#include "muu/span.h"
#include "muu/blob.h"
#include "muu/emplacement_array.h"
#include "muu/strings.h"
#include "muu/scope_guard.h"
#include "muu/math.h"
#include "os.h"

MUU_DISABLE_WARNINGS;
#include <atomic>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <optional>
#if MUU_ISET_SSE2
	#if MUU_LINUX
		#include <emmintrin.h>
	#endif
	#define spin_wait_iteration() _mm_pause()
#else
	#define spin_wait_iteration() MUU_NOOP
#endif
#if !MUU_HAS_EXCEPTIONS
	#include <exception> // std::terminate()
#endif
MUU_ENABLE_WARNINGS;

#include "source_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(warning(disable : 26110)) // core guidelines: Caller failing to hold lock (false-positive)
MUU_PRAGMA_MSVC(warning(disable : 26495)) // core guidelines: uninitialized member

using namespace std::chrono_literals;
using namespace std::string_literals;
using namespace std::string_view_literals;

#define MUU_MOVE_CHECK MUU_ASSERT(storage_ != nullptr && "The thread_pool has been moved from!")

//----------------------------------------------------------------------------------------------------------------------
// thread_pool internal implementation
//----------------------------------------------------------------------------------------------------------------------

namespace
{
	class thread_pool_monitor final
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
				cv.wait_for(lock, 250ms);
		}

		void increment(size_t i = 1) noexcept
		{
			MUU_ASSERT(i > 0);

			std::lock_guard lock{ mutex };
			busy += i;
		}

		void decrement(size_t i = 1) noexcept
		{
			MUU_ASSERT(i > 0);

			bool notify = false;
			{
				std::lock_guard lock{ mutex };
				MUU_ASSERT(i <= busy);
				busy -= i;
				notify = !busy;
			}
			if (notify)
				cv.notify_all();
		}
	};

	class thread_pool_queue final
	{
	  private:
		byte_span pool;
		thread_pool_monitor& monitor;
		size_t capacity, front = {}, back = {};
		size_t enqueues = {};
		mutable std::mutex mutex;
		mutable std::condition_variable cv;
		std::atomic_bool terminated_ = false;

		using task = impl::thread_pool_task;

		MUU_NODISCARD
		MUU_ATTR(returns_nonnull)
		MUU_ATTR(assume_aligned(impl::thread_pool_task_granularity))
		task* get_task(size_t i) noexcept
		{
			return launder(reinterpret_cast<task*>(muu::assume_aligned<impl::thread_pool_task_granularity>(pool.data())
												   + impl::thread_pool_task_granularity * ((front + i) % capacity)));
		}

		MUU_NODISCARD
		MUU_ATTR(returns_nonnull)
		MUU_ATTR(assume_aligned(impl::thread_pool_task_granularity))
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
		MUU_ATTR(assume_aligned(impl::thread_pool_task_granularity))
		task* pop_front_task(void* buf) noexcept
		{
			MUU_ASSERT(back > front);

			auto t = pop_front_task();
			MUU_ASSERT(t->action_invoker);

			auto result = ::new (muu::assume_aligned<impl::thread_pool_task_granularity>(buf)) task{ MUU_MOVE(*t) };
			MUU_ASSERT(result->action_invoker);

			t->~task();
			return result;
		}

	  public:
		MUU_NODISCARD_CTOR
		thread_pool_queue(byte_span tp, thread_pool_monitor& mon) noexcept //
			: pool{ tp },
			  monitor{ mon },
			  capacity{ pool.size() / impl::thread_pool_task_granularity }
		{
			MUU_ASSERT(!pool.empty());
			MUU_ASSERT(capacity);
		}

		~thread_pool_queue() noexcept
		{
			std::lock_guard lock{ mutex };
			if (const auto remaining = size(); remaining)
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
			bool expected = false;
			if (terminated_.compare_exchange_strong(expected, true))
				cv.notify_all();
		}

		MUU_NODISCARD
		bool terminated() const noexcept
		{
			return terminated_;
		}

		MUU_NODISCARD
		size_t size() const noexcept
		{
			return back - front;
		}

		MUU_NODISCARD
		bool full() const noexcept
		{
			return size() >= capacity;
		}

		MUU_NODISCARD
		bool empty() const noexcept
		{
			return back == front;
		}

		MUU_NODISCARD
		bool try_lock() noexcept
		{
			if (!mutex.try_lock())
				return false;
			enqueues = 0;
			return true;
		}

		// void lock() noexcept
		//{
		//	mutex.lock();
		//	enqueues = 0;
		//}

		MUU_NODISCARD
		MUU_ATTR(returns_nonnull)
		MUU_ATTR(assume_aligned(impl::thread_pool_task_granularity))
		void* acquire() noexcept
		{
			MUU_ASSERT(!full());
			enqueues++;
			return muu::assume_aligned<impl::thread_pool_task_granularity>(pool.data())
				 + impl::thread_pool_task_granularity * (back++ % capacity);
		}

		void unlock() noexcept
		{
			size_t enq = enqueues;
			if (enq)
			{
				monitor.increment(enq);
#ifndef NDEBUG
				MUU_ASSERT(get_task(back - front - 1u)->action_invoker);
#endif
			}

			mutex.unlock();

			if (enq == 1)
				cv.notify_one();
			else if (enq > 1)
				cv.notify_all();
		}

		MUU_NODISCARD
		MUU_ATTR(nonnull)
		MUU_ATTR(assume_aligned(impl::thread_pool_task_granularity))
		task* try_pop(void* buf) noexcept
		{
			std::unique_lock lock{ mutex, std::try_to_lock };
			if (!lock || empty() || terminated())
				return nullptr;

			return pop_front_task(muu::assume_aligned<impl::thread_pool_task_granularity>(buf));
		}

		MUU_NODISCARD
		MUU_ATTR(nonnull)
		MUU_ATTR(assume_aligned(impl::thread_pool_task_granularity))
		task* pop(void* buf) noexcept
		{
			std::unique_lock lock{ mutex };
			while (empty() && !terminated())
				cv.wait_for(lock, 250ms);

			if (terminated())
				return nullptr;

			return pop_front_task(muu::assume_aligned<impl::thread_pool_task_granularity>(buf));
		}
	};

	inline constexpr size_t thread_pool_spin_wait_iterations = 20;

	class thread_pool_worker final
	{
	  private:
		std::thread thread;
		std::atomic_bool terminated_ = false;
		using task					 = impl::thread_pool_task;

	  public:
		void terminate() noexcept
		{
			terminated_ = true;
		}

		MUU_NODISCARD
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

								 alignas(impl::thread_pool_task_granularity)
									 std::byte pop_buffer[impl::thread_pool_task_granularity];

								 while (!terminated())
								 {
									 const size_t tries = queues.size() * thread_pool_spin_wait_iterations;

									 task* t = nullptr;
									 for (size_t i = 0; i < tries && !t; i++)
									 {
										 spin_wait_iteration();
										 t = queues[(worker_index + i) % queues.size()].try_pop(pop_buffer);
									 }
									 if (!t)
										 t = queues[worker_index].pop(pop_buffer); // blocks
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

	MUU_NODISCARD
	static size_t calc_thread_pool_workers(size_t worker_count) noexcept
	{
		static constexpr auto absolute_max_workers = 1024_sz;
		static const auto concurrency			= max(static_cast<size_t>(std::thread::hardware_concurrency()), 1_sz);
		static const auto effective_max_workers = min(concurrency * 64_sz, absolute_max_workers);

		return min(worker_count ? worker_count : concurrency, effective_max_workers);
	}

	MUU_NODISCARD
	static size_t calc_thread_pool_worker_queue_size(size_t worker_count, size_t task_queue_size) noexcept
	{
		static constexpr size_t max_buffer_size			= 256_mb; // 4M tasks on x64
		static constexpr size_t default_buffer_size		= 64_kb;  // 1024 tasks on x64
		static constexpr size_t max_task_queue_size		= max_buffer_size / impl::thread_pool_task_granularity;
		static constexpr size_t default_task_queue_size = default_buffer_size / impl::thread_pool_task_granularity;
		static_assert(max_task_queue_size > 0);
		static_assert(default_task_queue_size > 0);
		MUU_ASSUME(worker_count > 0);

		if (!task_queue_size)
			task_queue_size = default_task_queue_size;

		return min(
			static_cast<size_t>(std::ceil(static_cast<double>(task_queue_size) / static_cast<double>(worker_count))),
			max_task_queue_size / worker_count);
	}

	struct thread_pool_impl final
	{
		byte_span queue_buffer;
		byte_span worker_buffer;
		byte_span task_buffer;
		size_t worker_count{};
		size_t worker_queue_size{};
		std::atomic<size_t> next_queue = 0_sz;
		mutable thread_pool_monitor monitor;

		MUU_NODISCARD
		MUU_ATTR(pure)
		MUU_ATTR(nonnull)
		thread_pool_queue& queue(size_t idx) noexcept
		{
			MUU_ASSERT(idx < worker_count);
			return *muu::launder(
				reinterpret_cast<thread_pool_queue*>(queue_buffer.data() + sizeof(thread_pool_queue) * idx));
		}

		MUU_NODISCARD
		MUU_ATTR(pure)
		MUU_ATTR(nonnull)
		thread_pool_worker& worker(size_t idx) noexcept
		{
			MUU_ASSERT(idx < worker_count);
			return *muu::launder(
				reinterpret_cast<thread_pool_worker*>(worker_buffer.data() + sizeof(thread_pool_worker) * idx));
		}

		MUU_NODISCARD_CTOR
		thread_pool_impl(string_param&& name, byte_span queue_buffer_, byte_span worker_buffer_, byte_span task_buffer_)
			: queue_buffer{ queue_buffer_ },
			  worker_buffer{ worker_buffer_ },
			  task_buffer{ task_buffer_ }
		{
			MUU_ASSERT(!queue_buffer_.empty());
			MUU_ASSERT(!worker_buffer_.empty());
			MUU_ASSERT(!task_buffer_.empty());
			MUU_ASSERT(queue_buffer_.size() % sizeof(thread_pool_queue) == 0_sz);
			MUU_ASSERT(worker_buffer_.size() % sizeof(thread_pool_worker) == 0_sz);
			MUU_ASSERT(task_buffer_.size() % impl::thread_pool_task_granularity == 0_sz);
			MUU_ASSERT(reinterpret_cast<uintptr_t>(task_buffer_.data()) % impl::thread_pool_task_granularity == 0_sz);

			worker_count	  = worker_buffer_.size() / sizeof(thread_pool_worker);
			worker_queue_size = task_buffer.size() / impl::thread_pool_task_granularity / worker_count;
			MUU_ASSERT(queue_buffer_.size() / sizeof(thread_pool_queue) == worker_count);

			for (size_t i = 0; i < worker_count; i++)
			{
				byte_span pool{ task_buffer.data() + impl::thread_pool_task_granularity * worker_queue_size * i,
								impl::thread_pool_task_granularity * worker_queue_size };
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

		MUU_NODISCARD
		size_t lock() noexcept
		{
			// exponential falloff based on
			// https://software.intel.com/content/www/us/en/develop/articles/benefitting-power-and-performance-sleep-loops.html

			const auto find_queue = [workers	= worker_count,
									 start		= next_queue++,
									 iterations = worker_count * thread_pool_spin_wait_iterations,
									 this]() noexcept
			{
				for (size_t i = 0; i < iterations; i++)
				{
					spin_wait_iteration();
					const auto qindex = (start + i) % workers;
					auto& q			  = queue(qindex);
					if (q.try_lock())
					{
						if (!q.full())
							return std::optional<size_t>{ qindex };
						q.unlock();
					}
				}
				return std::optional<size_t>{};
			};

			static constexpr auto repeat_with_delay =
				[](auto&& action, std::chrono::milliseconds delay, size_t max_attempts) noexcept
			{
				using result_type = decltype(static_cast<decltype(action)&&>(action)());
				for (size_t i = 0; i < max_attempts; i++)
				{
					std::this_thread::sleep_for(delay);
					if (auto q = static_cast<decltype(action)&&>(action)())
						return q;
				}
				return result_type{};
			};

			auto qindex = find_queue();
			if (!qindex)
				qindex = repeat_with_delay(find_queue, 0ms, 100);
			if (!qindex)
				qindex = repeat_with_delay(find_queue, 10ms, 10);
			if (!qindex)
				qindex = repeat_with_delay(find_queue, 100ms, constants<size_t>::highest);

			MUU_ASSERT(qindex);
			return *qindex;
		}

		MUU_NODISCARD
		MUU_ATTR(returns_nonnull)
		MUU_ATTR(assume_aligned(muu::impl::thread_pool_task_granularity))
		void* acquire(size_t qindex) noexcept
		{
			return queue(qindex).acquire();
		}

		void unlock(size_t qindex) noexcept
		{
			return queue(qindex).unlock();
		}

		void wait() const noexcept
		{
			monitor.wait();
		}
	};

	struct thread_pool_storage
	{
		generic_allocator* allocator;
		byte_span buffer;
		thread_pool_impl impl;
	};

	MUU_NODISCARD
	MUU_ALWAYS_INLINE
	MUU_ATTR(const)
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

thread_pool::thread_pool(size_t worker_count, size_t task_queue_size, string_param name, generic_allocator* allocator)
{
	if (!allocator)
		allocator = &impl::get_default_allocator();

	worker_count				 = calc_thread_pool_workers(worker_count);
	const auto worker_queue_size = calc_thread_pool_worker_queue_size(worker_count, task_queue_size);
	task_queue_size				 = worker_count * worker_queue_size;

	static_assert(impl::thread_pool_task_granularity >= sizeof(impl::thread_pool_task));

	constexpr auto storage_start = 0_sz;
	const auto queues_start =
		apply_alignment<impl::thread_pool_task_granularity>(storage_start + sizeof(thread_pool_storage));
	const auto queues_end		= queues_start + sizeof(thread_pool_queue) * worker_count;
	const auto workers_start	= apply_alignment<impl::thread_pool_task_granularity>(queues_end);
	const auto workers_end		= workers_start + sizeof(thread_pool_worker) * worker_count;
	const auto tasks_start		= apply_alignment<impl::thread_pool_task_granularity>(workers_end);
	const auto tasks_end		= tasks_start + impl::thread_pool_task_granularity * task_queue_size;
	const auto total_allocation = tasks_end - storage_start;

	auto buffer_ptr =
		allocator->allocate(total_allocation, max(alignof(thread_pool_storage), impl::thread_pool_task_granularity));

#if !MUU_HAS_EXCEPTIONS
	{
		MUU_ASSERT(buffer_ptr && "allocate() failed!");
		if (!buffer_ptr)
			std::terminate();
	}
#endif

	const auto unwind = scope_fail{ [=]() noexcept { allocator->deallocate(buffer_ptr); } };

	byte_span buffer{ static_cast<std::byte*>(buffer_ptr), total_allocation };
	byte_span queue_buffer{ buffer.data() + queues_start, queues_end - queues_start };
	byte_span worker_buffer{ buffer.data() + workers_start, workers_end - workers_start };
	byte_span task_buffer{ buffer.data() + tasks_start, tasks_end - tasks_start };

	storage_ = ::new (buffer_ptr)
		thread_pool_storage{ allocator,
							 buffer,
							 thread_pool_impl{ MUU_MOVE(name), queue_buffer, worker_buffer, task_buffer } };
}

thread_pool::thread_pool(thread_pool&& other) noexcept : storage_{ other.storage_ }
{
	other.storage_ = nullptr;
}

thread_pool& thread_pool::operator=(thread_pool&& rhs) noexcept
{
	storage_ = std::exchange(rhs.storage_, nullptr);
	return *this;
}

thread_pool::~thread_pool() noexcept
{
	if (auto sptr = std::exchange(storage_, nullptr))
	{
		auto& storage  = storage_cast(sptr);
		auto buffer	   = storage.buffer;
		auto allocator = storage.allocator;
		storage.~thread_pool_storage();
		allocator->deallocate(buffer.data());
	}
}

size_t thread_pool::lock() noexcept
{
	MUU_MOVE_CHECK;
	return storage_cast(storage_).impl.lock();
}

void* thread_pool::acquire(size_t qindex) noexcept
{
	MUU_MOVE_CHECK;
	return storage_cast(storage_).impl.acquire(qindex);
}

void thread_pool::unlock(size_t qindex) noexcept
{
	MUU_MOVE_CHECK;
	storage_cast(storage_).impl.unlock(qindex);
}

size_t thread_pool::workers() const noexcept
{
	return storage_ ? storage_cast(storage_).impl.worker_count : 0_sz;
}

size_t thread_pool::capacity() const noexcept
{
	return storage_ ? storage_cast(storage_).impl.worker_count * storage_cast(storage_).impl.worker_queue_size : 0_sz;
}

void thread_pool::wait() noexcept
{
	MUU_MOVE_CHECK;
	storage_cast(storage_).impl.wait();
}
