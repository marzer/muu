// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"
#include "batching.h"
#include "../include/muu/vector.h"
#include "../include/muu/accumulator.h"
#include "../include/muu/span.h"

namespace
{
	template <typename T, size_t Dimensions, size_t NUM>
	static void vector_construction_test_from_scalars() noexcept
	{
		static_assert(NUM != 1 || Dimensions == 1);

		if constexpr (Dimensions >= NUM)
		{
			TEST_INFO("constructing from "sv << NUM << " scalars"sv);
			using vector_t = vector<T, Dimensions>;

			const auto vals = random_array<T, NUM>(1, 10);

			// std::apply is super taxing for the compiler to instantiate on some implementations
			// I'm simulating it for small value counts
			vector_t vec;
			if constexpr (NUM == 1)
				vec = vector_t{ vals[0] };
			if constexpr (NUM == 2)
				vec = vector_t{ vals[0], vals[1] };
			if constexpr (NUM == 3)
				vec = vector_t{ vals[0], vals[1], vals[2] };
			if constexpr (NUM == 4)
				vec = vector_t{ vals[0], vals[1], vals[2], vals[3] };
			if constexpr (NUM == 5)
				vec = vector_t{ vals[0], vals[1], vals[2], vals[3], vals[4] };
			if constexpr (NUM == 6)
				vec = vector_t{ vals[0], vals[1], vals[2], vals[3], vals[4], vals[5] };
			if constexpr (NUM == 7)
				vec = vector_t{ vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6] };
			if constexpr (NUM == 8)
				vec = vector_t{ vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6], vals[7] };
			if constexpr (NUM == 9)
				vec = vector_t{ vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6], vals[7], vals[8] };
			if constexpr (NUM >= 10)
				vec = std::apply([](auto&&... s) noexcept { return vector_t{ s... }; }, vals);

			for (size_t i = 0; i < NUM; i++)
				CHECK(vec[i] == vals[i]);
			for (size_t i = NUM; i < Dimensions; i++)
				CHECK(vec[i] == T{});
		}
	}

	template <typename T, size_t Dimensions, size_t NUM>
	static void vector_construction_test_from_array() noexcept
	{
		if constexpr (Dimensions >= NUM)
		{
			TEST_INFO("constructing from "sv << NUM << " contiguous elements"sv);

			using vector_t = vector<T, Dimensions>;

			const auto arr = random_array<T, NUM>(1, 10);

			BATCHED_SECTION("std::array")
			{
				auto vec = vector_t{ arr };
				for (size_t i = 0; i < NUM; i++)
					CHECK(vec[i] == arr[i]);
				for (size_t i = NUM; i < Dimensions; i++)
					CHECK(vec[i] == T{});
			}

			BATCHED_SECTION("raw array")
			{
				T raw_arr[NUM];
				memcpy(raw_arr, arr.data(), sizeof(raw_arr));

				auto vec = vector_t{ raw_arr };
				for (size_t i = 0; i < NUM; i++)
					CHECK(vec[i] == raw_arr[i]);
				for (size_t i = NUM; i < Dimensions; i++)
					CHECK(vec[i] == T{});
			}

			BATCHED_SECTION("pointer to scalars + count")
			{
				auto vec = vector_t{ arr.data(), NUM };
				for (size_t i = 0; i < NUM; i++)
					CHECK(vec[i] == arr[i]);
				for (size_t i = NUM; i < vector_t::dimensions; i++)
					CHECK(vec[i] == T{});
			}

			BATCHED_SECTION("pointer to scalars")
			if constexpr (Dimensions == NUM)
			{
				auto vec = vector_t{ arr.data() };
				for (size_t i = 0; i < NUM; i++)
					CHECK(vec[i] == arr[i]);
				for (size_t i = NUM; i < vector_t::dimensions; i++)
					CHECK(vec[i] == T{});
			}

			BATCHED_SECTION("statically-sized span")
			{
				auto vec = vector_t{ span<T, NUM>{ arr } };
				for (size_t i = 0; i < NUM; i++)
					CHECK(vec[i] == arr[i]);
				for (size_t i = NUM; i < Dimensions; i++)
					CHECK(vec[i] == T{});
			}

			BATCHED_SECTION("dynamically-sized span")
			{
				auto vec = vector_t{ span<T>{ arr.data(), NUM } };
				for (size_t i = 0; i < NUM; i++)
					CHECK(vec[i] == arr[i]);
				for (size_t i = NUM; i < Dimensions; i++)
					CHECK(vec[i] == T{});
			}
		}
	}

	template <typename T, size_t Dimensions, size_t NUM>
	static void vector_construction_test_from_smaller_vector() noexcept
	{
		if constexpr (Dimensions > NUM)
		{
			TEST_INFO("constructing from a smaller vector with "sv << NUM << " elements"sv);
			using vector_t = vector<T, Dimensions>;

			auto smaller = vector<T, NUM>{ random_array<T, NUM>(1, 10) };
			auto vec	 = vector_t{ smaller };
			for (size_t i = 0; i < NUM; i++)
				CHECK(vec[i] == smaller[i]);
			for (size_t i = NUM; i < Dimensions; i++)
				CHECK(vec[i] == T{});
		}
	}

	template <typename T, size_t Dimensions, size_t NUM>
	static void vector_construction_test_from_larger_vector() noexcept
	{
		if constexpr (Dimensions < NUM)
		{
			TEST_INFO("constructing from a larger vector with "sv << NUM << " elements"sv);
			using vector_t = vector<T, Dimensions>;

			auto larger = vector<T, NUM>{ random_array<T, NUM>(1, 10) };
			auto vec	= vector_t{ larger };
			for (size_t i = 0; i < Dimensions; i++)
				CHECK(vec[i] == larger[i]);
		}
	}

	template <typename T, size_t Dimensions>
	struct blittable
	{
		T values[Dimensions];
	};

	template <typename... T>
	using vectors = std::tuple<vector<T, 1>..., vector<T, 2>..., vector<T, 3>..., vector<T, 4>..., vector<T, 5>...>;

	template <size_t N, typename... T>
	using vectors_N = std::tuple<vector<T, N>...>;

	template <size_t N1, size_t N2, typename... T>
	using vectors_NN = std::tuple<vector<T, N1>..., vector<T, N2>...>;

	template <size_t N1, size_t N2, size_t N3, typename... T>
	using vectors_NNN = std::tuple<vector<T, N1>..., vector<T, N2>..., vector<T, N3>...>;
}

