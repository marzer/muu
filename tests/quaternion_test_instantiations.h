// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

//#pragma once /* no! explicitly allowed to be re-included */

#include "quaternion_test_templates.h"

#if !defined(SCALAR_TYPE) && MUU_INTELLISENSE
	#define SCALAR_TYPE float
#endif
#ifndef SCALAR_TYPE
	#error SCALAR_TYPE was not defined
#endif

#undef TEST_TYPE
#define TEST_TYPE(func)											\
		func<SCALAR_TYPE>(MUU_MAKE_STRING_VIEW(SCALAR_TYPE))

#undef QUATERNION_TEST_CASE
#define QUATERNION_TEST_CASE(name)	TEST_CASE("quaternion<" MUU_MAKE_STRING(SCALAR_TYPE) "> - " name)

template <>
inline constexpr bool invoke_trait_tests<SCALAR_TYPE> = []() noexcept
{
	TEST_TYPE(trait_tests);
	return true;
}();

QUATERNION_TEST_CASE("construction")
{
	TEST_TYPE(construction_tests);
}

QUATERNION_TEST_CASE("equality")
{
	TEST_TYPE(equality_tests);
}

QUATERNION_TEST_CASE("zero")
{
	TEST_TYPE(zero_tests);
}

QUATERNION_TEST_CASE("infinity_or_nan")
{
	TEST_TYPE(infinity_or_nan_tests);
}

QUATERNION_TEST_CASE("dot")
{
	TEST_TYPE(dot_tests);
}

QUATERNION_TEST_CASE("normalization")
{
	if constexpr (is_floating_point<SCALAR_TYPE>)
	{
		TEST_TYPE(normalization_tests);
	}
}

QUATERNION_TEST_CASE("euler")
{
	TEST_TYPE(euler_tests);
}

#if 0




QUATERNION_TEST_CASE("lerp")
{
	TEST_TYPE(lerp_tests);
}

QUATERNION_TEST_CASE("min and max")
{
	TEST_TYPE(min_max_tests);
}

QUATERNION_TEST_CASE("angle")
{
	TEST_TYPE(angle_tests);
}

QUATERNION_TEST_CASE("accumulator")
{
	TEST_TYPE(accumulator_tests);
}

#endif

#undef SCALAR_TYPE
