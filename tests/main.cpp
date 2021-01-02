// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "settings.h"
#include "../include/muu/preprocessor.h"

MUU_PRAGMA_GCC(diagnostic ignored "-Wpadded")

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