namespace muu
{
	template <typename T, size_t Dimensions>
	inline constexpr bool allow_implicit_bit_cast<blittable<T, Dimensions>, vector<T, Dimensions>> = true;
}

TEMPLATE_BATCHED_TEST("vector constructors", vectors<ALL_ARITHMETIC>)
{
	using vector_t = TestType;
	using T		   = typename vector_t::scalar_type;
	TEST_INFO("vector<"sv << nameof<T> << ", "sv << vector_t::dimensions << ">"sv);

	static_assert(sizeof(vector_t) == sizeof(T) * vector_t::dimensions);
	static_assert(sizeof(vector_t[5]) == sizeof(T) * vector_t::dimensions * 5);
	static_assert(std::is_standard_layout_v<vector_t>);
	static_assert(std::is_trivially_constructible_v<vector_t>);
	static_assert(std::is_trivially_copy_constructible_v<vector_t>);
	static_assert(std::is_trivially_copy_assignable_v<vector_t>);
	static_assert(std::is_trivially_move_constructible_v<vector_t>);
	static_assert(std::is_trivially_move_assignable_v<vector_t>);
	static_assert(std::is_trivially_destructible_v<vector_t>);
	static_assert(std::is_nothrow_constructible_v<vector_t, T>);
	static_assert(vector_t::dimensions < 2 || std::is_nothrow_constructible_v<vector_t, T, T>);
	static_assert(vector_t::dimensions < 3 || std::is_nothrow_constructible_v<vector_t, T, T, T>);
	static_assert(vector_t::dimensions < 4 || std::is_nothrow_constructible_v<vector_t, T, T, T, T>);
	static_assert(vector_t::dimensions < 5 || std::is_nothrow_constructible_v<vector_t, T, T, T, T, T>);

#if MUU_HAS_VECTORCALL
	static_assert(
		impl::is_hva<vector_t> == (vector_t::dimensions <= 4 && is_same_as_any<T, float, double, long double>));
#endif

	BATCHED_SECTION("zero-initialization")
	{
		const auto v = vector_t{};
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK(v[i] == T{});
	}

	BATCHED_SECTION("fill constructor")
	{
		const auto val = random<T>(1, 10);
		vector_t vec{ val };
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK(val == vec[i]);
	}

	BATCHED_SECTION("copy constructor")
	{
		vector_t v1;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			v1[i] = random<T>(1, 10);
		vector_t v2{ v1 };
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK(v1[i] == v2[i]);
	}

	BATCHED_SECTION("blitting constructor")
	{
		blittable<T, vector_t::dimensions> v1;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			v1.values[i] = random<T>(1, 10);
		vector_t v2{ v1 };
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK(v1.values[i] == v2[i]);
	}

	BATCHED_SECTION("scalar constructor")
	{
		if constexpr (vector_t::dimensions > 3_sz)
			vector_construction_test_from_scalars<T, vector_t::dimensions, 3>();
		if constexpr (vector_t::dimensions > 1_sz)
			vector_construction_test_from_scalars<T, vector_t::dimensions, vector_t::dimensions>();
	}

	BATCHED_SECTION("array constructor")
	{
		if constexpr (vector_t::dimensions > 3_sz)
			vector_construction_test_from_array<T, vector_t::dimensions, 3>();
		vector_construction_test_from_array<T, vector_t::dimensions, vector_t::dimensions>();
	}

	BATCHED_SECTION("coercing constructor")
	{
		using other_type = std::
			conditional_t<is_same_as_any<T, signed int, unsigned int>, float, set_signed<signed int, is_signed<T>>>;

		vector<other_type, vector_t::dimensions> other;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			other[i] = random<other_type>(i);

		vector_t coerced{ other };
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK(coerced[i] == static_cast<T>(other[i]));
	}

	BATCHED_SECTION("enlarging constructor")
	{
		if constexpr (vector_t::dimensions > 1_sz)
			vector_construction_test_from_smaller_vector<T, vector_t::dimensions, vector_t::dimensions - 1_sz>();
		if constexpr (vector_t::dimensions > 3_sz)
			vector_construction_test_from_smaller_vector<T, vector_t::dimensions, vector_t::dimensions - 3_sz>();
	}

	BATCHED_SECTION("truncating constructor")
	{
		vector_construction_test_from_larger_vector<T, vector_t::dimensions, vector_t::dimensions + 2_sz>();
	}

	// pair constructor
	if constexpr (vector_t::dimensions >= 2)
	{
		BATCHED_SECTION("constructing from a std::pair")
		{
			auto values = std::pair{ random<T>(1, 10), random<T>(1, 10) };
			vector_t vec{ values };
			CHECK(vec[0] == std::get<0>(values));
			CHECK(vec[1] == std::get<1>(values));
			if constexpr (vector_t::dimensions > 2)
				CHECK(vec[2] == T{});
		}
	}

	// tuple constructor (3 elems)
	if constexpr (vector_t::dimensions >= 3)
	{
		BATCHED_SECTION("constructing from a std::tuple")
		{
			auto values = std::tuple{ random<T>(1, 10), random<T>(1, 10), random<T>(1, 10) };
			vector_t vec{ values };
			CHECK(vec[0] == std::get<0>(values));
			CHECK(vec[1] == std::get<1>(values));
			CHECK(vec[2] == std::get<2>(values));
			if constexpr (vector_t::dimensions > 3)
				CHECK(vec[3] == T{});
		}
	}

	if constexpr (vector_t::dimensions >= 3)
	{
		BATCHED_SECTION("concatenating constructor (xy, z)")
		{
			const vector<T, 2> xy{ random_array<T, 2>(1, 10) };
			const auto z = random<T>(1, 10);
			const vector_t vec{ xy, z };
			CHECK(vec[0] == xy[0]);
			CHECK(vec[1] == xy[1]);
			CHECK(vec[2] == z);
			for (size_t i = 3; i < vector_t::dimensions; i++)
				CHECK(vec[i] == T{});
		}
	}

	if constexpr (vector_t::dimensions >= 4)
	{
		const vector<T, 2> xy{ random_array<T, 2>(1, 10) };
		const vector<T, 2> zw{ random_array<T, 2>(1, 10) };

		BATCHED_SECTION("concatenating constructor (xy, zw)")
		{
			const vector_t vec{ xy, zw };
			CHECK(vec[0] == xy[0]);
			CHECK(vec[1] == xy[1]);
			CHECK(vec[2] == zw[0]);
			CHECK(vec[3] == zw[1]);
			for (size_t i = 4; i < vector_t::dimensions; i++)
				CHECK(vec[i] == T{});
		}

		BATCHED_SECTION("appending constructor (xy, z, w)")
		{
			const vector_t vec{ xy, zw[0], zw[1] };
			CHECK(vec[0] == xy[0]);
			CHECK(vec[1] == xy[1]);
			CHECK(vec[2] == zw[0]);
			CHECK(vec[3] == zw[1]);
			for (size_t i = 4; i < vector_t::dimensions; i++)
				CHECK(vec[i] == T{});
		}
	}
}

