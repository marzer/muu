// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "plane_test_templates.h"

#if !defined(SCALAR_TYPE) && MUU_INTELLISENSE
	#define SCALAR_TYPE float
#endif
#ifndef SCALAR_TYPE
	#error SCALAR_TYPE was not defined
#endif

#undef TEST_TYPE
#define TEST_TYPE(func) func<SCALAR_TYPE>(MUU_MAKE_STRING_VIEW(SCALAR_TYPE))

#undef PLANE_TEST_CASE
#define PLANE_TEST_CASE(name) TEST_CASE("plane<" MUU_MAKE_STRING(SCALAR_TYPE) "> - " name)

template <>
inline constexpr bool plane_invoke_trait_tests<SCALAR_TYPE> = []() noexcept
{
	TEST_TYPE(plane_trait_tests);
	return true;
}();

PLANE_TEST_CASE("construction")
{
	TEST_TYPE(plane_construction_tests);
}

PLANE_TEST_CASE("equality")
{
	TEST_TYPE(plane_equality_tests);
}

PLANE_TEST_CASE("zero")
{
	TEST_TYPE(plane_zero_tests);
}

PLANE_TEST_CASE("infinity_or_nan")
{
	TEST_TYPE(plane_infinity_or_nan_tests);
}

#if 0

PLANE_TEST_CASE("intersections")
{
	TEST_TYPE(plane_intersects_tests);
}
#endif

#undef SCALAR_TYPE
