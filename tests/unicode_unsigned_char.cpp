// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
//-----
// this file was generated by generate_unicode_functions.py - do not modify it directly
// clang-format off

#include "tests.h"
#include "unicode.h"
#include "../include/muu/strings.h"

TEST_CASE("unicode - is_ascii_code_point (unsigned char)")
{
	static constexpr auto fn = static_cast<code_unit_func<unsigned char>*>(is_ascii_code_point);
 
	// values which should return true
	static constexpr code_unit_range<uint32_t> true_ranges[] = 
	{
		{ 0x00u, 0x7Fu },
	};
	for (const auto& r : true_ranges)
	{
		REQUIRE(in(fn, r));
		REQUIRE(in_only<1>(fn, r));
	}
 
	// values which should return false
	static constexpr code_unit_range<uint32_t> false_ranges[] = 
	{
		{ 0x80u, 0xFFu },
	};
	for (const auto& r : false_ranges)
		REQUIRE(not_in(fn, r));
}

TEST_CASE("unicode - is_ascii_hyphen (unsigned char)")
{
	static constexpr auto fn = static_cast<code_unit_func<unsigned char>*>(is_ascii_hyphen);
 
	// values which should return true
	static constexpr unsigned char true_values[] = 
	{
		0x2Du,
	};
	for (auto v : true_values)
	{
		REQUIRE(fn(v));
		REQUIRE(in_only<2>(fn, v));
	}
 
	// values which should return false
	static constexpr code_unit_range<uint32_t> false_ranges[] = 
	{
		{ 0x00u, 0x2Cu }, { 0x2Eu, 0xFFu },
	};
	for (const auto& r : false_ranges)
		REQUIRE(not_in(fn, r));
}

TEST_CASE("unicode - is_ascii_letter (unsigned char)")
{
	static constexpr auto fn = static_cast<code_unit_func<unsigned char>*>(is_ascii_letter);
 
	// values which should return true
	static constexpr code_unit_range<uint32_t> true_ranges[] = 
	{
		{ 0x41u, 0x5Au }, { 0x61u, 0x7Au },
	};
	for (const auto& r : true_ranges)
	{
		REQUIRE(in(fn, r));
		REQUIRE(in_only<3>(fn, r));
	}
 
	// values which should return false
	static constexpr code_unit_range<uint32_t> false_ranges[] = 
	{
		{ 0x00u, 0x40u }, { 0x5Bu, 0x60u }, { 0x7Bu, 0xFFu },
	};
	for (const auto& r : false_ranges)
		REQUIRE(not_in(fn, r));
}

TEST_CASE("unicode - is_ascii_lowercase (unsigned char)")
{
	static constexpr auto fn = static_cast<code_unit_func<unsigned char>*>(is_ascii_lowercase);
 
	// values which should return true
	static constexpr code_unit_range<uint32_t> true_ranges[] = 
	{
		{ 0x61u, 0x7Au },
	};
	for (const auto& r : true_ranges)
	{
		REQUIRE(in(fn, r));
		REQUIRE(in_only<6>(fn, r));
	}
 
	// values which should return false
	static constexpr code_unit_range<uint32_t> false_ranges[] = 
	{
		{ 0x00u, 0x60u }, { 0x7Bu, 0xFFu },
	};
	for (const auto& r : false_ranges)
		REQUIRE(not_in(fn, r));
}

TEST_CASE("unicode - is_ascii_number (unsigned char)")
{
	static constexpr auto fn = static_cast<code_unit_func<unsigned char>*>(is_ascii_number);
 
	// values which should return true
	static constexpr code_unit_range<uint32_t> true_ranges[] = 
	{
		{ 0x30u, 0x39u },
	};
	for (const auto& r : true_ranges)
	{
		REQUIRE(in(fn, r));
		REQUIRE(in_only<4>(fn, r));
	}
 
	// values which should return false
	static constexpr code_unit_range<uint32_t> false_ranges[] = 
	{
		{ 0x00u, 0x2Fu }, { 0x3Au, 0xFFu },
	};
	for (const auto& r : false_ranges)
		REQUIRE(not_in(fn, r));
}

TEST_CASE("unicode - is_ascii_uppercase (unsigned char)")
{
	static constexpr auto fn = static_cast<code_unit_func<unsigned char>*>(is_ascii_uppercase);
 
	// values which should return true
	static constexpr code_unit_range<uint32_t> true_ranges[] = 
	{
		{ 0x41u, 0x5Au },
	};
	for (const auto& r : true_ranges)
	{
		REQUIRE(in(fn, r));
		REQUIRE(in_only<7>(fn, r));
	}
 
	// values which should return false
	static constexpr code_unit_range<uint32_t> false_ranges[] = 
	{
		{ 0x00u, 0x40u }, { 0x5Bu, 0xFFu },
	};
	for (const auto& r : false_ranges)
		REQUIRE(not_in(fn, r));
}

