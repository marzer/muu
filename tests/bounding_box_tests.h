// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"
#include "batching.h"
#include "../include/muu/bounding_box.h"
#include "../include/muu/triangle.h"

namespace
{
	template <typename T, typename Func>
	static void aabb_for_each(T& bb, Func&& func)
	{
		static_cast<Func&&>(func)(bb.center.x, 0_sz);
		static_cast<Func&&>(func)(bb.center.y, 1_sz);
		static_cast<Func&&>(func)(bb.center.z, 2_sz);
		static_cast<Func&&>(func)(bb.extents.x, 3_sz);
		static_cast<Func&&>(func)(bb.extents.y, 4_sz);
		static_cast<Func&&>(func)(bb.extents.z, 5_sz);
	}

	template <typename T, typename U, typename Func>
	static void aabb_for_each(T& bb1, U& bb2, Func&& func)
	{
		static_cast<Func&&>(func)(bb1.center.x, bb2.center.x, 0_sz);
		static_cast<Func&&>(func)(bb1.center.y, bb2.center.y, 1_sz);
		static_cast<Func&&>(func)(bb1.center.z, bb2.center.z, 2_sz);
		static_cast<Func&&>(func)(bb1.extents.x, bb2.extents.x, 3_sz);
		static_cast<Func&&>(func)(bb1.extents.y, bb2.extents.y, 4_sz);
		static_cast<Func&&>(func)(bb1.extents.z, bb2.extents.z, 5_sz);
	}

	inline constexpr size_t aabb_scalar_count = 6;

	template <typename T>
	struct blittable
	{
		vector<T, 3> center;
		vector<T, 3> extents;
	};

	template <typename... T>
	using bounding_boxes = type_list<bounding_box<T>...>;
}

namespace muu
{
	template <typename T>
	inline constexpr bool allow_implicit_bit_cast<::blittable<T>, bounding_box<T>> = true;
}

