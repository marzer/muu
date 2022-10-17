// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#ifdef _MSC_VER
	#pragma warning(disable : 5262)
	#pragma warning(disable : 5264)
#endif

#include "settings.h"
#include "../include/muu/preprocessor.h"

MUU_PRAGMA_GCC(diagnostic ignored "-Wpadded")

#define CATCH_CONFIG_RUNNER
#include "catch2.h"
#include <clocale>

#ifdef _WIN32
	#define MAIN_CALLCONV __cdecl
#else
	#define MAIN_CALLCONV
#endif

int MAIN_CALLCONV main(int argc, char* argv[])
{
#ifdef _WIN32
	SetConsoleOutputCP(65001);
#endif
	std::setlocale(LC_ALL, "");
	std::locale::global(std::locale(""));
	srand(static_cast<unsigned>(time(nullptr)));
	return Catch::Session().run(argc, argv);
}
