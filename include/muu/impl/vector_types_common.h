// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once
/// \cond

#include "../math.h"
#include "std_iosfwd.h"
#include "header_start.h"

namespace muu::impl
{
	template <typename T, typename U>
	using equality_check_type =
		std::conditional_t<is_signed<T> != is_signed<U> || is_floating_point<T> != is_floating_point<U>,
						   promote_if_small_float<highest_ranked<T, U>>,
						   highest_ranked<T, U>>;

	template <typename T>
	MUU_CONST_INLINE_GETTER
	constexpr auto MUU_VECTORCALL raw_modulo(T lhs, T rhs) noexcept // todo: constexpr fmod
	{
		if constexpr (is_floating_point<T>)
		{
			if constexpr (is_standard_arithmetic<T>)
				return std::fmod(lhs, rhs);
#if MUU_HAS_QUADMATH
			else if constexpr (std::is_same_v<float128_t, T>)
				return ::fmodq(lhs, rhs);
#endif
			else
				return static_cast<T>(std::fmod(static_cast<clamp_to_standard_float<T>>(lhs),
												static_cast<clamp_to_standard_float<T>>(rhs)));
		}
		else
			return lhs % rhs;
	}

	MUU_API
	void MUU_CALLCONV print_vector(std::ostream& stream, const half*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::ostream& stream, const float*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::ostream& stream, const double*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::ostream& stream, const long double*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::ostream& stream, const signed char*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::ostream& stream, const signed short*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::ostream& stream, const signed int*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::ostream& stream, const signed long*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::ostream& stream, const signed long long*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::ostream& stream, const unsigned char*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::ostream& stream, const unsigned short*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::ostream& stream, const unsigned int*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::ostream& stream, const unsigned long*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::ostream& stream, const unsigned long long*, size_t);
#if MUU_HAS_FLOAT16
	MUU_API
	void MUU_CALLCONV print_vector(std::ostream& stream, const _Float16*, size_t);
#endif
#if MUU_HAS_FP16
	MUU_API
	void MUU_CALLCONV print_vector(std::ostream& stream, const __fp16*, size_t);
#endif
#if MUU_HAS_FLOAT128
	MUU_API
	void MUU_CALLCONV print_vector(std::ostream& stream, const float128_t*, size_t);
#endif
#if MUU_HAS_INT128
	MUU_API
	void MUU_CALLCONV print_vector(std::ostream& stream, const int128_t*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::ostream& stream, const uint128_t*, size_t);
#endif

	MUU_API
	void MUU_CALLCONV print_vector(std::wostream& stream, const half*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::wostream& stream, const float*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::wostream& stream, const double*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::wostream& stream, const long double*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::wostream& stream, const signed char*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::wostream& stream, const signed short*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::wostream& stream, const signed int*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::wostream& stream, const signed long*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::wostream& stream, const signed long long*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::wostream& stream, const unsigned char*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::wostream& stream, const unsigned short*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::wostream& stream, const unsigned int*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::wostream& stream, const unsigned long*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::wostream& stream, const unsigned long long*, size_t);
#if MUU_HAS_FLOAT16
	MUU_API
	void MUU_CALLCONV print_vector(std::wostream& stream, const _Float16*, size_t);
#endif
#if MUU_HAS_FP16
	MUU_API
	void MUU_CALLCONV print_vector(std::wostream& stream, const __fp16*, size_t);
#endif
#if MUU_HAS_FLOAT128
	MUU_API
	void MUU_CALLCONV print_vector(std::wostream& stream, const float128_t*, size_t);
#endif
#if MUU_HAS_INT128
	MUU_API
	void MUU_CALLCONV print_vector(std::wostream& stream, const int128_t*, size_t);
	MUU_API
	void MUU_CALLCONV print_vector(std::wostream& stream, const uint128_t*, size_t);
#endif

	MUU_API
	void MUU_CALLCONV print_matrix(std::ostream& stream, const half*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::ostream& stream, const float*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::ostream& stream, const double*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::ostream& stream, const long double*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::ostream& stream, const signed char*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::ostream& stream, const signed short*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::ostream& stream, const signed int*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::ostream& stream, const signed long*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::ostream& stream, const signed long long*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::ostream& stream, const unsigned char*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::ostream& stream, const unsigned short*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::ostream& stream, const unsigned int*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::ostream& stream, const unsigned long*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::ostream& stream, const unsigned long long*, size_t, size_t);
#if MUU_HAS_FLOAT16
	MUU_API
	void MUU_CALLCONV print_matrix(std::ostream& stream, const _Float16*, size_t, size_t);
#endif
#if MUU_HAS_FP16
	MUU_API
	void MUU_CALLCONV print_matrix(std::ostream& stream, const __fp16*, size_t, size_t);
#endif
#if MUU_HAS_FLOAT128
	MUU_API
	void MUU_CALLCONV print_matrix(std::ostream& stream, const float128_t*, size_t, size_t);
#endif
#if MUU_HAS_INT128
	MUU_API
	void MUU_CALLCONV print_matrix(std::ostream& stream, const int128_t*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::ostream& stream, const uint128_t*, size_t, size_t);
#endif

