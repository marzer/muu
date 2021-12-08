// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

// this file does not contain header guards; this is intentional

#include "../preprocessor.h"

MUU_PRAGMA_GCC(pop_options)

#if MUU_MSVC || MUU_ICC_CL
	#pragma pop_macro("min")
	#pragma pop_macro("max")
	#pragma float_control(pop)
	#pragma inline_recursion(off)
#endif

MUU_POP_WARNINGS;
