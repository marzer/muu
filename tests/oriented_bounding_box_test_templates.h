// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"
#include "../include/muu/oriented_bounding_box.h"

template <typename T, typename Func>
inline void obb_for_each(T& bb, Func&& func) noexcept
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
inline void obb_for_each(T& bb1, U& bb2, Func&& func) noexcept
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

template <typename T>
inline constexpr bool obb_invoke_trait_tests = false;

template <typename T>
inline constexpr void obb_trait_tests(std::string_view /*scalar_typename*/) noexcept
{
	using obb = oriented_bounding_box<T>;
	static_assert(sizeof(obb) == (sizeof(vector<T, 3>) * 2 + sizeof(matrix<T, 3, 3>)));
	static_assert(sizeof(obb[5]) == (sizeof(vector<T, 3>) * 2 + sizeof(matrix<T, 3, 3>)) * 5);
	static_assert(std::is_standard_layout_v<obb>);
	static_assert(std::is_trivially_constructible_v<obb>);
	static_assert(std::is_trivially_copy_constructible_v<obb>);
	static_assert(std::is_trivially_copy_assignable_v<obb>);
	static_assert(std::is_trivially_move_constructible_v<obb>);
	static_assert(std::is_trivially_move_assignable_v<obb>);
	static_assert(std::is_trivially_destructible_v<obb>);
	static_assert(std::is_nothrow_constructible_v<obb, vector<T, 3>, vector<T, 3>>);
	static_assert(std::is_nothrow_constructible_v<obb, vector<T, 3>, T, T, T>);
	static_assert(std::is_nothrow_constructible_v<obb, vector<T, 3>, T>);
	static_assert(std::is_nothrow_constructible_v<obb, vector<T, 3>>);
	static_assert(std::is_nothrow_constructible_v<obb, T, T, T, vector<T, 3>>);
	static_assert(std::is_nothrow_constructible_v<obb, T, T, T, T, T, T>);
	static_assert(std::is_nothrow_constructible_v<obb, T>);
};

template <typename T>
struct blittable
{
	vector<T, 3> center;
	vector<T, 3> extents;
	matrix<T, 3, 3> axes;
};

namespace muu
{
	template <typename T>
	inline constexpr bool allow_implicit_bit_cast<blittable<T>, oriented_bounding_box<T>> = true;
}

template <typename T>
inline void obb_construction_tests(std::string_view scalar_typename) noexcept
{
	INFO("oriented_bounding_box<"sv << scalar_typename << ">"sv)
	using obb = oriented_bounding_box<T>;
	using vec = vector<T, 3>;
	using mat = matrix<T, 3, 3>;

	{
		INFO("zero-initialization")

		const auto bb = obb{};
		obb_for_each(bb, [](auto s, size_t)
		{
			CHECK(s == T{});
		});
	}

	{
		INFO("vector + vector constructor")

		const auto c = random_array<T, 3>();
		const auto e = random_array<T, 3>();
		const auto bb = obb{ vec{ c }, vec{ e } };
		CHECK(bb.center[0] == c[0]);
		CHECK(bb.center[1] == c[1]);
		CHECK(bb.center[2] == c[2]);
		CHECK(bb.extents[0] == e[0]);
		CHECK(bb.extents[1] == e[1]);
		CHECK(bb.extents[2] == e[2]);
		CHECK(bb.axes == mat::constants::identity);
	}

	{
		INFO("vector + 3 scalars constructor")

		const auto c = random_array<T, 3>();
		const auto e = random_array<T, 3>();
		const auto bb = obb{ vec{ c }, e[0], e[1], e[2] };
		CHECK(bb.center[0] == c[0]);
		CHECK(bb.center[1] == c[1]);
		CHECK(bb.center[2] == c[2]);
		CHECK(bb.extents[0] == e[0]);
		CHECK(bb.extents[1] == e[1]);
		CHECK(bb.extents[2] == e[2]);
		CHECK(bb.axes == mat::constants::identity);
	}

	{
		INFO("vector + 1 scalar constructor")

		const auto c = random_array<T, 3>();
		const auto e = random<T>();
		const auto bb = obb{ vec{ c }, e };
		CHECK(bb.center[0] == c[0]);
		CHECK(bb.center[1] == c[1]);
		CHECK(bb.center[2] == c[2]);
		CHECK(bb.extents[0] == e);
		CHECK(bb.extents[1] == e);
		CHECK(bb.extents[2] == e);
		CHECK(bb.axes == mat::constants::identity);
	}

	{
		INFO("vector constructor")

		const auto e = random_array<T, 3>();
		const auto bb = obb{ vec{ e } };
		CHECK(bb.center[0] == T{});
		CHECK(bb.center[1] == T{});
		CHECK(bb.center[2] == T{});
		CHECK(bb.extents[0] == e[0]);
		CHECK(bb.extents[1] == e[1]);
		CHECK(bb.extents[2] == e[2]);
		CHECK(bb.axes == mat::constants::identity);
	}

	{
		INFO("3 scalars + vector constructor")

		const auto c = random_array<T, 3>();
		const auto e = random_array<T, 3>();
		const auto bb = obb{ c[0], c[1], c[2], vec{ e } };
		CHECK(bb.center[0] == c[0]);
		CHECK(bb.center[1] == c[1]);
		CHECK(bb.center[2] == c[2]);
		CHECK(bb.extents[0] == e[0]);
		CHECK(bb.extents[1] == e[1]);
		CHECK(bb.extents[2] == e[2]);
		CHECK(bb.axes == mat::constants::identity);
	}

	{
		INFO("6 scalars constructor")

		const auto vals = random_array<T, 6>();
		const auto bb = obb{ vals[0], vals[1], vals[2], vals[3], vals[4], vals[5] };
		CHECK(bb.center[0] == vals[0]);
		CHECK(bb.center[1] == vals[1]);
		CHECK(bb.center[2] == vals[2]);
		CHECK(bb.extents[0] == vals[3]);
		CHECK(bb.extents[1] == vals[4]);
		CHECK(bb.extents[2] == vals[5]);
		CHECK(bb.axes == mat::constants::identity);
	}

	{
		INFO("scalar constructor")

		const auto e = random<T>();
		const auto bb = obb{ e };
		CHECK(bb.center[0] == T{});
		CHECK(bb.center[1] == T{});
		CHECK(bb.center[2] == T{});
		CHECK(bb.extents[0] == e);
		CHECK(bb.extents[1] == e);
		CHECK(bb.extents[2] == e);
		CHECK(bb.axes == mat::constants::identity);
	}

	{
		INFO("copy constructor")

		obb bb1;
		obb_for_each(bb1,[](auto& s1, size_t) { s1 = random<T>(); });
		obb bb2{ bb1 };
		obb_for_each(bb1, bb2, [](auto s1, auto s2, size_t)
		{
			CHECK(s1 == s2);
		});
	}

	{
		INFO("blitting constructor")

		blittable<T> bb1;
		obb_for_each(bb1, [](auto& s1, size_t) { s1 = random<T>(); });
		obb bb2{ bb1 };
		obb_for_each(bb1, bb2, [](auto s1, auto s2, size_t)
		{
			CHECK(s1 == s2);
		});
	}
}

