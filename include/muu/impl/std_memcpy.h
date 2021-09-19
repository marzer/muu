// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

#include "../preprocessor.h"

#if MUU_GCC || MUU_CLANG || MUU_HAS_BUILTIN(__builtin_memcpy)
	#define MUU_MEMCPY(to, from, size) __builtin_memcpy(to, from, size)
#endif

#if MUU_GCC || MUU_CLANG || MUU_HAS_BUILTIN(__builtin_memmove)
	#define MUU_MEMMOVE(to, from, size) __builtin_memmove(to, from, size)
#endif

#if MUU_GCC || MUU_CLANG || MUU_HAS_BUILTIN(__builtin_memcmp)
	#define MUU_MEMCMP(a, b, size) __builtin_memcmp(a, b, size)
#endif

#if MUU_GCC || MUU_CLANG || MUU_HAS_BUILTIN(__builtin_memset)
	#define MUU_MEMSET(buf, val, size) __builtin_memset(buf, val, size)
#endif

MUU_DISABLE_WARNINGS;
#if !defined(MUU_MEMCPY) || !defined(MUU_MEMMOVE) || !defined(MUU_MEMCMP) || !defined(MUU_MEMSET)
	#include <cstring>
#endif
MUU_ENABLE_WARNINGS;

#ifndef MUU_MEMCPY
	#define MUU_MEMCPY(to, from, size) std::memcpy(to, from, size)
#endif

#ifndef MUU_MEMMOVE
	#define MUU_MEMMOVE(to, from, size) std::memmove(to, from, size)
#endif

#ifndef MUU_MEMCMP
	#define MUU_MEMCMP(a, b, size) std::memcmp(a, b, size)
#endif

#ifndef MUU_MEMSET
	#define MUU_MEMSET(buf, val, size) std::memset(buf, val, size)
#endif
