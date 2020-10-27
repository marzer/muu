// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"
#include "../include/muu/quaternion.h"

template <typename T, typename Func>
inline void for_each(T& q, Func&& func) noexcept
{
	static_cast<Func&&>(func)(q.s,   0_sz);
	static_cast<Func&&>(func)(q.v.x, 1_sz);
	static_cast<Func&&>(func)(q.v.y, 2_sz);
	static_cast<Func&&>(func)(q.v.z, 3_sz);
}

template <typename T, typename U, typename Func>
inline void for_each(T& q1, U& q2, Func&& func) noexcept
{
	static_cast<Func&&>(func)(q1.s,   q2.s,   0_sz);
	static_cast<Func&&>(func)(q1.v.x, q2.v.x, 1_sz);
	static_cast<Func&&>(func)(q1.v.y, q2.v.y, 2_sz);
	static_cast<Func&&>(func)(q1.v.z, q2.v.z, 3_sz);
}

template <typename T>
inline constexpr bool invoke_trait_tests = false;

template <typename T>
inline constexpr void trait_tests(std::string_view /*scalar_typename*/) noexcept
{
	using quat_t = quaternion<T>;
	static_assert(sizeof(quat_t) == sizeof(T) * 4);
	static_assert(sizeof(quat_t[5]) == sizeof(T) * 4 * 5);
	static_assert(std::is_standard_layout_v<quat_t>);
	static_assert(std::is_trivially_constructible_v<quat_t>);
	static_assert(std::is_trivially_copy_constructible_v<quat_t>);
	static_assert(std::is_trivially_copy_assignable_v<quat_t>);
	static_assert(std::is_trivially_move_constructible_v<quat_t>);
	static_assert(std::is_trivially_move_assignable_v<quat_t>);
	static_assert(std::is_trivially_destructible_v<quat_t>);
	static_assert(std::is_nothrow_constructible_v<quat_t, T, T, T, T>);
	static_assert(std::is_nothrow_constructible_v<quat_t, T, vector<T, 3>>);

	#if MUU_HAS_VECTORCALL
	static_assert(impl::is_hva<quaternion<T>> == is_same_as_any<T, float, double, long double>);
	#endif
};

template <typename T>
inline void construction_tests(std::string_view scalar_typename) noexcept
{
	INFO("quaternion<"sv << scalar_typename << ">"sv)
	using quat_t = quaternion<T>;

	{
		INFO("scalar constructor")

		const auto vals = random_array<T, 4>();
		const auto q = quat_t{ vals[0], vals[1], vals[2], vals[3] };
		CHECK(q.s == vals[0]);
		CHECK(q.v[0] == vals[1]);
		CHECK(q.v[1] == vals[2]);
		CHECK(q.v[2] == vals[3]);
	}

	{
		INFO("scalar + vector constructor")

		const auto r = random<T>();
		const auto i = random_array<T, 3>();
		const auto q = quat_t{ r, vector{ i } };
		CHECK(q.s == r);
		CHECK(q.v[0] == i[0]);
		CHECK(q.v[1] == i[1]);
		CHECK(q.v[2] == i[2]);
	}

	{
		INFO("copy constructor")

		quat_t q1{ random<T>(), random<T>(), random<T>(), random<T>() };
		quat_t q2{ q1 };
		CHECK(q1.s == q2.s);
		CHECK(q1.v[0] == q2.v[0]);
		CHECK(q1.v[1] == q2.v[1]);
		CHECK(q1.v[2] == q2.v[2]);
	}
}

