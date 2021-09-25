// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
// clang-format off
#pragma once
#ifndef MUU_STD_STRING_INCLUDED
#define MUU_STD_STRING_INCLUDED

#include "../preprocessor.h"

MUU_DISABLE_WARNINGS;
#include <string>
MUU_ENABLE_WARNINGS;

#if !defined(MUU_HAS_CHAR8_STRINGS)
	#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201907
		#define MUU_HAS_CHAR8_STRINGS 1
	#else
		#define MUU_HAS_CHAR8_STRINGS 0
	#endif
#endif

#if defined(__cpp_lib_constexpr_string) && __cpp_lib_constexpr_string >= 201907
	#define MUU_CONSTEXPR_STD_STRING constexpr
#else
	#define MUU_CONSTEXPR_STD_STRING inline
#endif

#include "type_name_specializations.h"

#endif // MUU_STD_STRING_INCLUDED
// clang-format on