TEMPLATE_BATCHED_TEST("vector accessors", vectors<ALL_ARITHMETIC>)
{
	using vector_t = TestType;
	using T		   = typename vector_t::scalar_type;
	TEST_INFO("vector<"sv << nameof<T> << ", "sv << vector_t::dimensions << ">"sv);

	const auto x = random_array<T, vector_t::dimensions>(1, 10);
	vector_t vec{ x };
	const vector_t& vec_const = vec;

	BATCHED_SECTION("operator[]")
	{
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK(x[i] == vec[i]);
	}

	BATCHED_SECTION("get()")
	{
		CHECK(x[0] == vec.template get<0>());
		if constexpr (vector_t::dimensions >= 2)
			CHECK(x[1] == vec.template get<1>());
		if constexpr (vector_t::dimensions >= 3)
			CHECK(x[2] == vec.template get<2>());
		if constexpr (vector_t::dimensions >= 4)
			CHECK(x[3] == vec.template get<3>());
		if constexpr (vector_t::dimensions >= 5)
			CHECK(x[4] == vec.template get<4>());
	}

	BATCHED_SECTION("operator[] (const)")
	{
		for (size_t i = 0; i < vector_t::dimensions; i++)
		{
			CHECK(x[i] == vec_const[i]);
			CHECK(&vec[i] == &vec_const[i]);
		}
	}

	BATCHED_SECTION("get() (const)")
	{
		CHECK(x[0] == vec_const.template get<0>());
		if constexpr (vector_t::dimensions >= 2)
			CHECK(x[1] == vec_const.template get<1>());
		if constexpr (vector_t::dimensions >= 3)
			CHECK(x[2] == vec_const.template get<2>());
		if constexpr (vector_t::dimensions >= 4)
			CHECK(x[3] == vec_const.template get<3>());
		if constexpr (vector_t::dimensions >= 5)
			CHECK(x[4] == vec_const.template get<4>());
		CHECK(&vec.template get<0>() == &vec_const[0]);
		if constexpr (vector_t::dimensions >= 2)
			CHECK(&vec.template get<1>() == &vec_const[1]);
		if constexpr (vector_t::dimensions >= 3)
			CHECK(&vec.template get<2>() == &vec_const[2]);
		if constexpr (vector_t::dimensions >= 4)
			CHECK(&vec.template get<3>() == &vec_const[3]);
		if constexpr (vector_t::dimensions >= 5)
			CHECK(&vec.template get<4>() == &vec_const[4]);
	}

	BATCHED_SECTION("ranged-for")
	{
		size_t i = 0;
		for (auto& val : vec)
		{
			CHECK(x[i] == val);
			CHECK(&vec[i] == &val);
			i++;
		}
		CHECK(i == vector_t::dimensions);
	}

	BATCHED_SECTION("ranged-for (const)")
	{
		size_t i = 0;
		for (auto& val : vec_const)
		{
			CHECK(x[i] == val);
			CHECK(&vec[i] == &val);
			CHECK(&vec_const[i] == &val);
			i++;
		}
		CHECK(i == vector_t::dimensions);
	}

	if constexpr (vector_t::dimensions <= 4)
	{
		BATCHED_SECTION("named members")
		{
			CHECK(&vec.x == &vec.template get<0>());
			if constexpr (vector_t::dimensions >= 2)
				CHECK(&vec.y == &vec.template get<1>());
			if constexpr (vector_t::dimensions >= 3)
				CHECK(&vec.z == &vec.template get<2>());
			if constexpr (vector_t::dimensions >= 4)
				CHECK(&vec.w == &vec.template get<3>());
		}
	}
}

