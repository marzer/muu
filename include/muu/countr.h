// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the definitions of #muu::countr_zero() and #muu::countr_one().

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
	constexpr int MUU_VECTORCALL countr_zero_naive(T val) noexcept
	{
		MUU_ASSUME(val > T{});

		using bit_type = largest<T, unsigned>;
		int count	   = 0;
		bit_type bit   = 1;
		while (true)
		{
			if ((bit & val))
				break;
			count++;
			bit <<= 1;
		}
		return count;
	}

#define MUU_HAS_INTRINSIC_COUNTR_ZERO 1

	template <typename T>
	MUU_CONST_INLINE_GETTER
	int MUU_VECTORCALL countr_zero_intrinsic(T val) noexcept
	{
		MUU_ASSUME(val > T{});

#if MUU_GCC || MUU_CLANG

		if constexpr (std::is_same_v<T, unsigned long long>)
			return __builtin_ctzll(val);
		else if constexpr (std::is_same_v<T, unsigned long>)
			return __builtin_ctzl(val);
		else if constexpr (std::is_same_v<T, unsigned int> || sizeof(T) <= sizeof(unsigned int))
			return __builtin_ctz(val);
		else
			static_assert(always_false<T>, "Evaluated unreachable branch!");

#elif MUU_MSVC || MUU_ICC_CL

		if constexpr (sizeof(T) == sizeof(unsigned long long))
		{
	#if MUU_ARCH_X64

			unsigned long p;
			_BitScanForward64(&p, static_cast<unsigned long long>(val));
			return static_cast<int>(p);

	#else

			if (const auto low = static_cast<unsigned long>(val); low != 0ull)
				return countr_zero_intrinsic(low);
			return 32 + countr_zero_intrinsic(static_cast<unsigned long>(val >> 32));

	#endif
		}
		else if constexpr (sizeof(T) == sizeof(unsigned long))
		{
			unsigned long p;
			_BitScanForward(&p, static_cast<unsigned long>(val));
			return static_cast<int>(p);
		}
		else if constexpr (sizeof(T) < sizeof(unsigned long))
			return countr_zero_intrinsic(static_cast<unsigned long>(val));
		else
			static_assert(always_false<T>, "Evaluated unreachable branch!");

#else

	#undef MUU_HAS_INTRINSIC_COUNTR_ZERO
	#define MUU_HAS_INTRINSIC_COUNTR_ZERO 0

		static_assert(always_false<T>, "countr_zero not implemented on this compiler");

#endif
	}
}
/// \endcond

namespace muu
{
	/// \brief	Counts the number of consecutive 0 bits, starting from the right.
	/// \ingroup core
	///
	/// \remark This is equivalent to C++20's std::countr_zero, with the addition of also being
	/// 		 extended to work with enum types.
	///
	/// \tparam	T		An unsigned integer or enum type.
	/// \param 	val		The input value.
	///
	/// \returns	The number of consecutive zeros from the right end of an integer's bits.
	MUU_CONSTRAINED_TEMPLATE(is_unsigned<T>, typename T)
	MUU_CONST_GETTER
	MUU_ATTR(flatten)
	constexpr int MUU_VECTORCALL countr_zero(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return countr_zero(static_cast<std::underlying_type_t<T>>(val));

#if MUU_HAS_INT128
		else if constexpr (std::is_same_v<T, uint128_t>)
		{
			if (const auto low = countr_zero(static_cast<uint64_t>(val)); low < 64)
				return low;
			else
				return 64 + countr_zero(static_cast<uint64_t>(val >> 64));
		}
#endif

		else
		{
			if (!val)
				return static_cast<int>(sizeof(T) * CHAR_BIT);

			if constexpr (!build::supports_is_constant_evaluated || !MUU_HAS_INTRINSIC_COUNTR_ZERO)
				return impl::countr_zero_naive(val);
			else
			{
				MUU_IF_CONSTEVAL
				{
					return impl::countr_zero_naive(val);
				}
				else
				{
					return impl::countr_zero_intrinsic(val);
				}
			}
		}
	}

	/// \brief	Counts the number of consecutive 1 bits, starting from the right.
	/// \ingroup core
	///
	/// \remark This is equivalent to C++20's std::countr_one, with the addition of also being
	/// 		 extended to work with enum types.
	///
	/// \tparam	T		An unsigned integer or enum type.
	/// \param 	val		The value to test.
	///
	/// \returns	The number of consecutive ones from the right end of an integer's bits.
	MUU_CONSTRAINED_TEMPLATE(is_unsigned<T>, typename T)
	MUU_CONST_GETTER
	constexpr int MUU_VECTORCALL countr_one(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return countr_one(static_cast<std::underlying_type_t<T>>(val));
		else
			return countr_zero(static_cast<T>(~val));
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
