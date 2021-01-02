// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
//-----
// this file was generated by generate_unicode_functions.py - do not modify it directly

#pragma once
#include "../../muu/impl/unicode_unsigned_char.h"

MUU_NAMESPACE_START
{
	/// \addtogroup strings
	/// @{

	/// \addtogroup code_units
	/// @{

	/// \addtogroup	is_ascii_code_point	is_ascii_code_point()
	/// @{

	/// \brief		Returns true if a character is a valid code point from the ASCII range.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_ascii_code_point(char c) noexcept
	{
		return is_ascii_code_point(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_ascii_hyphen	is_ascii_hyphen()
	/// @{

	/// \brief		Returns true if a character is a hyphen code point from the ASCII range.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_ascii_hyphen(char c) noexcept
	{
		return is_ascii_hyphen(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_ascii_letter	is_ascii_letter()
	/// @{

	/// \brief		Returns true if a character is a letter code point from the ASCII range.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_ascii_letter(char c) noexcept
	{
		return is_ascii_letter(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_ascii_lowercase	is_ascii_lowercase()
	/// @{

	/// \brief		Returns true if a character is a lowercase code point from the ASCII range.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_ascii_lowercase(char c) noexcept
	{
		return is_ascii_lowercase(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_ascii_number	is_ascii_number()
	/// @{

	/// \brief		Returns true if a character is a number code point from the ASCII range.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_ascii_number(char c) noexcept
	{
		return is_ascii_number(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_ascii_uppercase	is_ascii_uppercase()
	/// @{

	/// \brief		Returns true if a character is an uppercase code point from the ASCII range.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_ascii_uppercase(char c) noexcept
	{
		return is_ascii_uppercase(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_ascii_whitespace	is_ascii_whitespace()
	/// @{

	/// \brief		Returns true if a character is a whitespace code point from the ASCII range.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_ascii_whitespace(char c) noexcept
	{
		return is_ascii_whitespace(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_non_ascii_code_point	is_non_ascii_code_point()
	/// @{

	/// \brief		Returns true if a character is a valid code point from outside the ASCII range.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_non_ascii_code_point(char c) noexcept
	{
		(void)c;
		return false;
	}

	/// @}

	/// \addtogroup	is_non_ascii_hyphen	is_non_ascii_hyphen()
	/// @{

	/// \brief		Returns true if a character is a hyphen code point from outside the ASCII range.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_non_ascii_hyphen(char c) noexcept
	{
		(void)c;
		return false;
	}

	/// @}

	/// \addtogroup	is_non_ascii_letter	is_non_ascii_letter()
	/// @{

	/// \brief		Returns true if a character is a letter code point from outside the ASCII range.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_non_ascii_letter(char c) noexcept
	{
		(void)c;
		return false;
	}

	/// @}

	/// \addtogroup	is_non_ascii_lowercase	is_non_ascii_lowercase()
	/// @{

	/// \brief		Returns true if a character is a lowercase code point from outside the ASCII range.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_non_ascii_lowercase(char c) noexcept
	{
		(void)c;
		return false;
	}

	/// @}

	/// \addtogroup	is_non_ascii_number	is_non_ascii_number()
	/// @{

	/// \brief		Returns true if a character is a number code point from outside the ASCII range.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_non_ascii_number(char c) noexcept
	{
		(void)c;
		return false;
	}

	/// @}

	/// \addtogroup	is_non_ascii_uppercase	is_non_ascii_uppercase()
	/// @{

	/// \brief		Returns true if a character is an uppercase code point from outside the ASCII range.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_non_ascii_uppercase(char c) noexcept
	{
		(void)c;
		return false;
	}

	/// @}

	/// \addtogroup	is_non_ascii_whitespace	is_non_ascii_whitespace()
	/// @{

	/// \brief		Returns true if a character is a whitespace code point from outside the ASCII range.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_non_ascii_whitespace(char c) noexcept
	{
		(void)c;
		return false;
	}

	/// @}

	/// \addtogroup	is_not_code_point	is_not_code_point()
	/// @{

	/// \brief		Returns true if a character is not a valid code point.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_not_code_point(char c) noexcept
	{
		return is_not_code_point(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_not_hyphen	is_not_hyphen()
	/// @{

	/// \brief		Returns true if a character is not a hyphen code point.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_not_hyphen(char c) noexcept
	{
		return is_not_hyphen(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_not_letter	is_not_letter()
	/// @{

	/// \brief		Returns true if a character is not a letter code point.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_not_letter(char c) noexcept
	{
		return is_not_letter(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_not_lowercase	is_not_lowercase()
	/// @{

	/// \brief		Returns true if a character is not a lowercase code point.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_not_lowercase(char c) noexcept
	{
		return is_not_lowercase(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_not_number	is_not_number()
	/// @{

	/// \brief		Returns true if a character is not a number code point.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_not_number(char c) noexcept
	{
		return is_not_number(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_not_uppercase	is_not_uppercase()
	/// @{

	/// \brief		Returns true if a character is not an uppercase code point.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_not_uppercase(char c) noexcept
	{
		return is_not_uppercase(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_not_whitespace	is_not_whitespace()
	/// @{

	/// \brief		Returns true if a character is not a whitespace code point.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_not_whitespace(char c) noexcept
	{
		return is_not_whitespace(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_code_point	is_code_point()
	/// @{

	/// \brief		Returns true if a character is a valid code point.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_code_point(char c) noexcept
	{
		return is_code_point(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_code_point_boundary	is_code_point_boundary()
	/// @{

	/// \brief		Returns true if a character is a code point boundary.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_code_point_boundary(char c) noexcept
	{
		return is_code_point_boundary(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_combining_mark	is_combining_mark()
	/// @{

	/// \brief		Returns true if a character is a combining mark code point.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_combining_mark(char c) noexcept
	{
		(void)c;
		return false;
	}

	/// @}

	/// \addtogroup	is_decimal_digit	is_decimal_digit()
	/// @{

	/// \brief		Returns true if a character is a decimal digit code point.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_decimal_digit(char c) noexcept
	{
		return is_decimal_digit(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_hexadecimal_digit	is_hexadecimal_digit()
	/// @{

	/// \brief		Returns true if a character is a hexadecimal digit code point.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_hexadecimal_digit(char c) noexcept
	{
		return is_hexadecimal_digit(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_hyphen	is_hyphen()
	/// @{

	/// \brief		Returns true if a character is a hyphen code point.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_hyphen(char c) noexcept
	{
		return is_hyphen(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_letter	is_letter()
	/// @{

	/// \brief		Returns true if a character is a letter code point.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_letter(char c) noexcept
	{
		return is_letter(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_lowercase	is_lowercase()
	/// @{

	/// \brief		Returns true if a character is a lowercase code point.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_lowercase(char c) noexcept
	{
		return is_lowercase(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_number	is_number()
	/// @{

	/// \brief		Returns true if a character is a number code point.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_number(char c) noexcept
	{
		return is_number(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_octal_digit	is_octal_digit()
	/// @{

	/// \brief		Returns true if a character is an octal digit code point.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_octal_digit(char c) noexcept
	{
		return is_octal_digit(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_uppercase	is_uppercase()
	/// @{

	/// \brief		Returns true if a character is an uppercase code point.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_uppercase(char c) noexcept
	{
		return is_uppercase(static_cast<unsigned char>(c));
	}

	/// @}

	/// \addtogroup	is_whitespace	is_whitespace()
	/// @{

	/// \brief		Returns true if a character is a whitespace code point.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_whitespace(char c) noexcept
	{
		return is_whitespace(static_cast<unsigned char>(c));
	}

	/// @}

	/** @} */	// strings::code_units
	/** @} */	// strings
}
MUU_NAMESPACE_END
