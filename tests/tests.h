// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "float_test_data.h"

MUU_PRAGMA_CLANG("clang diagnostic ignored \"-Wc++2a-compat\"")
MUU_PRAGMA_CLANG("clang diagnostic ignored \"-Wfloat-equal\"")
MUU_PRAGMA_GCC("GCC diagnostic ignored \"-Wfloat-equal\"")
MUU_PRAGMA_GCC("GCC diagnostic ignored \"-Wpedantic\"")

MUU_PUSH_WARNINGS
MUU_DISABLE_ALL_WARNINGS
#include "catch2.h"
#include <sstream>
#include <string_view>
MUU_NAMESPACE_START
{
	#if MUU_HAS_INTERCHANGE_FP16
	template <> struct float_test_data<__fp16> : float_test_data_by_traits<16, 11> {};
	#endif
	#if MUU_HAS_FLOAT16
	template <> struct float_test_data<float16_t> : float_test_data_by_traits<16, 11> {};
	#endif
	#if MUU_HAS_FLOAT128
	template <> struct float_test_data<float128_t> : float_test_data_by_traits<128, __FLT128_MANT_DIG__> {};
	#endif

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
MUU_POP_WARNINGS

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
