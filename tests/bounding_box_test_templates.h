// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"
#include "../include/muu/bounding_box.h"

template <typename T, typename Func>
inline void aabb_for_each(T& bb, Func&& func) noexcept
{
	size_t idx{};
	for (size_t i = 0; i < 3; i++)
		static_cast<Func&&>(func)(bb.center[i], idx++);
	for (size_t i = 0; i < 3; i++)
		static_cast<Func&&>(func)(bb.extents[i], idx++);
}

template <typename T, typename U, typename Func>
inline void aabb_for_each(T& bb1, U& bb2, Func&& func) noexcept
{
	static_cast<Func&&>(func)(bb1.center.x, bb2.center.x, 0_sz);
	static_cast<Func&&>(func)(bb1.center.y, bb2.center.y, 1_sz);
	static_cast<Func&&>(func)(bb1.center.z, bb2.center.z, 2_sz);
	static_cast<Func&&>(func)(bb1.extents.x, bb2.extents.x, 3_sz);
	static_cast<Func&&>(func)(bb1.extents.y, bb2.extents.y, 4_sz);
	static_cast<Func&&>(func)(bb1.extents.z, bb2.extents.z, 5_sz);
}

template <typename T>
inline constexpr bool aabb_invoke_trait_tests = false;

template <typename T>
inline constexpr void aabb_trait_tests(std::string_view /*scalar_typename*/) noexcept
{
	using aabb = bounding_box<T>;
	static_assert(sizeof(aabb) == sizeof(vector<T, 3>) * 2);
	static_assert(sizeof(aabb[5]) == sizeof(vector<T, 3>) * 2 * 5);
	static_assert(std::is_standard_layout_v<aabb>);
	static_assert(std::is_trivially_constructible_v<aabb>);
	static_assert(std::is_trivially_copy_constructible_v<aabb>);
	static_assert(std::is_trivially_copy_assignable_v<aabb>);
	static_assert(std::is_trivially_move_constructible_v<aabb>);
	static_assert(std::is_trivially_move_assignable_v<aabb>);
	static_assert(std::is_trivially_destructible_v<aabb>);
	static_assert(std::is_nothrow_constructible_v<aabb, vector<T, 3>, vector<T, 3>>);
	static_assert(std::is_nothrow_constructible_v<aabb, vector<T, 3>, T, T, T>);
	static_assert(std::is_nothrow_constructible_v<aabb, vector<T, 3>, T>);
	static_assert(std::is_nothrow_constructible_v<aabb, vector<T, 3>>);
	static_assert(std::is_nothrow_constructible_v<aabb, T, T, T, vector<T, 3>>);
	static_assert(std::is_nothrow_constructible_v<aabb, T, T, T, T, T, T>);
	static_assert(std::is_nothrow_constructible_v<aabb, T>);
};

template <typename T>
struct blittable
{
	vector<T, 3> center;
	vector<T, 3> extents;
};

namespace muu
{
	template <typename T>
	inline constexpr bool allow_implicit_bit_cast<blittable<T>, bounding_box<T>> = true;
}

template <typename T>
inline void aabb_construction_tests(std::string_view scalar_typename) noexcept
{
	INFO("bounding_box<"sv << scalar_typename << ">"sv)
	using aabb = bounding_box<T>;
	using vec = vector<T, 3>;

	{
		INFO("zero-initialization")

		const auto bb = aabb{};
		aabb_for_each(bb, [](auto s, size_t)
		{
			CHECK(s == T{});
		});
	}

	{
		INFO("vector + vector constructor")

		const auto c = random_array<T, 3>();
		const auto e = random_array<T, 3>();
		const auto bb = aabb{ vec{ c }, vec{ e } };
		CHECK(bb.center[0] == c[0]);
		CHECK(bb.center[1] == c[1]);
		CHECK(bb.center[2] == c[2]);
		CHECK(bb.extents[0] == e[0]);
		CHECK(bb.extents[1] == e[1]);
		CHECK(bb.extents[2] == e[2]);
	}

	{
		INFO("vector + 3 scalars constructor")

		const auto c = random_array<T, 3>();
		const auto e = random_array<T, 3>();
		const auto bb = aabb{ vec{ c }, e[0], e[1], e[2] };
		CHECK(bb.center[0] == c[0]);
		CHECK(bb.center[1] == c[1]);
		CHECK(bb.center[2] == c[2]);
		CHECK(bb.extents[0] == e[0]);
		CHECK(bb.extents[1] == e[1]);
		CHECK(bb.extents[2] == e[2]);
	}

	{
		INFO("vector + 1 scalar constructor")

		const auto c = random_array<T, 3>();
		const auto e = random<T>();
		const auto bb = aabb{ vec{ c }, e };
		CHECK(bb.center[0] == c[0]);
		CHECK(bb.center[1] == c[1]);
		CHECK(bb.center[2] == c[2]);
		CHECK(bb.extents[0] == e);
		CHECK(bb.extents[1] == e);
		CHECK(bb.extents[2] == e);
	}

	{
		INFO("vector constructor")

		const auto e = random_array<T, 3>();
		const auto bb = aabb{ vec{ e } };
		CHECK(bb.center[0] == T{});
		CHECK(bb.center[1] == T{});
		CHECK(bb.center[2] == T{});
		CHECK(bb.extents[0] == e[0]);
		CHECK(bb.extents[1] == e[1]);
		CHECK(bb.extents[2] == e[2]);
	}

	{
		INFO("3 scalars + vector constructor")

		const auto c = random_array<T, 3>();
		const auto e = random_array<T, 3>();
		const auto bb = aabb{ c[0], c[1], c[2], vec{ e } };
		CHECK(bb.center[0] == c[0]);
		CHECK(bb.center[1] == c[1]);
		CHECK(bb.center[2] == c[2]);
		CHECK(bb.extents[0] == e[0]);
		CHECK(bb.extents[1] == e[1]);
		CHECK(bb.extents[2] == e[2]);
	}

	{
		INFO("6 scalars constructor")

		const auto vals = random_array<T, 6>();
		const auto bb = aabb{ vals[0], vals[1], vals[2], vals[3], vals[4], vals[5] };
		CHECK(bb.center[0] == vals[0]);
		CHECK(bb.center[1] == vals[1]);
		CHECK(bb.center[2] == vals[2]);
		CHECK(bb.extents[0] == vals[3]);
		CHECK(bb.extents[1] == vals[4]);
		CHECK(bb.extents[2] == vals[5]);
	}

	{
		INFO("scalar constructor")

		const auto e = random<T>();
		const auto bb = aabb{ e };
		CHECK(bb.center[0] == T{});
		CHECK(bb.center[1] == T{});
		CHECK(bb.center[2] == T{});
		CHECK(bb.extents[0] == e);
		CHECK(bb.extents[1] == e);
		CHECK(bb.extents[2] == e);
	}

	{
		INFO("copy constructor")

		aabb bb1;
		aabb_for_each(bb1, [](auto& s1, size_t) { s1 = random<T>(); });
		aabb bb2{ bb1 };
		aabb_for_each(bb1, bb2, [](auto s1, auto s2, size_t)
		{
			CHECK(s1 == s2);
		});
	}

	{
		INFO("blitting constructor")

		blittable<T> bb1;
		aabb_for_each(bb1, [](auto& s1, size_t) { s1 = random<T>(); });
		aabb bb2{ bb1 };
		aabb_for_each(bb1, bb2, [](auto s1, auto s2, size_t)
		{
			CHECK(s1 == s2);
		});
	}
}

