// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"
#include "../include/muu/vector.h"

template <typename T>
inline constexpr bool invoke_trait_tests = false;

template <typename T, size_t Dimensions>
inline constexpr void trait_tests(std::string_view /*scalar_typename*/) noexcept
{
	using vector_t = vector<T, Dimensions>;
	static_assert(sizeof(vector_t) == sizeof(T) * Dimensions);
	static_assert(sizeof(vector_t[5]) == sizeof(T) * Dimensions * 5);
	static_assert(std::is_standard_layout_v<vector_t>);
	static_assert(std::is_trivially_constructible_v<vector_t>);
	static_assert(std::is_trivially_copy_constructible_v<vector_t>);
	static_assert(std::is_trivially_copy_assignable_v<vector_t>);
	static_assert(std::is_trivially_move_constructible_v<vector_t>);
	static_assert(std::is_trivially_move_assignable_v<vector_t>);
	static_assert(std::is_trivially_destructible_v<vector_t>);
	static_assert(std::is_nothrow_constructible_v<vector_t, T>);
	static_assert(Dimensions < 2 || std::is_nothrow_constructible_v<vector_t, T, T>);
	static_assert(Dimensions < 3 || std::is_nothrow_constructible_v<vector_t, T, T, T>);
	static_assert(Dimensions < 4 || std::is_nothrow_constructible_v<vector_t, T, T, T, T>);
	static_assert(Dimensions < 5 || std::is_nothrow_constructible_v<vector_t, T, T, T, T, T>);

	#if MUU_HAS_VECTORCALL
	static_assert(impl::is_hva<vector<T, Dimensions>> == (
		Dimensions <= 4
		&& is_same_as_any<T, float, double, long double>
	));
	#endif
};

template <typename Vector, size_t NUM>
inline void construction_test_from_scalars() noexcept
{
	using scalar_type = typename Vector::scalar_type;
	INFO("constructing from "sv << NUM << " scalars"sv)

	const auto vals = random_array<scalar_type, NUM>();
	Vector vec = std::apply([](auto&& ... v) noexcept { return Vector{ v... }; }, vals);
	for (size_t i = 0; i < NUM; i++)
		CHECK(vec[i] == vals[i]);
	for (size_t i = NUM; i < Vector::dimensions; i++)
		CHECK(vec[i] == scalar_type{});
}

template <typename Vector, size_t NUM>
inline void construction_test_from_array() noexcept
{
	using scalar_type = typename Vector::scalar_type;

	const auto arr = random_array<scalar_type, NUM>();
	{
		auto vec = Vector{ arr };
		INFO("constructing from std::array with "sv << NUM << " elements"sv)
		for (size_t i = 0; i < NUM; i++)
			CHECK(vec[i] == arr[i]);
		for (size_t i = NUM; i < Vector::dimensions; i++)
			CHECK(vec[i] == scalar_type{});
	}

	scalar_type raw_arr[NUM];
	memcpy(raw_arr, arr.data(), sizeof(raw_arr));
	{
		auto vec = Vector{ raw_arr };
		INFO("constructing from raw array with "sv << NUM << " elements"sv)
		for (size_t i = 0; i < NUM; i++)
			CHECK(vec[i] == raw_arr[i]);
		for (size_t i = NUM; i < Vector::dimensions; i++)
			CHECK(vec[i] == scalar_type{});
	}

	{
		auto vec = Vector{ arr.data(), NUM };
		INFO("constructing from pointer to scalars"sv)
		for (size_t i = 0; i < NUM; i++)
			CHECK(vec[i] == arr[i]);
		for (size_t i = NUM; i < Vector::dimensions; i++)
			CHECK(vec[i] == scalar_type{});
	}
}

template <typename Vector, size_t NUM>
inline void construction_test_from_smaller_vector() noexcept
{
	using scalar_type = typename Vector::scalar_type;
	INFO("constructing from a smaller vector with "sv << NUM << " elements"sv)
	static_assert(NUM < Vector::dimensions);

	auto smaller = vector<scalar_type, NUM>{ random_array<scalar_type, NUM>() };
	auto vec = Vector{ smaller };
	for (size_t i = 0; i < NUM; i++)
		CHECK(vec[i] == smaller[i]);
	for (size_t i = NUM; i < Vector::dimensions; i++)
		CHECK(vec[i] == scalar_type{});
}

