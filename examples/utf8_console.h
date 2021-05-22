// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include <muu/preprocessor.h>
MUU_DISABLE_WARNINGS;
#include <iostream>
#include <iomanip>
#ifdef _WIN32
	#include <Windows.h>
#endif
MUU_ENABLE_WARNINGS;

inline void init_utf8_console(bool sync_with_stdio = false) noexcept
{
	std::ios_base::sync_with_stdio(sync_with_stdio);

#ifdef _WIN32
	SetConsoleOutputCP(65001); // CP_UTF8
#endif

	std::cout << std::boolalpha;
}
