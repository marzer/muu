// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the definitions of #muu::bit_fill_left() and #muu::bit_fill_right().

#include "meta.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS; // these should be considered "intrinsics"
MUU_DISABLE_ARITHMETIC_WARNINGS;

namespace muu
{
	/// \brief	Returns an unsigned integer filled from the right
	/// 		with the desired number of consecutive ones.
	/// \ingroup core
	///
	/// \details \cpp
	/// const auto val1 = bit_fill_right<uint32_t>(5);
	/// const auto val2 = 0b00000000000000000000000000011111u;
	/// assert(val1 == val2);
	///  \ecpp
	///
	/// \tparam	T		An unsigned integer type.
	/// \param 	count	Number of consecutive ones.
	///
	/// \returns	An instance of T right-filled with the desired number of ones.
	MUU_CONSTRAINED_TEMPLATE(is_unsigned<T>, typename T)
	MUU_CONST_GETTER
	constexpr T MUU_VECTORCALL bit_fill_right(size_t count) noexcept
	{
		if constexpr (is_enum<T>)
			return T{ bit_fill_right<remove_enum<remove_cvref<T>>>(count) };
		else
		{
			if (!count)
				return T{};
			if (count >= build::bits_per_byte * sizeof(T))
				return static_cast<T>(~T{});
			return static_cast<T>((T{ 1 } << count) - T{ 1 });
		}
	}

	/// \brief	Returns an unsigned integer filled from the left
	/// 		with the desired number of consecutive ones.
	/// \ingroup core
	///
	/// \details \cpp
	/// const auto val1 = bit_fill_left<uint32_t>(5);
	/// const auto val2 = 0b11111000000000000000000000000000u;
	/// assert(val1 == val2);
	///  \ecpp
	///
	/// \tparam	T		An unsigned integer type.
	/// \param 	count	Number of consecutive ones.
	///
	/// \returns	An instance of T left-filled with the desired number of ones.
	MUU_CONSTRAINED_TEMPLATE(is_unsigned<T>, typename T)
	MUU_CONST_GETTER
	constexpr T MUU_VECTORCALL bit_fill_left(size_t count) noexcept
	{
		if constexpr (is_enum<T>)
			return T{ bit_fill_left<remove_enum<remove_cvref<T>>>(count) };
		else
		{
			if (!count)
				return T{};
			if (count >= build::bits_per_byte * sizeof(T))
				return static_cast<T>(~T{});
			return static_cast<T>(bit_fill_right<T>(count) << (build::bits_per_byte * sizeof(T) - count));
		}
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