template <typename Vector, size_t NUM>
inline void construction_test_from_larger_vector() noexcept
{
	using scalar_type = typename Vector::scalar_type;
	INFO("constructing from a larger vector with "sv << NUM << " elements"sv)
	static_assert(NUM > Vector::dimensions);

	auto larger = vector<scalar_type, NUM>{ random_array<scalar_type, NUM>() };
	auto vec = Vector{ larger };
	for (size_t i = 0; i < Vector::dimensions; i++)
		CHECK(vec[i] == larger[i]);
}

template <typename T, size_t Dimensions>
inline void construction_tests(std::string_view scalar_typename) noexcept
{
	INFO("vector<"sv << scalar_typename << ", "sv << Dimensions << ">"sv)
	using vector_t = vector<T, Dimensions>;

	{
		INFO("fill constructor")

		const auto val = random<T>();
		vector_t vec{ val };
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(val == vec[i]);
	}

	{
		INFO("copy constructor"sv)
		vector_t v1;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			v1[i] = random<T>();
		vector_t v2{ v1 };
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK(v1[i] == v2[i]);
	}

	// scalar constructors
	if constexpr (Dimensions >= 2) construction_test_from_scalars<vector_t, 2>();
	if constexpr (Dimensions >= 3) construction_test_from_scalars<vector_t, 3>();
	if constexpr (Dimensions >= 4) construction_test_from_scalars<vector_t, 4>();
	if constexpr (Dimensions >= 5) construction_test_from_scalars<vector_t, 5>();
	// no single-scalar test; it's the fill constructor.

	// array constructor
	construction_test_from_array<vector_t, 1>();
	if constexpr (Dimensions >= 2) construction_test_from_array<vector_t, 2>();
	if constexpr (Dimensions >= 3) construction_test_from_array<vector_t, 3>();
	if constexpr (Dimensions >= 4) construction_test_from_array<vector_t, 4>();
	if constexpr (Dimensions >= 5) construction_test_from_array<vector_t, 5>();

	// coercing constructor
	{
		using other_type = std::conditional_t<
			is_same_as_any<T, signed int, unsigned int>,
			float,
			set_signed<signed int, is_signed<T>>
		>;

		vector<other_type, Dimensions> other;
		for (size_t i = 0; i < Dimensions; i++)
			other[i] = static_cast<other_type>(i);

		vector_t coerced{ other };
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(coerced[i] == static_cast<T>(other[i]));
	}

	// enlarging constructor
	if constexpr (Dimensions > 1) construction_test_from_smaller_vector<vector_t, 1>();
	if constexpr (Dimensions > 2) construction_test_from_smaller_vector<vector_t, 2>();
	if constexpr (Dimensions > 3) construction_test_from_smaller_vector<vector_t, 3>();
	if constexpr (Dimensions > 4) construction_test_from_smaller_vector<vector_t, 4>();
	if constexpr (Dimensions > 5) construction_test_from_smaller_vector<vector_t, 5>();

	// truncating constructor
	if constexpr (Dimensions < 2) construction_test_from_larger_vector<vector_t, 2>();
	if constexpr (Dimensions < 3) construction_test_from_larger_vector<vector_t, 3>();
	if constexpr (Dimensions < 4) construction_test_from_larger_vector<vector_t, 4>();
	if constexpr (Dimensions < 5) construction_test_from_larger_vector<vector_t, 5>();
	if constexpr (Dimensions < 10) construction_test_from_larger_vector<vector_t, 10>();

	// pair constructor
	if constexpr (Dimensions >= 2)
	{
		INFO("constructing from a std::pair"sv)

		auto values = std::pair{ random<T>(), random<T>() };
		vector_t vec{ values };
		CHECK(vec[0] == std::get<0>(values));
		CHECK(vec[1] == std::get<1>(values));
		if constexpr (Dimensions > 2) CHECK(vec[2] == T{});
	}

	// tuple constructor (3 elems)
	if constexpr (Dimensions >= 3)
	{
		INFO("constructing from a std::tuple"sv)

		auto values = std::tuple{ random<T>(), random<T>(), random<T>() };
		vector_t vec{ values };
		CHECK(vec[0] == std::get<0>(values));
		CHECK(vec[1] == std::get<1>(values));
		CHECK(vec[2] == std::get<2>(values));
		if constexpr (Dimensions > 3) CHECK(vec[3] == T{});
	}


	if constexpr (Dimensions >= 3)
	{
		INFO("concatenating constructor (xy, z)")

		const vector<T, 2> xy{ random_array<T, 2>() };
		const auto z = random<T>();
		const vector_t vec{ xy, z };
		CHECK(vec[0] == xy[0]);
		CHECK(vec[1] == xy[1]);
		CHECK(vec[2] == z);
		for (size_t i = 3; i < Dimensions; i++)
			CHECK(vec[i] == T{});
	}

	if constexpr (Dimensions >= 4)
	{
		const vector<T, 2> xy{ random_array<T, 2>() };
		const vector<T, 2> zw{ random_array<T, 2>() };
			
		{
			INFO("concatenating constructor (xy, zw)")

			const vector_t vec{ xy, zw };
			CHECK(vec[0] == xy[0]);
			CHECK(vec[1] == xy[1]);
			CHECK(vec[2] == zw[0]);
			CHECK(vec[3] == zw[1]);
			for (size_t i = 4; i < Dimensions; i++)
				CHECK(vec[i] == T{});
		}

		{
			INFO("appending constructor (xy, z, w)")

			const vector_t vec{ xy, zw[0], zw[1] };
			CHECK(vec[0] == xy[0]);
			CHECK(vec[1] == xy[1]);
			CHECK(vec[2] == zw[0]);
			CHECK(vec[3] == zw[1]);
			for (size_t i = 4; i < Dimensions; i++)
				CHECK(vec[i] == T{});
		}
	}
}

