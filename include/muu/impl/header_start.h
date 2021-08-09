// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

// this file does not contain header guards; this is intentional

#include "../preprocessor.h"

MUU_PUSH_WARNINGS;
MUU_DISABLE_SPAM_WARNINGS;

#if MUU_MSVC || MUU_ICC_CL
	#pragma inline_recursion(on)
	#pragma float_control(push)
	#pragma push_macro("min")
	#pragma push_macro("max")
	#undef min
	#undef max
#endif

MUU_PRAGMA_GCC(push_options)

MUU_PRAGMA_CLANG_GE(13, float_control(push))
