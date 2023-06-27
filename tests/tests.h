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
#if MUU_CLANG >= 13
MUU_PRAGMA_CLANG(diagnostic ignored "-Wreserved-identifier") // catch-2 spams __ in the internal identifiers :(
#endif
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
#include "../include/muu/for_sequence.h"
#include "../include/muu/pointer_cast.h"
#include "../include/muu/impl/core_utils.h"
#include "../include/muu/impl/std_iosfwd.h"
#include "../include/muu/impl/std_memcpy.h"
#include "../include/muu/impl/std_string_view.h"

MUU_DISABLE_WARNINGS;
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
MUU_DISABLE_SPAM_WARNINGS;

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
// (because compilers suck)
#if MUU_ICC
	#define CHECK_W(...)				   MUU_NOOP
	#define CHECK_AND_STATIC_ASSERT_W(...) MUU_NOOP
#elif MUU_CLANG && MUU_CLANG <= 8
	#define CHECK_W(...)				   CHECK(__VA_ARGS__)
	#define CHECK_AND_STATIC_ASSERT_W(...) CHECK(__VA_ARGS__)
#else
	#define CHECK_W(...)				   CHECK(__VA_ARGS__)
	#define CHECK_AND_STATIC_ASSERT_W(...) CHECK_AND_STATIC_ASSERT(__VA_ARGS__)
#endif
#if MUU_HAS_CHAR8_STRINGS
	#define CHECK_u8(...)					CHECK(__VA_ARGS__)
	#define CHECK_AND_STATIC_ASSERT_u8(...) CHECK_AND_STATIC_ASSERT(__VA_ARGS__)
#else
	#define CHECK_u8(...)					MUU_NOOP
	#define CHECK_AND_STATIC_ASSERT_u8(...) MUU_NOOP
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
		{                                                                                                              \
			static_cast<void>(epsilon_);                                                                               \
			CHECK((expected_) == (actual_));                                                                           \
		}                                                                                                              \
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
		CHECK((lhs) == (rhs));                                                                                         \
		CHECK_FALSE((lhs) != (rhs));                                                                                   \
		CHECK((rhs) == (lhs));                                                                                         \
		CHECK_FALSE((rhs) != (lhs));                                                                                   \
	}                                                                                                                  \
	while (false)

