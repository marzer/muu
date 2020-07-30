// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/strings.h"

#define APPEND_SV(v) MUU_CONCAT(v, sv)

#if !MUU_CLANG || MUU_CLANG > 8
	#define CHECK_AND_STATIC_ASSERT_W(x) CHECK_AND_STATIC_ASSERT(x)
#else
	#define CHECK_AND_STATIC_ASSERT_W(x) CHECK(x)
#endif

#define CHECK_FUNC(func, input, expected)																	\
	CHECK_AND_STATIC_ASSERT(func(APPEND_SV(input)) == APPEND_SV(expected));									\
	CHECK_AND_STATIC_ASSERT(func(MUU_CONCAT(u8, APPEND_SV(input))) == MUU_CONCAT(u8, APPEND_SV(expected)));	\
	CHECK_AND_STATIC_ASSERT(func(MUU_CONCAT(u, APPEND_SV(input)))  == MUU_CONCAT(u,  APPEND_SV(expected)));	\
	CHECK_AND_STATIC_ASSERT(func(MUU_CONCAT(U, APPEND_SV(input)))  == MUU_CONCAT(U,  APPEND_SV(expected)));	\
	CHECK_AND_STATIC_ASSERT_W(func(MUU_CONCAT(L, APPEND_SV(input)))  == MUU_CONCAT(L,  APPEND_SV(expected)))

#define ALL_WS	"\u0009\u000A\u000B\u000C\u000D\u0020\u0085\u00A0\u1680\u2000\u2001\u2002\u3000"	\
				"\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200A\u2028\u2029\u202F\u205F"


TEST_CASE("strings - utf_find")
{
	auto kek = impl::utf_find("abcde"sv, false, [](char32_t c)noexcept { return c == U'b'; });
	CHECK(kek);
	CHECK(kek.index == 1);
	CHECK(kek.length == 1);

	kek = impl::utf_find("abcde"sv, false, [](char32_t c)noexcept { return c == U'd'; });
	CHECK(kek);
	CHECK(kek.index == 3);
	CHECK(kek.length == 1);

	kek = impl::utf_find("abcde"sv, true, [](char32_t c)noexcept { return c == U'b'; });
	CHECK(kek);
	CHECK(kek.index == 1);
	CHECK(kek.length == 1);

	kek = impl::utf_find("abcde"sv, true, [](char32_t c)noexcept { return c == U'd'; });
	CHECK(kek);
	CHECK(kek.index == 3);
	CHECK(kek.length == 1);
}

TEST_CASE("strings - trim")
{
	CHECK_FUNC(trim, "",						"");
	CHECK_FUNC(trim, " ",						"");
	CHECK_FUNC(trim, " \t",						"");
	CHECK_FUNC(trim, " \t       ",				"");
	CHECK_FUNC(trim, "test",					"test");
	CHECK_FUNC(trim, " test ",					"test");
	CHECK_FUNC(trim, "\ttest\t",				"test");
	CHECK_FUNC(trim, "\t test \t",				"test");
	CHECK_FUNC(trim, "\u0009 test \u0009",		"test");
	CHECK_FUNC(trim, "\u000A test \u000A",		"test");
	CHECK_FUNC(trim, "\u000B test \u000B",		"test");
	CHECK_FUNC(trim, "\u000C test \u000C",		"test");
	CHECK_FUNC(trim, "\u000D test \u000D",		"test");
	CHECK_FUNC(trim, "\u0020 test \u0020",		"test");
	CHECK_FUNC(trim, "\u0085 test \u0085",		"test");
	CHECK_FUNC(trim, "\u00A0 test \u00A0",		"test");
	CHECK_FUNC(trim, "\u1680 test \u1680",		"test");
	CHECK_FUNC(trim, "\u2000 test \u2000",		"test");
	CHECK_FUNC(trim, "\u2001 test \u2001",		"test");
	CHECK_FUNC(trim, "\u2002 test \u2002",		"test");
	CHECK_FUNC(trim, "\u2003 test \u2003",		"test");
	CHECK_FUNC(trim, "\u2004 test \u2004",		"test");
	CHECK_FUNC(trim, "\u2005 test \u2005",		"test");
	CHECK_FUNC(trim, "\u2006 test \u2006",		"test");
	CHECK_FUNC(trim, "\u2007 test \u2007",		"test");
	CHECK_FUNC(trim, "\u2008 test \u2008",		"test");
	CHECK_FUNC(trim, "\u2009 test \u2009",		"test");
	CHECK_FUNC(trim, "\u200A test \u200A",		"test");
	CHECK_FUNC(trim, "\u2028 test \u2028",		"test");
	CHECK_FUNC(trim, "\u2029 test \u2029",		"test");
	CHECK_FUNC(trim, "\u202F test \u202F",		"test");
	CHECK_FUNC(trim, "\u205F test \u205F",		"test");
	CHECK_FUNC(trim, "\u3000 test \u3000",		"test");
	CHECK_FUNC(trim, ALL_WS "test" ALL_WS,		"test");
}

