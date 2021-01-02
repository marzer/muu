// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "muu/quaternion.h"
#include "printing.h"

using namespace muu;

namespace
{
	template <typename Char, typename T>
	static void print(std::basic_ostream<Char>& os, const T* q) noexcept
	{
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

		// "{ "
		printer(object_open);

		// "<scalar>"
		os << std::right;
		os.width(saver.width);
		printer(*q);

		// ", <vector>"
		printer(next_list_item);
		os.width(saver.width);
		impl::print_vector_to_stream(os, q + 1, 3_sz);

		// " }"
		printer(object_close);

		saver.width = 0; // operator<< consumes width
	}
}

MUU_IMPL_NAMESPACE_START
{
	void print_quaternion_to_stream(std::ostream& os, const half* q)
	{
		::print(os, q);
	}

	void print_quaternion_to_stream(std::ostream& os, const float* q)
	{
		::print(os, q);
	}

	void print_quaternion_to_stream(std::ostream& os, const double* q)
	{
		::print(os, q);
	}

	void print_quaternion_to_stream(std::ostream& os, const long double* q)
	{
		::print(os, q);
	}

	#if MUU_HAS_FLOAT16
	void print_quaternion_to_stream(std::ostream& os, const _Float16* q)
	{
		::print(os, q);
	}
	#endif

	#if MUU_HAS_FP16
	void print_quaternion_to_stream(std::ostream& os, const __fp16* q)
	{
		::print(os, q);
	}
	#endif

	#if MUU_HAS_FLOAT128
	void print_quaternion_to_stream(std::ostream& os, const float128_t* q)
	{
		::print(os, q);
	}
	#endif

	void print_quaternion_to_stream(std::wostream& os, const half* q)
	{
		::print(os, q);
	}

	void print_quaternion_to_stream(std::wostream& os, const float* q)
	{
		::print(os, q);
	}

	void print_quaternion_to_stream(std::wostream& os, const double* q)
	{
		::print(os, q);
	}

	void print_quaternion_to_stream(std::wostream& os, const long double* q)
	{
		::print(os, q);
	}

	#if MUU_HAS_FLOAT16
	void print_quaternion_to_stream(std::wostream& os, const _Float16* q)
	{
		::print(os, q);
	}
	#endif

	#if MUU_HAS_FP16
	void print_quaternion_to_stream(std::wostream& os, const __fp16* q)
	{
		::print(os, q);
	}
	#endif

	#if MUU_HAS_FLOAT128
	void print_quaternion_to_stream(std::wostream& os, const float128_t* q)
	{
		::print(os, q);
	}
	#endif
}
MUU_IMPL_NAMESPACE_END