TEMPLATE_BATCHED_TEST("vector equality", vectors<ALL_ARITHMETIC>)
{
	using vector_t = TestType;
	using T		   = typename vector_t::scalar_type;
	TEST_INFO("vector<"sv << nameof<T> << ", "sv << vector_t::dimensions << ">"sv);

	vector_t vec;
	for (size_t i = 0; i < vector_t::dimensions; i++)
		vec[i] = random<T>(1, 10);

	BATCHED_SECTION("same type")
	{
		vector_t same{ vec };
		CHECK_SYMMETRIC_EQUAL(vec, same);
		if constexpr (is_floating_point<T>)
		{
			CHECK(vector_t::approx_equal(vec, same));
			CHECK(vec.approx_equal(same));
			CHECK(muu::approx_equal(vec, same));
		}

		vector_t different{ vec };
		for (size_t i = 0; i < vector_t::dimensions; i++)
			different[i]++;
		CHECK_SYMMETRIC_INEQUAL(vec, different);
		if constexpr (is_floating_point<T>)
		{
			CHECK_FALSE(vector_t::approx_equal(vec, different));
			CHECK_FALSE(vec.approx_equal(different));
			CHECK_FALSE(muu::approx_equal(vec, different));
		}
	}

	if constexpr (!is_floating_point<T>)
	{
		BATCHED_SECTION("different type")
		{
			using other_t = vector<std::conditional_t<std::is_same_v<T, long>, int, long>, vector_t::dimensions>;

			other_t same{ vec };
			CHECK_SYMMETRIC_EQUAL(vec, same);

			other_t different{ vec };
			for (size_t i = 0; i < vector_t::dimensions; i++)
				different[i]++;
			CHECK_SYMMETRIC_INEQUAL(vec, different);
		}
	}
}

TEMPLATE_BATCHED_TEST("vector zero", vectors<ALL_ARITHMETIC>)
{
	using vector_t = TestType;
	using T		   = typename vector_t::scalar_type;
	TEST_INFO("vector<"sv << nameof<T> << ", "sv << vector_t::dimensions << ">"sv);

	BATCHED_SECTION("all zeroes")
	{
		vector_t vec{ T{} };

		CHECK(vec.zero());
		if constexpr (is_floating_point<T>)
		{
			CHECK(vector_t::approx_zero(vec));
			CHECK(vec.approx_zero());
			CHECK(muu::approx_zero(vec));
		}
	}

	BATCHED_SECTION("no zeroes")
	{
		vector_t vec;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			vec[i] = random<T>(1, 10);

		CHECK_FALSE(vec.zero());
		if constexpr (is_floating_point<T>)
		{
			CHECK_FALSE(vector_t::approx_zero(vec));
			CHECK_FALSE(vec.approx_zero());
			CHECK_FALSE(muu::approx_zero(vec));
		}
	}

	if constexpr (vector_t::dimensions > 1)
	{
		BATCHED_SECTION("some zeroes")
		{
			vector_t vec{ T{ 1 } };
			for (size_t i = 0; i < vector_t::dimensions; i += 2)
				vec[i] = T{};

			CHECK_FALSE(vec.zero());
			if constexpr (is_floating_point<T>)
			{
				CHECK_FALSE(vector_t::approx_zero(vec));
				CHECK_FALSE(vec.approx_zero());
				CHECK_FALSE(muu::approx_zero(vec));
			}
		}
	}

	BATCHED_SECTION("one zero")
	{
		for (size_t i = 0; i < vector_t::dimensions; i++)
		{
			vector_t vec{ T{} };
			vec[i] = random<T>(1, 10);

			CHECK_FALSE(vec.zero());
			if constexpr (is_floating_point<T>)
			{
				CHECK_FALSE(vector_t::approx_zero(vec));
				CHECK_FALSE(vec.approx_zero());
				CHECK_FALSE(muu::approx_zero(vec));
			}
		}
	}
}

TEMPLATE_BATCHED_TEST("vector infinity_or_nan", vectors<ALL_ARITHMETIC>)
{
	using vector_t = TestType;
	using T		   = typename vector_t::scalar_type;
	TEST_INFO("vector<"sv << nameof<T> << ", "sv << vector_t::dimensions << ">"sv);

	vector_t vector1;
	for (size_t i = 0; i < vector_t::dimensions; i++)
		vector1[i] = static_cast<T>(i);

	BATCHED_SECTION("all finite")
	{
		CHECK_FALSE(vector1.infinity_or_nan());
		CHECK_FALSE(vector_t::infinity_or_nan(vector1));
		CHECK_FALSE(muu::infinity_or_nan(vector1));
	}

	if constexpr (is_floating_point<T>)
	{
		BATCHED_SECTION("contains one NaN")
		{
			for (size_t i = 0; i < vector_t::dimensions; i++)
			{
				vector_t vector2{ vector1 };
				vector2[i] = make_nan<T>();
				CHECK(vector2.infinity_or_nan());
				CHECK(vector_t::infinity_or_nan(vector2));
				CHECK(muu::infinity_or_nan(vector2));
			}
		}

		BATCHED_SECTION("contains one infinity")
		{
			for (size_t i = 0; i < vector_t::dimensions; i++)
			{
				vector_t vector2{ vector1 };
				vector2[i] = make_infinity<T>();
				CHECK(vector2.infinity_or_nan());
				CHECK(vector_t::infinity_or_nan(vector2));
				CHECK(muu::infinity_or_nan(vector2));
			}
		}
	}
}

TEMPLATE_BATCHED_TEST("vector dot", vectors<ALL_ARITHMETIC>)
{
	using vector_t = TestType;
	using T		   = typename vector_t::scalar_type;
	TEST_INFO("vector<"sv << nameof<T> << ", "sv << vector_t::dimensions << ">"sv);

	const auto x1 = random_array<T, vector_t::dimensions>(0, 5);
	const auto x2 = random_array<T, vector_t::dimensions>(0, 5);
	const vector_t vector1{ x1 }, vector2{ x2 };

	using dot_type = decltype(vector1.dot(vector2));

	// expected result
	// (accumulating in a potentially more precise intermediate type then coverting the result is what happens in
	// the vector class to minimize loss in float16, so that same behaviour is replicated here)
	using intermediate_type = impl::promote_if_small_float<dot_type>;
	auto expected_sum		= intermediate_type{};
	for (size_t i = 0; i < vector_t::dimensions; i++)
	{
		MUU_FMA_BLOCK;
		expected_sum += static_cast<intermediate_type>(x1[i]) * static_cast<intermediate_type>(x2[i]);
	}
	const auto expected = static_cast<dot_type>(expected_sum);

	CHECK_APPROX_EQUAL(vector1.dot(vector2), expected);
	CHECK_APPROX_EQUAL(vector_t::dot(vector1, vector2), expected);
	CHECK_APPROX_EQUAL(muu::dot(vector1, vector2), expected);
}

