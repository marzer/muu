// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Functions to simplify working with strings.

#include "bit.h"
#include "chars.h"
#include "meta.h"
#include "utf_decode.h"
#include "impl/core_constants.h"
#include "impl/std_string.h"
#include "impl/std_string_view.h"
#include "impl/std_iosfwd.h"
#include "impl/std_memcpy.h"
#include "impl/header_start.h"
MUU_DISABLE_ARITHMETIC_WARNINGS;
MUU_FORCE_NDEBUG_OPTIMIZATIONS;

namespace muu
{
	/// \addtogroup		strings			Strings
	/// \brief			Utilities to simplify working with strings.
	/// @{

	/// \defgroup		code_units		Code units
	/// \brief			Utilities for manipulating individual code units ('characters').

#if 1 // unicode/boilerplate -------------------------------------------------------------------------------------------

	/// \cond
	namespace impl
	{
		struct utf_find_result
		{
			size_t index;
			size_t length;

			MUU_PURE_GETTER
			explicit constexpr operator bool() const noexcept
			{
				return index != constants<size_t>::highest;
			}

			MUU_PURE_GETTER
			constexpr size_t end() const noexcept
			{
				return index + length;
			}
		};

		template <typename T, typename Func>
		constexpr utf_find_result utf_find(std::basic_string_view<T> str, bool reverse, Func&& predicate) noexcept
		{
			utf_find_result result{ constants<size_t>::highest, {} };
			if (!str.empty())
			{
				utf_decode(str,
						   reverse,
						   [&](char32_t cp, size_t starts_at, size_t goes_for) noexcept
						   {
							   if (static_cast<Func&&>(predicate)(cp))
							   {
								   result = { starts_at, goes_for };
								   return false;
							   }
							   return true;
						   });
			}
			return result;
		}

		template <typename T>
		class MUU_TRIVIAL_ABI utf8_code_point
		{
		  private:
			static_assert(sizeof(T) == 1);
			T code_units[4] = {};

		  public:
			MUU_NODISCARD_CTOR
			constexpr utf8_code_point(uint_least32_t cp) noexcept // *** assumes cp is platform-endian ***
			{
				if (cp <= 0x7Fu)
				{
					code_units[0] = static_cast<T>(cp);
				}
				else if (cp <= 0x7FFu)
				{
					code_units[0] = static_cast<T>((cp >> 6) | 0xC0u);
					code_units[1] = static_cast<T>((cp & 0x3Fu) | 0x80u);
				}
				else if (cp <= 0xFFFFu)
				{
					code_units[0] = static_cast<T>((cp >> 12) | 0xE0u);
					code_units[1] = static_cast<T>(((cp >> 6) & 0x3Fu) | 0x80u);
					code_units[2] = static_cast<T>((cp & 0x3Fu) | 0x80u);
				}
				else if (cp <= 0x10FFFFu)
				{
					code_units[0] = static_cast<T>((cp >> 18) | 0xF0u);
					code_units[1] = static_cast<T>(((cp >> 12) & 0x3Fu) | 0x80u);
					code_units[2] = static_cast<T>(((cp >> 6) & 0x3Fu) | 0x80u);
					code_units[3] = static_cast<T>((cp & 0x3Fu) | 0x80u);
				}
			}

			MUU_NODISCARD_CTOR
			constexpr utf8_code_point(char32_t cp) noexcept // *** assumes cp is platform-endian ***
				: utf8_code_point{ static_cast<uint_least32_t>(cp) }
			{}

			MUU_PURE_INLINE_GETTER
			constexpr std::basic_string_view<T> view() const noexcept
			{
				return code_units[3] ? std::basic_string_view<T>{ code_units, 4_sz }
									 : std::basic_string_view<T>{ code_units };
			}

			MUU_PURE_INLINE_GETTER
			constexpr operator std::basic_string_view<T>() const noexcept
			{
				return view();
			}
		};

		template <typename T>
		class MUU_TRIVIAL_ABI utf16_code_point
		{
		  private:
			static_assert(sizeof(T) == 2);
			T code_units[2] = {};

		  public:
			MUU_NODISCARD_CTOR
			constexpr utf16_code_point(uint_least32_t cp) noexcept // *** assumes cp is platform-endian ***
			{
				if (cp < 0x10000u)
				{
					code_units[0] = static_cast<T>(cp);
				}
				else
				{
					cp -= 0x10000u;
					code_units[0] = static_cast<T>(static_cast<uint16_t>(0xD800u | (cp >> 10)));	// high
					code_units[1] = static_cast<T>(static_cast<uint16_t>(0xDC00u | (cp & 0x3FFu))); // low
				}
			}

