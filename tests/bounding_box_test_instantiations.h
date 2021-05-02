// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

//#pragma once /* no! explicitly allowed to be re-included */

#include "bounding_box_test_templates.h"

#if !defined(SCALAR_TYPE) && MUU_INTELLISENSE
	#define SCALAR_TYPE float
#endif
#ifndef SCALAR_TYPE
	#error SCALAR_TYPE was not defined
#endif

#undef TEST_TYPE
#define TEST_TYPE(func)											\
		func<SCALAR_TYPE>(MUU_MAKE_STRING_VIEW(SCALAR_TYPE))

#undef AABB_TEST_CASE
#define AABB_TEST_CASE(name)	TEST_CASE("bounding_box<" MUU_MAKE_STRING(SCALAR_TYPE) "> - " name)

template <>
inline constexpr bool aabb_invoke_trait_tests<SCALAR_TYPE> = []() noexcept
{
	TEST_TYPE(aabb_trait_tests);
	return true;
}();

AABB_TEST_CASE("construction")
{
	TEST_TYPE(aabb_construction_tests);
}

AABB_TEST_CASE("equality")
{
	TEST_TYPE(aabb_equality_tests);
}

AABB_TEST_CASE("zero")
{
	TEST_TYPE(aabb_zero_tests);
}

AABB_TEST_CASE("infinity_or_nan")
{
	TEST_TYPE(aabb_infinity_or_nan_tests);
}

AABB_TEST_CASE("intersections")
{
	TEST_TYPE(aabb_intersects_tests);
}

#undef SCALAR_TYPE
