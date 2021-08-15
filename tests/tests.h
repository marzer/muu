// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once

// environment ---------------------------------------------------------------------------------------------------------

#include "settings.h"
#include "../include/muu/preprocessor.h"

#if MUU_ICC
	#define UNICODE_LITERALS_OK 0
#else
	#define UNICODE_LITERALS_OK 1
#endif
#if MUU_HAS_FP16 != SHOULD_HAVE_FP16
	#error MUU_HAS_FP16 was not deduced correctly
#endif
#if MUU_HAS_FLOAT16 != SHOULD_HAVE_FLOAT16
	#error MUU_HAS_FLOAT16 was not deduced correctly
#endif
#if MUU_HAS_FLOAT128 != SHOULD_HAVE_FLOAT128
	#error MUU_HAS_FLOAT128 was not deduced correctly
#endif
#if MUU_HAS_INT128 != SHOULD_HAVE_INT128
	#error MUU_HAS_INT128 was not deduced correctly
#endif
#if MUU_HAS_EXCEPTIONS != SHOULD_HAVE_EXCEPTIONS
	#error MUU_HAS_EXCEPTIONS was not deduced correctly
#endif

MUU_PRAGMA_CLANG(diagnostic ignored "-Wc++2a-compat")
MUU_PRAGMA_CLANG(diagnostic ignored "-Wfloat-equal")
MUU_PRAGMA_CLANG(diagnostic ignored "-Wunused-template")
MUU_PRAGMA_CLANG(diagnostic ignored "-Wunused-macros")
MUU_PRAGMA_CLANG(diagnostic ignored "-Wpadded")
MUU_PRAGMA_CLANG(diagnostic ignored "-Wdouble-promotion")
MUU_PRAGMA_GCC(diagnostic ignored "-Wfloat-equal")
MUU_PRAGMA_GCC(diagnostic ignored "-Wpedantic")
MUU_PRAGMA_GCC(diagnostic ignored "-Wpadded")
MUU_PRAGMA_GCC(diagnostic ignored "-Wuseless-cast")
MUU_PRAGMA_GCC(diagnostic ignored "-Wsign-conversion") // in catch2 macros :(
MUU_PRAGMA_MSVC(warning(disable : 4127))			   // conditional expression is constant

// includes ------------------------------------------------------------------------------------------------------------

#include "float_test_data.h"
#include "../include/muu/math.h"
#include "../include/muu/half.h"
#include "../include/muu/type_list.h"

MUU_DISABLE_WARNINGS;
#include <iosfwd>
#include <cstring>

namespace std
{
#if MUU_HAS_FLOAT16
	ostream& operator<<(ostream&, _Float16);
#endif
#if MUU_HAS_FLOAT128
	ostream& operator<<(ostream&, __float128);
#endif
#if MUU_HAS_INT128
	ostream& operator<<(ostream&, __int128_t);
	ostream& operator<<(ostream&, __uint128_t);
#endif
}

#include "catch2.h"
#include <array>
#include <string_view>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <tuple>

using namespace Catch::literals;
using namespace muu;
using namespace std::string_view_literals;
using namespace std::chrono_literals;

MUU_ENABLE_WARNINGS;

// macros --------------------------------------------------------------------------------------------------------------

#define TEST_INFO(...)	  INFO(__VA_ARGS__) static_assert(true)
#define TEST_WARNING(...) WARN(__VA_ARGS__) static_assert(true)

#define TEMPLATE_TESTS_1(name, ...) TEMPLATE_TEST_CASE(name, "", __VA_ARGS__)
#define TEMPLATE_TESTS(name, ...)	TEMPLATE_TESTS_1(name, __VA_ARGS__)

#define TEMPLATE_PRODUCT_TEST_1(name, typelist_1, typelist_2)                                                          \
	TEMPLATE_PRODUCT_TEST_CASE(name, "", typelist_1, typelist_2)
