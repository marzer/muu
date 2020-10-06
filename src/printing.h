// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "muu/core.h"
MUU_DISABLE_WARNINGS
#include <ostream>
#include <iomanip>
MUU_ENABLE_WARNINGS

MUU_PUSH_WARNINGS
MUU_DISABLE_SPAM_WARNINGS

MUU_NAMESPACE_START
{
	template <typename Char>
	struct stream_saver final
	{
		std::basic_ios<Char>& stream;
		std::ios_base::fmtflags flags;
		std::streamsize precision;
		std::streamsize width;
		Char fill;

		stream_saver(std::basic_ios<Char>& os) noexcept
			: stream{ os },
			flags{ stream.flags() },
			precision{ stream.precision() },
			width{ stream.width() },
			fill{ stream.fill() }
		{}

		stream_saver(const stream_saver&) = delete;
		stream_saver(stream_saver&&) = delete;
		stream_saver& operator = (const stream_saver&) = delete;
		stream_saver& operator = (stream_saver&&) = delete;

		~stream_saver() noexcept
		{
			stream.flags(flags);
			stream.precision(precision);
			stream.width(width);
			stream.fill(fill);
		}
	};

	template <typename Char>
	stream_saver(std::basic_ios<Char>&) -> stream_saver<Char>;

	namespace impl
	{
		template <typename From>
		struct non_convertible
		{
			constexpr operator From() const noexcept;
		};

		template <typename T, typename Char>
		using has_stream_output_operator_ = decltype(std::declval<std::basic_ostream<Char>&>() << std::declval<non_convertible<const T&>>());

		template <typename T> struct stream_output_cast_type_ { using type = T; };
		template <> struct stream_output_cast_type_<half> { using type = float; };
		template <> struct stream_output_cast_type_<signed char> { using type = signed int; };
		template <> struct stream_output_cast_type_<unsigned char> { using type = unsigned int; };
		#if MUU_HAS_FP16
		template <> struct stream_output_cast_type_<__fp16> { using type = float; };
		#endif
		#if MUU_HAS_FLOAT16
		template <> struct stream_output_cast_type_<_Float16> { using type = float; };
		#endif
		#if MUU_HAS_FLOAT128
		template <> struct stream_output_cast_type_<quad> { using type = long double; };
		#endif
		#if MUU_HAS_INT128
		template <> struct stream_output_cast_type_<int128_t> { using type = signed long long; };
		template <> struct stream_output_cast_type_<uint128_t> { using type = unsigned long long; };
		#endif
	}

	template <typename T, typename Char>
	inline constexpr bool has_stream_output_operator = impl::is_detected<impl::has_stream_output_operator_, T, Char>;

	template <typename T>
	using stream_output_cast_type = typename impl::stream_output_cast_type_<T>::type;

	template <typename Char, typename T>
	inline void print_number(std::basic_ostream<Char>& os, T value) noexcept
	{
		// print chars as ints so they don't get interpreted as text;
		// non-standard ints and floats can just get casted if they don't have an operator<< overload
		if constexpr ((is_integral<T> && sizeof(T) == 1) || !has_stream_output_operator<T, Char>)
			os << static_cast<stream_output_cast_type<T>>(value);

		// sane things
		else
			os << value;
	}
}
MUU_NAMESPACE_END

MUU_POP_WARNINGS // MUU_DISABLE_SPAM_WARNINGS
