// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

//#pragma once /* no! explicitly allowed to be re-included */

#include "vector_test_templates.h"

#if !defined(SCALAR_TYPE) && MUU_INTELLISENSE
	#define SCALAR_TYPE float
#endif
#ifndef SCALAR_TYPE
	#error SCALAR_TYPE was not defined
#endif

#undef TEST_TYPE
#define TEST_TYPE(func)												\
		func<SCALAR_TYPE, 1>(MUU_MAKE_STRING_VIEW(SCALAR_TYPE));	\
		func<SCALAR_TYPE, 2>(MUU_MAKE_STRING_VIEW(SCALAR_TYPE));	\
		func<SCALAR_TYPE, 3>(MUU_MAKE_STRING_VIEW(SCALAR_TYPE));	\
		func<SCALAR_TYPE, 4>(MUU_MAKE_STRING_VIEW(SCALAR_TYPE));	\
		func<SCALAR_TYPE, 5>(MUU_MAKE_STRING_VIEW(SCALAR_TYPE));	\
		func<SCALAR_TYPE, 10>(MUU_MAKE_STRING_VIEW(SCALAR_TYPE))

#undef VECTOR_TEST_CASE
#define VECTOR_TEST_CASE(name)	TEST_CASE("vector<" MUU_MAKE_STRING(SCALAR_TYPE) ", *> - " name)

template <>
inline constexpr bool invoke_trait_tests<SCALAR_TYPE> = []() noexcept
{
	TEST_TYPE(trait_tests);
	return true;
}();

VECTOR_TEST_CASE("construction")
{
	TEST_TYPE(construction_tests);
}

VECTOR_TEST_CASE("accessors")
{
	TEST_TYPE(accessor_tests);
}

VECTOR_TEST_CASE("equality")
{
	TEST_TYPE(equality_tests);
}

VECTOR_TEST_CASE("zero")
{
	TEST_TYPE(zero_tests);
}

VECTOR_TEST_CASE("infinity_or_nan")
{
	TEST_TYPE(infinity_or_nan_tests);
}

VECTOR_TEST_CASE("length/distance")
{
	{
		INFO("vector<float, 2>"sv)

		const vector<float, 2> a{ 0, 10 };
		const vector<float, 2> b{ 15, 12 };
		const float distance = a.distance(b);
		CHECK(distance == approx( std::sqrt(15.0f * 15.0f + 2.0f * 2.0f) ));
	}

	{
		INFO("vector<float, 3>"sv)

		const vector<float, 3> a{ 0, 10, 3 };
		const vector<float, 3> b{ 15, 12, -4 };
		const float distance = a.distance(b);
		CHECK(distance == approx( std::sqrt(15.0f * 15.0f + 2.0f * 2.0f + 7.0f * 7.0f)));
	}

	{
		INFO("vector<float, 4>"sv)

		const vector<float, 4> a{ 9, 10, 3, 5 };
		const vector<float, 4> b{ 15, 12, -4, 1 };
		const float distance = a.distance(b);
		CHECK(distance == approx( std::sqrt(6.0f * 6.0f + 2.0f * 2.0f + 7.0f * 7.0f + 4.0f * 4.0f)) );
	}
}

VECTOR_TEST_CASE("dot")
{
	TEST_TYPE(dot_tests);
}

VECTOR_TEST_CASE("cross")
{
	if constexpr (is_floating_point<SCALAR_TYPE>) // cross works with ints, but the test isn't meaningful as written
	{
		cross_tests<SCALAR_TYPE, 3>(MUU_MAKE_STRING_VIEW(SCALAR_TYPE));
	}
}

VECTOR_TEST_CASE("addition")
{
	TEST_TYPE(addition_tests);
}

VECTOR_TEST_CASE("subtraction")
{
	TEST_TYPE(subtraction_tests);
}

VECTOR_TEST_CASE("multiplication")
{
	TEST_TYPE(multiplication_tests);
}

VECTOR_TEST_CASE("division")
{
	TEST_TYPE(division_tests);
}

VECTOR_TEST_CASE("modulo")
{
	TEST_TYPE(modulo_tests);
}

VECTOR_TEST_CASE("bitwise shifts")
{
	if constexpr (is_integral<SCALAR_TYPE>)
	{
		TEST_TYPE(bitwise_shift_tests);
	}
}

VECTOR_TEST_CASE("normalization")
{
	if constexpr (is_floating_point<SCALAR_TYPE>)
	{
		TEST_TYPE(normalization_tests);
	}
}

VECTOR_TEST_CASE("lerp")
{
	TEST_TYPE(lerp_tests);
}

VECTOR_TEST_CASE("min and max")
{
	TEST_TYPE(min_max_tests);
}

VECTOR_TEST_CASE("angle")
{
	TEST_TYPE(angle_tests);
}

#undef SCALAR_TYPE
