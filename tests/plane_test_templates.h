// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"
#include "../include/muu/plane.h"

template <typename T, typename Func>
inline void plane_for_each(T& p, Func&& func) noexcept
{
	static_cast<Func&&>(func)(p.n.x, 0_sz);
	static_cast<Func&&>(func)(p.n.y, 1_sz);
	static_cast<Func&&>(func)(p.n.z, 2_sz);
	static_cast<Func&&>(func)(p.d, 3_sz);
}

template <typename T, typename U, typename Func>
inline void plane_for_each(T& p1, U& p2, Func&& func) noexcept
{
	static_cast<Func&&>(func)(p1.n.x, p2.n.x, 0_sz);
	static_cast<Func&&>(func)(p1.n.y, p2.n.y, 1_sz);
	static_cast<Func&&>(func)(p1.n.z, p2.n.z, 2_sz);
	static_cast<Func&&>(func)(p1.d, p2.d, 3_sz);
}

template <typename T>
inline constexpr bool plane_invoke_trait_tests = false;

template <typename T>
inline constexpr void plane_trait_tests(std::string_view /*scalar_typename*/) noexcept
{
	static_assert(sizeof(plane<T>) == sizeof(T) * 4);
	static_assert(std::is_standard_layout_v<plane<T>>);
	static_assert(std::is_trivially_constructible_v<plane<T>>);
	static_assert(std::is_trivially_copy_constructible_v<plane<T>>);
	static_assert(std::is_trivially_copy_assignable_v<plane<T>>);
	static_assert(std::is_trivially_move_constructible_v<plane<T>>);
	static_assert(std::is_trivially_move_assignable_v<plane<T>>);
	static_assert(std::is_trivially_destructible_v<plane<T>>);
	static_assert(std::is_nothrow_constructible_v<plane<T>, vector<T, 3>, vector<T, 3>, vector<T, 3>>);
	static_assert(std::is_nothrow_constructible_v<plane<T>, vector<T, 3>, vector<T, 3>>);
	static_assert(std::is_nothrow_constructible_v<plane<T>, vector<T, 3>, T>);
};

template <typename T>
struct blittable
{
	vector<T, 3> n;
	T d;
};

namespace muu
{
	template <typename T>
	inline constexpr bool allow_implicit_bit_cast<blittable<T>, plane<T>> = true;
}

template <typename T>
inline void plane_construction_tests(std::string_view scalar_typename) noexcept
{
	TEST_INFO("plane<"sv << scalar_typename << ">"sv);
	using vec = vector<T, 3>;

	{
		TEST_INFO("zero-initialization");

		const auto p = plane<T>{};
		plane_for_each(p, [](auto s, size_t) { CHECK(s == T{}); });
	}

	{
		TEST_INFO("n + d constructor");

		const auto n = vec::normalize(vec{ random_array<T, 3>() });
		const auto d = random<T>();
		const auto p = plane<T>{ n, d };
		CHECK(p.n[0] == n[0]);
		CHECK(p.n[1] == n[1]);
		CHECK(p.n[2] == n[2]);
		CHECK(p.d == d);
	}

	{
		TEST_INFO("position + direction constructor");

		const auto pos = vec{ random_array<T, 3>() };
		const auto dir = vec::normalize(vec{ random_array<T, 3>() });
		const auto p   = plane<T>{ pos, dir };
		CHECK(p.n[0] == dir[0]);
		CHECK(p.n[1] == dir[1]);
		CHECK(p.n[2] == dir[2]);
		CHECK(p.contains(pos));
	}

	{
		TEST_INFO("three points constructor");

		const auto p1 = vec{ random_array<T, 3>() };
		const auto p2 = vec{ random_array<T, 3>() };
		const auto p3 = vec{ random_array<T, 3>() };
		const auto p  = plane<T>{ p1, p2, p3 };
		CHECK(p.contains(p1));
		CHECK(p.contains(p2));
		CHECK(p.contains(p3));
	}

	{
		TEST_INFO("copy constructor");

		plane<T> p1;
		plane_for_each(p1, [](auto& s1, size_t) { s1 = random<T>(); });
		plane<T> p2{ p1 };
		plane_for_each(p1, p2, [](auto s1, auto s2, size_t) { CHECK(s1 == s2); });
	}

	{
		TEST_INFO("blitting constructor");

		blittable<T> p1;
		plane_for_each(p1, [](auto& s1, size_t) { s1 = random<T>(); });
		plane<T> p2{ p1 };
		plane_for_each(p1, p2, [](auto s1, auto s2, size_t) { CHECK(s1 == s2); });
	}
}

