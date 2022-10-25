// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the implementation of #muu::utf_decode.

#include "meta.h"
#include "bit.h"
#include "chars.h"
#include "utf8_decoder.h"
#include "utf16_decoder.h"
#include "impl/std_string_view.h"
#include "impl/header_start.h"
MUU_DISABLE_ARITHMETIC_WARNINGS;
MUU_FORCE_NDEBUG_OPTIMIZATIONS;

/// \cond
namespace muu::impl
{
	template <typename T>
	using utf_decoder =
		std::conditional_t<sizeof(T) == 2, utf16_decoder, std::conditional_t<sizeof(T) == 1, utf8_decoder, void>>;

	template <typename T,
			  typename Func,
			  size_t PositionalArgs = (std::is_nothrow_invocable_v<Func, T, size_t, size_t>
										   ? 2u
										   : (std::is_nothrow_invocable_v<Func, T, size_t> ? 1u : 0u))>
	struct utf_decode_func_traits
	{
		using return_type =
			decltype(std::declval<Func>()(std::declval<T>(), std::declval<size_t>(), std::declval<size_t>()));
	};

	template <typename T, typename Func>
	struct utf_decode_func_traits<T, Func, 1>
	{
		using return_type = decltype(std::declval<Func>()(std::declval<T>(), std::declval<size_t>()));
	};

	template <typename T, typename Func>
	struct utf_decode_func_traits<T, Func, 0>
	{
		using return_type = decltype(std::declval<Func>()(std::declval<T>()));
	};

	template <typename T>
	MUU_PURE_GETTER
	constexpr bool utf_detect_platform_endian(const T* data, const T* const end) noexcept
	{
		static_assert(sizeof(T) >= 2);
		int low{}, high{}; // number of zeroes in low/high byte
		for (auto c = data; c < end; c++)
		{
			low += static_cast<int>(byte_select<0>(*c) == uint8_t{});
			high += static_cast<int>(byte_select<sizeof(T) - 1u>(*c) == uint8_t{});
		}
		if constexpr (build::is_big_endian)
			return low > high;
		else
			return high > low;
	}
}
/// \endcond