template <typename T>
inline void equality_tests(std::string_view scalar_typename) noexcept
{
	INFO("quaternion<"sv << scalar_typename << ">"sv)
	using quat_t = quaternion<T>;

	quat_t q;
	for_each(q, [](auto& s, size_t) noexcept { s = random<T>(); });
		
	{
		INFO("same type"sv)
		
		quat_t same{ q };
		CHECK_SYMMETRIC_EQUAL(q, same);
		if constexpr (is_floating_point<T>)
		{
			CHECK(quat_t::approx_equal(q, same));
			CHECK(muu::approx_equal(q, same));
		}

		quat_t different{ q };
		for_each(different, [](auto& s, size_t) noexcept { s++; });
		CHECK_SYMMETRIC_INEQUAL(q, different);
		if constexpr (is_floating_point<T>)
		{
			CHECK_FALSE(quat_t::approx_equal(q, different));
			CHECK_FALSE(muu::approx_equal(q, different));
		}
	}

	if constexpr (!is_floating_point<T>)
	{
		INFO("different type"sv)

		using other_t = quaternion<
			std::conditional_t<std::is_same_v<T, long>, int, long>
		>;

		other_t same{ q };
		CHECK_SYMMETRIC_EQUAL(q, same);

		other_t different{ q };
		for_each(different, [](auto& s, size_t) noexcept { s++; });
		CHECK_SYMMETRIC_INEQUAL(q, different);
	}
}

template <typename T>
inline void zero_tests(std::string_view scalar_typename) noexcept
{
	INFO("quaternion<"sv << scalar_typename << ">"sv)
	using quat_t = quaternion<T>;

	{
		INFO("all zeroes"sv)

		quat_t q{ T{}, T{}, T{}, T{} };
		CHECK(q.zero());
	}

	{
		INFO("no zeroes"sv)

		quat_t q { T{}, T{}, T{}, T{} };
		for_each(q, [](auto& s, size_t) noexcept { s = random<T>(1, 10); });
		CHECK_FALSE(q.zero());
	}

	{
		INFO("some zeroes"sv)

		quat_t q { T{}, T{}, T{}, T{} };
		for_each(q, [](auto& s, size_t i) noexcept
		{
			if ((i % 2u))
				s = random<T>(1, 10);
		});
		CHECK_FALSE(q.zero());
	}

	{
		INFO("one zero"sv)
		for (size_t i = 0; i < 4; i++)
		{
			quat_t q{ T{}, T{}, T{}, T{} };
			for_each(q, [=](auto& s, size_t j) noexcept
			{
				if (i == j)
					s = random<T>(1, 10);
			});
			CHECK_FALSE(q.zero());
		}
	}
}

template <typename T>
inline void infinity_or_nan_tests(std::string_view scalar_typename) noexcept
{
	INFO("quaternion<"sv << scalar_typename << ">"sv)
	using quat_t = quaternion<T>;

	{
		INFO("all finite"sv)

		quat_t q;
		for_each(q, [](auto& s, size_t) noexcept { s = random<T>(1, 10); });
		CHECK_FALSE(q.infinity_or_nan());
		CHECK_FALSE(muu::infinity_or_nan(q));
	}
	
	if constexpr (is_floating_point<T>)
	{
		INFO("contains one NaN"sv)

		for (size_t i = 0; i < 4; i++)
		{
			quat_t q{ T{}, T{}, T{}, T{} };
			for_each(q, [=](auto& s, size_t j) noexcept
			{
				if (i == j)
					s = make_nan<T>();
			});
			CHECK(q.infinity_or_nan());
			CHECK(muu::infinity_or_nan(q));
		}
	}

	if constexpr (is_floating_point<T>)
	{
		INFO("contains one infinity"sv)

		for (size_t i = 0; i < 4; i++)
		{
			quat_t q{ T{}, T{}, T{}, T{} };
			for_each(q, [=](auto& s, size_t j) noexcept
			{
				if (i == j)
					s = make_infinity<T>();
			});
			CHECK(q.infinity_or_nan());
			CHECK(muu::infinity_or_nan(q));
		}
	}
}

