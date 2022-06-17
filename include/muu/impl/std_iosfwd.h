// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#ifndef MUU_STD_IOSFWD_H
#define MUU_STD_IOSFWD_H

#include "../preprocessor.h"

MUU_DISABLE_WARNINGS;
#include <iosfwd>
MUU_ENABLE_WARNINGS;

#if MUU_DOXYGEN || (defined(__cpp_lib_syncbuf) && __cpp_lib_syncbuf >= 201803)
	#define MUU_HAS_SYNCSTREAMS 1
#else
	#define MUU_HAS_SYNCSTREAMS 0
#endif

#if MUU_DOXYGEN || (defined(__cpp_lib_spanstream) && __cpp_lib_spanstream >= 202106)
	#define MUU_HAS_SPANSTREAMS 1
#else
	#define MUU_HAS_SPANSTREAMS 0
#endif

#endif // MUU_STD_IOSFWD_H
