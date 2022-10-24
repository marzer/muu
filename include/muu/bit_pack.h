// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the implementation of #muu::bit_pack().

#include "meta.h"
#include "bit_ceil.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS; // these should be considered "intrinsics"
MUU_DISABLE_ARITHMETIC_WARNINGS;

namespace muu
{
	/// \brief	Bitwise-packs integers left-to-right into a larger integer.
	/// \ingroup core
	///
	/// \details \cpp
	/// auto   val1  = bit_pack(0xAABB_u16, 0xCCDD_u16);
	/// assert(val1 == 0xAABBCCDD_u32);
	///
	/// auto   val2  = bit_pack(0xAABB_u16, 0xCCDD_u16, 0xEEFF_u16);
	/// assert(val2 == 0x0000AABBCCDDEEFF_u64);
	///               // ^^^^ input was 48 bits, zero-padded to 64 on the left
	/// \ecpp
	///
	/// \tparam	Return	An integer or enum type,
	///			or leave as `void` to choose an unsigned type based on the total size of the inputs.
	/// \tparam	T	  	An integer or enum type.
	/// \tparam	U	  	An integer or enum type.
	/// \tparam	V	  	Integer or enum types.
	/// \param 	val1	The left-most value to be packed.
	/// \param 	val2	The second-left-most value to be packed.
	/// \param 	vals	Any remaining values to be packed.
	///
	/// \returns	An integral value containing the input values packed bitwise left-to-right. If the total size of the
	/// 			inputs was less than the return type, the output will be zero-padded on the left.
	MUU_CONSTRAINED_TEMPLATE((all_integral<T, U, V...>), typename Return = void, typename T, typename U, typename... V)
	MUU_CONST_GETTER
	constexpr auto MUU_VECTORCALL bit_pack(T val1, U val2, V... vals) noexcept
	{
		static_assert(!is_cvref<Return>, "Return type cannot be const, volatile, or a reference");
		static_assert((total_size<T, U, V...> * CHAR_BIT) <= (MUU_HAS_INT128 ? 128 : 64),
					  "No integer type large enough to hold the packed values exists on the target platform");
		using return_type = std::conditional_t<std::is_void_v<Return>,
											   unsigned_integer<bit_ceil(total_size<T, U, V...> * CHAR_BIT)>,
											   Return>;
		static_assert(total_size<T, U, V...> <= sizeof(return_type), "Return type cannot fit all the input values");

		if constexpr (any_enum<return_type, T, U, V...>)
		{
			return static_cast<return_type>(
				bit_pack<remove_enum<return_type>>(static_cast<std::underlying_type_t<T>>(val1),
												   static_cast<std::underlying_type_t<U>>(val2),
												   static_cast<std::underlying_type_t<V>>(vals)...));
		}
		else if constexpr (any_signed<return_type, T, U, V...>)
		{
			return static_cast<return_type>(
				bit_pack<make_unsigned<return_type>>(static_cast<make_unsigned<T>>(val1),
													 static_cast<make_unsigned<U>>(val2),
													 static_cast<make_unsigned<V>>(vals)...));
		}
		else if constexpr (sizeof...(V) > 0)
		{
			return static_cast<return_type>(
				static_cast<return_type>(static_cast<return_type>(val1) << (total_size<U, V...> * CHAR_BIT))
				| bit_pack<return_type>(val2, vals...));
		}
		else
		{
			return static_cast<return_type>(
				static_cast<return_type>(static_cast<return_type>(val1) << (sizeof(U) * CHAR_BIT))
				| static_cast<return_type>(val2));
		}
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
