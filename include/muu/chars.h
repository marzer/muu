// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Functions to simplify working with characters (code units).

#include "impl/core_constants.h"
#include "impl/unicode_char.h"
#include "impl/unicode_wchar_t.h"
#include "impl/unicode_char16_t.h"
#include "impl/unicode_char32_t.h"
#if MUU_HAS_CHAR8
	#include "impl/unicode_char8_t.h"
#endif
#include "impl/std_type_traits.h"
#include "impl/std_iosfwd.h"
#include "impl/header_start.h"
MUU_DISABLE_ARITHMETIC_WARNINGS;
MUU_FORCE_NDEBUG_OPTIMIZATIONS;

namespace muu
{
	/// \brief Converts a code units containing a hex value `[0-9, a-f, A-F]` to it's equivalent unsigned integer `[0x0, 0xF]`.
	/// \ingroup code_units
	template <typename T>
	MUU_CONST_GETTER
	constexpr unsigned hex_to_dec(T codepoint) noexcept
	{
		if constexpr (std::is_same_v<remove_cvref<T>, unsigned>)
			return codepoint >= 0x41u					 // >= 'A'
					 ? 10u + (codepoint | 0x20u) - 0x61u // - 'a'
					 : codepoint - 0x30u				 // - '0'
				;
		else
			return hex_to_dec(static_cast<unsigned>(codepoint));
	}

	/// \brief Converts an unsigned integer in the range `[0x0, 0xF]` to it's equivalent code units `[0-9, a-f, A-F]`.
	/// \ingroup code_units
	template <typename Char = char>
	MUU_CONST_INLINE_GETTER
	constexpr Char dec_to_hex(unsigned val, Char a = constants<Char>::letter_a) noexcept
	{
		return static_cast<Char>(val >= 10u ? static_cast<unsigned>(a) + (val - 10u) : constants<Char>::digit_0 + val);
	}

	/// \brief A pair of code units in a byte pair.
	/// \ingroup code_units
	template <typename Char = char>
	struct hex_char_pair
	{
		Char high;
		Char low;

		template <typename Traits>
		friend std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& lhs,
															const hex_char_pair& rhs)
		{
			lhs.put(rhs.high);
			lhs.put(rhs.low);
			return lhs;
		}
	};

	/// \brief Converts a byte value to it's equivalent pair of code units in the range `[0x0, 0xF]`.
	/// \ingroup code_units
	template <typename Char = char>
	MUU_CONST_GETTER
	constexpr hex_char_pair<Char> byte_to_hex(uint8_t byte, Char a = constants<Char>::letter_a) noexcept
	{
		return { dec_to_hex(byte / 16u, a), dec_to_hex(byte % 16u, a) };
	}

	/// \brief Converts a byte value to it's equivalent pair of code units in the range `[0x0, 0xF]`.
	/// \ingroup code_units
	template <typename Char = char>
	MUU_CONST_INLINE_GETTER
	constexpr hex_char_pair<Char> byte_to_hex(std::byte byte, Char a = constants<Char>::letter_a) noexcept
	{
		return byte_to_hex(unwrap(byte), a);
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
