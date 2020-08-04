// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/uuid.h"
#include <set>
#include <unordered_set>

static_assert(sizeof(uuid) == 16);
static_assert(std::is_standard_layout_v<uuid>);
static_assert(std::is_trivially_constructible_v<uuid>);
static_assert(std::is_trivially_copy_constructible_v<uuid>);
static_assert(std::is_trivially_copy_assignable_v<uuid>);
static_assert(std::is_trivially_move_constructible_v<uuid>);
static_assert(std::is_trivially_move_assignable_v<uuid>);
static_assert(std::is_trivially_destructible_v<uuid>);

TEST_CASE("uuid - initialization")
{
	//null uuids
	CHECK_AND_STATIC_ASSERT(!uuid( nullptr ));
	CHECK_AND_STATIC_ASSERT(!uuid(0, 0, 0, 0, 0, 0));
	CHECK_AND_STATIC_ASSERT(uuid(nullptr).variant() == uuid_variant::none);
	CHECK_AND_STATIC_ASSERT(uuid(nullptr).version() == uuid_version::none);
	CHECK_AND_STATIC_ASSERT(uuid( nullptr ) == uuid(0, 0, 0, 0, 0, 0));
	{
		const auto id = uuid{ nullptr };
		for (auto byte : id.bytes.value)
			CHECK(byte == 0x00_byte);
	}

	//explicit uuids
	{
		// 00112233-4455-6677-8899-aabbccddeeff
		auto id = uuid{ 0x00112233u, 0x4455_u16, 0x6677_u16, 0x88_u8, 0x99_u8, 0xAABBCCDDEEFF_u64 };
		CHECK(id.bytes.value[0]  == 0x00_byte);
		CHECK(id.bytes.value[1]  == 0x11_byte);
		CHECK(id.bytes.value[2]  == 0x22_byte);
		CHECK(id.bytes.value[3]  == 0x33_byte);
		CHECK(id.bytes.value[4]  == 0x44_byte);
		CHECK(id.bytes.value[5]  == 0x55_byte);
		CHECK(id.bytes.value[6]  == 0x66_byte);
		CHECK(id.bytes.value[7]  == 0x77_byte);
		CHECK(id.bytes.value[8]  == 0x88_byte);
		CHECK(id.bytes.value[9]  == 0x99_byte);
		CHECK(id.bytes.value[10] == 0xAA_byte);
		CHECK(id.bytes.value[11] == 0xBB_byte);
		CHECK(id.bytes.value[12] == 0xCC_byte);
		CHECK(id.bytes.value[13] == 0xDD_byte);
		CHECK(id.bytes.value[14] == 0xEE_byte);
		CHECK(id.bytes.value[15] == 0xFF_byte);
		CHECK(id.version() == uuid_version::unknown); // ((0x66 & 0b11110000) >> 4) == 0b0110 == unknown (>= 6)
		CHECK(id.variant() == uuid_variant::standard); // ((0x88 & 0b11100000) >> 5) == 0b100 == standard
		CHECK(id.time_low() == 0x00112233u);
		CHECK(id.time_mid() == 0x4455_u16);
		CHECK(id.time_high_and_version() == 0x6677_u16);
		CHECK(id.clock_seq_high_and_reserved() == 0x88_u8);
		CHECK(id.clock_seq_low() == 0x99_u8);
		CHECK(id.node() == 0xAABBCCDDEEFF_u64);

		// FFEEDDCC-BBAA-9988-7766-554433221100
		id = uuid{ 0xFFEEDDCCu, 0xBBAA_u16, 0x9988_u16, 0x7766_u16, 0x554433221100_u64 };
		CHECK(id.bytes.value[0]  == 0xFF_byte);
		CHECK(id.bytes.value[1]  == 0xEE_byte);
		CHECK(id.bytes.value[2]  == 0xDD_byte);
		CHECK(id.bytes.value[3]  == 0xCC_byte);
		CHECK(id.bytes.value[4]  == 0xBB_byte);
		CHECK(id.bytes.value[5]  == 0xAA_byte);
		CHECK(id.bytes.value[6]  == 0x99_byte);
		CHECK(id.bytes.value[7]  == 0x88_byte);
		CHECK(id.bytes.value[8]  == 0x77_byte);
		CHECK(id.bytes.value[9]  == 0x66_byte);
		CHECK(id.bytes.value[10] == 0x55_byte);
		CHECK(id.bytes.value[11] == 0x44_byte);
		CHECK(id.bytes.value[12] == 0x33_byte);
		CHECK(id.bytes.value[13] == 0x22_byte);
		CHECK(id.bytes.value[14] == 0x11_byte);
		CHECK(id.bytes.value[15] == 0x00_byte);
		CHECK(id.version() == uuid_version::unknown); // ((0x99 & 0b11110000) >> 4) == 0b1001 == unknown (>= 6)
		CHECK(id.variant() == uuid_variant::reserved_ncs); // ((0x77 & 0b11100000) >> 5) == 0b011 == reserved_ncs
		CHECK(id.time_low() == 0xFFEEDDCCu);
		CHECK(id.time_mid() == 0xBBAA_u16);
		CHECK(id.time_high_and_version() == 0x9988_u16);
		CHECK(id.clock_seq_high_and_reserved() == 0x77_u8);
		CHECK(id.clock_seq_low() == 0x66_u8);
		CHECK(id.node() == 0x554433221100_u64);

		// 7D444840-9DC0-11D1-B245-5FFDCE74FAD2 (example from the rfc doc)
		id = uuid{ 0x7D444840u, 0x9DC0_u16, 0x11D1_u16, 0xB245_u16, 0x5FFDCE74FAD2_u64 };
		CHECK(id.bytes.value[0]  == 0x7D_byte);
		CHECK(id.bytes.value[1]  == 0x44_byte);
		CHECK(id.bytes.value[2]  == 0x48_byte);
		CHECK(id.bytes.value[3]  == 0x40_byte);
		CHECK(id.bytes.value[4]  == 0x9D_byte);
		CHECK(id.bytes.value[5]  == 0xC0_byte);
		CHECK(id.bytes.value[6]  == 0x11_byte);
		CHECK(id.bytes.value[7]  == 0xD1_byte);
		CHECK(id.bytes.value[8]  == 0xB2_byte);
		CHECK(id.bytes.value[9]  == 0x45_byte);
		CHECK(id.bytes.value[10] == 0x5F_byte);
		CHECK(id.bytes.value[11] == 0xFD_byte);
		CHECK(id.bytes.value[12] == 0xCE_byte);
		CHECK(id.bytes.value[13] == 0x74_byte);
		CHECK(id.bytes.value[14] == 0xFA_byte);
		CHECK(id.bytes.value[15] == 0xD2_byte);
		CHECK(id.version() == uuid_version::time); // ((0x11 & 0b11110000) >> 4) == 0b0001 == version 1 (time)
		CHECK(id.variant() == uuid_variant::standard); // ((0xB2 & 0b11100000) >> 5) == 0b101 == standard
		CHECK(id.time_low() == 0x7D444840u);
		CHECK(id.time_mid() == 0x9DC0_u16);
		CHECK(id.time_high_and_version() == 0x11D1_u16);
		CHECK(id.clock_seq_high_and_reserved() == 0xB2_u8);
		CHECK(id.clock_seq_low() == 0x45_u8);
		CHECK(id.node() == 0x5FFDCE74FAD2_u64);

		#if MUU_HAS_INT128

		// 0CBC8F62-78FE-4061-8473-9FCB662AEEDF
		auto i128 = pack(0x0CBC8F6278FE4061_u64, 0x84739FCB662AEEDF_u64);
		if constexpr (build::is_big_endian)
			i128 = byte_reverse(i128);
		id = uuid{ i128 };
		CHECK(id.bytes.value[0] == 0x0C_byte);
		CHECK(id.bytes.value[1] == 0xBC_byte);
		CHECK(id.bytes.value[2] == 0x8F_byte);
		CHECK(id.bytes.value[3] == 0x62_byte);
		CHECK(id.bytes.value[4] == 0x78_byte);
		CHECK(id.bytes.value[5] == 0xFE_byte);
		CHECK(id.bytes.value[6] == 0x40_byte);
		CHECK(id.bytes.value[7] == 0x61_byte);
		CHECK(id.bytes.value[8] == 0x84_byte);
		CHECK(id.bytes.value[9] == 0x73_byte);
		CHECK(id.bytes.value[10] == 0x9F_byte);
		CHECK(id.bytes.value[11] == 0xCB_byte);
		CHECK(id.bytes.value[12] == 0x66_byte);
		CHECK(id.bytes.value[13] == 0x2A_byte);
		CHECK(id.bytes.value[14] == 0xEE_byte);
		CHECK(id.bytes.value[15] == 0xDF_byte);
		CHECK(id.version() == uuid_version::random); // ((0x40 & 0b11110000) >> 4) == 0b0100 == version 4 (random)
		CHECK(id.variant() == uuid_variant::standard); // ((0x84 & 0b11100000) >> 5) == 0b100 == standard
		CHECK(id.time_low() == 0x0CBC8F62u);
		CHECK(id.time_mid() == 0x78FE_u16);
		CHECK(id.time_high_and_version() == 0x4061_u16);
		CHECK(id.clock_seq_high_and_reserved() == 0x84_u8);
		CHECK(id.clock_seq_low() == 0x73_u8);
		CHECK(id.node() == 0x9FCB662AEEDF_u64);
		CHECK(static_cast<uint128_t>(id) == i128);

		#endif // MUU_HAS_INT128
	}
}