#define TEMPLATE_PRODUCT_TEST(name, typelist_1, typelist_2) TEMPLATE_PRODUCT_TEST_1(name, typelist_1, typelist_2)

#define TEMPLATE_LIST_TEST_1(name, typelist) TEMPLATE_LIST_TEST_CASE(name, "", typelist)
#define TEMPLATE_LIST_TEST(name, typelist)	 TEMPLATE_LIST_TEST_1(name, typelist)

#if MUU_INTELLISENSE
	#define CHECK_AND_STATIC_ASSERT(...) CHECK(__VA_ARGS__)
#else
	#define CHECK_AND_STATIC_ASSERT(...)                                                                               \
		do                                                                                                             \
		{                                                                                                              \
			static_assert(__VA_ARGS__);                                                                                \
			CHECK(__VA_ARGS__);                                                                                        \
		}                                                                                                              \
		while (false)
#endif

// CHECK asserts for string-related code
// because a bunch of wide string traits code doesn't work in constexpr on older clang
// (and straight-up fails to link on intel-cl)
#if MUU_CLANG && MUU_CLANG <= 8
	#define CHECK_STRINGS_W(...) CHECK(__VA_ARGS__)
#elif MUU_ICC_CL
	#define CHECK_STRINGS(...)	 CHECK(__VA_ARGS__)
	#define CHECK_STRINGS_W(...) MUU_NOOP
#endif
#ifndef CHECK_STRINGS
	#define CHECK_STRINGS(...) CHECK_AND_STATIC_ASSERT(__VA_ARGS__)
#endif
#ifndef CHECK_STRINGS_W
	#define CHECK_STRINGS_W(...) CHECK_AND_STATIC_ASSERT(__VA_ARGS__)
#endif

#define CHECK_APPROX_EQUAL_EPS(actual_, expected_, epsilon_)                                                           \
	do                                                                                                                 \
	{                                                                                                                  \
		if constexpr (any_floating_point<decltype(actual_), decltype(expected_)>)                                      \
		{                                                                                                              \
			const auto cae_expected = expected_;                                                                       \
			INFO("expected: "sv << print_aligned{ cae_expected } << "    "sv << MUU_MAKE_STRING(expected_))            \
                                                                                                                       \
			const auto cae_actual = actual_;                                                                           \
			INFO("  actual: "sv << print_aligned{ cae_actual } << "    "sv << MUU_MAKE_STRING(actual_))                \
                                                                                                                       \
			const auto cae_epsilon = epsilon_;                                                                         \
			INFO(" epsilon: "sv << print_aligned{ cae_epsilon } << "    "sv << MUU_MAKE_STRING(epsilon_))              \
                                                                                                                       \
			CHECK(approx_equal(cae_expected, cae_actual, cae_epsilon));                                                \
		}                                                                                                              \
		else                                                                                                           \
			CHECK((expected_) == (actual_));                                                                           \
	}                                                                                                                  \
	while (false)

#define CHECK_APPROX_EQUAL(actual_, expected_)                                                                         \
	do                                                                                                                 \
	{                                                                                                                  \
		if constexpr (any_floating_point<decltype(actual_), decltype(expected_)>)                                      \
		{                                                                                                              \
			const auto cae_expected = expected_;                                                                       \
			INFO("expected: "sv << print_aligned{ cae_expected } << "    "sv << MUU_MAKE_STRING(expected_))            \
                                                                                                                       \
			const auto cae_actual = actual_;                                                                           \
			INFO("  actual: "sv << print_aligned{ cae_actual } << "    "sv << MUU_MAKE_STRING(actual_))                \
                                                                                                                       \
			using cae_epsilon_type = impl::highest_ranked<decltype(actual_), decltype(expected_)>;                     \
			INFO(" epsilon: "sv << print_aligned{ constants<cae_epsilon_type>::default_epsilon })                      \
                                                                                                                       \
			CHECK(approx_equal(cae_expected, cae_actual));                                                             \
		}                                                                                                              \
		else                                                                                                           \
			CHECK((expected_) == (actual_));                                                                           \
	}                                                                                                                  \
	while (false)

