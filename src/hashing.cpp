// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "muu/hashing.h"

MUU_PUSH_WARNINGS
MUU_DISABLE_SPAM_WARNINGS
using namespace muu;

namespace { namespace sha1_utils
{
	using raw_block = uint32_t[16];
	struct block
	{
		raw_block value;
	};

	[[nodiscard]]
	MUU_ATTR(const)
	static constexpr uint32_t MUU_VECTORCALL rol(uint32_t value, uint32_t bits) noexcept
	{
		return (value << bits) | (value >> (32u - bits));
	}

	[[nodiscard]]
	static constexpr uint32_t MUU_VECTORCALL blk(const raw_block& bl, size_t i) noexcept
	{
		return rol(bl[(i + 13u) & 15u] ^ bl[(i + 8u) & 15u] ^ bl[(i + 2u) & 15u] ^ bl[i], 1);
	}

	static constexpr void MUU_VECTORCALL
	R0(const raw_block& bl, uint32_t v, uint32_t& w, uint32_t x, uint32_t y, uint32_t& z, size_t i) noexcept
	{
		z += ((w & (x ^ y)) ^ y) + bl[i] + 0x5a827999u + rol(v, 5u);
		w = rol(w, 30u);
	}

	static constexpr void MUU_VECTORCALL
	R1(raw_block& bl, uint32_t v, uint32_t& w, uint32_t x, uint32_t y, uint32_t& z, size_t i) noexcept
	{
		bl[i] = blk(bl, i);
		z += ((w & (x ^ y)) ^ y) + bl[i] + 0x5a827999u + rol(v, 5u);
		w = rol(w, 30u);
	}

	static constexpr void MUU_VECTORCALL
	R2(raw_block& bl, uint32_t v, uint32_t& w, uint32_t x, uint32_t y, uint32_t& z, size_t i) noexcept
	{
		bl[i] = blk(bl, i);
		z += (w ^ x ^ y) + bl[i] + 0x6ed9eba1u + rol(v, 5u);
		w = rol(w, 30u);
	}

	static constexpr void MUU_VECTORCALL
	R3(raw_block& bl, uint32_t v, uint32_t& w, uint32_t x, uint32_t y, uint32_t& z, size_t i) noexcept
	{
		bl[i] = blk(bl, i);
		z += (((w | x) & y) | (w & x)) + bl[i] + 0x8f1bbcdcu + rol(v, 5u);
		w = rol(w, 30u);
	}

	static constexpr void MUU_VECTORCALL
	R4(raw_block& bl, uint32_t v, uint32_t& w, uint32_t x, uint32_t y, uint32_t& z, size_t i) noexcept
	{
		bl[i] = blk(bl, i);
		z += (w ^ x ^ y) + bl[i] + 0xca62c1d6u + rol(v, 5u);
		w = rol(w, 30u);
	}

	static void process_block(const uint8_t(&block_)[64], uint32_t(&digest)[5]) noexcept
	{
		block bl = muu::bit_cast<block>(block_);
		if constexpr (muu::build::is_little_endian)
		{
			for (auto& i : bl.value)
				i = muu::byte_reverse(i);
		}

		// copy digest
		uint32_t a = digest[0];
		uint32_t b = digest[1];
		uint32_t c = digest[2];
		uint32_t d = digest[3];
		uint32_t e = digest[4];

		// 4 rounds of 20 operations each, loop unrolled
		R0(bl.value, a, b, c, d, e,  0u);
		R0(bl.value, e, a, b, c, d,  1u);
		R0(bl.value, d, e, a, b, c,  2u);
		R0(bl.value, c, d, e, a, b,  3u);
		R0(bl.value, b, c, d, e, a,  4u);
		R0(bl.value, a, b, c, d, e,  5u);
		R0(bl.value, e, a, b, c, d,  6u);
		R0(bl.value, d, e, a, b, c,  7u);
		R0(bl.value, c, d, e, a, b,  8u);
		R0(bl.value, b, c, d, e, a,  9u);
		R0(bl.value, a, b, c, d, e, 10u);
		R0(bl.value, e, a, b, c, d, 11u);
		R0(bl.value, d, e, a, b, c, 12u);
		R0(bl.value, c, d, e, a, b, 13u);
		R0(bl.value, b, c, d, e, a, 14u);
		R0(bl.value, a, b, c, d, e, 15u);
		R1(bl.value, e, a, b, c, d,  0u);
		R1(bl.value, d, e, a, b, c,  1u);
		R1(bl.value, c, d, e, a, b,  2u);
		R1(bl.value, b, c, d, e, a,  3u);
		R2(bl.value, a, b, c, d, e,  4u);
		R2(bl.value, e, a, b, c, d,  5u);
		R2(bl.value, d, e, a, b, c,  6u);
		R2(bl.value, c, d, e, a, b,  7u);
		R2(bl.value, b, c, d, e, a,  8u);
		R2(bl.value, a, b, c, d, e,  9u);
		R2(bl.value, e, a, b, c, d, 10u);
		R2(bl.value, d, e, a, b, c, 11u);
		R2(bl.value, c, d, e, a, b, 12u);
		R2(bl.value, b, c, d, e, a, 13u);
		R2(bl.value, a, b, c, d, e, 14u);
		R2(bl.value, e, a, b, c, d, 15u);
		R2(bl.value, d, e, a, b, c,  0u);
		R2(bl.value, c, d, e, a, b,  1u);
		R2(bl.value, b, c, d, e, a,  2u);
		R2(bl.value, a, b, c, d, e,  3u);
		R2(bl.value, e, a, b, c, d,  4u);
		R2(bl.value, d, e, a, b, c,  5u);
		R2(bl.value, c, d, e, a, b,  6u);
		R2(bl.value, b, c, d, e, a,  7u);
		R3(bl.value, a, b, c, d, e,  8u);
		R3(bl.value, e, a, b, c, d,  9u);
		R3(bl.value, d, e, a, b, c, 10u);
		R3(bl.value, c, d, e, a, b, 11u);
		R3(bl.value, b, c, d, e, a, 12u);
		R3(bl.value, a, b, c, d, e, 13u);
		R3(bl.value, e, a, b, c, d, 14u);
		R3(bl.value, d, e, a, b, c, 15u);
		R3(bl.value, c, d, e, a, b,  0u);
		R3(bl.value, b, c, d, e, a,  1u);
		R3(bl.value, a, b, c, d, e,  2u);
		R3(bl.value, e, a, b, c, d,  3u);
		R3(bl.value, d, e, a, b, c,  4u);
		R3(bl.value, c, d, e, a, b,  5u);
		R3(bl.value, b, c, d, e, a,  6u);
		R3(bl.value, a, b, c, d, e,  7u);
		R3(bl.value, e, a, b, c, d,  8u);
		R3(bl.value, d, e, a, b, c,  9u);
		R3(bl.value, c, d, e, a, b, 10u);
		R3(bl.value, b, c, d, e, a, 11u);
		R4(bl.value, a, b, c, d, e, 12u);
		R4(bl.value, e, a, b, c, d, 13u);
		R4(bl.value, d, e, a, b, c, 14u);
		R4(bl.value, c, d, e, a, b, 15u);
		R4(bl.value, b, c, d, e, a,  0u);
		R4(bl.value, a, b, c, d, e,  1u);
		R4(bl.value, e, a, b, c, d,  2u);
		R4(bl.value, d, e, a, b, c,  3u);
		R4(bl.value, c, d, e, a, b,  4u);
		R4(bl.value, b, c, d, e, a,  5u);
		R4(bl.value, a, b, c, d, e,  6u);
		R4(bl.value, e, a, b, c, d,  7u);
		R4(bl.value, d, e, a, b, c,  8u);
		R4(bl.value, c, d, e, a, b,  9u);
		R4(bl.value, b, c, d, e, a, 10u);
		R4(bl.value, a, b, c, d, e, 11u);
		R4(bl.value, e, a, b, c, d, 12u);
		R4(bl.value, d, e, a, b, c, 13u);
		R4(bl.value, c, d, e, a, b, 14u);
		R4(bl.value, b, c, d, e, a, 15u);

		// add back into digest
		digest[0] += a;
		digest[1] += b;
		digest[2] += c;
		digest[3] += d;
		digest[4] += e;
	}
}}

