// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the definition of #muu::byte_reverse().

#include "meta.h"
#include "is_constant_evaluated.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS; // these should be considered "intrinsics"
MUU_DISABLE_ARITHMETIC_WARNINGS;

/// \cond
namespace muu::impl
{
#define MUU_HAS_INTRINSIC_BYTE_REVERSE 1

	template <typename T>
	MUU_CONST_INLINE_GETTER
	T MUU_VECTORCALL byte_reverse_intrinsic(T val) noexcept
	{
#if MUU_GCC || MUU_CLANG

		if constexpr (std::is_same_v<T, uint16_t>)
			return __builtin_bswap16(val);
		else if constexpr (std::is_same_v<T, uint32_t>)
			return __builtin_bswap32(val);
		else if constexpr (std::is_same_v<T, uint64_t>)
			return __builtin_bswap64(val);
	#if MUU_HAS_INT128
		else if constexpr (std::is_same_v<T, uint128_t>)
		{
		#if MUU_HAS_BUILTIN(__builtin_bswap128)
			return __builtin_bswap128(val);
		#else
			return (static_cast<uint128_t>(byte_reverse_intrinsic(static_cast<uint64_t>(val))) << 64)
				 | byte_reverse_intrinsic(static_cast<uint64_t>(val >> 64));
		#endif
		}
	#endif
		else
			static_assert(always_false<T>, "Unsupported integer type");

#elif MUU_MSVC || MUU_ICC_CL

		if constexpr (sizeof(T) == sizeof(unsigned short))
			return static_cast<T>(_byteswap_ushort(static_cast<unsigned short>(val)));
		else if constexpr (sizeof(T) == sizeof(unsigned long))
			return static_cast<T>(_byteswap_ulong(static_cast<unsigned long>(val)));
		else if constexpr (sizeof(T) == sizeof(unsigned long long))
			return static_cast<T>(_byteswap_uint64(static_cast<unsigned long long>(val)));
		else
			static_assert(always_false<T>, "Unsupported integer type");

#else

	#undef MUU_HAS_INTRINSIC_BYTE_REVERSE
	#define MUU_HAS_INTRINSIC_BYTE_REVERSE 0

		static_assert(always_false<T>, "byte_reverse_intrinsic not implemented for this compiler");

#endif
	}

	template <typename T>
	MUU_CONST_GETTER
	constexpr T MUU_VECTORCALL byte_reverse_naive(T val) noexcept
	{
		if constexpr (sizeof(T) == sizeof(uint16_t))
		{
			return static_cast<T>(static_cast<uint32_t>(val << 8) | static_cast<uint32_t>(val >> 8));
		}
		else if constexpr (sizeof(T) == sizeof(uint32_t))
		{
			return (val << 24)				  //
				 | ((val << 8) & 0x00FF0000u) //
				 | ((val >> 8) & 0x0000FF00u) //
				 | (val >> 24);
		}
		else if constexpr (sizeof(T) == sizeof(uint64_t))
		{
			return (val << 56)						   //
				 | ((val << 40) & 0x00FF000000000000u) //
				 | ((val << 24) & 0x0000FF0000000000u) //
				 | ((val << 8) & 0x000000FF00000000u)  //
				 | ((val >> 8) & 0x00000000FF000000u)  //
				 | ((val >> 24) & 0x0000000000FF0000u) //
				 | ((val >> 40) & 0x000000000000FF00u) //
				 | (val >> 56);
		}
#if MUU_HAS_INT128
		else if constexpr (sizeof(T) == sizeof(uint128_t))
		{
			return (static_cast<uint128_t>(byte_reverse_naive(static_cast<uint64_t>(val))) << 64)
				 | byte_reverse_naive(static_cast<uint64_t>(val >> 64));
		}
#endif
		else
			static_assert(always_false<T>, "Unsupported integer type");
	}
}
/// \endcond

namespace muu
{
	/// \brief	Reverses the byte order of an unsigned integral type.
	/// \ingroup core
	///
	/// \details \cpp
	/// const auto i = 0xAABBCCDDu;
	/// const auto j = byte_reverse(i);
	/// std::cout << std::hex << i << "\n" << j;
	/// \ecpp
	///
	/// \out
	/// AABBCCDD
	/// DDCCBBAA
	/// \eout
	///
	/// \tparam	T	An unsigned integer or enum type.
	/// \param 	val	An unsigned integer or enum value.
	///
	/// \returns	A copy of the input value with the byte order reversed.
	MUU_CONSTRAINED_TEMPLATE(is_unsigned<T>, typename T)
	MUU_CONST_INLINE_GETTER
	constexpr T MUU_VECTORCALL byte_reverse(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return static_cast<T>(byte_reverse(static_cast<std::underlying_type_t<T>>(val)));
		else
		{
			if constexpr (!build::supports_is_constant_evaluated || !MUU_HAS_INTRINSIC_BYTE_REVERSE)
				return impl::byte_reverse_naive(val);
			else
			{
				MUU_IF_CONSTEVAL
				{
					return impl::byte_reverse_naive(val);
				}
				else
				{
					return impl::byte_reverse_intrinsic(val);
				}
			}
		}
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
