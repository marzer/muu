// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/hashing.h"

template <size_t v>
using i = std::integral_constant<size_t, v>;

TEST_CASE("hashing - fnv1a")
{
	static constexpr auto hash = [](auto hash_size, auto input) constexpr noexcept
	{
		fnv1a<decltype(hash_size)::value> hasher;
		hasher(input);
		return hasher.value();
	};

	#define CHECK_FNV1A_STR(input, expected)				\
		CHECK_AND_STATIC_ASSERT(hash(i<sizeof(expected)* CHAR_BIT>{}, input) == expected)

	#define CHECK_FNV1A(input, expected)					\
		CHECK_FNV1A_STR(input, expected);					\
		CHECK_FNV1A_STR(MUU_CONCAT(u8, input), expected);	\
		CHECK_FNV1A_STR(MUU_CONCAT(L, input), expected)

	CHECK_FNV1A("The quick brown fox jumps over the lazy dog"sv, 0x048FFF90_u32);
	CHECK_FNV1A("The quick brown fox jumps over the lazy dog"sv, 0xF3F9B7F5E7E47110_u64);
	CHECK_FNV1A("ði ıntəˈnæʃənəl fəˈnɛtık əsoʊsiˈeıʃn"sv, 0xAFED7BC2_u32);
	CHECK_FNV1A("ði ıntəˈnæʃənəl fəˈnɛtık əsoʊsiˈeıʃn"sv, 0xF1C64B37A795CB62_u64);
}

TEST_CASE("hashing - sha1")
{
	static constexpr auto test = [](std::string_view input, std::string_view expected) noexcept
	{
		sha1 hasher;
		hasher(input);
		hasher.finish();
		std::ostringstream oss;
		oss << hasher;
		CHECK(oss.str() == expected);
	};

	test("The quick brown fox jumps over the lazy dog"sv, "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12"sv);
}