sha1::sha1() noexcept
	: state{
		{{
			0x67452301u,
			0xEFCDAB89u,
			0x98BADCFEu,
			0x10325476u,
			0xC3D2E1F0u
		}}
	}
{}

void sha1::add(uint8_t byte) noexcept
{
	MUU_ASSERT(!finished_);
	MUU_ASSERT(current_block_length < 64_u8);

	current_block[current_block_length++] = byte;
	if (current_block_length == 64_u8)
	{
		current_block_length = 0_u8;
		sha1_utils::process_block(current_block, state.digest.value);
		processed_blocks++;
	}
}

void sha1::add(const uint8_t* bytes, size_t num) noexcept
{
	MUU_ASSERT(!finished_);
	MUU_ASSERT(current_block_length < 64_u8);
	MUU_ASSERT(num > 0_sz);
	MUU_ASSERT(num <= (64_sz - current_block_length));

	if (bytes)
		memcpy(current_block + current_block_length, bytes, num);
	else
		memset(current_block + current_block_length, 0, num);
	current_block_length += static_cast<uint8_t>(num);
	if (current_block_length == 64_u8)
	{
		current_block_length = 0_u8;
		sha1_utils::process_block(current_block, state.digest.value);
		processed_blocks++;
	}
}

sha1& sha1::operator() (uint8_t byte) noexcept
{
	if (!finished_)
		add(byte);
	return *this;
}

sha1& sha1::operator() (const void* data, size_t size) noexcept
{
	if (!finished_ && data && size)
	{
		const auto end = reinterpret_cast<const uint8_t*>(data) + size;
		auto ptr = reinterpret_cast<const uint8_t*>(data);
		if (current_block_length)
		{
			const auto delta = (min)(64_sz - current_block_length, size);
			add(ptr, delta);
			ptr += delta;
		}
		while (ptr < end)
		{
			const auto delta = (min)(64_sz, static_cast<size_t>(end - ptr));
			add(ptr, delta);
			ptr += delta;
		}
	}
	return *this;
}

sha1& sha1::finish() noexcept
{
	if (finished_)
		return *this;

	uint64_t hashed_bits = (processed_blocks * 64_u64 + current_block_length) * build::bits_per_byte;
	if constexpr (build::is_little_endian)
		hashed_bits = byte_reverse(hashed_bits);

	// "message length in bits is a multiple of 8"
	add(0x80_u8);

	// padding
	if (current_block_length > 56u)
		add(nullptr, 64u - current_block_length);
	if (current_block_length < 56u)
		add(nullptr, 56u - current_block_length);

	MUU_ASSERT(current_block_length == 56u);

	// add hashed bit count and finish
	add(reinterpret_cast<const uint8_t*>(&hashed_bits), 8u);
	finished_ = true;

	// correct the endianness of the digest if necessary
	if constexpr (build::is_little_endian)
	{
		for (auto& i : state.digest.value)
			i = byte_reverse(i);
	}

	// convert the digest to bytes
	state.hash = bit_cast<hash_type>(state.digest);

	return *this;
}

MUU_POP_WARNINGS // MUU_DISABLE_SPAM_WARNINGS
