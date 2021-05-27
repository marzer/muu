// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"
#include "batching.h"
#include "../include/muu/core.h"

namespace
{
#define INF_OR_NAN_CHECK(expr)                                                                                         \
	static_assert(MUU_INTELLISENSE || !build::supports_constexpr_infinity_or_nan || (expr));                           \
	CHECK(expr)

	template <typename T>
	static void test_nan_ranges()
	{
		using data = float_test_data<T>;

		INF_OR_NAN_CHECK(infinity_or_nan(muu::bit_cast<T>(data::bits_pos_nan_min)));
		INF_OR_NAN_CHECK(infinity_or_nan(muu::bit_cast<T>(data::bits_pos_nan_max)));
		INF_OR_NAN_CHECK(infinity_or_nan(muu::bit_cast<T>(data::bits_neg_nan_min)));
		INF_OR_NAN_CHECK(infinity_or_nan(muu::bit_cast<T>(data::bits_neg_nan_max)));

		if constexpr (data::int_blittable)
		{
			constexpr auto test_range = [](auto min_, auto max_)
			{
				using blit_type		  = decltype(min_);
				const blit_type first = (min)(min_, max_); // normalize for sign/endiannness
				const blit_type last  = (max)(min_, max_); //
				TEST_INFO("first: "sv << first);
				TEST_INFO(" last: "sv << last);

				REQUIRE(infinity_or_nan(muu::bit_cast<T>(first)));
				REQUIRE(infinity_or_nan(muu::bit_cast<T>(last)));

				if constexpr (constants<T>::significand_digits < 23)
				{
					for (auto bits = first; bits < last; bits++)
						REQUIRE(infinity_or_nan(muu::bit_cast<T>(bits)));
				}
				else
				{
					const auto step = ((last - first) + blit_type{ 1 }) / blit_type{ 16384 };
					TEST_INFO(" step: "sv << step);

					auto bits = first;
					for (unsigned i = 0; i < 16384u; i++, bits += step)
						REQUIRE(infinity_or_nan(muu::bit_cast<T>(bits)));
				}
			};

			test_range(data::bits_pos_nan_min, data::bits_pos_nan_max);
			test_range(data::bits_neg_nan_min, data::bits_neg_nan_max);
		}
	}
}

BATCHED_TEST_CASE("core - infinity_or_nan", type_list<ALL_FLOATS>)
{
	using T = TestType;
	TEST_INFO(nameof<T>);

	INF_OR_NAN_CHECK(!infinity_or_nan(T{}));
	INF_OR_NAN_CHECK(infinity_or_nan(make_nan<T>()));
	INF_OR_NAN_CHECK(infinity_or_nan(make_infinity<T>(-1)));
	INF_OR_NAN_CHECK(infinity_or_nan(make_infinity<T>()));

	test_nan_ranges<T>();
	test_nan_ranges<T>();
}
