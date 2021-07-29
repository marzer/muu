// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/function_view.h"

namespace
{
	static int val = 1;
	static int func(int mult) noexcept
	{
		val *= mult;
		return val;
	}
	static_assert(std::is_same_v<decltype(function_view{ func }), function_view<int(int) noexcept>>);
}

TEST_CASE("function_view")
{
	// free functions
	{
		val = 1;
		function_view fv{ func };
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
		function_view<void() noexcept> fv{ lambda };
		fv();
	}
	CHECK(val == 2);

	// stateful lambdas
	{
		val				  = 1;
		int v			  = 1;
		const auto lambda = [&]() noexcept { val += v; };
		function_view<void() noexcept> fv{ lambda };
		fv();
	}
	CHECK(val == 2);
}
