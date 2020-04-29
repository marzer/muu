// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::uuid.
#pragma once
#include "../muu/common.h"

namespace muu
{
	/// \brief A 16-byte universally-unique identifier (UUID).
	///
	/// \see RFC 4122: https://www.ietf.org/rfc/rfc4122.txt
	struct uuid
	{
		/// \brief The raw bytes in the UUID.
		/// \detail The byte layout of the UUID is big-endian regardless of the target platform:
		/// <br>` [ 0] ` time_low - most significant byte
		/// <br>` [ 1] ` ...
		/// <br>` [ 2] ` ...
		/// <br>` [ 3] ` time_low - least significant byte
		/// <br>` [ 4] ` time_mid - most significante byte
		/// <br>` [ 5] ` time_mid - least significante byte
		/// <br>` [ 6] ` time_hi_and_version - most significante byte
		/// <br>` [ 7] ` time_hi_and_version - least significante byte
		/// <br>` [ 8] ` clock_seq_and_reserved
		/// <br>` [ 9] ` clock_seq_low
		/// <br>` [10] ` node - most significant byte
		/// <br>` [11] ` ...
		/// <br>` [12] ` ...
		/// <br>` [13] ` ...
		/// <br>` [14] ` ...
		/// <br>` [15] ` node - least significant byte
		std::byte bytes[16];
		
		/// \brief Creates a new UUID using the platform's UUID generator.
		static MUU_API uuid generate() noexcept;

		/// \brief Default constructor. Leaves the UUID uninitialized.
		uuid() noexcept = default;

		/// \brief	Creates a UUID from it's raw integral components, as per RFC 4122.
		///
		/// \param	time_low				The "time-low" component.
		/// \param	time_mid				The "time-middle" component.
		/// \param	time_hi_and_version		The "time-high-and-version" component.
		/// \param	clock_seq_and_reserved	The "clock-seq-and-reserved" component.
		/// \param	clock_seq_low			The "clock-seq-low" component.
		/// \param	node					The "node" component. The highest 2 bytes are ignored
		/// 								(the `node` component of a UUID is a 48-bit unsigned int).
		///
		/// \remarks Despite the UUID's byte storage being big-endian,
		/// 		 inputs are assumed to be platform-endian and are re-ordered as required.
		constexpr uuid(
				uint32_t time_low,
				uint16_t time_mid,
				uint16_t time_hi_and_version,
				uint8_t clock_seq_and_reserved,
				uint8_t clock_seq_low,
				uint64_t node) noexcept
			: bytes
			{
				std::byte{ select_byte<build::is_little_endian ? 3 : 0>(time_low) },
				std::byte{ select_byte<build::is_little_endian ? 2 : 1>(time_low) },
				std::byte{ select_byte<build::is_little_endian ? 1 : 2>(time_low) },
				std::byte{ select_byte<build::is_little_endian ? 0 : 3>(time_low) },
				std::byte{ select_byte<build::is_little_endian ? 1 : 0>(time_mid) },
				std::byte{ select_byte<build::is_little_endian ? 0 : 1>(time_mid) },
				std::byte{ select_byte<build::is_little_endian ? 1 : 0>(time_hi_and_version) },
				std::byte{ select_byte<build::is_little_endian ? 0 : 1>(time_hi_and_version) },
				std::byte{ clock_seq_and_reserved },
				std::byte{ clock_seq_low },
				std::byte{ select_byte<build::is_little_endian ? 5 : 2>(node) },
				std::byte{ select_byte<build::is_little_endian ? 4 : 3>(node) },
				std::byte{ select_byte<build::is_little_endian ? 3 : 4>(node) },
				std::byte{ select_byte<build::is_little_endian ? 2 : 5>(node) },
				std::byte{ select_byte<build::is_little_endian ? 1 : 6>(node) },
				std::byte{ select_byte<build::is_little_endian ? 0 : 7>(node) }
			}
		{}

