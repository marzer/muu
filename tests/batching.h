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

template <typename... T>
struct batch_picker;

#if TEST_BATCHES > 1

template <typename... T>
struct batch_picker
{
	static constexpr size_t item_count	  = sizeof...(T);
	static constexpr size_t current_batch = static_cast<size_t>(TEST_BATCH);
	static constexpr size_t total_batches = static_cast<size_t>(TEST_BATCHES);

	MUU_NODISCARD
	MUU_CONSTEVAL
	static auto get_type_list_tag() noexcept
	{
		if constexpr (total_batches == 1_sz)
		{
			return type_list<T...>{};
		}
		else if constexpr (item_count <= total_batches)
		{
			if constexpr (current_batch < item_count)
			{
				return typename type_list<T...>::template slice<current_batch, 1>{};
			}
			else
			{
				return type_list<>{};
			}
		}
		else
		{
			struct range_t
			{
				size_t start;
				size_t size;
			};
			constexpr auto range = []() noexcept
			{
				constexpr size_t constant = item_count / total_batches;
				size_t overflow			  = item_count % total_batches;
				range_t val				  = {};

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
			}();

			return typename type_list<T...>::template slice<range.start, range.size>{};
		}
	}

	using types = decltype(get_type_list_tag());
};

#else

template <typename... T>
struct batch_picker
{
	using types = type_list<T...>;
};

#endif

template <typename... T>
struct batch_picker<type_list<T...>>
{
	using types = typename batch_picker<T...>::types;
};
template <>
struct batch_picker<type_list<>>
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

#define TEMPLATE_BATCHED_TEST_IMPL(name, func_name, ...)                                                               \
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

#define TEMPLATE_BATCHED_TEST(name, ...)                                                                               \
	TEMPLATE_BATCHED_TEST_IMPL(name, MUU_CONCAT(TEMPLATE_BATCHED_TEST_func_, __LINE__), __VA_ARGS__)

#if TEST_BATCHES > 1
	#define BATCHED_SECTION(...) DYNAMIC_SECTION(__VA_ARGS__ << " - "sv << nameof<T> << TEST_BATCH_SUFFIX)
#else
	#define BATCHED_SECTION(...) DYNAMIC_SECTION(__VA_ARGS__ << " - "sv << nameof<T>)
#endif
