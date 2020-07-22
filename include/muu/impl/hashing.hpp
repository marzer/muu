// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "../../muu/preprocessor.h"
#if !MUU_IMPLEMENTATION
	#error This is an implementation-only header.
#endif

#include "../../muu/hashing.h"

MUU_ANON_NAMESPACE_START { namespace sha1_utils
{
	using block = MUU_NAMESPACE::impl::array<uint32_t, 16>;

	[[nodiscard]]
	MUU_INTERNAL_LINKAGE
	MUU_ATTR(const)
	constexpr uint32_t MUU_VECTORCALL rol(uint32_t value, uint32_t bits) noexcept
	{
		return (value << bits) | (value >> (32u - bits));
	}

	[[nodiscard]]
	MUU_INTERNAL_LINKAGE
	constexpr uint32_t MUU_VECTORCALL blk(const block& bl, size_t i) noexcept
	{
		return rol(bl[(i + 13u) & 15u] ^ bl[(i + 8u) & 15u] ^ bl[(i + 2u) & 15u] ^ bl[i], 1);
	}

	MUU_INTERNAL_LINKAGE
	constexpr void MUU_VECTORCALL
	R0(const block& bl, uint32_t v, uint32_t& w, uint32_t x, uint32_t y, uint32_t& z, size_t i) noexcept
	{
		z += ((w & (x ^ y)) ^ y) + bl[i] + 0x5a827999u + rol(v, 5u);
		w = rol(w, 30u);
	}

	MUU_INTERNAL_LINKAGE
	constexpr void MUU_VECTORCALL
	R1(block& bl, uint32_t v, uint32_t& w, uint32_t x, uint32_t y, uint32_t& z, size_t i) noexcept
	{
		bl[i] = blk(bl, i);
		z += ((w & (x ^ y)) ^ y) + bl[i] + 0x5a827999u + rol(v, 5u);
		w = rol(w, 30u);
	}

	MUU_INTERNAL_LINKAGE
	constexpr void MUU_VECTORCALL
	R2(block& bl, uint32_t v, uint32_t& w, uint32_t x, uint32_t y, uint32_t& z, size_t i) noexcept
	{
		bl[i] = blk(bl, i);
		z += (w ^ x ^ y) + bl[i] + 0x6ed9eba1u + rol(v, 5u);
		w = rol(w, 30u);
	}

	MUU_INTERNAL_LINKAGE
	constexpr void MUU_VECTORCALL
	R3(block& bl, uint32_t v, uint32_t& w, uint32_t x, uint32_t y, uint32_t& z, size_t i) noexcept
	{
		bl[i] = blk(bl, i);
		z += (((w | x) & y) | (w & x)) + bl[i] + 0x8f1bbcdcu + rol(v, 5u);
		w = rol(w, 30u);
	}

	MUU_INTERNAL_LINKAGE
	constexpr void MUU_VECTORCALL
	R4(block& bl, uint32_t v, uint32_t& w, uint32_t x, uint32_t y, uint32_t& z, size_t i) noexcept
	{
		bl[i] = blk(bl, i);
		z += (w ^ x ^ y) + bl[i] + 0xca62c1d6u + rol(v, 5u);
		w = rol(w, 30u);
	}

	MUU_INTERNAL_LINKAGE
	void process_block(const uint8_t(&raw_block)[64], uint32_t(&digest)[5]) noexcept
	{
		using namespace MUU_NAMESPACE;
		block bl = bit_cast<block>(raw_block);
		if constexpr (build::is_little_endian)
		{
			for (auto& i : bl)
				i = byte_reverse(i);
		}

		// copy digest
		uint32_t a = digest[0];
		uint32_t b = digest[1];
		uint32_t c = digest[2];
		uint32_t d = digest[3];
		uint32_t e = digest[4];

		// 4 rounds of 20 operations each, loop unrolled
		R0(bl, a, b, c, d, e,  0_sz);
		R0(bl, e, a, b, c, d,  1_sz);
		R0(bl, d, e, a, b, c,  2_sz);
		R0(bl, c, d, e, a, b,  3_sz);
		R0(bl, b, c, d, e, a,  4_sz);
		R0(bl, a, b, c, d, e,  5_sz);
		R0(bl, e, a, b, c, d,  6_sz);
		R0(bl, d, e, a, b, c,  7_sz);
		R0(bl, c, d, e, a, b,  8_sz);
		R0(bl, b, c, d, e, a,  9_sz);
		R0(bl, a, b, c, d, e, 10_sz);
		R0(bl, e, a, b, c, d, 11_sz);
		R0(bl, d, e, a, b, c, 12_sz);
		R0(bl, c, d, e, a, b, 13_sz);
		R0(bl, b, c, d, e, a, 14_sz);
		R0(bl, a, b, c, d, e, 15_sz);
		R1(bl, e, a, b, c, d,  0_sz);
		R1(bl, d, e, a, b, c,  1_sz);
		R1(bl, c, d, e, a, b,  2_sz);
		R1(bl, b, c, d, e, a,  3_sz);
		R2(bl, a, b, c, d, e,  4_sz);
		R2(bl, e, a, b, c, d,  5_sz);
		R2(bl, d, e, a, b, c,  6_sz);
		R2(bl, c, d, e, a, b,  7_sz);
		R2(bl, b, c, d, e, a,  8_sz);
		R2(bl, a, b, c, d, e,  9_sz);
		R2(bl, e, a, b, c, d, 10_sz);
		R2(bl, d, e, a, b, c, 11_sz);
		R2(bl, c, d, e, a, b, 12_sz);
		R2(bl, b, c, d, e, a, 13_sz);
		R2(bl, a, b, c, d, e, 14_sz);
		R2(bl, e, a, b, c, d, 15_sz);
		R2(bl, d, e, a, b, c,  0_sz);
		R2(bl, c, d, e, a, b,  1_sz);
		R2(bl, b, c, d, e, a,  2_sz);
		R2(bl, a, b, c, d, e,  3_sz);
		R2(bl, e, a, b, c, d,  4_sz);
		R2(bl, d, e, a, b, c,  5_sz);
		R2(bl, c, d, e, a, b,  6_sz);
		R2(bl, b, c, d, e, a,  7_sz);
		R3(bl, a, b, c, d, e,  8_sz);
		R3(bl, e, a, b, c, d,  9_sz);
		R3(bl, d, e, a, b, c, 10_sz);
		R3(bl, c, d, e, a, b, 11_sz);
		R3(bl, b, c, d, e, a, 12_sz);
		R3(bl, a, b, c, d, e, 13_sz);
		R3(bl, e, a, b, c, d, 14_sz);
		R3(bl, d, e, a, b, c, 15_sz);
		R3(bl, c, d, e, a, b,  0_sz);
		R3(bl, b, c, d, e, a,  1_sz);
		R3(bl, a, b, c, d, e,  2_sz);
		R3(bl, e, a, b, c, d,  3_sz);
		R3(bl, d, e, a, b, c,  4_sz);
		R3(bl, c, d, e, a, b,  5_sz);
		R3(bl, b, c, d, e, a,  6_sz);
		R3(bl, a, b, c, d, e,  7_sz);
		R3(bl, e, a, b, c, d,  8_sz);
		R3(bl, d, e, a, b, c,  9_sz);
		R3(bl, c, d, e, a, b, 10_sz);
		R3(bl, b, c, d, e, a, 11_sz);
		R4(bl, a, b, c, d, e, 12_sz);
		R4(bl, e, a, b, c, d, 13_sz);
		R4(bl, d, e, a, b, c, 14_sz);
		R4(bl, c, d, e, a, b, 15_sz);
		R4(bl, b, c, d, e, a,  0_sz);
		R4(bl, a, b, c, d, e,  1_sz);
		R4(bl, e, a, b, c, d,  2_sz);
		R4(bl, d, e, a, b, c,  3_sz);
		R4(bl, c, d, e, a, b,  4_sz);
		R4(bl, b, c, d, e, a,  5_sz);
		R4(bl, a, b, c, d, e,  6_sz);
		R4(bl, e, a, b, c, d,  7_sz);
		R4(bl, d, e, a, b, c,  8_sz);
		R4(bl, c, d, e, a, b,  9_sz);
		R4(bl, b, c, d, e, a, 10_sz);
		R4(bl, a, b, c, d, e, 11_sz);
		R4(bl, e, a, b, c, d, 12_sz);
		R4(bl, d, e, a, b, c, 13_sz);
		R4(bl, c, d, e, a, b, 14_sz);
		R4(bl, b, c, d, e, a, 15_sz);

		// add back into digest
		digest[0] += a;
		digest[1] += b;
		digest[2] += c;
		digest[3] += d;
		digest[4] += e;
	}
}}
MUU_ANON_NAMESPACE_END