TEST_CASE("unicode - is_ascii_whitespace (unsigned char)")
{
	static constexpr auto fn = static_cast<code_unit_func<unsigned char>*>(is_ascii_whitespace);
 
	// values which should return true
	static constexpr code_unit_range<uint32_t> true_ranges[] = 
	{
		{ 0x09u, 0x0Du },
	};
	for (const auto& r : true_ranges)
	{
		REQUIRE(in(fn, r));
		REQUIRE(in_only<5>(fn, r));
	}
	static constexpr unsigned char true_values[] = 
	{
		0x20u,
	};
	for (auto v : true_values)
	{
		REQUIRE(fn(v));
		REQUIRE(in_only<5>(fn, v));
	}
 
	// values which should return false
	static constexpr code_unit_range<uint32_t> false_ranges[] = 
	{
		{ 0x00u, 0x08u }, { 0x0Eu, 0x1Fu }, { 0x21u, 0xFFu },
	};
	for (const auto& r : false_ranges)
		REQUIRE(not_in(fn, r));
}

TEST_CASE("unicode - is_non_ascii_code_point (unsigned char)")
{
	static constexpr auto fn = static_cast<code_unit_func<unsigned char>*>(is_non_ascii_code_point);
 
	// values which should return false
	static constexpr code_unit_range<uint32_t> false_ranges[] = 
	{
		{ 0x00u, 0xFFu },
	};
	for (const auto& r : false_ranges)
		REQUIRE(not_in(fn, r));
}

TEST_CASE("unicode - is_non_ascii_hyphen (unsigned char)")
{
	static constexpr auto fn = static_cast<code_unit_func<unsigned char>*>(is_non_ascii_hyphen);
 
	// values which should return false
	static constexpr code_unit_range<uint32_t> false_ranges[] = 
	{
		{ 0x00u, 0xFFu },
	};
	for (const auto& r : false_ranges)
		REQUIRE(not_in(fn, r));
}

TEST_CASE("unicode - is_non_ascii_letter (unsigned char)")
{
	static constexpr auto fn = static_cast<code_unit_func<unsigned char>*>(is_non_ascii_letter);
 
	// values which should return false
	static constexpr code_unit_range<uint32_t> false_ranges[] = 
	{
		{ 0x00u, 0xFFu },
	};
	for (const auto& r : false_ranges)
		REQUIRE(not_in(fn, r));
}

TEST_CASE("unicode - is_non_ascii_lowercase (unsigned char)")
{
	static constexpr auto fn = static_cast<code_unit_func<unsigned char>*>(is_non_ascii_lowercase);
 
	// values which should return false
	static constexpr code_unit_range<uint32_t> false_ranges[] = 
	{
		{ 0x00u, 0xFFu },
	};
	for (const auto& r : false_ranges)
		REQUIRE(not_in(fn, r));
}

TEST_CASE("unicode - is_non_ascii_number (unsigned char)")
{
	static constexpr auto fn = static_cast<code_unit_func<unsigned char>*>(is_non_ascii_number);
 
	// values which should return false
	static constexpr code_unit_range<uint32_t> false_ranges[] = 
	{
		{ 0x00u, 0xFFu },
	};
	for (const auto& r : false_ranges)
		REQUIRE(not_in(fn, r));
}

TEST_CASE("unicode - is_non_ascii_uppercase (unsigned char)")
{
	static constexpr auto fn = static_cast<code_unit_func<unsigned char>*>(is_non_ascii_uppercase);
 
	// values which should return false
	static constexpr code_unit_range<uint32_t> false_ranges[] = 
	{
		{ 0x00u, 0xFFu },
	};
	for (const auto& r : false_ranges)
		REQUIRE(not_in(fn, r));
}

TEST_CASE("unicode - is_non_ascii_whitespace (unsigned char)")
{
	static constexpr auto fn = static_cast<code_unit_func<unsigned char>*>(is_non_ascii_whitespace);
 
	// values which should return false
	static constexpr code_unit_range<uint32_t> false_ranges[] = 
	{
		{ 0x00u, 0xFFu },
	};
	for (const auto& r : false_ranges)
		REQUIRE(not_in(fn, r));
}

TEST_CASE("unicode - is_code_point (unsigned char)")
{
	static constexpr auto fn = static_cast<code_unit_func<unsigned char>*>(is_code_point);
 
	// values which should return true
	static constexpr code_unit_range<uint32_t> true_ranges[] = 
	{
		{ 0x00u, 0x7Fu },
	};
	for (const auto& r : true_ranges)
		REQUIRE(in(fn, r));
 
	// values which should return false
	static constexpr code_unit_range<uint32_t> false_ranges[] = 
	{
		{ 0x80u, 0xFFu },
	};
	for (const auto& r : false_ranges)
		REQUIRE(not_in(fn, r));
}

