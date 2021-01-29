// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"
#include "../include/muu/quaternion.h"

///////////////////////////////////////////////////////////////////////////////////
// Some runtime tests in this file adapted from
// 1) https://github.com/google/mathfu/blob/master/unit_tests/quaternion_test/quaternion_test.cpp
///////////////////////////////////////////////////////////////////////////////////

template <typename T, typename Func>
inline void quat_for_each(T& q, Func&& func) noexcept
{
	static_cast<Func&&>(func)(q.s,   0_sz);
	static_cast<Func&&>(func)(q.v.x, 1_sz);
	static_cast<Func&&>(func)(q.v.y, 2_sz);
	static_cast<Func&&>(func)(q.v.z, 3_sz);
}

template <typename T, typename U, typename Func>
inline void quat_for_each(T& q1, U& q2, Func&& func) noexcept
{
	static_cast<Func&&>(func)(q1.s,   q2.s,   0_sz);
	static_cast<Func&&>(func)(q1.v.x, q2.v.x, 1_sz);
	static_cast<Func&&>(func)(q1.v.y, q2.v.y, 2_sz);
	static_cast<Func&&>(func)(q1.v.z, q2.v.z, 3_sz);
}

template <typename T>
inline constexpr bool quat_invoke_trait_tests = false;

template <typename T>
inline constexpr void quat_trait_tests(std::string_view /*scalar_typename*/) noexcept
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
	static_assert(impl::is_hva<quat_t> == is_same_as_any<T, float, double, long double>);
	#endif
};

template <typename T>
struct blittable
{
	T s;
	vector<T, 3> v;
};

namespace muu
{
	template <typename T>
	inline constexpr bool allow_implicit_bit_cast<blittable<T>, quaternion<T>> = true;
}

template <typename T>
inline void quat_construction_tests(std::string_view scalar_typename) noexcept
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

	{
		INFO("blitting constructor")

		blittable<T> q1{ random<T>(), { random<T>(), random<T>(), random<T>() } };
		quat_t q2{ q1 };
		CHECK(q1.s == q2.s);
		CHECK(q1.v[0] == q2.v[0]);
		CHECK(q1.v[1] == q2.v[1]);
		CHECK(q1.v[2] == q2.v[2]);
	}
}

template <typename T>
inline void quat_equality_tests(std::string_view scalar_typename) noexcept
{
	INFO("quaternion<"sv << scalar_typename << ">"sv)
	using quat_t = quaternion<T>;

	quat_t q;
	quat_for_each(q, [](auto& s, size_t) noexcept { s = random<T>(); });
		
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
		quat_for_each(different, [](auto& s, size_t) noexcept { s++; });
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
		quat_for_each(different, [](auto& s, size_t) noexcept { s++; });
		CHECK_SYMMETRIC_INEQUAL(q, different);
	}
}

template <typename T>
inline void quat_zero_tests(std::string_view scalar_typename) noexcept
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
		quat_for_each(q, [](auto& s, size_t) noexcept { s = random<T>(1, 10); });
		CHECK_FALSE(q.zero());
	}

	{
		INFO("some zeroes"sv)

		quat_t q { T{}, T{}, T{}, T{} };
		quat_for_each(q, [](auto& s, size_t i) noexcept
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
			quat_for_each(q, [=](auto& s, size_t j) noexcept
			{
				if (i == j)
					s = random<T>(1, 10);
			});
			CHECK_FALSE(q.zero());
		}
	}
}

