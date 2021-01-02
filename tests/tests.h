// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "float_test_data.h"
#include "../include/muu/math.h"
#include "../include/muu/half.h"

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
#if MUU_EXCEPTIONS != SHOULD_HAVE_EXCEPTIONS
	#error MUU_EXCEPTIONS was not deduced correctly
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
MUU_PRAGMA_MSVC(warning(disable: 4127)) // conditional expression is constant

MUU_DISABLE_WARNINGS
#include <iosfwd>
#if MUU_HAS_FLOAT16
std::ostream& operator << (std::ostream&, _Float16);
#endif
#if MUU_HAS_FLOAT128
std::ostream& operator << (std::ostream&, __float128);
#endif
#include "catch2.h"
#include <array>
#include <string_view>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <tuple>
MUU_NAMESPACE_START
{
	template <typename T>
	[[nodiscard]]
	MUU_ATTR(const)
	MUU_CONSTEVAL T make_infinity(int sign = 1) noexcept
	{
		using ftd = float_test_data<T>;
		return bit_cast<T>(sign >= 0 ? ftd::bits_pos_inf : ftd::bits_neg_inf);
	}

	template <typename T>
	[[nodiscard]]
	MUU_ATTR(const)
	MUU_CONSTEVAL T make_nan() noexcept
	{
		return bit_cast<T>(float_test_data<T>::bits_qnan);
	}

	template <typename T>
	[[nodiscard]]
	inline T random() noexcept
	{
		if constexpr (is_floating_point<T>)
		{
			using fp = impl::highest_ranked<T, float>;
			return static_cast<T>(static_cast<fp>(::rand()) / static_cast<fp>(RAND_MAX)); // 0.0 - 1.0
		}
		else
			return static_cast<T>(static_cast<T>(::rand()) % constants<T>::highest); // 0 - min(RAND_MAX, limit)
	}

	template <typename T, typename Max>
	[[nodiscard]]
	inline T random(Max max_) noexcept
	{
		if constexpr (is_floating_point<T>)
		{
			using fp = impl::highest_ranked<T, float>;
			return static_cast<T>(random<fp>() * static_cast<T>(max_));
		}
		else
			return static_cast<T>(random<double>() * static_cast<T>(max_));
	}

	template <typename T, typename Min, typename Max>
	[[nodiscard]]
	inline T random(Min min_, Max max_) noexcept
	{
		return static_cast<T>(static_cast<T>(min_) + random<T>(static_cast<T>(max_) - static_cast<T>(min_)));
	}

	template <typename T, size_t Num, typename Min, typename Max>
	[[nodiscard]]
	inline std::array<T, Num> random_array(Min min_, Max max_) noexcept
	{
		std::array<T, Num> vals;
		for (auto& v : vals)
			v = random<T>(static_cast<T>(min_), static_cast<T>(max_));
		return vals;
	}

	template <typename T, size_t Num>
	[[nodiscard]]
	inline std::array<T, Num> random_array() noexcept
	{
		std::array<T, Num> vals;
		for (auto& v : vals)
			v = random<T>();
		return vals;
	}

	template <typename T MUU_ENABLE_IF(is_floating_point<T>)> MUU_REQUIRES(is_floating_point<T>)
	inline Approx approx(T val, T eps) noexcept
	{
		Approx a(val);
		a.epsilon(eps);
		return a;
	}

	template <typename T MUU_ENABLE_IF(is_floating_point<T>)> MUU_REQUIRES(is_floating_point<T>)
	inline Approx approx(T val) noexcept
	{
		Approx a(val);
		a.epsilon(std::numeric_limits<T>::epsilon() * 100); // catch2 default
		return a;
	}

	template <typename T MUU_ENABLE_IF_2(!is_floating_point<T>)> MUU_REQUIRES(!is_floating_point<T>)
	inline T&& approx(T&& val) noexcept
	{
		return static_cast<T&&>(val);
	}

	template <typename T>
	struct print_aligned
	{
		T value;

		template <typename Char>
		friend std::basic_ostream<Char>& operator << (std::basic_ostream<Char>& lhs, const print_aligned& rhs)
		{
			if constexpr (is_floating_point<T>)
			{
				auto left = 1 + (muu::max)(0, static_cast<int>(muu::floor(std::log10(static_cast<impl::clamp_to_standard_float<T>>(muu::abs(rhs.value))))));
				left += rhs.value < T{} ? 1 : 0;
				left = 4 - left;
				while (left-- > 0)
					lhs.put(constants<Char>::space);

				lhs << std::fixed
					<< std::setprecision(constants<T>::decimal_digits)
					<< rhs.value;
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
}
MUU_NAMESPACE_END
using namespace Catch::literals;
using namespace muu;
using namespace std::string_view_literals;
using namespace std::chrono_literals;
MUU_ENABLE_WARNINGS

// Q: why is this CHECK_AND_STATIC_ASSERT thing?
// A: because std::is_constant_evaluated().

#if MUU_INTELLISENSE
	#define CHECK_AND_STATIC_ASSERT(...)	\
		CHECK(__VA_ARGS__)
#else
	#define CHECK_AND_STATIC_ASSERT(...)	\
		static_assert(__VA_ARGS__);			\
		CHECK(__VA_ARGS__)
#endif

// CHECK asserts for string-related code
// because a bunch of wide string traits code doesn't work in constexpr on older clang
// (and straight-up fails to link on intel-cl)
#if MUU_CLANG && MUU_CLANG <= 8
	#define CHECK_STRINGS_W(...)	CHECK(__VA_ARGS__)
#elif MUU_ICC_CL
	#define CHECK_STRINGS(...)		CHECK(__VA_ARGS__)
	#define CHECK_STRINGS_W(...)	(void)0
#endif
#ifndef CHECK_STRINGS
	#define CHECK_STRINGS(...)			CHECK_AND_STATIC_ASSERT(__VA_ARGS__)
#endif
#ifndef CHECK_STRINGS_W
	#define CHECK_STRINGS_W(...)		CHECK_AND_STATIC_ASSERT(__VA_ARGS__)
#endif

#define CHECK_APPROX_EQUAL_EPS(actual_, expected_, epsilon_)														\
	do																												\
	{																												\
		if constexpr (any_floating_point<decltype(actual_), decltype(expected_)>)									\
		{																											\
			const auto cae_expected = expected_;																	\
			INFO("expected: "sv << print_aligned{ cae_expected } << "    "sv << MUU_MAKE_STRING(expected_))			\
																													\
			const auto cae_actual = actual_;																		\
			INFO("  actual: "sv << print_aligned{ cae_actual } << "    "sv << MUU_MAKE_STRING(actual_))				\
																													\
			const auto cae_epsilon = epsilon_;																		\
			INFO(" epsilon: "sv << print_aligned{ cae_epsilon } << "    "sv << MUU_MAKE_STRING(epsilon_))			\
																													\
			CHECK(approx_equal(cae_expected, cae_actual, cae_epsilon));												\
		}																											\
		else																										\
			CHECK((expected_) == (actual_));																		\
	}																												\
	while (false)

#define CHECK_APPROX_EQUAL(actual_, expected_)																		\
	do																												\
	{																												\
		if constexpr (any_floating_point<decltype(actual_), decltype(expected_)>)									\
		{																											\
			const auto cae_expected = expected_;																	\
			INFO("expected: "sv << print_aligned{ cae_expected } << "    "sv << MUU_MAKE_STRING(expected_))			\
																													\
			const auto cae_actual = actual_;																		\
			INFO("  actual: "sv << print_aligned{ cae_actual } << "    "sv << MUU_MAKE_STRING(actual_))				\
																													\
			using cae_epsilon_type = impl::highest_ranked<decltype(actual_), decltype(expected_)>;					\
			INFO(" epsilon: "sv<< print_aligned{ constants<cae_epsilon_type>::approx_equal_epsilon })				\
																													\
			CHECK(approx_equal(cae_expected, cae_actual));															\
		}																											\
		else																										\
			CHECK((expected_) == (actual_));																		\
	}																												\
	while (false)

#define CHECK_SYMMETRIC_EQUAL(lhs, rhs)		\
	do										\
	{										\
		CHECK(lhs == rhs);					\
		CHECK_FALSE(lhs != rhs);			\
		CHECK(rhs == lhs);					\
		CHECK_FALSE(rhs != lhs);			\
	}										\
	while (false)

#define CHECK_SYMMETRIC_INEQUAL(lhs, rhs)	\
	do										\
	{										\
		CHECK_FALSE(lhs == rhs);			\
		CHECK(lhs != rhs);					\
		CHECK_FALSE(rhs == lhs);			\
		CHECK(rhs != lhs);					\
	}										\
	while (false)

template <typename T>
struct nameof_;
template <typename T>
inline constexpr auto nameof = nameof_<T>::value;

#define MAKE_NAME_OF(type)	\
	template <> struct nameof_<type> { static constexpr auto value = MUU_MAKE_STRING_VIEW(type); }

MAKE_NAME_OF(char);
MAKE_NAME_OF(wchar_t);
MAKE_NAME_OF(char16_t);
MAKE_NAME_OF(char32_t);
#ifdef __cpp_char8_t
MAKE_NAME_OF(char8_t);
#endif

#define MSVC_UNFUCK(x) x // without this msvc treats forwarded __VA_ARGS__ as a single token

#if MUU_HAS_FP16
	#define FOREACH_FLOAT_1(func)				func(__fp16)
	#define FOREACH_FLOAT_VARARGS_1(func, ...)	func(__fp16, __VA_ARGS__)
#else
	#define FOREACH_FLOAT_1(func)
	#define FOREACH_FLOAT_VARARGS_1(func, ...)
#endif
#if MUU_HAS_FLOAT16
	#define FOREACH_FLOAT_2(func)				func(_Float16)
	#define FOREACH_FLOAT_VARARGS_2(func, ...)	func(_Float16, __VA_ARGS__)
#else
	#define FOREACH_FLOAT_2(func)
	#define FOREACH_FLOAT_VARARGS_2(func, ...)
#endif
#if MUU_HAS_FLOAT128
	#define FOREACH_FLOAT_3(func)				func(float128_t)
	#define FOREACH_FLOAT_VARARGS_3(func, ...)	func(float128_t, __VA_ARGS__)
#else
	#define FOREACH_FLOAT_3(func)
	#define FOREACH_FLOAT_VARARGS_3(func, ...)
#endif

#define FOREACH_FLOAT(func)	\
	FOREACH_FLOAT_1(func)	\
	FOREACH_FLOAT_2(func)	\
	FOREACH_FLOAT_3(func)	\
	func(half)				\
	func(float)				\
	func(double)			\
	func(long double)

#define FOREACH_FLOAT_VARARGS(func, ...)						\
	MSVC_UNFUCK(FOREACH_FLOAT_VARARGS_1(func,	__VA_ARGS__))	\
	MSVC_UNFUCK(FOREACH_FLOAT_VARARGS_2(func,	__VA_ARGS__))	\
	MSVC_UNFUCK(FOREACH_FLOAT_VARARGS_3(func,	__VA_ARGS__))	\
	MSVC_UNFUCK(func(half,						__VA_ARGS__))	\
	MSVC_UNFUCK(func(float,						__VA_ARGS__))	\
	MSVC_UNFUCK(func(double,					__VA_ARGS__))	\
	MSVC_UNFUCK(func(long double,				__VA_ARGS__))
