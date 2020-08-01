// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/strings.h"

#define SV(v) MUU_APPEND_SV(v)

#define CHECK_FUNC(func, input, expected)														\
	CHECK_AND_STATIC_ASSERT(func(SV(input)) == SV(expected));									\
	CHECK_AND_STATIC_ASSERT(func(MUU_CONCAT(u8, SV(input))) == MUU_CONCAT(u8, SV(expected)));	\
	CHECK_AND_STATIC_ASSERT(func(MUU_CONCAT(u, SV(input)))  == MUU_CONCAT(u,  SV(expected)));	\
	CHECK_AND_STATIC_ASSERT(func(MUU_CONCAT(U, SV(input)))  == MUU_CONCAT(U,  SV(expected)));	\
	CHECK_AND_STATIC_ASSERT_W(func(MUU_CONCAT(L, SV(input)))  == MUU_CONCAT(L,  SV(expected)))

#define ALL_WS	"\u0009\u000A\u000B\u000C\u000D\u0020\u0085\u00A0\u1680\u2000\u2001\u2002\u3000"	\
				"\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200A\u2028\u2029\u202F\u205F"

#define UTF_TEST_TEXT_EXPECTED(...)																			\
                              MUU_CONCAT(__VA_ARGS__, R"(The quick brown fox jumped over the lazy dog)"	)	\
MUU_CONCAT(__VA_ARGS__, "\n")																				\
MUU_CONCAT(__VA_ARGS__, "\n") MUU_CONCAT(__VA_ARGS__, R"(ሰማይ አይታረስ ንጉሥ አይከሰስ።)"						)	\
MUU_CONCAT(__VA_ARGS__, "\n") MUU_CONCAT(__VA_ARGS__, R"(ብላ ካለኝ እንደአባቴ በቆመጠኝ።)"						)	\
MUU_CONCAT(__VA_ARGS__, "\n") MUU_CONCAT(__VA_ARGS__, R"(ጌጥ ያለቤቱ ቁምጥና ነው።)"							)	\
MUU_CONCAT(__VA_ARGS__, "\n") MUU_CONCAT(__VA_ARGS__, R"(ደሀ በሕልሙ ቅቤ ባይጠጣ ንጣት በገደለው።)"				)	\
MUU_CONCAT(__VA_ARGS__, "\n") MUU_CONCAT(__VA_ARGS__, R"(የአፍ ወለምታ በቅቤ አይታሽም።)"						)	\
MUU_CONCAT(__VA_ARGS__, "\n") MUU_CONCAT(__VA_ARGS__, R"(አይጥ በበላ ዳዋ ተመታ።)"							)	\
MUU_CONCAT(__VA_ARGS__, "\n") MUU_CONCAT(__VA_ARGS__, R"(ሲተረጉሙ ይደረግሙ።)"								)	\
MUU_CONCAT(__VA_ARGS__, "\n") MUU_CONCAT(__VA_ARGS__, R"(ቀስ በቀስ፥ ዕንቁላል በእግሩ ይሄዳል።)"					)	\
MUU_CONCAT(__VA_ARGS__, "\n") MUU_CONCAT(__VA_ARGS__, R"(ድር ቢያብር አንበሳ ያስር።)"							)	\
MUU_CONCAT(__VA_ARGS__, "\n") MUU_CONCAT(__VA_ARGS__, R"(ሰው እንደቤቱ እንጅ እንደ ጉረቤቱ አይተዳደርም።)"			)	\
MUU_CONCAT(__VA_ARGS__, "\n") MUU_CONCAT(__VA_ARGS__, R"(እግዜር የከፈተውን ጉሮሮ ሳይዘጋው አይድርም።)"			)	\
MUU_CONCAT(__VA_ARGS__, "\n") MUU_CONCAT(__VA_ARGS__, R"(የጎረቤት ሌባ፥ ቢያዩት ይስቅ ባያዩት ያጠልቅ።)"				)	\
MUU_CONCAT(__VA_ARGS__, "\n") MUU_CONCAT(__VA_ARGS__, R"(ሥራ ከመፍታት ልጄን ላፋታት።)"						)	\
MUU_CONCAT(__VA_ARGS__, "\n") MUU_CONCAT(__VA_ARGS__, R"(ዓባይ ማደሪያ የለው፥ ግንድ ይዞ ይዞራል።)"				)	\
MUU_CONCAT(__VA_ARGS__, "\n") MUU_CONCAT(__VA_ARGS__, R"(የእስላም አገሩ መካ የአሞራ አገሩ ዋርካ።)"					)	\
MUU_CONCAT(__VA_ARGS__, "\n") MUU_CONCAT(__VA_ARGS__, R"(ተንጋሎ ቢተፉ ተመልሶ ባፉ።)"							)	\
MUU_CONCAT(__VA_ARGS__, "\n") MUU_CONCAT(__VA_ARGS__, R"(ወዳጅህ ማር ቢሆን ጨርስህ አትላሰው።)"					)	\
MUU_CONCAT(__VA_ARGS__, "\n") MUU_CONCAT(__VA_ARGS__, R"(እግርህን በፍራሽህ ልክ ዘርጋ።)"							)	\
MUU_CONCAT(__VA_ARGS__, SV("\n"))

