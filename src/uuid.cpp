// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "muu/uuid.h"
#include "muu/hashing.h"
#include "printing.h"
MUU_DISABLE_WARNINGS
#if MUU_MSVC
	#include <rpc.h>
	MUU_PRAGMA_MSVC(comment(lib, "rpcrt4.lib"))
#else
	#include <random>
	// todo: not everyone will be ok with thread_local.
	// I should implement a way of controlling this via a library config option or two.

	[[nodiscard]]
	static std::random_device& random_device() noexcept
	{
		thread_local std::random_device rdev;
		return rdev;
	}

	[[nodiscard]]
	static std::mt19937& mersenne_twister() noexcept
	{
		thread_local std::mt19937 engine{ random_device()() };
		return engine;
	}
#endif
MUU_ENABLE_WARNINGS

using namespace muu;

uuid uuid::generate() noexcept
{
	uuid val;
	#if MUU_MSVC
	{
		UUID native;
		static_assert(sizeof(UUID) == sizeof(uuid));

		UuidCreate(&native);
		if constexpr (build::is_little_endian)
		{
			native.Data1 = byte_reverse(native.Data1);
			native.Data2 = byte_reverse(native.Data2);
			native.Data3 = byte_reverse(native.Data3);
		}
		memcpy(&val, &native, sizeof(UUID));
	}
	#else
	{
		// generate a version 4 uuid as per https://www.cryptosys.net/pki/uuid-rfc4122.html

		// "Set all the other bits to randomly (or pseudo-randomly) chosen values."
		std::uniform_int_distribution<unsigned> dist{ 0, 255 };
		for (auto& b : val.bytes.value)
			b = static_cast<std::byte>(dist(mersenne_twister()));

		// "Set the four most significant bits (bits 12 through 15) of the
		// time_hi_and_version field to the 4-bit version number."
		val.bytes.value[6] = (val.bytes.value[6] & 0b00001111_byte) | 0b01000000_byte; //version 4 (random)

		// "Set the two most significant bits (bits 6 and 7) of the
		// clock_seq_hi_and_reserved to zero and one, respectively."
		val.bytes.value[8] = (val.bytes.value[8] & 0b00111111_byte) | 0b10000000_byte; //variant (standard)

	}
	#endif
	return val;
}

uuid::uuid(const uuid& name_space, const void* name_data, size_t name_size) noexcept
{
	//hash the two values with SHA1 and use them as the initial value for the uuid
	{
		sha1 hasher;
		hasher(&name_space, sizeof(uuid));
		if (name_data && name_size)
			hasher(name_data, name_size);
		hasher.finish();
		memcpy(&bytes, &hasher.value(), sizeof(bytes));
	}

	// "Set the four most significant bits (bits 12 through 15) of the
	// time_hi_and_version field to the 4-bit version number."
	bytes.value[6] = (bytes.value[6] & 0b00001111_byte) | 0b01010000_byte; //version 5 (SHA1)

	// "Set the two most significant bits (bits 6 and 7) of the
	// clock_seq_hi_and_reserved to zero and one, respectively."
	bytes.value[8] = (bytes.value[8] & 0b00111111_byte) | 0b10000000_byte; //variant (standard)
}

namespace
{
	template <typename Char>
	static void print(std::basic_ostream<Char>& os, const uuid& id) noexcept
	{
		using chars = constants<Char>;

		// pre-format the uuid in a buffer so that the user's stream padding and alignment affects the whole UUID
		// (likely faster anyway)
		Char buffer[36];
		Char* pos = buffer;
		const auto write = [&](const auto& cstr) noexcept
		{
			memcpy(pos, &cstr, sizeof(cstr));
			pos += sizeof(cstr) / sizeof(Char);
		};
		static_assert(sizeof(impl::hex_char_pair<Char>) == sizeof(Char) * 2);
		write(impl::byte_to_hex(id.bytes.value[0], chars::letter_A));
		write(impl::byte_to_hex(id.bytes.value[1], chars::letter_A));
		write(impl::byte_to_hex(id.bytes.value[2], chars::letter_A));
		write(impl::byte_to_hex(id.bytes.value[3], chars::letter_A));
		write(chars::hyphen);
		write(impl::byte_to_hex(id.bytes.value[4], chars::letter_A));
		write(impl::byte_to_hex(id.bytes.value[5], chars::letter_A));
		write(chars::hyphen);
		write(impl::byte_to_hex(id.bytes.value[6], chars::letter_A));
		write(impl::byte_to_hex(id.bytes.value[7], chars::letter_A));
		write(chars::hyphen);
		write(impl::byte_to_hex(id.bytes.value[8], chars::letter_A));
		write(impl::byte_to_hex(id.bytes.value[9], chars::letter_A));
		write(chars::hyphen);
		write(impl::byte_to_hex(id.bytes.value[10], chars::letter_A));
		write(impl::byte_to_hex(id.bytes.value[11], chars::letter_A));
		write(impl::byte_to_hex(id.bytes.value[12], chars::letter_A));
		write(impl::byte_to_hex(id.bytes.value[13], chars::letter_A));
		write(impl::byte_to_hex(id.bytes.value[14], chars::letter_A));
		write(impl::byte_to_hex(id.bytes.value[15], chars::letter_A));
		MUU_ASSERT(pos == buffer + 36);

		// print the thing
		os << std::basic_string_view<Char>{ buffer, 36_sz };
	}
}

MUU_IMPL_NAMESPACE_START
{
	void print_to_stream(std::ostream& os, const uuid& id)
	{
		::print(os, id);
	}

	void print_to_stream(std::wostream& os, const uuid& id)
	{
		::print(os, id);
	}
}
MUU_IMPL_NAMESPACE_END