#define CHECK_SYMMETRIC_INEQUAL(lhs, rhs)                                                                              \
	do                                                                                                                 \
	{                                                                                                                  \
		CHECK_FALSE((lhs) == (rhs));                                                                                   \
		CHECK((lhs) != (rhs));                                                                                         \
		CHECK_FALSE((rhs) == (lhs));                                                                                   \
		CHECK((rhs) != (lhs));                                                                                         \
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

#if MUU_MSVC_LIKE // double and long double are identical on msvc so we just ignore long double
	#define STANDARD_FLOATS float, double
#else
	#define STANDARD_FLOATS float, double, long double
#endif

#define SMALL_FLOATS_1

#if MUU_HAS_FLOAT16
	#define SMALL_FLOATS_2 , _Float16
#else
	#define SMALL_FLOATS_2
#endif

#if MUU_HAS_FLOAT128
	#define LARGE_FLOATS_1 , float128_t
#else
	#define LARGE_FLOATS_1
#endif

#define SMALL_FLOATS			half SMALL_FLOATS_1 SMALL_FLOATS_2
#define EXTENDED_FLOATS			SMALL_FLOATS LARGE_FLOATS_1
#define NON_FP16_FLOATS			STANDARD_FLOATS LARGE_FLOATS_1
#define ALL_FLOATS				STANDARD_FLOATS, EXTENDED_FLOATS
#define ALL_ARITHMETIC			ALL_FLOATS, ALL_INTS
#define ALL_SIGNED_ARITHMETIC	ALL_FLOATS, ALL_SIGNED_INTS
#define ALL_UNSIGNED_ARITHMETIC ALL_UNSIGNED_INTS

// todo: refactor this crap out

#define FOREACH_FLOAT_VARARGS_1(func, ...)

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

#define RANDOM_ITERATIONS for (size_t ri_iteration = 0; ri_iteration < 1000_sz; ri_iteration++)

#define SKIP_INF_NAN(...)                                                                                              \
	if (muu::infinity_or_nan(__VA_ARGS__))                                                                             \
	continue

// nameof
// --------------------------------------------------------------------------------------------------------------

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
	MUU_CONST_GETTER
	MUU_CONSTEVAL
	muu::remove_cvref<T> make_infinity(int sign = 1) noexcept
	{
		using ftd = float_test_data<muu::remove_cvref<T>>;
		return muu::bit_cast<muu::remove_cvref<T>>(sign >= 0 ? ftd::bits_pos_inf : ftd::bits_neg_inf);
	}

	template <typename T>
	MUU_CONST_GETTER
	MUU_CONSTEVAL
	muu::remove_cvref<T> make_nan() noexcept
	{
		return muu::bit_cast<muu::remove_cvref<T>>(float_test_data<muu::remove_cvref<T>>::bits_qnan);
	}

	template <typename T>
	MUU_NODISCARD
	inline T random() noexcept
	{
		if constexpr (is_floating_point<T>)			// 0.0 - 1.0
		{
			if constexpr (impl::is_small_float_<T>) // NaN's otherwise
			{
				T val;
				do
				{
					static constexpr int granularity = 500;

					val = static_cast<T>(static_cast<float>(::rand() % granularity)
										 / static_cast<float>(granularity - 1));
				}
				while (infinity_or_nan(val));
				return val;
			}
			else
			{
				using fp = impl::highest_ranked<T, float>;
				return static_cast<T>(static_cast<fp>(::rand()) / static_cast<fp>(RAND_MAX));
			}
		}
		else if constexpr (std::is_same_v<std::remove_cv_t<T>, bool>)
		{
			return !(::rand() % 2);
		}
		else
		{
			if constexpr (RAND_MAX > constants<T>::highest)
				return static_cast<T>(::rand() % static_cast<decltype(::rand())>(constants<T>::highest));
			else
				return static_cast<T>(::rand());
		}
	}

	template <typename T, typename Max>
	MUU_NODISCARD
	inline T random(Max max_) noexcept
	{
		if constexpr (is_floating_point<T>)
		{
			T val;
			do
			{
				using fp = impl::highest_ranked<T, float>;
				val		 = static_cast<T>(random<fp>() * static_cast<fp>(max_));
			}
			while (infinity_or_nan(val));
			return val;
		}
		else
			return static_cast<T>(random<double>() * static_cast<double>(max_));
	}

	template <typename T, typename Min, typename Max>
	MUU_NODISCARD
	inline T random(Min min_, Max max_) noexcept
	{
		if constexpr (is_floating_point<T>)
		{
			T val;
			do
			{
				val = static_cast<T>(static_cast<T>(min_) + random<T>(static_cast<T>(max_) - static_cast<T>(min_)));
			}
			while (infinity_or_nan(val));
			return val;
		}
		else
		{
			return static_cast<T>(static_cast<T>(min_) + random<T>(static_cast<T>(max_) - static_cast<T>(min_)));
		}
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

	MUU_CONSTRAINED_TEMPLATE(is_floating_point<T>, typename T)
	inline Approx approx(T val, T eps) noexcept
	{
		Approx a(val);
		a.epsilon(eps);
		return a;
	}

	MUU_CONSTRAINED_TEMPLATE(is_floating_point<T>, typename T)
	inline Approx approx(T val) noexcept
	{
		Approx a(val);
		a.epsilon(std::numeric_limits<T>::epsilon() * 100); // catch2 default
		return a;
	}

	MUU_CONSTRAINED_TEMPLATE(!is_floating_point<T>, typename T)
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

				lhs << std::fixed << std::setprecision(constants<T>::decimal_digits + 2) << rhs.value;
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

	template <size_t Skip, size_t Length, typename T>
	struct print_bits
	{
		T value;

		static_assert(is_unsigned<T> && is_integer<T>);

		template <typename Char>
		friend std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& lhs, const print_bits& rhs)
		{
			T v	   = rhs.value;
			T mask = T{ 1 } << ((sizeof(T) * CHAR_BIT) - 1u);
			if constexpr (Skip != 0)
			{
				mask >>= Skip;
				v >>= Skip;
			}
			static constexpr size_t Remaining = (sizeof(T) * CHAR_BIT) - Skip;
			if constexpr (Remaining > Length)
				mask >>= (Remaining - Length);

			while (mask)
			{
				lhs.put(static_cast<Char>(48 + static_cast<int>(!!(v & mask))));
				mask >>= T{ 1 };
			}

			return lhs;
		}
	};

	template <typename T>
	print_bits(T) -> print_bits<0, static_cast<size_t>(-1), T>;

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

#if MUU_CPP >= 20 && MUU_CLANG && MUU_CLANG <= 14 // https://github.com/llvm/llvm-project/issues/55560

	#define CLANG_LIBSTDCPP_STRINGS_LINK_ERROR_WORKAROUND                                                              \
		namespace                                                                                                      \
		{                                                                                                              \
			[[maybe_unused]] static std::u16string clang_string_workaround_1(const char16_t* a, const char16_t* b)     \
			{                                                                                                          \
				return { a, b };                                                                                       \
			}                                                                                                          \
			[[maybe_unused]] static std::u32string clang_string_workaround_2(const char32_t* a, const char32_t* b)     \
			{                                                                                                          \
				return { a, b };                                                                                       \
			}                                                                                                          \
			[[maybe_unused]] static std::u8string clang_string_workaround_3(const char8_t* a, const char8_t* b)        \
			{                                                                                                          \
				return { a, b };                                                                                       \
			}                                                                                                          \
		}                                                                                                              \
		static_assert(true)

#else

	#define CLANG_LIBSTDCPP_STRINGS_LINK_ERROR_WORKAROUND static_assert(true)

#endif