BATCHED_TEST_CASE("bounding_box constructors", bounding_boxes<ALL_FLOATS>)
{
	using aabb = TestType;
	using T	   = typename aabb::scalar_type;
	using vec3 = vector<T, 3>;
	TEST_INFO("bounding_box<"sv << nameof<T> << ">"sv);

	static_assert(sizeof(aabb) == sizeof(vec3) * 2);
	static_assert(std::is_standard_layout_v<aabb>);
	static_assert(std::is_trivially_constructible_v<aabb>);
	static_assert(std::is_trivially_copy_constructible_v<aabb>);
	static_assert(std::is_trivially_copy_assignable_v<aabb>);
	static_assert(std::is_trivially_move_constructible_v<aabb>);
	static_assert(std::is_trivially_move_assignable_v<aabb>);
	static_assert(std::is_trivially_destructible_v<aabb>);
	static_assert(std::is_nothrow_constructible_v<aabb, vec3, vec3>);
	static_assert(std::is_nothrow_constructible_v<aabb, vec3, T, T, T>);
	static_assert(std::is_nothrow_constructible_v<aabb, vec3, T>);
	static_assert(std::is_nothrow_constructible_v<aabb, vec3>);
	static_assert(std::is_nothrow_constructible_v<aabb, T, T, T, vec3>);
	static_assert(std::is_nothrow_constructible_v<aabb, T, T, T, T, T, T>);
	static_assert(std::is_nothrow_constructible_v<aabb, T>);

	BATCHED_SECTION("zero-initialization")
	{
		const auto bb = aabb{};
		aabb_for_each(bb, [](auto s, size_t) { CHECK(s == T{}); });
	}

	BATCHED_SECTION("vector + vector constructor")
	{
		const auto c  = random_array<T, 3>();
		const auto e  = random_array<T, 3>();
		const auto bb = aabb{ vec3{ c }, vec3{ e } };
		CHECK(bb.center[0] == c[0]);
		CHECK(bb.center[1] == c[1]);
		CHECK(bb.center[2] == c[2]);
		CHECK(bb.extents[0] == e[0]);
		CHECK(bb.extents[1] == e[1]);
		CHECK(bb.extents[2] == e[2]);
	}

	BATCHED_SECTION("vector + 3 scalars constructor")
	{
		const auto c  = random_array<T, 3>();
		const auto e  = random_array<T, 3>();
		const auto bb = aabb{ vec3{ c }, e[0], e[1], e[2] };
		CHECK(bb.center[0] == c[0]);
		CHECK(bb.center[1] == c[1]);
		CHECK(bb.center[2] == c[2]);
		CHECK(bb.extents[0] == e[0]);
		CHECK(bb.extents[1] == e[1]);
		CHECK(bb.extents[2] == e[2]);
	}

	BATCHED_SECTION("vector + 1 scalar constructor")
	{
		const auto c  = random_array<T, 3>();
		const auto e  = random<T>();
		const auto bb = aabb{ vec3{ c }, e };
		CHECK(bb.center[0] == c[0]);
		CHECK(bb.center[1] == c[1]);
		CHECK(bb.center[2] == c[2]);
		CHECK(bb.extents[0] == e);
		CHECK(bb.extents[1] == e);
		CHECK(bb.extents[2] == e);
	}

	BATCHED_SECTION("vector constructor")
	{
		const auto e  = random_array<T, 3>();
		const auto bb = aabb{ vec3{ e } };
		CHECK(bb.center[0] == T{});
		CHECK(bb.center[1] == T{});
		CHECK(bb.center[2] == T{});
		CHECK(bb.extents[0] == e[0]);
		CHECK(bb.extents[1] == e[1]);
		CHECK(bb.extents[2] == e[2]);
	}

	BATCHED_SECTION("3 scalars + vector constructor")
	{
		const auto c  = random_array<T, 3>();
		const auto e  = random_array<T, 3>();
		const auto bb = aabb{ c[0], c[1], c[2], vec3{ e } };
		CHECK(bb.center[0] == c[0]);
		CHECK(bb.center[1] == c[1]);
		CHECK(bb.center[2] == c[2]);
		CHECK(bb.extents[0] == e[0]);
		CHECK(bb.extents[1] == e[1]);
		CHECK(bb.extents[2] == e[2]);
	}

	BATCHED_SECTION("6 scalars constructor")
	{
		const auto vals = random_array<T, 6>();
		const auto bb	= aabb{ vals[0], vals[1], vals[2], vals[3], vals[4], vals[5] };
		CHECK(bb.center[0] == vals[0]);
		CHECK(bb.center[1] == vals[1]);
		CHECK(bb.center[2] == vals[2]);
		CHECK(bb.extents[0] == vals[3]);
		CHECK(bb.extents[1] == vals[4]);
		CHECK(bb.extents[2] == vals[5]);
	}

	BATCHED_SECTION("scalar constructor")
	{
		const auto e  = random<T>();
		const auto bb = aabb{ e };
		CHECK(bb.center[0] == T{});
		CHECK(bb.center[1] == T{});
		CHECK(bb.center[2] == T{});
		CHECK(bb.extents[0] == e);
		CHECK(bb.extents[1] == e);
		CHECK(bb.extents[2] == e);
	}

	BATCHED_SECTION("copy constructor")
	{
		aabb bb1;
		aabb_for_each(bb1, [](auto& s1, size_t) { s1 = random<T>(); });
		aabb bb2{ bb1 };
		aabb_for_each(bb1, bb2, [](auto s1, auto s2, size_t) { CHECK(s1 == s2); });
	}

	BATCHED_SECTION("blitting constructor")
	{
		blittable<T> bb1;
		aabb_for_each(bb1, [](auto& s1, size_t) { s1 = random<T>(); });
		aabb bb2{ bb1 };
		aabb_for_each(bb1, bb2, [](auto s1, auto s2, size_t) { CHECK(s1 == s2); });
	}

	BATCHED_SECTION("data()")
	{
		auto bb				 = aabb{};
		const auto& bb_const = bb;
		CHECK(reinterpret_cast<uintptr_t>(bb.data()) == reinterpret_cast<uintptr_t>(&bb));
		CHECK(reinterpret_cast<uintptr_t>(bb_const.data()) == reinterpret_cast<uintptr_t>(&bb_const));
	}
}

