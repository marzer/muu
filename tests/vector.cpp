// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/vector.h"
#include "../include/muu/half.h"

#if MUU_HAS_VECTORCALL

static_assert(!impl::is_hva<vector<half, 1>>);
static_assert(!impl::is_hva<vector<half, 2>>);
static_assert(!impl::is_hva<vector<half, 3>>);
static_assert(!impl::is_hva<vector<half, 4>>);
static_assert(!impl::is_hva<vector<half, 5>>);

static_assert(impl::is_hva<vector<float, 1>>);
static_assert(impl::is_hva<vector<float, 2>>);
static_assert(impl::is_hva<vector<float, 3>>);
static_assert(impl::is_hva<vector<float, 4>>);
static_assert(!impl::is_hva<vector<float, 5>>);

static_assert(impl::is_hva<vector<double, 1>>);
static_assert(impl::is_hva<vector<double, 2>>);
static_assert(impl::is_hva<vector<double, 3>>);
static_assert(impl::is_hva<vector<double, 4>>);
static_assert(!impl::is_hva<vector<double, 5>>);

static_assert(impl::is_hva<vector<long double, 1>>);
static_assert(impl::is_hva<vector<long double, 2>>);
static_assert(impl::is_hva<vector<long double, 3>>);
static_assert(impl::is_hva<vector<long double, 4>>);
static_assert(!impl::is_hva<vector<long double, 5>>);

static_assert(!impl::is_hva<vector<signed char, 1>>);
static_assert(!impl::is_hva<vector<signed char, 2>>);
static_assert(!impl::is_hva<vector<signed char, 3>>);
static_assert(!impl::is_hva<vector<signed char, 4>>);
static_assert(!impl::is_hva<vector<signed char, 5>>);

static_assert(!impl::is_hva<vector<signed int, 1>>);
static_assert(!impl::is_hva<vector<signed int, 2>>);
static_assert(!impl::is_hva<vector<signed int, 3>>);
static_assert(!impl::is_hva<vector<signed int, 4>>);
static_assert(!impl::is_hva<vector<signed int, 5>>);

static_assert(!impl::is_hva<vector<signed long, 1>>);
static_assert(!impl::is_hva<vector<signed long, 2>>);
static_assert(!impl::is_hva<vector<signed long, 3>>);
static_assert(!impl::is_hva<vector<signed long, 4>>);
static_assert(!impl::is_hva<vector<signed long, 5>>);

static_assert(!impl::is_hva<vector<signed long long, 1>>);
static_assert(!impl::is_hva<vector<signed long long, 2>>);
static_assert(!impl::is_hva<vector<signed long long, 3>>);
static_assert(!impl::is_hva<vector<signed long long, 4>>);
static_assert(!impl::is_hva<vector<signed long long, 5>>);

static_assert(!impl::is_hva<vector<unsigned char, 1>>);
static_assert(!impl::is_hva<vector<unsigned char, 2>>);
static_assert(!impl::is_hva<vector<unsigned char, 3>>);
static_assert(!impl::is_hva<vector<unsigned char, 4>>);
static_assert(!impl::is_hva<vector<unsigned char, 5>>);

static_assert(!impl::is_hva<vector<unsigned int, 1>>);
static_assert(!impl::is_hva<vector<unsigned int, 2>>);
static_assert(!impl::is_hva<vector<unsigned int, 3>>);
static_assert(!impl::is_hva<vector<unsigned int, 4>>);
static_assert(!impl::is_hva<vector<unsigned int, 5>>);

static_assert(!impl::is_hva<vector<unsigned long, 1>>);
static_assert(!impl::is_hva<vector<unsigned long, 2>>);
static_assert(!impl::is_hva<vector<unsigned long, 3>>);
static_assert(!impl::is_hva<vector<unsigned long, 4>>);
static_assert(!impl::is_hva<vector<unsigned long, 5>>);

