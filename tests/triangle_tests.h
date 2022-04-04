// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"
#include "batching.h"
#include "../include/muu/triangle.h"

namespace
{
	template <typename T, typename Func>
	static void triangle_for_each(T&& tri, Func&& func)
	{
		static_cast<Func&&>(func)(tri.points[0].x, 0_sz);
		static_cast<Func&&>(func)(tri.points[0].y, 1_sz);
		static_cast<Func&&>(func)(tri.points[0].z, 2_sz);
		static_cast<Func&&>(func)(tri.points[1].x, 3_sz);
		static_cast<Func&&>(func)(tri.points[1].y, 4_sz);
		static_cast<Func&&>(func)(tri.points[1].z, 5_sz);
		static_cast<Func&&>(func)(tri.points[2].x, 6_sz);
		static_cast<Func&&>(func)(tri.points[2].y, 7_sz);
		static_cast<Func&&>(func)(tri.points[2].z, 8_sz);
	}

	template <typename T, typename U, typename Func>
	static void triangle_for_each(T&& tri1, U&& tri2, Func&& func)
	{
		static_cast<Func&&>(func)(tri1.points[0].x, tri2.points[0].x, 0_sz);
		static_cast<Func&&>(func)(tri1.points[0].y, tri2.points[0].y, 1_sz);
		static_cast<Func&&>(func)(tri1.points[0].z, tri2.points[0].z, 2_sz);
		static_cast<Func&&>(func)(tri1.points[1].x, tri2.points[1].x, 3_sz);
		static_cast<Func&&>(func)(tri1.points[1].y, tri2.points[1].y, 4_sz);
		static_cast<Func&&>(func)(tri1.points[1].z, tri2.points[1].z, 5_sz);
		static_cast<Func&&>(func)(tri1.points[2].x, tri2.points[2].x, 6_sz);
		static_cast<Func&&>(func)(tri1.points[2].y, tri2.points[2].y, 7_sz);
		static_cast<Func&&>(func)(tri1.points[2].z, tri2.points[2].z, 8_sz);
	}

	template <typename T>
	struct blittable
	{
		vector<T, 3> points[3];
	};

	template <typename... T>
	using triangles = type_list<triangle<T>...>;

	using tested_triangles = triangles<NON_FP16_FLOATS>; // waste of time checking fp16
}

namespace muu
{
	template <typename T>
	inline constexpr bool allow_implicit_bit_cast<blittable<T>, triangle<T>> = true;
}

BATCHED_TEST_CASE("triangle constructors", tested_triangles)
{
	using triangle = TestType;
	using T		   = typename triangle::scalar_type;
	using vec3	   = vector<T, 3>;
	TEST_INFO("triangle<"sv << nameof<T> << ">"sv);

	static_assert(sizeof(triangle) == sizeof(vec3) * 3);
	static_assert(std::is_standard_layout_v<triangle>);
	static_assert(std::is_trivially_constructible_v<triangle>);
	static_assert(std::is_trivially_copy_constructible_v<triangle>);
	static_assert(std::is_trivially_copy_assignable_v<triangle>);
	static_assert(std::is_trivially_move_constructible_v<triangle>);
	static_assert(std::is_trivially_move_assignable_v<triangle>);
	static_assert(std::is_trivially_destructible_v<triangle>);
	static_assert(std::is_nothrow_constructible_v<triangle, vec3, vec3, vec3>);
	static_assert(std::is_nothrow_constructible_v<triangle, const vec3(&)[3]>);
	static_assert(std::is_nothrow_constructible_v<triangle, T, T, T, T, T, T, T, T, T>);

	BATCHED_SECTION("zero-initialization")
	{
		const auto tri = triangle{};
		for (auto& p : tri)
			CHECK(p == vec3{});
	}

	BATCHED_SECTION("three points constructor")
	{
		const auto p0  = vec3{ random_array<T, 3>() };
		const auto p1  = vec3{ random_array<T, 3>() };
		const auto p2  = vec3{ random_array<T, 3>() };
		const auto tri = triangle{ p0, p1, p2 };
		TEST_INFO("tri: "sv << tri);
		CHECK(tri[0] == p0);
		CHECK(tri[1] == p1);
		CHECK(tri[2] == p2);
	}

	BATCHED_SECTION("array constructor")
	{
		const vec3 points[3] = { vec3{ random_array<T, 3>() },
								 vec3{ random_array<T, 3>() },
								 vec3{ random_array<T, 3>() } };
		const auto tri		 = triangle{ points };
		TEST_INFO("tri: "sv << tri);
		CHECK(tri[0] == points[0]);
		CHECK(tri[1] == points[1]);
		CHECK(tri[2] == points[2]);
	}

	BATCHED_SECTION("copy constructor")
	{
		triangle tri1;
		triangle_for_each(tri1, [](auto& s1, size_t) noexcept { s1 = random<T>(); });
		triangle tri2{ tri1 };
		triangle_for_each(tri1, tri2, [](auto s1, auto s2, size_t) { CHECK(s1 == s2); });
	}

	BATCHED_SECTION("blitting constructor")
	{
		blittable<T> tri1{};
		triangle_for_each(tri1, [](auto& s1, size_t) { s1 = random<T>(); });
		triangle tri2{ tri1 };
		triangle_for_each(tri1, tri2, [](auto s1, auto s2, size_t) { CHECK(s1 == s2); });
	}

	BATCHED_SECTION("data()")
	{
		auto tri			  = triangle{};
		const auto& tri_const = tri;
		CHECK(reinterpret_cast<uintptr_t>(tri.data()) == reinterpret_cast<uintptr_t>(&tri));
		CHECK(reinterpret_cast<uintptr_t>(tri_const.data()) == reinterpret_cast<uintptr_t>(&tri_const));
	}
}