template <typename T, size_t Dimensions>
inline void accessor_tests(std::string_view scalar_typename) noexcept
{
	INFO("vector<"sv << scalar_typename << ", "sv << Dimensions << ">"sv)
	using vector_t = vector<T, Dimensions>;

	const auto x = random_array<T, Dimensions>();
	vector_t vec{ x };
	const vector_t& vec_const = vec;

	{
		INFO("operator[]"sv)
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(x[i] == vec[i]);
	}

	{
		INFO("get()"sv)
		CHECK(x[0] == vec.template get<0>());
		if constexpr (Dimensions >= 2) CHECK(x[1] == vec.template get<1>());
		if constexpr (Dimensions >= 3) CHECK(x[2] == vec.template get<2>());
		if constexpr (Dimensions >= 4) CHECK(x[3] == vec.template get<3>());
		if constexpr (Dimensions >= 5) CHECK(x[4] == vec.template get<4>());
	}

	{
		INFO("operator[] (const)"sv)
		for (size_t i = 0; i < Dimensions; i++)
		{
			CHECK(x[i] == vec_const[i]);
			CHECK(&vec[i] == &vec_const[i]);
		}
	}

	{
		INFO("get() (const)"sv)
		CHECK(x[0] == vec_const.template get<0>());
		if constexpr (Dimensions >= 2) CHECK(x[1] == vec_const.template get<1>());
		if constexpr (Dimensions >= 3) CHECK(x[2] == vec_const.template get<2>());
		if constexpr (Dimensions >= 4) CHECK(x[3] == vec_const.template get<3>());
		if constexpr (Dimensions >= 5) CHECK(x[4] == vec_const.template get<4>());
		CHECK(&vec.template get<0>() == &vec_const[0]);
		if constexpr (Dimensions >= 2) CHECK(&vec.template get<1>() == &vec_const[1]);
		if constexpr (Dimensions >= 3) CHECK(&vec.template get<2>() == &vec_const[2]);
		if constexpr (Dimensions >= 4) CHECK(&vec.template get<3>() == &vec_const[3]);
		if constexpr (Dimensions >= 5) CHECK(&vec.template get<4>() == &vec_const[4]);
	}

	{
		INFO("ranged-for"sv)
		size_t i = 0;
		for (auto& val : vec)
		{
			CHECK(x[i] == val);
			CHECK(&vec[i] == &val);
			i++;
		}
		CHECK(i == Dimensions);
	}

	{
		INFO("ranged-for (const)"sv)
		size_t i = 0;
		for (auto& val : vec_const)
		{
			CHECK(x[i] == val);
			CHECK(&vec[i] == &val);
			CHECK(&vec_const[i] == &val);
			i++;
		}
		CHECK(i == Dimensions);
	}

	if constexpr (Dimensions <= 4)
	{
		INFO("named members"sv)
		CHECK(&vec.x == &vec.template get<0>());
		if constexpr (Dimensions >= 2) CHECK(&vec.y == &vec.template get<1>());
		if constexpr (Dimensions >= 3) CHECK(&vec.z == &vec.template get<2>());
		if constexpr (Dimensions >= 4) CHECK(&vec.w == &vec.template get<3>());
	}
}

