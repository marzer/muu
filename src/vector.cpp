// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "muu/vector.h"
#include "printing.h"

using namespace muu;

namespace
{
	template <typename Char, typename T>
	static void print(std::basic_ostream<Char>& os, const T* x, size_t dims) noexcept
	{
		auto saver = stream_saver{ os }; // restores flags, precision, width and fill
		os.width(0);

		if constexpr (is_floating_point<T>)
		{
			constexpr std::streamsize precision = std::streamsize{ 3 }
				+ (sizeof(T) >= sizeof(float) ? 3 : 0)
				+ (sizeof(T) >= sizeof(double) ? 3 : 0)
				+ (sizeof(T) > sizeof(double) ? 3 : 0);
			os.precision(precision);
		}
		os << std::dec;

		auto printer = ::printer{ os };

		// "{ "
		printer(object_open);

		// ", <scalar>"
		for (size_t i = 0; i < dims; i++)
		{
			if (i > 0)
				printer(next_list_item);

			os << std::right;
			os.width(saver.width);
			printer(x[i]);
		}

		// " }"
		printer(object_close);

		saver.width = 0; // operator<< consumes width
	}
}

namespace muu::impl
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
	void print_vector_to_stream(std::ostream& os, const float128_t* x, size_t dims)
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
	void print_vector_to_stream(std::wostream& os, const float128_t* x, size_t dims)
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
