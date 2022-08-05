// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"
#include "batching.h"
#include "../include/muu/plane.h"

namespace
{
	template <typename T, typename Func>
	static void plane_for_each(T&& p, Func&& func)
	{
		static_cast<Func&&>(func)(static_cast<T&&>(p).normal.x, 0_sz);
		static_cast<Func&&>(func)(static_cast<T&&>(p).normal.y, 1_sz);
		static_cast<Func&&>(func)(static_cast<T&&>(p).normal.z, 2_sz);
		static_cast<Func&&>(func)(static_cast<T&&>(p).d, 3_sz);
	}

	template <typename T, typename U, typename Func>
	static void plane_for_each(T&& p1, U&& p2, Func&& func)
	{
		static_cast<Func&&>(func)(static_cast<T&&>(p1).normal.x, static_cast<U&&>(p2).normal.x, 0_sz);
		static_cast<Func&&>(func)(static_cast<T&&>(p1).normal.y, static_cast<U&&>(p2).normal.y, 1_sz);
		static_cast<Func&&>(func)(static_cast<T&&>(p1).normal.z, static_cast<U&&>(p2).normal.z, 2_sz);
		static_cast<Func&&>(func)(static_cast<T&&>(p1).d, static_cast<U&&>(p2).d, 3_sz);
	}

	inline constexpr size_t plane_scalar_count = 4;

	template <typename T>
	struct blittable
	{
		vector<T, 3> normal;
		T d;
	};

	template <typename... T>
	using planes = type_list<plane<T>...>;

	using tested_planes = planes<NON_FP16_FLOATS>; // waste of time checking fp16
}

namespace muu
{
	template <typename T>
	inline constexpr bool allow_implicit_bit_cast<blittable<T>, plane<T>> = true;
}

BATCHED_TEST_CASE("plane constructors", tested_planes)
{
	using plane = TestType;
	using T		= typename plane::scalar_type;
	using vec3	= vector<T, 3>;
	TEST_INFO("plane<"sv << nameof<T> << ">"sv);

	static_assert(sizeof(plane) == sizeof(T) * 4);
	static_assert(std::is_standard_layout_v<plane>);
	static_assert(std::is_trivially_constructible_v<plane>);
	static_assert(std::is_trivially_copy_constructible_v<plane>);
	static_assert(std::is_trivially_copy_assignable_v<plane>);
	static_assert(std::is_trivially_move_constructible_v<plane>);
	static_assert(std::is_trivially_move_assignable_v<plane>);
	static_assert(std::is_trivially_destructible_v<plane>);
	static_assert(std::is_nothrow_constructible_v<plane, vec3, vec3, vec3>);
	static_assert(std::is_nothrow_constructible_v<plane, vec3, vec3>);
	static_assert(std::is_nothrow_constructible_v<plane, vec3, T>);

	BATCHED_SECTION("zero-initialization")
	{
		const auto p = plane{};
		plane_for_each(p, [](auto s, size_t) { CHECK(s == T{}); });
	}

	BATCHED_SECTION("n + d constructor")
	{
		const auto n = vec3::normalize(vec3{ random_array<T, 3>() });
		const auto d = random<T>();
		const auto p = plane{ n, d };
		CHECK(p.normal[0] == n[0]);
		CHECK(p.normal[1] == n[1]);
		CHECK(p.normal[2] == n[2]);
		CHECK(p.d == d);
	}

	BATCHED_SECTION("position + direction constructor")
	{
		const auto pos = vec3{ random_array<T, 3>() };
		const auto dir = vec3::normalize(vec3{ random_array<T, 3>() });
		const auto p   = plane{ pos, dir };
		CHECK(p.normal[0] == dir[0]);
		CHECK(p.normal[1] == dir[1]);
		CHECK(p.normal[2] == dir[2]);
		CHECK(p.contains(pos));
	}

	BATCHED_SECTION("three points constructor")
	{
		const auto p1 = vec3{ random_array<T, 3>() };
		const auto p2 = vec3{ random_array<T, 3>() };
		const auto p3 = vec3{ random_array<T, 3>() };
		const auto p  = plane{ p1, p2, p3 };
		TEST_INFO("p: "sv << p);
		CHECK(p.contains(p1));
		CHECK(p.contains(p2));
		CHECK(p.contains(p3));
	}

	BATCHED_SECTION("copy constructor")
	{
		plane p1;
		plane_for_each(p1, [](auto& s1, size_t) { s1 = random<T>(); });
		plane p2{ p1 };
		plane_for_each(p1, p2, [](auto s1, auto s2, size_t) { CHECK(s1 == s2); });
	}

	BATCHED_SECTION("blitting constructor")
	{
		blittable<T> p1;
		plane_for_each(p1, [](auto& s1, size_t) { s1 = random<T>(); });
		plane p2{ p1 };
		plane_for_each(p1, p2, [](auto s1, auto s2, size_t) { CHECK(s1 == s2); });
	}

	BATCHED_SECTION("data()")
	{
		auto p				= plane{};
		const auto& p_const = p;
		CHECK(reinterpret_cast<uintptr_t>(p.data()) == reinterpret_cast<uintptr_t>(&p));
		CHECK(reinterpret_cast<uintptr_t>(p_const.data()) == reinterpret_cast<uintptr_t>(&p_const));
	}
}

