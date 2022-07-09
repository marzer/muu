// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/thread_pool.h"
MUU_DISABLE_WARNINGS;
#include <thread>
#include <atomic>
MUU_ENABLE_WARNINGS;
MUU_DISABLE_SPAM_WARNINGS;

// clang-format off

TEST_CASE("thread_pool - initialization")
{
	// creating and immediately destroying a threadpool causes a spurious deadlock
	// on the CI servers for some reason, so this is a workaround
	static constexpr auto anti_hang = []() noexcept
	{
		std::this_thread::sleep_for(100ms);
	};

	{
		INFO("single worker, queue size == 1")

		thread_pool pool{ 1u, 1u };
		CHECK(pool.workers() == 1u);

		thread_pool pool2{ std::move(pool) };
		CHECK(pool.workers() == 0u);
		CHECK(pool2.workers() == 1u);
		pool2.for_each(0_sz, pool2.workers(), anti_hang);

		std::this_thread::sleep_for(100ms);
	}

	{
		INFO("single worker, queue size == 10")

		thread_pool pool { 1u, 10u };
		CHECK(pool.workers() == 1u);
		pool.for_each(0_sz, pool.workers(), anti_hang);

		std::this_thread::sleep_for(100ms);
	}

	{
		INFO("auto workers, auto queue size")

		thread_pool pool;
		CHECK(pool.workers() == std::thread::hardware_concurrency());
		pool.for_each(0_sz, pool.workers(), anti_hang);

		std::this_thread::sleep_for(100ms);
	}

	{
		INFO("auto workers, queue size == 10")
		thread_pool pool { 0u, 10u };
		pool.for_each(0_sz, pool.workers(), anti_hang);

		std::this_thread::sleep_for(100ms);
	}
}

namespace
{
	int test_value = {};
	size_t test_worker_index = {};

	static void test_func(size_t worker) noexcept
	{
		test_value++;
		test_worker_index  = worker;
	}

	static constexpr auto storage_threshold = impl::thread_pool_task::buffer_capacity;

	template <typename T>
	struct callable_counter
	{
		std::atomic<T>* value_;

		callable_counter(std::atomic<T>& value) noexcept
			: value_{ &value }
		{}

		callable_counter(const callable_counter& other) noexcept
			: value_{ other.value_ }
		{}

		callable_counter(callable_counter&& other) noexcept
			: value_{ std::exchange(other.value_, nullptr) }
		{}

		callable_counter& operator = (const callable_counter& other) noexcept
		{
			value_ = other.value_;
			return *this;
		}

		callable_counter& operator = (callable_counter&& other) noexcept
		{
			value_ = std::exchange(other.value_, nullptr);
			return *this;
		}

		template <typename... Args>
		void operator() (Args&&...) & noexcept
		{
			if (value_)
				(*value_)++;
		}

		template <typename... Args>
		void operator() (Args&&...) && noexcept
		{
			if (value_)
			{
				(*value_)++;
				value_ = nullptr;
			}
		}
	};

	template <typename T>
	callable_counter(std::atomic<T>&) -> callable_counter<T>;
}

TEST_CASE("thread_pool - enqueue")
{
	thread_pool pool{ min(std::thread::hardware_concurrency(), 16u) };

	{
		INFO("tasks with no state at all")
		{
			INFO("# 1")
			test_value = {};

			pool.enqueue([]() noexcept { test_value++; });
			pool.wait();

			CHECK(test_value == 1);
		}
		{
			INFO("# 2")
			test_value = {};
			test_worker_index = 0xFFFFFFFFu;

			pool.enqueue([](auto workerIndex) noexcept { test_value++; test_worker_index = workerIndex; });
			pool.wait();

			CHECK(test_value == 1);
			CHECK(test_worker_index < pool.workers());
		}
		{
			INFO("# 3")
			test_value = {};
			test_worker_index = 0xFFFFFFFFu;

			pool.enqueue(test_func);
			pool.wait();

			CHECK(test_value == 1);
			CHECK(test_worker_index < pool.workers());
		}
	}

	{
		INFO("a task with state but still small enough to fit in storage")
		{
			INFO("# 1")
			std::atomic_int i = 0;
			auto task = [&]() noexcept { i++; };
			static_assert(sizeof(task) <= storage_threshold);

			pool.enqueue(std::move(task));
			pool.wait();

			CHECK(i == 1);
		}
		{
			INFO("# 2")
			std::atomic_size_t val = 0_sz;
			static_assert(sizeof(callable_counter<size_t>) <= storage_threshold);

			pool.enqueue(callable_counter{ val });
			pool.wait();

			CHECK(val == 1_sz);
		}
		{
			INFO("# 3")
			std::atomic_size_t val = 0_sz;
			static_assert(sizeof(callable_counter<size_t>) <= storage_threshold);

			auto counter = callable_counter{ val };
			pool.enqueue(counter);
			pool.wait();

			CHECK(val == 1_sz);
		}
	}

	{
		INFO("a task with state large enough to require pointer indirection")

		std::atomic_int i = 0;
		struct Kek
		{
			int i;
			std::byte junk[storage_threshold];
		};

		Kek kek{ 68, {} };
		auto task = [&, kek]() mutable noexcept { kek.i++; i = kek.i; };
		static_assert(sizeof(task) > storage_threshold);

		pool.enqueue(task);
		pool.wait();

		CHECK(i == 69);
	}

	{
		INFO("a small task that has alignment requirements high enough to require pointer indirection")

		std::atomic_int i = 0;
		struct Kek
		{
			alignas(impl::thread_pool_alignment << 1) int i;
		};
		static_assert(alignof(Kek) == (impl::thread_pool_alignment << 1));

		Kek kek{ 41 };
		auto task = [&, kek]() mutable noexcept { kek.i++; i = kek.i; };

		pool.enqueue(task);
		pool.wait();
		CHECK(i == 42);
	}

	{
		INFO("a bunch of tasks all at once")

		std::atomic_int i = 0;
		for (int j = 0; j < 200; j++)
		{
			pool.enqueue([&]() noexcept
			{
				std::this_thread::sleep_for(25ms);
				i++;
			});
		}
		pool.wait();
		CHECK(i == 200);
	}
}

