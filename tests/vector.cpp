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

	template <typename Vector, size_t NUM>
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
		if constexpr (DIM < 2) construction_test_from_larger_vector<vector_t, 2>();
		if constexpr (DIM < 3) construction_test_from_larger_vector<vector_t, 3>();
		if constexpr (DIM < 4) construction_test_from_larger_vector<vector_t, 4>();
		if constexpr (DIM < 5) construction_test_from_larger_vector<vector_t, 5>();
		if constexpr (DIM < 10) construction_test_from_larger_vector<vector_t, 10>();

		// pair constructor
		if constexpr (DIM >= 2)
		{
			INFO("constructing from a std::pair"sv)

			auto values = std::pair{ random<T>(), random<T>() };
			vector<T, DIM> vec{ values };
			CHECK(vec[0] == std::get<0>(values));
			CHECK(vec[1] == std::get<1>(values));
			if constexpr (DIM > 2) CHECK(vec[2] == T{});
		}

		// tuple constructor (3 elems)
		if constexpr (DIM >= 3)
		{
			INFO("constructing from a std::tuple"sv)

			auto values = std::tuple{ random<T>(), random<T>(), random<T>() };
			vector<T, DIM> vec{ values };
			CHECK(vec[0] == std::get<0>(values));
			CHECK(vec[1] == std::get<1>(values));
			CHECK(vec[2] == std::get<2>(values));
			if constexpr (DIM > 3) CHECK(vec[3] == T{});
		}

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


	}
}

TEST_CASE("vector - construction")
{
	TEST_ALL_TYPES(construction_tests);
}

namespace
{
	template <typename T, size_t DIM>
	void equality_tests(std::string_view scalar_typename) noexcept
	{
		INFO("vector<"sv << scalar_typename << ", "sv << DIM << ">"sv)
		using vector_t = vector<T, DIM>;

		vector_t vec;
		for (size_t i = 0; i < DIM; i++)
			vec[i] = random<T>();
		
		{
			INFO("same type"sv)
		
			vector_t same{ vec };
			CHECK(vector_t::equal(vec, same));
			CHECK(vec == same);
			CHECK_FALSE(vec != same);
			if constexpr (is_floating_point<T>)
				CHECK(vector_t::approx_equal(vec, same));

			vector_t different{ vec };
			for (size_t i = 0; i < DIM; i++)
				different[i]++;
			CHECK_FALSE(vector_t::equal(vec, different));
			CHECK(vec != different);
			CHECK_FALSE(vec == different);
			if constexpr (is_floating_point<T>)
				CHECK_FALSE(vector_t::approx_equal(vec, different));
		}

		if constexpr (!is_floating_point<T>)
		{
			INFO("different type"sv)

			using other_t = vector<
				std::conditional_t<std::is_same_v<T, long>, int, long>,
				DIM
			>;

			other_t same{ vec };
			CHECK(vector_t::equal(vec, same));
			CHECK(vec == same);
			CHECK_FALSE(vec != same);

			other_t different{ vec };
			for (size_t i = 0; i < DIM; i++)
				different[i]++;
			CHECK_FALSE(vector_t::equal(vec, different));
			CHECK(vec != different);
			CHECK_FALSE(vec == different);
		}
	}
}

TEST_CASE("vector - equality")
{
	TEST_ALL_TYPES(equality_tests);
}

namespace
{
	template <typename T, size_t DIM>
	void is_zero_tests(std::string_view scalar_typename) noexcept
	{
		INFO("vector<"sv << scalar_typename << ", "sv << DIM << ">"sv)
		using vector_t = vector<T, DIM>;

		vector_t vec{ T{} };
		{
			INFO("all zeroes"sv)
			CHECK(vec.is_zero());
		}

		{
			INFO("no zeroes"sv)
			for (size_t i = 0; i < DIM; i++)
				vec[i] = random(T{ 1 }, T{ 10 });
			CHECK_FALSE(vec.is_zero());
		}

		if constexpr (DIM > 1)
		{
			INFO("some zeroes"sv)
			for (size_t i = 0; i < DIM; i += 2)
				vec[i] = T{};
			CHECK_FALSE(vec.is_zero());
		}

		{
			INFO("one zero"sv)
			for (size_t i = 0; i < DIM; i++)
			{
				vector_t vec2{ T{} };
				vec2[i] = random(T{ 1 }, T{ 10 });
				CHECK_FALSE(vec.is_zero());
			}
		}
	}
}

TEST_CASE("vector - is_zero")
{
	TEST_ALL_TYPES(is_zero_tests);
}


namespace
{
	template <typename T, size_t DIM>
	void is_infinity_or_nan_tests(std::string_view scalar_typename) noexcept
	{
		INFO("vector<"sv << scalar_typename << ", "sv << DIM << ">"sv)
		using vector_t = vector<T, DIM>;

		vector_t vector1;
		{
			INFO("all finite"sv)
			for (size_t i = 0; i < DIM; i++)
				vector1[i] = static_cast<T>(i);
			CHECK_FALSE(vector1.is_infinity_or_nan());
		}


		if constexpr (is_floating_point<T>)
		{
			{
				INFO("contains one NaN"sv)
				for (size_t i = 0; i < DIM; i++)
				{
					vector_t vector2{ vector1 };
					vector2[i] = make_nan<T>();
					CHECK(vector2.is_infinity_or_nan());
				}
			}

			{
				INFO("contains one infinity"sv)
				for (size_t i = 0; i < DIM; i++)
				{
					vector_t vector2{ vector1 };
					vector2[i] = make_infinity<T>();
					CHECK(vector2.is_infinity_or_nan());
				}
			}
		}
	}
}

TEST_CASE("vector - is_infinity_or_nan")
{
	TEST_ALL_TYPES(is_infinity_or_nan_tests);
}
