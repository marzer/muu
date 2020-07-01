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

namespace muu
{
	MUU_EXTERNAL_LINKAGE
	MUU_API
	uuid uuid::generate() noexcept
	{
		uuid val;
		#if MUU_WINDOWS
		{
			UUID native;
			UuidCreate(&native);
			val.bytes[0] = std::byte{ byte_select<build::is_little_endian ? 3 : 0>(native.Data1) };
			val.bytes[1] = std::byte{ byte_select<build::is_little_endian ? 2 : 1>(native.Data1) };
			val.bytes[2] = std::byte{ byte_select<build::is_little_endian ? 1 : 2>(native.Data1) };
			val.bytes[3] = std::byte{ byte_select<build::is_little_endian ? 0 : 3>(native.Data1) };
			val.bytes[4] = std::byte{ byte_select<build::is_little_endian ? 1 : 0>(native.Data2) };
			val.bytes[5] = std::byte{ byte_select<build::is_little_endian ? 0 : 1>(native.Data2) };
			val.bytes[6] = std::byte{ byte_select<build::is_little_endian ? 1 : 0>(native.Data3) };
			val.bytes[7] = std::byte{ byte_select<build::is_little_endian ? 0 : 1>(native.Data3) };
			memcpy(&val.bytes[8], native.Data4, 8);
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
