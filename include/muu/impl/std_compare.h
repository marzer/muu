// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#ifndef MUU_STD_COMPARE_H
#define MUU_STD_COMPARE_H

#include "../preprocessor.h"

MUU_DISABLE_WARNINGS;
#if MUU_HAS_INCLUDE(<compare>) && defined(__cpp_lib_three_way_comparison) && __cpp_lib_three_way_comparison >= 201907

	#include <compare>

	#define MUU_HAS_THREE_WAY_COMPARISON 1

#else

	#define MUU_HAS_THREE_WAY_COMPARISON 0

#endif
MUU_ENABLE_WARNINGS;

#endif // MUU_STD_COMPARE_H
