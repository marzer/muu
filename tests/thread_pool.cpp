// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/thread_pool.h"
MUU_DISABLE_WARNINGS
#include <thread>
#include <atomic>
MUU_ENABLE_WARNINGS
MUU_DISABLE_SPAM_WARNINGS

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
		pool2.for_range(0_sz, pool2.workers(), anti_hang);

		std::this_thread::sleep_for(100ms);
	}

	{
		INFO("single worker, queue size == 10")

		thread_pool pool { 1u, 10u };
		CHECK(pool.workers() == 1u);
		pool.for_range(0_sz, pool.workers(), anti_hang);

		std::this_thread::sleep_for(100ms);
	}

	{
		INFO("auto workers, auto queue size")

		thread_pool pool;
		CHECK(pool.workers() == std::thread::hardware_concurrency());
		pool.for_range(0_sz, pool.workers(), anti_hang);

		std::this_thread::sleep_for(100ms);
	}

	{
		INFO("auto workers, queue size == 10")
		thread_pool pool { 0u, 10u };
		pool.for_range(0_sz, pool.workers(), anti_hang);

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

	static constexpr auto heap_free_threshold = sizeof(impl::thread_pool_task::callable_buffer_type);
}

TEST_CASE("thread_pool - enqueue")
{
	thread_pool pool{ min(std::thread::hardware_concurrency(), 16u) };

	//tasks with no state at all
	{
		test_value = {};

		pool.enqueue([]() noexcept { test_value++; });
		pool.wait();

		CHECK(test_value == 1);
	}
	{
		test_value = {};
		test_worker_index = 0xFFFFFFFFu;

		pool.enqueue([](auto workerIndex) noexcept { test_value++; test_worker_index = workerIndex; });
		pool.wait();

		CHECK(test_value == 1);
		CHECK(test_worker_index < pool.workers());
	}
	{
		test_value = {};
		test_worker_index = 0xFFFFFFFFu;

		pool.enqueue(test_func);
		pool.wait();

		CHECK(test_value == 1);
		CHECK(test_worker_index < pool.workers());
	}

	//a task with state but still small enough to not require the heap
	{
		std::atomic_int i = 0;
		auto task = [&]() noexcept { i++; };
		static_assert(sizeof(task) < heap_free_threshold);

		pool.enqueue(std::move(task));
		pool.wait();

		CHECK(i == 1);
	}

	//a task with state large enough to require the heap
	{
		std::atomic_int i = 0;
		struct Kek
		{
			int i;
			std::byte junk[heap_free_threshold];
		};

		Kek kek{ 68, {} };
		auto task = [&, kek]() mutable noexcept { kek.i++; i = kek.i; };
		static_assert(sizeof(task) > heap_free_threshold);

		pool.enqueue(std::move(task));
		pool.wait();

		CHECK(i == 69);
	}

	//a small task that has alignment requirements high enough to require the heap
	{
		std::atomic_int i = 0;
		struct Kek
		{
			alignas(impl::thread_pool_task_granularity << 1) int i;
		};
		static_assert(alignof(Kek) == (impl::thread_pool_task_granularity << 1));

		Kek kek{ 41 };
		auto task = [&, kek]() mutable noexcept { kek.i++; i = kek.i; };

		pool.enqueue(std::move(task));
		pool.wait();
		CHECK(i == 42);
	}

	//a bunch of tasks all at once
	{
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

TEST_CASE("thread_pool - for_range")
{
	thread_pool pool{ min(std::thread::hardware_concurrency(), 16u) };

	std::array<int, 1000> values;
	std::vector<int> thread_values(pool.workers(), 0);

	//[A, B)
	memset(&values, 0, sizeof(values));
	pool.for_range(0_sz, values.size(), [&](auto i) noexcept { values[i]++; });
	pool.wait();
	for (auto& v : values)
		CHECK(v == 1);
	memset(&values, 0, sizeof(values));
	pool.for_range(0_sz, values.size(), [&](auto i, auto workerIndex) noexcept
	{
		values[i]++;
		thread_values[workerIndex]++;
	});
	pool.wait();
	for (auto& v : values)
		CHECK(v == 1);

	//[A, B)
	pool.for_range(10u, 100u, [&](auto i) noexcept { values[i]--; });
	pool.wait();
	for (size_t i = 0; i < 10; i++)
		CHECK(values[i] == 1);
	for (size_t i = 10; i < 100; i++)
		CHECK(values[i] == 0);
	for (size_t i = 100; i < values.size(); i++)
		CHECK(values[i] == 1);

	//[A, B) where A > B
	memset(&values, 0, sizeof(values));
	pool.for_range(500u, 300u, [&](auto i) noexcept { values[i] = 69; });
	pool.wait();
	for (size_t i = 0; i <= 300; i++)
		CHECK(values[i] == 0);
	for (size_t i = 301; i <= 500; i++)
		CHECK(values[i] == 69);
	for (size_t i = 501; i < values.size(); i++)
		CHECK(values[i] == 0);

	//[A, A)
	memset(&values, 0, sizeof(values));
	pool.for_range(100u, 100u, [&](auto i) noexcept { values[i] = 100; });
	pool.wait();
	for (auto& v : values)
		CHECK(v == 0);
}

TEST_CASE("thread_pool - for_each")
{
	thread_pool pool{ min(std::thread::hardware_concurrency(), 16u) };

	std::array<int, 1000> values;
	std::vector<int> thread_values(pool.workers(), 0);

	// collection
	memset(&values, 0, sizeof(values));
	pool.for_each(values, [&](auto& v) noexcept { v++; });
	pool.wait();
	for (auto& v : values)
		CHECK(v == 1);

	// [begin, end)
	memset(&values, 0, sizeof(values));
	pool.for_each(values.begin(), values.end(), [&](auto& v) noexcept { v++; });
	pool.wait();
	for (auto& v : values)
		CHECK(v == 1);

	// [end, begin) (should be no-op)
	memset(&values, 0, sizeof(values));
	pool.for_each(values.end(), values.begin(), [&](auto& v) noexcept { v++; });
	pool.wait();
	for (auto& v : values)
		CHECK(v == 0);

	// collection with thread_index reader
	memset(&values, 0, sizeof(values));
	pool.for_each(values, [&](auto& v, auto workerIndex) noexcept
	{
		v++;
		thread_values[workerIndex]++;
	});
	pool.wait();
	for (auto& v : values)
		CHECK(v == 1);
}