TEST_CASE("strings - trim_left")
{
	CHECK_FUNC(trim_left, "",						"");
	CHECK_FUNC(trim_left, " ",						"");
	CHECK_FUNC(trim_left, " \t",					"");
	CHECK_FUNC(trim_left, " \t       ",				"");
	CHECK_FUNC(trim_left, "test",					"test");
	CHECK_FUNC(trim_left, " test ",					"test ");
	CHECK_FUNC(trim_left, "\ttest\t",				"test\t");
	CHECK_FUNC(trim_left, "\t test \t",				"test \t");
	CHECK_FUNC(trim_left, "\u0009 test \u0009",		"test \u0009");
	CHECK_FUNC(trim_left, "\u000A test \u000A",		"test \u000A");
	CHECK_FUNC(trim_left, "\u000B test \u000B",		"test \u000B");
	CHECK_FUNC(trim_left, "\u000C test \u000C",		"test \u000C");
	CHECK_FUNC(trim_left, "\u000D test \u000D",		"test \u000D");
	CHECK_FUNC(trim_left, "\u0020 test \u0020",		"test \u0020");
	CHECK_FUNC(trim_left, "\u0085 test \u0085",		"test \u0085");
	CHECK_FUNC(trim_left, "\u00A0 test \u00A0",		"test \u00A0");
	CHECK_FUNC(trim_left, "\u1680 test \u1680",		"test \u1680");
	CHECK_FUNC(trim_left, "\u2000 test \u2000",		"test \u2000");
	CHECK_FUNC(trim_left, "\u2001 test \u2001",		"test \u2001");
	CHECK_FUNC(trim_left, "\u2002 test \u2002",		"test \u2002");
	CHECK_FUNC(trim_left, "\u2003 test \u2003",		"test \u2003");
	CHECK_FUNC(trim_left, "\u2004 test \u2004",		"test \u2004");
	CHECK_FUNC(trim_left, "\u2005 test \u2005",		"test \u2005");
	CHECK_FUNC(trim_left, "\u2006 test \u2006",		"test \u2006");
	CHECK_FUNC(trim_left, "\u2007 test \u2007",		"test \u2007");
	CHECK_FUNC(trim_left, "\u2008 test \u2008",		"test \u2008");
	CHECK_FUNC(trim_left, "\u2009 test \u2009",		"test \u2009");
	CHECK_FUNC(trim_left, "\u200A test \u200A",		"test \u200A");
	CHECK_FUNC(trim_left, "\u2028 test \u2028",		"test \u2028");
	CHECK_FUNC(trim_left, "\u2029 test \u2029",		"test \u2029");
	CHECK_FUNC(trim_left, "\u202F test \u202F",		"test \u202F");
	CHECK_FUNC(trim_left, "\u205F test \u205F",		"test \u205F");
	CHECK_FUNC(trim_left, "\u3000 test \u3000",		"test \u3000");
	CHECK_FUNC(trim_left, ALL_WS "test" ALL_WS,		"test" ALL_WS);
}

TEST_CASE("strings - trim_right")
{
	CHECK_FUNC(trim_right, "",						"");
	CHECK_FUNC(trim_right, " ",						"");
	CHECK_FUNC(trim_right, " \t",					"");
	CHECK_FUNC(trim_right, " \t       ",			"");
	CHECK_FUNC(trim_right, "test",					"test");
	CHECK_FUNC(trim_right, " test ",				" test");
	CHECK_FUNC(trim_right, "\ttest\t",				"\ttest");
	CHECK_FUNC(trim_right, "\t test \t",			"\t test");
	CHECK_FUNC(trim_right, "\u0009 test \u0009",	"\u0009 test");
	CHECK_FUNC(trim_right, "\u000A test \u000A",	"\u000A test");
	CHECK_FUNC(trim_right, "\u000B test \u000B",	"\u000B test");
	CHECK_FUNC(trim_right, "\u000C test \u000C",	"\u000C test");
	CHECK_FUNC(trim_right, "\u000D test \u000D",	"\u000D test");
	CHECK_FUNC(trim_right, "\u0020 test \u0020",	"\u0020 test");
	CHECK_FUNC(trim_right, "\u0085 test \u0085",	"\u0085 test");
	CHECK_FUNC(trim_right, "\u00A0 test \u00A0",	"\u00A0 test");
	CHECK_FUNC(trim_right, "\u1680 test \u1680",	"\u1680 test");
	CHECK_FUNC(trim_right, "\u2000 test \u2000",	"\u2000 test");
	CHECK_FUNC(trim_right, "\u2001 test \u2001",	"\u2001 test");
	CHECK_FUNC(trim_right, "\u2002 test \u2002",	"\u2002 test");
	CHECK_FUNC(trim_right, "\u2003 test \u2003",	"\u2003 test");
	CHECK_FUNC(trim_right, "\u2004 test \u2004",	"\u2004 test");
	CHECK_FUNC(trim_right, "\u2005 test \u2005",	"\u2005 test");
	CHECK_FUNC(trim_right, "\u2006 test \u2006",	"\u2006 test");
	CHECK_FUNC(trim_right, "\u2007 test \u2007",	"\u2007 test");
	CHECK_FUNC(trim_right, "\u2008 test \u2008",	"\u2008 test");
	CHECK_FUNC(trim_right, "\u2009 test \u2009",	"\u2009 test");
	CHECK_FUNC(trim_right, "\u200A test \u200A",	"\u200A test");
	CHECK_FUNC(trim_right, "\u2028 test \u2028",	"\u2028 test");
	CHECK_FUNC(trim_right, "\u2029 test \u2029",	"\u2029 test");
	CHECK_FUNC(trim_right, "\u202F test \u202F",	"\u202F test");
	CHECK_FUNC(trim_right, "\u205F test \u205F",	"\u205F test");
	CHECK_FUNC(trim_right, "\u3000 test \u3000",	"\u3000 test");
	CHECK_FUNC(trim_right, ALL_WS "test" ALL_WS,	ALL_WS "test");
}

