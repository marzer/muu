// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"

namespace std
{
#if MUU_HAS_FLOAT16
	ostream& operator<<(ostream& os, _Float16 flt)
	{
		return os << static_cast<float>(flt);
	}
#endif

#if MUU_HAS_FLOAT128
	ostream& operator<<(ostream& os, __float128 flt)
	{
	#if MUU_HAS_QUADMATH
		{
			char buf[256];
			int n = quadmath_snprintf(buf, sizeof(buf), "%." MUU_MAKE_STRING(__FLT128_DIG__) "Qf", flt);
			if (n <= 0 || n >= 256)
			{
				return os << "[quadmath_snprintf() returned unexpected value: "sv << n << "]"sv;
			}
			else
				return os << std::string_view{ buf, static_cast<size_t>(n) };
		}
	#else
		return os << static_cast<long double>(flt);
	#endif
	}
#endif

#if MUU_HAS_INT128
	ostream& operator<<(ostream& os, __int128_t val)
	{
		if (val < constants<int64_t>::lowest || val > constants<int64_t>::highest)
		{
			return os << "<HUGE>"sv;
		}
		else
			return os << static_cast<int64_t>(val);
	}

	ostream& operator<<(ostream& os, __uint128_t val)
	{
		if (val > constants<uint64_t>::highest)
		{
			return os << "<HUGE>"sv;
		}
		else
			return os << static_cast<uint64_t>(val);
	}
#endif
}

namespace muu
{
#define EXTERN_ARITHMETIC_TEMPLATES(T)                                                                                 \
	template T random() noexcept;                                                                                      \
	template T random(T) noexcept;                                                                                     \
	template T random(T, T) noexcept;                                                                                  \
	template struct print_aligned<T>;

	MUU_FOR_EACH(EXTERN_ARITHMETIC_TEMPLATES, ALL_ARITHMETIC);

#define EXTERN_FLOAT_TEMPLATES(T)                                                                                      \
	template Approx approx(T, T) noexcept;                                                                             \
	template Approx approx(T) noexcept;

	MUU_FOR_EACH(EXTERN_FLOAT_TEMPLATES, ALL_FLOATS);
}