static_assert(!impl::is_hva<vector<unsigned long long, 1>>);
static_assert(!impl::is_hva<vector<unsigned long long, 2>>);
static_assert(!impl::is_hva<vector<unsigned long long, 3>>);
static_assert(!impl::is_hva<vector<unsigned long long, 4>>);
static_assert(!impl::is_hva<vector<unsigned long long, 5>>);

#endif

#define TEST_TYPE(func, T)						\
		func<T, 1>(MUU_MAKE_STRING_VIEW(T));	\
		func<T, 2>(MUU_MAKE_STRING_VIEW(T));	\
		func<T, 3>(MUU_MAKE_STRING_VIEW(T));	\
		func<T, 4>(MUU_MAKE_STRING_VIEW(T));	\
		func<T, 5>(MUU_MAKE_STRING_VIEW(T))

#if MUU_HAS_FLOAT16
	#define TEST_FLOAT16(func)	TEST_TYPE(func, _Float16)
#else
	#define TEST_FLOAT16(func)	(void)0
#endif
#if MUU_HAS_FP16
	#define TEST_FP16(func)		TEST_TYPE(func, __fp16)
#else
	#define TEST_FP16(func)		(void)0
#endif
#if MUU_HAS_FLOAT128
	#define TEST_FLOAT128(func)	TEST_TYPE(func, quad)
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
			CHECK(same == vec);
			CHECK_FALSE(same != vec);
			if constexpr (is_floating_point<T>)
				CHECK(vector_t::approx_equal(vec, same));

			vector_t different{ vec };
			for (size_t i = 0; i < DIM; i++)
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
				DIM
			>;

			other_t same{ vec };
			CHECK(vector_t::equal(vec, same));
			CHECK(vec == same);
			CHECK_FALSE(vec != same);
			CHECK(vec == same);
			CHECK_FALSE(vec != same);

			other_t different{ vec };
			for (size_t i = 0; i < DIM; i++)
				different[i]++;
			CHECK_FALSE(vector_t::equal(vec, different));
			CHECK(vec != different);
			CHECK_FALSE(vec == different);
			CHECK(different != vec);
			CHECK_FALSE(different == vec);
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
	void zero_tests(std::string_view scalar_typename) noexcept
	{
		INFO("vector<"sv << scalar_typename << ", "sv << DIM << ">"sv)
		using vector_t = vector<T, DIM>;

		vector_t vec{ T{} };
		{
			INFO("all zeroes"sv)
			CHECK(vec.zero());
		}

		{
			INFO("no zeroes"sv)
			for (size_t i = 0; i < DIM; i++)
				vec[i] = random(T{ 1 }, T{ 10 });
			CHECK_FALSE(vec.zero());
		}

		if constexpr (DIM > 1)
		{
			INFO("some zeroes"sv)
			for (size_t i = 0; i < DIM; i += 2)
				vec[i] = T{};
			CHECK_FALSE(vec.zero());
		}

		{
			INFO("one zero"sv)
			for (size_t i = 0; i < DIM; i++)
			{
				vector_t vec2{ T{} };
				vec2[i] = random(T{ 1 }, T{ 10 });
				CHECK_FALSE(vec.zero());
			}
		}
	}
}

TEST_CASE("vector - zero")
{
	TEST_ALL_TYPES(zero_tests);
}

namespace
{
	template <typename T, size_t DIM>
	void infinity_or_nan_tests(std::string_view scalar_typename) noexcept
	{
		INFO("vector<"sv << scalar_typename << ", "sv << DIM << ">"sv)
		using vector_t = vector<T, DIM>;

		vector_t vector1;
		{
			INFO("all finite"sv)
			for (size_t i = 0; i < DIM; i++)
				vector1[i] = static_cast<T>(i);
			CHECK_FALSE(vector1.infinity_or_nan());
		}


		if constexpr (is_floating_point<T>)
		{
			{
				INFO("contains one NaN"sv)
				for (size_t i = 0; i < DIM; i++)
				{
					vector_t vector2{ vector1 };
					vector2[i] = make_nan<T>();
					CHECK(vector2.infinity_or_nan());
				}
			}

			{
				INFO("contains one infinity"sv)
				for (size_t i = 0; i < DIM; i++)
				{
					vector_t vector2{ vector1 };
					vector2[i] = make_infinity<T>();
					CHECK(vector2.infinity_or_nan());
				}
			}
		}
	}
}

