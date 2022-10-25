// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the implementation of# muu::utf16_decoder.

#include "fwd.h"
#include "integer_literals.h"
#include "impl/core_utils.h"
#include "impl/header_start.h"

namespace muu
{
	/// \brief A state machine for decoding UTF-16 data.
	/// \ingroup code_units
	class MUU_TRIVIAL_ABI utf16_decoder
	{
		enum decoder_state : unsigned
		{
			ds_initial,
			ds_expecting_low_surrogate,
			ds_has_codepoint,
			ds_error
		};
		decoder_state state{};
		uint_least32_t codepoint{};

	  public:
		/// \brief Returns true if the decoder has entered an error state.
		MUU_PURE_INLINE_GETTER
		constexpr bool error() const noexcept
		{
			return state == ds_error;
		}

		/// \brief Clears the error state.
		constexpr void clear_error() noexcept
		{
			MUU_ASSERT(error());
			state = ds_initial;
		}

		/// \brief Returns `true` if the decoder has a decoded a full UTF-32 codepoint.
		MUU_PURE_INLINE_GETTER
		constexpr bool has_value() const noexcept
		{
			return state == ds_has_codepoint;
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
			return state == ds_expecting_low_surrogate;
		}

		/// \brief Appends a UTF-16 code unit to the stream being decoded.
		constexpr void operator()(uint16_t code_unit) noexcept
		{
			MUU_ASSERT(!error());

			enum code_unit_classes : unsigned
			{
				basic_multilingual,
				low_surrogate  = 0b00010000u,
				high_surrogate = 0b00100000u
			};

			const auto code_unit_class = (static_cast<unsigned>(between(code_unit, 0xDC00_u16, 0xDFFF_u16)) << 4)
									   | (static_cast<unsigned>(between(code_unit, 0xD800_u16, 0xDBFF_u16)) << 5);

			switch (code_unit_class | state)
			{
				case unwrap(basic_multilingual) | ds_initial: [[fallthrough]];
				case unwrap(basic_multilingual) | ds_has_codepoint:
					codepoint = static_cast<uint_least32_t>(code_unit);
					state	  = ds_has_codepoint;
					return;

				case unwrap(high_surrogate) | ds_initial: [[fallthrough]];
				case unwrap(high_surrogate) | ds_has_codepoint:
					codepoint = static_cast<uint_least32_t>(code_unit); // will get multiplexed in the next step
					state	  = ds_has_codepoint;
					return;

				case unwrap(low_surrogate) | ds_expecting_low_surrogate:
					codepoint =
						((codepoint - 0xD800u) << 10) + (static_cast<uint_least32_t>(code_unit) - 0xDC00u) + 0x10000u;
					state = ds_has_codepoint;
					return;

				default: state = ds_error;
			}
		}

		/// \brief Appends a UTF-16 code unit to the stream being decoded.
		MUU_ALWAYS_INLINE
		constexpr void operator()(char16_t code_unit) noexcept
		{
			(*this)(static_cast<uint16_t>(code_unit));
		}

#if MUU_WCHAR_BYTES == 2

		/// \brief Appends a UTF-16 code unit to the stream being decoded.
		/// \availability This overload is only available when `sizeof(wchar_t) == 2`.
		MUU_ALWAYS_INLINE
		constexpr void operator()(wchar_t code_unit) noexcept
		{
			(*this)(static_cast<uint16_t>(code_unit));
		}

#endif
	};
}

#include "impl/header_end.h"
