// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief Contains the definitions of functions and types related to the generation of hashes.

#pragma once
#include "../muu/core.h"

MUU_PRAGMA_MSVC(inline_recursion(on))

namespace muu::impl
{
	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(const)
	constexpr char dec_to_hex_lowercase(unsigned val) noexcept
	{
		return static_cast<char>(val >= 10u ? 'a' + (val - 10u) : '0' + val);
	}

	[[nodiscard]]
	MUU_ATTR(const)
	MUU_ATTR_CLANG(flatten)
	constexpr array<char,2> byte_to_chars_lowercase(std::byte byte) noexcept
	{
		return {{ dec_to_hex_lowercase(unwrap(byte) / 16u), dec_to_hex_lowercase(unwrap(byte) % 16u) }};
	}
}

MUU_PUSH_WARNINGS
MUU_DISABLE_PADDING_WARNINGS

namespace muu
{
	/// \addtogroup		hashing		Hashing
	/// \brief Functions and types related to the generation of hashes.
	/// @{
	
	/// \brief	The default hash size of hash functions, in bits.
	inline constexpr size_t default_hash_size = sizeof(size_t) * CHAR_BIT;

	namespace impl
	{
		template <size_t Bits>
		struct hash_combiner
		{
			static_assert(
				dependent_false<std::integral_constant<size_t, Bits>>,
				"Hash combining with hashes of given number of bits is not implemented on this platform"
			);
		};

		template <>
		struct hash_combiner<16>
		{
			static constexpr auto offset = 0x9E37_u16;
			static constexpr auto left_shift = 3;
			static constexpr auto right_shift = 1;
		};

		template <>
		struct hash_combiner<32>
		{
			static constexpr auto offset = 0x9E3779B9_u32;
			static constexpr auto left_shift = 6;
			static constexpr auto right_shift = 2;
		};

		template <>
		struct hash_combiner<64>
		{
			static constexpr auto offset = 0x9E3779B97F4A7C15_u64;
			static constexpr auto left_shift = 12;
			static constexpr auto right_shift = 4;
		};

		#if MUU_HAS_INT128

		template <>
		struct hash_combiner<128>
		{
			static constexpr auto offset = (0x9E3779B97F4A7C15_u128 << 64) | 0xF39CC0605D396154_u128;
			static constexpr auto left_shift = 24;
			static constexpr auto right_shift = 8;
		};

		#endif
	}

	/// \brief	A hash combiner.
	///
	/// \tparam	Bits	The hash size, in bits.
	template <size_t Bits = default_hash_size>
	class MUU_TRIVIAL_ABI hash_combiner
	{
		public:
			using hash_type = canonical_uint<Bits>;

		private:
			hash_type value_;

		public:
			constexpr hash_combiner(hash_type seed_hash) noexcept
				: value_{ seed_hash }
			{}

			constexpr hash_combiner& operator() (hash_type new_hash) noexcept
			{
				using constants = impl::hash_combiner<Bits>;
				value_ ^= (new_hash + constants::offset + (value_ << constants::left_shift) + (value_ >> constants::right_shift));
				return *this;
			}

			[[nodiscard]]
			constexpr hash_type value() const noexcept
			{
				return value_;
			}
	};

	namespace impl
	{
		template <size_t Bits>
		struct fnv1a
		{
			static_assert(
				dependent_false<std::integral_constant<size_t, Bits>>,
				"FNV-1a hashing with integers of given number of bits is not implemented on this platform"
			);
		};

		template <>
		struct fnv1a<32>
		{
			static constexpr auto prime = 0x01000193_u32;
			static constexpr auto offset_basis = 0x811C9DC5_u32;
		};

		template <>
		struct fnv1a<64>
		{
			static constexpr auto prime = 0x00000100000001B3_u64;
			static constexpr auto offset_basis = 0xCBF29CE484222325_u64;
		};

		#if MUU_HAS_INT128

		template <>
		struct fnv1a<128>
		{
			static constexpr auto prime =        (0x0000000001000000_u128 << 64) | 0x000000000000013B_u128;
			static constexpr auto offset_basis = (0x6C62272E07BB0142_u128 << 64) | 0x62B821756295C58D_u128;
		};

		#endif
	}

