// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/hashing.h"

// expected hashes in this file were generated using https://md5calc.com/hash

#define CHECK_HASH(func, input, expected)		\
	func(input, expected);					\
	func(MUU_CONCAT(u8, input), expected);	\
	func(MUU_CONCAT(u, input), expected);	\
	func(MUU_CONCAT(U, input), expected);	\
	func(MUU_CONCAT(L, input), expected)


TEST_CASE("hashing - fnv1a")
{
	static constexpr auto hash = [](auto input, auto hash_size) constexpr noexcept
	{
		fnv1a<decltype(hash_size)::value> hasher;
		hasher(input);
		return hasher.value();
	};

	#define FNV1A(input, expected)				\
		CHECK_AND_STATIC_ASSERT(hash(input, std::integral_constant<size_t, sizeof(expected) * CHAR_BIT>{}) == expected)

	CHECK_HASH(FNV1A, "The quick brown fox jumps over the lazy dog"sv, 0x048FFF90_u32);
	CHECK_HASH(FNV1A, "The quick brown fox jumps over the lazy dog"sv, 0xF3F9B7F5E7E47110_u64);
	CHECK_HASH(FNV1A, "ði ıntəˈnæʃənəl fəˈnɛtık əsoʊsiˈeıʃn"sv, 0xAFED7BC2_u32);
	CHECK_HASH(FNV1A, "ði ıntəˈnæʃənəl fəˈnɛtık əsoʊsiˈeıʃn"sv, 0xF1C64B37A795CB62_u64);
}

TEST_CASE("hashing - sha1")
{
	static constexpr auto hash = [](auto input) noexcept
	{
		sha1 hasher;
		hasher(input);
		hasher.finish();
		std::ostringstream oss;
		oss << hasher;
		return oss.str();
	};

	#define SHA1(input, expected)	\
		 CHECK(hash(input) == expected)

	CHECK_HASH(SHA1, "The quick brown fox jumps over the lazy dog"sv, "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12"sv);
	CHECK_HASH(SHA1, "ði ıntəˈnæʃənəl fəˈnɛtık əsoʊsiˈeıʃn"sv, "e3f4f6fb8f7d479fa91e5fa238bcd18e2083c2bd"sv);
	
}