template <typename T, size_t Dimensions>
inline void equality_tests(std::string_view scalar_typename) noexcept
{
	INFO("vector<"sv << scalar_typename << ", "sv << Dimensions << ">"sv)
	using vector_t = vector<T, Dimensions>;

	vector_t vec;
	for (size_t i = 0; i < Dimensions; i++)
		vec[i] = random<T>();
		
	{
		INFO("same type"sv)
		
		vector_t same{ vec };
		CHECK(vector_t::equal(vec, same));
		CHECK(vec == same);
		CHECK_FALSE(vec != same);
		CHECK(same == vec);
		CHECK_FALSE(same != vec);
		if constexpr (is_floating_point<T>)
			CHECK(vector_t::approx_equal(vec, same));

		vector_t different{ vec };
		for (size_t i = 0; i < Dimensions; i++)
			different[i]++;
		CHECK_FALSE(vector_t::equal(vec, different));
		CHECK(vec != different);
		CHECK_FALSE(vec == different);
		CHECK(different != vec);
		CHECK_FALSE(different == vec);
		if constexpr (is_floating_point<T>)
			CHECK_FALSE(vector_t::approx_equal(vec, different));
	}

	if constexpr (!is_floating_point<T>)
	{
		INFO("different type"sv)

		using other_t = vector<
			std::conditional_t<std::is_same_v<T, long>, int, long>,
			Dimensions
		>;

		other_t same{ vec };
		CHECK(vector_t::equal(vec, same));
		CHECK(vec == same);
		CHECK_FALSE(vec != same);
		CHECK(vec == same);
		CHECK_FALSE(vec != same);

		other_t different{ vec };
		for (size_t i = 0; i < Dimensions; i++)
			different[i]++;
		CHECK_FALSE(vector_t::equal(vec, different));
		CHECK(vec != different);
		CHECK_FALSE(vec == different);
		CHECK(different != vec);
		CHECK_FALSE(different == vec);
	}
}

template <typename T, size_t Dimensions>
inline void zero_tests(std::string_view scalar_typename) noexcept
{
	INFO("vector<"sv << scalar_typename << ", "sv << Dimensions << ">"sv)
	using vector_t = vector<T, Dimensions>;

	vector_t vec{ T{} };
	{
		INFO("all zeroes"sv)
		CHECK(vec.zero());
	}

	{
		INFO("no zeroes"sv)
		for (size_t i = 0; i < Dimensions; i++)
			vec[i] = random(T{ 1 }, T{ 10 });
		CHECK_FALSE(vec.zero());
	}

	if constexpr (Dimensions > 1)
	{
		INFO("some zeroes"sv)
		for (size_t i = 0; i < Dimensions; i += 2)
			vec[i] = T{};
		CHECK_FALSE(vec.zero());
	}

	{
		INFO("one zero"sv)
		for (size_t i = 0; i < Dimensions; i++)
		{
			vector_t vec2{ T{} };
			vec2[i] = random(T{ 1 }, T{ 10 });
			CHECK_FALSE(vec.zero());
		}
	}
}

template <typename T, size_t Dimensions>
inline void infinity_or_nan_tests(std::string_view scalar_typename) noexcept
{
	INFO("vector<"sv << scalar_typename << ", "sv << Dimensions << ">"sv)
	using vector_t = vector<T, Dimensions>;

	vector_t vector1;
	{
		INFO("all finite"sv)
		for (size_t i = 0; i < Dimensions; i++)
			vector1[i] = static_cast<T>(i);
		CHECK_FALSE(vector1.infinity_or_nan());
	}


	if constexpr (is_floating_point<T>)
	{
		{
			INFO("contains one NaN"sv)
			for (size_t i = 0; i < Dimensions; i++)
			{
				vector_t vector2{ vector1 };
				vector2[i] = make_nan<T>();
				CHECK(vector2.infinity_or_nan());
			}
		}

		{
			INFO("contains one infinity"sv)
			for (size_t i = 0; i < Dimensions; i++)
			{
				vector_t vector2{ vector1 };
				vector2[i] = make_infinity<T>();
				CHECK(vector2.infinity_or_nan());
			}
		}
	}
}

template <typename T, size_t Dimensions>
inline void dot_tests(std::string_view scalar_typename) noexcept
{
	INFO("vector<"sv << scalar_typename << ", "sv << Dimensions << ">"sv)
	using vector_t = vector<T, Dimensions>;

	const auto x1 = random_array<T, Dimensions>();
	const auto x2 = random_array<T, Dimensions>();
	vector_t vector1(x1), vector2(x2);
		
	using dot_type = decltype(vector1.dot(vector2));
	static_assert(is_floating_point<dot_type>);

	// expected result
	// (accumulating in a potentially more precise intermediate type then coverting the result is what happens in
	// the vector class to minimize loss in float16, so that same behaviour is replicated here)
	using intermediate_type = impl::promote_if_small_float<dot_type>;
	auto expected_sum = intermediate_type{};
	for (size_t i = 0; i < Dimensions; i++)
		expected_sum += static_cast<intermediate_type>(x1[i]) * static_cast<intermediate_type>(x2[i]);
	const auto expected = static_cast<dot_type>(expected_sum);

	CHECK(vector1.dot(vector2) == approx(expected));
	CHECK(vector_t::dot(vector1, vector2) == approx(expected));
	//CHECK(muu::dot(vector1, vector2) == approx(expected));
}

