// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include <iostream>
#include <string_view>
#include "utf8_console.h"

#include <muu/hashing.h>
#include <muu/matrix.h>

using namespace std::string_view_literals;

int main(int /*argc*/, char** /*argv*/)
{
	std::ios_base::sync_with_stdio(false);
	init_utf8_console();

	{
		std::cout << "expected: 2fd4e1c67a2d28fced849ee1bb76e7391b93eb12"sv << std::endl;
		muu::sha1 hasher;
		hasher("The quick brown fox jumps over the lazy dog"sv);
		hasher.finish();
		std::cout << "  actual: "sv << hasher << std::endl;
	}

	{
		std::cout << "expected: 048fff90"sv << std::endl;
		muu::fnv1a<32> hasher;
		hasher("The quick brown fox jumps over the lazy dog"sv);
		std::cout << "  actual: "sv << hasher << std::endl;
	}

	{
		std::cout << "expected: f3f9b7f5e7e47110"sv << std::endl;
		muu::fnv1a<64> hasher;
		hasher("The quick brown fox jumps over the lazy dog"sv);
		std::cout << "  actual: "sv << hasher << std::endl;
	}

	#if !MUU_ICC
	{
		std::cout << "expected: f1c64b37a795cb62"sv << std::endl;
		muu::fnv1a<64> hasher;
		hasher("ði ıntəˈnæʃənəl fəˈnɛtık əsoʊsiˈeıʃn"sv);
		std::cout << "  actual: "sv << hasher << std::endl;
	}

	{
		std::cout << "expected: f1c64b37a795cb62"sv << std::endl;
		muu::fnv1a<64> hasher;
		hasher(L"ði ıntəˈnæʃənəl fəˈnɛtık əsoʊsiˈeıʃn"sv);
		std::cout << "  actual: "sv << hasher << std::endl;
	}
	#endif

	// explicitly-sized matrices:
	std::cout << std::setw(4) << muu::matrix<int, 2, 3>{ 1, 2, 3, 4, 5, 6 } << "\n";

	// 2x2, 3x3 and 4x4 matrices can be deduced automatically from 4, 9 and 16 inputs:
	std::cout << std::setw(4) << muu::matrix{ 1, 2, 3, 4 } << "\n";
	std::cout << std::setw(4) << muu::matrix{ 1, 2, 3, 4, 5, 6, 7, 8, 9 } << "\n";
	std::cout << std::setw(4) << muu::matrix{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } << "\n";

	return 0;
}
