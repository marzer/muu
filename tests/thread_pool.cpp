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
	{
		thread_pool threads{ 1u, 1u };
		CHECK(threads.size() == 1u);

		thread_pool threads2{ std::move(threads) };
		CHECK(threads.size() == 0u);
		CHECK(threads2.size() == 1u);
	}

	{
		thread_pool threads{ 1u, 10u };
		CHECK(threads.size() == 1u);
	}

	{
		thread_pool threads;
		CHECK(threads.size() == std::thread::hardware_concurrency());
	}

	{
		thread_pool threads{ 0u, 10u };
		CHECK(threads.size() == std::thread::hardware_concurrency());
	}
}

namespace
{
	int test_value = {};
	size_t test_worker_index = {};
}

TEST_CASE("thread_pool - enqueue")
{
	thread_pool threads;

	//tasks with no state at all
	{
		test_value = {};
		auto task = []() noexcept { test_value++; };
		static_assert(!impl::thread_pool_task::requires_heap<decltype(task)>);
		threads.enqueue(std::move(task));
		threads.wait();
		CHECK(test_value == 1);
	}
	{
		test_value = {};
		test_worker_index = 0xFFFFFFFFu;
		auto task = [](auto workerIndex) noexcept { test_value++; test_worker_index = workerIndex; };
		static_assert(!impl::thread_pool_task::requires_heap<decltype(task)>);

		threads.enqueue(std::move(task));
		threads.wait();
		CHECK(test_value == 1);
		CHECK(test_worker_index < threads.size());
	}

	//a task with state but still small enough to not require the heap
	{
		std::atomic_int i = 0;
		auto task = [&]() noexcept { i++; };
		static_assert(sizeof(task) < impl::thread_pool_task::heap_free_threshold);
		static_assert(!impl::thread_pool_task::requires_heap<decltype(task)>);

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
			std::byte junk[impl::thread_pool_task::heap_free_threshold];
		};

		Kek kek{ 68, {} };
		auto task = [&, kek]() mutable noexcept { kek.i++; i = kek.i; };
		static_assert(sizeof(task) > impl::thread_pool_task::heap_free_threshold);
		static_assert(impl::thread_pool_task::requires_heap<decltype(task)>);

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
		static_assert(impl::thread_pool_task::requires_heap<decltype(task)>);

		threads.enqueue(std::move(task));
		threads.wait();
		CHECK(i == 42);
	}

	//a bunch of tasks all at once
	{
		std::atomic_int i = 0;
		for (int j = 0; j < 200; j++)
			threads.enqueue([&]() noexcept { std::this_thread::sleep_for(10ms);  i++; });
		threads.wait();
		CHECK(i == 200);
	}
}

#if 0

TEST_CASE("thread_pool - for")
{
	thread_pool threads;
	std::array<int, 1000> values;
	std::vector<int> threadValues(threads.size(), 0);

	//[A, B)
	memset(&values, 0, sizeof(values));
	threads.For(0u, values.size(), [&](auto i) noexcept { values[i]++; });
	threads.wait();
	for (auto& v : values)
		CHECK(v == 1);
	memset(&values, 0, sizeof(values));
	threads.For(0u, values.size(), [&](auto i, auto workerIndex) noexcept { values[i]++; threadValues[workerIndex]++; Sleep(1ms); });
	threads.wait();
	for (auto& v : values)
		CHECK(v == 1);
	{
		size_t usedThreads{};
		for (auto& v : threadValues)
			if (v > 0)
				usedThreads++;
		CHECK(usedThreads >= 2u * threads.size() / 3u);
	}

	//[A, B)
	threads.For(10u, 100u, [&](auto i) noexcept { values[i]--; });
	threads.wait();
	for (size_t i = 0; i < 10; i++)
		CHECK(values[i] == 1);
	for (size_t i = 10; i < 100; i++)
		CHECK(values[i] == 0);
	for (size_t i = 100; i < values.size(); i++)
		CHECK(values[i] == 1);

	//[A, B) where A > B
	memset(&values, 0, sizeof(values));
	threads.For(500u, 300u, [&](auto i) noexcept { values[i] = 69; });
	threads.wait();
	for (size_t i = 0; i <= 300; i++)
		CHECK(values[i] == 0);
	for (size_t i = 301; i <= 500; i++)
		CHECK(values[i] == 69);
	for (size_t i = 501; i < values.size(); i++)
		CHECK(values[i] == 0);

	//[A, A)
	memset(&values, 0, sizeof(values));
	threads.For(100u, 100u, [&](auto i) noexcept { values[i] = 100; });
	threads.wait();
	for (auto& v : values)
		CHECK(v == 0);
}

TEST_CASE("thread_pool - for_each")
{
	thread_pool threads;
	std::array<int, 1000> values;
	std::vector<int> threadValues(threads.size(), 0);

	memset(&values, 0, sizeof(values));
	threads.ForEach(values, [&](auto& v) noexcept { v++; });
	threads.wait();
	for (auto& v : values)
		CHECK(v == 1);
	memset(&values, 0, sizeof(values));
	threads.ForEach(values, [&](auto& v, auto workerIndex) noexcept { v++; threadValues[workerIndex]++; Sleep(1ms); });
	threads.wait();
	for (auto& v : values)
		CHECK(v == 1);
	{
		size_t usedThreads{};
		for (auto& v : threadValues)
			if (v > 0)
				usedThreads++;
		CHECK(usedThreads >= 2u * threads.size() / 3u);
	}
}
#endif
