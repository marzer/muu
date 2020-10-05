// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
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
		std::this_thread::sleep_for(20ms);
	};

	{
		INFO("single worker, queue size == 1")

		thread_pool threads{ 1u, 1u };
		CHECK(threads.size() == 1u);

		thread_pool threads2{ std::move(threads) };
		CHECK(threads.size() == 0u);
		CHECK(threads2.size() == 1u);
		threads2.for_range(0_sz, threads2.size(), anti_hang);

		std::this_thread::sleep_for(100ms);
	}

	{
		INFO("single worker, queue size == 10")

		thread_pool threads{ 1u, 10u };
		CHECK(threads.size() == 1u);
		threads.for_range(0_sz, threads.size(), anti_hang);

		std::this_thread::sleep_for(100ms);
	}

	{
		INFO("auto workers, auto queue size")

		thread_pool threads;
		CHECK(threads.size() == std::thread::hardware_concurrency());
		threads.for_range(0_sz, threads.size(), anti_hang);

		std::this_thread::sleep_for(100ms);
	}

	{
		INFO("auto workers, queue size == 10")
		thread_pool threads{ 0u, 10u };
		threads.for_range(0_sz, threads.size(), anti_hang);

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
	thread_pool threads;
	CHECK(threads.size() == std::thread::hardware_concurrency());

	//tasks with no state at all
	{
		test_value = {};

		threads.enqueue([]() noexcept { test_value++; });
		threads.wait();

		CHECK(test_value == 1);
	}
	{
		test_value = {};
		test_worker_index = 0xFFFFFFFFu;

		threads.enqueue([](auto workerIndex) noexcept { test_value++; test_worker_index = workerIndex; });
		threads.wait();

		CHECK(test_value == 1);
		CHECK(test_worker_index < threads.size());
	}
	{
		test_value = {};
		test_worker_index = 0xFFFFFFFFu;

		threads.enqueue(test_func);
		threads.wait();

		CHECK(test_value == 1);
		CHECK(test_worker_index < threads.size());
	}

	//a task with state but still small enough to not require the heap
	{
		std::atomic_int i = 0;
		auto task = [&]() noexcept { i++; };
		static_assert(sizeof(task) < heap_free_threshold);

		threads.enqueue(std::move(task));
		threads.wait();

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

		threads.enqueue(std::move(task));
		threads.wait();

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

		threads.enqueue(std::move(task));
		threads.wait();
		CHECK(i == 42);
	}

	//a bunch of tasks all at once
	{
		std::atomic_int i = 0;
		for (int j = 0; j < 200; j++)
		{
			threads.enqueue([&]() noexcept
			{
				std::this_thread::sleep_for(10ms);
				i++;
			});
		}
		threads.wait();
		CHECK(i == 200);
	}
}

TEST_CASE("thread_pool - for_range")
{
	thread_pool threads;
	CHECK(threads.size() == std::thread::hardware_concurrency());

	std::array<int, 1000> values;
	std::vector<int> thread_values(threads.size(), 0);

	//[A, B)
	memset(&values, 0, sizeof(values));
	threads.for_range(0_sz, values.size(), [&](auto i) noexcept { values[i]++; });
	threads.wait();
	for (auto& v : values)
		CHECK(v == 1);
	memset(&values, 0, sizeof(values));
	threads.for_range(0_sz, values.size(), [&](auto i, auto workerIndex) noexcept
	{
		values[i]++;
		thread_values[workerIndex]++;
		std::this_thread::sleep_for(10ms);
	});
	threads.wait();
	for (auto& v : values)
		CHECK(v == 1);
	{
		size_t used_threads{};
		for (auto& v : thread_values)
			if (v > 0)
				used_threads++;
		CHECK(used_threads >= 2u * threads.size() / 3u);
	}

	//[A, B)
	threads.for_range(10u, 100u, [&](auto i) noexcept { values[i]--; });
	threads.wait();
	for (size_t i = 0; i < 10; i++)
		CHECK(values[i] == 1);
	for (size_t i = 10; i < 100; i++)
		CHECK(values[i] == 0);
	for (size_t i = 100; i < values.size(); i++)
		CHECK(values[i] == 1);

	//[A, B) where A > B
	memset(&values, 0, sizeof(values));
	threads.for_range(500u, 300u, [&](auto i) noexcept { values[i] = 69; });
	threads.wait();
	for (size_t i = 0; i <= 300; i++)
		CHECK(values[i] == 0);
	for (size_t i = 301; i <= 500; i++)
		CHECK(values[i] == 69);
	for (size_t i = 501; i < values.size(); i++)
		CHECK(values[i] == 0);

	//[A, A)
	memset(&values, 0, sizeof(values));
	threads.for_range(100u, 100u, [&](auto i) noexcept { values[i] = 100; });
	threads.wait();
	for (auto& v : values)
		CHECK(v == 0);
}

TEST_CASE("thread_pool - for_each")
{
	thread_pool threads;
	CHECK(threads.size() == std::thread::hardware_concurrency());

	std::array<int, 1000> values;
	std::vector<int> thread_values(threads.size(), 0);

	// collection
	memset(&values, 0, sizeof(values));
	threads.for_each(values, [&](auto& v) noexcept { v++; });
	threads.wait();
	for (auto& v : values)
		CHECK(v == 1);

	// [begin, end)
	memset(&values, 0, sizeof(values));
	threads.for_each(values.begin(), values.end(), [&](auto& v) noexcept { v++; });
	threads.wait();
	for (auto& v : values)
		CHECK(v == 1);

	// [end, begin) (should be no-op)
	memset(&values, 0, sizeof(values));
	threads.for_each(values.end(), values.begin(), [&](auto& v) noexcept { v++; });
	threads.wait();
	for (auto& v : values)
		CHECK(v == 0);

	// collection with thread_index reader
	memset(&values, 0, sizeof(values));
	threads.for_each(values, [&](auto& v, auto workerIndex) noexcept
	{
		v++;
		thread_values[workerIndex]++;
		std::this_thread::sleep_for(1ms);
	});
	threads.wait();
	for (auto& v : values)
		CHECK(v == 1);
	{
		size_t used_threads{};
		for (auto& v : thread_values)
			if (v > 0)
				used_threads++;
		CHECK(used_threads >= 2u * threads.size() / 3u);
	}
}

