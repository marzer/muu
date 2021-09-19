// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "muu/uuid.h"
#include "muu/hashing.h"

MUU_DISABLE_WARNINGS;
#include <ostream>
#if MUU_WINDOWS
	#include <rpc.h>
	#pragma comment(lib, "rpcrt4.lib")
#else
	#include <random>
#endif
MUU_ENABLE_WARNINGS;

#include "source_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;

namespace
{
	template <typename Char>
	MUU_ATTR(nonnull)
	static void print(std::basic_ostream<Char>& os, const std::byte* id)
	{
		MUU_ASSUME(id);
		using chars = constants<Char>;

		// pre-format the uuid in a buffer so that the user's stream padding and alignment affects the whole UUID
		// (likely faster anyway)
		Char buffer[36];
		Char* pos		 = buffer;
		const auto write = [&](const auto& cstr) noexcept
		{
			MUU_MEMCPY(pos, &cstr, sizeof(cstr));
			pos += sizeof(cstr) / sizeof(Char);
		};
		static_assert(sizeof(impl::hex_char_pair<Char>) == sizeof(Char) * 2);
		write(impl::byte_to_hex(id[0], chars::letter_A));
		write(impl::byte_to_hex(id[1], chars::letter_A));
		write(impl::byte_to_hex(id[2], chars::letter_A));
		write(impl::byte_to_hex(id[3], chars::letter_A));
		write(chars::hyphen);
		write(impl::byte_to_hex(id[4], chars::letter_A));
		write(impl::byte_to_hex(id[5], chars::letter_A));
		write(chars::hyphen);
		write(impl::byte_to_hex(id[6], chars::letter_A));
		write(impl::byte_to_hex(id[7], chars::letter_A));
		write(chars::hyphen);
		write(impl::byte_to_hex(id[8], chars::letter_A));
		write(impl::byte_to_hex(id[9], chars::letter_A));
		write(chars::hyphen);
		write(impl::byte_to_hex(id[10], chars::letter_A));
		write(impl::byte_to_hex(id[11], chars::letter_A));
		write(impl::byte_to_hex(id[12], chars::letter_A));
		write(impl::byte_to_hex(id[13], chars::letter_A));
		write(impl::byte_to_hex(id[14], chars::letter_A));
		write(impl::byte_to_hex(id[15], chars::letter_A));
		MUU_ASSERT(pos == buffer + 36);

		// print the thing
		os << std::basic_string_view<Char>{ buffer, 36_sz };
	}

#if !MUU_WINDOWS
	MUU_DISABLE_WARNINGS;

	// todo: not everyone will be ok with thread_local.
	// I should implement a way of controlling this via a library config option or two.

	MUU_NODISCARD
	static std::random_device& random_device() noexcept
	{
		thread_local std::random_device rdev;
		return rdev;
	}

	MUU_NODISCARD
	static std::mt19937& mersenne_twister() noexcept
	{
		thread_local std::mt19937 engine{ random_device()() };
		return engine;
	}

	MUU_ENABLE_WARNINGS;
#endif //!MUU_WINDOWS
}

extern "C" //
{
	void MUU_CALLCONV muu_impl_uuid_print_to_ostream(const std::byte* id, std::ostream* os)
	{
		MUU_ASSUME(os);
		MUU_ASSUME(id);

		::print(*os, id);
	}

	void MUU_CALLCONV muu_impl_uuid_print_to_wostream(const std::byte* id, std::wostream* os)
	{
		MUU_ASSUME(os);
		MUU_ASSUME(id);

		::print(*os, id);
	}

	void MUU_CALLCONV muu_impl_uuid_generate(std::byte* id) noexcept
	{
		MUU_ASSUME(id);

#if MUU_WINDOWS
		{
			UUID native;
			static_assert(sizeof(UUID) == 16);

			UuidCreate(&native);
			if constexpr (build::is_little_endian)
			{
				native.Data1 = byte_reverse(native.Data1);
				native.Data2 = byte_reverse(native.Data2);
				native.Data3 = byte_reverse(native.Data3);
			}
			MUU_MEMCPY(id, &native, sizeof(UUID));
		}
#else
		{
			// generate a version 4 uuid as per https://www.cryptosys.net/pki/uuid-rfc4122.html

			// "Set all the other bits to randomly (or pseudo-randomly) chosen values."
			std::uniform_int_distribution<unsigned> dist{ 0, 255 };
			for (size_t i = 0; i < 16_sz; i++)
				id[i] = static_cast<std::byte>(dist(mersenne_twister()));

			// "Set the four most significant bits (bits 12 through 15) of the
			// time_hi_and_version field to the 4-bit version number."
			id[6] = (id[6] & std::byte{ 0b00001111_u8 }) | std::byte{ 0b01000000_u8 }; // version 4 (random)

			// "Set the two most significant bits (bits 6 and 7) of the
			// clock_seq_hi_and_reserved to zero and one, respectively."
			id[8] = (id[8] & std::byte{ 0b00111111_u8 }) | std::byte{ 0b10000000_u8 }; // variant (standard)
		}
#endif
	}

	void MUU_CALLCONV muu_impl_uuid_generate_named(std::byte* id,
												   const uuid* name_space,
												   const void* name_data,
												   size_t name_size) noexcept
	{
		MUU_ASSUME(id);
		MUU_ASSUME(name_space);

		// hash the two values with SHA1 and use them as the initial value for the uuid
		{
			sha1 hasher;
			hasher(name_space, sizeof(uuid));
			if (name_data && name_size)
				hasher(name_data, name_size);
			hasher.finish();
			MUU_MEMCPY(id, &hasher.value(), 16_sz);
		}

		// "Set the four most significant bits (bits 12 through 15) of the
		// time_hi_and_version field to the 4-bit version number."
		id[6] = (id[6] & std::byte{ 0b00001111_u8 }) | std::byte{ 0b01010000_u8 }; // version 5 (SHA1)

		// "Set the two most significant bits (bits 6 and 7) of the
		// clock_seq_hi_and_reserved to zero and one, respectively."
		id[8] = (id[8] & std::byte{ 0b00111111_u8 }) | std::byte{ 0b10000000_u8 }; // variant (standard)
	}
}