template <typename T>
inline void dot_tests(std::string_view scalar_typename) noexcept
{
	INFO("quaternion<"sv << scalar_typename << ">"sv)
	using quat_t = quaternion<T>;

	const quat_t q1{ random<T>(), random<T>(), random<T>(), random<T>() };
	const quat_t q2{ random<T>(), random<T>(), random<T>(), random<T>() };
		
	using dot_type = decltype(q1.dot(q2));
	static_assert(is_floating_point<dot_type>);

	// expected result
	// (accumulating in a potentially more precise intermediate type then coverting the result is what happens in
	// the quaternion class to minimize loss in float16, so that same behaviour is replicated here)
	using intermediate_type = impl::promote_if_small_float<dot_type>;
	auto expected_sum = intermediate_type{};
	for_each(q1, q2, [&](auto s1, auto s2, size_t) noexcept
	{
		MUU_FMA_BLOCK
		expected_sum += static_cast<intermediate_type>(s1) * static_cast<intermediate_type>(s2);
	});
	const auto expected = static_cast<dot_type>(expected_sum);

	CHECK_APPROX_EQUAL(q1.dot(q2), expected);
	CHECK_APPROX_EQUAL(quat_t::dot(q1, q2), expected);
	CHECK_APPROX_EQUAL(muu::dot(q1, q2), expected);
}

template <typename T>
inline void normalization_tests(std::string_view scalar_typename) noexcept
{
	INFO("quaternion<"sv << scalar_typename << ">"sv)
	using quat_t = quaternion<T>;

	const quat_t q{ random<T>(2, 10), random<T>(2, 10), random<T>(2, 10), random<T>(2, 10) };

	{
		INFO("quaternion.normalize()"sv)

		quat_t q2{ q };
		q2.normalize();
		CHECK(q2.unit_length());
		const vector<T, 4> v{ q2.s, q2.v.x, q2.v.y, q2.v.z };
		CHECK(v.unit_length());
		CHECK(v.length() == approx(T{ 1 }));
	}

	{
		INFO("quaternion::normalize(quaternion)"sv)

		const auto q2 = quat_t::normalize(q);
		CHECK(q2.unit_length());
		const vector<T, 4> v{ q2.s, q2.v.x, q2.v.y, q2.v.z };
		CHECK(v.unit_length());
		CHECK(v.length() == approx(T{ 1 }));
	}

	{
		INFO("muu::normalize(quaternion)"sv)
	
		const auto q2 = muu::normalize(q);
		CHECK(q2.unit_length());
		const vector<T, 4> v{ q2.s, q2.v.x, q2.v.y, q2.v.z };
		CHECK(v.unit_length());
		CHECK(v.length() == approx(T{ 1 }));
	}
}