		/// \brief	Creats a UUID from it's raw integral components, as per RFC 4122.
		///
		/// \param	time_low				The "time-low" component.
		/// \param	time_mid				The "time-middle" component.
		/// \param	time_hi_and_version		The "time-high-and-version" component.
		/// \param	clock_seq				The "clock-seq-and-reserved" (MSB) and "clock-seq-low" (LSB) components.
		/// \param	node					The "node" component. The highest 2 bytes are ignored
		/// 								(the `node` component of a UUID is a 48-bit unsigned int).
		///
		/// \remarks Despite the UUID's byte storage being big-endian,
		/// 		 inputs are assumed to be platform-endian and are re-ordered as required.
		constexpr uuid(uint32_t time_low,
				uint16_t time_mid,
				uint16_t time_hi_and_version,
				uint16_t clock_seq,
				uint64_t node) noexcept
			: bytes
			{
				std::byte{ select_byte<build::is_little_endian ? 3 : 0>(time_low) },
				std::byte{ select_byte<build::is_little_endian ? 2 : 1>(time_low) },
				std::byte{ select_byte<build::is_little_endian ? 1 : 2>(time_low) },
				std::byte{ select_byte<build::is_little_endian ? 0 : 3>(time_low) },
				std::byte{ select_byte<build::is_little_endian ? 1 : 0>(time_mid) },
				std::byte{ select_byte<build::is_little_endian ? 0 : 1>(time_mid) },
				std::byte{ select_byte<build::is_little_endian ? 1 : 0>(time_hi_and_version) },
				std::byte{ select_byte<build::is_little_endian ? 0 : 1>(time_hi_and_version) },
				std::byte{ select_byte<build::is_little_endian ? 1 : 0>(clock_seq) },
				std::byte{ select_byte<build::is_little_endian ? 0 : 1>(clock_seq) },
				std::byte{ select_byte<build::is_little_endian ? 5 : 2>(node) },
				std::byte{ select_byte<build::is_little_endian ? 4 : 3>(node) },
				std::byte{ select_byte<build::is_little_endian ? 3 : 4>(node) },
				std::byte{ select_byte<build::is_little_endian ? 2 : 5>(node) },
				std::byte{ select_byte<build::is_little_endian ? 1 : 6>(node) },
				std::byte{ select_byte<build::is_little_endian ? 0 : 7>(node) }
			}
		{}

		#if 0
		private:

			Uuid(const Uuid&, const std::byte*, size_t) noexcept;

		public:

		/// \brief	Creates a 'named' UUID from a namespace ID and a name.
		///
		/// \param	idNamespace	The 'namespace' for the id.
		/// \param	nameData  	The bytes used for the 'name' of the ID.
		/// 
		/// \remarks IDs generated by this constructor are deterministic; the same namespace ID and name will always produce the same ID.
		Uuid(const Uuid& idNamespace, ConstByteView nameData) noexcept
			: Uuid{ idNamespace, nameData.Data(), nameData.Size() }
		{}

		/// \brief	Creates a 'named' UUID from a namespace ID and a name.
		///
		/// \param	idNamespace	The 'namespace' for the id.
		/// \param	name  		The 'name' of the ID.
		/// 
		/// \remarks IDs generated by this constructor are deterministic; the same namespace ID and name will always produce the same ID.
		Uuid(const Uuid& idNamespace, std::string_view name) noexcept
			: Uuid{ idNamespace, PointerCast<const std::byte*>(name.data()), name.length() }
		{}

		/// \brief	Tries to parse a UUID from a string.
		///
		/// \tparam	STR		A string type.
		/// \param	str   	The string to parse.
		///
		/// \return	A UUID containing the parsed value if the parse operation was successful, or an empty std::optional if parsing failed.
		template <typename STR, typename = OnlyIf<Traits::IsString<STR>::value>>
		[[nodiscard]]
		static constexpr std::optional<Uuid> Parse(STR && str) noexcept
		{
			using char_t = Traits::CharacterOf<STR>;
			using view_t = Traits::StringViewOf<STR>;
			using chars = Constants<char_t>;

			//get rid of valid prefix and suffix characters (whitespace, curly brackets)
			view_t view = TrimLeft(View(std::forward<STR>(str)), [](char_t c) noexcept
			{
				return IsWhitespace(c) || c == chars::LeftBrace;
			});
			view = TrimRight(view, [](char_t c) noexcept
			{
				return IsWhitespace(c) || c == chars::RightBrace;
			});
			if (view.length() < 32_sz) //two hex characters per byte, ignoring punctuation etc.
				return {};

			//check that the trimmed string begins and ends with hexadecimal digits
			if (!IsHexadecimal(view.front()) || !IsHexadecimal(view.back()))
				return {};

			//check that the trimmed string doesn't contain invalid characters
			if (Find(view, [](char_t c) noexcept { return !IsWhitespace(c) && !IsHexadecimal(c) && c != chars::Comma && c != chars::Hyphen; }))
				return {};

			Uuid outVal{};
			char_t last{};
			size_t index{};
			for (char_t ch : view)
			{
				if (!IsHexadecimal(ch)) //whitespace/comma/hyphen
				{
					if (last)
						return {}; //don't allow octet splicing
					continue;
				}
				if (index >= 16_sz)
					return {};
				if (!last)
					last = ch;
				else
				{
					outVal.Bytes[index++] = std::byte{ ToByte((*HexToByte(last) << 4) | *HexToByte(ch)) };
					last = char_t{};
				}
			}
			if (index == 16_sz)
				return outVal;
			return {};
		}
		#endif

