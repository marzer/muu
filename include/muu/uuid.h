// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::uuid.

#pragma once
#include "../muu/hashing.h"

MUU_IMPL_NAMESPACE_START
{
	using uuid_bytes = std::byte[16];

	namespace MUU_EVAL(MUU_BIG_ENDIAN, be, le)
	{
		template <unsigned>
		struct uuid_slicer;

		template <>
		struct uuid_slicer<2>
		{
			[[nodiscard]]
			MUU_ATTR(pure)
			static constexpr uint16_t slice(const uuid_bytes& byte_arr, unsigned first) noexcept
			{
				MUU_ASSUME((first + 2u) <= 16u);
				if constexpr (build::is_little_endian)
					return pack(byte_arr[first], byte_arr[first + 1]);
				else
					return pack(byte_arr[first + 1], byte_arr[first]);
			}
		};

		template <>
		struct uuid_slicer<4>
		{
			[[nodiscard]]
			MUU_ATTR(pure)
			static constexpr uint32_t slice(const uuid_bytes& byte_arr, unsigned first) noexcept
			{
				MUU_ASSUME((first + 4u) <= 16u);
				if constexpr (build::is_little_endian)
					return pack(byte_arr[first], byte_arr[first + 1], byte_arr[first + 2], byte_arr[first + 3]);
				else
					return pack(byte_arr[first + 3], byte_arr[first + 2], byte_arr[first + 1], byte_arr[first]);
			}
		};

		template <>
		struct uuid_slicer<8>
		{
			[[nodiscard]]
			MUU_ATTR(pure)
			static constexpr uint64_t slice(const uuid_bytes& byte_arr, unsigned first) noexcept
			{
				MUU_ASSUME((first + 8u) <= 16u);
				if constexpr (build::is_little_endian)
					return pack(
						byte_arr[first], byte_arr[first + 1], byte_arr[first + 2], byte_arr[first + 3],
						byte_arr[first + 4], byte_arr[first + 5], byte_arr[first + 6], byte_arr[first + 7]
					);
				else
					return pack(
						byte_arr[first + 7], byte_arr[first + 6], byte_arr[first + 5], byte_arr[first + 4],
						byte_arr[first + 3], byte_arr[first + 2], byte_arr[first + 1], byte_arr[first]
					);
			}
		};
	} // be/le

	template <unsigned N>
	MUU_ATTR(pure)
	MUU_ATTR(flatten)
	constexpr auto uuid_slice(const uuid_bytes& byte_arr, unsigned first) noexcept
	{
		using slicer = MUU_EVAL(MUU_BIG_ENDIAN, be, le)::uuid_slicer<N>;
		return slicer::slice(byte_arr, first);
	}
}
MUU_IMPL_NAMESPACE_END