template <typename T, size_t Dimensions>
inline void cross_tests([[maybe_unused]] std::string_view scalar_typename) noexcept
{
	static_assert(Dimensions == 3);

	INFO("vector<"sv << scalar_typename << ", "sv << Dimensions << ">"sv)
	using vector_t = vector<T, Dimensions>;
	using promoted = impl::highest_ranked<T, double>;
	using vec3d = vector<promoted, 3>;

	// everything here is double or higher EXCEPT the calls to vector::cross()
	// (since that's the thing we're actually testing). Otherwise fp errors cause this to yield false negatives
	// with fp16 etc.

	auto vec1 = vec3d{ static_cast<promoted>(1.1), static_cast<promoted>(4.5), static_cast<promoted>(9.8) };
	auto vec2 = vec3d{ static_cast<promoted>(-1.4), static_cast<promoted>(9.5), static_cast<promoted>(3.2) };
	vec1.normalize();
	vec2.normalize();
	CHECK(vec1.unit_length());
	CHECK(vec2.unit_length());

	{
		INFO("vector.cross(vector)"sv)

		const auto cross_vector = vec3d{ vector_t{ vec1 }.cross(vector_t{ vec2 }) };
		const auto vec1_dot = cross_vector.dot(vec3d{ vec1 });
		const auto vec2_dot = cross_vector.dot(vec3d{ vec2 });
		CHECK(approx_equal(vec1_dot, promoted{}, static_cast<promoted>(constants<T>::approx_equal_epsilon)));
		CHECK(approx_equal(vec2_dot, promoted{}, static_cast<promoted>(constants<T>::approx_equal_epsilon)));
	}

	{
		INFO("vector::cross(vector, vector)"sv)

		const auto cross_vector = vec3d{ vector_t::cross(vector_t{ vec1 }, vector_t{ vec2 }) };
		const auto vec1_dot = cross_vector.dot(vec3d{ vec1 });
		const auto vec2_dot = cross_vector.dot(vec3d{ vec2 });
		CHECK(approx_equal(vec1_dot, promoted{}, static_cast<promoted>(constants<T>::approx_equal_epsilon)));
		CHECK(approx_equal(vec2_dot, promoted{}, static_cast<promoted>(constants<T>::approx_equal_epsilon)));
	}

	//{
	//	INFO("cross(vector, vector)"sv)
	//
	//	const auto cross_vector = cross(vec1, vec2);
	//	const auto vec1_dot = fcross_vector.dot(vec1);
	//	const auto vec2_dot = fcross_vector.dot(vec2);
	//	CHECK(vec1_dot == approx(T{}));
	//	CHECK(vec2_dot == approx(T{}));
	//}
}

template <typename T, size_t Dimensions>
inline void addition_tests(std::string_view scalar_typename) noexcept
{
	INFO("vector<"sv << scalar_typename << ", "sv << Dimensions << ">"sv)
	using vector_t = vector<T, Dimensions>;

	const auto x1 = random_array<T, Dimensions>(0, 5);
	const auto x2 = random_array<T, Dimensions>(0, 5);
	const vector_t vector1{ x1 }, vector2{ x2 };

	{
		INFO("vector + vector"sv)

		vector_t result = vector1 + vector2;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(static_cast<T>(x1[i] + x2[i]) == approx(result[i]));
	}

	{
		INFO("vector += vector"sv)

		vector_t result(vector1);
		result += vector2;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(static_cast<T>(x1[i] + x2[i]) == approx(result[i]));
	}
}

