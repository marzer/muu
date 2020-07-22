// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/accumulator.h"
#include "../include/muu/half.h"

MUU_PUSH_WARNINGS
MUU_DISABLE_ARITHMETIC_WARNINGS

template <typename T>
static void test_float_accumulator() noexcept
{
	using data = float_test_data<T>;

	using big = largest<T, long double>;

	//test calling add() for each value
	{
		T raw_sum = {};
		accumulator<T> accum;
		CHECK(accum.sample_count() == 0_sz);
		CHECK(accum.value() == T{});
		for (auto val : data::values)
		{
			raw_sum += static_cast<T>(val);
			accum.add(static_cast<T>(val));
		}
		CHECK(accum.sample_count() == std::size(data::values));
		CHECK(static_cast<big>(raw_sum) != data::values_sum);
		CHECK((abs(static_cast<big>(data::values_sum) - static_cast<big>(accum.value())))
		   <= (abs(static_cast<big>(data::values_sum) - static_cast<big>(raw_sum))));
		CHECK(static_cast<big>(accum.value()) >= static_cast<big>(data::values_sum_low));
		CHECK(static_cast<big>(accum.value()) <= static_cast<big>(data::values_sum_high));
	}

	//test calling add(begin, end)
	//if constexpr (!std::is_same_v<T, half>)
	{
		accumulator<T> accum;
		accum.add(std::begin(data::values), std::end(data::values));
		CHECK(accum.sample_count() == std::size(data::values));
		CHECK(static_cast<big>(accum.value()) >= static_cast<big>(data::values_sum_low));
		CHECK(static_cast<big>(accum.value()) <= static_cast<big>(data::values_sum_high));
	}
}

#if MUU_HAS_FLOAT16

TEST_CASE("accumulator - float16_t")
{
	test_float_accumulator<float16_t>();
}

#endif // MUU_HAS_FLOAT16

TEST_CASE("accumulator - half")
{
	test_float_accumulator<half>();
}

TEST_CASE("accumulator - float")
{
	test_float_accumulator<float>();
}

TEST_CASE("accumulator - double")
{
	test_float_accumulator<double>();
}

TEST_CASE("accumulator - long double")
{
	test_float_accumulator<long double>();
}

TEST_CASE("accumulator - integers")
{
	int raw_sum = {};
	accumulator<int> accum;
	CHECK(accum.sample_count() == 0_sz);
	CHECK(accum.value() == 0);
	for (size_t i = 0; i < 1000; i++)
	{
		const auto val = rand();
		raw_sum += val;
		accum.add(val);
	}
	CHECK(accum.sample_count() == 1000_sz);
	CHECK(accum.value() == raw_sum);
}

MUU_POP_WARNINGS // MUU_DISABLE_ARITHMETIC_WARNINGS