			MUU_NODISCARD_CTOR
			constexpr utf16_code_point(char32_t cp) noexcept // *** assumes cp is platform-endian ***
				: utf16_code_point{ static_cast<uint_least32_t>(cp) }
			{}

			MUU_PURE_INLINE_GETTER
			constexpr std::basic_string_view<T> view() const noexcept
			{
				return code_units[1] ? std::basic_string_view<T>{ code_units, 2_sz }
									 : std::basic_string_view<T>{ code_units };
			}

			MUU_PURE_INLINE_GETTER
			constexpr operator std::basic_string_view<T>() const noexcept
			{
				return view();
			}
		};

		template <typename T, size_t S = sizeof(T)>
		struct utf_code_point_selector;
		template <typename T>
		struct utf_code_point_selector<T, 1>
		{
			using type = utf8_code_point<T>;
		};
		template <typename T>
		struct utf_code_point_selector<T, 2>
		{
			using type = utf16_code_point<T>;
		};
		template <typename T>
		struct utf_code_point_selector<T, 4>
		{
			using type = char32_t;
		};

		template <typename T>
		using utf_code_point = typename utf_code_point_selector<T>::type;
	}
	/// \endcond

#endif // unicode/boilerplate

#if 1 // trim -----------------------------------------------------------------------------------------------------
	/// \addtogroup 	trim	trim()
	/// \brief Trims whitespace from both ends of a UTF string.
	/// @{

	/// \cond
	namespace impl
	{
		template <typename T, typename Func>
		MUU_NODISCARD
		constexpr auto predicated_trim(std::basic_string_view<T> str, Func&& predicate) noexcept
			-> std::basic_string_view<T>
		{
			using view = std::basic_string_view<T>;

			if (str.empty())
				return view{};

			const auto first = utf_find(str, false, static_cast<Func&&>(predicate));
			if (!first)
				return view{};

			const auto last = utf_find(str, true, static_cast<Func&&>(predicate));

			return str.substr(first.index, last.end() - first.index);
		}
	}
	/// \endcond

	/// \brief		Trims whitespace from both ends of a UTF-8 string.
	MUU_PURE_GETTER
	constexpr std::string_view trim(std::string_view str) noexcept
	{
		return impl::predicated_trim(str, static_cast<bool (*)(char32_t) noexcept>(is_not_whitespace));
	}

	/// \brief		Trims whitespace from both ends of a UTF wide string.
	MUU_PURE_GETTER
	constexpr std::wstring_view trim(std::wstring_view str) noexcept
	{
		return impl::predicated_trim(str, static_cast<bool (*)(char32_t) noexcept>(is_not_whitespace));
	}

	/// \brief		Trims whitespace from both ends of a UTF-16 string.
	MUU_PURE_GETTER
	constexpr std::u16string_view trim(std::u16string_view str) noexcept
	{
		return impl::predicated_trim(str, static_cast<bool (*)(char32_t) noexcept>(is_not_whitespace));
	}

	/// \brief		Trims whitespace from both ends of a UTF-32 string.
	MUU_PURE_GETTER
	constexpr std::u32string_view trim(std::u32string_view str) noexcept
	{
		return impl::predicated_trim(str, static_cast<bool (*)(char32_t) noexcept>(is_not_whitespace));
	}

	#if MUU_HAS_CHAR8_STRINGS

	/// \brief		Trims whitespace from both ends of a UTF-8 string.
	MUU_PURE_GETTER
	constexpr std::u8string_view trim(std::u8string_view str) noexcept
	{
		return impl::predicated_trim(str, static_cast<bool (*)(char32_t) noexcept>(is_not_whitespace));
	}

	#endif // MUU_HAS_CHAR8_STRINGS

	/** @} */ // strings::trim
#endif		  // trim

#if 1 // trim_left -----------------------------------------------------------------------------------------------------
	/// \addtogroup 	trim_left	trim_left()
	/// \brief Trims whitespace from the left end of a UTF string.
	/// @{