BATCHED_TEST_CASE("bounding_box equality", bounding_boxes<ALL_FLOATS>)
{
	using aabb = TestType;
	using T	   = typename aabb::scalar_type;
	TEST_INFO("bounding_box<"sv << nameof<T> << ">"sv);

	aabb bb;
	aabb_for_each(bb, [](auto& s, size_t) noexcept { s = random<T>(); });

	BATCHED_SECTION("same type")
	{
		aabb same{ bb };
		CHECK_SYMMETRIC_EQUAL(bb, same);
		if constexpr (is_floating_point<T>)
		{
			CHECK(aabb::approx_equal(bb, same));
			CHECK(muu::approx_equal(bb, same));
		}

		aabb different{ bb };
		aabb_for_each(different, [](auto& s, size_t) noexcept { s++; });
		CHECK_SYMMETRIC_INEQUAL(bb, different);
		if constexpr (is_floating_point<T>)
		{
			CHECK_FALSE(aabb::approx_equal(bb, different));
			CHECK_FALSE(muu::approx_equal(bb, different));
		}
	}

	if constexpr (!is_floating_point<T>)
	{
		BATCHED_SECTION("different type")
		{
			using other_scalar = std::conditional_t<std::is_same_v<T, long>, int, long>;
			using other		   = bounding_box<other_scalar>;

			other same;
			aabb_for_each(same,
						  bb,
						  [](auto& lhs, auto& rhs, size_t) noexcept { lhs = static_cast<other_scalar>(rhs); });
			CHECK_SYMMETRIC_EQUAL(bb, same);

			other different;
			aabb_for_each(different,
						  bb,
						  [](auto& lhs, auto& rhs, size_t) noexcept
						  {
							  lhs = static_cast<other_scalar>(rhs);
							  lhs++;
						  });
			CHECK_SYMMETRIC_INEQUAL(bb, different);
		}
	}
}

BATCHED_TEST_CASE("bounding_box zero()", bounding_boxes<ALL_FLOATS>)
{
	using aabb = TestType;
	using T	   = typename aabb::scalar_type;
	TEST_INFO("bounding_box<"sv << nameof<T> << ">"sv);

	BATCHED_SECTION("all zeroes")
	{
		aabb bb{};
		CHECK(bb.zero());
	}

	BATCHED_SECTION("no zeroes")
	{
		aabb bb;
		aabb_for_each(bb, [](auto& s, size_t) noexcept { s = random<T>(1, 10); });
		CHECK_FALSE(bb.zero());
	}

	BATCHED_SECTION("some zeroes")
	{
		aabb bb{};
		aabb_for_each(bb,
					  [](auto& s, size_t i) noexcept
					  {
						  if ((i % 2u))
							  s = random<T>(1, 10);
					  });
		CHECK_FALSE(bb.zero());
	}

	BATCHED_SECTION("one zero")
	{
		for (size_t i = 0; i < aabb_scalar_count; i++)
		{
			aabb bb{};
			aabb_for_each(bb,
						  [=](auto& s, size_t j) noexcept
						  {
							  if (i == j)
								  s = random<T>(1, 10);
						  });
			CHECK_FALSE(bb.zero());
		}
	}
}