TEST_CASE("uuid - generation")
{
	//generated uuids
	static constexpr auto num_random_ids = 50000_sz;
	{
		//set (uses std::less)
		std::set<uuid> ids;
		for (size_t i = 0; i < num_random_ids; i++)
		{
			auto id = uuid::generate();
			CHECK(id);
			CHECK(id.variant() == uuid_variant::standard);
			CHECK(id.version() == uuid_version::random);
			CHECK(ids.insert(id).second == true);
		}
	}
	{
		//unordered set (uses std::hash)
		std::unordered_set<uuid> ids;
		ids.reserve(num_random_ids);
		for (size_t i = 0; i < num_random_ids; i++)
		{
			auto id = uuid::generate();
			CHECK(id);
			CHECK(id.variant() == uuid_variant::standard);
			CHECK(id.version() == uuid_version::random);
			CHECK(ids.insert(id).second == true);
		}
	}
}

TEST_CASE("uuid - named")
{
	// the expected outputs here were externally generated using https://www.uuidtools.com/v5
	{
		// 4BE0643F-1D98-573B-97CD-CA98A65347DD
		const auto id = uuid{ constants<uuid>::namespace_dns, u8"test"sv };
		CHECK(id == uuid{ 0x4BE0643Fu, 0x1D98_u16, 0x573B_u16, 0x97CD_u16, 0xCA98A65347DD_u64 });
	}
	{
		// 74738FF5-5367-5958-9AEE-98FFFDCD1876
		const auto id = uuid{ constants<uuid>::namespace_dns, u8"www.example.org"sv };
		CHECK(id == uuid{ 0x74738FF5u, 0x5367_u16, 0x5958_u16, 0x9AEE_u16, 0x98FFFDCD1876_u64 });
	}
	{
		// 7ED715E6-67ED-5C47-8F14-755B755E6E5C
		const auto id = uuid{ constants<uuid>::namespace_url, u8"ði ıntəˈnæʃənəl fəˈnɛtık əsoʊsiˈeıʃn"sv };
		CHECK(id == uuid{ 0x7ED715E6u, 0x67ED_u16, 0x5C47_u16, 0x8F14_u16, 0x755B755E6E5C_u64 });
	}
}

