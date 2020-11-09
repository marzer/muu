// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

//#pragma once /* no! explicitly allowed to be re-included */

#include "matrix_test_templates.h"

#if !defined(SCALAR_TYPE) && MUU_INTELLISENSE
	#define SCALAR_TYPE float
#endif
#ifndef SCALAR_TYPE
	#error SCALAR_TYPE was not defined
#endif

#undef TEST_TYPE_R
#define TEST_TYPE_R(func, R)										\
		func<SCALAR_TYPE, R, 1>(MUU_MAKE_STRING_VIEW(SCALAR_TYPE));	\
		func<SCALAR_TYPE, R, 2>(MUU_MAKE_STRING_VIEW(SCALAR_TYPE));	\
		func<SCALAR_TYPE, R, 3>(MUU_MAKE_STRING_VIEW(SCALAR_TYPE));	\
		func<SCALAR_TYPE, R, 4>(MUU_MAKE_STRING_VIEW(SCALAR_TYPE));	\
		func<SCALAR_TYPE, R, 5>(MUU_MAKE_STRING_VIEW(SCALAR_TYPE))

#undef TEST_TYPE
#define TEST_TYPE(func)			\
		TEST_TYPE_R(func, 1);	\
		TEST_TYPE_R(func, 2);	\
		TEST_TYPE_R(func, 3);	\
		TEST_TYPE_R(func, 4);	\
		TEST_TYPE_R(func, 5)

#undef MATRIX_TEST_CASE
#define MATRIX_TEST_CASE(name)	TEST_CASE("matrix<" MUU_MAKE_STRING(SCALAR_TYPE) ", *, *> - " name)

template <>
inline constexpr bool matrix_invoke_trait_tests<SCALAR_TYPE> = []() noexcept
{
	TEST_TYPE(matrix_trait_tests);
	return true;
}();

MATRIX_TEST_CASE("construction")
{
	TEST_TYPE(matrix_construction_tests);
}

MATRIX_TEST_CASE("accessors")
{
	TEST_TYPE(matrix_accessor_tests);
}

MATRIX_TEST_CASE("equality")
{
	TEST_TYPE(matrix_equality_tests);
}

#undef SCALAR_TYPE
