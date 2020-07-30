// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief Functions and types for working with strings.

#pragma once
#include "../muu/core.h"
#include "../muu/impl/unicode_char.h"
#include "../muu/impl/unicode_wchar_t.h"
#include "../muu/impl/unicode_char16_t.h"
#include "../muu/impl/unicode_char32_t.h"
#ifdef __cpp_char8_t
#include "../muu/impl/unicode_char8_t.h"
#endif

MUU_PUSH_WARNINGS
MUU_DISABLE_ALL_WARNINGS
#include <string_view>
MUU_POP_WARNINGS

//=====================================================================================================================
// UNICODE
//=====================================================================================================================

MUU_IMPL_NAMESPACE_START
{
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
			#if MUU_WCHAR_BYTES == 2
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

	template <typename T, typename Func, size_t PositionalArgs = (
		std::is_nothrow_invocable_v<Func, T, size_t, size_t> ? 2 :
		(std::is_nothrow_invocable_v<Func, T, size_t> ? 1 : 0)
	)>
	struct utf_decode_func_traits
	{
		using return_type = decltype(std::declval<Func>()(
			std::declval<T>(),
			std::declval<size_t>(),
			std::declval<size_t>()
		));
	};

	template <typename T, typename Func>
	struct utf_decode_func_traits<T, Func, 1>
	{
		using return_type = decltype(std::declval<Func>()(
			std::declval<T>(),
			std::declval<size_t>()
		));
	};

	template <typename T, typename Func>
	struct utf_decode_func_traits<T, Func, 0>
	{
		using return_type = decltype(std::declval<Func>()(
			std::declval<T>()
		));
	};

	template <typename T, typename Func>
	constexpr void utf_decode(std::basic_string_view<T> str, bool reverse, Func&& func) noexcept
	{
		if (str.empty())
			return;

		static_assert(
			is_code_unit<T>,
			"unknown code unit type"
		);
		static_assert(
			   std::is_nothrow_invocable_v<Func&&, T, size_t, size_t>
			|| std::is_nothrow_invocable_v<Func&&, T, size_t>
			|| std::is_nothrow_invocable_v<Func&&, T>,
			"decoder func must be nothrow-invocable with (T), (T, size_t) or (T, size_t, size_t)"
		);

		using func_return_type = typename utf_decode_func_traits<T, Func>::return_type;
		constexpr auto stop_after_invoking = [](auto&& f, char32_t cp, size_t cp_start, size_t cu_count) noexcept -> bool
		{
			(void)cp_start;
			(void)cu_count;
			if constexpr (std::is_convertible_v<func_return_type, bool>)
			{
				if constexpr (std::is_nothrow_invocable_v<Func&&, T, size_t, size_t>)
					return !static_cast<bool>(std::forward<decltype(f)>(f)(cp, cp_start, cu_count));
				else if constexpr (std::is_nothrow_invocable_v<Func&&, T, size_t>)
					return !static_cast<bool>(std::forward<decltype(f)>(f)(cp, cp_start));
				else
					return !static_cast<bool>(std::forward<decltype(f)>(f)(cp));
			}
			else
			{
				if constexpr (std::is_nothrow_invocable_v<Func&&, T, size_t, size_t>)
					std::forward<decltype(f)>(f)(cp, cp_start, cu_count);
				else if constexpr (std::is_nothrow_invocable_v<Func&&, T, size_t>)
					std::forward<decltype(f)>(f)(cp, cp_start);
				else
					std::forward<decltype(f)>(f)(cp);
				return false;
			}
		};
		
		bool stop = false;
		if constexpr (sizeof(T) == 4)
		{
			// todo: bom handling
			if (reverse)
			{
				for (size_t i = str.length(); i --> 0u && !stop;)
					stop = stop_after_invoking(std::forward<Func>(func), static_cast<char32_t>(str[i]), i, 1);
			}
			else
			{
				for (size_t i = 0, e = str.length(); i < e && !stop; i++)
					stop = stop_after_invoking(std::forward<Func>(func), static_cast<char32_t>(str[i]), i, 1);
			}
		}
		else
		{
			// todo: bom handling
			utf_decoder<T> decoder;
			if (reverse)
			{
				size_t cp_start = str.length();
				size_t cu_count = {};
				constexpr size_t max_cu_count = 4_sz / sizeof(T);
				while (cp_start--> 0u && !stop)
				{
					using muu::is_code_point_boundary;

					cu_count++;
					if (cu_count == max_cu_count || is_code_point_boundary(str[cp_start]))
					{
						for (size_t i = cp_start, e = cp_start + cu_count; i < e; i++)
						{
							decoder(str[i]);
							if (decoder.error())
								break;
						}

						if (decoder.has_value())
							stop = stop_after_invoking(std::forward<Func>(func), decoder.value(), cp_start, cu_count);
						else if (decoder.error())
						{
							decoder.clear_error();
							for (size_t i = cp_start + cu_count; i --> cp_start && !stop;)
								stop = stop_after_invoking(std::forward<Func>(func), static_cast<char32_t>(str[i]), i, 1);
						}
						cu_count = {};
					}
				}
			}
			else
			{
				for (size_t i = 0, e = str.length(), cp_start = 0, cu_count = 0; i < e && !stop; i++)
				{
					cu_count++;
					decoder(str[i]);
					if (decoder.has_value())
					{
						stop = stop_after_invoking(std::forward<Func>(func), decoder.value(), cp_start, cu_count);
						cp_start = i + 1u;
						cu_count = {};
					}
					else if (decoder.error())
					{
						decoder.clear_error();
						for (size_t j = cp_start, je = cp_start + cu_count; j < je && !stop; j++)
							stop = stop_after_invoking(std::forward<Func>(func), static_cast<char32_t>(str[j]), j, 1);
						cp_start = i + 1u;
						cu_count = {};
					}
				}
			}
		}
	}

	template <typename T, typename Func>
	constexpr void utf_decode(std::basic_string_view<T> str, Func&& func) noexcept
	{
		utf_decode(str, false, std::forward<Func>(func));
	}

	struct utf_find_result
	{
		size_t index;
		size_t length;

		[[nodiscard]]
		MUU_ATTR(pure)
			explicit constexpr operator bool() const noexcept
		{
			return index != impl::numeric_limits<size_t>::highest;
		}

		[[nodiscard]]
		MUU_ATTR(pure)
			constexpr size_t end() const noexcept
		{
			return index + length;
		}
	};

	template <typename T, typename Func>
	constexpr utf_find_result utf_find(std::basic_string_view<T> str, bool reverse, Func&& predicate) noexcept
	{
		utf_find_result result{ numeric_limits<size_t>::highest, {} };
		if (!str.empty())
		{
			utf_decode(str, reverse, [&](char32_t cp, size_t starts_at, size_t goes_for) noexcept
			{
				if (std::forward<Func>(predicate)(cp))
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

//=====================================================================================================================
// TRIM
//=====================================================================================================================

MUU_NAMESPACE_START
{
	namespace impl
	{
		template <typename T, typename Func>
		[[nodiscard]]
		constexpr auto predicated_trim(std::basic_string_view<T> str, Func&& predicate) noexcept
			-> std::basic_string_view<T>
		{
			using view = std::basic_string_view<T>;

			if (str.empty())
				return view{};

			auto first = utf_find(str, false, std::forward<Func>(predicate));
			if (!first)
				return view{};

			auto last = utf_find(str, true, std::forward<Func>(predicate));

			return str.substr(first.index, last.end() - first.index);
		}

		template <typename T, typename Func>
		[[nodiscard]]
		constexpr auto predicated_trim_left(std::basic_string_view<T> str, Func&& predicate) noexcept
			-> std::basic_string_view<T>
		{
			using view = std::basic_string_view<T>;

			if (str.empty())
				return view{};

			auto first = utf_find(str, false, std::forward<Func>(predicate));
			if (!first)
				return view{};

			return str.substr(first.index);
		}

		template <typename T, typename Func>
		[[nodiscard]]
		constexpr auto predicated_trim_right(std::basic_string_view<T> str, Func&& predicate) noexcept
			-> std::basic_string_view<T>
		{
			using view = std::basic_string_view<T>;

			if (str.empty())
				return view{};

			auto last = utf_find(str, true, std::forward<Func>(predicate));
			if (!last)
				return view{};

			return str.substr(0, last.end());
		}
	}

	/// \addtogroup strings
	/// @{

	/// \brief		Trims whitespace from both ends of a UTF-8 string.
	[[nodiscard]]
	MUU_ATTR(pure)
	constexpr std::string_view trim(std::string_view str) noexcept
	{
		return impl::predicated_trim(str, static_cast<bool(*)(char32_t)>(is_not_whitespace));
	}

	/// \brief		Trims whitespace from both ends of a wide string.
	[[nodiscard]]
	MUU_ATTR(pure)
	constexpr std::wstring_view trim(std::wstring_view str) noexcept
	{
		return impl::predicated_trim(str, static_cast<bool(*)(char32_t)>(is_not_whitespace));
	}

	/// \brief		Trims whitespace from both ends of a UTF-16 string.
	[[nodiscard]]
	MUU_ATTR(pure)
	constexpr std::u16string_view trim(std::u16string_view str) noexcept
	{
		return impl::predicated_trim(str, static_cast<bool(*)(char32_t)>(is_not_whitespace));
	}

	/// \brief		Trims whitespace from both ends of a UTF-32 string.
	[[nodiscard]]
	MUU_ATTR(pure)
	constexpr std::u32string_view trim(std::u32string_view str) noexcept
	{
		return impl::predicated_trim(str, static_cast<bool(*)(char32_t)>(is_not_whitespace));
	}

	#ifdef __cpp_lib_char8_t
	/// \brief		Trims whitespace from both ends of a UTF-8 string.
	[[nodiscard]]
	MUU_ATTR(pure)
	constexpr std::u8string_view trim(std::u8string_view str) noexcept
	{
		return impl::predicated_trim(str, static_cast<bool(*)(char32_t)>(is_not_whitespace));
	}
	#endif

	/// \brief		Trims whitespace from the left end of a UTF-8 string.
	[[nodiscard]]
	MUU_ATTR(pure)
	constexpr std::string_view trim_left(std::string_view str) noexcept
	{
		return impl::predicated_trim_left(str, static_cast<bool(*)(char32_t)>(is_not_whitespace));
	}

	/// \brief		Trims whitespace from the left end of a wide string.
	[[nodiscard]]
	MUU_ATTR(pure)
	constexpr std::wstring_view trim_left(std::wstring_view str) noexcept
	{
		return impl::predicated_trim_left(str, static_cast<bool(*)(char32_t)>(is_not_whitespace));
	}

	/// \brief		Trims whitespace from the left end of a UTF-16 string.
	[[nodiscard]]
	MUU_ATTR(pure)
	constexpr std::u16string_view trim_left(std::u16string_view str) noexcept
	{
		return impl::predicated_trim_left(str, static_cast<bool(*)(char32_t)>(is_not_whitespace));
	}

	/// \brief		Trims whitespace from the left end of a UTF-32 string.
	[[nodiscard]]
	MUU_ATTR(pure)
	constexpr std::u32string_view trim_left(std::u32string_view str) noexcept
	{
		return impl::predicated_trim_left(str, static_cast<bool(*)(char32_t)>(is_not_whitespace));
	}

	#ifdef __cpp_lib_char8_t
	/// \brief		Trims whitespace from the left end of a UTF-8 string.
	[[nodiscard]]
	MUU_ATTR(pure)
	constexpr std::u8string_view trim_left(std::u8string_view str) noexcept
	{
		return impl::predicated_trim_left(str, static_cast<bool(*)(char32_t)>(is_not_whitespace));
	}
	#endif

	/// \brief		Trims whitespace from the right end of a UTF-8 string.
	[[nodiscard]]
	MUU_ATTR(pure)
	constexpr std::string_view trim_right(std::string_view str) noexcept
	{
		return impl::predicated_trim_right(str, static_cast<bool(*)(char32_t)>(is_not_whitespace));
	}

	/// \brief		Trims whitespace from the right end of a wide string.
	[[nodiscard]]
	MUU_ATTR(pure)
	constexpr std::wstring_view trim_right(std::wstring_view str) noexcept
	{
		return impl::predicated_trim_right(str, static_cast<bool(*)(char32_t)>(is_not_whitespace));
	}

	/// \brief		Trims whitespace from the right end of a UTF-16 string.
	[[nodiscard]]
	MUU_ATTR(pure)
	constexpr std::u16string_view trim_right(std::u16string_view str) noexcept
	{
		return impl::predicated_trim_right(str, static_cast<bool(*)(char32_t)>(is_not_whitespace));
	}

	/// \brief		Trims whitespace from the right end of a UTF-32 string.
	[[nodiscard]]
	MUU_ATTR(pure)
	constexpr std::u32string_view trim_right(std::u32string_view str) noexcept
	{
		return impl::predicated_trim_right(str, static_cast<bool(*)(char32_t)>(is_not_whitespace));
	}

	#ifdef __cpp_lib_char8_t
	/// \brief		Trims whitespace from the right end of a UTF-8 string.
	[[nodiscard]]
	MUU_ATTR(pure)
	constexpr std::u8string_view trim_right(std::u8string_view str) noexcept
	{
		return impl::predicated_trim_right(str, static_cast<bool(*)(char32_t)>(is_not_whitespace));
	}
	#endif

	/// @}
}
MUU_NAMESPACE_END
