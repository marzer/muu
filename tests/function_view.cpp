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
}

TEST_CASE("function_view")
{
	static_assert(std::is_trivially_destructible_v<function_view<int(int) noexcept>>);
	static_assert(std::is_trivially_copy_constructible_v<function_view<int(int) noexcept>>);
	static_assert(std::is_trivially_copy_assignable_v<function_view<int(int) noexcept>>);

	// free functions
	{
		val		= 1;
		auto fv = function_view{ func1 };
		static_assert(std::is_same_v<decltype(fv), function_view<int(int) noexcept>>);
		fv(2);
	}
	CHECK(val == 2);
	{
		val		= 1;
		auto fv = function_view{ func2 };
		static_assert(std::is_same_v<decltype(fv), function_view<int(int) noexcept>>);
		fv(2);
	}

	// check that we can use in conditional operators + rebind
	RANDOM_ITERATIONS
	{
		val		 = 1;
		auto fv1 = function_view{ func1 };
		auto fv	 = random<bool>() ? fv1 : func2;
		fv(2);
		CHECK(val == 2);
	}

	// stateless lambdas (rvalue)
	{
		val		= 1;
		auto fv = function_view{ []() noexcept { val++; } };
		static_assert(std::is_same_v<decltype(fv), function_view<void() noexcept>>);
		fv();
	}
	CHECK(val == 2);

	// stateless lambdas (lvalue)
	{
		val				  = 1;
		const auto lambda = []() noexcept { val++; };
		static_assert(is_stateless_lambda<decltype(lambda)>);
		auto fv = function_view{ lambda };
		static_assert(std::is_same_v<decltype(fv), function_view<void() noexcept>>);
		fv();
	}
	CHECK(val == 2);

	// stateful lambdas
	{
		val				  = 1;
		const auto lambda = [v = 1]() noexcept { val += v; };
		static_assert(!is_stateless_lambda<decltype(lambda)>);
		auto fv = function_view{ lambda };
		static_assert(std::is_same_v<decltype(fv), function_view<void() noexcept>>);
		fv();
	}
	CHECK(val == 2);
}
