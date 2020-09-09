// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "muu/thread_pool.h"
#include "muu/span.h"
#include "muu/blob.h"
#include "muu/emplacement_array.h"
#include "os_internal.h"
MUU_DISABLE_WARNINGS
#include <atomic>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <optional>
#if MUU_WINDOWS
#include <objbase.h> // CoInitializeEx, CoUninitialize
#endif
MUU_ENABLE_WARNINGS

MUU_PUSH_WARNINGS
MUU_DISABLE_SPAM_WARNINGS
using namespace muu;
using namespace std::chrono_literals;
using namespace std::string_literals;
using namespace std::string_view_literals;

#define MUU_MOVE_CHECK	MUU_ASSERT(pimpl_ != nullptr && "The thread_pool has been moved from!")

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
					cv.wait(lock);
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
			mutable std::condition_variable wait;
			std::atomic_bool terminated_ = false;

			using task = impl::thread_pool_task;

			[[nodiscard]]
			MUU_ATTR(returns_nonnull)
			MUU_ATTR(assume_aligned(impl::thread_pool_task_granularity))
			task* get_task(size_t i) noexcept
			{
				return launder(reinterpret_cast<task*>(
					assume_aligned<impl::thread_pool_task_granularity>(pool.data())
					+ impl::thread_pool_task_granularity * ((front + i) % capacity)
				));
			}

			[[nodiscard]]
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

			[[nodiscard]]
			MUU_ATTR(nonnull)
			MUU_ATTR(returns_nonnull)
			MUU_ATTR(assume_aligned(impl::thread_pool_task_granularity))
			task* pop_front_task(void* buf) noexcept
			{
				MUU_ASSERT(back > front);

				auto t = pop_front_task();
				MUU_ASSERT(t->action_invoker);

				auto result = new (assume_aligned<impl::thread_pool_task_granularity>(buf)) task{ std::move(*t) };
				MUU_ASSERT(result->action_invoker);

				t->~task();
				return result;
			}

		public:

			MUU_NODISCARD_CTOR
			thread_pool_queue(byte_span tp, thread_pool_monitor& mon) noexcept
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
					wait.notify_all();
			}

			[[nodiscard]]
			bool terminated() const noexcept
			{
				return terminated_;
			}

			[[nodiscard]]
			size_t size() const noexcept
			{
				return back - front;
			}

			[[nodiscard]]
			bool full() const noexcept
			{
				return size() >= capacity;
			}

			[[nodiscard]]
			bool empty() const noexcept
			{
				return back == front;
			}

			[[nodiscard]]
			bool try_lock() noexcept
			{
				if (!mutex.try_lock())
					return false;
				enqueues = 0;
				return true;
			}

			//void lock() noexcept
			//{
			//	mutex.lock();
			//	enqueues = 0;
			//}

			[[nodiscard]]
			MUU_ATTR(returns_nonnull)
			MUU_ATTR(assume_aligned(impl::thread_pool_task_granularity))
			void* acquire() noexcept
			{
				MUU_ASSERT(!full());
				enqueues++;
				return assume_aligned<impl::thread_pool_task_granularity>(pool.data()) + impl::thread_pool_task_granularity * (back++ % capacity);
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
					wait.notify_one();
				else if (enq > 1)
					wait.notify_all();
			}

			[[nodiscard]]
			MUU_ATTR(nonnull)
			MUU_ATTR(assume_aligned(impl::thread_pool_task_granularity))
			task* try_pop(void* buf) noexcept
			{
				std::unique_lock lock{ mutex, std::try_to_lock };
				if (!lock || empty() || terminated())
					return nullptr;

				return pop_front_task(assume_aligned<impl::thread_pool_task_granularity>(buf));
			}

			[[nodiscard]]
			MUU_ATTR(nonnull)
			MUU_ATTR(assume_aligned(impl::thread_pool_task_granularity))
			task* pop(void* buf) noexcept
			{
				std::unique_lock lock{ mutex };
				while (empty() && !terminated())
					wait.wait(lock);

				if (terminated())
					return nullptr;

				return pop_front_task(assume_aligned<impl::thread_pool_task_granularity>(buf));
			}
	};

	inline constexpr size_t thread_pool_wait_free_iterations = 20;

	class thread_pool_worker final
	{
		private:
			std::thread thread;
			std::atomic_bool terminated_ = false;
			using task = impl::thread_pool_task;

		public:

			void terminate() noexcept
			{
				terminated_ = true;
			}

			[[nodiscard]]
			bool terminated() const noexcept
			{
				return terminated_;
			}

			MUU_NODISCARD_CTOR
			thread_pool_worker(size_t worker_index, std::string&& worker_name, span<thread_pool_queue> queues, thread_pool_monitor& monitor_) noexcept
			{
				thread = std::thread{
					[this, worker_index, name = std::move(worker_name), queues, monitor = &monitor_]() noexcept
					{
						MUU_ASSUME(monitor != nullptr);

						#if MUU_WINDOWS
						CoInitializeEx(nullptr, COINIT_MULTITHREADED);
						auto at_exit = scope_guard{ []() noexcept { CoUninitialize(); } };
						#endif

						set_thread_name(name);

						MUU_ALIGN(impl::thread_pool_task_granularity) std::byte pop_buffer[impl::thread_pool_task_granularity];

						while (!terminated())
						{
							const size_t tries = queues.size() * thread_pool_wait_free_iterations;

							task* t = nullptr;
							for (size_t i = 0; i < tries && !t; i++)
								t = queues[(worker_index + i) % queues.size()].try_pop(pop_buffer);
							if (!t)
								t = queues[worker_index].pop(pop_buffer); // blocks
							if (t)
							{
								(*t)(worker_index);
								monitor->decrement();
								t->~task();
							}
						}
					}
				};
			}

			~thread_pool_worker() noexcept
			{
				if (thread.joinable())
					thread.join();
			}

			MUU_DELETE_COPY(thread_pool_worker);
			MUU_DELETE_MOVE(thread_pool_worker);
	};

	static size_t calc_thread_pool_workers(size_t worker_count) noexcept
	{
		static constexpr unsigned max_workers = 512;

		const auto concurrency = (max)(std::thread::hardware_concurrency(), 1u);
		if (!worker_count)
			return concurrency;
		return (min)(worker_count, static_cast<size_t>((min)(concurrency * 100, max_workers)));
	}

	static size_t calc_thread_pool_worker_queue_size(size_t worker_count, size_t task_queue_size) noexcept
	{
		static constexpr size_t max_buffer_size = 256 * 1024 * 1024; // 256 MB (4M tasks on x64)
		static constexpr size_t default_buffer_size = 64 * 1024;	 // 64 KB (1024 tasks on x64)
		static constexpr size_t max_task_queue_size = max_buffer_size / impl::thread_pool_task_granularity;
		static constexpr size_t default_task_queue_size = default_buffer_size / impl::thread_pool_task_granularity;
		static_assert(max_task_queue_size > 0);
		static_assert(default_task_queue_size > 0);
		MUU_ASSUME(worker_count > 0);

		if (!task_queue_size)
			task_queue_size = default_task_queue_size;

		return (muu::min)(
			static_cast<size_t>(std::ceil(static_cast<double>(task_queue_size) / static_cast<double>(worker_count))),
			max_task_queue_size / worker_count
		);
	}
}

