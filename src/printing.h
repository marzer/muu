// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "muu/core.h"
#include "muu/strings.h"
#include "muu/half.h"

MUU_DISABLE_WARNINGS;
#include <ostream>
#include <iomanip>
MUU_ENABLE_WARNINGS;

MUU_PUSH_WARNINGS;
MUU_DISABLE_SPAM_WARNINGS;

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

		stream_saver(std::basic_ios<Char>& ios) noexcept
			: stream{ ios },
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
		template <> struct stream_output_cast_type_<signed char> { using type = signed int; };
		template <> struct stream_output_cast_type_<unsigned char> { using type = unsigned int; };
		#if MUU_HAS_FP16
		template <> struct stream_output_cast_type_<__fp16> { using type = float; };
		#endif
		#if MUU_HAS_FLOAT16
		template <> struct stream_output_cast_type_<_Float16> { using type = float; };
		#endif
		#if MUU_HAS_FLOAT128
		template <> struct stream_output_cast_type_<float128_t> { using type = long double; };
		#endif
		#if MUU_HAS_INT128
		template <> struct stream_output_cast_type_<int128_t> { using type = signed long long; };
		template <> struct stream_output_cast_type_<uint128_t> { using type = unsigned long long; };
		#endif
	}

	template <typename T, typename Char>
	inline constexpr bool has_stream_output_operator = is_detected<impl::has_stream_output_operator_, T, Char>;

	template <typename T>
	using stream_output_cast_type = typename impl::stream_output_cast_type_<T>::type;

	template <typename Char>
	struct print_constants
	{
		static constexpr auto object_open = "{ "sv;
		static constexpr auto object_close = " }"sv;
		static constexpr auto next_list_item = ", "sv;
		static constexpr auto indent_buf = "                                                  "sv;
		static constexpr auto indent_width = 2u;
		static constexpr auto indent_max = static_cast<int>(indent_buf.length() / indent_width);
	};

	template <>
	struct print_constants<wchar_t>
	{
		static constexpr auto object_open = L"{ "sv;
		static constexpr auto object_close = L" }"sv;
		static constexpr auto next_list_item = L", "sv;
		static constexpr auto indent_buf = L"                                                  "sv;
		static constexpr auto indent_width = 2u;
		static constexpr auto indent_max = static_cast<int>(indent_buf.length() / indent_width);
	};

	#define MAKE_PRINTER_TAG(name) struct name##_tag {}; inline constexpr auto name = name##_tag{}
	MAKE_PRINTER_TAG(new_line);
	MAKE_PRINTER_TAG(indent);
	MAKE_PRINTER_TAG(object_open);
	MAKE_PRINTER_TAG(object_close);
	MAKE_PRINTER_TAG(next_list_item);

	template <typename Char>
	struct printer // a wrapper around ostreams to make them less stupid
	{
		std::basic_ostream<Char>& stream;
		int indent_count = 0;

		printer(std::basic_ostream<Char>& os) noexcept
			: stream{ os }
		{}

		template <typename T>
		static void print_number(std::basic_ostream<Char>& os, T value)
		{
			//todo: use quadmath sprintf for float128

			// print chars as ints so they don't get interpreted as text;
			// non-standard ints and floats can just get casted if they don't have an operator<< overload
			if constexpr ((is_integral<T> && sizeof(T) == 1) || !has_stream_output_operator<T, Char>)
				os << static_cast<stream_output_cast_type<T>>(value);

			// sane things
			else
				os << value;
		}

		printer& operator() (  signed char      val) { print_number(stream, val); return *this; }
		printer& operator() (  signed short     val) { print_number(stream, val); return *this; }
		printer& operator() (  signed int       val) { print_number(stream, val); return *this; }
		printer& operator() (  signed long      val) { print_number(stream, val); return *this; }
		printer& operator() (  signed long long val) { print_number(stream, val); return *this; }
		printer& operator() (unsigned char      val) { print_number(stream, val); return *this; }
		printer& operator() (unsigned short     val) { print_number(stream, val); return *this; }
		printer& operator() (unsigned int       val) { print_number(stream, val); return *this; }
		printer& operator() (unsigned long      val) { print_number(stream, val); return *this; }
		printer& operator() (unsigned long long val) { print_number(stream, val); return *this; }
		printer& operator() (half               val) { print_number(stream, val); return *this; }
		printer& operator() (float              val) { print_number(stream, val); return *this; }
		printer& operator() (double             val) { print_number(stream, val); return *this; }
		printer& operator() (long double        val) { print_number(stream, val); return *this; }
		#if MUU_HAS_FP16
		printer& operator() (__fp16 val)             { print_number(stream, val); return *this; }
		#endif
		#if MUU_HAS_FLOAT16
		printer& operator() (_Float16 val)           { print_number(stream, val); return *this; }
		#endif
		#if MUU_HAS_FLOAT128
		printer& operator() (float128_t val)         { print_number(stream, val); return *this; }
		#endif
		#if MUU_HAS_INT128
		printer& operator() (int128_t val)           { print_number(stream, val); return *this; }
		printer& operator() (uint128_t val)          { print_number(stream, val); return *this; }
		#endif

		static void print_string(std::basic_ostream<Char>& os, const Char* start, size_t len)
		{
			os.write(start, static_cast<std::streamsize>(len));
		}

		printer& operator() (Char c)
		{
			stream.put(c);
			return *this;
		}

		printer& operator() (const std::basic_string_view<Char>& s)
		{
			if (!s.empty())
				print_string(stream, s.data(), s.length());
			return *this;
		}

		printer& operator() (const std::basic_string<Char>& s)
		{
			if (!s.empty())
				print_string(stream, s.c_str(), s.length());
			return *this;
		}

		printer& operator() (const Char* s)
		{
			if (s)
				print_string(stream, s, std::char_traits<Char>::length(s));
		}

		template <size_t N>
		printer& operator()(const Char(&s)[N])
		{
			print_string(stream, &s, N);
		}

		printer& operator++(int) noexcept
		{
			indent_count++;
			return *this;
		}

		printer& operator--(int) noexcept
		{
			indent_count--;
			return *this;
		}

		printer& operator()(indent_tag)
		{
			using pc = print_constants<Char>;
			auto i = muu::max(indent_count, 0);
			while (i > 0)
			{
				print_string(stream, pc::indent_buf.data(), static_cast<size_t>(muu::min(i, pc::indent_max)) * pc::indent_width);
				i -= pc::indent_max;
			}
			return *this;
		}

		printer& operator()(new_line_tag)             { return (*this)(constants<Char>::new_line); }
		printer& operator()(object_open_tag)          { return (*this)(print_constants<Char>::object_open); }
		printer& operator()(object_close_tag)         { return (*this)(print_constants<Char>::object_close); }
		printer& operator()(next_list_item_tag)       { return (*this)(print_constants<Char>::next_list_item); }
	};

	template <typename Char>
	printer(std::basic_ostream<Char>&) -> printer<Char>;
}
MUU_NAMESPACE_END

MUU_POP_WARNINGS; // MUU_DISABLE_SPAM_WARNINGS
