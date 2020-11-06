// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "muu/matrix.h"
#include "printing.h"

using namespace muu;

namespace
{
	template <typename Char, typename T>
	static void print(std::basic_ostream<Char>& os, const T* m, size_t rows, size_t cols) noexcept
	{
		if (rows == 1)
		{
			impl::print_vector_to_stream(os, m, cols);
			return;
		}

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

		auto printer = ::printer{ os };

		// "{\n"
		printer(complex_object_open);

		for (size_t r = 0; r < rows; r++)
		{
			// ",\n"
			if (r > 0)
			{
				printer(next_list_item);
				printer(new_line);
			}
			printer(indent);

			for (size_t c = 0; c < cols; c++)
			{
				// ", "
				if (c > 0)
					printer(next_list_item);

				os << std::right;
				os.width(saver.width);
				printer(m[c * rows + r]);
			}
		}

		// "\n}"
		printer(complex_object_close);

		saver.width = 0; // operator<< consumes width
	}
}


MUU_IMPL_NAMESPACE_START
{
	void print_matrix_to_stream(std::ostream& os, const half* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::ostream& os, const float* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::ostream& os, const double* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::ostream& os, const long double* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::ostream& os, const signed char* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::ostream& os, const signed short* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::ostream& os, const signed int* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::ostream& os, const signed long* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::ostream& os, const signed long long* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::ostream& os, const unsigned char* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::ostream& os, const unsigned short* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::ostream& os, const unsigned int* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::ostream& os, const unsigned long* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::ostream& os, const unsigned long long* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	#if MUU_HAS_FLOAT16
	void print_matrix_to_stream(std::ostream& os, const _Float16* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}
	#endif

	#if MUU_HAS_FP16
	void print_matrix_to_stream(std::ostream& os, const __fp16* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}
	#endif

	#if MUU_HAS_FLOAT128
	void print_matrix_to_stream(std::ostream& os, const float128_t* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}
	#endif

	#if MUU_HAS_INT128
	void print_matrix_to_stream(std::ostream& os, const int128_t* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::ostream& os, const uint128_t* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}
	#endif

	void print_matrix_to_stream(std::wostream& os, const half* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::wostream& os, const float* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::wostream& os, const double* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::wostream& os, const long double* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::wostream& os, const signed char* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::wostream& os, const signed short* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::wostream& os, const signed int* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::wostream& os, const signed long* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::wostream& os, const signed long long* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::wostream& os, const unsigned char* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::wostream& os, const unsigned short* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::wostream& os, const unsigned int* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::wostream& os, const unsigned long* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::wostream& os, const unsigned long long* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	#if MUU_HAS_FLOAT16
	void print_matrix_to_stream(std::wostream& os, const _Float16* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}
	#endif

	#if MUU_HAS_FP16
	void print_matrix_to_stream(std::wostream& os, const __fp16* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}
	#endif

	#if MUU_HAS_FLOAT128
	void print_matrix_to_stream(std::wostream& os, const float128_t* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}
	#endif

	#if MUU_HAS_INT128
	void print_matrix_to_stream(std::wostream& os, const int128_t* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}

	void print_matrix_to_stream(std::wostream& os, const uint128_t* m, size_t rows, size_t cols)
	{
		::print(os, m, rows, cols);
	}
	#endif
}
MUU_IMPL_NAMESPACE_END
