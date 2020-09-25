// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/vector.h"
#include "../include/muu/half.h"

#define TEST_TYPE(func, T)						\
		func<T, 1>(MUU_MAKE_STRING_VIEW(T));	\
		func<T, 2>(MUU_MAKE_STRING_VIEW(T));	\
		func<T, 3>(MUU_MAKE_STRING_VIEW(T));	\
		func<T, 4>(MUU_MAKE_STRING_VIEW(T));	\
		func<T, 5>(MUU_MAKE_STRING_VIEW(T))

#if MUU_HAS_FLOAT16
	#define TEST_FLOAT16(func)	TEST_TYPE(func, float16_t)
#else
	#define TEST_FLOAT16(func)	(void)0
#endif
#if MUU_HAS_INTERCHANGE_FP16
	#define TEST_FP16(func)		TEST_TYPE(func, __fp16)
#else
	#define TEST_FP16(func)		(void)0
#endif
#if MUU_HAS_FLOAT128
	#define TEST_FLOAT128(func)	TEST_TYPE(func, float128_t)
#else
	#define TEST_FLOAT128(func)	(void)0
#endif

#define TEST_FLOATS(func)			\
	TEST_TYPE(func, half);			\
	TEST_TYPE(func, float);			\
	TEST_TYPE(func, double);		\
	TEST_TYPE(func, long double);	\
	TEST_FP16(func);				\
	TEST_FLOAT16(func);				\
	TEST_FLOAT128(func)


#if MUU_HAS_INT128
	#define TEST_INT128(func)	TEST_TYPE(func, int128_t)
#else
	#define TEST_INT128(func)	(void)0
#endif

#define TEST_INTS(func)					\
	TEST_TYPE(func, signed char);		\
	TEST_TYPE(func, signed short);		\
	TEST_TYPE(func, signed int);		\
	TEST_TYPE(func, signed long);		\
	TEST_TYPE(func, signed long long);	\
	TEST_INT128(func)

#define TEST_ALL_TYPES(func)	\
	TEST_FLOATS(func);			\
	TEST_INTS(func)

namespace
{
	template <typename T, size_t DIM>
	constexpr void trait_tests(std::string_view /*scalar_typename*/) noexcept
	{
		using vector_t = vector<T, DIM>;
		static_assert(sizeof(vector_t) == sizeof(T) * DIM);
		static_assert(std::is_standard_layout_v<vector_t>);
		static_assert(std::is_trivially_constructible_v<vector_t>);
		static_assert(std::is_trivially_copy_constructible_v<vector_t>);
		static_assert(std::is_trivially_copy_assignable_v<vector_t>);
		static_assert(std::is_trivially_move_constructible_v<vector_t>);
		static_assert(std::is_trivially_move_assignable_v<vector_t>);
		static_assert(std::is_trivially_destructible_v<vector_t>);
		static_assert(std::is_nothrow_constructible_v<vector_t, T>);
		static_assert(DIM < 2 || std::is_nothrow_constructible_v<vector_t, T, T>);
		static_assert(DIM < 3 || std::is_nothrow_constructible_v<vector_t, T, T, T>);
		static_assert(DIM < 4 || std::is_nothrow_constructible_v<vector_t, T, T, T, T>);
		static_assert(DIM < 5 || std::is_nothrow_constructible_v<vector_t, T, T, T, T, T>);
	};

	[[maybe_unused]]
	static constexpr bool invoke_trait_tests = []() noexcept
	{
		TEST_ALL_TYPES(trait_tests);
		return true;
	}();

