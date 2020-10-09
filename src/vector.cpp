// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "muu/vector.h"
#include "muu/half.h"
#include "printing.h"

using namespace muu;

namespace
{
	template <typename Char, typename T>
	static void print(std::basic_ostream<Char>& os, const T* x, size_t dims) noexcept
	{
		using chars = constants<Char>;
		auto saver = stream_saver{ os }; // restores flags, precision, width and fill
		
		os.width(0);
		if constexpr (is_floating_point<T>)
		{
			constexpr std::streamsize precision = 3
				+ (sizeof(T) >= sizeof(float) ? 3 : 0)
				+ (sizeof(T) >= sizeof(double) ? 3 : 0)
				+ (sizeof(T) > sizeof(double) ? 3 : 0);
			os.precision(precision);
		}
		os << std::dec;

		os.put(chars::left_brace);
		os.put(chars::space);
		for (size_t i = 0; i < dims; i++)
		{
			if (i > 0)
			{
				os.put(chars::comma);
				os.put(chars::space);
			}

			os << std::right;
			os.width(saver.width);
			print_number(os, x[i]);
		}

		os.put(chars::space);
		os.put(chars::right_brace);
		saver.width = 0; // operator<< consumes width
	}
}

MUU_IMPL_NAMESPACE_START
{
	void print_vector_to_stream(std::ostream& os, const half* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::ostream& os, const float* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::ostream& os, const double* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::ostream& os, const long double* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::ostream& os, const signed char* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::ostream& os, const signed short* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::ostream& os, const signed int* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::ostream& os, const signed long* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::ostream& os, const signed long long* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::ostream& os, const unsigned char* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::ostream& os, const unsigned short* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::ostream& os, const unsigned int* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::ostream& os, const unsigned long* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::ostream& os, const unsigned long long* x, size_t dims)
	{
		::print(os, x, dims);
	}

	#if MUU_HAS_FLOAT16
	void print_vector_to_stream(std::ostream& os, const _Float16* x, size_t dims)
	{
		::print(os, x, dims);
	}
	#endif

	#if MUU_HAS_FP16
	void print_vector_to_stream(std::ostream& os, const __fp16* x, size_t dims)
	{
		::print(os, x, dims);
	}
	#endif

	#if MUU_HAS_FLOAT128
	void print_vector_to_stream(std::ostream& os, const quad* x, size_t dims)
	{
		::print(os, x, dims);
	}
	#endif

	#if MUU_HAS_INT128
	void print_vector_to_stream(std::ostream& os, const int128_t* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::ostream& os, const uint128_t* x, size_t dims)
	{
		::print(os, x, dims);
	}
	#endif

	void print_vector_to_stream(std::wostream& os, const half* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::wostream& os, const float* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::wostream& os, const double* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::wostream& os, const long double* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::wostream& os, const signed char* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::wostream& os, const signed short* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::wostream& os, const signed int* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::wostream& os, const signed long* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::wostream& os, const signed long long* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::wostream& os, const unsigned char* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::wostream& os, const unsigned short* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::wostream& os, const unsigned int* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::wostream& os, const unsigned long* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::wostream& os, const unsigned long long* x, size_t dims)
	{
		::print(os, x, dims);
	}

	#if MUU_HAS_FLOAT16
	void print_vector_to_stream(std::wostream& os, const _Float16* x, size_t dims)
	{
		::print(os, x, dims);
	}
	#endif

	#if MUU_HAS_FP16
	void print_vector_to_stream(std::wostream& os, const __fp16* x, size_t dims)
	{
		::print(os, x, dims);
	}
	#endif

	#if MUU_HAS_FLOAT128
	void print_vector_to_stream(std::wostream& os, const quad* x, size_t dims)
	{
		::print(os, x, dims);
	}
	#endif

	#if MUU_HAS_INT128
	void print_vector_to_stream(std::wostream& os, const int128_t* x, size_t dims)
	{
		::print(os, x, dims);
	}

	void print_vector_to_stream(std::wostream& os, const uint128_t* x, size_t dims)
	{
		::print(os, x, dims);
	}
	#endif
}
MUU_IMPL_NAMESPACE_END
