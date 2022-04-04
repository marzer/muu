// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"
#include "batching.h"
#include "../include/muu/line_segment.h"

namespace
{
	template <typename T, typename Func>
	static void line_segment_for_each(T&& seg, Func&& func)
	{
		static_cast<Func&&>(func)(seg.points[0].x, 0_sz);
		static_cast<Func&&>(func)(seg.points[0].y, 1_sz);
		static_cast<Func&&>(func)(seg.points[0].z, 2_sz);
		static_cast<Func&&>(func)(seg.points[1].x, 3_sz);
		static_cast<Func&&>(func)(seg.points[1].y, 4_sz);
		static_cast<Func&&>(func)(seg.points[1].z, 5_sz);
	}

	template <typename T, typename U, typename Func>
	static void line_segment_for_each(T&& seg1, U&& seg2, Func&& func)
	{
		static_cast<Func&&>(func)(seg1.points[0].x, seg2.points[0].x, 0_sz);
		static_cast<Func&&>(func)(seg1.points[0].y, seg2.points[0].y, 1_sz);
		static_cast<Func&&>(func)(seg1.points[0].z, seg2.points[0].z, 2_sz);
		static_cast<Func&&>(func)(seg1.points[1].x, seg2.points[1].x, 3_sz);
		static_cast<Func&&>(func)(seg1.points[1].y, seg2.points[1].y, 4_sz);
		static_cast<Func&&>(func)(seg1.points[1].z, seg2.points[1].z, 5_sz);
	}

	template <typename T>
	struct blittable
	{
		vector<T, 3> points[2];
	};

	template <typename... T>
	using line_segments = type_list<line_segment<T>...>;

	using tested_line_segments = line_segments<NON_FP16_FLOATS>; // waste of time checking fp16
}

namespace muu
{
	template <typename T>
	inline constexpr bool allow_implicit_bit_cast<blittable<T>, line_segment<T>> = true;
}

BATCHED_TEST_CASE("line_segment constructors", tested_line_segments)
{
	using line_segment = TestType;
	using T			   = typename line_segment::scalar_type;
	using vec3		   = vector<T, 3>;
	TEST_INFO("line_segment<"sv << nameof<T> << ">"sv);

	static_assert(sizeof(line_segment) == sizeof(vec3) * 2);
	static_assert(std::is_standard_layout_v<line_segment>);
	static_assert(std::is_trivially_constructible_v<line_segment>);
	static_assert(std::is_trivially_copy_constructible_v<line_segment>);
	static_assert(std::is_trivially_copy_assignable_v<line_segment>);
	static_assert(std::is_trivially_move_constructible_v<line_segment>);
	static_assert(std::is_trivially_move_assignable_v<line_segment>);
	static_assert(std::is_trivially_destructible_v<line_segment>);
	static_assert(std::is_nothrow_constructible_v<line_segment, vec3, vec3>);
	static_assert(std::is_nothrow_constructible_v<line_segment, const vec3(&)[2]>);
	static_assert(std::is_nothrow_constructible_v<line_segment, T, T, T, T, T, T>);

	BATCHED_SECTION("zero-initialization")
	{
		const auto seg = line_segment{};
		for (auto& p : seg)
			CHECK(p == vec3{});
	}

	BATCHED_SECTION("two points constructor")
	{
		const auto p0  = vec3{ random_array<T, 3>() };
		const auto p1  = vec3{ random_array<T, 3>() };
		const auto seg = line_segment{ p0, p1 };
		TEST_INFO("seg: "sv << seg);
		CHECK(seg[0] == p0);
		CHECK(seg[1] == p1);
	}

	BATCHED_SECTION("array constructor")
	{
		const vec3 points[2] = { vec3{ random_array<T, 3>() }, vec3{ random_array<T, 3>() } };
		const auto seg		 = line_segment{ points };
		TEST_INFO("seg: "sv << seg);
		CHECK(seg[0] == points[0]);
		CHECK(seg[1] == points[1]);
	}

	BATCHED_SECTION("copy constructor")
	{
		line_segment seg1;
		line_segment_for_each(seg1, [](auto& s1, size_t) noexcept { s1 = random<T>(); });
		line_segment seg2{ seg1 };
		line_segment_for_each(seg1, seg2, [](auto s1, auto s2, size_t) { CHECK(s1 == s2); });
	}

	BATCHED_SECTION("blitting constructor")
	{
		blittable<T> seg1{};
		line_segment_for_each(seg1, [](auto& s1, size_t) { s1 = random<T>(); });
		line_segment seg2{ seg1 };
		line_segment_for_each(seg1, seg2, [](auto s1, auto s2, size_t) { CHECK(s1 == s2); });
	}

	BATCHED_SECTION("data()")
	{
		auto seg			  = line_segment{};
		const auto& seg_const = seg;
		CHECK(reinterpret_cast<uintptr_t>(seg.data()) == reinterpret_cast<uintptr_t>(&seg));
		CHECK(reinterpret_cast<uintptr_t>(seg_const.data()) == reinterpret_cast<uintptr_t>(&seg_const));
	}
}
