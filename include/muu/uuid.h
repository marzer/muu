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
	/// \see RFC 4122: https://tools.ietf.org/html/rfc4122
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
	/// \see RFC 4122: https://tools.ietf.org/html/rfc4122
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
	/// \see RFC 4122: https://tools.ietf.org/html/rfc4122
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
		/// \see RFC 4122: https://tools.ietf.org/html/rfc4122
		struct uuid_data { std::byte bytes[16]; } data;
		
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
		MUU_NODISCARD_CTOR
		constexpr uuid(
				uint32_t time_low,
				uint16_t time_mid,
				uint16_t time_high_and_version,
				uint8_t clock_seq_high_and_reserved,
				uint8_t clock_seq_low,
				uint64_t node) noexcept
			: data
			{{
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
			}}
		{}

		/// \brief	Creats a UUID from it's raw integral components, as per RFC 4122.
		///
		/// \param	time_low				The 'time-low' field.
		/// \param	time_mid				The 'time-middle' field.
		/// \param	time_high_and_version	The 'time-high-and-version' field.
		/// \param	clock_seq				The 'clock-seq-high-and-reserved' (MSB) and 'clock-seq-low' (LSB) fields.
		/// \param	node					The 'node' field. The highest 2 bytes are ignored
		/// 								(the 'node' field of a UUID is a 48-bit unsigned int).
		MUU_NODISCARD_CTOR
		constexpr uuid(uint32_t time_low,
				uint16_t time_mid,
				uint16_t time_high_and_version,
				uint16_t clock_seq,
				uint64_t node) noexcept
			: data
			{{
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
			}}
		{}

		/// \brief	Explicitly constructs a null UUID.
		MUU_NODISCARD_CTOR
		constexpr uuid(std::nullptr_t) noexcept
			: data
			{{
				std::byte{}, std::byte{}, std::byte{}, std::byte{},
				std::byte{}, std::byte{}, std::byte{}, std::byte{},
				std::byte{}, std::byte{}, std::byte{}, std::byte{},
				std::byte{}, std::byte{}, std::byte{}, std::byte{}
			}}
		{}

		#if MUU_HAS_INT128 || MUU_DOXYGEN

		/// \brief	Constructs a UUID directly from a 128-bit integer.
		/// 
		/// \param 	val	The value to convert into a UUID.
		/// 
		/// \attention This constructor is only available when MUU_HAS_INT128 is `1`.
		MUU_NODISCARD_CTOR
		constexpr uuid(uint128_t val) noexcept
			#if MUU_BIG_ENDIAN
				: data{ bit_cast<decltype(data)>(val) }
			#else
				: data{ bit_cast<decltype(data)>(byte_reverse(val)) }
			#endif
		{}

		#endif

		/// \brief	Constructs a version-5 named UUID by hashing some binary data.
		///
		/// \param 	name_space		The 'name space' the ID will belong to.
		/// \param 	name_data		The 'name' data to hash.
		/// \param 	name_size	  	Size of the name data in bytes.
		/// 
		/// \remarks UUIDs generated by this constructor are deterministic;
		/// 		 the same namespace and name will always produce the same UUID.
		/// 
		/// \see RFC 4122 Section 4.3 'Algorithm for Creating a Name-Based UUID': https://tools.ietf.org/html/rfc4122#section-4.3
		MUU_NODISCARD_CTOR
		MUU_API
		uuid(const uuid& name_space, const void* name_data, size_t name_size) noexcept;

		/// \brief	Constructs a version-5 named UUID by hashing a string.
		///
		/// \param	idNamespace	The 'namespace' for the id.
		/// \param	name  		The 'name' of the ID.
		/// 
		/// \remarks UUIDs generated by this constructor are deterministic;
		/// 		 the same namespace and name will always produce the same UUID.
		/// 
		/// \see RFC 4122 Section 4.3 'Algorithm for Creating a Name-Based UUID': https://tools.ietf.org/html/rfc4122#section-4.3
		template <typename Char, typename Traits>
		MUU_NODISCARD_CTOR
		uuid(const uuid& name_space, std::basic_string_view<Char, Traits> name) noexcept
			: uuid{ name_space, name.data(), name.length() }
		{}

		/// \brief	Returns the UUID's variant.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr uuid_variant variant() const noexcept
		{
			const auto var = static_cast<unsigned>(unwrap((data.bytes[8] & 0b11100000_byte) >> 5));
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
		MUU_ATTR(pure)
		constexpr uuid_version version() const noexcept
		{
			const auto var = static_cast<unsigned>(unwrap((data.bytes[6] & 0b11110000_byte) >> 4));
			MUU_ASSUME(var <= 0b1111u);

			return var > 5u ? uuid_version::unknown : static_cast<uuid_version>(var);
		}

		/// \brief	Returns the value of the 'time-low' field.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr uint32_t time_low() const noexcept
		{
			return impl::uuid_slice<4>(data.bytes, 0);
		}

		/// \brief	Returns the value of the 'time-mid' field.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr uint16_t time_mid() const noexcept
		{
			return impl::uuid_slice<2>(data.bytes, 4);
		}

		/// \brief	Returns the value of the 'time-high-and-version' field.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr uint16_t time_high_and_version() const noexcept
		{
			return impl::uuid_slice<2>(data.bytes, 6);
		}

		/// \brief	Returns the value of the 'clock-seq-high-and-reserved' field.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr uint8_t clock_seq_high_and_reserved() const noexcept
		{
			return unwrap(data.bytes[8]);
		}

		/// \brief	Returns the value of the 'clock-seq-low' field.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr uint8_t clock_seq_low() const noexcept
		{
			return unwrap(data.bytes[9]);
		}

		/// \brief	Returns the value of the 'node' field.
		/// 
		/// \remarks The two most significant bytes of the return value will be zero
		/// 		 (the 'node' field of a UUID is a 48-bit unsigned int).
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr uint64_t node() const noexcept
		{
			return impl::uuid_slice<8>(data.bytes, 8) & 0x0000FFFFFFFFFFFF_u64;
		}

		#if 0

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

		#if MUU_HAS_INT128 || MUU_DOXYGEN

		/// \brief	Converts a UUID directly into a 128-bit integer.
		/// 
		/// \attention This conversion is only available when MUU_HAS_INT128 is `1`.
		[[nodiscard]]
		MUU_ATTR(pure)
		explicit constexpr operator uint128_t() const noexcept
		{
			if constexpr (build::supports_constexpr_bit_cast)
			{
				if constexpr (build::is_little_endian)
					return byte_reverse(bit_cast<uint128_t>(data.bytes));
				else
					return bit_cast<uint128_t>(data.bytes);
			}
			else
			{
				if constexpr (build::is_little_endian)
					return pack(
						data.bytes[0], data.bytes[1],  data.bytes[2],  data.bytes[3],
						data.bytes[4], data.bytes[5],  data.bytes[6],  data.bytes[7],
						data.bytes[8], data.bytes[9],  data.bytes[10], data.bytes[11],
						data.bytes[12],data.bytes[13], data.bytes[14], data.bytes[15]
					);
				else
					return pack(
						data.bytes[15], data.bytes[14], data.bytes[13], data.bytes[12],
						data.bytes[11], data.bytes[10], data.bytes[9],	data.bytes[8],
						data.bytes[7],  data.bytes[6],  data.bytes[5],	data.bytes[4],
						data.bytes[3],  data.bytes[2],  data.bytes[1],	data.bytes[0]
					);
			}
		}
		
		#endif

		/// \brief	Returns true if this UUID has non-zero value.
		[[nodiscard]]
		MUU_ATTR(pure)
		explicit constexpr operator bool() const noexcept
		{
			#if MUU_HAS_INT128
			if constexpr (build::supports_constexpr_bit_cast)
				return bit_cast<uint128_t>(data.bytes) != uint128_t{};
			else
			#endif
			{
				for (size_t i = 0; i < 16_sz; i++)
					if (data.bytes[i] != std::byte{})
						return true;
				return false;
			}
		}

		/// \brief	Returns true if two UUIDs are equal.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr bool operator == (const uuid& lhs, const uuid& rhs) noexcept
		{
			if (&lhs == &rhs)
				return true;

			#if MUU_HAS_INT128
			if constexpr (build::supports_constexpr_bit_cast)
				return bit_cast<uint128_t>(lhs) == bit_cast<uint128_t>(rhs);
			else
			#endif
			{
				for (size_t i = 0; i < 16_sz; i++)
				{
					if (lhs.data.bytes[i] != rhs.data.bytes[i])
						return false;
				}
				return true;
			}
		}

		/// \brief	Returns true if two UUIDs are not equal.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr bool operator != (const uuid& lhs, const uuid& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		/// \brief	Returns the lexicographical ordering of two UUID's.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr int32_t compare(const uuid& lhs, const uuid& rhs) noexcept
		{
			if (&lhs == &rhs)
				return 0;
			
			#if MUU_HAS_INT128
			{
				const auto l = static_cast<uint128_t>(lhs);
				const auto r = static_cast<uint128_t>(rhs);
				if (l != r)
					return l < r ? -1 : 1;
			}
			#else
			{
				for (size_t i = 0; i < 16_sz; i++)
				{
					if (lhs.data.bytes[i] != rhs.data.bytes[i])
						return lhs.data.bytes[i] < rhs.data.bytes[i] ? -1 : 1;
				}
			}
			#endif

			return 0;
		}

		/// \brief	Returns true if the LHS UUID is ordered lower than the RHS UUID.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr bool operator < (const uuid& lhs, const uuid& rhs) noexcept
		{
			return compare(lhs, rhs) < 0;
		}

		/// \brief	Returns true if the LHS UUID is ordered lower than or equal to the RHS UUID.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr bool operator <= (const uuid& lhs, const uuid& rhs) noexcept
		{
			return compare(lhs, rhs) <= 0;
		}

		/// \brief	Returns true if the LHS UUID is ordered higher than the RHS UUID.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr bool operator > (const uuid& lhs, const uuid& rhs) noexcept
		{
			return compare(lhs, rhs) > 0;
		}

		/// \brief	Returns true if the LHS UUID is ordered higher than or equal to the RHS UUID.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr bool operator >= (const uuid& lhs, const uuid& rhs) noexcept
		{
			return compare(lhs, rhs) >= 0;
		}
	};

	/// \brief	UUID constants.
	template <>
	struct constants<uuid>
	{
		/// \brief	The null UUID (all bytes are zero).
		static constexpr uuid null = uuid{ nullptr };

 		/// \brief	The UUID namespace for domain names (6BA7B810-9DAD-11D1-80B4-00C04FD430C8).
		static constexpr uuid namespace_dns = uuid{ 0x6BA7B810u, 0x9DAD_u16, 0x11D1_u16, 0x80B4_u16, 0x00C04FD430C8_u64 };

		/// \brief	The UUID namespace for URLs (6BA7B811-9DAD-11D1-80B4-00C04FD430C8).
		static constexpr uuid namespace_url = uuid{ 0x6BA7B811u, 0x9DAD_u16, 0x11D1_u16, 0x80B4_u16, 0x00C04FD430C8_u64 };

		/// \brief	The UUID namespace for ISO OIDs (6BA7B812-9DAD-11D1-80B4-00C04FD430C8).
		static constexpr uuid namespace_oid = uuid{ 0x6BA7B812u, 0x9DAD_u16, 0x11D1_u16, 0x80B4_u16, 0x00C04FD430C8_u64 };

		/// \brief	The UUID namespace for X.500 DNs (6BA7B814-9DAD-11D1-80B4-00C04FD430C8).
		static constexpr uuid namespace_x500 = uuid{ 0x6BA7B814u, 0x9DAD_u16, 0x11D1_u16, 0x80B4_u16, 0x00C04FD430C8_u64 };
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
		MUU_ATTR(pure)
		constexpr size_t operator()(const MUU_NAMESPACE::uuid& id) const noexcept
		{
			using namespace MUU_NAMESPACE;

			#define get_slice(idx)	\
				impl::uuid_slice<sizeof(size_t)>(id.data.bytes, sizeof(size_t) * idx)

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
