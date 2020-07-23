// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "settings.h"
#define MUU_IMPLEMENTATION
#include "../include/muu/preprocessor.h"
#if !MUU_ALL_INLINE
	#include "../include/muu/all.h"
#endif

#if MUU_BIG_ENDIAN != SHOULD_BE_BIG_ENDIAN
	#error MUU_BIG_ENDIAN was not deduced correctly
#endif
#if MUU_HAS_INTERCHANGE_FP16 != SHOULD_HAVE_FP16
	#error MUU_HAS_INTERCHANGE_FP16 was not deduced correctly
#endif
#if MUU_HAS_FLOAT16 != SHOULD_HAVE_FLOAT16
	#error MUU_HAS_FLOAT16 was not deduced correctly
#endif
#if MUU_HAS_INT128 != SHOULD_HAVE_INT128
	#error MUU_HAS_INT128 was not deduced correctly
#endif

#define CATCH_CONFIG_RUNNER
#include "catch2.h"
#include <clocale>

int main(int argc, char* argv[])
{
	#ifdef _WIN32
		SetConsoleOutputCP(65001);
	#endif
	std::setlocale(LC_ALL, "");
	std::locale::global(std::locale(""));
	return Catch::Session().run(argc, argv);
}
