// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the implementation of #muu::countl_zero().

#include "meta.h"
#include "is_constant_evaluated.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS; // these should be considered "intrinsics"
MUU_DISABLE_ARITHMETIC_WARNINGS;

/// \cond
namespace muu::impl
{
	template <typename T>
	MUU_CONST_GETTER
	constexpr int MUU_VECTORCALL countl_zero_naive(T val) noexcept
	{
		MUU_ASSUME(val > T{});

		using bit_type = largest<T, unsigned>;
		int count	   = 0;
		bit_type bit   = bit_type{ 1 } << (sizeof(T) * CHAR_BIT - 1);
		while (true)
		{
			if ((bit & val))
				break;
			count++;
			bit >>= 1;
		}
		return count;
	}

#define MUU_HAS_INTRINSIC_COUNTL_ZERO 1

	template <typename T>
	MUU_CONST_INLINE_GETTER
	int MUU_VECTORCALL countl_zero_intrinsic(T val) noexcept
	{
		MUU_ASSUME(val > T{});

#if MUU_GCC || MUU_CLANG

		if constexpr (std::is_same_v<T, unsigned long long>)
			return __builtin_clzll(val);
		else if constexpr (std::is_same_v<T, unsigned long>)
			return __builtin_clzl(val);
		else if constexpr (std::is_same_v<T, unsigned int> || sizeof(T) == sizeof(unsigned int))
			return __builtin_clz(static_cast<unsigned int>(val));
		else if constexpr (sizeof(T) < sizeof(unsigned int))
			return __builtin_clz(val) - static_cast<int>((sizeof(unsigned int) - sizeof(T)) * CHAR_BIT);
		else
			static_assert(always_false<T>, "Evaluated unreachable branch!");

#elif MUU_MSVC || MUU_ICC_CL

		if constexpr (sizeof(T) == sizeof(unsigned long long))
		{
	#if MUU_ARCH_X64

			unsigned long p;
			_BitScanReverse64(&p, static_cast<unsigned long long>(val));
			return 63 - static_cast<int>(p);

	#else

			if (const auto high = static_cast<unsigned long>(val >> 32); high != 0ull)
				return countl_zero_intrinsic(high);
			return 32 + countl_zero_intrinsic(static_cast<unsigned long>(val));

	#endif
		}
		else if constexpr (sizeof(T) == sizeof(unsigned long))
		{
			unsigned long p;
			_BitScanReverse(&p, static_cast<unsigned long>(val));
			return 31 - static_cast<int>(p);
		}
		else if constexpr (sizeof(T) < sizeof(unsigned long))
			return countl_zero_intrinsic(static_cast<unsigned long>(val))
				 - static_cast<int>((sizeof(unsigned long) - sizeof(T)) * CHAR_BIT);
		else
			static_assert(always_false<T>, "Evaluated unreachable branch!");

#else

	#undef MUU_HAS_INTRINSIC_COUNTL_ZERO
	#define MUU_HAS_INTRINSIC_COUNTL_ZERO 0

		static_assert(always_false<T>, "countl_zero not implemented on this compiler");

#endif
	}
}
/// \endcond

namespace muu
{
	/// \brief	Counts the number of consecutive 0 bits, starting from the left.
	/// \ingroup core
	///
	/// \remark This is equivalent to C++20's std::countl_zero, with the addition of also being
	/// 		 extended to work with enum types.
	///
	/// \tparam	T		An unsigned integer or enum type.
	/// \param 	val		The value to test.
	///
	/// \returns	The number of consecutive zeros from the left end of an integer's bits.
	MUU_CONSTRAINED_TEMPLATE(is_unsigned<T>, typename T)
	MUU_CONST_GETTER
	MUU_ATTR(flatten)
	constexpr int MUU_VECTORCALL countl_zero(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return countl_zero(static_cast<std::underlying_type_t<T>>(val));

#if MUU_HAS_INT128
		else if constexpr (std::is_same_v<T, uint128_t>)
		{
			if (const auto high = countl_zero(static_cast<uint64_t>(val >> 64)); high < 64)
				return high;
			else
				return 64 + countl_zero(static_cast<uint64_t>(val));
		}
#endif

		else
		{
			if (!val)
				return static_cast<int>(sizeof(T) * CHAR_BIT);

			if constexpr (!build::supports_is_constant_evaluated || !MUU_HAS_INTRINSIC_COUNTL_ZERO)
				return impl::countl_zero_naive(val);
			else
			{
				MUU_IF_CONSTEVAL
				{
					return impl::countl_zero_naive(val);
				}
				else
				{
					return impl::countl_zero_intrinsic(val);
				}
			}
		}
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
