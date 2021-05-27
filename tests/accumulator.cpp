// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "batching.h"
#include "../include/muu/accumulator.h"

namespace
{
	template <typename... T>
	using accumulators = type_list<accumulator<T>...>;
}

BATCHED_TEST_CASE("accumulator", accumulators<ALL_ARITHMETIC>)
{
	using accumulator = TestType;
	using T			  = typename accumulator::value_type;
	TEST_INFO("accumulator<"sv << nameof<T> << ">"sv);

	if constexpr (is_floating_point<T>)
	{
		using data = float_test_data<T>;

		using big = impl::highest_ranked<T, long double>;

		BATCHED_SECTION("add(value)")
		{
			T raw_sum = {};
			accumulator accum;
			CHECK(accum.sample_count() == 0_sz);
			CHECK(accum.sum() == T{});
			for (auto val : data::values)
			{
				raw_sum = static_cast<T>(raw_sum + static_cast<T>(val));
				accum.add(static_cast<T>(val));
			}
			CHECK(accum.sample_count() == std::size(data::values));
			CHECK(static_cast<big>(raw_sum) != data::values_sum);
			CHECK((muu::abs(static_cast<big>(data::values_sum) - static_cast<big>(accum.sum())))
				  <= (muu::abs(static_cast<big>(data::values_sum) - static_cast<big>(raw_sum))));
			CHECK(static_cast<big>(accum.sum()) >= static_cast<big>(data::values_sum_low));
			CHECK(static_cast<big>(accum.sum()) <= static_cast<big>(data::values_sum_high));
			if constexpr (sizeof(T) > 2u)
			{
				CHECK(static_cast<big>(accum.min()) == Approx(static_cast<big>(data::values_min)));
				CHECK(static_cast<big>(accum.max()) == Approx(static_cast<big>(data::values_max)));
			}
		}

		BATCHED_SECTION("add(begin, end)")
		{
			accumulator accum;
			accum.add(std::begin(data::values), std::end(data::values));
			CHECK(accum.sample_count() == std::size(data::values));
			CHECK(static_cast<big>(accum.sum()) >= static_cast<big>(data::values_sum_low));
			CHECK(static_cast<big>(accum.sum()) <= static_cast<big>(data::values_sum_high));
			if constexpr (sizeof(T) > 2u)
			{
				CHECK(static_cast<big>(accum.min()) == Approx(static_cast<big>(data::values_min)));
				CHECK(static_cast<big>(accum.max()) == Approx(static_cast<big>(data::values_max)));
			}
		}
	}
	else
	{
		accumulator accum;
		using sum_type	 = remove_cvref<decltype(accum.sum())>;
		sum_type raw_sum = {};
		CHECK(accum.sample_count() == 0u);
		CHECK(accum.sum() == sum_type{});

		constexpr auto next = []() noexcept
		{
			using rand_type = largest<make_unsigned<T>, unsigned long long>;
			static constexpr auto limit =
				(muu::min)(static_cast<rand_type>(constants<T>::highest), rand_type{ 32768ull });
			return static_cast<T>(static_cast<rand_type>(rand()) % limit);
		};
		for (size_t i = 0; i < 1000; i++)
		{
			const auto val = next();
			raw_sum += val;
			accum.add(val);
		}
		CHECK(accum.sample_count() == 1000u);
		CHECK(accum.sum() == raw_sum);
	}
}
