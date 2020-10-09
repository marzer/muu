// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"

#if MUU_HAS_FLOAT16
std::ostream& operator << (std::ostream& os, _Float16 flt)
{
	return os << static_cast<float>(flt);
}

std::wostream& operator << (std::wostream& os, _Float16 flt)
{
	return os << static_cast<float>(flt);
}
#endif
#if MUU_HAS_FLOAT128
std::ostream& operator << (std::ostream& os, __float128 flt)
{
	return os << static_cast<long double>(flt);
}

std::wostream& operator << (std::wostream& os, __float128 flt)
{
	return os << static_cast<long double>(flt);
}
#endif