BATCHED_TEST_CASE("bounding_box infinity_or_nan()", bounding_boxes<ALL_FLOATS>)
{
	using aabb = TestType;
	using T	   = typename aabb::scalar_type;
	TEST_INFO("bounding_box<"sv << nameof<T> << ">"sv);

	BATCHED_SECTION("all finite")
	{
		aabb bb;
		aabb_for_each(bb, [](auto& s, size_t) noexcept { s = random<T>(1, 10); });
		CHECK_FALSE(bb.infinity_or_nan());
		CHECK_FALSE(muu::infinity_or_nan(bb));
	}

	if constexpr (is_floating_point<T>)
	{
		BATCHED_SECTION("contains one NaN")
		{
			for (size_t i = 0; i < aabb_scalar_count; i++)
			{
				aabb bb{};
				aabb_for_each(bb,
							  [=](auto& s, size_t j) noexcept
							  {
								  if (i == j)
									  s = make_nan<T>();
							  });
				CHECK(bb.infinity_or_nan());
				CHECK(muu::infinity_or_nan(bb));
			}
		}
	}

	if constexpr (is_floating_point<T>)
	{
		BATCHED_SECTION("contains one infinity")
		{
			for (size_t i = 0; i < aabb_scalar_count; i++)
			{
				aabb bb{};
				aabb_for_each(bb,
							  [=](auto& s, size_t j) noexcept
							  {
								  if (i == j)
									  s = make_infinity<T>();
							  });
				CHECK(bb.infinity_or_nan());
				CHECK(muu::infinity_or_nan(bb));
			}
		}
	}
}

BATCHED_TEST_CASE("bounding_box degenerate()", bounding_boxes<ALL_FLOATS>)
{
	using aabb = TestType;
	using T	   = typename aabb::scalar_type;
	using vec3 = vector<T, 3>;
	TEST_INFO("bounding_sphere<"sv << nameof<T> << ">"sv);

	BATCHED_SECTION("false")
	{
		aabb bb;

		RANDOM_ITERATIONS
		{
			bb.center  = vec3{ random_array<T, 3>(1, 10) };
			bb.extents = vec3{ random_array<T, 3>(1, 10) };
			CHECK(!bb.degenerate());
			CHECK(!aabb::degenerate(bb));
			CHECK(!muu::degenerate(bb));
		}
	}

	BATCHED_SECTION("true")
	{
		aabb bb;

		RANDOM_ITERATIONS
		{
			bb.center  = vec3{ random_array<T, 3>(1, 10) };
			bb.extents = vec3{ random_array<T, 3>(-10, 0) };
			CHECK(bb.degenerate());
			CHECK(aabb::degenerate(bb));
			CHECK(muu::degenerate(bb));
		}
	}
}

