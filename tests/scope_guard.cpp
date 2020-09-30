// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/scope_guard.h"

namespace
{
	static int val = 1;
	static void func() noexcept { val *= 2;  }
	static_assert(std::is_same_v<decltype(scope_guard{ func }), scope_guard<std::add_pointer_t<decltype(func)>>>);

	template <typename T>
	struct static_checks;

	template <typename T>
	struct static_checks<scope_guard<T>>
	{
		static_assert(!std::is_empty_v<T> || sizeof(scope_guard<T>) == sizeof(T)); // ebco check
		static constexpr bool ok = true;
	};
}

TEST_CASE("scope_guard")
{
	// free functions
	{
		val = 1;
		scope_guard sg1{ func };
		scope_guard sg2{ func };
		sg2.dismiss();

		static_assert(static_checks<decltype(sg1)>::ok);
	}
	CHECK(val == 2);

	// stateless lambdas (rvalue)
	{
		val = 1;
		scope_guard sg1{ []()noexcept { func(); } };
		scope_guard sg2{ []()noexcept { func(); } };
		sg2.dismiss();

		static_assert(static_checks<decltype(sg1)>::ok);
		static_assert(static_checks<decltype(sg2)>::ok);
	}
	CHECK(val == 2);

	// stateless lambdas (lvalue)
	{
		val = 1;
		auto lambda = []()noexcept { func(); };
		scope_guard sg1{ lambda };
		scope_guard sg2{ lambda };
		sg2.dismiss();

		static_assert(static_checks<decltype(sg1)>::ok);
	}
	CHECK(val == 2);

	// stateful lambdas (rvalue)
	{
		int v = 1;
		{
			scope_guard sg1{ [&]()noexcept { v++; } };
			scope_guard sg2{ [&]()noexcept { v += 10; } };
			sg2.dismiss();

			static_assert(static_checks<decltype(sg1)>::ok);
			static_assert(static_checks<decltype(sg2)>::ok);
		}
		CHECK(v == 2);
	}

	// stateful lambdas (lvalue)
	{
		int v = 1;
		{
			auto lambda1 = [&]()noexcept { v++; };
			scope_guard sg1{ lambda1 };
			auto lambda2 = [&]()noexcept { v += 10; };
			scope_guard sg2{ lambda2 };
			sg2.dismiss();

			static_assert(static_checks<decltype(sg1)>::ok);
			static_assert(static_checks<decltype(sg2)>::ok);
		}
		CHECK(v == 2);
	}
}
