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
		//00112233-4455-6677-8899-aabbccddeeff
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

		//FFEEDDCC-BBAA-9988-7766-554433221100
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

		//7D444840-9DC0-11D1-B245-5FFDCE74FAD2 (example from the rfc doc)
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

	////test example from the RFC
	//{
	//	const auto rfc = "7D444840-9DC0-11D1-B245-5FFDCE74FAD2"_uuid;
	//	ASSERT_EQ(uuid::Compare(rfc, rfc), 0);
	//	ASSERT_EQ(uuid::Compare(rfc, uuids::Namespaces::DNS), 1);
	//	ASSERT_EQ(uuid::Compare(uuids::Namespaces::DNS, rfc), -1);
	//}

	////named uuids
	//{
	//	auto id = uuid{ uuids::Namespaces::DNS, "www.example.org"sv };
	//	ASSERT_EQ(id, "74738FF5-5367-5958-9AEE-98FFFDCD1876"_uuid);
	//}
}