#define CHECK_SYMMETRIC_EQUAL(lhs, rhs)                                                                                \
	do                                                                                                                 \
	{                                                                                                                  \
		CHECK(lhs == rhs);                                                                                             \
		CHECK_FALSE(lhs != rhs);                                                                                       \
		CHECK(rhs == lhs);                                                                                             \
		CHECK_FALSE(rhs != lhs);                                                                                       \
	}                                                                                                                  \
	while (false)

#define CHECK_SYMMETRIC_INEQUAL(lhs, rhs)                                                                              \
	do                                                                                                                 \
	{                                                                                                                  \
		CHECK_FALSE(lhs == rhs);                                                                                       \
		CHECK(lhs != rhs);                                                                                             \
		CHECK_FALSE(rhs == lhs);                                                                                       \
		CHECK(rhs != lhs);                                                                                             \
	}                                                                                                                  \
	while (false)

#if MUU_HAS_CHAR8
	#define ALL_CHARS_ , char8_t
#else
	#define ALL_CHARS_
#endif
#define ALL_CHARS char, wchar_t, char16_t, char32_t ALL_CHARS_

#define STANDARD_SIGNED_INTS   signed char, short, int, long, long long
#define STANDARD_UNSIGNED_INTS unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long
#define STANDARD_INTS		   STANDARD_SIGNED_INTS, STANDARD_UNSIGNED_INTS

#if MUU_HAS_INT128
	#define EXTENDED_SIGNED_INTS   int128_t
	#define EXTENDED_UNSIGNED_INTS uint128_t
	#define EXTENDED_INTS		   int128_t, uint128_t
	#define ALL_SIGNED_INTS		   STANDARD_SIGNED_INTS, EXTENDED_SIGNED_INTS
	#define ALL_UNSIGNED_INTS	   STANDARD_UNSIGNED_INTS, EXTENDED_UNSIGNED_INTS
	#define ALL_INTS			   STANDARD_INTS, EXTENDED_INTS
#else
	#define EXTENDED_SIGNED_INTS
	#define EXTENDED_UNSIGNED_INTS
	#define EXTENDED_INTS
	#define ALL_SIGNED_INTS	  STANDARD_SIGNED_INTS
	#define ALL_UNSIGNED_INTS STANDARD_UNSIGNED_INTS
	#define ALL_INTS		  STANDARD_INTS
#endif

#ifdef _MSC_VER // double and long double are identical on msvc so we just ignore long double
	#define STANDARD_FLOATS float, double
#else
	#define STANDARD_FLOATS float, double, long double
#endif

#if MUU_HAS_FP16
	#define EXTENDED_FLOATS_1 , __fp16
#else
	#define EXTENDED_FLOATS_1
#endif

#if MUU_HAS_FLOAT16
	#define EXTENDED_FLOATS_2 , _Float16
#else
	#define EXTENDED_FLOATS_2
#endif

#if MUU_HAS_FLOAT128
	#define EXTENDED_FLOATS_3 , float128_t
#else
	#define EXTENDED_FLOATS_3
#endif

#define EXTENDED_FLOATS			half EXTENDED_FLOATS_1 EXTENDED_FLOATS_2 EXTENDED_FLOATS_3
#define ALL_FLOATS				STANDARD_FLOATS, EXTENDED_FLOATS
#define ALL_ARITHMETIC			ALL_FLOATS, ALL_INTS
#define ALL_SIGNED_ARITHMETIC	ALL_FLOATS, ALL_SIGNED_INTS
#define ALL_UNSIGNED_ARITHMETIC ALL_UNSIGNED_INTS

// todo: refactor this crap out

#if MUU_HAS_FP16
	#define FOREACH_FLOAT_VARARGS_1(func, ...) func(__fp16, __VA_ARGS__)