TEMPLATE_BATCHED_TEST("vector cross", vectors_N<3, ALL_FLOATS>)
{
	using vector_t = TestType;
	using T		   = typename vector_t::scalar_type;
	TEST_INFO("vector<"sv << nameof<T> << ", "sv << vector_t::dimensions << ">"sv);

	using promoted = impl::highest_ranked<T, double>;
	using vec3d	   = vector<promoted, 3>;

	// everything here is double or higher EXCEPT the calls to vector::cross()
	// (since that's the thing we're actually testing). Otherwise fp errors cause this to yield false negatives
	// with fp16 etc.

	auto vec1 = vec3d{ static_cast<promoted>(1.1), static_cast<promoted>(4.5), static_cast<promoted>(9.8) };
	auto vec2 = vec3d{ static_cast<promoted>(-1.4), static_cast<promoted>(9.5), static_cast<promoted>(3.2) };
	vec1.normalize();
	vec2.normalize();
	CHECK(vec1.normalized());
	CHECK(vec2.normalized());

	static const auto eps = static_cast<promoted>(constants<T>::default_epsilon);

	{
		INFO("vector.cross(vector)")

		const auto cross_vector = vec3d{ vector_t{ vec1 }.cross(vector_t{ vec2 }) };
		const auto vec1_dot		= cross_vector.dot(vec3d{ vec1 });
		const auto vec2_dot		= cross_vector.dot(vec3d{ vec2 });
		CHECK_APPROX_EQUAL_EPS(vec1_dot, promoted{}, eps);
		CHECK_APPROX_EQUAL_EPS(vec2_dot, promoted{}, eps);
	}

	{
		INFO("vector::cross(vector, vector)")

		const auto cross_vector = vec3d{ vector_t::cross(vector_t{ vec1 }, vector_t{ vec2 }) };
		const auto vec1_dot		= cross_vector.dot(vec3d{ vec1 });
		const auto vec2_dot		= cross_vector.dot(vec3d{ vec2 });
		CHECK_APPROX_EQUAL_EPS(vec1_dot, promoted{}, eps);
		CHECK_APPROX_EQUAL_EPS(vec2_dot, promoted{}, eps);
	}

	{
		INFO("muu::cross(vector, vector)")

		const auto cross_vector = muu::cross(vec1, vec2);
		const auto vec1_dot		= cross_vector.dot(vec3d{ vec1 });
		const auto vec2_dot		= cross_vector.dot(vec3d{ vec2 });
		CHECK_APPROX_EQUAL_EPS(vec1_dot, promoted{}, eps);
		CHECK_APPROX_EQUAL_EPS(vec2_dot, promoted{}, eps);
	}
}

TEMPLATE_BATCHED_TEST("vector addition", vectors<ALL_ARITHMETIC>)
{
	using vector_t = TestType;
	using T		   = typename vector_t::scalar_type;
	TEST_INFO("vector<"sv << nameof<T> << ", "sv << vector_t::dimensions << ">"sv);

	const auto x1 = random_array<T, vector_t::dimensions>(0, 5);
	const auto x2 = random_array<T, vector_t::dimensions>(0, 5);
	const vector_t vector1{ x1 }, vector2{ x2 };

	BATCHED_SECTION("vector + vector")
	{
		vector_t result = vector1 + vector2;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK_APPROX_EQUAL(static_cast<T>(x1[i] + x2[i]), result[i]);
	}

	BATCHED_SECTION("vector += vector")
	{
		vector_t result(vector1);
		result += vector2;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK_APPROX_EQUAL(static_cast<T>(x1[i] + x2[i]), result[i]);
	}
}

TEMPLATE_BATCHED_TEST("vector subtraction", vectors<ALL_ARITHMETIC>)
{
	using vector_t = TestType;
	using T		   = typename vector_t::scalar_type;
	TEST_INFO("vector<"sv << nameof<T> << ", "sv << vector_t::dimensions << ">"sv);

	const auto x1 =
		is_signed<T> ? random_array<T, vector_t::dimensions>(0, 10) : random_array<T, vector_t::dimensions>(11, 20);
	const auto x2 = random_array<T, vector_t::dimensions>(0, 10);
	const vector_t vector1{ x1 }, vector2{ x2 };

	BATCHED_SECTION("vector - vector")
	{
		const vector_t result = vector1 - vector2;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK_APPROX_EQUAL(static_cast<T>(x1[i] - x2[i]), result[i]);
	}

	BATCHED_SECTION("vector -= vector")
	{
		vector_t result(vector1);
		result -= vector2;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK_APPROX_EQUAL(static_cast<T>(x1[i] - x2[i]), result[i]);
	}

	if constexpr (is_signed<T>)
	{
		BATCHED_SECTION("-vector")
		{
			const vector_t result = -vector1;
			for (size_t i = 0; i < vector_t::dimensions; i++)
				CHECK_APPROX_EQUAL(static_cast<T>(-x1[i]), result[i]);
		}
	}
}

