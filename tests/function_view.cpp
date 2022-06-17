// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/function_view.h"

#if MUU_WINDOWS
	#define FUNC_VIEW_TEST_CALLCONV_1 __cdecl
	#define FUNC_VIEW_TEST_CALLCONV_2 __vectorcall
#else
	#define FUNC_VIEW_TEST_CALLCONV_1
	#define FUNC_VIEW_TEST_CALLCONV_2
#endif

namespace
{
	static int val = 1;
	static int FUNC_VIEW_TEST_CALLCONV_1 func1(int mult) noexcept
	{
		val *= mult;
		return val;
	}
	static int FUNC_VIEW_TEST_CALLCONV_2 func2(int mult) noexcept
	{
		val *= mult;
		return val;
	}
	static_assert(std::is_same_v<decltype(function_view{ func1 }), function_view<int(int) noexcept>>);
}

TEST_CASE("function_view")
{
	// free functions
	{
		val = 1;
		function_view fv{ func1 };
		fv(2);
	}
	CHECK(val == 2);
	{
		val = 1;
		function_view fv{ func2 };
		fv(2);
	}
	CHECK(val == 2);

	// stateless lambdas (rvalue)
	{
		val = 1;
		function_view<void() noexcept> fv{ []() noexcept { val++; } };
		fv();
	}
	CHECK(val == 2);

	// stateless lambdas (lvalue)
	{
		val				  = 1;
		const auto lambda = []() noexcept { val++; };
		static_assert(is_stateless_lambda<decltype(lambda)>);
		function_view<void() noexcept> fv{ lambda };
		fv();
	}
	CHECK(val == 2);

	// stateful lambdas
	{
		val				  = 1;
		int v			  = 1;
		const auto lambda = [&]() noexcept { val += v; };
		static_assert(!is_stateless_lambda<decltype(lambda)>);
		function_view<void() noexcept> fv{ lambda };
		fv();
	}
	CHECK(val == 2);
}