	template <typename Vector, size_t NUM>
	void construction_test_from_scalars() noexcept
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
	void construction_test_from_array() noexcept
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
	}

	template <typename Vector, size_t NUM>
	void construction_test_from_smaller_vector() noexcept
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

	template <typename Vector, size_t NUM = 10>
	void construction_test_from_larger_vector() noexcept
	{
		using scalar_type = typename Vector::scalar_type;
		INFO("constructing from a larger vector with "sv << NUM << " elements"sv)
		static_assert(NUM > Vector::dimensions);

		auto larger = vector<scalar_type, NUM>{ random_array<scalar_type, NUM>() };
		auto vec = Vector{ larger };
		for (size_t i = 0; i < Vector::dimensions; i++)
			CHECK(vec[i] == larger[i]);
	}

	template <typename T, size_t DIM>
	void construction_tests(std::string_view scalar_typename) noexcept
	{
		INFO("vector<"sv << scalar_typename << ", "sv << DIM << ">"sv)
		using vector_t = vector<T, DIM>;

		{
			INFO("fill constructor")

			const auto val = random(T{}, T{ 10 });
			vector_t vec{ val };
			for (size_t i = 0; i < DIM; i++)
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
		if constexpr (DIM >= 2) construction_test_from_scalars<vector_t, 2>();
		if constexpr (DIM >= 3) construction_test_from_scalars<vector_t, 3>();
		if constexpr (DIM >= 4) construction_test_from_scalars<vector_t, 4>();
		if constexpr (DIM >= 5) construction_test_from_scalars<vector_t, 5>();
		// no single-scalar test; it's the fill constructor.

		// array constructor
		construction_test_from_array<vector_t, 1>();
		if constexpr (DIM >= 2) construction_test_from_array<vector_t, 2>();
		if constexpr (DIM >= 3) construction_test_from_array<vector_t, 3>();
		if constexpr (DIM >= 4) construction_test_from_array<vector_t, 4>();
		if constexpr (DIM >= 5) construction_test_from_array<vector_t, 5>();

		// coercing constructor
		{
			using other_type = std::conditional_t<std::is_same_v<T, int>, float, int>;
			vector<other_type, DIM> other;
			for (size_t i = 0; i < DIM; i++)
				other[i] = static_cast<other_type>(i);
			vector<T, DIM> coerced{ other };
			for (size_t i = 0; i < DIM; i++)
				CHECK(coerced[i] == static_cast<T>(other[i]));
		}

		// enlarging constructor
		if constexpr (DIM > 1) construction_test_from_smaller_vector<vector_t, 1>();
		if constexpr (DIM > 2) construction_test_from_smaller_vector<vector_t, 2>();
		if constexpr (DIM > 3) construction_test_from_smaller_vector<vector_t, 3>();
		if constexpr (DIM > 4) construction_test_from_smaller_vector<vector_t, 4>();
		if constexpr (DIM > 5) construction_test_from_smaller_vector<vector_t, 5>();

		// truncating constructor
		construction_test_from_larger_vector<vector_t>();

		//// combining constructor (xy + z)
		//if constexpr (DIM >= 3)
		//{
		//	Vector<T, 2> xy{ 2, 5 };
		//	Vector<T, DIM> vector{ xy, 4 };
		//	ASSERT_NEAR_EX(vector[0], 2);
		//	ASSERT_NEAR_EX(vector[1], 5);
		//	ASSERT_NEAR_EX(vector[2], 4);
		//	if constexpr (DIM > 3) ASSERT_NEAR_EX(vector[3], 0.0);
		//}

		//// combining constructor (xyz + w)
		//if constexpr (DIM >= 4)
		//{
		//	Vector<T, 3> xyz{ 2, 5, 4 };
		//	Vector<T, DIM> vector{ xyz, 9 };
		//	ASSERT_NEAR_EX(vector[0], 2);
		//	ASSERT_NEAR_EX(vector[1], 5);
		//	ASSERT_NEAR_EX(vector[2], 4);
		//	ASSERT_NEAR_EX(vector[3], 9);
		//	if constexpr (DIM > 4) ASSERT_NEAR_EX(vector[4], 0.0);
		//}

		//// pair constructor
		//if constexpr (DIM >= 2)
		//{
		//	auto values = std::pair{ Random<T>(T{}, static_cast<T>(10)), Random<T>(T{}, static_cast<T>(10)) };
		//	Vector<T, DIM> vector{ values };
		//	ASSERT_NEAR_EX(vector[0], std::get<0>(values));
		//	ASSERT_NEAR_EX(vector[1], std::get<1>(values));
		//	if constexpr (DIM > 2) ASSERT_NEAR_EX(vector[2], 0.0);
		//}

		//// tuple constructor (3 elems)
		//if constexpr (DIM >= 3)
		//{
		//	auto values = std::tuple{ 2, 5, Random<T>(T{}, static_cast<T>(10)) };
		//	Vector<T, DIM> vector{ values };
		//	ASSERT_NEAR_EX(vector[0], std::get<0>(values));
		//	ASSERT_NEAR_EX(vector[1], std::get<1>(values));
		//	ASSERT_NEAR_EX(vector[2], std::get<2>(values));
		//	if constexpr (DIM > 3) ASSERT_NEAR_EX(vector[3], 0.0);
		//}

	}
}


TEST_CASE("vector - construction")
{
	TEST_ALL_TYPES(construction_tests);
}
