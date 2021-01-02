// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
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
inline constexpr bool quat_invoke_trait_tests<SCALAR_TYPE> = []() noexcept
{
	TEST_TYPE(quat_trait_tests);
	return true;
}();

QUATERNION_TEST_CASE("construction")
{
	TEST_TYPE(quat_construction_tests);
}

QUATERNION_TEST_CASE("equality")
{
	TEST_TYPE(quat_equality_tests);
}

QUATERNION_TEST_CASE("zero")
{
	TEST_TYPE(quat_zero_tests);
}

QUATERNION_TEST_CASE("infinity_or_nan")
{
	TEST_TYPE(quat_infinity_or_nan_tests);
}

QUATERNION_TEST_CASE("dot")
{
	TEST_TYPE(quat_dot_tests);
}

QUATERNION_TEST_CASE("normalization")
{
	TEST_TYPE(quat_normalization_tests);
}

QUATERNION_TEST_CASE("euler")
{
	TEST_TYPE(quat_euler_tests);
}

QUATERNION_TEST_CASE("conjugate")
{
	TEST_TYPE(quat_conjugate_tests);
}

QUATERNION_TEST_CASE("slerp")
{
	TEST_TYPE(quat_slerp_tests);
}

QUATERNION_TEST_CASE("multiplication")
{
	TEST_TYPE(quat_multiplication_tests);
}

#undef SCALAR_TYPE
