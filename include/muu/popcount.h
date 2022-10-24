// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the implementation of #muu::popcount().

#include "integer_literals.h"
#include "meta.h"
#include "bit_pack.h"
#include "is_constant_evaluated.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS; // these should be considered "intrinsics"
MUU_DISABLE_ARITHMETIC_WARNINGS;

/// \cond
namespace muu::impl
{
	template <size_t>
	struct popcount_traits;

	template <>
	struct popcount_traits<8>
	{
		static constexpr uint8_t m1	 = 0x55_u8;
		static constexpr uint8_t m2	 = 0x33_u8;
		static constexpr uint8_t m4	 = 0x0f_u8;
		static constexpr uint8_t h01 = 0x01_u8;
		static constexpr int rsh	 = 0;
	};

	template <>
	struct popcount_traits<16>
	{
		static constexpr uint16_t m1  = 0x5555_u16;
		static constexpr uint16_t m2  = 0x3333_u16;
		static constexpr uint16_t m4  = 0x0f0f_u16;
		static constexpr uint16_t h01 = 0x0101_u16;
		static constexpr int rsh	  = 8;
	};

	template <>
	struct popcount_traits<32>
	{
		static constexpr uint32_t m1  = 0x55555555_u32;
		static constexpr uint32_t m2  = 0x33333333_u32;
		static constexpr uint32_t m4  = 0x0f0f0f0f_u32;
		static constexpr uint32_t h01 = 0x01010101_u32;
		static constexpr int rsh	  = 24;
	};

	template <>
	struct popcount_traits<64>
	{
		static constexpr uint64_t m1  = 0x5555555555555555_u64;
		static constexpr uint64_t m2  = 0x3333333333333333_u64;
		static constexpr uint64_t m4  = 0x0f0f0f0f0f0f0f0f_u64;
		static constexpr uint64_t h01 = 0x0101010101010101_u64;
		static constexpr int rsh	  = 56;
	};

#if MUU_HAS_INT128
	template <>
	struct popcount_traits<128>
	{
		static constexpr uint128_t m1  = bit_pack(0x5555555555555555_u64, 0x5555555555555555_u64);
		static constexpr uint128_t m2  = bit_pack(0x3333333333333333_u64, 0x3333333333333333_u64);
		static constexpr uint128_t m4  = bit_pack(0x0f0f0f0f0f0f0f0f_u64, 0x0f0f0f0f0f0f0f0f_u64);
		static constexpr uint128_t h01 = bit_pack(0x0101010101010101_u64, 0x0101010101010101_u64);
		static constexpr int rsh	   = 120;
	};
#endif

	template <typename T>
	MUU_CONST_GETTER
	constexpr int MUU_VECTORCALL popcount_naive(T val) noexcept
	{
		MUU_ASSUME(val > T{});

		using pt = popcount_traits<sizeof(T) * CHAR_BIT>;
		val -= ((val >> 1) & pt::m1);
		val = static_cast<T>((val & pt::m2) + ((val >> 2) & pt::m2));
		return static_cast<int>(static_cast<T>(((val + (val >> 4)) & pt::m4) * pt::h01) >> pt::rsh);
	}

#define MUU_HAS_INTRINSIC_POPCOUNT 1

	template <typename T>
	MUU_CONST_INLINE_GETTER
	auto MUU_VECTORCALL popcount_intrinsic(T val) noexcept
	{
		MUU_ASSUME(val > T{});

#if MUU_GCC || MUU_CLANG

		if constexpr (sizeof(T) <= sizeof(unsigned int))
			return __builtin_popcount(static_cast<unsigned int>(val));
		else if constexpr (std::is_same_v<T, unsigned long>)
			return __builtin_popcountl(val);
		else if constexpr (std::is_same_v<T, unsigned long long>)
			return __builtin_popcountll(val);
	#if MUU_HAS_INT128
		else if constexpr (std::is_same_v<T, uint128_t>)
			return __builtin_popcountll(static_cast<unsigned long long>(val >> 64))
				 + __builtin_popcountll(static_cast<unsigned long long>(val));
	#endif
		else
			static_assert(always_false<T>, "Unsupported integer type");

#elif MUU_ICC

		if constexpr (sizeof(T) <= sizeof(int))
			return _popcnt32(static_cast<int>(val));
		else if constexpr (sizeof(T) == sizeof(__int64))
			return _popcnt64(static_cast<__int64>(val));
		else
			static_assert(always_false<T>, "Unsupported integer type");

#elif MUU_MSVC

		if constexpr (sizeof(T) <= sizeof(unsigned short))
		{
	#if MUU_MSVC >= 1928 // VS 16.8
			return __popcnt16(static_cast<unsigned short>(val));
	#else
			return popcount_naive(val);
	#endif
		}
		else if constexpr (sizeof(T) == sizeof(unsigned int))
			return __popcnt(static_cast<unsigned int>(val));
		else if constexpr (std::is_same_v<T, unsigned __int64>)
		{
	#if MUU_MSVC >= 1928 && MUU_ARCH_X64 // VS 16.8
			return __popcnt64(static_cast<unsigned __int64>(val));
	#else
			return __popcnt(static_cast<unsigned int>(val >> 32)) + __popcnt(static_cast<unsigned int>(val));
	#endif
		}
		else
			static_assert(always_false<T>, "Unsupported integer type");

#else

	#undef MUU_HAS_INTRINSIC_POPCOUNT
	#define MUU_HAS_INTRINSIC_POPCOUNT 0

		static_assert(always_false<T>, "popcount_intrinsic not implemented for this compiler");

#endif
	}
}
/// \endcond

namespace muu
{
	/// \brief	Counts the number of set bits (the 'population count') of an unsigned integer.
	/// \ingroup core
	///
	/// \remark	This is equivalent to C++20's std::popcount, with the addition of also being
	/// 		extended to work with enum types.
	///
	/// \tparam	T		An unsigned integer or enum type.
	/// \param 	val		The input value.
	///
	/// \returns	The number of bits that were set to `1` in `val`.
	MUU_CONSTRAINED_TEMPLATE(is_unsigned<T>, typename T)
	MUU_CONST_GETTER
	MUU_ATTR(flatten)
	constexpr int MUU_VECTORCALL popcount(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return popcount(static_cast<std::underlying_type_t<T>>(val));
		else
		{
			if (!val)
				return 0;

			if constexpr (!build::supports_is_constant_evaluated || !MUU_HAS_INTRINSIC_POPCOUNT)
				return impl::popcount_naive(val);
			else
			{
				MUU_IF_CONSTEVAL
				{
					return impl::popcount_naive(val);
				}
				else
				{
					return static_cast<int>(impl::popcount_intrinsic(val));
				}
			}
		}
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