struct thread_pool::pimpl final
{
	size_t	worker_count;
	size_t	worker_queue_size;
	blob	task_buffer;
	emplacement_array<thread_pool_queue> queues;
	emplacement_array<thread_pool_worker> workers;
	std::atomic<size_t> next_queue = 0_sz;
	mutable thread_pool_monitor monitor;

	pimpl(size_t workers_, size_t task_queue_size, std::string_view name) noexcept
		: worker_count{ calc_thread_pool_workers(workers_) },
		worker_queue_size{ calc_thread_pool_worker_queue_size(worker_count, task_queue_size) },
		task_buffer{ impl::thread_pool_task_granularity * worker_count * worker_queue_size, nullptr, impl::thread_pool_task_granularity }
	{
		if (name.empty())
			name = "muu::thread_pool"sv;

		queues = emplacement_array<thread_pool_queue>{ worker_count };
		for (size_t i = 0; i < worker_count; i++)
		{
			byte_span pool{
				task_buffer.data() + impl::thread_pool_task_granularity * worker_queue_size * i,
				impl::thread_pool_task_granularity * worker_queue_size
			};
			queues.emplace_back(pool, monitor);
		}

		workers = emplacement_array<thread_pool_worker>{ worker_count };
		for (size_t i = 0; i < worker_count; i++)
		{
			std::string n;
			n.reserve(name.length() + 5u);
			n.append(name);
			n.append(" ["sv);
			n.append(std::to_string(i));
			n += ']';

			workers.emplace_back(
				i, // worker_index
				std::move(n),
				span{ queues.data(), queues.size() }, // queues
				monitor
			);
		}
	}

