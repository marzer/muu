// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Bit manipulation functions.

#include "bit_cast.h"
#include "bit_ceil.h"
#include "bit_fill.h"
#include "bit_floor.h"
#include "bit_pack.h"
#include "countl_zero.h"
#include "countr_zero.h"
#include "build.h"
#include "integer_literals.h"
#include "popcount.h"
#include "impl/core_utils.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS; // these should be considered "intrinsics"
MUU_DISABLE_ARITHMETIC_WARNINGS;
MUU_PRAGMA_MSVC(warning(disable : 4191)) // unsafe pointer conversion

namespace muu
{
	/// \brief	Counts the number of consecutive 1 bits, starting from the left.
	/// \ingroup core
	///
	/// \remark This is equivalent to C++20's std::countl_one, with the addition of also being
	/// 		 extended to work with enum types.
	///
	/// \tparam	T		An unsigned integer or enum type.
	/// \param 	val		The value to test.
	///
	/// \returns	The number of consecutive ones from the left end of an integer's bits.
	MUU_CONSTRAINED_TEMPLATE(is_unsigned<T>, typename T)
	MUU_CONST_GETTER
	constexpr int MUU_VECTORCALL countl_one(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return countl_one(static_cast<std::underlying_type_t<T>>(val));
		else
			return countl_zero(static_cast<T>(~val));
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

	/// \brief	Checks if an integral value has only a single bit set.
	/// \ingroup core
	///
	/// \remark	This is equivalent to C++20's std::has_single_bit, with the addition of also being
	/// 		extended to work with enum types.
	///
	/// \tparam	T		An unsigned integer or enum type.
	/// \param 	val		The value to test.
	///
	/// \returns	True if the input value had only a single bit set (and thus was a power of two).
	MUU_CONSTRAINED_TEMPLATE(is_unsigned<T>, typename T)
	MUU_CONST_GETTER
	constexpr bool MUU_VECTORCALL has_single_bit(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return has_single_bit(static_cast<std::underlying_type_t<T>>(val));
		else
		{
			if constexpr (!build::supports_is_constant_evaluated || !MUU_HAS_INTRINSIC_POPCOUNT)
				return val != T{} && (val & (val - T{ 1 })) == T{};
			else
			{
				MUU_IF_CONSTEVAL
				{
					return val != T{} && (val & (val - T{ 1 })) == T{};
				}
				else
				{
					return impl::popcount_intrinsic(val) == 1;
				}
			}
		}
	}

	/// \brief	Finds the smallest number of bits needed to represent the given value.
	/// \ingroup core
	///
	/// \remark	This is equivalent to C++20's std::bit_width, with the addition of also being
	/// 		extended to work with enum types.
	///
	/// \tparam	T		An unsigned integer or enum type.
	/// \param 	val		The input value.
	///
	/// \returns	If `val` is not zero, calculates the number of bits needed to store `val` (i.e. `1 + log2(x)`).
	/// 			Returns `0` if `val` is zero.
	MUU_CONSTRAINED_TEMPLATE(is_unsigned<T>, typename T)
	MUU_CONST_GETTER
	constexpr T MUU_VECTORCALL bit_width(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return static_cast<T>(bit_width(static_cast<std::underlying_type_t<T>>(val)));
		else
			return static_cast<T>(sizeof(T) * CHAR_BIT - static_cast<size_t>(countl_zero(val)));
	}

	/// \brief	Gets a specific byte from an integer.
	/// \ingroup core
	///
	/// \details \cpp
	/// const auto i = 0xAABBCCDDu;
	/// //                ^ ^ ^ ^
	/// // byte indices:  3 2 1 0
	///
	/// std::cout << std::hex;
	///	std::cout << "0: " << byte_select<0>(i) << "\n";
	///	std::cout << "1: " << byte_select<1>(i) << "\n";
	///	std::cout << "2: " << byte_select<2>(i) << "\n";
	///	std::cout << "3: " << byte_select<3>(i) << "\n";
	/// \ecpp
	///
	/// \out
	/// 0: DD
	/// 1: CC
	/// 2: BB
	/// 3: AA
	/// (on a little-endian system)
	/// \eout
	///
	/// \tparam	Index	Index of the byte to retrieve.
	/// \tparam	T		An integer or enum type.
	/// \param 	val		An integer or enum value.
	///
	/// \remark The indexation order of bytes is the _memory_ order, not their
	/// 		 numeric significance (i.e. byte 0 is always the first byte in the integer's
	/// 		 memory allocation, regardless of the endianness of the platform).
	///
	/// \returns	The value of the selected byte.
	MUU_CONSTRAINED_TEMPLATE(is_integral<T>, size_t Index, typename T)
	MUU_CONST_INLINE_GETTER
	constexpr uint8_t MUU_VECTORCALL byte_select(T val) noexcept
	{
		static_assert(Index < sizeof(T),
					  "The byte index is out-of-range; it must be less than the size of the input integer");

		if constexpr (is_enum<T>)
			return byte_select<Index>(static_cast<std::underlying_type_t<T>>(val));
		else if constexpr (is_signed<T>)
			return byte_select<Index>(static_cast<make_unsigned<T>>(val));
		else if constexpr (sizeof(T) == 1_sz)
			return static_cast<uint8_t>(val);
		else
		{
			constexpr auto mask = static_cast<T>(0xFFu) << (Index * CHAR_BIT);
			if constexpr (Index == 0_sz)
				return static_cast<uint8_t>(mask & static_cast<T>(val));
			else
				return static_cast<uint8_t>((mask & static_cast<T>(val)) >> (Index * CHAR_BIT));
		}
	}

	/// \brief	Gets a specific byte from an integer.
	/// \ingroup core
	///
	/// \details \cpp
	/// const auto i = 0xAABBCCDDu;
	/// //                ^ ^ ^ ^
	/// // byte indices:  3 2 1 0
	///
	/// std::cout << std::hex;
	///	std::cout << "0: " << byte_select(i, 0) << "\n";
	///	std::cout << "1: " << byte_select(i, 1) << "\n";
	///	std::cout << "2: " << byte_select(i, 2) << "\n";
	///	std::cout << "3: " << byte_select(i, 3) << "\n";
	/// \ecpp
	///
	/// \out
	/// 0: DD
	/// 1: CC
	/// 2: BB
	/// 3: AA
	/// (on a little-endian system)
	/// \eout
	///
	/// \tparam	T		An integer or enum type.
	/// \param 	val		An integer or enum value.
	/// \param	index	Index of the byte to retrieve.
	///
	/// \remark The indexation order of bytes is the _memory_ order, not their
	/// 		 numeric significance (i.e. byte 0 is always the first byte in the integer's
	/// 		 memory allocation, regardless of the endianness of the platform).
	///
	/// \returns	The value of the selected byte, or 0 if the index was out-of-range.
	MUU_CONSTRAINED_TEMPLATE(is_integral<T>, typename T)
	MUU_CONST_INLINE_GETTER
	constexpr uint8_t MUU_VECTORCALL byte_select(T val, size_t index) noexcept
	{
		if (index > sizeof(T))
			return uint8_t{};

		if constexpr (is_enum<T>)
			return byte_select(static_cast<std::underlying_type_t<T>>(val), index);
		else if constexpr (is_signed<T>)
			return byte_select(static_cast<make_unsigned<T>>(val), index);
		else if constexpr (sizeof(T) == 1_sz)
			return static_cast<uint8_t>(val);
		else
			return static_cast<uint8_t>(static_cast<T>(val >> (index * CHAR_BIT)) & static_cast<T>(0xFFu));
	}

	/// \cond
	namespace impl
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
				return (val << 24)					 //
					 | ((val << 8) & 0x00FF0000_u32) //
					 | ((val >> 8) & 0x0000FF00_u32) //
					 | (val >> 24);
			}
			else if constexpr (sizeof(T) == sizeof(uint64_t))
			{
				return (val << 56)							  //
					 | ((val << 40) & 0x00FF000000000000_u64) //
					 | ((val << 24) & 0x0000FF0000000000_u64) //
					 | ((val << 8) & 0x000000FF00000000_u64)  //
					 | ((val >> 8) & 0x00000000FF000000_u64)  //
					 | ((val >> 24) & 0x0000000000FF0000_u64) //
					 | ((val >> 40) & 0x000000000000FF00_u64) //
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

	/// \brief	Select and re-pack arbitrary bytes from an integer.
	/// \ingroup core
	///
	/// \details \cpp
	///
	/// const auto i = 0xAABBCCDDu;
	/// //                ^ ^ ^ ^
	/// // byte indices:  3 2 1 0
	///
	/// std::cout << std::hex << std::setfill('0');
	///	std::cout << "      <0>: " << std::setw(8) <<       swizzle<0>(i) << "\n";
	///	std::cout << "   <1, 0>: " << std::setw(8) <<    swizzle<1, 0>(i) << "\n";
	///	std::cout << "<3, 2, 3>: " << std::setw(8) << swizzle<3, 2, 3>(i) << "\n";
	/// \ecpp
	///
	/// \out
	///       <0>: 000000DD
	///    <1, 0>: 0000CCDD
	/// <3, 2, 3>: 00AABBAA
	/// (on a little-endian system)
	/// \eout
	///
	/// \tparam	ByteIndices		Indices of the bytes from the source integer in the (little-endian) order they're to be packed.
	/// \tparam	T				An integer or enum type.
	/// \param	val				An integer or enum value.
	///
	/// \remark The indexation order of bytes is the _memory_ order, not their
	/// 		 numeric significance (i.e. byte 0 is always the first byte in the integer's
	/// 		 memory allocation, regardless of the endianness of the platform).
	///
	/// \returns	An integral value containing the selected bytes packed bitwise left-to-right. If the total size of the
	/// 			inputs was less than the return type, the output will be zero-padded on the left.
	MUU_CONSTRAINED_TEMPLATE(is_integral<T>, size_t... ByteIndices, typename T)
	MUU_CONST_GETTER
	constexpr auto MUU_VECTORCALL swizzle(T val) noexcept
	{
		static_assert(sizeof...(ByteIndices) > 0_sz, "At least one byte index must be specified.");
		static_assert((sizeof...(ByteIndices) * CHAR_BIT) <= (MUU_HAS_INT128 ? 128 : 64),
					  "No integer type large enough to hold the swizzled value exists on the target platform");
		static_assert((true && ... && (ByteIndices < sizeof(T))),
					  "One or more of the source byte indices was out-of-range");
		using swizzle_type = std::conditional_t<is_signed<T>,
												signed_integer<bit_ceil(sizeof...(ByteIndices) * CHAR_BIT)>,
												unsigned_integer<bit_ceil(sizeof...(ByteIndices) * CHAR_BIT)>>;
		using return_type  = std::conditional_t<sizeof...(ByteIndices) == sizeof(T), T, swizzle_type>;

		if constexpr (is_enum<T>)
			return static_cast<return_type>(swizzle<ByteIndices...>(static_cast<std::underlying_type_t<T>>(val)));
		else if constexpr (is_signed<T>)
			return static_cast<return_type>(swizzle<ByteIndices...>(static_cast<make_unsigned<T>>(val)));
		else if constexpr (sizeof...(ByteIndices) == 1_sz)
			return static_cast<return_type>(byte_select<ByteIndices...>(val));
		else
			return bit_pack<return_type>(byte_select<ByteIndices>(val)...);
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
