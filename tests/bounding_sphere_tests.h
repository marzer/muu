// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"
#include "batching.h"
#include "../include/muu/bounding_sphere.h"

namespace
{
	template <typename T, typename Func>
	static void bs_for_each(T&& s, Func&& func)
	{
		for (size_t i = 0; i < 3; i++)
			static_cast<Func&&>(func)(static_cast<T&&>(s).center[i], i);
		static_cast<Func&&>(func)(static_cast<T&&>(s).radius, 3_sz);
	}

	template <typename T, typename U, typename Func>
	static void bs_for_each(T&& s1, U&& s2, Func&& func)
	{
		for (size_t i = 0; i < 3; i++)
			static_cast<Func&&>(func)(static_cast<T&&>(s1).center[i], static_cast<U&&>(s2).center[i], i);
		static_cast<Func&&>(func)(static_cast<T&&>(s1).radius, static_cast<U&&>(s2).radius, 3_sz);
	}

	inline constexpr size_t bs_scalar_count = 4;

	template <typename T>
	struct blittable
	{
		vector<T, 3> center;
		T radius;
	};

	template <typename... T>
	using bounding_spheres = type_list<bounding_sphere<T>...>;
}

namespace muu
{
	template <typename T>
	inline constexpr bool allow_implicit_bit_cast<blittable<T>, bounding_sphere<T>> = true;
}

BATCHED_TEST_CASE("bounding_sphere constructors", bounding_spheres<ALL_FLOATS>)
{
	using bs   = TestType;
	using T	   = typename bs::scalar_type;
	using vec3 = vector<T, 3>;
	TEST_INFO("bounding_sphere<"sv << nameof<T> << ">"sv);

	static_assert(sizeof(bs) == (sizeof(vec3) + sizeof(T)));
	static_assert(std::is_standard_layout_v<bs>);
	static_assert(std::is_trivially_constructible_v<bs>);
	static_assert(std::is_trivially_copy_constructible_v<bs>);
	static_assert(std::is_trivially_copy_assignable_v<bs>);
	static_assert(std::is_trivially_move_constructible_v<bs>);
	static_assert(std::is_trivially_move_assignable_v<bs>);
	static_assert(std::is_trivially_destructible_v<bs>);
	static_assert(std::is_nothrow_constructible_v<bs, vec3, T>);
	static_assert(std::is_nothrow_constructible_v<bs, T, T, T, T>);
	static_assert(std::is_nothrow_constructible_v<bs, T>);

	BATCHED_SECTION("zero-initialization")
	{
		const auto s = bs{};
		bs_for_each(s, [](auto v, size_t) { CHECK(v == T{}); });
	}

	BATCHED_SECTION("vector + radius constructor")
	{
		const auto c = random_array<T, 3>();
		const auto r = random<T>();
		const auto s = bs{ vec3{ c }, r };
		CHECK(s.center[0] == c[0]);
		CHECK(s.center[1] == c[1]);
		CHECK(s.center[2] == c[2]);
		CHECK(s.radius == r);
	}

	BATCHED_SECTION("scalars + radius constructor")
	{
		const auto c = random_array<T, 3>();
		const auto r = random<T>();
		const auto s = bs{ c[0], c[1], c[2], r };
		CHECK(s.center[0] == c[0]);
		CHECK(s.center[1] == c[1]);
		CHECK(s.center[2] == c[2]);
		CHECK(s.radius == r);
	}

	BATCHED_SECTION("radius constructor")
	{
		const auto r = random<T>();
		const auto s = bs{ r };
		CHECK(s.center[0] == T{});
		CHECK(s.center[1] == T{});
		CHECK(s.center[2] == T{});
		CHECK(s.radius == r);
	}

	BATCHED_SECTION("copy constructor")
	{
		bs s1;
		bs_for_each(s1, [](auto& v1, size_t) { v1 = random<T>(); });
		bs s2{ s1 };
		bs_for_each(s1, s2, [](auto v1, auto v2, size_t) { CHECK(v1 == v2); });
	}

	BATCHED_SECTION("blitting constructor")
	{
		blittable<T> s1;
		bs_for_each(s1, [](auto& v1, size_t) { v1 = random<T>(); });
		bs s2{ s1 };
		bs_for_each(s1, s2, [](auto v1, auto v2, size_t) { CHECK(v1 == v2); });
	}

	BATCHED_SECTION("data()")
	{
		auto s				= bs{};
		const auto& s_const = s;
		CHECK(reinterpret_cast<uintptr_t>(s.data()) == reinterpret_cast<uintptr_t>(&s));
		CHECK(reinterpret_cast<uintptr_t>(s_const.data()) == reinterpret_cast<uintptr_t>(&s_const));
	}
}