TEST_CASE("vector - infinity_or_nan")
{
	TEST_ALL_TYPES(infinity_or_nan_tests);
}

TEST_CASE("vector - length/distance")
{
	{
		INFO("vector<float, 2>"sv)

		const vector<float, 2> a{ 0, 10 };
		const vector<float, 2> b{ 15, 12 };
		const float distance = a.distance(b);
		CHECK(distance == Approx( std::sqrt(15.0f * 15.0f + 2.0f * 2.0f) ));
	}

	{
		INFO("vector<float, 3>"sv)

		const vector<float, 3> a{ 0, 10, 3 };
		const vector<float, 3> b{ 15, 12, -4 };
		const float distance = a.distance(b);
		CHECK(distance == Approx( std::sqrt(15.0f * 15.0f + 2.0f * 2.0f + 7.0f * 7.0f)));
	}

	{
		INFO("vector<float, 4>"sv)

		const vector<float, 4> a{ 9, 10, 3, 5 };
		const vector<float, 4> b{ 15, 12, -4, 1 };
		const float distance = a.distance(b);
		CHECK(distance == Approx( std::sqrt(6.0f * 6.0f + 2.0f * 2.0f + 7.0f * 7.0f + 4.0f * 4.0f)) );
	}
}

namespace
{
	template <typename T, size_t DIM>
	void dot_tests(std::string_view scalar_typename) noexcept
	{
		INFO("vector<"sv << scalar_typename << ", "sv << DIM << ">"sv)
		using vector_t = vector<T, DIM>;

		const auto x1 = random_array<T, DIM>();
		const auto x2 = random_array<T, DIM>();
		vector_t vector1(x1), vector2(x2);
		
		using dot_type = decltype(vector1.dot(vector2));
		static_assert(is_floating_point<dot_type>);

		// expected result
		auto expected = dot_type{};
		for (size_t i = 0; i < DIM; i++)
			expected += static_cast<dot_type>(x1[i]) * static_cast<dot_type>(x2[i]);

		CHECK(vector1.dot(vector2) == Approx(expected));
		CHECK(vector_t::dot(vector1, vector2) == Approx(expected));
		//CHECK(muu::dot(vector1, vector2) == Approx(expected));
	}

	//template <typename T, size_t DIM>
	//void cross_tests([[maybe_unused]] std::string_view scalar_typename) noexcept
	//{
	//	if constexpr (DIM == 3)
	//	{
	//		INFO("vector<"sv << scalar_typename << ", "sv << DIM << ">"sv)
	//		using vector_t = vector<T, DIM>;

	//		vector_t f1_vector(1.1, 4.5, 9.8);
	//		vector_t f2_vector(-1.4, 9.5, 3.2);
	//		f1_vector.Normalize();
	//		f2_vector.Normalize();

	//		//member
	//		vector_t fcross_vector = f1_vector.Cross(f2_vector);
	//		T f1_dot = fcross_vector.Dot(f1_vector);
	//		T f2_dot = fcross_vector.Dot(f2_vector);
	//		ASSERT_NEAR(static_cast<double>(f1_dot), 0.0, DefaultPrecision<T> * 10);
	//		ASSERT_NEAR(static_cast<double>(f2_dot), 0.0, DefaultPrecision<T> * 10);

	//		//static
	//		fcross_vector = vector_t::Cross(f1_vector, f2_vector);
	//		f1_dot = fcross_vector.Dot(f1_vector);
	//		f2_dot = fcross_vector.Dot(f2_vector);
	//		ASSERT_NEAR(static_cast<double>(f1_dot), 0.0, DefaultPrecision<T> * 10);
	//		ASSERT_NEAR(static_cast<double>(f2_dot), 0.0, DefaultPrecision<T> * 10);

