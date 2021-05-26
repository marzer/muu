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

template <typename T>
struct type_tag
{
	using type = T;
};

template <auto I>
struct index_tag
{
	static constexpr auto value = I;
};

template <typename... T>
struct batch_picker
{
	static constexpr size_t item_count	  = sizeof...(T);
	static constexpr size_t current_batch = static_cast<size_t>(TEST_BATCH);
	static constexpr size_t total_batches = static_cast<size_t>(TEST_BATCHES);

	MUU_NODISCARD
	MUU_CONSTEVAL
	static auto get_tuple_tag() noexcept
	{
		if constexpr (total_batches == 1_sz)
		{
			return type_tag<std::tuple<T...>>{};
		}
		else if constexpr (item_count <= total_batches)
		{
			if constexpr (current_batch < item_count)
			{
				return type_tag<std::tuple<std::tuple_element_t<current_batch, std::tuple<T...>>>>{};
			}
			else
			{
				return type_tag<std::tuple<>>{};
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

			using subset = decltype(tuple_subset<range.start>(std::declval<std::tuple<T...>>(),
															  std::make_index_sequence<range.size>{}));
			return type_tag<subset>{};
		}
	}

	using types = typename decltype(get_tuple_tag())::type;
};
template <typename... T>
struct batch_picker<std::tuple<T...>>
{
	using types = typename batch_picker<T...>::types;
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
	TEST_CASE(name TEST_BATCH_SUFFIX, "")                                                                              \
	{                                                                                                                  \
		muu::for_sequence<std::tuple_size_v<batched<__VA_ARGS__>>>(                                                    \
			[](auto i)                                                                                                 \
			{                                                                                                          \
				using TestType = std::tuple_element_t<decltype(i)::value, batched<__VA_ARGS__>>;                       \
				func_name<TestType>();                                                                                 \
			});                                                                                                        \
	}                                                                                                                  \
	template <typename TestType>                                                                                       \
	static void func_name()

#define TEMPLATE_BATCHED_TEST(name, ...)                                                                               \
	TEMPLATE_BATCHED_TEST_IMPL(name, MUU_CONCAT(TEMPLATE_BATCHED_TEST_func_, __LINE__), __VA_ARGS__)

#if TEST_BATCHES > 1
	#define BATCHED_SECTION(...) DYNAMIC_SECTION(__VA_ARGS__ << " - "sv << nameof<T> << TEST_BATCH_SUFFIX)
#else
	#define BATCHED_SECTION(...) DYNAMIC_SECTION(__VA_ARGS__ << " - "sv << nameof<T>)
#endif
