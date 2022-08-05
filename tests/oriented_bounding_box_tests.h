// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"
#include "batching.h"
#include "../include/muu/oriented_bounding_box.h"

namespace
{
	template <typename T, typename Func>
	static void obb_for_each(T& bb, Func&& func)
	{
		size_t idx{};
		for (size_t i = 0; i < 3; i++)
			static_cast<Func&&>(func)(bb.center[i], idx++);
		for (size_t i = 0; i < 3; i++)
			static_cast<Func&&>(func)(bb.extents[i], idx++);
		for (size_t r = 0; r < 3; r++)
			for (size_t c = 0; c < 3; c++)
				static_cast<Func&&>(func)(bb.axes(r, c), idx++);
	}

	template <typename T, typename U, typename Func>
	static void obb_for_each(T& bb1, U& bb2, Func&& func)
	{
		size_t idx{};
		for (size_t i = 0; i < 3; i++)
			static_cast<Func&&>(func)(bb1.center[i], bb2.center[i], idx++);
		for (size_t i = 0; i < 3; i++)
			static_cast<Func&&>(func)(bb1.extents[i], bb2.extents[i], idx++);
		for (size_t r = 0; r < 3; r++)
			for (size_t c = 0; c < 3; c++)
				static_cast<Func&&>(func)(bb1.axes(r, c), bb2.axes(r, c), idx++);
	}

	inline constexpr size_t obb_scalar_count = 15;

	template <typename T>
	struct blittable
	{
		vector<T, 3> center;
		vector<T, 3> extents;
		matrix<T, 3, 3> axes;
	};

	template <typename... T>
	using oriented_bounding_boxes = type_list<oriented_bounding_box<T>...>;
}

namespace muu
{
	template <typename T>
	inline constexpr bool allow_implicit_bit_cast<blittable<T>, oriented_bounding_box<T>> = true;
}