	~pimpl() noexcept
	{
		for (auto& q : queues)
			q.terminate();
		wait();
		for (auto& w : workers)
			w.terminate();
	}

	[[nodiscard]]
	size_t lock() noexcept
	{
		const auto starting_queue = next_queue++;
		const auto iterations = queues.size() * thread_pool_wait_free_iterations;

		const auto find_queue = [&]() noexcept
			-> std::optional<size_t>
		{
			for (size_t i = 0; i < iterations; i++)
			{
				const auto qindex = (starting_queue + i) % queues.size();
				auto& q = queues[qindex];
				if (q.try_lock())
				{
					if (!q.full())
						return qindex;
					q.unlock();
				}
			}
			return std::nullopt;
		};


		const auto find_queue_with_delay = [&](std::chrono::milliseconds delay, size_t max_attempts) noexcept
			-> std::optional<size_t>
		{
			for (size_t i = 0; i < max_attempts; i++)
			{
				std::this_thread::sleep_for(delay);
				auto q = find_queue();
				if (q)
					return q;
			}
			return std::nullopt;
		};


		auto qindex = find_queue();
		if (!qindex)
			qindex = find_queue_with_delay(10ms, 10);
		if (!qindex)
			qindex = find_queue_with_delay(50ms, 4);
		if (!qindex)
			qindex = find_queue_with_delay(100ms, 2);
		if (!qindex)
			qindex = find_queue_with_delay(250ms, constants<size_t>::highest);

		MUU_ASSERT(qindex);
		return *qindex;
	}

	[[nodiscard]]
	MUU_ATTR(returns_nonnull)
	MUU_ATTR(assume_aligned(MUU_NAMESPACE::impl::thread_pool_task_granularity))
	void* acquire(size_t qindex) noexcept
	{
		return queues[qindex].acquire();
	}

	void unlock(size_t qindex) noexcept
	{
		return queues[qindex].unlock();
	}
	
	void wait() const noexcept
	{
		monitor.wait();
	}
};

thread_pool::thread_pool(size_t worker_count, size_t task_queue_size, std::string_view name) noexcept
	: pimpl_{ new pimpl{ worker_count, task_queue_size, name }}
{ }

thread_pool::thread_pool(thread_pool&& other) noexcept
	: pimpl_{ other.pimpl_ }
{
	other.pimpl_ = nullptr;
}

thread_pool& thread_pool::operator= (thread_pool&& rhs) noexcept
{
	pimpl_ = rhs.pimpl_;
	rhs.pimpl_ = nullptr;
	return *this;
}

thread_pool::~thread_pool() noexcept
{
	delete pimpl_;
}

size_t thread_pool::lock() noexcept
{
	MUU_MOVE_CHECK;
	return pimpl_->lock();
}

void* thread_pool::acquire(size_t qindex) noexcept
{
	MUU_MOVE_CHECK;
	return pimpl_->acquire(qindex);
}

void thread_pool::unlock(size_t qindex) noexcept
{
	MUU_MOVE_CHECK;
	pimpl_->unlock(qindex);
}

size_t thread_pool::size() const noexcept
{
	return pimpl_ ? pimpl_->worker_count : 0_sz;
}

void thread_pool::wait() noexcept
{
	MUU_MOVE_CHECK;
	pimpl_->wait();
}

MUU_POP_WARNINGS // MUU_DISABLE_SPAM_WARNINGS