template <typename T, size_t Dimensions>
inline void subtraction_tests(std::string_view scalar_typename) noexcept
{
	INFO("vector<"sv << scalar_typename << ", "sv << Dimensions << ">"sv)
	using vector_t = vector<T, Dimensions>;

	const auto x1 = is_signed<T>
		? random_array<T, Dimensions>(0, 10)
		: random_array<T, Dimensions>(11, 20);
	const auto x2 = random_array<T, Dimensions>(0, 10);
	const vector_t vector1{ x1 }, vector2{ x2 };

	{
		INFO("vector - vector"sv)

		const vector_t result = vector1 - vector2;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(static_cast<T>(x1[i] - x2[i]) == approx(result[i]));
	}

	{
		INFO("vector -= vector"sv)

		vector_t result(vector1);
		result -= vector2;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(static_cast<T>(x1[i] - x2[i]) == approx(result[i]));
	}

	if constexpr (is_signed<T>)
	{
		INFO("-vector"sv)

		const vector_t result = -vector1;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(static_cast<T>(-x1[i]) == approx(result[i]));
	}
}

template <typename T, size_t Dimensions>
inline void multiplication_tests(std::string_view scalar_typename) noexcept
{
	INFO("vector<"sv << scalar_typename << ", "sv << Dimensions << ">"sv)
	using vector_t = vector<T, Dimensions>;

	const auto scalar = static_cast<T>(2.4);
	const auto x1 = random_array<T, Dimensions>(0, 5);
	const auto x2 = random_array<T, Dimensions>(0, 5);
	const vector_t vector1{ x1 }, vector2{ x2 };

	{
		INFO("vector * vector"sv)

		vector_t result = vector1 * vector2;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(static_cast<T>(x1[i] * x2[i]) == approx(result[i]));
	}

	{
		INFO("vector *= vector"sv)

		vector_t result(vector1);
		result *= vector2;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(static_cast<T>(x1[i] * x2[i]) == approx(result[i]));
	}

	{
		INFO("vector * scalar"sv)

		vector_t result = vector1 * scalar;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(static_cast<T>(x1[i] * scalar) == approx(result[i]));
	}

	{
		INFO("scalar * vector"sv)

		vector_t result = scalar * vector2;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(static_cast<T>(x2[i] * scalar) == approx(result[i]));
	}


	{
		INFO("vector *= scalar"sv)

		vector_t result(vector1);
		result *= scalar;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(static_cast<T>(x1[i] * scalar) == approx(result[i]));
	}
}

template <typename T, size_t Dimensions>
inline void division_tests(std::string_view scalar_typename) noexcept
{
	INFO("vector<"sv << scalar_typename << ", "sv << Dimensions << ">"sv)
	using vector_t = vector<T, Dimensions>;

	const auto scalar = static_cast<T>(2.4);
	const vector_t vec1{ random_array<T, Dimensions>(2, 10) };
	const vector_t vec2{ random_array<T, Dimensions>(2, 10) };

	{
		INFO("vector / vector"sv)

		vector_t result = vec1 / vec2;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(static_cast<T>(vec1[i] / vec2[i]) == approx(result[i]));
	}

	{
		INFO("vector /= vector"sv)

		vector_t result = vec1;
		result /= vec2;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(static_cast<T>(vec1[i] / vec2[i]) == approx(result[i]));
	}

	{
		INFO("vector / scalar"sv)

		vector_t result = vec1 / scalar;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(static_cast<T>(vec1[i] / scalar) == approx(result[i]));
	}

	{
		INFO("vector /= scalar"sv)

		vector_t result = vec1;
		result /= scalar;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(static_cast<T>(vec1[i] / scalar) == approx(result[i]));
	}
}

template <typename T, size_t Dimensions>
inline void modulo_tests(std::string_view scalar_typename) noexcept
{
	INFO("vector<"sv << scalar_typename << ", "sv << Dimensions << ">"sv)
	using vector_t = vector<T, Dimensions>;

	const auto scalar = static_cast<T>(2.4);
	const vector_t vec1{ random_array<T, Dimensions>(2, 10) };
	const vector_t vec2{ random_array<T, Dimensions>(2, 10) };

	{
		INFO("vector % vector"sv)

		vector_t result = vec1 % vec2;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(static_cast<T>(impl::modulo(vec1[i], vec2[i])) == approx(result[i]));
	}

	{
		INFO("vector %= vector"sv)

		vector_t result = vec1;
		result %= vec2;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(static_cast<T>(impl::modulo(vec1[i], vec2[i])) == approx(result[i]));
	}

	{
		INFO("vector % scalar"sv)

		vector_t result = vec1 % scalar;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(static_cast<T>(impl::modulo(vec1[i], scalar)) == approx(result[i]));
	}

	{
		INFO("vector %= scalar"sv)

		vector_t result = vec1;
		result %= scalar;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(static_cast<T>(impl::modulo(vec1[i], scalar)) == approx(result[i]));
	}
}

