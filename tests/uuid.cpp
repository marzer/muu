// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/uuid.h"
#include <set>
#include <unordered_set>

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
		for (auto byte : id.bytes)
			CHECK(byte == 0x00_byte);
	}

	//explicit uuids
	{
		// 00112233-4455-6677-8899-aabbccddeeff
		auto id = uuid{ 0x00112233u, 0x4455_u16, 0x6677_u16, 0x88_u8, 0x99_u8, 0xAABBCCDDEEFF_u64 };
		CHECK(id.bytes[0]  == 0x00_byte);
		CHECK(id.bytes[1]  == 0x11_byte);
		CHECK(id.bytes[2]  == 0x22_byte);
		CHECK(id.bytes[3]  == 0x33_byte);
		CHECK(id.bytes[4]  == 0x44_byte);
		CHECK(id.bytes[5]  == 0x55_byte);
		CHECK(id.bytes[6]  == 0x66_byte);
		CHECK(id.bytes[7]  == 0x77_byte);
		CHECK(id.bytes[8]  == 0x88_byte);
		CHECK(id.bytes[9]  == 0x99_byte);
		CHECK(id.bytes[10] == 0xAA_byte);
		CHECK(id.bytes[11] == 0xBB_byte);
		CHECK(id.bytes[12] == 0xCC_byte);
		CHECK(id.bytes[13] == 0xDD_byte);
		CHECK(id.bytes[14] == 0xEE_byte);
		CHECK(id.bytes[15] == 0xFF_byte);
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
		CHECK(id.bytes[0]  == 0xFF_byte);
		CHECK(id.bytes[1]  == 0xEE_byte);
		CHECK(id.bytes[2]  == 0xDD_byte);
		CHECK(id.bytes[3]  == 0xCC_byte);
		CHECK(id.bytes[4]  == 0xBB_byte);
		CHECK(id.bytes[5]  == 0xAA_byte);
		CHECK(id.bytes[6]  == 0x99_byte);
		CHECK(id.bytes[7]  == 0x88_byte);
		CHECK(id.bytes[8]  == 0x77_byte);
		CHECK(id.bytes[9]  == 0x66_byte);
		CHECK(id.bytes[10] == 0x55_byte);
		CHECK(id.bytes[11] == 0x44_byte);
		CHECK(id.bytes[12] == 0x33_byte);
		CHECK(id.bytes[13] == 0x22_byte);
		CHECK(id.bytes[14] == 0x11_byte);
		CHECK(id.bytes[15] == 0x00_byte);
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
		CHECK(id.bytes[0]  == 0x7D_byte);
		CHECK(id.bytes[1]  == 0x44_byte);
		CHECK(id.bytes[2]  == 0x48_byte);
		CHECK(id.bytes[3]  == 0x40_byte);
		CHECK(id.bytes[4]  == 0x9D_byte);
		CHECK(id.bytes[5]  == 0xC0_byte);
		CHECK(id.bytes[6]  == 0x11_byte);
		CHECK(id.bytes[7]  == 0xD1_byte);
		CHECK(id.bytes[8]  == 0xB2_byte);
		CHECK(id.bytes[9]  == 0x45_byte);
		CHECK(id.bytes[10] == 0x5F_byte);
		CHECK(id.bytes[11] == 0xFD_byte);
		CHECK(id.bytes[12] == 0xCE_byte);
		CHECK(id.bytes[13] == 0x74_byte);
		CHECK(id.bytes[14] == 0xFA_byte);
		CHECK(id.bytes[15] == 0xD2_byte);
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
		CHECK(id.bytes[0] == 0x0C_byte);
		CHECK(id.bytes[1] == 0xBC_byte);
		CHECK(id.bytes[2] == 0x8F_byte);
		CHECK(id.bytes[3] == 0x62_byte);
		CHECK(id.bytes[4] == 0x78_byte);
		CHECK(id.bytes[5] == 0xFE_byte);
		CHECK(id.bytes[6] == 0x40_byte);
		CHECK(id.bytes[7] == 0x61_byte);
		CHECK(id.bytes[8] == 0x84_byte);
		CHECK(id.bytes[9] == 0x73_byte);
		CHECK(id.bytes[10] == 0x9F_byte);
		CHECK(id.bytes[11] == 0xCB_byte);
		CHECK(id.bytes[12] == 0x66_byte);
		CHECK(id.bytes[13] == 0x2A_byte);
		CHECK(id.bytes[14] == 0xEE_byte);
		CHECK(id.bytes[15] == 0xDF_byte);
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
	static constexpr auto num_random_ids = 5000_sz;
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

TEST_CASE("uuid - parsing")
{
	//'empty' strings
	CHECK_AND_STATIC_ASSERT(!uuid::parse(""sv));
	CHECK_AND_STATIC_ASSERT(!uuid::parse("  "sv));
	CHECK_AND_STATIC_ASSERT(!uuid::parse("{}"sv));
	CHECK_AND_STATIC_ASSERT(!uuid::parse(" {} "sv));

	//too short
	CHECK_AND_STATIC_ASSERT(!uuid::parse("FAEADACA-BBAA-9988-7766-55443322110"sv)); //missing one digit
	CHECK_AND_STATIC_ASSERT(!uuid::parse("FAEADACA-BBAA-9988-7766-5544332211"sv)); //missing one octet
	CHECK_AND_STATIC_ASSERT(!uuid::parse("FAEADACA-BBAA"sv));

	//too long
	CHECK_AND_STATIC_ASSERT(!uuid::parse("FAEADACA-BBAA-9988-7766-554433221100FF"sv)); //one whole octet too long
	CHECK_AND_STATIC_ASSERT(!uuid::parse("FAEADACA-BBAA-9988-7766-554433221100F"sv));  //one digit too long

	//begins/ends with invalid characters
	CHECK_AND_STATIC_ASSERT(!uuid::parse("FAEADACA-BBAA-9988-7766-554433221100Z"sv));
	CHECK_AND_STATIC_ASSERT(!uuid::parse("ZFAEADACA-BBAA-9988-7766-554433221100"sv));

	//contains invalid characters                              vv
	CHECK_AND_STATIC_ASSERT(!uuid::parse("FAEADACA-BBAA-9988-77GG-554433221100"sv));

	//valid characters but in the wrong place
	CHECK_AND_STATIC_ASSERT(!uuid::parse("}FAEADACA-BBAA-9988-7766-554433221100"sv));
	CHECK_AND_STATIC_ASSERT(!uuid::parse("FAEADACA-BBAA{9988-7766-554433221100"sv));
	CHECK_AND_STATIC_ASSERT(!uuid::parse("FAEADACA-BBAA-9988}7766-554433221100"sv));
	CHECK_AND_STATIC_ASSERT(!uuid::parse("FAEADACA-BBAA-9988-7766-554433221100{"sv));

	//bad use of brackets
	CHECK_AND_STATIC_ASSERT(!uuid::parse("{{FAEADACA-BBAA-9988-7766-554433221100}}"sv));
	CHECK_AND_STATIC_ASSERT(!uuid::parse("{FAEADACA-BBAA{9988-7766-554433221100"sv));
	CHECK_AND_STATIC_ASSERT(!uuid::parse("FAEADACA-BBAA-9988}7766-554433221100}"sv));
	CHECK_AND_STATIC_ASSERT(!uuid::parse("{{FAEADACA-BBAA-9988-7766-554433221100}"sv));
	CHECK_AND_STATIC_ASSERT(!uuid::parse("{FAEADACA-BBAA-9988-7766-554433221100}}"sv));

	//splices an octet                              v
	CHECK_AND_STATIC_ASSERT(!uuid::parse("FAEADACA-B-BAA-99887766-554433221100"sv));

	//valid!
	static constexpr auto valid = uuid{ 0xFAEADACAu, 0xBBAA_u16, 0x9988_u16, 0x7766_u16, 0x554433221100_u64 };
	CHECK_AND_STATIC_ASSERT(valid == uuid::parse("FAEADACA-BBAA-9988-7766-554433221100"sv));
	CHECK_AND_STATIC_ASSERT(valid == uuid::parse("  FAEADACA-BBAA-9988-7766-554433221100   "sv));
	CHECK_AND_STATIC_ASSERT(valid == uuid::parse("{FAEADACA-BBAA-9988-7766-554433221100}"sv));
	CHECK_AND_STATIC_ASSERT(valid == uuid::parse("FAEADACABBAA99887766554433221100"sv));
	CHECK_AND_STATIC_ASSERT(valid == uuid::parse("{FAEADACABBAA99887766554433221100}"sv));
	CHECK_AND_STATIC_ASSERT(valid == uuid::parse("{FA EA DA CA BB AA 99 88 77 66 55 44 33 22 11 00}"sv));

	////valid, this time using the UDL
	//CHECK_AND_STATIC_ASSERT(valid, "FAEADACA-BBAA-9988-7766-554433221100"_uuid);
	//CHECK_AND_STATIC_ASSERT(valid, "  FAEADACA-BBAA-9988-7766-554433221100   "_uuid);
	//CHECK_AND_STATIC_ASSERT(valid, "{FAEADACA-BBAA-9988-7766-554433221100}"_uuid);
	//CHECK_AND_STATIC_ASSERT(valid, "FAEADACABBAA99887766554433221100"_uuid);
	//CHECK_AND_STATIC_ASSERT(valid, "{FAEADACABBAA99887766554433221100}"_uuid);
	//CHECK_AND_STATIC_ASSERT(valid, "{FAEADACA,BBAA,9988,7766,554433221100}"_uuid);
	//CHECK_AND_STATIC_ASSERT(valid, "{ FAEADACA, BBAA, 9988, 7766, 554433221100 }"_uuid);
	//CHECK_AND_STATIC_ASSERT(valid, "{FA EA DA CA BB AA 99 88 77 66 55 44 33 22 11 00}"_uuid);
}