	MUU_API
	void MUU_CALLCONV print_matrix(std::wostream& stream, const half*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::wostream& stream, const float*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::wostream& stream, const double*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::wostream& stream, const long double*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::wostream& stream, const signed char*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::wostream& stream, const signed short*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::wostream& stream, const signed int*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::wostream& stream, const signed long*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::wostream& stream, const signed long long*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::wostream& stream, const unsigned char*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::wostream& stream, const unsigned short*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::wostream& stream, const unsigned int*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::wostream& stream, const unsigned long*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::wostream& stream, const unsigned long long*, size_t, size_t);
#if MUU_HAS_FLOAT16
	MUU_API
	void MUU_CALLCONV print_matrix(std::wostream& stream, const _Float16*, size_t, size_t);
#endif
#if MUU_HAS_FP16
	MUU_API
	void MUU_CALLCONV print_matrix(std::wostream& stream, const __fp16*, size_t, size_t);
#endif
#if MUU_HAS_FLOAT128
	MUU_API
	void MUU_CALLCONV print_matrix(std::wostream& stream, const float128_t*, size_t, size_t);
#endif
#if MUU_HAS_INT128
	MUU_API
	void MUU_CALLCONV print_matrix(std::wostream& stream, const int128_t*, size_t, size_t);
	MUU_API
	void MUU_CALLCONV print_matrix(std::wostream& stream, const uint128_t*, size_t, size_t);
#endif

	template <typename T>
	struct compound_vector_elem
	{
		const T* start;
		size_t count;
	};

	MUU_API
	void MUU_CALLCONV print_compound_vector(std::ostream&, const compound_vector_elem<half>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::ostream&, const compound_vector_elem<float>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::ostream&, const compound_vector_elem<double>*, size_t);

#if MUU_HAS_FLOAT16
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::ostream&, const compound_vector_elem<_Float16>*, size_t);
#endif
#if MUU_HAS_FP16
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::ostream&, const compound_vector_elem<__fp16>*, size_t);
#endif
#if MUU_HAS_FLOAT128
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::ostream&, const compound_vector_elem<float128_t>*, size_t);
#endif

	MUU_API
	void MUU_CALLCONV print_compound_vector(std::ostream&, const compound_vector_elem<long double>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::ostream&, const compound_vector_elem<signed char>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::ostream&, const compound_vector_elem<signed short>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::ostream&, const compound_vector_elem<signed int>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::ostream&, const compound_vector_elem<signed long>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::ostream&, const compound_vector_elem<signed long long>*, size_t);

	MUU_API
	void MUU_CALLCONV print_compound_vector(std::ostream&, const compound_vector_elem<unsigned char>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::ostream&, const compound_vector_elem<unsigned short>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::ostream&, const compound_vector_elem<unsigned int>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::ostream&, const compound_vector_elem<unsigned long>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::ostream&, const compound_vector_elem<unsigned long long>*, size_t);

#if MUU_HAS_INT128
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::ostream&, const compound_vector_elem<int128_t>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::ostream&, const compound_vector_elem<uint128_t>*, size_t);
#endif

	MUU_API
	void MUU_CALLCONV print_compound_vector(std::wostream&, const compound_vector_elem<half>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::wostream&, const compound_vector_elem<float>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::wostream&, const compound_vector_elem<double>*, size_t);

#if MUU_HAS_FLOAT16
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::wostream&, const compound_vector_elem<_Float16>*, size_t);
#endif
#if MUU_HAS_FP16
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::wostream&, const compound_vector_elem<__fp16>*, size_t);
#endif
#if MUU_HAS_FLOAT128
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::wostream&, const compound_vector_elem<float128_t>*, size_t);
#endif

	MUU_API
	void MUU_CALLCONV print_compound_vector(std::wostream&, const compound_vector_elem<long double>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::wostream&, const compound_vector_elem<signed char>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::wostream&, const compound_vector_elem<signed short>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::wostream&, const compound_vector_elem<signed int>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::wostream&, const compound_vector_elem<signed long>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::wostream&, const compound_vector_elem<signed long long>*, size_t);

	MUU_API
	void MUU_CALLCONV print_compound_vector(std::wostream&, const compound_vector_elem<unsigned char>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::wostream&, const compound_vector_elem<unsigned short>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::wostream&, const compound_vector_elem<unsigned int>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::wostream&, const compound_vector_elem<unsigned long>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::wostream&, const compound_vector_elem<unsigned long long>*, size_t);

#if MUU_HAS_INT128
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::wostream&, const compound_vector_elem<int128_t>*, size_t);
	MUU_API
	void MUU_CALLCONV print_compound_vector(std::wostream&, const compound_vector_elem<uint128_t>*, size_t);
#endif

	template <typename Char, typename T, size_t N>
	inline void print_compound_vector(std::basic_ostream<Char>& os, const compound_vector_elem<T> (&elems)[N])
	{
		print_compound_vector(os, elems, N);
	}
}

#include "header_end.h"
/// \endcond