TEST_CASE("thread_pool - for_each (integral inputs)")
{
	thread_pool pool{ min(std::thread::hardware_concurrency(), 16u) };

	std::array<int, 1000> values;
	std::vector<int> batches(pool.workers(), 0);

	const auto reset = [&]()noexcept
	{
		MUU_MEMSET(&values, 0, sizeof(values));
		MUU_MEMSET(batches.data(), 0, sizeof(typename remove_cvref<decltype(batches)>::value_type) * batches.size());
	};

	{
		INFO("[A, B)")
		reset();
		pool.for_each(0_sz, values.size(), [&](auto i) noexcept { values[i]++; });
		pool.wait();
		for (auto& v : values)
			CHECK(v == 1);

		reset();
		pool.for_each(0_sz, values.size(), [&](auto i, auto batch) noexcept
		{
			values[i]++;
			batches[batch]++;
		});
		pool.wait();
		for (auto& v : values)
			CHECK(v == 1);
		for (auto& b : batches)
			CHECK(b > 0);
	}

	{
		INFO("[A, B)")
		pool.for_each(10u, 100u, [&](auto i) noexcept { values[i]--; });
		pool.wait();
		for (size_t i = 0; i < 10; i++)
			CHECK(values[i] == 1);
		for (size_t i = 10; i < 100; i++)
			CHECK(values[i] == 0);
		for (size_t i = 100; i < values.size(); i++)
			CHECK(values[i] == 1);
	}

	{
		INFO("[A, B) where A > B")
		reset();
		pool.for_each(500u, 300u, [&](auto i) noexcept { values[i] = 69; });
		pool.wait();
		for (size_t i = 0; i <= 300; i++)
			CHECK(values[i] == 0);
		for (size_t i = 301; i <= 500; i++)
			CHECK(values[i] == 69);
		for (size_t i = 501; i < values.size(); i++)
			CHECK(values[i] == 0);
	}

	{
		INFO("[A, A)")
		reset();
		pool.for_each(100u, 100u, [&](auto i) noexcept { values[i] = 100; });
		pool.wait();
		for (auto& v : values)
			CHECK(v == 0);
	}

	{
		INFO("copy semantics")
		std::atomic_size_t val = 0_sz;
		auto callable = callable_counter{ val };
		pool.for_each(0_sz, values.size(), callable);
		pool.wait();
		CHECK(val == values.size());
	}
	{
		INFO("move semantics")
		std::atomic_size_t val = 0_sz;
		pool.for_each(0_sz, values.size(), callable_counter{ val });
		pool.wait();
		CHECK(val == values.size());
	}
}

TEST_CASE("thread_pool - for_each (iterators)")
{
	thread_pool pool{ min(std::thread::hardware_concurrency(), 16u) };

	std::array<int, 1000> values;
	std::vector<int> batches(pool.workers(), 0);

	const auto reset = [&]()noexcept
	{
		MUU_MEMSET(&values, 0, sizeof(values));
		MUU_MEMSET(batches.data(), 0, sizeof(typename remove_cvref<decltype(batches)>::value_type) * batches.size());
	};

	{
		INFO("collection")
		reset();
		pool.for_each(values, [](auto& v) noexcept { v++; });
		pool.wait();
		for (auto& v : values)
			CHECK(v == 1);
	}

	{
		INFO("collection with batch index")
		reset();
		pool.for_each(values, [&](auto& v, auto batch) noexcept
		{
			v++;
			batches[batch]++;
		});
		pool.wait();
		for (auto& v : values)
			CHECK(v == 1);
		for (auto& b : batches)
			CHECK(b > 0);
	}

	{
		INFO("[begin, end)")
		reset();
		pool.for_each(values.begin(), values.end(), [](auto& v) noexcept { v++; });
		pool.wait();
		for (auto& v : values)
			CHECK(v == 1);
	}

	{
		INFO("[end, begin)")
		reset();
		pool.for_each(values.end(), values.begin(), [](auto& v) noexcept { v++; });
		pool.wait();
		for (auto& v : values)
			CHECK(v == 0);
	}

	{
		INFO("copy semantics")
			std::atomic_size_t val = 0_sz;
		auto callable = callable_counter{ val };
		pool.for_each(values, callable);
		pool.wait();
		CHECK(val == values.size());
	}
	{
		INFO("move semantics")
			std::atomic_size_t val = 0_sz;
		pool.for_each(values, callable_counter{ val });
		pool.wait();
		CHECK(val == values.size());
	}

}