BATCHED_TEST_CASE("bounding_box intersections", bounding_boxes<ALL_FLOATS>)
{
	using aabb = TestType;
	using T	   = typename aabb::scalar_type;
	using vec3 = vector<T, 3>;
	TEST_INFO("bounding_box<"sv << nameof<T> << ">"sv);

	constexpr auto unit_box = aabb{ constants<T>::one_over_two };

	[[maybe_unused]] const auto tall_box =
		aabb{ constants<vec3>::zero, static_cast<T>(0.1), static_cast<T>(50), static_cast<T>(0.5) };

	// these test cases were created by experimentation and manual transcription from Maya
	// please press F to pay respects

	// box x box
	BATCHED_SECTION("aabb x aabb")
	{
#define CHECK_INTERSECTION(expected, x, y, z, w, h, d)                                                                 \
	CHECK(expected                                                                                                     \
		  == unit_box.intersects(aabb(vec3{ static_cast<T>(x), static_cast<T>(y), static_cast<T>(z) },                 \
									  vec3{ static_cast<T>(w), static_cast<T>(h), static_cast<T>(d) } / T{ 2 })))

		CHECK_INTERSECTION(true, 0, 0, 0, 1, 1, 1);
		CHECK_INTERSECTION(true, 0, 0, 0, 0.5, 0.5, 0.5);
		CHECK_INTERSECTION(true, 0.34, 0.236, 0.224, 0.5, 0.5, 0.5);

		CHECK_INTERSECTION(false, 1, 0, 0, 0.5, 0.5, 0.5);
		CHECK_INTERSECTION(false, 1, 0.6, 0.7, 0.5, 0.5, 0.5);
		CHECK_INTERSECTION(false, 0, 1, 0, 10, 0.5, 10);

#undef CHECK_INTERSECTION
	}

	// box x tri
	BATCHED_SECTION("aabb x tri")
	{
#define CHECK_INTERSECTION(box, expected, x0, y0, z0, x1, y1, z1, x2, y2, z2)                                          \
	CHECK(expected                                                                                                     \
		  == box.intersects(triangle{ static_cast<T>(x0),                                                              \
									  static_cast<T>(y0),                                                              \
									  static_cast<T>(z0),                                                              \
									  static_cast<T>(x1),                                                              \
									  static_cast<T>(y1),                                                              \
									  static_cast<T>(z1),                                                              \
									  static_cast<T>(x2),                                                              \
									  static_cast<T>(y2),                                                              \
									  static_cast<T>(z2) }))

		CHECK_INTERSECTION(unit_box, true, -2, 0, 2, 2, 0, 2, -2, 0, -2);
		CHECK_INTERSECTION(unit_box, true, -3, -1, 1, 0, -1, 3, 0, 1, 0);
		CHECK_INTERSECTION(unit_box, true, 0.3, 0.4, 0.8, 0.8, 0.4, 0.3, 0.3, 0.6, 0.3);
		CHECK_INTERSECTION(unit_box, true, 0, 0, 0.4, 0.4, 0, 0, 0, 0, 0);
		CHECK_INTERSECTION(unit_box, true, 0, 7, -0.403, 3, -2.5, -0.538, -3, -2.5, -0.538);

		CHECK_INTERSECTION(unit_box, false, 0, 7, -0.403, 3, -2.5, -0.547, -3, -2.5, -0.547);
		CHECK_INTERSECTION(unit_box, false, -3, -1, 1, 0, -1, 3, 0, 1.162, 0);
		CHECK_INTERSECTION(unit_box, false, -1, 0, 1, 1, 0, 1, -1, 2.5, -1);
		CHECK_INTERSECTION(unit_box, false, -2, 2, 2, 2, 2, 2, -2, 2, -2);

		if constexpr (!std::is_same_v<T, half>)
		{
			CHECK_INTERSECTION(unit_box,
							   true,
							   0.24255,
							   6.58204,
							   3.75181,
							   -0.50413,
							   0.43569,
							   -0.54796,
							   -3.00000,
							   -2.50000,
							   -0.37540);
			CHECK_INTERSECTION(unit_box,
							   true,
							   0.24255,
							   6.58204,
							   3.75181,
							   -0.50482,
							   0.43583,
							   -0.54797,
							   -3.00000,
							   -2.50000,
							   -0.37540);

			CHECK_INTERSECTION(unit_box,
							   false,
							   0.24255,
							   6.58204,
							   3.75181,
							   -0.50633,
							   0.43583,
							   -0.54797,
							   -3.00000,
							   -2.50000,
							   -0.37540);

			CHECK_INTERSECTION(tall_box, true, 0, 7, 70, 2, -2, -60, -2, -2, -60);
		}

#undef CHECK_INTERSECTION
	}
}

