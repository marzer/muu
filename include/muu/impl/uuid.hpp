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

MUU_PUSH_WARNINGS
MUU_DISABLE_ALL_WARNINGS
#if MUU_WINDOWS
	#include <rpc.h>
	MUU_PRAGMA_MSVC(comment(lib, "rpcrt4.lib"))
#else
	#include <cstdlib>
#endif
MUU_POP_WARNINGS

MUU_NAMESPACE_START
{
	MUU_EXTERNAL_LINKAGE
	MUU_API
	uuid uuid::generate() noexcept
	{
		uuid val;
		#if MUU_WINDOWS
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
			memcpy(&val.bytes, &native, sizeof(UUID));
		}
		#else
		{
			// generate a version 4 uuid as per https://www.cryptosys.net/pki/uuid-rfc4122.html
			for (auto& b : val.bytes)
				b = static_cast<std::byte>(rand() % 256);
			val.bytes[6] = (val.bytes[6] & 0b00001111_byte) | 0b10000000_byte;
			val.bytes[8] = (val.bytes[8] & 0b00111111_byte) | 0b10000000_byte;

		}
		#endif
		return val;
	}
}
MUU_NAMESPACE_END