	/// \cond
	namespace impl
	{
		template <typename T, typename Func>
		MUU_NODISCARD
		constexpr auto predicated_trim_left(std::basic_string_view<T> str, Func&& predicate) noexcept
			-> std::basic_string_view<T>
		{
			using view = std::basic_string_view<T>;

			if (str.empty())
				return view{};

			const auto first = utf_find(str, false, static_cast<Func&&>(predicate));
			if (!first)
				return view{};

			return str.substr(first.index);
		}
	}
	/// \endcond

	/// \brief		Trims whitespace from the left end of a UTF-8 string.
	MUU_PURE_GETTER
	constexpr std::string_view trim_left(std::string_view str) noexcept
	{
		return impl::predicated_trim_left(str, static_cast<bool (*)(char32_t) noexcept>(is_not_whitespace));
	}

	/// \brief		Trims whitespace from the left end of a UTF wide string.
	MUU_PURE_GETTER
	constexpr std::wstring_view trim_left(std::wstring_view str) noexcept
	{
		return impl::predicated_trim_left(str, static_cast<bool (*)(char32_t) noexcept>(is_not_whitespace));
	}

	/// \brief		Trims whitespace from the left end of a UTF-16 string.
	MUU_PURE_GETTER
	constexpr std::u16string_view trim_left(std::u16string_view str) noexcept
	{
		return impl::predicated_trim_left(str, static_cast<bool (*)(char32_t) noexcept>(is_not_whitespace));
	}

	/// \brief		Trims whitespace from the left end of a UTF-32 string.
	MUU_PURE_GETTER
	constexpr std::u32string_view trim_left(std::u32string_view str) noexcept
	{
		return impl::predicated_trim_left(str, static_cast<bool (*)(char32_t) noexcept>(is_not_whitespace));
	}

	#if MUU_HAS_CHAR8_STRINGS

	/// \brief		Trims whitespace from the left end of a UTF-8 string.
	MUU_PURE_GETTER
	constexpr std::u8string_view trim_left(std::u8string_view str) noexcept
	{
		return impl::predicated_trim_left(str, static_cast<bool (*)(char32_t) noexcept>(is_not_whitespace));
	}

	#endif // MUU_HAS_CHAR8_STRINGS

	/** @} */ // strings::trim_left
#endif		  // trim_left

#if 1 // trim_right ----------------------------------------------------------------------------------------------------
	/// \addtogroup 	trim_right	trim_right()
	/// \brief Trims whitespace from the right end of a UTF string.
	/// @{

	/// \cond
	namespace impl
	{
		template <typename T, typename Func>
		MUU_NODISCARD
		constexpr auto predicated_trim_right(std::basic_string_view<T> str, Func&& predicate) noexcept
			-> std::basic_string_view<T>
		{
			using view = std::basic_string_view<T>;

			if (str.empty())
				return view{};

			const auto last = utf_find(str, true, static_cast<Func&&>(predicate));
			if (!last)
				return view{};

			return str.substr(0, last.end());
		}
	}
	/// \endcond

	/// \brief		Trims whitespace from the right end of a UTF-8 string.
	MUU_PURE_GETTER
	constexpr std::string_view trim_right(std::string_view str) noexcept
	{
		return impl::predicated_trim_right(str, static_cast<bool (*)(char32_t) noexcept>(is_not_whitespace));
	}

	/// \brief		Trims whitespace from the right end of a UTF wide string.
	MUU_PURE_GETTER
	constexpr std::wstring_view trim_right(std::wstring_view str) noexcept
	{
		return impl::predicated_trim_right(str, static_cast<bool (*)(char32_t) noexcept>(is_not_whitespace));
	}

	/// \brief		Trims whitespace from the right end of a UTF-16 string.
	MUU_PURE_GETTER
	constexpr std::u16string_view trim_right(std::u16string_view str) noexcept
	{
		return impl::predicated_trim_right(str, static_cast<bool (*)(char32_t) noexcept>(is_not_whitespace));
	}

	/// \brief		Trims whitespace from the right end of a UTF-32 string.
	MUU_PURE_GETTER
	constexpr std::u32string_view trim_right(std::u32string_view str) noexcept
	{
		return impl::predicated_trim_right(str, static_cast<bool (*)(char32_t) noexcept>(is_not_whitespace));
	}

	#if MUU_HAS_CHAR8_STRINGS

	/// \brief		Trims whitespace from the right end of a UTF-8 string.
	MUU_PURE_GETTER
	constexpr std::u8string_view trim_right(std::u8string_view str) noexcept
	{
		return impl::predicated_trim_right(str, static_cast<bool (*)(char32_t) noexcept>(is_not_whitespace));
	}