BATCHED_TEST_CASE("bounding_box corners", bounding_boxes<ALL_FLOATS>)
{
	using aabb = TestType;
	using T	   = typename aabb::scalar_type;
	using vec3 = vector<T, 3>;
	TEST_INFO("bounding_box<"sv << nameof<T> << ">"sv);

	constexpr auto pos = constants<T>::one_over_two;
	constexpr auto neg = -constants<T>::one_over_two;

	const auto box = aabb{ vec3{}, vec3{ pos } };

	BATCHED_SECTION("aabb::corner<>()")
	{
		CHECK(aabb::template corner<box_corner::min>(box) == vec3{ neg });
		CHECK(aabb::template corner<box_corner::x>(box) == vec3{ pos, neg, neg });
		CHECK(aabb::template corner<box_corner::y>(box) == vec3{ neg, pos, neg });
		CHECK(aabb::template corner<box_corner::z>(box) == vec3{ neg, neg, pos });
		CHECK(aabb::template corner<box_corner::xy>(box) == vec3{ pos, pos, neg });
		CHECK(aabb::template corner<box_corner::xz>(box) == vec3{ pos, neg, pos });
		CHECK(aabb::template corner<box_corner::yz>(box) == vec3{ neg, pos, pos });
		CHECK(aabb::template corner<box_corner::xyz>(box) == vec3{ pos });
		CHECK(aabb::template corner<box_corner::max>(box) == vec3{ pos });
	}

	BATCHED_SECTION("aabb.corner<>()")
	{
		CHECK(box.template corner<box_corner::min>() == vec3{ neg });
		CHECK(box.template corner<box_corner::x>() == vec3{ pos, neg, neg });
		CHECK(box.template corner<box_corner::y>() == vec3{ neg, pos, neg });
		CHECK(box.template corner<box_corner::z>() == vec3{ neg, neg, pos });
		CHECK(box.template corner<box_corner::xy>() == vec3{ pos, pos, neg });
		CHECK(box.template corner<box_corner::xz>() == vec3{ pos, neg, pos });
		CHECK(box.template corner<box_corner::yz>() == vec3{ neg, pos, pos });
		CHECK(box.template corner<box_corner::xyz>() == vec3{ pos });
		CHECK(box.template corner<box_corner::max>() == vec3{ pos });
	}

	BATCHED_SECTION("aabb::corner()")
	{
		CHECK(aabb::corner(box, box_corner::min) == vec3{ neg });
		CHECK(aabb::corner(box, box_corner::x) == vec3{ pos, neg, neg });
		CHECK(aabb::corner(box, box_corner::y) == vec3{ neg, pos, neg });
		CHECK(aabb::corner(box, box_corner::z) == vec3{ neg, neg, pos });
		CHECK(aabb::corner(box, box_corner::xy) == vec3{ pos, pos, neg });
		CHECK(aabb::corner(box, box_corner::xz) == vec3{ pos, neg, pos });
		CHECK(aabb::corner(box, box_corner::yz) == vec3{ neg, pos, pos });
		CHECK(aabb::corner(box, box_corner::xyz) == vec3{ pos });
		CHECK(aabb::corner(box, box_corner::max) == vec3{ pos });
	}

	BATCHED_SECTION("aabb.corner()")
	{
		CHECK(box.corner(box_corner::min) == vec3{ neg });
		CHECK(box.corner(box_corner::x) == vec3{ pos, neg, neg });
		CHECK(box.corner(box_corner::y) == vec3{ neg, pos, neg });
		CHECK(box.corner(box_corner::z) == vec3{ neg, neg, pos });
		CHECK(box.corner(box_corner::xy) == vec3{ pos, pos, neg });
		CHECK(box.corner(box_corner::xz) == vec3{ pos, neg, pos });
		CHECK(box.corner(box_corner::yz) == vec3{ neg, pos, pos });
		CHECK(box.corner(box_corner::xyz) == vec3{ pos });
		CHECK(box.corner(box_corner::max) == vec3{ pos });
	}

	BATCHED_SECTION("min_corner()")
	{
		CHECK(aabb::min_corner(box) == vec3{ neg });
		CHECK(box.min_corner() == vec3{ neg });
	}

	BATCHED_SECTION("max_corner()")
	{
		CHECK(aabb::max_corner(box) == vec3{ pos });
		CHECK(box.max_corner() == vec3{ pos });
	}
}