#else
	#define FOREACH_FLOAT_VARARGS_1(func, ...)
#endif

#if MUU_HAS_FLOAT16
	#define FOREACH_FLOAT_VARARGS_2(func, ...) func(_Float16, __VA_ARGS__)
#else
	#define FOREACH_FLOAT_VARARGS_2(func, ...)
#endif

#if MUU_HAS_FLOAT128
	#define FOREACH_FLOAT_VARARGS_3(func, ...) func(float128_t, __VA_ARGS__)
#else
	#define FOREACH_FLOAT_VARARGS_3(func, ...)
#endif

#define FOREACH_FLOAT_VARARGS(func, ...)                                                                               \
	MUU_FOR_EACH_FORCE_UNROLL(FOREACH_FLOAT_VARARGS_1(func, __VA_ARGS__))                                              \
	MUU_FOR_EACH_FORCE_UNROLL(FOREACH_FLOAT_VARARGS_2(func, __VA_ARGS__))                                              \
	MUU_FOR_EACH_FORCE_UNROLL(FOREACH_FLOAT_VARARGS_3(func, __VA_ARGS__))                                              \
	MUU_FOR_EACH_FORCE_UNROLL(func(half, __VA_ARGS__))                                                                 \
	MUU_FOR_EACH_FORCE_UNROLL(func(float, __VA_ARGS__))                                                                \
	MUU_FOR_EACH_FORCE_UNROLL(func(double, __VA_ARGS__))                                                               \
	MUU_FOR_EACH_FORCE_UNROLL(func(long double, __VA_ARGS__))                                                          \
	static_assert(true)

// nameof --------------------------------------------------------------------------------------------------------------

template <typename T>
struct nameof_;
template <typename T>
inline constexpr auto nameof = nameof_<T>::value;

#define MAKE_NAME_OF(type)                                                                                             \
	template <>                                                                                                        \
	struct nameof_<type>                                                                                               \
	{                                                                                                                  \
		static constexpr auto value = MUU_MAKE_STRING_VIEW(type);                                                      \
	};

MUU_FOR_EACH(MAKE_NAME_OF, ALL_CHARS)
MUU_FOR_EACH(MAKE_NAME_OF, ALL_ARITHMETIC)

// misc utils ----------------------------------------------------------------------------------------------------------

namespace muu
{
	template <typename T>
	MUU_NODISCARD
	MUU_ATTR(const)
	MUU_CONSTEVAL
	muu::remove_cvref<T> make_infinity(int sign = 1) noexcept
	{
		using ftd = float_test_data<muu::remove_cvref<T>>;
		return muu::bit_cast<muu::remove_cvref<T>>(sign >= 0 ? ftd::bits_pos_inf : ftd::bits_neg_inf);
	}

	template <typename T>
	MUU_NODISCARD
	MUU_ATTR(const)
	MUU_CONSTEVAL
	muu::remove_cvref<T> make_nan() noexcept
	{
		return muu::bit_cast<muu::remove_cvref<T>>(float_test_data<muu::remove_cvref<T>>::bits_qnan);
	}

	template <typename T>
	MUU_NODISCARD
	inline T random() noexcept
	{
		if constexpr (is_floating_point<T>) // 0.0 - 1.0
		{
			if constexpr (impl::is_small_float_<T>) // NaN's otherwise
			{
				return static_cast<T>(static_cast<float>(1 + (::rand() % 1000)) / 1000.0f);
			}
			else
			{
				using fp = impl::highest_ranked<T, float>;
				return static_cast<T>(static_cast<fp>(::rand()) / static_cast<fp>(RAND_MAX));
			}
		}
		else // 0 - min(RAND_MAX, limit)
			return static_cast<T>(static_cast<T>(::rand()) % constants<T>::highest);
	}