MUU_NAMESPACE_START
{
	MUU_EXTERNAL_LINKAGE
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

	MUU_EXTERNAL_LINKAGE
	void sha1::add(uint8_t byte) noexcept
	{
		MUU_USING_ANON_NAMESPACE;
		MUU_ASSERT(!finished_);
		MUU_ASSERT(current_block_length < 64_u8);

		current_block[current_block_length++] = byte;
		if (current_block_length == 64_u8)
		{
			current_block_length = 0_u8;
			sha1_utils::process_block(current_block, state.digest.values);
			processed_blocks++;
		}
	}

	MUU_EXTERNAL_LINKAGE
	void sha1::add(const uint8_t* bytes, size_t num) noexcept
	{
		MUU_USING_ANON_NAMESPACE;
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
			sha1_utils::process_block(current_block, state.digest.values);
			processed_blocks++;
		}
	}

	MUU_EXTERNAL_LINKAGE
	sha1& sha1::operator() (uint8_t byte) noexcept
	{
		MUU_USING_ANON_NAMESPACE;

		if (!finished_)
			add(byte);
		return *this;
	}

	MUU_EXTERNAL_LINKAGE
	sha1& sha1::operator() (const void* data, size_t size) noexcept
	{
		MUU_USING_ANON_NAMESPACE;

		if (!finished_ && data && size)
		{
			const auto end = pointer_cast<const uint8_t*>(data) + size;
			auto ptr = pointer_cast<const uint8_t*>(data);
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

	MUU_EXTERNAL_LINKAGE
	sha1& sha1::finish() noexcept
	{
		MUU_USING_ANON_NAMESPACE;

		if (finished_)
			return *this;

		uint64_t hashed_bits = (processed_blocks * 64_u64 + current_block_length) * build::bits_per_byte;
		if constexpr (build::is_little_endian)
			hashed_bits = byte_reverse(hashed_bits);

		// "message length in bits is a multiple of 8"
		add(0x80_u8);

		// padding
		if (current_block_length > 56_u8)
			add(nullptr, (64_sz - current_block_length) + 56_sz);
		else if (current_block_length < 56_u8)
			add(nullptr, 56_sz - current_block_length);

		MUU_ASSERT(current_block_length == 56_u8);

		// add hashed bit count and finish
		add(pointer_cast<const uint8_t*>(&hashed_bits), 8_sz);
		finished_ = true;

		// correct the endianness of the digest if necessary
		if constexpr (build::is_little_endian)
		{
			for (auto& i : state.digest.values)
				i = byte_reverse(i);
		}

		// convert the digest to bytes
		state.hash = bit_cast<decltype(state.hash)>(state.digest);

		return *this;
	}
}
MUU_NAMESPACE_END
