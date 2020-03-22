#include "tests.h"
#include "../include/muu/scope_guard.h"

MUU_PUSH_WARNINGS
MUU_DISABLE_ALL_WARNINGS

namespace
{
	int val = 1;

	static void func() noexcept { val *= 2;  }
	static_assert(std::is_same_v<decltype(scope_guard{ func }), scope_guard<std::add_pointer_t<decltype(func)>>>);
}


TEST_CASE("scope_guard")
{
	// free functions
	{
		val = 1;
		scope_guard sg1{ func };
		scope_guard sg2{ func };
		sg2.suppress();
	}
	CHECK(val == 2);

	// stateless lambdas (rvalue)
	{
		val = 1;
		scope_guard sg1{ []()noexcept { func(); } };
		scope_guard sg2{ []()noexcept { func(); } };
		sg2.suppress();
	}
	CHECK(val == 2);

	// stateless lambdas (lvalue)
	{
		val = 1;
		auto lambda = []()noexcept { func(); };
		scope_guard sg1{ lambda };
		scope_guard sg2{ lambda };
		sg2.suppress();
	}
	CHECK(val == 2);

	// stateful lambdas (rvalue)
	{
		int v = 1;
		{
			scope_guard sg1{ [&]()noexcept { v++; } };
			scope_guard sg2{ [&]()noexcept { v += 10; } };
			sg2.suppress();
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
			sg2.suppress();
		}
		CHECK(v == 2);
	}
}

MUU_POP_WARNINGS