namespace muu
{
	template <typename T, typename Func>
	constexpr void utf_decode(std::basic_string_view<T> str, bool reverse, Func&& func) noexcept
	{
		if (str.empty())
			return;

		static_assert(is_code_unit<T>, "unknown code unit type");
		static_assert(std::is_nothrow_invocable_v<Func&&, T, size_t, size_t> //
						  || std::is_nothrow_invocable_v<Func&&, T, size_t>	 //
						  || std::is_nothrow_invocable_v<Func&&, T>,		 //
					  "decoder func must be nothrow-invocable with (T), (T, size_t) or (T, size_t, size_t)");

		using func_return_type = typename impl::utf_decode_func_traits<T, Func>::return_type;
		constexpr auto stop_after_invoking =
			[](auto&& f, char32_t cp, size_t cu_start, size_t cu_count) noexcept -> bool
		{
			MUU_UNUSED(cu_start);
			MUU_UNUSED(cu_count);
			if constexpr (std::is_convertible_v<func_return_type, bool>)
			{
				if constexpr (std::is_nothrow_invocable_v<Func&&, T, size_t, size_t>)
					return !static_cast<bool>(static_cast<decltype(f)&&>(f)(cp, cu_start, cu_count));
				else if constexpr (std::is_nothrow_invocable_v<Func&&, T, size_t>)
					return !static_cast<bool>(static_cast<decltype(f)&&>(f)(cp, cu_start));
				else
					return !static_cast<bool>(static_cast<decltype(f)&&>(f)(cp));
			}
			else
			{
				if constexpr (std::is_nothrow_invocable_v<Func&&, T, size_t, size_t>)
					static_cast<decltype(f)&&>(f)(cp, cu_start, cu_count);
				else if constexpr (std::is_nothrow_invocable_v<Func&&, T, size_t>)
					static_cast<decltype(f)&&>(f)(cp, cu_start);
				else
					static_cast<decltype(f)&&>(f)(cp);
				return false;
			}
		};

		bool stop							 = false;
		size_t data_start					 = 0;
		[[maybe_unused]] bool requires_bswap = false;
		const auto get						 = [&](size_t idx) noexcept -> T
		{
			if constexpr (sizeof(T) == 1)
				return str[idx];
			else
			{
				if (requires_bswap)
					return static_cast<T>(byte_reverse(static_cast<unsigned_integer<sizeof(T) * CHAR_BIT>>(str[idx])));
				return str[idx];
			}
		};

		// UTF-32
		if constexpr (sizeof(T) == 4)
		{
			// endianness
			if (static_cast<uint32_t>(str[0]) == 0xFFFE0000u)
			{
				data_start	   = 1u;
				requires_bswap = true;
			}
			else if (static_cast<uint32_t>(str[0]) == 0x0000FEFFu)
				data_start = 1u;
			else
				requires_bswap = !impl::utf_detect_platform_endian(str.data(), str.data() + min(str.length(), 16_sz));

			if (reverse)
			{
				for (size_t i = str.length(); i-- > data_start && !stop;)
					stop = stop_after_invoking(static_cast<Func&&>(func), static_cast<char32_t>(get(i)), i, 1);
			}
			else
			{
				for (size_t i = data_start, e = str.length(); i < e && !stop; i++)
					stop = stop_after_invoking(static_cast<Func&&>(func), static_cast<char32_t>(get(i)), i, 1);
			}
		}

		// UTF-8, UTF-16
		else
		{
			// endianness
			if constexpr (sizeof(T) == 1)
			{
				if (str.length() >= 3u && bit_pack(str[0], str[1], str[2]) == 0x00EFBBBF)
					data_start = 3u;
			}
			else
			{
				if (static_cast<uint16_t>(str[0]) == 0xFFFEu)
				{
					data_start	   = 1u;
					requires_bswap = true;
				}
				else if (static_cast<uint16_t>(str[0]) == 0xFEFFu)
					data_start = 1u;
				else
					requires_bswap =
						!impl::utf_detect_platform_endian(str.data(), str.data() + min(str.length(), 16_sz));
			}

			impl::utf_decoder<T> decoder;
			if (reverse)
			{
				size_t cp_start				  = str.length();
				size_t cu_count				  = {};
				constexpr size_t max_cu_count = 4_sz / sizeof(T);
				while (cp_start-- > data_start && !stop)
				{
					cu_count++;
					if (cu_count == max_cu_count || is_code_point_boundary(get(cp_start)))
					{
						for (size_t i = cp_start, e = cp_start + cu_count; i < e; i++)
						{
							decoder(get(i));
							if (decoder.error())
								break;
						}

						if (decoder.has_value())
							stop = stop_after_invoking(static_cast<Func&&>(func), decoder.value(), cp_start, cu_count);
						else if (decoder.error())
						{
							decoder.clear_error();
							for (size_t i = cp_start + cu_count; i-- > cp_start && !stop;)
								stop =
									stop_after_invoking(static_cast<Func&&>(func), static_cast<char32_t>(get(i)), i, 1);
						}
						cu_count = {};
					}
				}
			}
			else
			{
				for (size_t i = data_start, e = str.length(), cp_start = data_start, cu_count = 0; i < e && !stop; i++)
				{
					cu_count++;
					decoder(get(i));
					if (decoder.has_value())
					{
						stop	 = stop_after_invoking(static_cast<Func&&>(func), decoder.value(), cp_start, cu_count);
						cp_start = i + 1u;
						cu_count = {};
					}
					else if (decoder.error())
					{
						decoder.clear_error();
						for (size_t j = cp_start, je = cp_start + cu_count; j < je && !stop; j++)
							stop = stop_after_invoking(static_cast<Func&&>(func), static_cast<char32_t>(get(j)), j, 1);
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
		utf_decode(str, false, static_cast<Func&&>(func));
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