template <typename T>
inline void plane_equality_tests(std::string_view scalar_typename) noexcept
{
	TEST_INFO("plane<"sv << scalar_typename << ">"sv);

	plane<T> p;
	plane_for_each(p, [](auto& s, size_t) noexcept { s = random<T>(); });

	{
		TEST_INFO("same type"sv);

		plane<T> same{ p };
		CHECK_SYMMETRIC_EQUAL(p, same);
		if constexpr (is_floating_point<T>)
		{
			CHECK(plane<T>::approx_equal(p, same));
			CHECK(muu::approx_equal(p, same));
		}

		plane<T> different{ p };
		plane_for_each(different, [](auto& s, size_t) noexcept { s++; });
		CHECK_SYMMETRIC_INEQUAL(p, different);
		if constexpr (is_floating_point<T>)
		{
			CHECK_FALSE(plane<T>::approx_equal(p, different));
			CHECK_FALSE(muu::approx_equal(p, different));
		}
	}

	if constexpr (!is_floating_point<T>)
	{
		TEST_INFO("different type"sv);

		using other_scalar = std::conditional_t<std::is_same_v<T, long>, int, long>;
		using other		   = plane<other_scalar>;

		other same;
		plane_for_each(same, p, [](auto& lhs, auto& rhs, size_t) noexcept { lhs = static_cast<other_scalar>(rhs); });
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

template <typename T>
inline void plane_zero_tests(std::string_view scalar_typename) noexcept
{
	TEST_INFO("plane<"sv << scalar_typename << ">"sv);

	{
		TEST_INFO("all zeroes"sv);

		plane<T> p{};
		CHECK(p.zero());
	}

	{
		TEST_INFO("no zeroes"sv);

		plane<T> p;
		plane_for_each(p, [](auto& s, size_t) noexcept { s = random<T>(1, 10); });
		CHECK_FALSE(p.zero());
	}

	{
		TEST_INFO("some zeroes"sv);

		plane<T> p{};
		plane_for_each(p,
					   [](auto& s, size_t i) noexcept
					   {
						   if ((i % 2u))
							   s = random<T>(1, 10);
					   });
		CHECK_FALSE(p.zero());
	}

	{
		TEST_INFO("one zero"sv);
		for (size_t i = 0; i < 4; i++)
		{
			plane<T> p{};
			plane_for_each(p,
						   [=](auto& s, size_t j) noexcept
						   {
							   if (i == j)
								   s = random<T>(1, 10);
						   });
			CHECK_FALSE(p.zero());
		}
	}
}

template <typename T>
inline void plane_infinity_or_nan_tests(std::string_view scalar_typename) noexcept
{
	TEST_INFO("plane<"sv << scalar_typename << ">"sv);

	{
		TEST_INFO("all finite"sv);

		plane<T> p;
		plane_for_each(p, [](auto& s, size_t) noexcept { s = random<T>(1, 10); });
		CHECK_FALSE(p.infinity_or_nan());
		CHECK_FALSE(muu::infinity_or_nan(p));
	}

	if constexpr (is_floating_point<T>)
	{
		TEST_INFO("contains one NaN"sv);

		for (size_t i = 0; i < 4; i++)
		{
			plane<T> p{};
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

	if constexpr (is_floating_point<T>)
	{
		TEST_INFO("contains one infinity"sv);

		for (size_t i = 0; i < 4; i++)
		{
			plane<T> p{};
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

#if 0

// clang-format off

template <typename T>
inline void plane_intersects_tests(std::string_view scalar_typename) noexcept
{
	TEST_INFO("plane<"sv << scalar_typename << ">"sv);
	using vec3				= vector<T, 3>;
	using plane				= plane<T>;

	constexpr auto unit_box = plane{ constants<T>::one_over_two };

	[[maybe_unused]]
	const auto tall_box = plane{ constants<vec3>::zero, T{ 0.1 }, T{ 50 }, T{ 0.5 } };

	// these test cases were created by experimentation and manual transcription from Maya
	// please press F to pay respects

	// box x box
	{
		TEST_INFO("plane x plane"sv);

		#define CHECK_INTERSECTION(expected, x, y, z, w, h, d)                                                         \
			CHECK(expected                                                                                             \
				  == unit_box.intersects(plane(vec3{ static_cast<T>(x), static_cast<T>(y), static_cast<T>(z) },         \
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
	{
		TEST_INFO("plane x plane"sv);

		#define CHECK_INTERSECTION(box, expected, x0, y0, z0, x1, y1, z1, x2, y2, z2)                                  \
			CHECK(expected                                                                                             \
			  == box.intersects_triangle(vec3{ static_cast<T>(x0), static_cast<T>(y0), static_cast<T>(z0) },           \
											  vec3{ static_cast<T>(x1), static_cast<T>(y1), static_cast<T>(z1) },      \
											  vec3{ static_cast<T>(x2), static_cast<T>(y2), static_cast<T>(z2) }))

		CHECK_INTERSECTION(unit_box, true,	-2, 0, 2,			2, 0, 2,			-2, 0, -2);
		CHECK_INTERSECTION(unit_box, true,	-3, -1, 1,			0, -1, 3,			0, 1, 0);
		CHECK_INTERSECTION(unit_box, true,	0.3, 0.4, 0.8,		0.8, 0.4, 0.3,		0.3, 0.6, 0.3);
		CHECK_INTERSECTION(unit_box, true,	0, 0, 0.4,			0.4, 0, 0,			0, 0, 0);
		CHECK_INTERSECTION(unit_box, true,	0, 7, -0.403,		3, -2.5, -0.538,	-3, -2.5, -0.538);
		
		CHECK_INTERSECTION(unit_box, false,	0, 7, -0.403,		3, -2.5, -0.547,	-3, -2.5, -0.547);
		CHECK_INTERSECTION(unit_box, false,	-3, -1, 1,			0, -1, 3,			0, 1.162, 0);
		CHECK_INTERSECTION(unit_box, false,	-1, 0, 1,			1, 0, 1,			-1, 2.5, -1);
		CHECK_INTERSECTION(unit_box, false,	-2, 2, 2,			2, 2, 2,			-2, 2, -2);

		if constexpr (!std::is_same_v<T, half>)
		{
			CHECK_INTERSECTION(unit_box, true, 0.24255, 6.58204, 3.75181, -0.50413, 0.43569, -0.54796, -3.00000, -2.50000, -0.37540);
			CHECK_INTERSECTION(unit_box, true, 0.24255, 6.58204, 3.75181, -0.50482, 0.43583, -0.54797, -3.00000, -2.50000, -0.37540);
			
			CHECK_INTERSECTION(unit_box, false, 0.24255, 6.58204, 3.75181, -0.50633, 0.43583, -0.54797, -3.00000, -2.50000, -0.37540);

			CHECK_INTERSECTION(tall_box, true,	0, 7, 70,			2, -2, -60,			-2, -2, -60);
		}

		#undef CHECK_INTERSECTION
	}
}

// clang-format on

#endif