	//		//global
	//		fcross_vector = Cross(f1_vector, f2_vector);
	//		f1_dot = fcross_vector.Dot(f1_vector);
	//		f2_dot = fcross_vector.Dot(f2_vector);
	//		ASSERT_NEAR(static_cast<double>(f1_dot), 0.0, DefaultPrecision<T> * 10);
	//		ASSERT_NEAR(static_cast<double>(f2_dot), 0.0, DefaultPrecision<T> * 10);
	//	}
	//}
}

TEST_CASE("vector - dot")
{
	TEST_ALL_TYPES(dot_tests);
}

//TEST_CASE("vector - cross")
//{
//	TEST_ALL_TYPES(cross_tests);
//}

namespace
{
	template <typename T, size_t DIM>
	void addition_tests(std::string_view scalar_typename) noexcept
	{
		INFO("vector<"sv << scalar_typename << ", "sv << DIM << ">"sv)
		using vector_t = vector<T, DIM>;

		const auto x1 = random_array<T, DIM>(0, 5);
		const auto x2 = random_array<T, DIM>(0, 5);
		const vector_t vector1{ x1 }, vector2{ x2 };

		{
			INFO("add(vector, vector)"sv)

			vector_t result = vector_t::add(vector1, vector2);
			for (size_t i = 0; i < DIM; i++)
				CHECK((x1[i] + x2[i]) == approx_if_float(result[i]));
		}

		{
			INFO("vector + vector"sv)

			vector_t result = vector1 + vector2;
			for (size_t i = 0; i < DIM; i++)
				CHECK((x1[i] + x2[i]) == approx_if_float(result[i]));
		}

		{
			INFO("vector += vector"sv)

			vector_t result(vector1);
			result += vector2;
			for (size_t i = 0; i < DIM; i++)
				CHECK((x1[i] + x2[i]) == approx_if_float(result[i]));
		}
	}

	template <typename T, size_t DIM>
	void subtraction_tests(std::string_view scalar_typename) noexcept
	{
		INFO("vector<"sv << scalar_typename << ", "sv << DIM << ">"sv)
		using vector_t = vector<T, DIM>;

		const auto x1 = random_array<T, DIM>(0, 5);
		const auto x2 = random_array<T, DIM>(0, 5);
		const vector_t vector1{ x1 }, vector2{ x2 };

		{
			INFO("subtract(vector, vector)"sv)

			const vector_t result = vector_t::subtract(vector1, vector2);
			for (size_t i = 0; i < DIM; i++)
				CHECK((x1[i] - x2[i]) == approx_if_float(result[i]));
		}

		{
			INFO("vector - vector"sv)

			const vector_t result = vector1 - vector2;
			for (size_t i = 0; i < DIM; i++)
				CHECK((x1[i] - x2[i]) == approx_if_float(result[i]));
		}

		{
			INFO("vector -= vector"sv)

			vector_t result(vector1);
			result -= vector2;
			for (size_t i = 0; i < DIM; i++)
				CHECK((x1[i] - x2[i]) == approx_if_float(result[i]));
		}

		{
			INFO("-vector"sv)

			const vector_t result = -vector1;
			for (size_t i = 0; i < DIM; i++)
				CHECK(-x1[i] == approx_if_float(result[i]));
		}
	}
}

TEST_CASE("vector - addition")
{
	TEST_ALL_TYPES(addition_tests);
}

TEST_CASE("vector - subtraction")
{
	TEST_ALL_TYPES(subtraction_tests);
}