BATCHED_TEST_CASE("oriented_bounding_box constructors", oriented_bounding_boxes<ALL_FLOATS>)
{
	using obb  = TestType;
	using T	   = typename obb::scalar_type;
	using vec3 = vector<T, 3>;
	using mat3 = matrix<T, 3, 3>;
	TEST_INFO("oriented_bounding_box<"sv << nameof<T> << ">"sv);

	static_assert(sizeof(obb) == (sizeof(vec3) * 2 + sizeof(mat3)));
	static_assert(std::is_standard_layout_v<obb>);
	static_assert(std::is_trivially_constructible_v<obb>);
	static_assert(std::is_trivially_copy_constructible_v<obb>);
	static_assert(std::is_trivially_copy_assignable_v<obb>);
	static_assert(std::is_trivially_move_constructible_v<obb>);
	static_assert(std::is_trivially_move_assignable_v<obb>);
	static_assert(std::is_trivially_destructible_v<obb>);
	static_assert(std::is_nothrow_constructible_v<obb, vec3, vec3>);
	static_assert(std::is_nothrow_constructible_v<obb, vec3, T, T, T>);
	static_assert(std::is_nothrow_constructible_v<obb, vec3, T>);
	static_assert(std::is_nothrow_constructible_v<obb, vec3>);
	static_assert(std::is_nothrow_constructible_v<obb, T, T, T, vec3>);
	static_assert(std::is_nothrow_constructible_v<obb, T, T, T, T, T, T>);
	static_assert(std::is_nothrow_constructible_v<obb, T>);

	BATCHED_SECTION("zero-initialization")
	{
		const auto bb = obb{};
		obb_for_each(bb, [](auto s, size_t) { CHECK(s == T{}); });
	}

	BATCHED_SECTION("vector + vector constructor")
	{
		const auto c  = random_array<T, 3>();
		const auto e  = random_array<T, 3>();
		const auto bb = obb{ vec3{ c }, vec3{ e } };
		CHECK(bb.center[0] == c[0]);
		CHECK(bb.center[1] == c[1]);
		CHECK(bb.center[2] == c[2]);
		CHECK(bb.extents[0] == e[0]);
		CHECK(bb.extents[1] == e[1]);
		CHECK(bb.extents[2] == e[2]);
		CHECK(bb.axes == mat3::constants::identity);
	}

	BATCHED_SECTION("vector + 3 scalars constructor")
	{
		const auto c  = random_array<T, 3>();
		const auto e  = random_array<T, 3>();
		const auto bb = obb{ vec3{ c }, e[0], e[1], e[2] };
		CHECK(bb.center[0] == c[0]);
		CHECK(bb.center[1] == c[1]);
		CHECK(bb.center[2] == c[2]);
		CHECK(bb.extents[0] == e[0]);
		CHECK(bb.extents[1] == e[1]);
		CHECK(bb.extents[2] == e[2]);
		CHECK(bb.axes == mat3::constants::identity);
	}

	BATCHED_SECTION("vector + 1 scalar constructor")
	{
		const auto c  = random_array<T, 3>();
		const auto e  = random<T>();
		const auto bb = obb{ vec3{ c }, e };
		CHECK(bb.center[0] == c[0]);
		CHECK(bb.center[1] == c[1]);
		CHECK(bb.center[2] == c[2]);
		CHECK(bb.extents[0] == e);
		CHECK(bb.extents[1] == e);
		CHECK(bb.extents[2] == e);
		CHECK(bb.axes == mat3::constants::identity);
	}

	BATCHED_SECTION("vector constructor")
	{
		const auto e  = random_array<T, 3>();
		const auto bb = obb{ vec3{ e } };
		CHECK(bb.center[0] == T{});
		CHECK(bb.center[1] == T{});
		CHECK(bb.center[2] == T{});
		CHECK(bb.extents[0] == e[0]);
		CHECK(bb.extents[1] == e[1]);
		CHECK(bb.extents[2] == e[2]);
		CHECK(bb.axes == mat3::constants::identity);
	}

	BATCHED_SECTION("3 scalars + vector constructor")
	{
		const auto c  = random_array<T, 3>();
		const auto e  = random_array<T, 3>();
		const auto bb = obb{ c[0], c[1], c[2], vec3{ e } };
		CHECK(bb.center[0] == c[0]);
		CHECK(bb.center[1] == c[1]);
		CHECK(bb.center[2] == c[2]);
		CHECK(bb.extents[0] == e[0]);
		CHECK(bb.extents[1] == e[1]);
		CHECK(bb.extents[2] == e[2]);
		CHECK(bb.axes == mat3::constants::identity);
	}

	BATCHED_SECTION("6 scalars constructor")
	{
		const auto vals = random_array<T, 6>();
		const auto bb	= obb{ vals[0], vals[1], vals[2], vals[3], vals[4], vals[5] };
		CHECK(bb.center[0] == vals[0]);
		CHECK(bb.center[1] == vals[1]);
		CHECK(bb.center[2] == vals[2]);
		CHECK(bb.extents[0] == vals[3]);
		CHECK(bb.extents[1] == vals[4]);
		CHECK(bb.extents[2] == vals[5]);
		CHECK(bb.axes == mat3::constants::identity);
	}

	BATCHED_SECTION("scalar constructor")
	{
		const auto e  = random<T>();
		const auto bb = obb{ e };
		CHECK(bb.center[0] == T{});
		CHECK(bb.center[1] == T{});
		CHECK(bb.center[2] == T{});
		CHECK(bb.extents[0] == e);
		CHECK(bb.extents[1] == e);
		CHECK(bb.extents[2] == e);
		CHECK(bb.axes == mat3::constants::identity);
	}

	BATCHED_SECTION("copy constructor")
	{
		obb bb1;
		obb_for_each(bb1, [](auto& s1, size_t) { s1 = random<T>(); });
		obb bb2{ bb1 };
		obb_for_each(bb1, bb2, [](auto s1, auto s2, size_t) { CHECK(s1 == s2); });
	}

	BATCHED_SECTION("blitting constructor")
	{
		blittable<T> bb1;
		obb_for_each(bb1, [](auto& s1, size_t) { s1 = random<T>(); });
		obb bb2{ bb1 };
		obb_for_each(bb1, bb2, [](auto s1, auto s2, size_t) { CHECK(s1 == s2); });
	}

	BATCHED_SECTION("data()")
	{
		auto bb				 = obb{};
		const auto& bb_const = bb;
		CHECK(reinterpret_cast<uintptr_t>(bb.data()) == reinterpret_cast<uintptr_t>(&bb));
		CHECK(reinterpret_cast<uintptr_t>(bb_const.data()) == reinterpret_cast<uintptr_t>(&bb_const));
	}
}

