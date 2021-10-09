// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
// clang-format off
#pragma once
#ifndef MUU_STD_STRING_VIEW_INCLUDED
#define MUU_STD_STRING_VIEW_INCLUDED

#include "../preprocessor.h"

MUU_DISABLE_WARNINGS;
#include <string_view>
MUU_ENABLE_WARNINGS;

#if !defined(MUU_HAS_CHAR8_STRINGS)
	#if defined(DOXYGEN) || (defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201907)
		#define MUU_HAS_CHAR8_STRINGS 1
	#else
		#define MUU_HAS_CHAR8_STRINGS 0
	#endif
#endif

#endif // MUU_STD_STRING_VIEW_INCLUDED
// clang-format off