template <typename T>
inline void aabb_equality_tests(std::string_view scalar_typename) noexcept
{
	INFO("bounding_box<"sv << scalar_typename << ">"sv)
	using aabb = bounding_box<T>;

	aabb bb;
	aabb_for_each(bb, [](auto& s, size_t) noexcept { s = random<T>(); });
		
	{
		INFO("same type"sv)
		
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
		INFO("different type"sv)

		using other_scalar = std::conditional_t<std::is_same_v<T, long>, int, long>;
		using other = bounding_box<other_scalar>;

		other same;
		aabb_for_each(same, bb, [](auto& lhs, auto& rhs, size_t) noexcept
		{
			lhs = static_cast<other_scalar>(rhs);
		});
		CHECK_SYMMETRIC_EQUAL(bb, same);

		other different;
		aabb_for_each(different, bb, [](auto& lhs, auto& rhs, size_t) noexcept
		{
			lhs = static_cast<other_scalar>(rhs);
			lhs++;
		});
		CHECK_SYMMETRIC_INEQUAL(bb, different);
	}
}

template <typename T>
inline void aabb_zero_tests(std::string_view scalar_typename) noexcept
{
	INFO("bounding_box<"sv << scalar_typename << ">"sv)
	using aabb = bounding_box<T>;

	{
		INFO("all zeroes"sv)

		aabb bb{};
		CHECK(bb.zero());
	}

	{
		INFO("no zeroes"sv)

		aabb bb;
		aabb_for_each(bb, [](auto& s, size_t) noexcept { s = random<T>(1, 10); });
		CHECK_FALSE(bb.zero());
	}

	{
		INFO("some zeroes"sv)

		aabb bb{};
		aabb_for_each(bb, [](auto& s, size_t i) noexcept
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
			aabb bb{};
			aabb_for_each(bb, [=](auto& s, size_t j) noexcept
			{
				if (i == j)
					s = random<T>(1, 10);
			});
			CHECK_FALSE(bb.zero());
		}
	}
}

template <typename T>
inline void aabb_infinity_or_nan_tests(std::string_view scalar_typename) noexcept
{
	INFO("bounding_box<"sv << scalar_typename << ">"sv)
	using aabb = bounding_box<T>;

	{
		INFO("all finite"sv)

		aabb bb;
		aabb_for_each(bb, [](auto& s, size_t) noexcept { s = random<T>(1, 10); });
		CHECK_FALSE(bb.infinity_or_nan());
		CHECK_FALSE(muu::infinity_or_nan(bb));
	}
	
	if constexpr (is_floating_point<T>)
	{
		INFO("contains one NaN"sv)

		for (size_t i = 0; i < 4; i++)
		{
			aabb bb{};
			aabb_for_each(bb, [=](auto& s, size_t j) noexcept
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
			aabb bb{};
			aabb_for_each(bb, [=](auto& s, size_t j) noexcept
			{
				if (i == j)
					s = make_infinity<T>();
			});
			CHECK(bb.infinity_or_nan());
			CHECK(muu::infinity_or_nan(bb));
		}
	}
}