TEST_CASE("strings - utf_decode")
{
	constexpr auto test = [](const char* path, std::string_view code_unit_name, auto code_unit_val) noexcept
	{
		(void)code_unit_val;
		using code_unit = remove_cvref<decltype(code_unit_val)>;
		using string_view = std::basic_string_view<code_unit>;

		INFO("Decoding " << path << " as UTF-" << sizeof(code_unit) * CHAR_BIT << " (" << code_unit_name << ")")

		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file)
			FAIL("Couldn't open stream");

		std::streamsize size = file.tellg();
		if (size <= 0)
			FAIL("Couldn't determine file size");
		if (static_cast<size_t>(size) % sizeof(code_unit) != 0u)
			FAIL("File size was not a multiple of code unit size (" << size << " % " << sizeof(code_unit) << " != 0)");

		file.seekg(0, std::ios::beg);
		std::vector<std::byte> input_buffer(static_cast<size_t>(size));
		if (!file.read(pointer_cast<char*>(input_buffer.data()), size))
			FAIL("Reading failed");
		const auto input = string_view{ pointer_cast<const code_unit*>(input_buffer.data()), input_buffer.size() / sizeof(code_unit) };

		std::vector<char32_t> output_buffer;
		impl::utf_decode(input, [&](char32_t cp) noexcept
		{
			output_buffer.push_back(cp);
		});
		const auto output = std::u32string_view{ output_buffer.data(), output_buffer.size() };
		CHECK(output == UTF_TEST_TEXT_EXPECTED(U));

	};

	#define CHECK_DECODER(file, type) \
		test(file, MUU_MAKE_STRING_VIEW(type), type{})

	CHECK_DECODER("data/unicode_test_file_utf8.bin",			char);
	CHECK_DECODER("data/unicode_test_file_utf8_bom.bin",		char);
	CHECK_DECODER("data/unicode_test_file_utf16be.bin",			char16_t);
	CHECK_DECODER("data/unicode_test_file_utf16be_bom.bin",		char16_t);
	CHECK_DECODER("data/unicode_test_file_utf16le.bin",			char16_t);
	CHECK_DECODER("data/unicode_test_file_utf16le_bom.bin",		char16_t);
	CHECK_DECODER("data/unicode_test_file_utf32be.bin",			char32_t);
	CHECK_DECODER("data/unicode_test_file_utf32be_bom.bin",		char32_t);
	CHECK_DECODER("data/unicode_test_file_utf32le.bin",			char32_t);
	CHECK_DECODER("data/unicode_test_file_utf32le_bom.bin",		char32_t);
	#ifdef __cpp_lib_char8_t
	CHECK_DECODER("data/unicode_test_file_utf8.bin",			char8_t);
	CHECK_DECODER("data/unicode_test_file_utf8_bom.bin",		char8_t);
	#endif
	#if MUU_WCHAR_BYTES > 1
	CHECK_DECODER("data/unicode_test_file_utf" MUU_MAKE_STRING(MUU_WCHAR_BITS) "be.bin",		wchar_t);
	CHECK_DECODER("data/unicode_test_file_utf" MUU_MAKE_STRING(MUU_WCHAR_BITS) "be_bom.bin",	wchar_t);
	CHECK_DECODER("data/unicode_test_file_utf" MUU_MAKE_STRING(MUU_WCHAR_BITS) "le.bin",		wchar_t);
	CHECK_DECODER("data/unicode_test_file_utf" MUU_MAKE_STRING(MUU_WCHAR_BITS) "le_bom.bin",	wchar_t);
	#else
	CHECK_DECODER("data/unicode_test_file_utf8.bin",			wchar_t);
	CHECK_DECODER("data/unicode_test_file_utf8_bom.bin",		wchar_t);
	#endif
}

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

