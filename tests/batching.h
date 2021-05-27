// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"

#if !defined(TEST_BATCHES) || TEST_BATCHES <= 0
	#undef TEST_BATCHES
	#define TEST_BATCHES 1
	#undef TEST_BATCH
	#define TEST_BATCH 0
#endif
#if TEST_BATCHES > 16
	#error TEST_BATCHES is limited to a max of 16.
#endif
#if !defined(TEST_BATCH) || TEST_BATCH < 0
	#undef TEST_BATCH
	#define TEST_BATCH 0
#endif
#ifndef TEST_BATCH_INFIX
	#define TEST_BATCH_INFIX
#endif

struct batch_range_vals
{
	size_t start;
	size_t size;
};

template <size_t ItemCount>
struct batch_range
{
	static constexpr batch_range_vals value = []() noexcept
	{
		constexpr size_t current_batch = static_cast<size_t>(TEST_BATCH);
		constexpr size_t total_batches = static_cast<size_t>(TEST_BATCHES);

		if constexpr (total_batches == 1_sz)
		{
			return batch_range_vals{ 0, ItemCount };
		}
		else if constexpr (ItemCount <= total_batches)
		{
			if constexpr (current_batch < ItemCount)
			{
				return batch_range_vals{ current_batch, 1_sz };
			}
			else
			{
				return batch_range_vals{};
			}
		}
		else
		{
			constexpr size_t constant = ItemCount / total_batches;
			size_t overflow			  = ItemCount % total_batches;
			batch_range_vals val	  = {};

			size_t i = 0;
			do
			{
				val.start += val.size;
				val.size = constant;
				if (overflow)
				{
					val.size++;
					overflow--;
				}
			}
			while (++i <= current_batch);

			return val;
		}
	}();
};

template <typename... T>
struct batch_picker;

template <typename... T>
struct batch_picker<type_list<T...>>
{
	using range = batch_range<sizeof...(T)>;
	using types = typename type_list<T...>::template slice<range::value.start, range::value.size>;
};

template <typename T0, typename... T>
struct batch_picker<T0, T...> : batch_picker<type_list<T0, T...>>
{};

template <>
struct batch_picker<type_list<>>
{
	using types = type_list<>;
};

template <>
struct batch_picker<>
{
	using types = type_list<>;
};

template <typename... T>
using batched = typename batch_picker<T...>::types;

#if TEST_BATCHES > 1
	#define TEST_BATCH_SUFFIX " - batch " MUU_MAKE_STRING(TEST_BATCH)
#else
	#define TEST_BATCH_SUFFIX
#endif

#define BATCHED_TEST_CASE_IMPL(name, func_name, ...)                                                                   \
	template <typename TestType>                                                                                       \
	static void func_name();                                                                                           \
	MUU_PUSH_WARNINGS;                                                                                                 \
	MUU_PRAGMA_MSVC(warning(disable : 4296))                                                                           \
	TEST_CASE(name TEST_BATCH_SUFFIX, "")                                                                              \
	{                                                                                                                  \
		if constexpr (batched<__VA_ARGS__>::length > 0)                                                                \
		{                                                                                                              \
			muu::for_sequence<batched<__VA_ARGS__>::length>(                                                           \
				[](auto i)                                                                                             \
				{                                                                                                      \
					using TestType = batched<__VA_ARGS__>::select<decltype(i)::value>;                                 \
					func_name<TestType>();                                                                             \
				});                                                                                                    \
		}                                                                                                              \
	}                                                                                                                  \
	MUU_POP_WARNINGS;                                                                                                  \
	template <typename TestType>                                                                                       \
	static void func_name()

#define BATCHED_TEST_CASE(name, ...)                                                                                   \
	BATCHED_TEST_CASE_IMPL(                                                                                            \
		name,                                                                                                          \
		MUU_CONCAT(MUU_CONCAT(BATCHED_TEST_CASE_, TEST_BATCH_INFIX), MUU_CONCAT(_, __LINE__)), __VA_ARGS__)

#if TEST_BATCHES > 1
	#define BATCHED_SECTION(...) DYNAMIC_SECTION(__VA_ARGS__ << " - "sv << nameof<T> << TEST_BATCH_SUFFIX)
#else
	#define BATCHED_SECTION(...) DYNAMIC_SECTION(__VA_ARGS__ << " - "sv << nameof<T>)
#endif
