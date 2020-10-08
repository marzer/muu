// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/accumulator.h"
#include "../include/muu/half.h"

namespace
{
	template <typename T>
	static void test_float_accumulator() noexcept
	{
		using data = float_test_data<T>;

		using big = impl::highest_ranked<T, long double>;

		//test calling add() for each value
		{
			T raw_sum = {};
			accumulator<T> accum;
			CHECK(accum.sample_count() == 0_sz);
			CHECK(accum.sum() == T{});
			for (auto val : data::values)
			{
				raw_sum = static_cast<T>(raw_sum + static_cast<T>(val));
				accum.add(static_cast<T>(val));
			}
			CHECK(accum.sample_count() == std::size(data::values));
			CHECK(static_cast<big>(raw_sum) != data::values_sum);
			CHECK((abs(static_cast<big>(data::values_sum) - static_cast<big>(accum.sum())))
				<= (abs(static_cast<big>(data::values_sum) - static_cast<big>(raw_sum))));
			CHECK(static_cast<big>(accum.sum()) >= static_cast<big>(data::values_sum_low));
			CHECK(static_cast<big>(accum.sum()) <= static_cast<big>(data::values_sum_high));
			if constexpr (sizeof(T) > 2u)
			{
				CHECK(static_cast<big>(accum.min()) == Approx(static_cast<big>(data::values_min)));
				CHECK(static_cast<big>(accum.max()) == Approx(static_cast<big>(data::values_max)));
			}
		}

		//test calling add(begin, end)
		{
			accumulator<T> accum;
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

	template <typename T>
	static void test_int_accumulator() noexcept
	{
		accumulator<T> accum;
		using sum_type = remove_cvref<decltype(accum.sum())>;
		sum_type raw_sum = {};
		CHECK(accum.sample_count() == 0u);
		CHECK(accum.sum() == sum_type{});

		constexpr auto next = []() noexcept
		{
			using rand_type = largest<make_unsigned<T>, unsigned long long>;
			static constexpr auto limit = (muu::min)(static_cast<rand_type>(constants<T>::highest), rand_type{ 32768ull });
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

	template <typename T>
	static void test_accumulator() noexcept
	{
		if constexpr (is_floating_point<T>)
			test_float_accumulator<T>();
		else
			test_int_accumulator<T>();
	}
}

#define CHECK_ACCUMULATOR(type)		\
	SECTION(#type)					\
	{								\
		test_accumulator<type>();	\
	} (void)0


TEST_CASE("accumulator")
{
	#if MUU_HAS_FP16
	CHECK_ACCUMULATOR(__fp16);
	#endif
	#if MUU_HAS_FLOAT16
	CHECK_ACCUMULATOR(_Float16);
	#endif
	CHECK_ACCUMULATOR(half);
	CHECK_ACCUMULATOR(float);
	CHECK_ACCUMULATOR(double);
	CHECK_ACCUMULATOR(long double);
	#if MUU_HAS_FLOAT128
	CHECK_ACCUMULATOR(quad);
	#endif

	CHECK_ACCUMULATOR(signed char);
	CHECK_ACCUMULATOR(unsigned char);
	CHECK_ACCUMULATOR(signed short);
	CHECK_ACCUMULATOR(unsigned short);
	CHECK_ACCUMULATOR(signed int);
	CHECK_ACCUMULATOR(unsigned int);
	CHECK_ACCUMULATOR(signed long);
	CHECK_ACCUMULATOR(unsigned long);
	CHECK_ACCUMULATOR(signed long long);
	CHECK_ACCUMULATOR(unsigned long long);
	#if MUU_HAS_INT128
	CHECK_ACCUMULATOR(int128_t);
	CHECK_ACCUMULATOR(uint128_t);
	#endif
}