template <typename T, size_t Dimensions>
inline void bitwise_shift_tests(std::string_view scalar_typename) noexcept
{
	INFO("vector<"sv << scalar_typename << ", "sv << Dimensions << ">"sv)
	using vector_t = vector<T, Dimensions>;

	const int shift = random<int>(5);
	const vector_t original{ T{1} };
	const vector_t shifted{ static_cast<T>(T{1} << shift) };

	{
		INFO("vector << int"sv)

		vector_t result = original << shift;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(result[i] == shifted[i]);
	}

	{
		INFO("vector <<= int"sv)

		vector_t result = original;
		result <<= shift;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(result[i] == shifted[i]);
	}

	{
		INFO("vector >> int"sv)

		vector_t result = shifted >> shift;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(result[i] == original[i]);
	}

	{
		INFO("vector >>= int"sv)

		vector_t result = shifted;
		result >>= shift;
		for (size_t i = 0; i < Dimensions; i++)
			CHECK(result[i] == original[i]);
	}
}

template <typename T, size_t Dimensions>
inline void normalization_tests(std::string_view scalar_typename) noexcept
{
	INFO("vector<"sv << scalar_typename << ", "sv << Dimensions << ">"sv)
	using vector_t = vector<T, Dimensions>;

	const vector_t x{ random_array<T, Dimensions>(2, 10) };

	{
		INFO("vector.normalize()"sv)

		vector_t vec{ x };
		vec.normalize();
		CHECK(vec.unit_length());
		CHECK(vec.length() == approx(T{ 1 }));
	}

	{
		INFO("vector::normalize(vector)"sv)

		vector_t vec{ x };
		vec = vector_t::normalize(vec);
		CHECK(vec.length() == approx(T{ 1 }));
	}
}

template <typename T>
struct lerp_test_case
{
	using alpha_type = std::conditional_t<is_floating_point<T>, T, double>;

	T start;
	T finish;
	alpha_type alpha;
	T expected;
};

template <typename T>
struct lerp_test_data
{
	static constexpr auto name = "general"sv;
	using alpha_type = typename lerp_test_case<T>::alpha_type;
	using c = muu::constants<T>;
	using a = muu::constants<alpha_type>;
	static constexpr lerp_test_case<T> cases[]
	{
		/* 0 */ { c::zero,			c::one,			a::two,				c::two },
		/* 1 */ { c::one,			c::two,			a::one,				c::two },
		/* 2 */ { c::one,			c::two,			a::two,				c::three },
		/* 3 */ { c::one,			c::two,			a::zero,			c::one },
		/* 4 */ { c::one,			c::one,			a::two,				c::one },
	};
};

template <typename T>
struct lerp_signed_test_data
{
	static constexpr auto name = "signed"sv;
	using alpha_type = typename lerp_test_case<T>::alpha_type;
	using c = muu::constants<T>;
	using a = muu::constants<alpha_type>;
	static constexpr lerp_test_case<T> cases[]
	{
		/* 0 */ { c::minus_one,		c::one,			a::two,				c::three },
		/* 1 */ { c::minus_one,		c::zero,		a::two,				c::one },
		/* 2 */ { c::one,			c::minus_one,	a::two,				c::minus_three },
		/* 3 */ { c::zero,			c::minus_one,	a::two,				c::minus_two },
		/* 4 */ { c::one,			c::zero,		a::two,				c::minus_one },
	};
};

template <typename T>
struct lerp_float_test_data
{
	static constexpr auto name = "floating-point"sv;
	using alpha_type = typename lerp_test_case<T>::alpha_type;
	using c = muu::constants<T>;
	using a = muu::constants<alpha_type>;
	static constexpr lerp_test_case<T> cases[]
	{
		/* 0 */ { c::minus_zero,	c::minus_zero,	a::one_over_two,	c::minus_zero },
		/* 1 */ { c::minus_five,	c::five,		a::one_over_two,	c::zero },
		/* 2 */ { c::one,			c::two,			a::one_over_two,	c::three_over_two },
		/* 3 */ { c::zero,			c::zero,		a::one_over_two,	c::zero },
	};
};

template <typename T, size_t Dimensions, typename Dataset>
inline void lerp_specific_tests() noexcept
{
	INFO("lerp test dataset: "sv << Dataset::name)
	using vector_t = vector<T, Dimensions>;

	size_t i{};
	for (const auto& case_ : Dataset::cases)
	{
		INFO("test case " << i++)

		{
			INFO("vector::lerp(start, finish, expected)")
			const auto vec = vector_t::lerp(vector_t{ case_.start }, vector_t{ case_.finish }, case_.alpha);
			CHECK(vec == vector_t{ case_.expected });
		}

		{
			INFO("vector.lerp(target, expected)")
			auto vec = vector_t{ case_.start };
			vec.lerp(vector_t{ case_.finish }, case_.alpha);
			CHECK(vec == vector_t{ case_.expected });
		}
	}
}