	/// \brief	FNV-1a hasher.
	///
	/// \tparam	Bits	The hash size, in bits.
	/// 
	/// \see [Fowler-Noll-Vo hash function](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function)
	template <size_t Bits = default_hash_size>
	class MUU_TRIVIAL_ABI fnv1a
	{
		public:
			using hash_type = canonical_uint<Bits>;
		
		private:
			hash_type value_ = impl::fnv1a<Bits>::offset_basis;

		public:
			constexpr fnv1a() noexcept = default;

			constexpr fnv1a& operator() (uint8_t byte) noexcept
			{
				value_ ^= static_cast<hash_type>(byte);
				value_ *= impl::fnv1a<Bits>::Prime;
			}

			constexpr fnv1a& operator() (std::byte byte) noexcept
			{
				return (*this)(unwrap(byte));
			}

			constexpr fnv1a& operator() (const void* data, size_t size) noexcept
			{
				const auto end = pointer_cast<const uint8_t*>(data) + size;
				for (auto ptr = pointer_cast<const uint8_t*>(data); ptr != end; ptr++)
					(*this)(*ptr);
				return *this;
			}

			template <typename Char, typename Traits>
			constexpr fnv1a& operator() (std::basic_string_view<Char, Traits> sv) noexcept
			{
				static_assert(sizeof(Char) == 1);
				return (*this)(sv.data(), sv.size());
			}

			[[nodiscard]]
			constexpr hash_type value() const noexcept
			{
				return value_;
			}
	};

	/// \brief	SHA-1 hasher.
	///
	/// \detail \cpp
	/// sha1 hasher;
	/// hasher("The quick brown fox jumps over the lazy dog"sv);
	/// hasher.finish();
	/// std::cout << hasher << std::endl;
	/// \ecpp
	/// 
	/// \out
	/// 2fd4e1c67a2d28fced849ee1bb76e7391b93eb12
	/// \eout
	/// 
	/// \see [SHA-1](https://en.wikipedia.org/wiki/SHA-1)
	class MUU_API sha1 final
	{
		public:
			using hash_type = std::byte[20];

		private:
			union state_t
			{
				impl::array<uint32_t, 5> digest;
				impl::array<std::byte, 20> hash;
			}
			state;
			uint32_t processed_blocks{};
			uint8_t current_block_length{};
			uint8_t current_block[64];
			bool finished_ = false;

			void add(uint8_t) noexcept;
			void add(const uint8_t*, size_t) noexcept;

		public:

			/// \brief	Constructs a new SHA-1 hasher.
			sha1() noexcept;

			/// \brief	Appends a single byte to the hash function's input.
			sha1& operator() (uint8_t byte) noexcept;

			/// \brief	Appends a single byte to the hash function's input.
			sha1& operator() (std::byte byte) noexcept
			{
				return (*this)(unwrap(byte));
			}

			/// \brief	Appends a sequence of data to the hash function's input.
			sha1& operator() (const void* data, size_t size) noexcept;

			/// \brief	Appends a string to the hash input.
			template <typename Char, typename Traits>
			constexpr sha1& operator() (std::basic_string_view<Char, Traits> sv) noexcept
			{
				static_assert(sizeof(Char) == 1);
				return (*this)(sv.data(), sv.size());
			}

			/// \brief	Finishes calculating the hash.
			/// \remarks Appending to the hash function's input has no effect after finish() is called.
			sha1& finish() noexcept;

			/// \brief	Returns the calculated hash value.
			///
			/// \warning Calling this before `finish()` has been called is undefined behaviour.
			[[nodiscard]]
			const hash_type& value() const noexcept
			{
				MUU_ASSERT(finished_);
				return state.hash.values;
			}

			/// \brief	Writes the calculated hash to a text stream in hexadecimal form.
			///
			///	\warning Calling this before `finish()` has been called is undefined behaviour.
			template <typename Char, typename Traits>
			friend std::basic_ostream<Char, Traits>& operator<< (std::basic_ostream<Char, Traits>& lhs, const sha1& rhs)
			{
				for (auto byte : rhs.value())
				{
					const auto hex = impl::byte_to_chars_lowercase(byte);
					lhs.write(hex.data(), hex.size());
				}
				return lhs;
			}
	};


	/// @}
}

MUU_POP_WARNINGS // MUU_DISABLE_PADDING_WARNINGS

MUU_PRAGMA_MSVC(inline_recursion(off))

#if MUU_IMPLEMENTATION
	#include "../muu/impl/hashing.hpp"
#endif