TEST_CASE("uuid - relops")
{
	constexpr auto zero = uuid{ nullptr };
	constexpr auto low = uuid{ 0x00000000_u32, 0x0000_u16, 0x0000_u16, 0x01_u8, 0x01_u8, 0x010101010101_u64 };
	constexpr auto mid = uuid{ 0x01010101_u32, 0x0101_u16, 0x0101_u16, 0x01_u8, 0x01_u8, 0x010101010101_u64 };
	constexpr auto high = uuid{ 0x01010101_u32, 0x0101_u16, 0x0101_u16, 0x02_u8, 0x02_u8, 0x020202020202_u64 };
	constexpr auto high2 = uuid{ 0x01010101_u32, 0x0101_u16, 0x0101_u16, 0x02_u8, 0x02_u8, 0x020202020202_u64 };

	//make sure equality and inequality are symmetrical
	CHECK_AND_STATIC_ASSERT(zero != low);
	CHECK_AND_STATIC_ASSERT(zero != mid);
	CHECK_AND_STATIC_ASSERT(zero != high);
	CHECK_AND_STATIC_ASSERT(zero != high2);
	CHECK_AND_STATIC_ASSERT(low != zero);
	CHECK_AND_STATIC_ASSERT(low != mid);
	CHECK_AND_STATIC_ASSERT(low != high);
	CHECK_AND_STATIC_ASSERT(low != high2);
	CHECK_AND_STATIC_ASSERT(mid != zero);
	CHECK_AND_STATIC_ASSERT(mid != low);
	CHECK_AND_STATIC_ASSERT(mid != high);
	CHECK_AND_STATIC_ASSERT(mid != high2);
	CHECK_AND_STATIC_ASSERT(high != zero);
	CHECK_AND_STATIC_ASSERT(high != low);
	CHECK_AND_STATIC_ASSERT(high != mid);
	CHECK_AND_STATIC_ASSERT(high == high2);
	CHECK_AND_STATIC_ASSERT(high2 == high);

	//relational ops
	CHECK_AND_STATIC_ASSERT(zero < low);
	CHECK_AND_STATIC_ASSERT(zero < mid);
	CHECK_AND_STATIC_ASSERT(zero < high);
	CHECK_AND_STATIC_ASSERT(zero < high2);
	CHECK_AND_STATIC_ASSERT(low > zero);
	CHECK_AND_STATIC_ASSERT(low < mid);
	CHECK_AND_STATIC_ASSERT(low < high);
	CHECK_AND_STATIC_ASSERT(low < high2);
	CHECK_AND_STATIC_ASSERT(mid > zero);
	CHECK_AND_STATIC_ASSERT(mid > low);
	CHECK_AND_STATIC_ASSERT(mid < high);
	CHECK_AND_STATIC_ASSERT(mid < high2);
	CHECK_AND_STATIC_ASSERT(high > zero);
	CHECK_AND_STATIC_ASSERT(high > low);
	CHECK_AND_STATIC_ASSERT(high > mid);
	CHECK_AND_STATIC_ASSERT(high <= high2);
	CHECK_AND_STATIC_ASSERT(high >= high2);
	CHECK_AND_STATIC_ASSERT(!(high > high2));
	CHECK_AND_STATIC_ASSERT(!(high < high2));
	CHECK_AND_STATIC_ASSERT(zero <= low);
	CHECK_AND_STATIC_ASSERT(zero <= mid);
	CHECK_AND_STATIC_ASSERT(zero <= high);
	CHECK_AND_STATIC_ASSERT(zero <= high2);
	CHECK_AND_STATIC_ASSERT(low >= zero);
	CHECK_AND_STATIC_ASSERT(low <= mid);
	CHECK_AND_STATIC_ASSERT(low <= high);
	CHECK_AND_STATIC_ASSERT(low <= high2);
	CHECK_AND_STATIC_ASSERT(mid >= zero);
	CHECK_AND_STATIC_ASSERT(mid >= low);
	CHECK_AND_STATIC_ASSERT(mid <= high);
	CHECK_AND_STATIC_ASSERT(mid <= high2);
	CHECK_AND_STATIC_ASSERT(high >= zero);
	CHECK_AND_STATIC_ASSERT(high >= low);
	CHECK_AND_STATIC_ASSERT(high >= mid);
	CHECK_AND_STATIC_ASSERT(high >= high2);

	//test example from the RFC
	{
		const auto rfc = uuid{ 0x7D444840u, 0x9DC0_u16, 0x11D1_u16, 0xB245_u16, 0x5FFDCE74FAD2_u64 };
		CHECK(uuid::compare(rfc, rfc) == 0);
		CHECK(uuid::compare(rfc, constants<uuid>::namespace_dns) == 1);
		CHECK(uuid::compare(constants<uuid>::namespace_dns, rfc) == -1);
	}
}

