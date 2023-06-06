// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the definition of #MUU_PAUSE.

#include "preprocessor.h"
MUU_DISABLE_WARNINGS;

//% preprocessor::pause start

// msvc-specific
#if !defined(MUU_PAUSE) && MUU_MSVC
	#if MUU_ARCH_X86 || MUU_ARCH_AMD64
		#define MUU_PAUSE() _mm_pause()
	#elif MUU_ARCH_ARM
		#define MUU_PAUSE() __yield()
	#endif
#endif

// __builtin_ia32_pause on GCC+clang
#if !defined(MUU_PAUSE) && (MUU_ARCH_X86 || MUU_ARCH_AMD64)                                                            \
	&& (MUU_CLANG || MUU_GCC || MUU_HAS_BUILTIN(__builtin_ia32_pause))
	#define MUU_PAUSE() __builtin_ia32_pause()
#endif

// YieldProcessor() on windows if available
#if !defined(MUU_PAUSE) && MUU_WINDOWS && defined(YieldProcessor)
	#define MUU_PAUSE() YieldProcessor()
#endif

// x86 fallback
#if !defined(MUU_PAUSE) && (MUU_ARCH_X86 || MUU_ARCH_AMD64)
	#include <immintrin.h>
	#define MUU_PAUSE() _mm_pause()
#endif

// ARM fallback
#if !defined(MUU_PAUSE) && MUU_ARCH_ARM
	#define MUU_PAUSE() __yield()
#endif

// no-op
#if !defined(MUU_PAUSE)
	#define MUU_PAUSE() static_cast<void>(0)
#endif

/// \def MUU_PAUSE()
/// \ingroup preprocessor
/// \brief Expands to a platform-specific intrinsic for pausing/yielding the CPU (e.g. `_mm_pause()`).

//% preprocessor::pause end

MUU_ENABLE_WARNINGS;
