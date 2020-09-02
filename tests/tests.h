// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "float_test_data.h"

#if MUU_ICC
	#define UNICODE_LITERALS_OK 0
#else
	#define UNICODE_LITERALS_OK 1
#endif
#if MUU_HAS_INTERCHANGE_FP16 != SHOULD_HAVE_FP16
	#error MUU_HAS_INTERCHANGE_FP16 was not deduced correctly
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

MUU_PRAGMA_CLANG("clang diagnostic ignored \"-Wc++2a-compat\"")
MUU_PRAGMA_CLANG("clang diagnostic ignored \"-Wfloat-equal\"")
MUU_PRAGMA_GCC("GCC diagnostic ignored \"-Wfloat-equal\"")
MUU_PRAGMA_GCC("GCC diagnostic ignored \"-Wpedantic\"")
MUU_PRAGMA_GCC("GCC diagnostic ignored \"-Wpadded\"")

MUU_DISABLE_WARNINGS
#include "catch2.h"
#include <array>
#include <string_view>
#include <sstream>
#include <iostream>
#include <fstream>
MUU_NAMESPACE_START
{
	struct half;
	std::ostream& operator << (std::ostream& os, const half& value);

	template <typename T>
	[[nodiscard]]
	MUU_ATTR(const)
	MUU_CONSTEVAL double make_infinity(int sign = 1) noexcept
	{
		using ftd = float_test_data<T>;
		return bit_cast<T>(sign >= 0 ? ftd::bits_pos_inf : ftd::bits_neg_inf);
	}

	template <typename T>
	[[nodiscard]]
	MUU_ATTR(const)
	MUU_CONSTEVAL double make_nan() noexcept
	{
		return bit_cast<T>(float_test_data<T>::bits_qnan);
	}
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
	#define CHECK_STRINGS(...)		CHECK_AND_STATIC_ASSERT(__VA_ARGS__)
#endif
#ifndef CHECK_STRINGS_W
	#define CHECK_STRINGS_W(...)	CHECK_AND_STATIC_ASSERT(__VA_ARGS__)
#endif
