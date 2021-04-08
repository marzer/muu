// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "muu/preprocessor.h"

#if MUU_WINDOWS
	#include "os_windows.h"
#elif MUU_UNIX
	#include "os_unix.h"
#endif

#if defined(_MSC_VER) && !(defined(NDEBUG) ^ defined(_DEBUG))
	#error muu requires one of NDEBUG or _DEBUG to be defined when building with MSVC.
#endif