namespace
{
	template <typename T, size_t DIM>
	void multiplication_tests(std::string_view scalar_typename) noexcept
	{
		INFO("vector<"sv << scalar_typename << ", "sv << DIM << ">"sv)
		using vector_t = vector<T, DIM>;

		const auto scalar = static_cast<T>(2.4);
		const auto x1 = random_array<T, DIM>(0, 5);
		const auto x2 = random_array<T, DIM>(0, 5);
		const vector_t vector1{ x1 }, vector2{ x2 };

		{
			INFO("multiply(vector, vector)"sv)

			vector_t result = vector_t::multiply(vector1, vector2);
			for (size_t i = 0; i < DIM; i++)
				CHECK(x1[i] * x2[i] == approx_if_float(result[i]));
		}

		{
			INFO("vector * vector"sv)

			vector_t result = vector1 * vector2;
			for (size_t i = 0; i < DIM; i++)
				CHECK(x1[i] * x2[i] == approx_if_float(result[i]));
		}

		{
			INFO("vector *= vector"sv)

			vector_t result(vector1);
			result *= vector2;
			for (size_t i = 0; i < DIM; i++)
				CHECK(x1[i] * x2[i] == approx_if_float(result[i]));
		}

		{
			INFO("multiply(vector, scalar)"sv)

			vector_t result = vector_t::multiply(vector1, scalar);
			for (size_t i = 0; i < DIM; i++)
				CHECK(x1[i] * scalar == approx_if_float(result[i]));
		}

		{
			INFO("vector * scalar"sv)

			vector_t result = vector1 * scalar;
			for (size_t i = 0; i < DIM; i++)
				CHECK(x1[i] * scalar == approx_if_float(result[i]));
		}

		{
			INFO("scalar * vector"sv)

			vector_t result = scalar * vector2;
			for (size_t i = 0; i < DIM; i++)
				CHECK(x2[i] * scalar == approx_if_float(result[i]));
		}


		{
			INFO("vector *= scalar"sv)

			vector_t result(vector1);
			result *= scalar;
			for (size_t i = 0; i < DIM; i++)
				CHECK(x1[i] * scalar == approx_if_float(result[i]));
		}
	}

	template <typename T, size_t DIM>
	void division_tests(std::string_view scalar_typename) noexcept
	{
		INFO("vector<"sv << scalar_typename << ", "sv << DIM << ">"sv)
		using vector_t = vector<T, DIM>;

		const auto scalar = static_cast<T>(2.4);
		const vector_t vec1{ random_array<T, DIM>(2, 10) };
		const vector_t vec2{ random_array<T, DIM>(2, 10) };

		{
			INFO("divide(vector, vector)"sv)

			vector_t result = vector_t::divide(vec1, vec2);
			for (size_t i = 0; i < DIM; i++)
				CHECK(vec1[i] / vec2[i] == approx_if_float(result[i]));
		}

		{
			INFO("vector / vector"sv)

			vector_t result = vec1 / vec2;
			for (size_t i = 0; i < DIM; i++)
				CHECK(vec1[i] / vec2[i] == approx_if_float(result[i]));
		}

		{
			INFO("vector /= vector"sv)

			vector_t result = vec1;
			result /= vec2;
			for (size_t i = 0; i < DIM; i++)
				CHECK(vec1[i] / vec2[i] == approx_if_float(result[i]));
		}

		{
			INFO("divide(vector, scalar)"sv)

			vector_t result = vector_t::divide(vec1, scalar);
			for (size_t i = 0; i < DIM; i++)
				CHECK(vec1[i] / scalar == approx_if_float(result[i]));
		}

		{
			INFO("vector / scalar"sv)

			vector_t result = vec1 / scalar;
			for (size_t i = 0; i < DIM; i++)
				CHECK(vec1[i] / scalar == approx_if_float(result[i]));
		}

		{
			INFO("vector /= scalar"sv)

			vector_t result = vec1;
			result /= scalar;
			for (size_t i = 0; i < DIM; i++)
				CHECK(vec1[i] / scalar == approx_if_float(result[i]));
		}
	}
}

TEST_CASE("vector - multiplication")
{
	TEST_ALL_TYPES(multiplication_tests);
}

TEST_CASE("vector - division")
{
	TEST_ALL_TYPES(division_tests);
}