	template <typename T, typename Max>
	MUU_NODISCARD
	inline T random(Max max_) noexcept
	{
		if constexpr (is_floating_point<T>)
		{
			using fp = impl::highest_ranked<T, float>;
			return static_cast<T>(random<fp>() * static_cast<fp>(max_));
		}
		else
			return static_cast<T>(random<double>() * static_cast<double>(max_));
	}

	template <typename T, typename Min, typename Max>
	MUU_NODISCARD
	inline T random(Min min_, Max max_) noexcept
	{
		return static_cast<T>(static_cast<T>(min_) + random<T>(static_cast<T>(max_) - static_cast<T>(min_)));
	}

	template <typename T, size_t Num, typename Min, typename Max>
	MUU_NODISCARD
	inline std::array<T, Num> random_array(Min min_, Max max_) noexcept
	{
		std::array<T, Num> vals;
		for (auto& v : vals)
			v = random<T>(static_cast<T>(min_), static_cast<T>(max_));
		return vals;
	}

	template <typename T, size_t Num>
	MUU_NODISCARD
	inline std::array<T, Num> random_array() noexcept
	{
		std::array<T, Num> vals;
		for (auto& v : vals)
			v = random<T>();
		return vals;
	}

	template <typename T MUU_ENABLE_IF(is_floating_point<T>)>
	MUU_REQUIRES(is_floating_point<T>)
	inline Approx approx(T val, T eps) noexcept
	{
		Approx a(val);
		a.epsilon(eps);
		return a;
	}

	template <typename T MUU_ENABLE_IF(is_floating_point<T>)>
	MUU_REQUIRES(is_floating_point<T>)
	inline Approx approx(T val) noexcept
	{
		Approx a(val);
		a.epsilon(std::numeric_limits<T>::epsilon() * 100); // catch2 default
		return a;
	}

	template <typename T MUU_ENABLE_IF_2(!is_floating_point<T>)>
	MUU_REQUIRES(!is_floating_point<T>)
	inline T&& approx(T&& val) noexcept
	{
		return static_cast<T&&>(val);
	}

	template <typename T>
	struct print_aligned
	{
		T value;

		template <typename Char>
		friend std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& lhs, const print_aligned& rhs)
		{
			if constexpr (is_floating_point<T>)
			{
				auto left = 1
						  + (muu::max)(0,
									   static_cast<int>(muu::floor(
										   std::log10(static_cast<clamp_to_standard_float<T>>(muu::abs(rhs.value))))));
				left += rhs.value < T{} ? 1 : 0;
				left = 4 - left;
				while (left-- > 0)
					lhs.put(constants<Char>::space);

				lhs << std::fixed << std::setprecision(constants<T>::decimal_digits) << rhs.value;
			}
			else
			{
				lhs << rhs.value;
			}
			return lhs;
		}
	};

	template <typename T>
	print_aligned(T) -> print_aligned<T>;

	template <size_t Offset = 0_sz, typename Tuple, size_t... Indices>
	constexpr auto tuple_subset(Tuple&& tpl, std::index_sequence<Indices...>) noexcept
	{
		return std::make_tuple(std::get<Indices + Offset>(static_cast<Tuple&&>(tpl))...);
	}

#define EXTERN_ARITHMETIC_TEMPLATES(T)                                                                                 \
	extern template T random() noexcept;                                                                               \
	extern template T random(T) noexcept;                                                                              \
	extern template T random(T, T) noexcept;                                                                           \
	extern template struct print_aligned<T>;

	MUU_FOR_EACH(EXTERN_ARITHMETIC_TEMPLATES, ALL_ARITHMETIC)

#define EXTERN_FLOAT_TEMPLATES(T)                                                                                      \
	extern template Approx approx(T, T) noexcept;                                                                      \
	extern template Approx approx(T) noexcept;

	MUU_FOR_EACH(EXTERN_FLOAT_TEMPLATES, ALL_FLOATS)

}

#undef EXTERN_ARITHMETIC_TEMPLATES
#undef EXTERN_FLOAT_TEMPLATES
