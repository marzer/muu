// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include <iostream>
#include <string_view>
#include "utf8_console.h"

#include <muu/hashing.h>

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

	return 0;
}
