// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

//#pragma once /* no! explicitly allowed to be re-included */

#include "oriented_bounding_box_test_templates.h"

#if !defined(SCALAR_TYPE) && MUU_INTELLISENSE
	#define SCALAR_TYPE float
#endif
#ifndef SCALAR_TYPE
	#error SCALAR_TYPE was not defined
#endif

#undef TEST_TYPE
#define TEST_TYPE(func)											\
		func<SCALAR_TYPE>(MUU_MAKE_STRING_VIEW(SCALAR_TYPE))

#undef OBB_TEST_CASE
#define OBB_TEST_CASE(name)	TEST_CASE("oriented_bounding_box<" MUU_MAKE_STRING(SCALAR_TYPE) "> - " name)

template <>
inline constexpr bool obb_invoke_trait_tests<SCALAR_TYPE> = []() noexcept
{
	TEST_TYPE(obb_trait_tests);
	return true;
}();

OBB_TEST_CASE("construction")
{
	TEST_TYPE(obb_construction_tests);
}

OBB_TEST_CASE("equality")
{
	TEST_TYPE(obb_equality_tests);
}

OBB_TEST_CASE("zero")
{
	TEST_TYPE(obb_zero_tests);
}

OBB_TEST_CASE("infinity_or_nan")
{
	TEST_TYPE(obb_infinity_or_nan_tests);
}

#undef SCALAR_TYPE