#define CHECK_PARSE_FAILS(input)																\
	CHECK_AND_STATIC_ASSERT(!uuid::parse(MUU_APPEND_SV(input)));								\
	CHECK_AND_STATIC_ASSERT(!uuid::parse(MUU_CONCAT(u8, MUU_APPEND_SV(input))));				\
	CHECK_AND_STATIC_ASSERT(!uuid::parse(MUU_CONCAT(u, MUU_APPEND_SV(input))));					\
	CHECK_AND_STATIC_ASSERT(!uuid::parse(MUU_CONCAT(U, MUU_APPEND_SV(input))));					\
	CHECK_AND_STATIC_ASSERT_W(!uuid::parse(MUU_CONCAT(L, MUU_APPEND_SV(input))))

#define CHECK_PARSE_SUCCEEDS(input, expected)													\
	CHECK_AND_STATIC_ASSERT(uuid::parse(MUU_APPEND_SV(input)) == expected);						\
	CHECK_AND_STATIC_ASSERT(uuid::parse(MUU_CONCAT(u8, MUU_APPEND_SV(input))) == expected);		\
	CHECK_AND_STATIC_ASSERT(uuid::parse(MUU_CONCAT(u, MUU_APPEND_SV(input))) == expected);		\
	CHECK_AND_STATIC_ASSERT(uuid::parse(MUU_CONCAT(U, MUU_APPEND_SV(input))) == expected);		\
	CHECK_AND_STATIC_ASSERT_W(uuid::parse(MUU_CONCAT(L, MUU_APPEND_SV(input))) == expected)

