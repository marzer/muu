// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"
#include "batching.h"
#include "../include/muu/ray.h"

namespace
{
	template <typename T, typename Func>
	static void ray_for_each(T&& r, Func&& func)
	{
		static_cast<Func&&>(func)(r.origin.x, 0_sz);
		static_cast<Func&&>(func)(r.origin.y, 1_sz);
		static_cast<Func&&>(func)(r.origin.z, 2_sz);
		static_cast<Func&&>(func)(r.direction.x, 3_sz);
		static_cast<Func&&>(func)(r.direction.y, 4_sz);
		static_cast<Func&&>(func)(r.direction.z, 5_sz);
	}

	template <typename T, typename U, typename Func>
	static void ray_for_each(T&& ray1, U&& ray2, Func&& func)
	{
		static_cast<Func&&>(func)(ray1.origin.x, ray2.origin.x, 0_sz);
		static_cast<Func&&>(func)(ray1.origin.y, ray2.origin.y, 1_sz);
		static_cast<Func&&>(func)(ray1.origin.z, ray2.origin.z, 2_sz);
		static_cast<Func&&>(func)(ray1.direction.x, ray2.direction.x, 3_sz);
		static_cast<Func&&>(func)(ray1.direction.y, ray2.direction.y, 4_sz);
		static_cast<Func&&>(func)(ray1.direction.z, ray2.direction.z, 5_sz);
	}

	inline constexpr size_t tri_scalar_count = 9;

	template <typename T>
	struct blittable
	{
		vector<T, 3> origin;
		vector<T, 3> direction;
	};

	template <typename... T>
	using rays = type_list<ray<T>...>;

	using tested_rays = rays<NON_FP16_FLOATS>; // waste of time checking fp16
}

namespace muu
{
	template <typename T>
	inline constexpr bool allow_implicit_bit_cast<blittable<T>, ray<T>> = true;
}

BATCHED_TEST_CASE("ray constructors", tested_rays)
{
	using ray  = TestType;
	using T	   = typename ray::scalar_type;
	using vec3 = vector<T, 3>;
	TEST_INFO("ray<"sv << nameof<T> << ">"sv);

	static_assert(sizeof(ray) == sizeof(vec3) * 2);
	static_assert(std::is_standard_layout_v<ray>);
	static_assert(std::is_trivially_constructible_v<ray>);
	static_assert(std::is_trivially_copy_constructible_v<ray>);
	static_assert(std::is_trivially_copy_assignable_v<ray>);
	static_assert(std::is_trivially_move_constructible_v<ray>);
	static_assert(std::is_trivially_move_assignable_v<ray>);
	static_assert(std::is_trivially_destructible_v<ray>);
	static_assert(std::is_nothrow_constructible_v<ray, vec3, vec3>);
	static_assert(std::is_nothrow_constructible_v<ray, T, T, T, T, T, T>);

	BATCHED_SECTION("zero-initialization")
	{
		const auto r = ray{};
		CHECK(r.origin == vec3{});
		CHECK(r.direction == vec3{});
	}

	BATCHED_SECTION("point-dir constructor")
	{
		const auto p0  = vec3{ random_array<T, 3>() };
		const auto dir = vec3{ random_array<T, 3>() };
		const auto r   = ray{ p0, dir };
		TEST_INFO("ray: "sv << r);
		CHECK(r.origin == p0);
		CHECK(r.direction == dir);
	}

	BATCHED_SECTION("scalar constructor")
	{
		const auto p0  = vec3{ random_array<T, 3>() };
		const auto dir = vec3{ random_array<T, 3>() };
		const auto r   = ray{ p0.x, p0.y, p0.z, dir.x, dir.y, dir.z };
		TEST_INFO("ray: "sv << r);
		CHECK(r.origin == p0);
		CHECK(r.direction == dir);
	}

	BATCHED_SECTION("copy constructor")
	{
		ray r1;
		ray_for_each(r1, [](auto& s1, size_t) noexcept { s1 = random<T>(); });
		ray r2{ r1 };
		ray_for_each(r1, r2, [](auto s1, auto s2, size_t) { CHECK(s1 == s2); });
	}

	BATCHED_SECTION("blitting constructor")
	{
		blittable<T> r1{};
		ray_for_each(r1, [](auto& s1, size_t) { s1 = random<T>(); });
		ray r2{ r1 };
		ray_for_each(r1, r2, [](auto s1, auto s2, size_t) { CHECK(s1 == s2); });
	}
}

BATCHED_TEST_CASE("ray-plane hit tests", tested_rays)
{
	using ray	= TestType;
	using T		= typename ray::scalar_type;
	using vec3	= vector<T, 3>;
	using plane = muu::plane<T>;
	TEST_INFO("ray<"sv << nameof<T> << ">"sv);

	const auto start_point = vec3{ T{}, T{ 5 }, T{} };

	// test rays that should hit the plane
	for (int x = -10; x <= 10; x++)
	{
		for (int y = -5; y < 5; y++) // start_point.y == 5.0
		{
			for (int z = -10; z <= 10; z++)
			{
				const auto end_point = vec3{ static_cast<T>(x), static_cast<T>(y), static_cast<T>(z) };
				const auto r		 = ray{ start_point, vec3::direction(start_point, end_point) };
				TEST_INFO("ray: "sv << r);

				const auto hit = r.hits(plane::constants::xz);
				CHECK(hit.has_value());
				CHECK(!infinity_or_nan(*hit));
				CHECK((*hit) > 0.0f);
				CHECK(plane::constants::xz.contains(r.origin + r.direction * (*hit)));
			}
		}
	}

	// test rays that should NOT hit the plane
	for (int x = -10; x <= 10; x++)
	{
		for (int y = 5; y <= 15; y++) // start_point.y == 5.0
		{
			for (int z = -10; z <= 10; z++)
			{
				const auto end_point = vec3{ static_cast<T>(x), static_cast<T>(y), static_cast<T>(z) };
				const auto r		 = ray{ start_point, vec3::direction(start_point, end_point) };
				TEST_INFO("ray: "sv << r);

				const auto hit = r.hits(plane::constants::xz);
				CHECK(!hit.has_value());
			}
		}
	}
}
