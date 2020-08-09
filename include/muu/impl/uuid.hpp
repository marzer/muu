// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "../../muu/preprocessor.h"
#if !MUU_IMPLEMENTATION
	#error This is an implementation-only header.
#endif

#include "../../muu/uuid.h"
#include "../../muu/hashing.h"
#if MUU_MSVC
	MUU_DISABLE_WARNINGS
	#include <rpc.h>
	MUU_PRAGMA_MSVC(comment(lib, "rpcrt4.lib"))
	MUU_ENABLE_WARNINGS
#else
	#include "../../muu/impl/random.hpp"
#endif

MUU_NAMESPACE_START
{
	MUU_EXTERNAL_LINKAGE
	MUU_API
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
				b = static_cast<std::byte>(dist(impl::mersenne_twister()));

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

	MUU_EXTERNAL_LINKAGE
	MUU_API
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
}
MUU_NAMESPACE_END