TEST_CASE("uuid - parsing")
{
	//'empty' strings
	CHECK_PARSE_FAILS("");
	CHECK_PARSE_FAILS("  ");
	CHECK_PARSE_FAILS("{}");
	CHECK_PARSE_FAILS(" {} ");

	//too short
	CHECK_PARSE_FAILS("FAEADACA-BBAA-9988-7766-55443322110"); //missing one digit
	CHECK_PARSE_FAILS("FAEADACA-BBAA-9988-7766-5544332211"); //missing one octet
	CHECK_PARSE_FAILS("FAEADACA-BBAA");

	//too long
	CHECK_PARSE_FAILS("FAEADACA-BBAA-9988-7766-554433221100FF"); //one whole octet too long
	CHECK_PARSE_FAILS("FAEADACA-BBAA-9988-7766-554433221100F");  //one digit too long

	//begins/ends with invalid characters
	CHECK_PARSE_FAILS(" FAEADACA-BBAA-9988-7766-554433221100Z");
	CHECK_PARSE_FAILS("ZFAEADACA-BBAA-9988-7766-554433221100");
	CHECK_PARSE_FAILS("T  {FAEADACA-BBAA-9988-7766-554433221100}  ");

	//contains invalid characters                              vv
	CHECK_PARSE_FAILS("FAEADACA-BBAA-9988-77GG-554433221100");

	//valid characters but in the wrong place
	CHECK_PARSE_FAILS("}FAEADACA-BBAA-9988-7766-554433221100");
	CHECK_PARSE_FAILS("FAEADACA-BBAA{9988-7766-554433221100");
	CHECK_PARSE_FAILS("FAEADACA-BBAA-9988}7766-554433221100");
	CHECK_PARSE_FAILS("FAEADACA-BBAA-9988-7766-554433221100{");

	//bad use of brackets
	CHECK_PARSE_FAILS("{{FAEADACA-BBAA-9988-7766-554433221100}}");
	CHECK_PARSE_FAILS("{FAEADACA-BBAA{9988-7766-554433221100");
	CHECK_PARSE_FAILS("FAEADACA-BBAA-9988}7766-554433221100}");
	CHECK_PARSE_FAILS("{{FAEADACA-BBAA-9988-7766-554433221100}");
	CHECK_PARSE_FAILS("{FAEADACA-BBAA-9988-7766-554433221100}}");

	//splices an octet                              v
	CHECK_PARSE_FAILS("FAEADACA-B-BAA-99887766-554433221100");

	//valid!
	static constexpr auto valid = uuid{ 0xFAEADACAu, 0xBBAA_u16, 0x9988_u16, 0x7766_u16, 0x554433221100_u64 };
	CHECK_PARSE_SUCCEEDS("FAEADACA-BBAA-9988-7766-554433221100", valid);
	CHECK_PARSE_SUCCEEDS("\t  FAEADACA-BBAA-9988-7766-554433221100 \t\t  ", valid);
	CHECK_PARSE_SUCCEEDS("{FAEADACA-BBAA-9988-7766-554433221100}", valid);
	CHECK_PARSE_SUCCEEDS("FAEADACABBAA99887766554433221100", valid);
	CHECK_PARSE_SUCCEEDS("{FAEADACABBAA99887766554433221100}", valid);
	CHECK_PARSE_SUCCEEDS("{FA EA DA CA BB AA 99 88 77 66 55 44 33 22 11 00}", valid);
}