BATCHED_TEST_CASE("oriented_bounding_box equality", oriented_bounding_boxes<ALL_FLOATS>)
{
	using obb = TestType;
	using T	  = typename obb::scalar_type;
	TEST_INFO("oriented_bounding_box<"sv << nameof<T> << ">"sv);

	obb bb;
	obb_for_each(bb, [](auto& s, size_t) noexcept { s = random<T>(); });

	BATCHED_SECTION("same type")
	{
		obb same{ bb };
		CHECK_SYMMETRIC_EQUAL(bb, same);
		if constexpr (is_floating_point<T>)
		{
			CHECK(obb::approx_equal(bb, same));
			CHECK(muu::approx_equal(bb, same));
		}

		obb different{ bb };
		obb_for_each(different, [](auto& s, size_t) noexcept { s++; });
		CHECK_SYMMETRIC_INEQUAL(bb, different);
		if constexpr (is_floating_point<T>)
		{
			CHECK_FALSE(obb::approx_equal(bb, different));
			CHECK_FALSE(muu::approx_equal(bb, different));
		}
	}

	if constexpr (!is_floating_point<T>)
	{
		BATCHED_SECTION("different type")
		{
			using other_scalar = std::conditional_t<std::is_same_v<T, long>, int, long>;
			using other		   = oriented_bounding_box<other_scalar>;

			other same;
			obb_for_each(same, bb, [](auto& lhs, auto& rhs, size_t) noexcept { lhs = static_cast<other_scalar>(rhs); });
			CHECK_SYMMETRIC_EQUAL(bb, same);

			other different;
			obb_for_each(different,
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

BATCHED_TEST_CASE("oriented_bounding_box zero()", oriented_bounding_boxes<ALL_FLOATS>)
{
	using obb = TestType;
	using T	  = typename obb::scalar_type;
	TEST_INFO("oriented_bounding_box<"sv << nameof<T> << ">"sv);

	BATCHED_SECTION("all zeroes")
	{
		obb bb{};
		CHECK(bb.zero());
	}

	BATCHED_SECTION("no zeroes")
	{
		obb bb;
		obb_for_each(bb, [](auto& s, size_t) noexcept { s = random<T>(1, 10); });
		CHECK_FALSE(bb.zero());
	}

	BATCHED_SECTION("some zeroes")
	{
		obb bb{};
		obb_for_each(bb,
					 [](auto& s, size_t i) noexcept
					 {
						 if ((i % 2u))
							 s = random<T>(1, 10);
					 });
		CHECK_FALSE(bb.zero());
	}

	BATCHED_SECTION("one zero")
	{
		for (size_t i = 0; i < obb_scalar_count; i++)
		{
			obb bb{};
			obb_for_each(bb,
						 [=](auto& s, size_t j) noexcept
						 {
							 if (i == j)
								 s = random<T>(1, 10);
						 });
			CHECK_FALSE(bb.zero());
		}
	}
}

BATCHED_TEST_CASE("oriented_bounding_box infinity_or_nan()", oriented_bounding_boxes<ALL_FLOATS>)
{
	using obb = TestType;
	using T	  = typename obb::scalar_type;
	TEST_INFO("oriented_bounding_box<"sv << nameof<T> << ">"sv);

	BATCHED_SECTION("all finite")
	{
		obb bb;
		obb_for_each(bb, [](auto& s, size_t) noexcept { s = random<T>(1, 10); });
		CHECK_FALSE(bb.infinity_or_nan());
		CHECK_FALSE(muu::infinity_or_nan(bb));
	}

	if constexpr (is_floating_point<T>)
	{
		BATCHED_SECTION("contains one NaN")
		{
			for (size_t i = 0; i < obb_scalar_count; i++)
			{
				obb bb{};
				obb_for_each(bb,
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
			for (size_t i = 0; i < obb_scalar_count; i++)
			{
				obb bb{};
				obb_for_each(bb,
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

BATCHED_TEST_CASE("oriented_bounding_box degenerate()", oriented_bounding_boxes<ALL_FLOATS>)
{
	using obb  = TestType;
	using T	   = typename obb::scalar_type;
	using vec3 = vector<T, 3>;
	using mat3 = matrix<T, 3, 3>;
	TEST_INFO("oriented_bounding_box<"sv << nameof<T> << ">"sv);

	BATCHED_SECTION("false")
	{
		obb bb;
		bb.axes = mat3::constants::identity;

		RANDOM_ITERATIONS
		{
			bb.center  = vec3{ random_array<T, 3>(1, 10) };
			bb.extents = vec3{ random_array<T, 3>(1, 10) };
			CHECK(!bb.degenerate());
			CHECK(!obb::degenerate(bb));
			CHECK(!muu::degenerate(bb));
		}
	}

	BATCHED_SECTION("true")
	{
		obb bb;
		bb.axes = mat3::constants::identity;

		RANDOM_ITERATIONS
		{
			bb.center  = vec3{ random_array<T, 3>(1, 10) };
			bb.extents = vec3{ random_array<T, 3>(-10, 0) };
			CHECK(bb.degenerate());
			CHECK(obb::degenerate(bb));
			CHECK(muu::degenerate(bb));
		}
	}
}

BATCHED_TEST_CASE("oriented_bounding_box corners", oriented_bounding_boxes<ALL_FLOATS>)
{
	using obb  = TestType;
	using T	   = typename obb::scalar_type;
	using vec3 = vector<T, 3>;
	using mat3 = matrix<T, 3, 3>;
	TEST_INFO("oriented_bounding_box<"sv << nameof<T> << ">"sv);

	constexpr auto pos = constants<T>::one_over_two;
	constexpr auto neg = -constants<T>::one_over_two;

	const auto box = obb{ vec3{}, vec3{ pos }, mat3::constants::identity };

	BATCHED_SECTION("obb::corner<>()")
	{
		CHECK(obb::template corner<box_corner::min>(box) == vec3{ neg });
		CHECK(obb::template corner<box_corner::x>(box) == vec3{ pos, neg, neg });
		CHECK(obb::template corner<box_corner::y>(box) == vec3{ neg, pos, neg });
		CHECK(obb::template corner<box_corner::z>(box) == vec3{ neg, neg, pos });
		CHECK(obb::template corner<box_corner::xy>(box) == vec3{ pos, pos, neg });
		CHECK(obb::template corner<box_corner::xz>(box) == vec3{ pos, neg, pos });
		CHECK(obb::template corner<box_corner::yz>(box) == vec3{ neg, pos, pos });
		CHECK(obb::template corner<box_corner::xyz>(box) == vec3{ pos });
		CHECK(obb::template corner<box_corner::max>(box) == vec3{ pos });
	}

	BATCHED_SECTION("obb.corner<>()")
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

	BATCHED_SECTION("obb::corner()")
	{
		CHECK(obb::corner(box, box_corner::min) == vec3{ neg });
		CHECK(obb::corner(box, box_corner::x) == vec3{ pos, neg, neg });
		CHECK(obb::corner(box, box_corner::y) == vec3{ neg, pos, neg });
		CHECK(obb::corner(box, box_corner::z) == vec3{ neg, neg, pos });
		CHECK(obb::corner(box, box_corner::xy) == vec3{ pos, pos, neg });
		CHECK(obb::corner(box, box_corner::xz) == vec3{ pos, neg, pos });
		CHECK(obb::corner(box, box_corner::yz) == vec3{ neg, pos, pos });
		CHECK(obb::corner(box, box_corner::xyz) == vec3{ pos });
		CHECK(obb::corner(box, box_corner::max) == vec3{ pos });
	}

	BATCHED_SECTION("obb.corner()")
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
		CHECK(obb::min_corner(box) == vec3{ neg });
		CHECK(box.min_corner() == vec3{ neg });
	}

	BATCHED_SECTION("max_corner()")
	{
		CHECK(obb::max_corner(box) == vec3{ pos });
		CHECK(box.max_corner() == vec3{ pos });
	}
}
