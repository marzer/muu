// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Bit manipulation functions. C++20's &lt;bit&gt; with extra steps.
/// \ingroup cpp20
/// \detail This header includes a number of others:
///		- bit_cast.h
///		- bit_ceil.h
///		- bit_fill.h
///		- bit_floor.h
///		- bit_pack.h
///		- countl.h
///		- countr.h
///		- popcount.h
///		- byte_reverse.h

#include "bit_cast.h"
#include "bit_ceil.h"
#include "bit_fill.h"
#include "bit_floor.h"
#include "bit_pack.h"
#include "countl.h"
#include "countr.h"
#include "popcount.h"
#include "byte_reverse.h"
#include "impl/core_utils.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS; // these should be considered "intrinsics"
MUU_DISABLE_ARITHMETIC_WARNINGS;
MUU_PRAGMA_MSVC(warning(disable : 4191)) // unsafe pointer conversion

namespace muu
{
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
		else if constexpr (sizeof(T) == 1u)
			return static_cast<uint8_t>(val);
		else
		{
			constexpr auto mask = static_cast<T>(0xFFu) << (Index * CHAR_BIT);
			if constexpr (Index == 0u)
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
		else if constexpr (sizeof(T) == 1u)
			return static_cast<uint8_t>(val);
		else
			return static_cast<uint8_t>(static_cast<T>(val >> (index * CHAR_BIT)) & static_cast<T>(0xFFu));
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
		static_assert(sizeof...(ByteIndices) > 0u, "At least one byte index must be specified.");
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
		else if constexpr (sizeof...(ByteIndices) == 1u)
			return static_cast<return_type>(byte_select<ByteIndices...>(val));
		else
			return bit_pack<return_type>(byte_select<ByteIndices>(val)...);
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