		/// \brief	Returns true if this UUID has non-zero value.
		[[nodiscard]]
		explicit constexpr operator bool() const noexcept
		{
			for (size_t i = 0; i < 16_sz; i++)
				if (bytes[i] != std::byte{})
					return true;
			return false;
		}

		/// \brief	Returns the lexicographical ordering of two UUID's.
		[[nodiscard]]
		static constexpr int32_t compare(const uuid& lhs, const uuid& rhs) noexcept
		{
			if (&lhs == &rhs)
				return 0;
			for (size_t i = 0; i < 16_sz; i++)
			{
				if (lhs.bytes[i] != rhs.bytes[i])
					return lhs.bytes[i] < rhs.bytes[i] ? -1 : 1;
			}
			return 0;
		}

		/// \brief	Returns true if two UUIDs are equal.
		[[nodiscard]]
		friend constexpr bool operator == (const uuid& lhs, const uuid& rhs) noexcept
		{
			return compare(lhs, rhs) == 0;
		}

		/// \brief	Returns true if two UUIDs are not equal.
		[[nodiscard]]
		friend constexpr bool operator != (const uuid& lhs, const uuid& rhs) noexcept
		{
			return compare(lhs, rhs) != 0;
		}

		/// \brief	Returns true if the LHS UUID is ordered lower than the RHS UUID.
		[[nodiscard]]
		friend constexpr bool operator < (const uuid& lhs, const uuid& rhs) noexcept
		{
			return compare(lhs, rhs) < 0;
		}

		/// \brief	Returns true if the LHS UUID is ordered lower than or equal to the RHS UUID.
		[[nodiscard]]
		friend constexpr bool operator <= (const uuid& lhs, const uuid& rhs) noexcept
		{
			return compare(lhs, rhs) <= 0;
		}

		/// \brief	Returns true if the LHS UUID is ordered higher than the RHS UUID.
		[[nodiscard]]
		friend constexpr bool operator > (const uuid& lhs, const uuid& rhs) noexcept
		{
			return compare(lhs, rhs) > 0;
		}

		/// \brief	Returns true if the LHS UUID is ordered higher than or equal to the RHS UUID.
		[[nodiscard]]
		friend constexpr bool operator >= (const uuid& lhs, const uuid& rhs) noexcept
		{
			return compare(lhs, rhs) >= 0;
		}
	};
}

#if MUU_IMPLEMENTATION

MUU_PUSH_WARNINGS
MUU_DISABLE_ALL_WARNINGS
#ifdef _WIN32
	#include <rpc.h>
	MUU_PRAGMA_MSVC(comment(lib, "rpcrt4.lib"))
#else
	#include <uuid/uuid.h>
#endif
MUU_POP_WARNINGS

namespace muu
{
	MUU_EXTERNAL_LINKAGE
	MUU_API
	uuid uuid::generate() noexcept
	{
		uuid val;
		#ifdef _WIN32
		{
			UUID native;
			UuidCreate(&native);
			val.bytes[0] = std::byte{ select_byte<build::is_little_endian ? 3 : 0>(native.Data1) };
			val.bytes[1] = std::byte{ select_byte<build::is_little_endian ? 2 : 1>(native.Data1) };
			val.bytes[2] = std::byte{ select_byte<build::is_little_endian ? 1 : 2>(native.Data1) };
			val.bytes[3] = std::byte{ select_byte<build::is_little_endian ? 0 : 3>(native.Data1) };
			val.bytes[4] = std::byte{ select_byte<build::is_little_endian ? 1 : 0>(native.Data2) };
			val.bytes[5] = std::byte{ select_byte<build::is_little_endian ? 0 : 1>(native.Data2) };
			val.bytes[6] = std::byte{ select_byte<build::is_little_endian ? 1 : 0>(native.Data3) };
			val.bytes[7] = std::byte{ select_byte<build::is_little_endian ? 0 : 1>(native.Data3) };
			memcpy(&val.bytes[8], native.Data4, 8);
		}
		#else
		{
			uuid_t native; //unsigned char[16]
			uuid_generate(native);
			static_assert(sizeof(uuid_t) == sizeof(val));
			memcpy(&val, native, sizeof(val));
		}
		#endif
		return val;
	}
}

#endif // MUU_IMPLEMENTATION
