// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/hashing.h"

TEST_CASE("hashing - sha1")
{
	static constexpr auto sha1_test = [](std::string_view input, std::string_view expected) noexcept
	{
		sha1 hasher;
		hasher(input);
		hasher.finish();
		std::ostringstream oss;
		oss << hasher;
		CHECK(oss.str() == expected);
	};

	sha1_test("The quick brown fox jumps over the lazy dog"sv, "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12"sv);
}