template <typename T>
inline void euler_tests(std::string_view scalar_typename) noexcept
{
	INFO("quaternion<"sv << scalar_typename << ">"sv)
	using quat_t = quaternion<T>;
	using vec_t = vector<T, 3>;
	using s = constants<T>;
	using v = constants<vec_t>;

	static const std::tuple<vec_t, euler_rotation<T>, vec_t> values[] =
	{
		{ v::forward, { T{}, T{}, T{} }, v::forward },

		// positive yaws
		{ v::forward, { s::pi_over_four, T{}, T{} }, normalize(v::forward + v::right) },
		{ v::forward, { s::pi_over_two, T{}, T{} }, v::right },
		{ v::forward, { s::three_pi_over_four, T{}, T{} }, normalize(v::backward + v::right) },
		{ v::forward, { s::pi, T{}, T{} }, v::backward },
		{ v::forward, { s::pi + s::pi_over_four, T{}, T{} }, normalize(v::backward + v::left) },
		{ v::forward, { s::three_pi_over_two, T{}, T{} }, v::left },
		{ v::forward, { s::three_pi_over_two + s::pi_over_four, T{}, T{} }, normalize(v::forward + v::left) },
		{ v::forward, { s::two_pi, T{}, T{} }, v::forward },

		// negative yaws
		{ v::forward, { -s::pi_over_four, T{}, T{} }, normalize(v::forward + v::left) },
		{ v::forward, { -s::pi_over_two, T{}, T{} }, v::left },
		{ v::forward, { -s::three_pi_over_four, T{}, T{} }, normalize(v::backward + v::left) },
		{ v::forward, { -s::pi, T{}, T{} }, v::backward },
		{ v::forward, { -s::pi - s::pi_over_four, T{}, T{} }, normalize(v::backward + v::right) },
		{ v::forward, { -s::three_pi_over_two, T{}, T{} }, v::right },
		{ v::forward, { -s::three_pi_over_two - s::pi_over_four, T{}, T{} }, normalize(v::forward + v::right) },
		{ v::forward, { -s::two_pi, T{}, T{} }, v::forward },

		// positive pitches
		{ v::forward, { T{}, s::pi_over_four, T{} }, normalize(v::forward + v::up) },
		{ v::forward, { T{}, s::pi_over_two, T{} }, v::up },
		{ v::forward, { T{}, s::three_pi_over_four, T{} }, normalize(v::backward + v::up) },
		{ v::forward, { T{}, s::pi, T{} }, v::backward },
		{ v::forward, { T{}, s::pi + s::pi_over_four, T{} }, normalize(v::backward + v::down) },
		{ v::forward, { T{}, s::three_pi_over_two, T{} }, v::down },
		{ v::forward, { T{}, s::three_pi_over_two + s::pi_over_four, T{} }, normalize(v::forward + v::down) },
		{ v::forward, { T{}, s::two_pi, T{} }, v::forward },

		// negative pitches
		{ v::forward, { T{}, -s::pi_over_four, T{} }, normalize(v::forward + v::down) },
		{ v::forward, { T{}, -s::pi_over_two, T{} }, v::down },
		{ v::forward, { T{}, -s::three_pi_over_four, T{} }, normalize(v::backward + v::down) },
		{ v::forward, { T{}, -s::pi, T{} }, v::backward },
		{ v::forward, { T{}, -s::pi - s::pi_over_four, T{} }, normalize(v::backward + v::up) },
		{ v::forward, { T{}, -s::three_pi_over_two, T{} }, v::up },
		{ v::forward, { T{}, -s::three_pi_over_two - s::pi_over_four, T{} }, normalize(v::forward + v::up) },
		{ v::forward, { T{}, -s::two_pi, T{} }, v::forward },

		// positive rolls
		{ v::right, { T{}, T{}, s::pi_over_four }, normalize(v::right + v::down) },
		{ v::right, { T{}, T{}, s::pi_over_two }, v::down },
		{ v::right, { T{}, T{}, s::three_pi_over_four }, normalize(v::left + v::down) },
		{ v::right, { T{}, T{}, s::pi }, v::left },
		{ v::right, { T{}, T{}, s::pi + s::pi_over_four }, normalize(v::left + v::up) },
		{ v::right, { T{}, T{}, s::three_pi_over_two }, v::up },
		{ v::right, { T{}, T{}, s::three_pi_over_two + s::pi_over_four }, normalize(v::right + v::up) },
		{ v::right, { T{}, T{}, s::two_pi }, v::right },
	};

	for (const auto& [input_dir, rot, expected_dir] : values)
	{
		const auto quat = quat_t::from_euler(rot);
		CHECK(!infinity_or_nan(quat));

		const auto dir = normalize(input_dir * quat);
		CHECK(!infinity_or_nan(dir));

		if (!approx_equal(dir, expected_dir, static_cast<T>(s::approx_equal_epsilon * 4)))
		{
			INFO("   input direction: " << input_dir)
			INFO("    input rotation: " << rot)
			INFO("        quaternion: " << quat)
			INFO("  output direction: " << dir)
			INFO("expected direction: " << expected_dir)
			FAIL_CHECK("output mismatch!");
			break;
		}
		else
			SUCCEED(); // increment assertion counter
	}

}