BATCHED_TEST_CASE("bounding_sphere equality", bounding_spheres<ALL_FLOATS>)
{
	using bs = TestType;
	using T	 = typename bs::scalar_type;
	TEST_INFO("bounding_sphere<"sv << nameof<T> << ">"sv);

	bs s;
	bs_for_each(s, [](auto& v, size_t) noexcept { v = random<T>(); });

	BATCHED_SECTION("same type")
	{
		bs same{ s };
		CHECK_SYMMETRIC_EQUAL(s, same);
		if constexpr (is_floating_point<T>)
		{
			CHECK(bs::approx_equal(s, same));
			CHECK(muu::approx_equal(s, same));
		}

		bs different{ s };
		bs_for_each(different, [](auto& v, size_t) noexcept { v++; });
		CHECK_SYMMETRIC_INEQUAL(s, different);
		if constexpr (is_floating_point<T>)
		{
			CHECK(!bs::approx_equal(s, different));
			CHECK(!muu::approx_equal(s, different));
		}
	}

	if constexpr (!is_floating_point<T>)
	{
		BATCHED_SECTION("different type")
		{
			using other_scalar = std::conditional_t<std::is_same_v<T, long>, int, long>;
			using other		   = bounding_sphere<other_scalar>;

			other same;
			bs_for_each(same, s, [](auto& lhs, auto& rhs, size_t) noexcept { lhs = static_cast<other_scalar>(rhs); });
			CHECK_SYMMETRIC_EQUAL(s, same);

			other different;
			bs_for_each(different,
						s,
						[](auto& lhs, auto& rhs, size_t) noexcept
						{
							lhs = static_cast<other_scalar>(rhs);
							lhs++;
						});
			CHECK_SYMMETRIC_INEQUAL(s, different);
		}
	}
}

BATCHED_TEST_CASE("bounding_sphere zero()", bounding_spheres<ALL_FLOATS>)
{
	using bs = TestType;
	using T	 = typename bs::scalar_type;
	TEST_INFO("bounding_sphere<"sv << nameof<T> << ">"sv);

	BATCHED_SECTION("all zeroes")
	{
		bs s{};
		CHECK(s.zero());
	}

	BATCHED_SECTION("no zeroes")
	{
		bs s;
		bs_for_each(s, [](auto& v, size_t) noexcept { v = random<T>(1, 10); });
		CHECK(!s.zero());
	}

	BATCHED_SECTION("some zeroes")
	{
		bs s{};
		bs_for_each(s,
					[](auto& v, size_t i) noexcept
					{
						if ((i % 2u))
							v = random<T>(1, 10);
					});
		CHECK(!s.zero());
	}

	BATCHED_SECTION("one zero")
	{
		for (size_t i = 0; i < bs_scalar_count; i++)
		{
			bs s{};
			bs_for_each(s,
						[=](auto& v, size_t j) noexcept
						{
							if (i == j)
								v = random<T>(1, 10);
						});
			CHECK(!s.zero());
		}
	}
}

BATCHED_TEST_CASE("bounding_sphere infinity_or_nan()", bounding_spheres<ALL_FLOATS>)
{
	using bs = TestType;
	using T	 = typename bs::scalar_type;
	TEST_INFO("bounding_sphere<"sv << nameof<T> << ">"sv);

	BATCHED_SECTION("all finite")
	{
		bs s;
		RANDOM_ITERATIONS
		{
			bs_for_each(s, [](auto& v, size_t) noexcept { v = random<T>(1, 10); });
			CHECK(!s.infinity_or_nan());
			CHECK(!muu::infinity_or_nan(s));
		}
	}

	if constexpr (is_floating_point<T>)
	{
		BATCHED_SECTION("contains one NaN")
		{
			for (size_t i = 0; i < bs_scalar_count; i++)
			{
				RANDOM_ITERATIONS
				{
					bs s{};
					bs_for_each(s,
								[=](auto& v, size_t j) noexcept
								{
									if (i == j)
										v = make_nan<T>();
								});
					CHECK(s.infinity_or_nan());
					CHECK(muu::infinity_or_nan(s));
				}
			}
		}
	}

	if constexpr (is_floating_point<T>)
	{
		BATCHED_SECTION("contains one infinity")
		{
			for (size_t i = 0; i < bs_scalar_count; i++)
			{
				RANDOM_ITERATIONS
				{
					bs s{};
					bs_for_each(s,
								[=](auto& v, size_t j) noexcept
								{
									if (i == j)
										v = make_infinity<T>();
								});
					CHECK(s.infinity_or_nan());
					CHECK(muu::infinity_or_nan(s));
				}
			}
		}
	}
}

BATCHED_TEST_CASE("bounding_sphere degenerate()", bounding_spheres<ALL_FLOATS>)
{
	using bs   = TestType;
	using T	   = typename bs::scalar_type;
	using vec3 = vector<T, 3>;
	TEST_INFO("bounding_sphere<"sv << nameof<T> << ">"sv);

	BATCHED_SECTION("false")
	{
		bs s;

		RANDOM_ITERATIONS
		{
			s.center = vec3{ random_array<T, 3>(1, 10) };
			s.radius = random<T>(1, 10);
			CHECK(!s.degenerate());
			CHECK(!bs::degenerate(s));
			CHECK(!muu::degenerate(s));
		}
	}

	BATCHED_SECTION("true")
	{
		bs s;

		RANDOM_ITERATIONS
		{
			s.center = vec3{ random_array<T, 3>(1, 10) };
			s.radius = random<T>(-10, 0);
			CHECK(s.degenerate());
			CHECK(bs::degenerate(s));
			CHECK(muu::degenerate(s));
		}
	}
}