TEST_CASE("unicode - is_combining_mark (unsigned char)")
{
	static constexpr auto fn = static_cast<code_unit_func<unsigned char>*>(is_combining_mark);
 
	// values which should return false
	static constexpr code_unit_range<uint32_t> false_ranges[] = 
	{
		{ 0x00u, 0xFFu },
	};
	for (const auto& r : false_ranges)
		REQUIRE(not_in(fn, r));
}

TEST_CASE("unicode - is_decimal_digit (unsigned char)")
{
	static constexpr auto fn = static_cast<code_unit_func<unsigned char>*>(is_decimal_digit);
 
	// values which should return true
	static constexpr code_unit_range<uint32_t> true_ranges[] = 
	{
		{ 0x30u, 0x39u },
	};
	for (const auto& r : true_ranges)
		REQUIRE(in(fn, r));
 
	// values which should return false
	static constexpr code_unit_range<uint32_t> false_ranges[] = 
	{
		{ 0x00u, 0x2Fu }, { 0x3Au, 0xFFu },
	};
	for (const auto& r : false_ranges)
		REQUIRE(not_in(fn, r));
}

TEST_CASE("unicode - is_hexadecimal_digit (unsigned char)")
{
	static constexpr auto fn = static_cast<code_unit_func<unsigned char>*>(is_hexadecimal_digit);
 
	// values which should return true
	static constexpr code_unit_range<uint32_t> true_ranges[] = 
	{
		{ 0x30u, 0x39u }, { 0x41u, 0x46u }, { 0x61u, 0x66u },
	};
	for (const auto& r : true_ranges)
		REQUIRE(in(fn, r));
 
	// values which should return false
	static constexpr code_unit_range<uint32_t> false_ranges[] = 
	{
		{ 0x00u, 0x2Fu }, { 0x3Au, 0x40u }, { 0x47u, 0x60u },
		{ 0x67u, 0xFFu },
	};
	for (const auto& r : false_ranges)
		REQUIRE(not_in(fn, r));
}

TEST_CASE("unicode - is_octal_digit (unsigned char)")
{
	static constexpr auto fn = static_cast<code_unit_func<unsigned char>*>(is_octal_digit);
 
	// values which should return true
	static constexpr code_unit_range<uint32_t> true_ranges[] = 
	{
		{ 0x30u, 0x37u },
	};
	for (const auto& r : true_ranges)
		REQUIRE(in(fn, r));
 
	// values which should return false
	static constexpr code_unit_range<uint32_t> false_ranges[] = 
	{
		{ 0x00u, 0x2Fu }, { 0x38u, 0xFFu },
	};
	for (const auto& r : false_ranges)
		REQUIRE(not_in(fn, r));
}

template <>
struct code_unit_func_group<unsigned char, 1>
{
	static constexpr code_unit_func<unsigned char>* functions[] =
	{
		is_ascii_code_point,
		is_non_ascii_code_point,
		is_not_code_point,
	};
};

template <>
struct code_unit_func_group<unsigned char, 2>
{
	static constexpr code_unit_func<unsigned char>* functions[] =
	{
		is_ascii_hyphen,
		is_non_ascii_hyphen,
		is_not_hyphen,
	};
};

template <>
struct code_unit_func_group<unsigned char, 3>
{
	static constexpr code_unit_func<unsigned char>* functions[] =
	{
		is_ascii_letter,
		is_non_ascii_letter,
		is_not_letter,
	};
};

template <>
struct code_unit_func_group<unsigned char, 4>
{
	static constexpr code_unit_func<unsigned char>* functions[] =
	{
		is_ascii_number,
		is_non_ascii_number,
		is_not_number,
	};
};

template <>
struct code_unit_func_group<unsigned char, 5>
{
	static constexpr code_unit_func<unsigned char>* functions[] =
	{
		is_ascii_whitespace,
		is_non_ascii_whitespace,
		is_not_whitespace,
	};
};

template <>
struct code_unit_func_group<unsigned char, 6>
{
	static constexpr code_unit_func<unsigned char>* functions[] =
	{
		is_ascii_lowercase,
		is_non_ascii_lowercase,
		is_not_lowercase,
	};
};

template <>
struct code_unit_func_group<unsigned char, 7>
{
	static constexpr code_unit_func<unsigned char>* functions[] =
	{
		is_ascii_uppercase,
		is_non_ascii_uppercase,
		is_not_uppercase,
	};
};

// clang-format on