TEMPLATE_BATCHED_TEST("vector multiplication", vectors<ALL_ARITHMETIC>)
{
	using vector_t = TestType;
	using T		   = typename vector_t::scalar_type;
	TEST_INFO("vector<"sv << nameof<T> << ", "sv << vector_t::dimensions << ">"sv);

	const auto scalar = static_cast<T>(2.4);
	const auto x1	  = random_array<T, vector_t::dimensions>(0, 5);
	const auto x2	  = random_array<T, vector_t::dimensions>(0, 5);
	const vector_t vector1{ x1 }, vector2{ x2 };

	BATCHED_SECTION("vector * vector")
	{
		vector_t result = vector1 * vector2;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK_APPROX_EQUAL(static_cast<T>(x1[i] * x2[i]), result[i]);
	}

	BATCHED_SECTION("vector *= vector")
	{
		vector_t result(vector1);
		result *= vector2;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK_APPROX_EQUAL(static_cast<T>(x1[i] * x2[i]), result[i]);
	}

	BATCHED_SECTION("vector * scalar")
	{
		vector_t result = vector1 * scalar;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK_APPROX_EQUAL(static_cast<T>(x1[i] * scalar), result[i]);
	}

	BATCHED_SECTION("scalar * vector")
	{
		vector_t result = scalar * vector2;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK_APPROX_EQUAL(static_cast<T>(x2[i] * scalar), result[i]);
	}

	BATCHED_SECTION("vector *= scalar")
	{
		vector_t result(vector1);
		result *= scalar;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK_APPROX_EQUAL(static_cast<T>(x1[i] * scalar), result[i]);
	}
}

TEMPLATE_BATCHED_TEST("vector division", vectors<ALL_ARITHMETIC>)
{
	using vector_t = TestType;
	using T		   = typename vector_t::scalar_type;
	TEST_INFO("vector<"sv << nameof<T> << ", "sv << vector_t::dimensions << ">"sv);

	const auto scalar = static_cast<T>(2.4);
	const vector_t vec1{ random_array<T, vector_t::dimensions>(2, 5) };
	const vector_t vec2{ random_array<T, vector_t::dimensions>(2, 5) };

	BATCHED_SECTION("vector / vector")
	{
		vector_t result = vec1 / vec2;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK_APPROX_EQUAL(static_cast<T>(vec1[i] / vec2[i]), result[i]);
	}

	BATCHED_SECTION("vector /= vector")
	{
		vector_t result = vec1;
		result /= vec2;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK_APPROX_EQUAL(static_cast<T>(vec1[i] / vec2[i]), result[i]);
	}

	BATCHED_SECTION("vector / scalar")
	{
		vector_t result = vec1 / scalar;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK_APPROX_EQUAL(static_cast<T>(vec1[i] / scalar), result[i]);
	}

	BATCHED_SECTION("vector /= scalar")
	{
		vector_t result = vec1;
		result /= scalar;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK_APPROX_EQUAL(static_cast<T>(vec1[i] / scalar), result[i]);
	}
}

TEMPLATE_BATCHED_TEST("vector modulo", vectors<ALL_ARITHMETIC>)
{
	using vector_t = TestType;
	using T		   = typename vector_t::scalar_type;
	TEST_INFO("vector<"sv << nameof<T> << ", "sv << vector_t::dimensions << ">"sv);

	const auto scalar = static_cast<T>(2.4);
	const vector_t vec1{ random_array<T, vector_t::dimensions>(2, 5) };
	const vector_t vec2{ random_array<T, vector_t::dimensions>(2, 5) };

	BATCHED_SECTION("vector % vector")
	{
		vector_t result = vec1 % vec2;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK_APPROX_EQUAL(static_cast<T>(impl::raw_modulo(vec1[i], vec2[i])), result[i]);
	}

	BATCHED_SECTION("vector %= vector")
	{
		vector_t result = vec1;
		result %= vec2;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK_APPROX_EQUAL(static_cast<T>(impl::raw_modulo(vec1[i], vec2[i])), result[i]);
	}

	BATCHED_SECTION("vector % scalar")
	{
		vector_t result = vec1 % scalar;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK_APPROX_EQUAL(static_cast<T>(impl::raw_modulo(vec1[i], scalar)), result[i]);
	}

	BATCHED_SECTION("vector %= scalar")
	{
		vector_t result = vec1;
		result %= scalar;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK_APPROX_EQUAL(static_cast<T>(impl::raw_modulo(vec1[i], scalar)), result[i]);
	}
}

TEMPLATE_BATCHED_TEST("vector bitwise shifts", vectors<ALL_INTS>)
{
	using vector_t = TestType;
	using T		   = typename vector_t::scalar_type;
	TEST_INFO("vector<"sv << nameof<T> << ", "sv << vector_t::dimensions << ">"sv);

	const int shift = random<int>(5);
	const vector_t original{ T{ 1 } };
	const vector_t shifted{ static_cast<T>(T{ 1 } << shift) };

	BATCHED_SECTION("vector << int")
	{
		vector_t result = original << shift;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK(result[i] == shifted[i]);
	}

	BATCHED_SECTION("vector <<= int")
	{
		vector_t result = original;
		result <<= shift;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK(result[i] == shifted[i]);
	}

	BATCHED_SECTION("vector >> int")
	{
		vector_t result = shifted >> shift;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK(result[i] == original[i]);
	}

	BATCHED_SECTION("vector >>= int")
	{
		vector_t result = shifted;
		result >>= shift;
		for (size_t i = 0; i < vector_t::dimensions; i++)
			CHECK(result[i] == original[i]);
	}
}

TEMPLATE_BATCHED_TEST("vector normalization", vectors<ALL_FLOATS>)
{
	using vector_t = TestType;
	using T		   = typename vector_t::scalar_type;
	TEST_INFO("vector<"sv << nameof<T> << ", "sv << vector_t::dimensions << ">"sv);

	const vector_t x{ random_array<T, vector_t::dimensions>(2, 10) };

	BATCHED_SECTION("vector.normalize()")
	{
		vector_t vec{ x };
		vec.normalize();
		CHECK(vec.normalized());
		CHECK(vec.length() == approx(T{ 1 }));
	}

	BATCHED_SECTION("vector::normalize(vector)")
	{
		const auto vec = vector_t::normalize(x);
		CHECK(vec.normalized());
		CHECK(vec.length() == approx(T{ 1 }));
	}

	BATCHED_SECTION("muu::normalize(vector)")
	{
		const auto vec = muu::normalize(x);
		CHECK(vec.normalized());
		CHECK(vec.length() == approx(T{ 1 }));
	}
}