template <typename T>
inline void obb_equality_tests(std::string_view scalar_typename) noexcept
{
	INFO("oriented_bounding_box<"sv << scalar_typename << ">"sv)
	using obb = oriented_bounding_box<T>;

	obb bb;
	obb_for_each(bb, [](auto& s, size_t) noexcept { s = random<T>(); });
		
	{
		INFO("same type"sv)
		
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
		INFO("different type"sv)

		using other_scalar = std::conditional_t<std::is_same_v<T, long>, int, long>;
		using other = oriented_bounding_box<other_scalar>;

		other same;
		obb_for_each(same, bb, [](auto& lhs, auto& rhs, size_t) noexcept
		{
			lhs = static_cast<other_scalar>(rhs);
		});
		CHECK_SYMMETRIC_EQUAL(bb, same);

		other different;
		obb_for_each(different, bb, [](auto& lhs, auto& rhs, size_t) noexcept
		{
			lhs = static_cast<other_scalar>(rhs);
			lhs++;
		});
		CHECK_SYMMETRIC_INEQUAL(bb, different);
	}
}

template <typename T>
inline void obb_zero_tests(std::string_view scalar_typename) noexcept
{
	INFO("oriented_bounding_box<"sv << scalar_typename << ">"sv)
	using obb = oriented_bounding_box<T>;

	{
		INFO("all zeroes"sv)

		obb bb{};
		CHECK(bb.zero());
	}

	{
		INFO("no zeroes"sv)

		obb bb;
		obb_for_each(bb, [](auto& s, size_t) noexcept { s = random<T>(1, 10); });
		CHECK_FALSE(bb.zero());
	}

	{
		INFO("some zeroes"sv)

		obb bb {};
		obb_for_each(bb, [](auto& s, size_t i) noexcept
		{
			if ((i % 2u))
				s = random<T>(1, 10);
		});
		CHECK_FALSE(bb.zero());
	}

	{
		INFO("one zero"sv)
		for (size_t i = 0; i < 4; i++)
		{
			obb bb{};
			obb_for_each(bb, [=](auto& s, size_t j) noexcept
			{
				if (i == j)
					s = random<T>(1, 10);
			});
			CHECK_FALSE(bb.zero());
		}
	}
}

template <typename T>
inline void obb_infinity_or_nan_tests(std::string_view scalar_typename) noexcept
{
	INFO("oriented_bounding_box<"sv << scalar_typename << ">"sv)
	using obb = oriented_bounding_box<T>;

	{
		INFO("all finite"sv)

		obb bb;
		obb_for_each(bb, [](auto& s, size_t) noexcept { s = random<T>(1, 10); });
		CHECK_FALSE(bb.infinity_or_nan());
		CHECK_FALSE(muu::infinity_or_nan(bb));
	}
	
	if constexpr (is_floating_point<T>)
	{
		INFO("contains one NaN"sv)

		for (size_t i = 0; i < 4; i++)
		{
			obb bb{};
			obb_for_each(bb, [=](auto& s, size_t j) noexcept
			{
				if (i == j)
					s = make_nan<T>();
			});
			CHECK(bb.infinity_or_nan());
			CHECK(muu::infinity_or_nan(bb));
		}
	}

	if constexpr (is_floating_point<T>)
	{
		INFO("contains one infinity"sv)

		for (size_t i = 0; i < 4; i++)
		{
			obb bb{};
			obb_for_each(bb, [=](auto& s, size_t j) noexcept
			{
				if (i == j)
					s = make_infinity<T>();
			});
			CHECK(bb.infinity_or_nan());
			CHECK(muu::infinity_or_nan(bb));
		}
	}
}
