// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of #muu::utf8_decoder.

#include "fwd.h"
#include "impl/header_start.h"

namespace muu
{
	/// \brief A state machine for decoding UTF-8 data.
	/// \ingroup code_units
	///
	/// \detail The implementation of this decoder is based on this: https://bjoern.hoehrmann.de/utf-8/decoder/dfa/
	class MUU_TRIVIAL_ABI utf8_decoder
	{
	  private:
		uint_least32_t state{};
		static constexpr uint8_t state_table[] = {
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	1,	1,	1,	1,	1,	1,
			1,	1,	1,	1,	1,	1,	1,	1,	1,	9,	9,	9,	9,	9,	9,	9,	9,	9,	9,	9,	9,	9,	9,	9,	9,	7,	7,
			7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,
			7,	7,	7,	8,	8,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,
			2,	2,	2,	2,	2,	2,	2,	2,	10, 3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	4,	3,	3,	11, 6,	6,
			6,	5,	8,	8,	8,	8,	8,	8,	8,	8,	8,	8,	8,

			0,	12, 24, 36, 60, 96, 84, 12, 12, 12, 48, 72, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 0,	12,
			12, 12, 12, 12, 0,	12, 0,	12, 12, 12, 24, 12, 12, 12, 12, 12, 24, 12, 24, 12, 12, 12, 12, 12, 12, 12, 12,
			12, 24, 12, 12, 12, 12, 12, 24, 12, 12, 12, 12, 12, 12, 12, 24, 12, 12, 12, 12, 12, 12, 12, 12, 12, 36, 12,
			36, 12, 12, 12, 36, 12, 12, 12, 12, 12, 36, 12, 36, 12, 12, 12, 36, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12
		};
		uint_least32_t codepoint{};

	  public:
		/// \brief Returns true if the decoder has entered an error state.
		MUU_PURE_INLINE_GETTER
		constexpr bool error() const noexcept
		{
			return state == uint_least32_t{ 12u };
		}

		/// \brief Clears the error state.
		constexpr void clear_error() noexcept
		{
			MUU_ASSERT(error());
			state = uint_least32_t{};
		}

		/// \brief Returns `true` if the decoder has a decoded a full UTF-32 codepoint.
		MUU_PURE_INLINE_GETTER
		constexpr bool has_value() const noexcept
		{
			return state == uint_least32_t{};
		}

		/// \brief Returns the currently-decoded UTF-32 codepoint.
		MUU_PURE_INLINE_GETTER
		constexpr char32_t value() const noexcept
		{
			return static_cast<char32_t>(codepoint);
		}

		/// \brief Returns `true` if the decoder needs more input before it can yield a UTF-32 codepoint.
		MUU_PURE_INLINE_GETTER
		constexpr bool needs_more_input() const noexcept
		{
			return state > uint_least32_t{} && state != uint_least32_t{ 12u };
		}

		/// \brief Appends a UTF-8 code unit to the stream being decoded.
		constexpr void operator()(uint8_t code_unit) noexcept
		{
			MUU_ASSERT(!error());

			const auto type = state_table[code_unit];

			codepoint = has_value()
						  ? (uint_least32_t{ 255u } >> type) & code_unit
						  : (code_unit & uint_least32_t{ 63u }) | (static_cast<uint_least32_t>(codepoint) << 6);

			state = state_table[state + uint_least32_t{ 256u } + type];
		}

		/// \brief Appends a UTF-8 code unit to the stream being decoded.
		MUU_ALWAYS_INLINE
		constexpr void operator()(char code_unit) noexcept
		{
			(*this)(static_cast<uint8_t>(code_unit));
		}

#if MUU_HAS_CHAR8

		/// \brief Appends a UTF-8 code unit to the stream being decoded.
		MUU_ALWAYS_INLINE
		constexpr void operator()(char8_t code_unit) noexcept
		{
			(*this)(static_cast<uint8_t>(code_unit));
		}
#endif
	};
}

#include "impl/header_end.h"
