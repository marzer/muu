// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \ingroup cpp20
/// \brief Contains the definition of #muu::bit_floor().

#include "meta.h"
#include "countl.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS; // these should be considered "intrinsics"
MUU_DISABLE_ARITHMETIC_WARNINGS;

namespace muu
{
	//% bit_floor start
	/// \brief	Finds the largest integral power of two not greater than the given value.
	/// \ingroup core
	///
	/// \remark	This is equivalent to C++20's std::bit_floor, with the addition of also being
	/// 		extended to work with enum types.
	///
	/// \tparam	T		An unsigned integer or enum type.
	/// \param 	val		The input value.
	///
	/// \returns	Zero if `val` is zero; otherwise, the largest integral power of two that is not greater than `val`.
	MUU_CONSTRAINED_TEMPLATE(is_unsigned<T>, typename T)
	MUU_CONST_GETTER
	constexpr T MUU_VECTORCALL bit_floor(T val) noexcept
	{
		static_assert(!is_cvref<T>);

		if constexpr (std::is_enum_v<T>)
			return static_cast<T>(bit_floor(static_cast<std::underlying_type_t<T>>(val)));
		else
		{
			if (!val)
				return T{ 0 };
			return T{ 1 } << (sizeof(T) * size_t{ CHAR_BIT } - size_t{ 1 } - static_cast<size_t>(countl_zero(val)));
		}
	}
	//% bit_floor end
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
