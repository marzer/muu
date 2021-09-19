// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

#include "../preprocessor.h"

#if MUU_CLANG >= 8 || MUU_GCC >= 7 || MUU_ICC >= 1910 || MUU_MSVC >= 1914 || MUU_HAS_BUILTIN(launder)
	#define MUU_LAUNDER(...) __builtin_launder(__VA_ARGS__)
#endif

MUU_DISABLE_WARNINGS;
#ifndef MUU_LAUNDER
	#include "std_new.h"
	#if defined(__cpp_lib_launder) && __cpp_lib_launder >= 201606
		#define MUU_LAUNDER(...) std::launder(__VA_ARGS__)
	#endif
#endif
MUU_ENABLE_WARNINGS;

#ifndef MUU_LAUNDER
	#define MUU_LAUNDER(...) __VA_ARGS__
#endif