#define CHECK_PRINTING(time_low, time_mid, time_high_and_version, clock_seq, node)		\
	CHECK(to_string(uuid(																\
		MUU_CONCAT(MUU_CONCAT(0x, time_low), _u32),										\
		MUU_CONCAT(MUU_CONCAT(0x, time_mid), _u16),										\
		MUU_CONCAT(MUU_CONCAT(0x, time_high_and_version), _u16),						\
		MUU_CONCAT(MUU_CONCAT(0x, clock_seq), _u16),									\
		MUU_CONCAT(MUU_CONCAT(0x, node), _u64)											\
	)) == MUU_MAKE_STRING_VIEW(time_low-time_mid-time_high_and_version-clock_seq-node))

TEST_CASE("uuid - printing")
{
	static constexpr auto to_string = [](const uuid& val) noexcept
	{
		std::ostringstream oss;
		oss << val;
		return oss.str();
	};

	CHECK_PRINTING(00000000, 0000, 0000, 0000, 000000000000);
	CHECK_PRINTING(FFFFFFFF, FFFF, FFFF, FFFF, FFFFFFFFFFFF);
	CHECK_PRINTING(052EBEDA, 3017, 4FC8, BBEE, 51C733EAF2F0);
	CHECK_PRINTING(63574AF0, 91BF, 4E5F, 9D35, 41DF1D89F02D);
	CHECK_PRINTING(4EA4106A, 593B, 4B7E, 9270, 56D8DBF568BF);
	CHECK_PRINTING(B4674434, E796, 474D, 9B2D, 6128518E6FBE);
	CHECK_PRINTING(77FC1931, 9A0F, 44A8, B34B, BE53F23EA04E);
	CHECK_PRINTING(D5FBA2EA, 3522, 4E0F, 9B49, 76EA92430D42);
	CHECK_PRINTING(7C120CE1, 4422, 4AF3, BD37, BDCCC7E76A53);
	CHECK_PRINTING(51BBBE58, 7B69, 475E, B81C, A3971E07E6C7);
	CHECK_PRINTING(322351AC, 0819, 4244, BF78, E1215F49D7EF);
	CHECK_PRINTING(8EC4E6AE, 908A, 489D, 9AEB, 6F19791DBC55);
	CHECK_PRINTING(687776AC, 18B1, 4C66, 9312, 8FF3E60358F6);
	CHECK_PRINTING(2B639DC2, B096, 46D7, 928F, 79B57CDA8400);
	CHECK_PRINTING(6C9F311B, BBCA, 4091, B6D6, 794A88D30FAD);
	CHECK_PRINTING(879917A5, 4BF9, 40E0, 8E2A, F2AF0352B2F8);
	CHECK_PRINTING(D475692B, 696A, 4BEF, 89AD, 9B68270A8B46);
	CHECK_PRINTING(25F07C8A, 6932, 4A48, 9DB7, 2E52B6895FF4);
	CHECK_PRINTING(19E17D52, BADF, 462A, 8C85, 9C2FCF8F90A1);
	CHECK_PRINTING(28BB9B89, 16DE, 4189, 9EC3, 3868A6AD0034);
	CHECK_PRINTING(10083130, A65D, 4D61, 9DF1, 537871D88CCB);
	CHECK_PRINTING(BB159EE2, DCD7, 47EB, 8C93, C1792A77BCA3);
	CHECK_PRINTING(027F7897, B7E2, 4BBD, 842F, DFD024733247);
	CHECK_PRINTING(ADE1C21C, EBDF, 4A78, 96B2, CD0B06FF767B);
	CHECK_PRINTING(E5868D11, 37A7, 4B87, 91BD, 9427E044D8FE);
	CHECK_PRINTING(CEB7D4A6, F168, 4C6E, 9291, EC80BA8A500C);
	CHECK_PRINTING(F35E37D5, 04D0, 4490, BC6F, AE9E50901FE2);
	CHECK_PRINTING(D5E13CF4, A292, 4DE6, A7CE, 05DA76506F18);
	CHECK_PRINTING(7F4F172A, 6600, 491A, B143, F4F6D235C99E);
	CHECK_PRINTING(98D75FF1, BD5E, 4F3B, 9028, F51C077FB94D);
	CHECK_PRINTING(0C90ADF1, DB60, 4F5A, 82A4, FBB2811E22B0);
	CHECK_PRINTING(6DECDCC7, BE56, 464C, 83CC, F3DEE7100DEC);
	CHECK_PRINTING(F94D5A87, 7A73, 4079, 92B1, E834BD7BAB3C);
	CHECK_PRINTING(A6E0D5CE, BA93, 4CCA, AC31, 05BBB72DFB87);
	CHECK_PRINTING(7033736A, C19C, 40CD, AD89, E79C63D30517);
	CHECK_PRINTING(88F50DC1, DE37, 4007, ADF8, B610AD0A46D9);
	CHECK_PRINTING(7A1C983D, 20C4, 410C, 8F03, CCB586860F73);
	CHECK_PRINTING(24A07B85, 04DC, 4DEC, 8DA3, C20A6C72A069);
	CHECK_PRINTING(1E7DCA57, 0603, 4FF1, B21A, C3F68D5CEA1C);
	CHECK_PRINTING(19E5E88B, C2BB, 4560, 8B66, 416F02269BBE);
	CHECK_PRINTING(C7A237C7, 69E9, 45D1, 830D, B3EBE83B05EF);
	CHECK_PRINTING(20C2F149, 8E5D, 44F9, 85E7, 2FEDF6198738);
	CHECK_PRINTING(98D13DE4, 4803, 42C7, 9F49, 27B3598F83D8);
	CHECK_PRINTING(24F5A15F, DC6C, 460F, 950B, B01EDB789D3F);
	CHECK_PRINTING(1CB09DD5, 7321, 46C1, 9B3B, 8CFB242DEE92);
	CHECK_PRINTING(01E34E01, F76B, 4A71, A926, 47575E9480D0);
	CHECK_PRINTING(ED1AA32C, 62F6, 4BCC, 8AA6, C59E6D18F950);
	CHECK_PRINTING(A65D0B46, 7720, 4F50, 8BA5, D693F8EB8476);
	CHECK_PRINTING(2715E40B, 2FF6, 4C91, 9C4F, 53923D7EB074);
	CHECK_PRINTING(35301594, AE31, 432A, ADA9, FA9C0C5D8C1F);
	CHECK_PRINTING(F3597327, D8AE, 4BE5, A25C, D92C707FECD7);
	CHECK_PRINTING(D0E5EE62, B643, 4BE9, 92F3, 15EF88CC8014);
}