BATCHED_TEST_CASE("plane equality", tested_planes)
{
	using plane = TestType;
	using T		= typename plane::scalar_type;
	TEST_INFO("plane<"sv << nameof<T> << ">"sv);

	plane p;
	plane_for_each(p, [](auto& s, size_t) noexcept { s = random<T>(); });

	BATCHED_SECTION("same type")
	{
		plane same{ p };
		CHECK_SYMMETRIC_EQUAL(p, same);
		if constexpr (is_floating_point<T>)
		{
			CHECK(plane::approx_equal(p, same));
			CHECK(muu::approx_equal(p, same));
		}

		plane different{ p };
		plane_for_each(different, [](auto& s, size_t) noexcept { s++; });
		CHECK_SYMMETRIC_INEQUAL(p, different);
		if constexpr (is_floating_point<T>)
		{
			CHECK_FALSE(plane::approx_equal(p, different));
			CHECK_FALSE(muu::approx_equal(p, different));
		}
	}

	if constexpr (!is_floating_point<T>)
	{
		BATCHED_SECTION("different type")
		{
			using other_scalar = std::conditional_t<std::is_same_v<T, long>, int, long>;
			using other		   = muu::plane<other_scalar>;

			other same;
			plane_for_each(same,
						   p,
						   [](auto& lhs, auto& rhs, size_t) noexcept { lhs = static_cast<other_scalar>(rhs); });
			CHECK_SYMMETRIC_EQUAL(p, same);

			other different;
			plane_for_each(different,
						   p,
						   [](auto& lhs, auto& rhs, size_t) noexcept
						   {
							   lhs = static_cast<other_scalar>(rhs);
							   lhs++;
						   });
			CHECK_SYMMETRIC_INEQUAL(p, different);
		}
	}
}

BATCHED_TEST_CASE("plane zero", tested_planes)
{
	using plane = TestType;
	using T		= typename plane::scalar_type;
	TEST_INFO("plane<"sv << nameof<T> << ">"sv);

	BATCHED_SECTION("all zeroes")
	{
		plane p{};
		CHECK(p.zero());
	}

	BATCHED_SECTION("no zeroes")
	{
		plane p;
		plane_for_each(p, [](auto& s, size_t) noexcept { s = random<T>(); });
		CHECK_FALSE(p.zero());
	}

	BATCHED_SECTION("some zeroes")
	{
		plane p{};
		plane_for_each(p,
					   [](auto& s, size_t i) noexcept
					   {
						   if ((i % 2u))
							   s = random<T>();
					   });
		CHECK_FALSE(p.zero());
	}

	BATCHED_SECTION("one zero")
	{
		for (size_t i = 0; i < plane_scalar_count; i++)
		{
			plane p{};
			plane_for_each(p,
						   [=](auto& s, size_t j) noexcept
						   {
							   if (i == j)
								   s = random<T>();
						   });
			CHECK_FALSE(p.zero());
		}
	}
}

BATCHED_TEST_CASE("plane infinity_or_nan", tested_planes)
{
	using plane = TestType;
	using T		= typename plane::scalar_type;
	TEST_INFO("plane<"sv << nameof<T> << ">"sv);

	BATCHED_SECTION("all finite")
	{
		plane p;
		plane_for_each(p, [](auto& s, size_t) noexcept { s = random<T>(); });
		CHECK_FALSE(p.infinity_or_nan());
		CHECK_FALSE(muu::infinity_or_nan(p));
	}

	if constexpr (is_floating_point<T>)
	{
		BATCHED_SECTION("contains one NaN")
		{
			for (size_t i = 0; i < plane_scalar_count; i++)
			{
				plane p{};
				plane_for_each(p,
							   [=](auto& s, size_t j) noexcept
							   {
								   if (i == j)
									   s = make_nan<T>();
							   });
				CHECK(p.infinity_or_nan());
				CHECK(muu::infinity_or_nan(p));
			}
		}
	}

	if constexpr (is_floating_point<T>)
	{
		BATCHED_SECTION("contains one infinity")
		{
			for (size_t i = 0; i < plane_scalar_count; i++)
			{
				plane p{};
				plane_for_each(p,
							   [=](auto& s, size_t j) noexcept
							   {
								   if (i == j)
									   s = make_infinity<T>();
							   });
				CHECK(p.infinity_or_nan());
				CHECK(muu::infinity_or_nan(p));
			}
		}
	}
}

BATCHED_TEST_CASE("plane distance and projection", tested_planes)
{
	using plane = TestType;
	using T		= typename plane::scalar_type;
	TEST_INFO("plane<"sv << nameof<T> << ">"sv);

	BATCHED_SECTION("origin()")
	{
		RANDOM_ITERATIONS
		{
			const auto point = vector<T, 3>{ random<T>(-5, 5), random<T>(-5, 5), random<T>(-5, 5) };
			T len;
			const auto dir = vector<T, 3>::normalize(point, len);
			SKIP_INF_NAN(dir);

			const plane p{ point, dir };
			CHECK_APPROX_EQUAL(len, muu::abs(p.d));

			const auto origin = p.origin();
			for (size_t i = 0; i < 3; i++)
				CHECK_APPROX_EQUAL(origin[i], point[i]);
		}
	}
}
