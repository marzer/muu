// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief Functions and types to simplify working with strings.

#pragma once
#include "../muu/core.h"

MUU_PUSH_WARNINGS
MUU_DISABLE_ALL_WARNINGS
#include <string_view>
MUU_POP_WARNINGS

MUU_IMPL_NAMESPACE_START
{
	template <typename T>
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool is_ascii_whitespace(T c) noexcept
	{
		using chars = constants<T>;
		return c == chars::space || (c >= chars::tab && c <= chars::carriage_return);
	}

	template <typename Char, typename Func>
	[[nodiscard]]
	constexpr auto predicated_trim(std::basic_string_view<Char> str, Func&& trim_pred) noexcept
		-> std::basic_string_view<Char>
	{
		using view = std::basic_string_view<Char>;

		if (str.empty())
			return view{};

		auto first = view::npos;
		for (size_t i = 0; i < str.length() && first == view::npos; i++)
			if (!std::forward<Func>(trim_pred)(str[i]))
				first = i;
		if (first == view::npos)
			return view{};

		auto last = view::npos;
		for (size_t i = str.length(); i --> 0 && last == view::npos && i >= first;)
			if (!std::forward<Func>(trim_pred)(str[i]))
				last = i;

		return str.substr(first, (last - first) + 1);
	}

	template <typename Char, typename Func>
	[[nodiscard]]
	constexpr auto predicated_trim_left(std::basic_string_view<Char> str, Func&& trim_pred) noexcept
		-> std::basic_string_view<Char>
	{
		using view = std::basic_string_view<Char>;

		if (str.empty())
			return view{};

		auto first = view::npos;
		for (size_t i = 0; i < str.length() && first == view::npos; i++)
			if (!std::forward<Func>(trim_pred)(str[i]))
				first = i;
		if (first == view::npos)
			return view{};

		return str.substr(first);
	}

	template <typename Char, typename Func>
	[[nodiscard]]
	constexpr auto predicated_trim_right(std::basic_string_view<Char> str, Func&& trim_pred) noexcept
		-> std::basic_string_view<Char>
	{
		using view = std::basic_string_view<Char>;

		if (str.empty())
			return view{};

		auto last = view::npos;
		for (size_t i = str.length(); i --> 0 && last == view::npos;)
			if (!std::forward<Func>(trim_pred)(str[i]))
				last = i;
		if (last == view::npos)
			return view{};

		return str.substr(0, last + 1);
	}

	class MUU_TRIVIAL_ABI utf8_decoder final
	{
		private:
			uint_least32_t state{};
			static constexpr uint8_t state_table[]
			{
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,		9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
				7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
				8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,		2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
				10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3,		11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

				0,12,24,36,60,96,84,12,12,12,48,72,		12,12,12,12,12,12,12,12,12,12,12,12,
				12, 0,12,12,12,12,12, 0,12, 0,12,12,	12,24,12,12,12,12,12,24,12,24,12,12,
				12,12,12,12,12,12,12,24,12,12,12,12,	12,24,12,12,12,12,12,12,12,24,12,12,
				12,12,12,12,12,12,12,36,12,36,12,12,	12,36,12,12,12,12,12,36,12,36,12,12,
				12,36,12,12,12,12,12,12,12,12,12,12
			};
			uint_least32_t codepoint{};

		public:

			[[nodiscard]]
			constexpr bool error() const noexcept
			{
				return state == uint_least32_t{ 12u };
			}

			constexpr void clear_error() noexcept
			{
				MUU_ASSERT(error());
				state = uint_least32_t{};
			}

			[[nodiscard]]
			constexpr bool has_value() const noexcept
			{
				return state == uint_least32_t{};
			}

			[[nodiscard]]
			constexpr char32_t value() const noexcept
			{
				return static_cast<char32_t>(codepoint);
			}

			[[nodiscard]]
			constexpr bool needs_more_input() const noexcept
			{
				return state > uint_least32_t{} && state != uint_least32_t{ 12u };
			}

			constexpr void operator () (uint8_t code_unit) noexcept
			{
				MUU_ASSERT(!error());

				const auto type = state_table[code_unit];

				codepoint = has_value()
						? (uint_least32_t{ 255u } >> type) & code_unit
						: (code_unit & uint_least32_t{ 63u }) | (static_cast<uint_least32_t>(codepoint) << 6);

				state = state_table[state + uint_least32_t{ 256u } + type];
			}
			constexpr void operator () (char code_unit) noexcept
			{
				(*this)(static_cast<uint8_t>(code_unit));
			}
			constexpr void operator () (std::byte code_unit) noexcept
			{
				(*this)(unwrap(code_unit));
			}
			#ifdef __cpp_char8_t
			constexpr void operator () (char8_t code_unit) noexcept
			{
				(*this)(static_cast<uint8_t>(code_unit));
			}
			#endif
	};

	class MUU_TRIVIAL_ABI utf16_decoder final
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

			[[nodiscard]]
			constexpr bool error() const noexcept
			{
				return state == ds_error;
			}

			constexpr void clear_error() noexcept
			{
				MUU_ASSERT(error());
				state = ds_initial;
			}

			[[nodiscard]]
			constexpr bool has_value() const noexcept
			{
				return state == ds_has_codepoint;
			}

			[[nodiscard]]
			constexpr char32_t value() const noexcept
			{
				return static_cast<char32_t>(codepoint);
			}

			[[nodiscard]]
			constexpr bool needs_more_input() const noexcept
			{
				return state == ds_expecting_low_surrogate;
			}

			constexpr void operator() (uint16_t code_unit) noexcept
			{
				MUU_ASSERT(!error());
	
				enum code_unit_classes : unsigned
				{
					basic_multilingual,
					low_surrogate  = 0b00010000u,
					high_surrogate = 0b00100000u
				};

				const auto code_unit_class =
					(static_cast<unsigned>(is_between(code_unit, 0xDC00_u16, 0xDFFF_u16)) << 4)
					| (static_cast<unsigned>(is_between(code_unit, 0xD800_u16, 0xDBFF_u16)) << 5);

				switch (code_unit_class | state)
				{
					case unwrap(basic_multilingual) | ds_initial: [[fallthrough]];
					case unwrap(basic_multilingual) | ds_has_codepoint:
						codepoint = static_cast<uint_least32_t>(code_unit);
						state = ds_has_codepoint;
						return;

					case unwrap(high_surrogate) | ds_initial: [[fallthrough]];
					case unwrap(high_surrogate) | ds_has_codepoint:
						codepoint = static_cast<uint_least32_t>(code_unit); //will get multiplexed in the next step
						state = ds_has_codepoint;
						return;

					case unwrap(low_surrogate) | ds_expecting_low_surrogate:
						codepoint = ((codepoint - 0xD800u) << 10) + (static_cast<uint_least32_t>(code_unit) - 0xDC00u) + 0x10000u;
						state = ds_has_codepoint;
						return;

					default:
						state = ds_error;
				}		
			}
			constexpr void operator () (char16_t code_unit) noexcept
			{
				(*this)(static_cast<uint16_t>(code_unit));
			}
			#if MUU_SIZEOF_WCHAR_T == 2
			constexpr void operator () (wchar_t code_unit) noexcept
			{
				(*this)(static_cast<uint16_t>(code_unit));
			}
			#endif
	};

	template <typename T>
	using utf_decoder = std::conditional_t<sizeof(T) == 2, utf16_decoder,
		std::conditional_t<sizeof(T) == 1, utf8_decoder,
		void
	>>;

	template <typename T, typename Func>
	constexpr void utf_decode(std::basic_string_view<T> sv, Func&& func) noexcept
	{
		if (sv.empty())
			return;

		if constexpr (sizeof(T) > 4_sz)
		{
			static_assert(dependent_false<T>, "unknown character type");
		}
		else if constexpr (sizeof(T) == 4_sz)
		{
			// todo: bom handling
			for (auto c : sv)
				func(static_cast<char32_t>(c));
		}
		else
		{
			// todo: bom handling
			utf_decoder<T> decoder;
			for (auto c : sv)
			{
				decoder(c);
				if (decoder.has_value())
					func(decoder.value());
				else if (decoder.error())
				{
					func(static_cast<char32_t>(c));
					decoder.clear_error();
				}
			}
		}
	}

	template <typename T = char>
	class MUU_TRIVIAL_ABI utf8_code_point
	{
		private:
			static_assert(sizeof(T) == 1);
			array<T, 4> bytes{};

		public:

			MUU_NODISCARD_CTOR
			constexpr utf8_code_point(uint_least32_t cp) noexcept
			{
				if (cp <= 0x7Fu)
				{
					bytes[0] = static_cast<T>(cp);
				}
				else if (cp <= 0x7FFu)
				{
					bytes[0] = static_cast<T>((cp >> 6) | 0xC0u);
					bytes[1] = static_cast<T>((cp & 0x3Fu) | 0x80u);
				}
				else if (cp <= 0xFFFFu)
				{
					bytes[0] = static_cast<T>((cp >> 12) | 0xE0u);
					bytes[1] = static_cast<T>(((cp >> 6) & 0x3Fu) | 0x80u);
					bytes[2] = static_cast<T>((cp & 0x3Fu) | 0x80u);
				}
				else if (cp <= 0x10FFFFu)
				{
					bytes[0] = static_cast<T>((cp >> 18) | 0xF0u);
					bytes[1] = static_cast<T>(((cp >> 12) & 0x3Fu) | 0x80u);
					bytes[2] = static_cast<T>(((cp >> 6) & 0x3Fu) | 0x80u);
					bytes[3] = static_cast<T>((cp & 0x3Fu) | 0x80u);
				}
			}

			MUU_NODISCARD_CTOR
			constexpr utf8_code_point(char32_t cp) noexcept
				: utf8_code_point{ static_cast<uint_least32_t>(cp) }
			{}

			[[nodiscard]]
			constexpr std::basic_string_view<T> view() const noexcept
			{
				return bytes[3]
					? std::basic_string_view<T>{ bytes.data(), 4_sz }
					: std::basic_string_view<T>{ bytes.data() };
			}

			[[nodiscard]]
			constexpr operator std::basic_string_view<T>() const noexcept
			{
				return view();
			}
	};
}
MUU_IMPL_NAMESPACE_END

MUU_NAMESPACE_START
{
	[[nodiscard]]
	constexpr std::string_view trim(std::string_view str) noexcept
	{
		return impl::predicated_trim(str, impl::is_ascii_whitespace<char>);
	}

	[[nodiscard]]
	constexpr std::wstring_view trim(std::wstring_view str) noexcept
	{
		return impl::predicated_trim(str, impl::is_ascii_whitespace<wchar_t>);
	}

	[[nodiscard]]
	constexpr std::u16string_view trim(std::u16string_view str) noexcept
	{
		return impl::predicated_trim(str, impl::is_ascii_whitespace<char16_t>);
	}

	[[nodiscard]]
	constexpr std::u32string_view trim(std::u32string_view str) noexcept
	{
		return impl::predicated_trim(str, impl::is_ascii_whitespace<char32_t>);
	}
}
MUU_NAMESPACE_END