template <typename T>
inline void quat_infinity_or_nan_tests(std::string_view scalar_typename) noexcept
{
	INFO("quaternion<"sv << scalar_typename << ">"sv)
	using quat_t = quaternion<T>;

	{
		INFO("all finite"sv)

		quat_t q;
		quat_for_each(q, [](auto& s, size_t) noexcept { s = random<T>(1, 10); });
		CHECK_FALSE(q.infinity_or_nan());
		CHECK_FALSE(muu::infinity_or_nan(q));
	}
	
	if constexpr (is_floating_point<T>)
	{
		INFO("contains one NaN"sv)

		for (size_t i = 0; i < 4; i++)
		{
			quat_t q{ T{}, T{}, T{}, T{} };
			quat_for_each(q, [=](auto& s, size_t j) noexcept
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
			quat_for_each(q, [=](auto& s, size_t j) noexcept
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
inline void quat_dot_tests(std::string_view scalar_typename) noexcept
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
	quat_for_each(q1, q2, [&](auto s1, auto s2, size_t) noexcept
	{
		MUU_FMA_BLOCK;
		expected_sum += static_cast<intermediate_type>(s1) * static_cast<intermediate_type>(s2);
	});
	const auto expected = static_cast<dot_type>(expected_sum);

	CHECK_APPROX_EQUAL(q1.dot(q2), expected);
	CHECK_APPROX_EQUAL(quat_t::dot(q1, q2), expected);
	CHECK_APPROX_EQUAL(muu::dot(q1, q2), expected);
}

template <typename T>
inline void quat_normalization_tests(std::string_view scalar_typename) noexcept
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
inline void quat_euler_tests(std::string_view scalar_typename) noexcept
{
	INFO("quaternion<"sv << scalar_typename << ">"sv)
	using quat_t = quaternion<T>;
	using vec_t = vector<T, 3>;
	using s = constants<T>;
	using v = constants<vec_t>;

	static const std::tuple<vec_t, euler_rotation<T>, vec_t> values[] =
	{
		/*  0 */ { v::forward, { T{}, T{}, T{} }, v::forward },

		// positive yaws
		/*  1 */ { v::forward, { s::pi_over_four, T{}, T{} }, normalize(v::forward + v::right) },
		/*  2 */ { v::forward, { s::pi_over_two, T{}, T{} }, v::right },
		/*  3 */ { v::forward, { s::three_pi_over_four, T{}, T{} }, normalize(v::backward + v::right) },
		/*  4 */ { v::forward, { s::pi, T{}, T{} }, v::backward },
		/*  5 */ { v::forward, { s::pi + s::pi_over_four, T{}, T{} }, normalize(v::backward + v::left) },
		/*  6 */ { v::forward, { s::three_pi_over_two, T{}, T{} }, v::left },
		/*  7 */ { v::forward, { s::three_pi_over_two + s::pi_over_four, T{}, T{} }, normalize(v::forward + v::left) },
		/*  8 */ { v::forward, { s::two_pi, T{}, T{} }, v::forward },

		// negative yaws
		/*  9 */ { v::forward, { -s::pi_over_four, T{}, T{} }, normalize(v::forward + v::left) },
		/* 10 */ { v::forward, { -s::pi_over_two, T{}, T{} }, v::left },
		/* 11 */ { v::forward, { -s::three_pi_over_four, T{}, T{} }, normalize(v::backward + v::left) },
		/* 12 */ { v::forward, { -s::pi, T{}, T{} }, v::backward },
		/* 13 */ { v::forward, { -s::pi - s::pi_over_four, T{}, T{} }, normalize(v::backward + v::right) },
		/* 14 */ { v::forward, { -s::three_pi_over_two, T{}, T{} }, v::right },
		/* 15 */ { v::forward, { -s::three_pi_over_two - s::pi_over_four, T{}, T{} }, normalize(v::forward + v::right) },
		/* 16 */ { v::forward, { -s::two_pi, T{}, T{} }, v::forward },

		// positive pitches
		/* 17 */ { v::forward, { T{}, s::pi_over_four, T{} }, normalize(v::forward + v::up) },
		/* 18 */ { v::forward, { T{}, s::pi_over_two, T{} }, v::up },
		/* 19 */ { v::forward, { T{}, s::three_pi_over_four, T{} }, normalize(v::backward + v::up) },
		/* 20 */ { v::forward, { T{}, s::pi, T{} }, v::backward },
		/* 21 */ { v::forward, { T{}, s::pi + s::pi_over_four, T{} }, normalize(v::backward + v::down) },
		/* 22 */ { v::forward, { T{}, s::three_pi_over_two, T{} }, v::down },
		/* 23 */ { v::forward, { T{}, s::three_pi_over_two + s::pi_over_four, T{} }, normalize(v::forward + v::down) },
		/* 24 */ { v::forward, { T{}, s::two_pi, T{} }, v::forward },

		// negative pitches
		/* 25 */ { v::forward, { T{}, -s::pi_over_four, T{} }, normalize(v::forward + v::down) },
		/* 26 */ { v::forward, { T{}, -s::pi_over_two, T{} }, v::down },
		/* 27 */ { v::forward, { T{}, -s::three_pi_over_four, T{} }, normalize(v::backward + v::down) },
		/* 28 */ { v::forward, { T{}, -s::pi, T{} }, v::backward },
		/* 29 */ { v::forward, { T{}, -s::pi - s::pi_over_four, T{} }, normalize(v::backward + v::up) },
		/* 30 */ { v::forward, { T{}, -s::three_pi_over_two, T{} }, v::up },
		/* 31 */ { v::forward, { T{}, -s::three_pi_over_two - s::pi_over_four, T{} }, normalize(v::forward + v::up) },
		/* 32 */ { v::forward, { T{}, -s::two_pi, T{} }, v::forward },

		// positive rolls
		/* 33 */ { v::right, { T{}, T{}, s::pi_over_four }, normalize(v::right + v::down) },
		/* 34 */ { v::right, { T{}, T{}, s::pi_over_two }, v::down },
		/* 35 */ { v::right, { T{}, T{}, s::three_pi_over_four }, normalize(v::left + v::down) },
		/* 36 */ { v::right, { T{}, T{}, s::pi }, v::left },
		/* 37 */ { v::right, { T{}, T{}, s::pi + s::pi_over_four }, normalize(v::left + v::up) },
		/* 38 */ { v::right, { T{}, T{}, s::three_pi_over_two }, v::up },
		/* 39 */ { v::right, { T{}, T{}, s::three_pi_over_two + s::pi_over_four }, normalize(v::right + v::up) },
		/* 40 */ { v::right, { T{}, T{}, s::two_pi }, v::right },

		// negative rolls
		/* 41 */ { v::right, { T{}, T{}, -s::pi_over_four }, normalize(v::right + v::up) },
		/* 42 */ { v::right, { T{}, T{}, -s::pi_over_two }, v::up },
		/* 43 */ { v::right, { T{}, T{}, -s::three_pi_over_four }, normalize(v::left + v::up) },
		/* 44 */ { v::right, { T{}, T{}, -s::pi }, v::left },
		/* 45 */ { v::right, { T{}, T{}, -s::pi - s::pi_over_four }, normalize(v::left + v::down) },
		/* 46 */ { v::right, { T{}, T{}, -s::three_pi_over_two }, v::down },
		/* 47 */ { v::right, { T{}, T{}, -s::three_pi_over_two - s::pi_over_four }, normalize(v::right + v::down) },
		/* 48 */ { v::right, { T{}, T{}, -s::two_pi }, v::right },
	};

	size_t counter = 0;
	for (const auto& [input_dir, rot, expected_dir] : values)
	{
		const auto i = counter++;
		INFO("test case " << i)

		const auto quat = quat_t::from_euler(rot);
		CHECK(!infinity_or_nan(quat));

		const auto dir = normalize(input_dir * quat);
		CHECK(!infinity_or_nan(dir));

		if (!approx_equal(dir, expected_dir, static_cast<T>(s::default_epsilon * 4)))
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

		// don't test round-trips at the pitch singularities because it makes no sense
		if (i == 18u || i == 22u || i == 26u || i == 30u)
			continue;

		// round-trip
		const auto rot2 = quat_t::to_euler(quat);
		const auto quat2 = quat_t::from_euler(rot2);
		CHECK(!infinity_or_nan(quat2));

		const auto dir2 = normalize(input_dir * quat2);
		CHECK(!infinity_or_nan(dir2));

		if (!approx_equal(dir2, expected_dir, static_cast<T>(s::default_epsilon * 4)))
		{
			INFO("    input direction: " << input_dir)
			INFO("     input rotation: " << rot)
			INFO("output quaternion 1: " << quat)
			INFO("  output rotation 1: " << rot2)
			INFO("output quaternion 2: " << quat2)
			INFO("   output direction: " << dir2)
			INFO(" expected direction: " << expected_dir)
			FAIL_CHECK("output mismatch! (round-tripped)");
			break;
		}
		else
			SUCCEED(); // increment assertion counter
	}
}

template <typename T>
inline void quat_conjugate_tests(std::string_view scalar_typename) noexcept
{
	INFO("quaternion<"sv << scalar_typename << ">"sv)
	using quat_t = quaternion<T>;

	// a quaternion multiplied by it's conjugate should be equivalent to an euler rotation of {0,0,0}.
	quat_t q{
		static_cast<T>(1.4),
		static_cast<T>(6.3),
		static_cast<T>(8.5),
		static_cast<T>(5.9)
	};
	q.normalize();

	const auto conjugate = q.conjugate();
	CHECK(conjugate.s == q.s);
	CHECK(conjugate.v == -q.v);

	const auto euler = (conjugate * q).to_euler();
	CHECK_APPROX_EQUAL(euler.yaw, T{});
	CHECK_APPROX_EQUAL(euler.pitch, T{});
	CHECK_APPROX_EQUAL(euler.roll, T{});
}

#define CHECK_APPROX_EQUAL_ORIENTATIONS(q1_, q2_)								\
	do																			\
	{																			\
		const auto caeo_q1 = q1_;												\
		auto caeo_q2 = q2_;														\
		if (caeo_q1.dot(caeo_q2) < 0)											\
			caeo_q2 = decltype(caeo_q2){ -caeo_q2.s, -caeo_q2.v };				\
																				\
		using caeo_scalar = typename decltype(caeo_q1)::scalar_type;			\
		const auto caeo_eps = constants<caeo_scalar>::default_epsilon * 10;\
		CHECK_APPROX_EQUAL_EPS(caeo_q1.s,	caeo_q2.s,	 caeo_eps);				\
		CHECK_APPROX_EQUAL_EPS(caeo_q1.v.x,	caeo_q2.v.x, caeo_eps);				\
		CHECK_APPROX_EQUAL_EPS(caeo_q1.v.y,	caeo_q2.v.y, caeo_eps);				\
		CHECK_APPROX_EQUAL_EPS(caeo_q1.v.z,	caeo_q2.v.z, caeo_eps);				\
	}																			\
	while (false)

template <typename T>
inline void quat_slerp_test_case(intmax_t angle, T alpha, intmax_t expected_angle) noexcept
{
	// Checks equality of
	// - quat(<some axis>, expected_angle) vs
	// - slerp(identity, quat(<some axis>, angle), t) vs
	// - slerp(quat(<some axis>, angle), identity, 1-t)
	// Angles are in degrees just for the sake of being human-friendly.

	using quat_t = quaternion<T>;
	using vec_t = vector<T, 3>;
	using angle_t = impl::promote_if_small_float<T>;

	//const T epsilon = 1e-6f;
	const vec_t up{ T{}, T{ 1 }, T{} };
	const quat_t original = quat_t::from_axis_angle(
		up,
		static_cast<T>(static_cast<angle_t>(angle) * constants<angle_t>::degrees_to_radians
	));
	const quat_t expected = quat_t::from_axis_angle(
		up,
		static_cast<T>(static_cast<angle_t>(expected_angle) * constants<angle_t>::degrees_to_radians
	));

	const quat_t slerp_result = quat_t::slerp(quat_t::constants::identity, original, alpha);
	CHECK_APPROX_EQUAL_ORIENTATIONS(expected, slerp_result);

	// Apply the invariant that slerp(a, b, t) == slerp(b, a, 1-t).
	const quat_t slerp_backwards_result = quat_t::slerp(original, quat_t::constants::identity, T{ 1 } - alpha);
	CHECK_APPROX_EQUAL_ORIENTATIONS(expected, slerp_backwards_result);

	const quat_t mul_result = original * alpha;
	CHECK_APPROX_EQUAL_ORIENTATIONS(expected, mul_result);
}

template <typename T>
inline void quat_slerp_tests(std::string_view scalar_typename) noexcept
{
	INFO("quaternion<"sv << scalar_typename << ">"sv)

	// Easy and unambiguous cases.
	quat_slerp_test_case(+160, static_cast<T>(0.375), +60);
	quat_slerp_test_case(-160, static_cast<T>(0.375), -60);

	// Shortening a "long way around" (> 180 degree) rotation
	// NOTE: These results are different from the mathematical quat slerp
	quat_slerp_test_case(+320, static_cast<T>(0.375), -15);  // Mathematically, should be +120
	quat_slerp_test_case(-320, static_cast<T>(0.375), +15);  // Mathematically, should be -120

	// Lengthening a "long way around" rotation
	quat_slerp_test_case(320, static_cast<T>(1.5), -60);  // Mathematically, should be 480 (ie -240)

	// Lengthening to a "long way around" (> 180 degree) rotation
	quat_slerp_test_case(+70, T{ 3 }, +210);
	quat_slerp_test_case(-70, T{ 3 }, -210);

	// An edge case that often causes NaNs
	quat_slerp_test_case(0, static_cast<T>(0.5), 0);

	// This edge case is ill-defined for "intuitive" slerp and can't be tested.
	// quat_slerp_test_case(180, static_cast<T>(0.25), 45);

	// Conversely, this edge case is well-defined for "intuitive" slerp.
	// For mathematical slerp, the axis is ill-defined and can take many values.
	quat_slerp_test_case(360, static_cast<T>(0.25), 0);
}

template <typename T>
inline void quat_multiplication_tests(std::string_view scalar_typename) noexcept
{
	INFO("quaternion<"sv << scalar_typename << ">"sv)
	using quat_t = quaternion<T>;
	using vec3_t = vector<T, 3>;
	//using vec4_t = vector<T, 4>;
	//using mat3_t = matrix<T, 3, 3>;
	//using mat4_t = matrix<T, 4, 4>;

	const auto axis = normalize(vec3_t{
		static_cast<T>(4.3),
		static_cast<T>(7.6),
		static_cast<T>(1.2)
	});
	const T angle1 = static_cast<T>(1.2);
	const T angle2 = static_cast<T>(0.7);
	const T angle3 = static_cast<T>(angle2 + constants<T>::default_epsilon);
	const auto qaa1 = quat_t::from_axis_angle(axis, angle1);
	const auto qaa2 = quat_t::from_axis_angle(axis, angle2);
	const auto qaa3 = quat_t::from_axis_angle(axis, angle3);

	// verify that multiplying two quaternions corresponds to the sum of the rotations
	auto axis_angle = (qaa1 * qaa2).to_axis_angle();
	CHECK_APPROX_EQUAL(angle1 + angle2, axis_angle.angle);

	// verify that multiplying a quaternion with a scalar corresponds to scaling the rotation
	axis_angle = (qaa1 * T{ 2 }).to_axis_angle();
	CHECK_APPROX_EQUAL(angle1 * T{ 2 }, axis_angle.angle);

	//// verify that multiplying by a vector corresponds to applying the rotation to that vector
	//vec3_t v(3.5f, 6.4f, 7.0f);
	//vec4_t v4(3.5f, 6.4f, 7.0f, 0.0f);
	//vec3_t quatRotatedV(qaa1 * v);
	//vec3_t matRotatedV(mat3_t::FromRotation(qaa1) * v);
	//vec4_t mat4RotatedV(mat4_t::FromRotation(qaa1) * v4);
	//CHECK_APPROX_EQUAL(static_cast<double>(quatRotatedV[0]), static_cast<double>(matRotatedV[0]));
	//CHECK_APPROX_EQUAL(static_cast<double>(quatRotatedV[1]), static_cast<double>(matRotatedV[1]));
	//CHECK_APPROX_EQUAL(static_cast<double>(quatRotatedV[2]), static_cast<double>(matRotatedV[2]));
	//CHECK_APPROX_EQUAL(static_cast<double>(quatRotatedV[0]), static_cast<double>(mat4RotatedV[0]));
	//CHECK_APPROX_EQUAL(static_cast<double>(quatRotatedV[1]), static_cast<double>(mat4RotatedV[1]));
	//CHECK_APPROX_EQUAL(static_cast<double>(quatRotatedV[2]), static_cast<double>(mat4RotatedV[2]));

	// verify that interpolating two quaternions corresponds to interpolating the angle
	const quat_t slerp1 = quat_t::slerp(qaa1, qaa2, static_cast<T>(0.5));
	axis_angle = slerp1.to_axis_angle();
	CHECK_APPROX_EQUAL(static_cast<T>(0.5) * (angle1 + angle2), axis_angle.angle);
	const quat_t slerp2 = quat_t::slerp(qaa2, qaa3, static_cast<T>(0.5));
	axis_angle = slerp2.to_axis_angle();
	CHECK_APPROX_EQUAL(static_cast<T>(0.5) * (angle2 + angle3), axis_angle.angle);
	const quat_t slerp3 = quat_t::slerp(qaa2, qaa2, static_cast<T>(0.5));
	axis_angle = slerp3.to_axis_angle();
	CHECK_APPROX_EQUAL(angle2, axis_angle.angle);
}
