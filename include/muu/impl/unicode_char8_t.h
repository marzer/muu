// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
//-----
// this file was generated by generate_unicode_functions.py - do not modify it directly

/// \file
/// \attention These are not the droids you are looking for. Try \ref strings instead.

#pragma once
#include "../../muu/fwd.h"

MUU_NAMESPACE_START
{
	/// \brief		Returns true if a UTF-8 code unit is within the ASCII range.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_ascii(char8_t c) noexcept
	{
		return c <= u8'\x7F';
	}

	/// \brief		Returns true if a UTF-8 code unit is not within the ASCII range (i.e. it is a part greater Unicode).
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_unicode(char8_t c) noexcept
	{
		return 0x80u <= c;
	}

	/// \brief		Returns true if a UTF-8 code unit is a whitespace code point from the ASCII range.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_ascii_whitespace(char8_t c) noexcept
	{
		return (u8'\t' <= c && c <= u8'\r') || c == u8' ';
	}

	/// \brief		Returns true if a UTF-8 code unit is a whitespace code point from outside the ASCII range.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_unicode_whitespace(char8_t c) noexcept
	{
		return c == 0x85u || c == 0xA0u;
	}

	/// \brief		Returns true if a UTF-8 code unit is a whitespace code point.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_whitespace(char8_t c) noexcept
	{
		return is_ascii_whitespace(c) || is_unicode_whitespace(c);
	}

	/// \brief		Returns true if a UTF-8 code unit is not a whitespace code point.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_not_whitespace(char8_t c) noexcept
	{
		return !is_whitespace(c);
	}

	/// \brief		Returns true if a UTF-8 code unit is a letter code point from the ASCII range.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_ascii_letter(char8_t c) noexcept
	{
		return (u8'A' <= c && c <= u8'Z') || (u8'a' <= c && c <= u8'z');
	}

	/// \brief		Returns true if a UTF-8 code unit is a letter code point from outside the ASCII range.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_unicode_letter(char8_t c) noexcept
	{
		if (0xAAu > c)
			return false;
		MUU_ASSUME(c <= 0xFFu);
		
		switch ((static_cast<uint_least64_t>(c) - 0xAAull) / 0x40ull)
		{
			case 0x00: return (1ull << (static_cast<uint_least64_t>(c) - 0xAAu)) & 0xFFFFDFFFFFC10801ull;
			case 0x01: return c != 0xF7u;
			MUU_NO_DEFAULT_CASE;
		}
		// 65 codepoints from 6 ranges (spanning a search area of 86)
	}

	/// \brief		Returns true if a UTF-8 code unit is a letter code point.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_letter(char8_t c) noexcept
	{
		return is_ascii_letter(c) || is_unicode_letter(c);
	}

	/// \brief		Returns true if a UTF-8 code unit is a number code point from the ASCII range.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_ascii_number(char8_t c) noexcept
	{
		return u8'0' <= c && c <= u8'9';
	}

	/// \brief		Returns true if a UTF-8 code unit is a number code point from outside the ASCII range.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_unicode_number(char8_t c) noexcept
	{
		(void)c;
		return false;
	}

	/// \brief		Returns true if a UTF-8 code unit is a number code point.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_number(char8_t c) noexcept
	{
		return is_ascii_number(c) || is_unicode_number(c);
	}

	/// \brief		Returns true if a UTF-8 code unit is a hyphen code point from the ASCII range.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_ascii_hyphen(char8_t c) noexcept
	{
		return c == u8'-';
	}

	/// \brief		Returns true if a UTF-8 code unit is a hyphen code point from outside the ASCII range.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_unicode_hyphen(char8_t c) noexcept
	{
		return c == 0xADu;
	}

	/// \brief		Returns true if a UTF-8 code unit is a hyphen code point.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_hyphen(char8_t c) noexcept
	{
		return is_ascii_hyphen(c) || is_unicode_hyphen(c);
	}

	/// \brief		Returns true if a UTF-8 code unit is a combining mark code point.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_combining_mark(char8_t c) noexcept
	{
		(void)c;
		return false;
	}

	/// \brief		Returns true if a UTF-8 code unit is an octal digit code point.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_octal_digit(char8_t c) noexcept
	{
		return u8'0' <= c && c <= u8'7';
	}

	/// \brief		Returns true if a UTF-8 code unit is a decimal digit code point.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_decimal_digit(char8_t c) noexcept
	{
		return u8'0' <= c && c <= u8'9';
	}

	/// \brief		Returns true if a UTF-8 code unit is a hexadecimal digit code point.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_hexadecimal_digit(char8_t c) noexcept
	{
		return u8'0' <= c && c <= u8'f' && (1ull << (static_cast<uint_least64_t>(c) - 0x30u)) & 0x7E0000007E03FFull;
	}

	/// \brief		Returns true if a UTF-8 code unit is an uppercase code point.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_uppercase(char8_t c) noexcept
	{
		return (u8'A' <= c && c <= u8'Z') || (0xC0u <= c && c <= 0xD6u) || (0xD8u <= c && c <= 0xDEu);
	}

	/// \brief		Returns true if a UTF-8 code unit is an lowercase code point.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_lowercase(char8_t c) noexcept
	{
		if (u8'a' > c)
			return false;
		MUU_ASSUME(c <= 0xFFu);
		
		switch ((static_cast<uint_least64_t>(c) - 0x61ull) / 0x40ull)
		{
			case 0x00: return c <= u8'z';
			case 0x01: return 0xAAu <= c && (1ull << (static_cast<uint_least64_t>(c) - 0xAAu)) & 0x60000000010801ull;
			case 0x02: return c != 0xF7u;
			MUU_NO_DEFAULT_CASE;
		}
		// 61 codepoints from 6 ranges (spanning a search area of 159)
	}

	/// \brief		Returns true if a UTF-8 code unit is a code point boundary.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_code_point_boundary(char8_t c) noexcept
	{
		return (c & 0b11000000u) != 0b10000000u;
	}

	/// \brief		Returns true if a UTF-8 code unit is in-and-of-itself a valid code point.
	/// \ingroup	characters
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_code_point(char8_t c) noexcept
	{
		return c <= 0x007Fu;
	}

}
MUU_NAMESPACE_END