TEMPLATE_BATCHED_TEST("vector length", vectors_NNN<2, 3, 4, float>)
{
	using vector_t = TestType;
	using T		   = typename vector_t::scalar_type;
	TEST_INFO("vector<"sv << nameof<T> << ", "sv << vector_t::dimensions << ">"sv);

	if constexpr (vector_t::dimensions == 2)
	{
		const vector<T, 2> a{ 0, 10 };
		const vector<T, 2> b{ 15, 12 };
		const float distance = a.distance(b);
		CHECK(distance == approx(std::sqrt(15.0f * 15.0f + 2.0f * 2.0f)));
	}

	else if constexpr (vector_t::dimensions == 3)
	{
		const vector<T, 3> a{ 0, 10, 3 };
		const vector<T, 3> b{ 15, 12, -4 };
		const float distance = a.distance(b);
		CHECK(distance == approx(std::sqrt(15.0f * 15.0f + 2.0f * 2.0f + 7.0f * 7.0f)));
	}

	else if constexpr (vector_t::dimensions == 4)
	{
		const vector<T, 4> a{ 9, 10, 3, 5 };
		const vector<T, 4> b{ 15, 12, -4, 1 };
		const float distance = a.distance(b);
		CHECK(distance == approx(std::sqrt(6.0f * 6.0f + 2.0f * 2.0f + 7.0f * 7.0f + 4.0f * 4.0f)));
	}
}

namespace
{
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
		static constexpr auto name				   = "general"sv;
		using alpha_type						   = typename lerp_test_case<T>::alpha_type;
		using c									   = muu::constants<T>;
		using a									   = muu::constants<alpha_type>;
		static constexpr lerp_test_case<T> cases[] = {
			/* 0 */ { c::zero, c::one, a::two, c::two },
			/* 1 */ { c::one, c::two, a::one, c::two },
			/* 2 */ { c::one, c::two, a::two, c::three },
			/* 3 */ { c::one, c::two, a::zero, c::one },
			/* 4 */ { c::one, c::one, a::two, c::one },
		};
	};

	template <typename T>
	struct lerp_signed_test_data
	{
		static constexpr auto name				   = "signed"sv;
		using alpha_type						   = typename lerp_test_case<T>::alpha_type;
		using c									   = muu::constants<T>;
		using a									   = muu::constants<alpha_type>;
		static constexpr lerp_test_case<T> cases[] = {
			/* 0 */ { -c::one, c::one, a::two, c::three },
			/* 1 */ { -c::one, c::zero, a::two, c::one },
			/* 2 */ { c::one, -c::one, a::two, -c::three },
			/* 3 */ { c::zero, -c::one, a::two, -c::two },
			/* 4 */ { c::one, c::zero, a::two, -c::one },
		};
	};

	template <typename T>
	struct lerp_float_test_data
	{
		static constexpr auto name				   = "floating-point"sv;
		using alpha_type						   = typename lerp_test_case<T>::alpha_type;
		using c									   = muu::constants<T>;
		using a									   = muu::constants<alpha_type>;
		static constexpr lerp_test_case<T> cases[] = {
			/* 0 */ { c::negative_zero, c::negative_zero, a::one_over_two, c::negative_zero },
			/* 1 */ { -c::five, c::five, a::one_over_two, c::zero },
			/* 2 */ { c::one, c::two, a::one_over_two, c::three_over_two },
			/* 3 */ { c::zero, c::zero, a::one_over_two, c::zero },
		};
	};
}

template <typename T, size_t Dimensions, typename Dataset>
inline void vector_lerp_specific_tests() noexcept
{
	TEST_INFO("lerp test dataset: "sv << Dataset::name);
	using vector_t = vector<T, Dimensions>;

	size_t i{};
	for (const auto& case_ : Dataset::cases)
	{
		TEST_INFO("test case " << i++);

		{
			TEST_INFO("vector::lerp(start, finish, expected)"sv);
			const auto vec = vector_t::lerp(vector_t{ case_.start }, vector_t{ case_.finish }, case_.alpha);
			CHECK(vec == vector_t{ case_.expected });
		}

		{
			TEST_INFO("vector.lerp(target, expected)"sv);
			auto vec = vector_t{ case_.start };
			vec.lerp(vector_t{ case_.finish }, case_.alpha);
			CHECK(vec == vector_t{ case_.expected });
		}

		{
			TEST_INFO("muu::lerp(start, finish, expected)"sv);
			const auto vec = muu::lerp(vector_t{ case_.start }, vector_t{ case_.finish }, case_.alpha);
			CHECK(vec == vector_t{ case_.expected });
		}
	}
}

TEMPLATE_BATCHED_TEST("vector lerp", vectors<ALL_ARITHMETIC>)
{
	using vector_t = TestType;
	using T		   = typename vector_t::scalar_type;
	TEST_INFO("vector<"sv << nameof<T> << ", "sv << vector_t::dimensions << ">"sv);

	vector_lerp_specific_tests<T, vector_t::dimensions, lerp_test_data<T>>();

	if constexpr (is_signed<T>)
		vector_lerp_specific_tests<T, vector_t::dimensions, lerp_signed_test_data<T>>();

	if constexpr (is_floating_point<T>)
		vector_lerp_specific_tests<T, vector_t::dimensions, lerp_float_test_data<T>>();
}

