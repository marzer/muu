// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "muu/impl/vector_types_common.h"
#include "stream_printer.h"

#include "source_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;

//======================================================================================================================
// vectors
//======================================================================================================================

namespace
{
	template <typename Char, typename T>
	static void print_vector(std::basic_ostream<Char>& os, const T* x, size_t dims) noexcept
	{
		auto saver	= stream_saver{ os }; // restores flags, precision, width and fill
		saver.width = 0;				  // operator<< consumes width

		if constexpr (is_floating_point<T>)
		{
			constexpr std::streamsize precision = std::streamsize{ 3 } + (sizeof(T) >= sizeof(float) ? 3 : 0)
												+ (sizeof(T) >= sizeof(double) ? 3 : 0)
												+ (sizeof(T) > sizeof(double) ? 3 : 0);
			os.precision(precision);
		}
		os << std::dec;

		auto printer = ::stream_printer{ os };

		// "{ "
		printer(object_open);

		// ", <scalar>"
		printer(list, x, dims);

		// " }"
		printer(object_close);
	}
}

namespace muu::impl
{
	void MUU_CALLCONV print_vector(std::ostream& os, const half* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::ostream& os, const float* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::ostream& os, const double* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::ostream& os, const long double* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::ostream& os, const signed char* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::ostream& os, const signed short* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::ostream& os, const signed int* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::ostream& os, const signed long* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::ostream& os, const signed long long* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::ostream& os, const unsigned char* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::ostream& os, const unsigned short* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::ostream& os, const unsigned int* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::ostream& os, const unsigned long* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::ostream& os, const unsigned long long* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

#if MUU_HAS_FLOAT16
	void MUU_CALLCONV print_vector(std::ostream& os, const _Float16* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}
#endif

#if MUU_HAS_FP16
	void MUU_CALLCONV print_vector(std::ostream& os, const __fp16* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}
#endif

#if MUU_HAS_FLOAT128
	void MUU_CALLCONV print_vector(std::ostream& os, const float128_t* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}
#endif

#if MUU_HAS_INT128
	void MUU_CALLCONV print_vector(std::ostream& os, const int128_t* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::ostream& os, const uint128_t* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}
#endif

	void MUU_CALLCONV print_vector(std::wostream& os, const half* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::wostream& os, const float* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::wostream& os, const double* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::wostream& os, const long double* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::wostream& os, const signed char* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::wostream& os, const signed short* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::wostream& os, const signed int* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::wostream& os, const signed long* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::wostream& os, const signed long long* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::wostream& os, const unsigned char* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::wostream& os, const unsigned short* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::wostream& os, const unsigned int* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::wostream& os, const unsigned long* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::wostream& os, const unsigned long long* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

#if MUU_HAS_FLOAT16
	void MUU_CALLCONV print_vector(std::wostream& os, const _Float16* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}
#endif

#if MUU_HAS_FP16
	void MUU_CALLCONV print_vector(std::wostream& os, const __fp16* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}
#endif

#if MUU_HAS_FLOAT128
	void MUU_CALLCONV print_vector(std::wostream& os, const float128_t* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}
#endif

#if MUU_HAS_INT128
	void MUU_CALLCONV print_vector(std::wostream& os, const int128_t* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}

	void MUU_CALLCONV print_vector(std::wostream& os, const uint128_t* x, size_t dims)
	{
		::print_vector(os, x, dims);
	}
#endif
}

//======================================================================================================================
// matrices
//======================================================================================================================

namespace
{
	template <typename Char, typename T>
	static void print_matrix(std::basic_ostream<Char>& os, const T* m, size_t rows, size_t cols) noexcept
	{
		if (rows == 1)
		{
			::print_vector(os, m, cols);
			return;
		}

		auto saver	= stream_saver{ os }; // restores flags, precision, width and fill
		saver.width = 0;				  // operator<< consumes width

		if constexpr (is_floating_point<T>)
		{
			constexpr std::streamsize precision = std::streamsize{ 3 } + (sizeof(T) >= sizeof(float) ? 3 : 0)
												+ (sizeof(T) >= sizeof(double) ? 3 : 0)
												+ (sizeof(T) > sizeof(double) ? 3 : 0);
			os.precision(precision);
		}
		os << std::dec;

		auto printer = ::stream_printer{ os };

		// "{ "
		printer(object_open);
		printer++;

		for (size_t r = 0; r < rows; r++)
		{
			// ",\n  "
			if (r > 0)
				printer(breaking_list_item);

			for (size_t c = 0; c < cols; c++)
			{
				// ", "
				if (c > 0)
					printer(next_list_item);

				// "<scalar>"
				printer(m[c * rows + r]);
			}
		}

		// " }"
		printer--;
		printer(object_close);
	}
}

namespace muu::impl
{
	void MUU_CALLCONV print_matrix(std::ostream& os, const half* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::ostream& os, const float* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::ostream& os, const double* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::ostream& os, const long double* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::ostream& os, const signed char* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::ostream& os, const signed short* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::ostream& os, const signed int* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::ostream& os, const signed long* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::ostream& os, const signed long long* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::ostream& os, const unsigned char* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::ostream& os, const unsigned short* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::ostream& os, const unsigned int* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::ostream& os, const unsigned long* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::ostream& os, const unsigned long long* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

#if MUU_HAS_FLOAT16
	void MUU_CALLCONV print_matrix(std::ostream& os, const _Float16* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}
#endif

#if MUU_HAS_FP16
	void MUU_CALLCONV print_matrix(std::ostream& os, const __fp16* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}
#endif

#if MUU_HAS_FLOAT128
	void MUU_CALLCONV print_matrix(std::ostream& os, const float128_t* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}
#endif

#if MUU_HAS_INT128
	void MUU_CALLCONV print_matrix(std::ostream& os, const int128_t* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::ostream& os, const uint128_t* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}
#endif

	void MUU_CALLCONV print_matrix(std::wostream& os, const half* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::wostream& os, const float* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::wostream& os, const double* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::wostream& os, const long double* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::wostream& os, const signed char* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::wostream& os, const signed short* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::wostream& os, const signed int* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::wostream& os, const signed long* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::wostream& os, const signed long long* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::wostream& os, const unsigned char* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::wostream& os, const unsigned short* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::wostream& os, const unsigned int* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::wostream& os, const unsigned long* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::wostream& os, const unsigned long long* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

#if MUU_HAS_FLOAT16
	void MUU_CALLCONV print_matrix(std::wostream& os, const _Float16* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}
#endif

#if MUU_HAS_FP16
	void MUU_CALLCONV print_matrix(std::wostream& os, const __fp16* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}
#endif

#if MUU_HAS_FLOAT128
	void MUU_CALLCONV print_matrix(std::wostream& os, const float128_t* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}
#endif

#if MUU_HAS_INT128
	void MUU_CALLCONV print_matrix(std::wostream& os, const int128_t* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}

	void MUU_CALLCONV print_matrix(std::wostream& os, const uint128_t* m, size_t rows, size_t cols)
	{
		::print_matrix(os, m, rows, cols);
	}
#endif
}

//======================================================================================================================
// compound vector types
//======================================================================================================================

namespace
{
	template <typename Char, typename T>
	static void print_compound_vector(std::basic_ostream<Char>& os,
									  const T* vals1,
									  size_t num1,
									  bool is_vec1,
									  const T* vals2,
									  size_t num2,
									  bool is_vec2) noexcept
	{
		auto saver	= stream_saver{ os }; // restores flags, precision, width and fill
		saver.width = 0;				  // operator<< consumes width

		if constexpr (is_floating_point<T>)
		{
			constexpr std::streamsize precision = std::streamsize{ 3 } + (sizeof(T) >= sizeof(float) ? 3 : 0)
												+ (sizeof(T) >= sizeof(double) ? 3 : 0)
												+ (sizeof(T) > sizeof(double) ? 3 : 0);
			os.precision(precision);
		}
		os << std::dec;

		auto printer = ::stream_printer{ os };

		// "{ "
		printer(object_open);

		// "<first vector or list>"
		if (is_vec1)
			printer(object_open);
		printer(list, vals1, num1);
		if (is_vec1)
			printer(object_close);

		// ", "
		printer(next_list_item);

		// "<second vector or list>"
		if (is_vec2)
			printer(object_open);
		printer(list, vals2, num2);
		if (is_vec2)
			printer(object_close);

		// " }"
		printer(object_close);
	}
}

namespace muu::impl
{
	void MUU_CALLCONV print_compound_vector(std::ostream& os,
											const half* vals1,
											size_t num1,
											bool is_vec1,
											const half* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::ostream& os,
											const float* vals1,
											size_t num1,
											bool is_vec1,
											const float* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::ostream& os,
											const double* vals1,
											size_t num1,
											bool is_vec1,
											const double* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::ostream& os,
											const long double* vals1,
											size_t num1,
											bool is_vec1,
											const long double* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::ostream& os,
											const signed char* vals1,
											size_t num1,
											bool is_vec1,
											const signed char* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::ostream& os,
											const signed short* vals1,
											size_t num1,
											bool is_vec1,
											const signed short* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::ostream& os,
											const signed int* vals1,
											size_t num1,
											bool is_vec1,
											const signed int* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::ostream& os,
											const signed long* vals1,
											size_t num1,
											bool is_vec1,
											const signed long* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::ostream& os,
											const signed long long* vals1,
											size_t num1,
											bool is_vec1,
											const signed long long* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::ostream& os,
											const unsigned char* vals1,
											size_t num1,
											bool is_vec1,
											const unsigned char* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::ostream& os,
											const unsigned short* vals1,
											size_t num1,
											bool is_vec1,
											const unsigned short* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::ostream& os,
											const unsigned int* vals1,
											size_t num1,
											bool is_vec1,
											const unsigned int* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::ostream& os,
											const unsigned long* vals1,
											size_t num1,
											bool is_vec1,
											const unsigned long* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::ostream& os,
											const unsigned long long* vals1,
											size_t num1,
											bool is_vec1,
											const unsigned long long* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

#if MUU_HAS_FLOAT16
	void MUU_CALLCONV print_compound_vector(std::ostream& os,
											const _Float16* vals1,
											size_t num1,
											bool is_vec1,
											const _Float16* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}
#endif

#if MUU_HAS_FP16
	void MUU_CALLCONV print_compound_vector(std::ostream& os,
											const __fp16* vals1,
											size_t num1,
											bool is_vec1,
											const __fp16* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}
#endif

#if MUU_HAS_FLOAT128
	void MUU_CALLCONV print_compound_vector(std::ostream& os,
											const float128_t* vals1,
											size_t num1,
											bool is_vec1,
											const float128_t* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}
#endif

#if MUU_HAS_INT128
	void MUU_CALLCONV print_compound_vector(std::ostream& os,
											const int128_t* vals1,
											size_t num1,
											bool is_vec1,
											const int128_t* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::ostream& os,
											const uint128_t* vals1,
											size_t num1,
											bool is_vec1,
											const uint128_t* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}
#endif

	void MUU_CALLCONV print_compound_vector(std::wostream& os,
											const half* vals1,
											size_t num1,
											bool is_vec1,
											const half* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::wostream& os,
											const float* vals1,
											size_t num1,
											bool is_vec1,
											const float* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::wostream& os,
											const double* vals1,
											size_t num1,
											bool is_vec1,
											const double* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::wostream& os,
											const long double* vals1,
											size_t num1,
											bool is_vec1,
											const long double* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::wostream& os,
											const signed char* vals1,
											size_t num1,
											bool is_vec1,
											const signed char* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::wostream& os,
											const signed short* vals1,
											size_t num1,
											bool is_vec1,
											const signed short* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::wostream& os,
											const signed int* vals1,
											size_t num1,
											bool is_vec1,
											const signed int* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::wostream& os,
											const signed long* vals1,
											size_t num1,
											bool is_vec1,
											const signed long* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::wostream& os,
											const signed long long* vals1,
											size_t num1,
											bool is_vec1,
											const signed long long* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::wostream& os,
											const unsigned char* vals1,
											size_t num1,
											bool is_vec1,
											const unsigned char* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::wostream& os,
											const unsigned short* vals1,
											size_t num1,
											bool is_vec1,
											const unsigned short* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::wostream& os,
											const unsigned int* vals1,
											size_t num1,
											bool is_vec1,
											const unsigned int* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::wostream& os,
											const unsigned long* vals1,
											size_t num1,
											bool is_vec1,
											const unsigned long* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::wostream& os,
											const unsigned long long* vals1,
											size_t num1,
											bool is_vec1,
											const unsigned long long* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

#if MUU_HAS_FLOAT16
	void MUU_CALLCONV print_compound_vector(std::wostream& os,
											const _Float16* vals1,
											size_t num1,
											bool is_vec1,
											const _Float16* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}
#endif

#if MUU_HAS_FP16
	void MUU_CALLCONV print_compound_vector(std::wostream& os,
											const __fp16* vals1,
											size_t num1,
											bool is_vec1,
											const __fp16* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}
#endif

#if MUU_HAS_FLOAT128
	void MUU_CALLCONV print_compound_vector(std::wostream& os,
											const float128_t* vals1,
											size_t num1,
											bool is_vec1,
											const float128_t* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}
#endif

#if MUU_HAS_INT128
	void MUU_CALLCONV print_compound_vector(std::wostream& os,
											const int128_t* vals1,
											size_t num1,
											bool is_vec1,
											const int128_t* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}

	void MUU_CALLCONV print_compound_vector(std::wostream& os,
											const uint128_t* vals1,
											size_t num1,
											bool is_vec1,
											const uint128_t* vals2,
											size_t num2,
											bool is_vec2)
	{
		::print_compound_vector(os, vals1, num1, is_vec1, vals2, num2, is_vec2);
	}
#endif
}