	#endif // MUU_HAS_CHAR8_STRINGS

	/** @} */ // strings::trim_right
#endif		  // trim_right

#if 1 // transcode -----------------------------------------------------------------------------------------------------
	/// \addtogroup 	transcode	transcode()
	/// \brief Transcodes a UTF string into another UTF encoding.
	/// @{

	/// \cond
	namespace impl
	{
		template <typename To, typename From>
		MUU_NODISCARD
		MUU_CONSTEXPR_STD_STRING std::basic_string<To> utf_transcode(std::basic_string_view<From> str) noexcept
		{
			if (str.empty())
				return {};

			static_assert(is_code_unit<From> && is_code_unit<To>, "To and From must both be valid code unit types");

			if constexpr (std::is_same_v<To, From>)
				return std::basic_string<To>(str);
			else if constexpr (sizeof(To) == sizeof(From))
			{
				std::basic_string<To> out;
				out.resize(str.length());
				MUU_MEMCPY(out.data(), str.data(), str.length() * sizeof(From));
				return out;
			}
			else
			{
				std::basic_string<To> out;
				if constexpr (sizeof(To) < sizeof(From))
					out.reserve((str.length() / 2u) * 3u);
				else
					out.reserve(str.length());

				utf_decode(str,
						   [&](char32_t cp) noexcept
						   {
							   if constexpr (std::is_same_v<char32_t, To>)
								   out += cp;
							   else if constexpr (sizeof(To) == sizeof(char32_t)) // e.g. wchar_t on linux
								   out += static_cast<To>(cp);
							   else
							   {
								   impl::utf_code_point<To> enc{ cp };
								   out.append(enc.view());
							   }
						   });

				return out;
			}
		}

	}
	/// \endcond

	/// \brief		Transcodes a UTF-8 string into another UTF encoding.
	template <typename Char>
	MUU_NODISCARD
	MUU_CONSTEXPR_STD_STRING auto transcode(std::string_view str) noexcept
		-> std::conditional_t<std::is_same_v<Char, char>, std::string_view, std::basic_string<Char>>
	{
		if constexpr (std::is_same_v<Char, char>)
			return str;
		else
			return impl::utf_transcode<Char>(str);
	}

	/// \brief		Transcodes a UTF wide string into another UTF encoding.
	template <typename Char>
	MUU_NODISCARD
	MUU_CONSTEXPR_STD_STRING auto transcode(std::wstring_view str) noexcept
		-> std::conditional_t<std::is_same_v<Char, wchar_t>, std::wstring_view, std::basic_string<Char>>
	{
		if constexpr (std::is_same_v<Char, wchar_t>)
			return str;
		else
			return impl::utf_transcode<Char>(str);
	}

	/// \brief		Transcodes a UTF-16 string into another UTF encoding.
	template <typename Char>
	MUU_NODISCARD
	MUU_CONSTEXPR_STD_STRING auto transcode(std::u16string_view str) noexcept
		-> std::conditional_t<std::is_same_v<Char, char16_t>, std::u16string_view, std::basic_string<Char>>
	{
		if constexpr (std::is_same_v<Char, char16_t>)
			return str;
		else
			return impl::utf_transcode<Char>(str);
	}

	/// \brief		Transcodes a UTF-32 string into another UTF encoding.
	template <typename Char>
	MUU_NODISCARD
	MUU_CONSTEXPR_STD_STRING auto transcode(std::u32string_view str) noexcept
		-> std::conditional_t<std::is_same_v<Char, char32_t>, std::u32string_view, std::basic_string<Char>>
	{
		if constexpr (std::is_same_v<Char, char32_t>)
			return str;
		else
			return impl::utf_transcode<Char>(str);
	}

	#if MUU_HAS_CHAR8_STRINGS

	/// \brief		Transcodes a UTF-8 string into another UTF encoding.
	template <typename Char>
	MUU_NODISCARD
	MUU_CONSTEXPR_STD_STRING auto transcode(std::u8string_view str) noexcept
		-> std::conditional_t<std::is_same_v<Char, char8_t>, std::u8string_view, std::basic_string<Char>>
	{
		if constexpr (std::is_same_v<Char, char8_t>)
			return str;
		else
			return impl::utf_transcode<Char>(str);
	}

	#endif // MUU_HAS_CHAR8_STRINGS

	/** @} */ // strings::transcode
#endif		  // transcode

	/** @} */ // strings
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