TEMPLATE_BATCHED_TEST("vector min/max", vectors<ALL_ARITHMETIC>)
{
	using vector_t = TestType;
	using T		   = typename vector_t::scalar_type;
	TEST_INFO("vector<"sv << nameof<T> << ", "sv << vector_t::dimensions << ">"sv);

	const vector_t zeroes{ T{} };				   // {  0,  0,  0, ...}
	vector_t sequential;						   // {  1,  2,  3, ...}
	[[maybe_unused]] vector_t sequential_negative; // { -1, -2, -3, ...} (signed only)
	vector_t interleaved1{ zeroes };			   // {  1,  0,  3, ...}
	vector_t interleaved2{ zeroes };			   // {  0,  2,  0, ...}
	for (size_t i = 0; i < vector_t::dimensions; i++)
	{
		sequential[i] = static_cast<T>(i + 1u);
		if constexpr (is_signed<T>)
			sequential_negative[i] = static_cast<T>(-sequential[i]);
		(*(i % 2u == 0 ? &interleaved1 : &interleaved2))[i] = sequential[i];
	}

	BATCHED_SECTION("(zeroes, *)")
	{
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
	}

	BATCHED_SECTION("(sequential, *)")
	{
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
	}

	if constexpr (is_signed<T>)
	{
		BATCHED_SECTION("(sequential_negative, *)")
		{
			CHECK(vector_t::min(sequential_negative, sequential_negative) == sequential_negative);
			CHECK(vector_t::max(sequential_negative, sequential_negative) == sequential_negative);
			CHECK(vector_t::min(sequential_negative, interleaved1) == sequential_negative);
			CHECK(vector_t::max(sequential_negative, interleaved1) == interleaved1);
			CHECK(vector_t::min(sequential_negative, interleaved2) == sequential_negative);
			CHECK(vector_t::max(sequential_negative, interleaved2) == interleaved2);
		}
	}

	BATCHED_SECTION("(interleaved1, *)")
	{
		CHECK(vector_t::min(interleaved1, interleaved1) == interleaved1);
		CHECK(vector_t::max(interleaved1, interleaved1) == interleaved1);
		CHECK(vector_t::min(interleaved1, interleaved2) == zeroes);
		CHECK(vector_t::max(interleaved1, interleaved2) == sequential);
	}

	BATCHED_SECTION("(interleaved2, *)")
	{
		CHECK(vector_t::min(interleaved2, interleaved2) == interleaved2);
		CHECK(vector_t::max(interleaved2, interleaved2) == interleaved2);
	}
}

TEMPLATE_BATCHED_TEST("vector angle", vectors_NN<2, 3, ALL_ARITHMETIC>)
{
	using vector_t = TestType;
	using T		   = typename vector_t::scalar_type;
	TEST_INFO("vector<"sv << nameof<T> << ", "sv << vector_t::dimensions << ">"sv);

	using delta_type	= typename vector_t::delta_type;
	using constant_type = impl::highest_ranked<delta_type, float>;

	[[maybe_unused]] static const auto eps = static_cast<delta_type>(
		(muu::max)(static_cast<long double>(constants<delta_type>::default_epsilon), 0.000000001L));

#define CHECK_ANGLE(val)                                                                                               \
	CHECK_APPROX_EQUAL_EPS(a.angle(b), static_cast<delta_type>(val), eps);                                             \
	CHECK_APPROX_EQUAL_EPS(b.angle(a), static_cast<delta_type>(val), eps)

	if constexpr (vector_t::dimensions == 2)
	{
		{
			// a
			// |__ b

			const vector_t a{ T{}, T(1) };
			const vector_t b{ T(1), T{} };
			CHECK_ANGLE(constants<constant_type>::pi_over_two);
		}

		if constexpr (is_signed<T>)
		{
			// a __ __ b

			const vector_t a{ T(-1), T{} };
			const vector_t b{ T(1), T{} };
			CHECK_ANGLE(constants<constant_type>::pi);
		}

		if constexpr (is_signed<T>)
		{
			//  __ a
			// |
			// b

			const vector_t a{ T(1), T{} };
			const vector_t b{ T{}, T(-1) };
			CHECK_ANGLE(constants<constant_type>::pi_over_two);
		}

		if constexpr (is_signed<T>)
		{
			// a
			//  \ __ b
			const vector_t a{ T(-1), T(1) };
			const vector_t b{ T(1), T{} };
			CHECK_ANGLE(constants<constant_type>::three_pi_over_four);
		}
	}
	else if constexpr (vector_t::dimensions == 3)
	{
		{
			const vector_t a{ T{}, T{}, T(1) };
			const vector_t b{ T{}, T(1), T{} };
			CHECK_ANGLE(constants<constant_type>::pi_over_two);
		}

		if constexpr (is_signed<T>)
		{
			const vector_t a{ T(1), T(2), T(3) };
			const vector_t b{ T(-10), T(3), T(-1) };
			CHECK_ANGLE(1.75013258616261269118297150271L);
		}

		if constexpr (is_signed<T>)
		{
			const vector_t a{ T(1), T(2), T(3) };
			const vector_t b{ T(-1), T(-2), T(-3) };
			CHECK_ANGLE(constants<constant_type>::pi);
		}
	}
}

TEMPLATE_BATCHED_TEST("vector accumulator", vectors<ALL_ARITHMETIC>)
{
	using vector_t = TestType;
	using T		   = typename vector_t::scalar_type;
	TEST_INFO("vector<"sv << nameof<T> << ", "sv << vector_t::dimensions << ">"sv);

	static constexpr size_t vectors = 100;
	const auto values				= random_array<T, vectors * vector_t::dimensions>(1, 10);
	muu::accumulator<T> scalar_accumulators[vector_t::dimensions];
	muu::accumulator<vector<T, vector_t::dimensions>> vector_accumulator;
	for (size_t i = 0; i < values.size(); i += vector_t::dimensions)
	{
		vector_t v;
		for (size_t d = 0; d < vector_t::dimensions; d++)
		{
			v[d] = values[i + d];
			scalar_accumulators[d].add(values[i + d]);
		}
		vector_accumulator.add(v);
	}

	CHECK(vector_accumulator.sample_count() == vectors);

	const auto vector_min = vector_accumulator.min();
	const auto vector_max = vector_accumulator.max();
	const auto vector_sum = vector_accumulator.sum();
	for (size_t d = 0; d < vector_t::dimensions; d++)
	{
		CHECK(vector_min[d] == scalar_accumulators[d].min());
		CHECK(vector_max[d] == scalar_accumulators[d].max());
		CHECK(vector_sum[d] == scalar_accumulators[d].sum());
	}
}