template <typename T, size_t Dimensions>
inline void lerp_tests(std::string_view scalar_typename) noexcept
{
	INFO("vector<"sv << scalar_typename << ", "sv << Dimensions << ">"sv)

	lerp_specific_tests<T, Dimensions, lerp_test_data<T>>();

	if constexpr (is_signed<T>)
		lerp_specific_tests<T, Dimensions, lerp_signed_test_data<T>>();

	if constexpr (is_floating_point<T>)
		lerp_specific_tests<T, Dimensions, lerp_float_test_data<T>>();
}

template <typename T, size_t Dimensions>
inline void min_max_tests(std::string_view scalar_typename) noexcept
{
	INFO("vector<"sv << scalar_typename << ", "sv << Dimensions << ">"sv)
	using vector_t = vector<T, Dimensions>;

	const vector_t zeroes{ T{} };                     // {  0,  0,  0, ...}
	vector_t sequential;                              // {  1,  2,  3, ...}
	[[maybe_unused]] vector_t sequential_negative;    // { -1, -2, -3, ...} (signed only)
	vector_t interleaved1{ zeroes };                  // {  1,  0,  3, ...}
	vector_t interleaved2{ zeroes };                  // {  0,  2,  0, ...}
	for (size_t i = 0; i < Dimensions; i++)
	{
		sequential[i] = static_cast<T>(i+1u);
		if constexpr (is_signed<T>)
			sequential_negative[i] = static_cast<T>(-sequential[i]);
		(*(i % 2u == 0 ? &interleaved1 : &interleaved2))[i] = sequential[i];
	}

	// (zeroes, *)
	CHECK(vector_t::min(zeroes, zeroes) == zeroes);
	CHECK(vector_t::max(zeroes, zeroes) == zeroes);
	CHECK(vector_t::min(zeroes, sequential) == zeroes);
	CHECK(vector_t::max(zeroes, sequential) == sequential);
	CHECK(vector_t::min(zeroes, interleaved1) == zeroes);
	CHECK(vector_t::max(zeroes, interleaved1) == interleaved1);
	CHECK(vector_t::min(zeroes, interleaved2) == zeroes);
	CHECK(vector_t::max(zeroes, interleaved2) == interleaved2);
	if constexpr (is_signed<T>)
	{
		CHECK(vector_t::min(zeroes, sequential_negative) == sequential_negative);
		CHECK(vector_t::max(zeroes, sequential_negative) == zeroes);
	}
	
	// (sequential, *)
	CHECK(vector_t::min(sequential, sequential) == sequential);
	CHECK(vector_t::max(sequential, sequential) == sequential);
	CHECK(vector_t::min(sequential, interleaved1) == interleaved1);
	CHECK(vector_t::max(sequential, interleaved1) == sequential);
	CHECK(vector_t::min(sequential, interleaved2) == interleaved2);
	CHECK(vector_t::max(sequential, interleaved2) == sequential);
	if constexpr (is_signed<T>)
	{
		CHECK(vector_t::min(sequential, sequential_negative) == sequential_negative);
		CHECK(vector_t::max(sequential, sequential_negative) == sequential);
	}

	// (sequential_negative, *)
	if constexpr (is_signed<T>)
	{
		CHECK(vector_t::min(sequential_negative, sequential_negative) == sequential_negative);
		CHECK(vector_t::max(sequential_negative, sequential_negative) == sequential_negative);
		CHECK(vector_t::min(sequential_negative, interleaved1) == sequential_negative);
		CHECK(vector_t::max(sequential_negative, interleaved1) == interleaved1);
		CHECK(vector_t::min(sequential_negative, interleaved2) == sequential_negative);
		CHECK(vector_t::max(sequential_negative, interleaved2) == interleaved2);
	}

	// (interleaved1, *)
	CHECK(vector_t::min(interleaved1, interleaved1) == interleaved1);
	CHECK(vector_t::max(interleaved1, interleaved1) == interleaved1);
	CHECK(vector_t::min(interleaved1, interleaved2) == zeroes);
	CHECK(vector_t::max(interleaved1, interleaved2) == sequential);

	// (interleaved2, *)
	CHECK(vector_t::min(interleaved2, interleaved2) == interleaved2);
	CHECK(vector_t::max(interleaved2, interleaved2) == interleaved2);
}