MUU_NAMESPACE_START
{


	/// \brief	A UUID variant as per RFC 4122.
	///
	/// \see RFC 4122: https://www.ietf.org/rfc/rfc4122.txt
	enum class uuid_variant : uint8_t
	{
		none,				///< No variant specified.
		standard,			///< The 'standard' variant outlaid in RFC 4122.
		reserved_ncs,		///< Reserved for backward compatibility.
		reserved_microsoft,	///< Reserved for backward compatibility.
		reserved_future,	///< Reserved for backward compatibility.
	};

	/// \brief	A UUID version as per RFC 4122.
	///
	/// \see RFC 4122: https://www.ietf.org/rfc/rfc4122.txt
	enum class uuid_version : uint8_t
	{
		none,		///< No version specified.
		time,		///< Version 1: Time-based UUID.
		dce,		///< Version 2: DCE Security version, with embedded POSIX UIDs.
		name_md5,	///< Version 3: Name-based UUID generated using MD5 hashing.
		random,		///< Version 4: Randomly-generated UUID.
		name_sha1,	///< Version 5: Name-based UUID generated using SHA-1 hashing.
		unknown	= 0b1111	///< Unknown or non-standard version.
	};

	/// \brief A 16-byte universally-unique identifier (UUID).
	///
	/// \see RFC 4122: https://www.ietf.org/rfc/rfc4122.txt
	struct uuid
	{
		/// \brief The raw bytes in the UUID.
		/// \detail The byte layout of the UUID is big-endian, regardless of the target platform:
		/// <br>
		/// <br>`[ 0]`&nbsp;&nbsp; time_low - most significant byte
		/// <br>`[ 1]`&nbsp;&nbsp; ...
		/// <br>`[ 2]`&nbsp;&nbsp; ...
		/// <br>`[ 3]`&nbsp;&nbsp; time_low - least significant byte
		/// <br>`[ 4]`&nbsp;&nbsp; time_mid - most significante byte
		/// <br>`[ 5]`&nbsp;&nbsp; time_mid - least significante byte
		/// <br>`[ 6]`&nbsp;&nbsp; time_hi_and_version - most significante byte
		/// <br>`[ 7]`&nbsp;&nbsp; time_hi_and_version - least significante byte
		/// <br>`[ 8]`&nbsp;&nbsp; clock_seq_and_reserved
		/// <br>`[ 9]`&nbsp;&nbsp; clock_seq_low
		/// <br>`[10]`&nbsp;&nbsp; node - most significant byte
		/// <br>`[11]`&nbsp;&nbsp; ...
		/// <br>`[12]`&nbsp;&nbsp; ...
		/// <br>`[13]`&nbsp;&nbsp; ...
		/// <br>`[14]`&nbsp;&nbsp; ...
		/// <br>`[15]`&nbsp;&nbsp; node - least significant byte
		/// <br><br>Relevent excerpt from the UUID rfc:
		/// \out
		/// In the absence of explicit application or presentation protocol
		/// specification to the contrary, a UUID is encoded as a 128-bit object,
		/// as follows:
		/// 
		/// The fields are encoded as 16 octets, with the sizes and order of the
		/// fields defined above, and with each field encoded with the Most
		/// Significant Byte first (known as network byte order).  Note that the
		/// field names, particularly for multiplexed fields, follow historical
		/// practice.
		/// 
		/// 0                   1                   2                   3
		///  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		/// |                          time_low                             |
		/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		/// |       time_mid                |      time_high_and_version    |
		/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		/// |clk_seq_hi_res |  clk_seq_low  |         node (0-1)            |
		/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		/// |                         node (2-5)                            |
		/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		/// \eout
		/// 
		/// \see RFC 4122: https://www.ietf.org/rfc/rfc4122.txt
		std::byte bytes[16];
		
		/// \brief Creates a new UUID using the platform's UUID generator.
		static MUU_API uuid generate() noexcept;

		/// \brief Default constructor. Leaves the UUID uninitialized.
		uuid() noexcept = default;

		/// \brief	Creates a UUID from it's raw integral components, as per RFC 4122.
		///
		/// \param	time_low						The 'time-low' field.
		/// \param	time_mid						The 'time-middle' field.
		/// \param	time_high_and_version			The 'time-high-and-version' field.
		/// \param	clock_seq_high_and_reserved		The 'clock-seq-high-and-reserved' field.
		/// \param	clock_seq_low					The 'clock-seq-low' field.
		/// \param	node							The 'node' field. The highest 2 bytes are ignored
		/// 										(the 'node' field of a UUID is a 48-bit unsigned int).
		///
		/// \remarks Despite the UUID's byte storage being big-endian,
		/// 		 inputs are assumed to be platform-endian and are re-ordered as required.
		constexpr uuid(
				uint32_t time_low,
				uint16_t time_mid,
				uint16_t time_high_and_version,
				uint8_t clock_seq_high_and_reserved,
				uint8_t clock_seq_low,
				uint64_t node) noexcept
			: bytes
			{
				std::byte{ byte_select<build::is_little_endian ? 3 : 0>(time_low) },
				std::byte{ byte_select<build::is_little_endian ? 2 : 1>(time_low) },
				std::byte{ byte_select<build::is_little_endian ? 1 : 2>(time_low) },
				std::byte{ byte_select<build::is_little_endian ? 0 : 3>(time_low) },
				std::byte{ byte_select<build::is_little_endian ? 1 : 0>(time_mid) },
				std::byte{ byte_select<build::is_little_endian ? 0 : 1>(time_mid) },
				std::byte{ byte_select<build::is_little_endian ? 1 : 0>(time_high_and_version) },
				std::byte{ byte_select<build::is_little_endian ? 0 : 1>(time_high_and_version) },
				std::byte{ clock_seq_high_and_reserved },
				std::byte{ clock_seq_low },
				std::byte{ byte_select<build::is_little_endian ? 5 : 2>(node) },
				std::byte{ byte_select<build::is_little_endian ? 4 : 3>(node) },
				std::byte{ byte_select<build::is_little_endian ? 3 : 4>(node) },
				std::byte{ byte_select<build::is_little_endian ? 2 : 5>(node) },
				std::byte{ byte_select<build::is_little_endian ? 1 : 6>(node) },
				std::byte{ byte_select<build::is_little_endian ? 0 : 7>(node) }
			}
		{}

		/// \brief	Creats a UUID from it's raw integral components, as per RFC 4122.
		///
		/// \param	time_low				The 'time-low' field.
		/// \param	time_mid				The 'time-middle' field.
		/// \param	time_high_and_version	The 'time-high-and-version' field.
		/// \param	clock_seq				The 'clock-seq-high-and-reserved' (MSB) and 'clock-seq-low' (LSB) fields.
		/// \param	node					The 'node' field. The highest 2 bytes are ignored
		/// 								(the 'node' field of a UUID is a 48-bit unsigned int).
		///
		/// \remarks Despite the UUID's byte storage being big-endian,
		/// 		 inputs are assumed to be platform-endian and are re-ordered as required.
		constexpr uuid(uint32_t time_low,
				uint16_t time_mid,
				uint16_t time_high_and_version,
				uint16_t clock_seq,
				uint64_t node) noexcept
			: bytes
			{
				std::byte{ byte_select<build::is_little_endian ? 3 : 0>(time_low) },
				std::byte{ byte_select<build::is_little_endian ? 2 : 1>(time_low) },
				std::byte{ byte_select<build::is_little_endian ? 1 : 2>(time_low) },
				std::byte{ byte_select<build::is_little_endian ? 0 : 3>(time_low) },
				std::byte{ byte_select<build::is_little_endian ? 1 : 0>(time_mid) },
				std::byte{ byte_select<build::is_little_endian ? 0 : 1>(time_mid) },
				std::byte{ byte_select<build::is_little_endian ? 1 : 0>(time_high_and_version) },
				std::byte{ byte_select<build::is_little_endian ? 0 : 1>(time_high_and_version) },
				std::byte{ byte_select<build::is_little_endian ? 1 : 0>(clock_seq) },
				std::byte{ byte_select<build::is_little_endian ? 0 : 1>(clock_seq) },
				std::byte{ byte_select<build::is_little_endian ? 5 : 2>(node) },
				std::byte{ byte_select<build::is_little_endian ? 4 : 3>(node) },
				std::byte{ byte_select<build::is_little_endian ? 3 : 4>(node) },
				std::byte{ byte_select<build::is_little_endian ? 2 : 5>(node) },
				std::byte{ byte_select<build::is_little_endian ? 1 : 6>(node) },
				std::byte{ byte_select<build::is_little_endian ? 0 : 7>(node) }
			}
		{}

		/// \brief	Explicitly constructs a null UUID.
		constexpr uuid(std::nullptr_t) noexcept
			: bytes
			{
				std::byte{}, std::byte{}, std::byte{}, std::byte{},
				std::byte{}, std::byte{}, std::byte{}, std::byte{},
				std::byte{}, std::byte{}, std::byte{}, std::byte{},
				std::byte{}, std::byte{}, std::byte{}, std::byte{}
			}
		{}

		/// \brief	Returns the UUID's variant.
		[[nodiscard]]
		constexpr uuid_variant variant() const noexcept
		{
			const auto var = static_cast<unsigned>(unwrap((bytes[8] & 0b11100000_byte) >> 5));
			MUU_ASSUME(var <= 0b111u);

			if (!var)
				return uuid_variant::none;
			else if MUU_UNLIKELY(var <= 0b011u) // 0 x x
				return uuid_variant::reserved_ncs;
			else
			{
				// any possible variant in this branch has the high bit set, i.e. 0b1XX
				
				if MUU_LIKELY((var | 0b101u) == 0b101u)	// 1 0 x
					return uuid_variant::standard;
				else if (var == 0b110) // 1 1 0
					return uuid_variant::reserved_microsoft;
				else // 1 1 1
					return uuid_variant::reserved_future;
			}
		}

		/// \brief	Returns the UUID's version.
		[[nodiscard]]
		constexpr uuid_version version() const noexcept
		{
			const auto var = static_cast<unsigned>(unwrap((bytes[6] & 0b11110000_byte) >> 4));
			MUU_ASSUME(var <= 0b1111u);

			return var > 5u ? uuid_version::unknown : static_cast<uuid_version>(var);
		}

		/// \brief	Returns the value of the 'time-low' field.
		[[nodiscard]]
		constexpr uint32_t time_low() const noexcept
		{
			return impl::uuid_slice<4>(bytes, 0);
		}

		/// \brief	Returns the value of the 'time-mid' field.
		[[nodiscard]]
		constexpr uint16_t time_mid() const noexcept
		{
			return impl::uuid_slice<2>(bytes, 4);
		}

		/// \brief	Returns the value of the 'time-high-and-version' field.
		[[nodiscard]]
		constexpr uint16_t time_high_and_version() const noexcept
		{
			return impl::uuid_slice<2>(bytes, 6);
		}

		/// \brief	Returns the value of the 'clock-seq-high-and-reserved' field.
		[[nodiscard]]
		constexpr uint8_t clock_seq_high_and_reserved() const noexcept
		{
			return unwrap(bytes[8]);
		}

		/// \brief	Returns the value of the 'clock-seq-low' field.
		[[nodiscard]]
		constexpr uint8_t clock_seq_low() const noexcept
		{
			return unwrap(bytes[9]);
		}

		/// \brief	Returns the value of the 'node' field.
		/// 
		/// \remarks The two most significant bytes of the return value will be zero
		/// 		 (the 'node' field of a UUID is a 48-bit unsigned int).
		[[nodiscard]]
		constexpr uint64_t node() const noexcept
		{
			return impl::uuid_slice<8>(bytes, 8) & 0x0000FFFFFFFFFFFF_u64;
		}

		#if 0
		private:

			Uuid(const Uuid&, const byte*, size_t) noexcept;

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
			: Uuid{ idNamespace, PointerCast<const byte*>(name.data()), name.length() }
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

		/// \brief	Returns true if two UUIDs are equal.
		[[nodiscard]]
		friend constexpr bool operator == (const uuid& lhs, const uuid& rhs) noexcept
		{
			if (&lhs == &rhs)
				return true;
			for (size_t i = 0; i < 16_sz; i++)
			{
				if (lhs.bytes[i] != rhs.bytes[i])
					return false;
			}
			return true;
		}

		/// \brief	Returns true if two UUIDs are not equal.
		[[nodiscard]]
		friend constexpr bool operator != (const uuid& lhs, const uuid& rhs) noexcept
		{
			return !(lhs == rhs);
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
MUU_NAMESPACE_END

namespace std
{
	/// \brief	Specialization of std::hash for muu::uuid.
	template <>
	struct hash<MUU_NAMESPACE::uuid>
	{
		[[nodiscard]]
		MUU_API
		constexpr size_t operator()(const MUU_NAMESPACE::uuid& id) const noexcept
		{
			using namespace MUU_NAMESPACE;

			#define get_slice(idx)	\
				impl::uuid_slice<sizeof(size_t)>(id.bytes, sizeof(size_t) * idx)

			hash_combiner<> hasher{ get_slice(0) };
			if constexpr (sizeof(size_t) < 16)
				hasher(get_slice(1));
			if constexpr (sizeof(size_t) < 8)
			{
				hasher(get_slice(2));
				hasher(get_slice(3));
			}
			if constexpr (sizeof(size_t) < 4)
			{
				hasher(get_slice(4));
				hasher(get_slice(5));
				hasher(get_slice(6));
				hasher(get_slice(7));
			}
			return hasher.value();

			#undef get_slice
		}
	};
}


#if MUU_IMPLEMENTATION
	#include "../muu/impl/uuid.hpp"
#endif
