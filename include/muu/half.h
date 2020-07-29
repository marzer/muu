// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief Contains the definition of muu::half.

#pragma once
#include "../muu/core.h"

MUU_PUSH_WARNINGS
MUU_DISABLE_ALL_WARNINGS

#define MUU_F16_USE_INTRINSICS 1
#if !MUU_HAS_INCLUDE(<immintrin.h>)
	#undef MUU_F16_USE_INTRINSICS
	#define MUU_F16_USE_INTRINSICS 0
#endif
#if MUU_F16_USE_INTRINSICS && MUU_CLANG
	#if !__has_feature(f16c)
		#undef MUU_F16_USE_INTRINSICS
		#define MUU_F16_USE_INTRINSICS 0
	#endif
#endif
#if MUU_F16_USE_INTRINSICS
	#include <immintrin.h>
#endif

MUU_POP_WARNINGS // MUU_DISABLE_ALL_WARNINGS


MUU_PUSH_WARNINGS
MUU_DISABLE_ARITHMETIC_WARNINGS

MUU_NAMESPACE_START
{
	namespace impl
	{
		[[nodiscard]] MUU_ATTR(const) constexpr uint16_t MUU_VECTORCALL f32_to_f16(float) noexcept;
		[[nodiscard]] MUU_ATTR(const) constexpr float MUU_VECTORCALL f16_to_f32(uint16_t) noexcept;
		struct f16_from_bits_tag {};
	}

	/// \brief	A 16-bit "half-precision" floating point type.
	/// 
	/// \detail This type is equipped with the full set of operators you'd expect from a float type,
	/// 		and is capable of being converted to other floats and integers, as well as direct construction
	/// 		using the `_f16` literal: \cpp
	/// 
	/// // creation from other floats and integers is explicit:
	/// auto f1 = half{ 1.0f };
	/// auto f2 = static_cast<half>(2);
	/// 
	/// // promotion to larger float types is implicit (to mimic the behaviour of the built-ins):
	/// double f3 = f2;
	/// 
	/// // using the _f16 literal
	/// using namespace muu::literals;
	/// auto f4 = 4.0_f16;
	/// 
	/// // arithmetic operators
	/// auto f5 = 4.0_f16 * 5.0_f16; // result is a half
	/// auto f6 = 4.0_f16 * 5.0;     // result is a double because of promotion
	/// auto f7 = 4.0_f16 * 5;       // result is a half because of promotion
	/// 
	/// // comparison operators
	/// auto b1 = 4.0_f16 <= 5.0_f16;
	/// auto b2 = 4.0_f16 <= 5.0;
	/// auto b3 = 4.0_f16 <= 5;
	/// \ecpp
	/// 
	/// \attention Despite the arithmetic operations being implemented as you'd expect, 16-bit floating-point
	/// 		 arithmetic is _very_ lossy and should be avoided for all but the most trivial cases.
	/// 		 In general it's better to do your arithmetic in a higher-precision type (e.g. float) and convert
	/// 		 back to half when you're finished.
	/// 
	/// \see [Half-precision floating-point (wikipedia)](https://en.wikipedia.org/wiki/Half-precision_floating-point_format)
	struct MUU_TRIVIAL_ABI half
	{
		/// \brief	The raw bits of the float.
		uint16_t bits;

		/// \brief	Static constants for this type.
		using constants = muu::constants<half>;

		//====================================================
		// CONSTRUCTORS
		//====================================================

		half() noexcept = default;
		constexpr half(const half&) noexcept = default;
		constexpr half& operator = (const half&) noexcept = default;

		private:

			constexpr half(uint16_t val, impl::f16_from_bits_tag) noexcept
				: bits{ val }
			{}


		public:

		/// \brief	Creates a half-precision float from its raw bit equivalent.
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(const)
		static constexpr half from_bits(uint16_t val) noexcept
		{
			return half{ val, impl::f16_from_bits_tag{} };
		}

		MUU_NODISCARD_CTOR
		explicit constexpr half(bool val) noexcept
			: bits{ val ? 0x3c00_u16 : 0_u16 }
		{ }

		#define MUU_F16_EXPLICIT_CONSTRUCTOR(type)					\
			MUU_NODISCARD_CTOR										\
			explicit constexpr half(type val) noexcept				\
				: bits{ impl::f32_to_f16(static_cast<float>(val)) }	\
			{}

		MUU_F16_EXPLICIT_CONSTRUCTOR(float)
		MUU_F16_EXPLICIT_CONSTRUCTOR(double)
		MUU_F16_EXPLICIT_CONSTRUCTOR(long double)
		MUU_F16_EXPLICIT_CONSTRUCTOR(char)
		MUU_F16_EXPLICIT_CONSTRUCTOR(signed char)
		MUU_F16_EXPLICIT_CONSTRUCTOR(unsigned char)
		MUU_F16_EXPLICIT_CONSTRUCTOR(short)
		MUU_F16_EXPLICIT_CONSTRUCTOR(unsigned short)
		MUU_F16_EXPLICIT_CONSTRUCTOR(int)
		MUU_F16_EXPLICIT_CONSTRUCTOR(unsigned int)
		MUU_F16_EXPLICIT_CONSTRUCTOR(long)
		MUU_F16_EXPLICIT_CONSTRUCTOR(unsigned long)
		MUU_F16_EXPLICIT_CONSTRUCTOR(long long)
		MUU_F16_EXPLICIT_CONSTRUCTOR(unsigned long long)
		#if MUU_HAS_INT128
		MUU_F16_EXPLICIT_CONSTRUCTOR(int128_t)
		MUU_F16_EXPLICIT_CONSTRUCTOR(uint128_t)
		#endif
		#undef MUU_F16_EXPLICIT_CONSTRUCTOR

		#if MUU_HAS_INTERCHANGE_FP16

		/*explicit*/ constexpr half(__fp16 val) noexcept
			: bits{ bit_cast<uint16_t>(val) }
		{}

		#endif // MUU_HAS_INTERCHANGE_FP16

		#if MUU_HAS_FLOAT16

		explicit constexpr half(float16_t val) noexcept
			: bits{ bit_cast<uint16_t>(val) }
		{}

		#endif // MUU_HAS_FLOAT16


		//====================================================
		// CONVERSIONS
		//====================================================

		#if MUU_HAS_INTERCHANGE_FP16

		[[nodiscard]]
		MUU_ATTR(pure)
		explicit constexpr operator __fp16() const noexcept
		{
			return bit_cast<__fp16>(bits);
		}

		#endif // MUU_HAS_INTERCHANGE_FP16

		#if MUU_HAS_FLOAT16

		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr operator float16_t() const noexcept
		{
			return bit_cast<float16_t>(bits);
		}

		#endif // MUU_HAS_FLOAT16

		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr operator float() const noexcept
		{
			return impl::f16_to_f32(bits);
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr operator double() const noexcept
		{
			return static_cast<double>(impl::f16_to_f32(bits));
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr operator long double() const noexcept
		{
			return static_cast<long double>(impl::f16_to_f32(bits));
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		MUU_ALWAYS_INLINE
		explicit constexpr operator bool() const noexcept
		{
			return (bits & 0x7FFFu) != 0u;
		}

		#define MUU_F16_EXPLICIT_CONVERSION(type)					\
			[[nodiscard]]											\
			explicit constexpr operator type() const noexcept		\
			{														\
				return static_cast<type>(impl::f16_to_f32(bits));	\
			}

		MUU_F16_EXPLICIT_CONVERSION(char)
		MUU_F16_EXPLICIT_CONVERSION(signed char)
		MUU_F16_EXPLICIT_CONVERSION(unsigned char)
		MUU_F16_EXPLICIT_CONVERSION(short)
		MUU_F16_EXPLICIT_CONVERSION(unsigned short)
		MUU_F16_EXPLICIT_CONVERSION(int)
		MUU_F16_EXPLICIT_CONVERSION(unsigned int)
		MUU_F16_EXPLICIT_CONVERSION(long)
		MUU_F16_EXPLICIT_CONVERSION(unsigned long)
		MUU_F16_EXPLICIT_CONVERSION(long long)
		MUU_F16_EXPLICIT_CONVERSION(unsigned long long)
		#if MUU_HAS_INT128
		MUU_F16_EXPLICIT_CONVERSION(int128_t)
		MUU_F16_EXPLICIT_CONVERSION(uint128_t)
		#endif
		#undef MUU_F16_EXPLICIT_CONVERSION

		//====================================================
		// COMPARISONS
		//====================================================

		[[nodiscard]]
		MUU_ATTR(const)
		friend constexpr bool MUU_VECTORCALL operator == (half lhs, half rhs) noexcept
		{
			return static_cast<float>(lhs) == static_cast<float>(rhs);
		}

		[[nodiscard]]
		MUU_ATTR(const)
		friend constexpr bool MUU_VECTORCALL operator != (half lhs, half rhs) noexcept
		{
			return static_cast<float>(lhs) != static_cast<float>(rhs);
		}

		[[nodiscard]]
		MUU_ATTR(const)
		friend constexpr bool MUU_VECTORCALL operator < (half lhs, half rhs) noexcept
		{
			return static_cast<float>(lhs) < static_cast<float>(rhs);
		}

		[[nodiscard]]
		MUU_ATTR(const)
		friend constexpr bool MUU_VECTORCALL operator <= (half lhs, half rhs) noexcept
		{
			return static_cast<float>(lhs) <= static_cast<float>(rhs);
		}

		[[nodiscard]]
		MUU_ATTR(const)
		friend constexpr bool MUU_VECTORCALL operator > (half lhs, half rhs) noexcept
		{
			return static_cast<float>(lhs) > static_cast<float>(rhs);
		}

		[[nodiscard]]
		MUU_ATTR(const)
		friend constexpr bool MUU_VECTORCALL operator >= (half lhs, half rhs) noexcept
		{
			return static_cast<float>(lhs) >= static_cast<float>(rhs);
		}

		#define MUU_F16_PROMOTING_BINARY_OP(return_type, input_type, op)					\
			[[nodiscard]] MUU_ATTR(const) friend constexpr return_type MUU_VECTORCALL		\
			operator op (half lhs, input_type rhs) noexcept									\
			{																				\
				return static_cast<input_type>(lhs) op rhs;									\
			}																				\
			[[nodiscard]] friend constexpr return_type MUU_VECTORCALL						\
			operator op (input_type rhs, half lhs) noexcept									\
			{																				\
				return lhs op static_cast<input_type>(rhs);									\
			}

		#define MUU_F16_CONVERTING_BINARY_OP(return_type, input_type, op)					\
			[[nodiscard]] MUU_ATTR(const) friend constexpr return_type MUU_VECTORCALL		\
			operator op (half lhs, input_type rhs) noexcept									\
			{																				\
				return return_type{ static_cast<float>(lhs) op static_cast<float>(rhs) };	\
			}																				\
			[[nodiscard]] friend constexpr return_type MUU_VECTORCALL						\
			operator op (input_type rhs, half lhs) noexcept									\
			{																				\
				return return_type{ static_cast<float>(lhs) op static_cast<float>(rhs) };	\
			}

		#define MUU_F16_BINARY_OPS(func, input_type)	\
			func(bool, input_type, ==)					\
			func(bool, input_type, !=)					\
			func(bool, input_type, < )					\
			func(bool, input_type, <=)					\
			func(bool, input_type, > )					\
			func(bool, input_type, >=)

		#if MUU_HAS_INTERCHANGE_FP16
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP, __fp16)
		#endif
		#if MUU_HAS_FLOAT16
		MUU_F16_BINARY_OPS(MUU_F16_PROMOTING_BINARY_OP,  float16_t)
		#endif
		MUU_F16_BINARY_OPS(MUU_F16_PROMOTING_BINARY_OP,  float)
		MUU_F16_BINARY_OPS(MUU_F16_PROMOTING_BINARY_OP,  double)
		MUU_F16_BINARY_OPS(MUU_F16_PROMOTING_BINARY_OP,  long double)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP, char)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP, signed char)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP, unsigned char)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP, short)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP, unsigned short)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP, int)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP, unsigned int)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP, long)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP, unsigned long)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP, long long)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP, unsigned long long)
		#if MUU_HAS_INT128
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP, int128_t)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP, uint128_t)
		#endif
		#undef MUU_F16_BINARY_OPS

		//====================================================
		// ARITHMETIC OPERATORS
		//====================================================

		[[nodiscard]]
		MUU_ATTR(const)
		friend constexpr half MUU_VECTORCALL operator + (half lhs, half rhs) noexcept
		{
			return half{ static_cast<float>(lhs) + static_cast<float>(rhs) };
		}

		[[nodiscard]]
		MUU_ATTR(const)
		friend constexpr half MUU_VECTORCALL operator - (half lhs, half rhs) noexcept
		{
			return half{ static_cast<float>(lhs) - static_cast<float>(rhs) };
		}

		[[nodiscard]]
		MUU_ATTR(const)
		friend constexpr half MUU_VECTORCALL operator * (half lhs, half rhs) noexcept
		{
			return half{ static_cast<float>(lhs) * static_cast<float>(rhs) };
		}

		[[nodiscard]]
		MUU_ATTR(const)
		friend constexpr half MUU_VECTORCALL operator / (half lhs, half rhs) noexcept
		{
			return half{ static_cast<float>(lhs) / static_cast<float>(rhs) };
		}

		#define MUU_F16_BINARY_OPS(func, return_type, input_type)	\
			func(return_type, input_type, +)						\
			func(return_type, input_type, -)						\
			func(return_type, input_type, *)						\
			func(return_type, input_type, /)

		#if MUU_HAS_INTERCHANGE_FP16
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP,	half,			__fp16)
		#endif
		#if MUU_HAS_FLOAT16
		MUU_F16_BINARY_OPS(MUU_F16_PROMOTING_BINARY_OP,		float16_t,		float16_t)
		#endif
		MUU_F16_BINARY_OPS(MUU_F16_PROMOTING_BINARY_OP,		float,			float)
		MUU_F16_BINARY_OPS(MUU_F16_PROMOTING_BINARY_OP,		double,			double)
		MUU_F16_BINARY_OPS(MUU_F16_PROMOTING_BINARY_OP,		long double,	long double)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP,	half,			char)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP,	half,			signed char)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP,	half,			unsigned char)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP,	half,			short)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP,	half,			unsigned short)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP,	half,			int)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP,	half,			unsigned int)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP,	half,			long)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP,	half,			unsigned long)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP,	half,			long long)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP,	half,			unsigned long long)
		#if MUU_HAS_INT128
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP,	half,			int128_t)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_BINARY_OP,	half,			uint128_t)
		#endif
		#undef MUU_F16_BINARY_OPS
		#undef MUU_F16_PROMOTING_BINARY_OP
		#undef MUU_F16_CONVERTING_BINARY_OP

		//====================================================
		// ARITHMETIC ASSIGNMENTS
		//====================================================

		friend constexpr half& operator += (half& lhs, half rhs) noexcept
		{
			lhs.bits = impl::f32_to_f16(static_cast<float>(lhs) + static_cast<float>(rhs));
			return lhs;
		}

		friend constexpr half& operator -= (half& lhs, half rhs) noexcept
		{
			lhs.bits = impl::f32_to_f16(static_cast<float>(lhs) - static_cast<float>(rhs));
			return lhs;
		}

		friend constexpr half& operator *= (half& lhs, half rhs) noexcept
		{
			lhs.bits = impl::f32_to_f16(static_cast<float>(lhs) * static_cast<float>(rhs));
			return lhs;
		}

		friend constexpr half& operator /= (half& lhs, half rhs) noexcept
		{
			lhs.bits = impl::f32_to_f16(static_cast<float>(lhs) / static_cast<float>(rhs));
			return lhs;
		}

		#define MUU_F16_DEMOTING_ASSIGN_OP(input_type, op)								\
			friend constexpr half& operator op##= (half& lhs, input_type rhs) noexcept	\
			{																			\
				lhs.bits = impl::f32_to_f16(static_cast<float>(lhs op rhs));			\
				return lhs;																\
			}

		#define MUU_F16_CONVERTING_ASSIGN_OP(input_type, op)							\
			friend constexpr half& operator op##= (half& lhs, input_type rhs) noexcept	\
			{																			\
				lhs.bits = impl::f32_to_f16(static_cast<float>(lhs) op rhs);			\
				return lhs;																\
			}

		#define MUU_F16_CASTING_ASSIGN_OP(input_type, op)								\
			friend constexpr half& operator op##= (half& lhs, input_type rhs) noexcept	\
			{																			\
				lhs.bits = bit_cast<uint16_t>(lhs op rhs);								\
				return lhs;																\
			}

		#define MUU_F16_BINARY_OPS(func, input_type)	\
			func(input_type, +)							\
			func(input_type, -)							\
			func(input_type, *)							\
			func(input_type, /)

		#if MUU_HAS_INTERCHANGE_FP16
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_ASSIGN_OP,	__fp16)
		#endif
		#if MUU_HAS_FLOAT16
		MUU_F16_BINARY_OPS(MUU_F16_CASTING_ASSIGN_OP,		float16_t)
		#endif
		MUU_F16_BINARY_OPS(MUU_F16_DEMOTING_ASSIGN_OP,		float)
		MUU_F16_BINARY_OPS(MUU_F16_DEMOTING_ASSIGN_OP,		double)
		MUU_F16_BINARY_OPS(MUU_F16_DEMOTING_ASSIGN_OP,		long double)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_ASSIGN_OP,	char)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_ASSIGN_OP,	signed char)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_ASSIGN_OP,	unsigned char)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_ASSIGN_OP,	short)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_ASSIGN_OP,	unsigned short)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_ASSIGN_OP,	int)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_ASSIGN_OP,	unsigned int)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_ASSIGN_OP,	long)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_ASSIGN_OP,	unsigned long)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_ASSIGN_OP,	long long)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_ASSIGN_OP,	unsigned long long)
		#if MUU_HAS_INT128
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_ASSIGN_OP,	int128_t)
		MUU_F16_BINARY_OPS(MUU_F16_CONVERTING_ASSIGN_OP,	uint128_t)
		#endif
		#undef MUU_F16_BINARY_OPS
		#undef MUU_F16_DEMOTING_ASSIGN_OP
		#undef MUU_F16_CONVERTING_ASSIGN_OP
		#undef MUU_F16_CASTING_ASSIGN_OP

		//====================================================
		// INCREMENTS AND DECREMENTS
		//====================================================

		constexpr half& operator++() noexcept
		{
			bits = impl::f32_to_f16(static_cast<float>(*this) + 1.0f);
			return *this;
		}

		constexpr half& operator--() noexcept
		{
			bits = impl::f32_to_f16(static_cast<float>(*this) - 1.0f);
			return *this;
		}

		constexpr half operator++(int) noexcept
		{
			half prev = *this;
			bits = impl::f32_to_f16(static_cast<float>(*this) + 1.0f);
			return prev;
		}

		constexpr half operator--(int) noexcept
		{
			half prev = *this;
			bits = impl::f32_to_f16(static_cast<float>(*this) - 1.0f);
			return prev;
		}

		//====================================================
		// MISC
		//====================================================

		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr half operator + () const noexcept
		{
			return *this;
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr half operator - () const noexcept
		{
			return from_bits(bits ^ 0b1000000000000000_u16);
		}

		template <typename Char, typename Traits>
		friend std::basic_ostream<Char, Traits>& operator << (std::basic_ostream<Char, Traits>& lhs, half rhs)
		{
			return lhs << static_cast<float>(rhs);
		}

		template <typename Char, typename Traits>
		friend std::basic_istream<Char, Traits>& operator >> (std::basic_istream<Char, Traits>& lhs, half& rhs)
		{
			float val;
			if (lhs >> val)
				rhs = static_cast<half>(val);
			return lhs;
		}

		/// \brief	Returns true if the value of the half is positive or negative infinity.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool is_infinity() const noexcept
		{
			return (0b0111110000000000_u16 & bits) == 0b0111110000000000_u16
				&& (0b0000001111111111_u16 & bits) == 0_u16;
		}

		/// \brief	Returns true if the value of the half is Not-A-Number.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool is_nan() const noexcept
		{
			return (0b0111110000000000_u16 & bits) == 0b0111110000000000_u16
				&& (0b0000001111111111_u16 & bits) != 0_u16;
		}

		/// \brief	Returns true if the value of the half is positive/negative infinity or Not-A-Number.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool is_infinity_or_nan() const noexcept
		{
			return (0b0111110000000000_u16 & bits) == 0b0111110000000000_u16;
		}
	};

	inline namespace literals
	{
		/// \brief	Literal for creating a half-precision float.
		[[nodiscard]]
		MUU_ATTR(const)
		MUU_CONSTEVAL half operator "" _f16(long double val) noexcept
		{
			return half{ val };
		}
	}

	template <>
	[[nodiscard]]
	constexpr half MUU_VECTORCALL abs<half, void>(half val) noexcept
	{
		return static_cast<float>(val) < 0.0f ? -val : val;
	}

	namespace impl
	{
		// hex/bit patterns for constants were generated using this: http://weitz.de/ieee/

		template <>
		struct numeric_limits<half>
		{
			static constexpr half lowest = half::from_bits(0xFBFF_u16);
			static constexpr half highest = half::from_bits(0x7BFF_u16);
		};

		template <>
		struct unsigned_integral_named_constants<half>
		{
			static constexpr half zero = half::from_bits(0b0'00000'0000000000_u16);
			static constexpr half one = half::from_bits(0b0'01111'0000000000_u16);
			static constexpr half two = half::from_bits(0b0'10000'0000000000_u16);
			static constexpr half three = half::from_bits(0b0'10000'1000000000_u16);
			static constexpr half four = half::from_bits(0b0'10001'0000000000_u16);
			static constexpr half five = half::from_bits(0b0'10001'0100000000_u16);
			static constexpr half six = half::from_bits(0b0'10001'1000000000_u16);
			static constexpr half seven = half::from_bits(0b0'10001'1100000000_u16);
			static constexpr half eight = half::from_bits(0b0'10010'0000000000_u16);
			static constexpr half nine = half::from_bits(0b0'10010'0010000000_u16);
			static constexpr half ten = half::from_bits(0b0'10010'0100000000_u16);
		};

		template <>
		struct signed_integral_named_constants<half>
		{
			static constexpr half minus_one = half::from_bits(0b1'01111'0000000000_u16);
			static constexpr half minus_two = half::from_bits(0b1'10000'0000000000_u16);
			static constexpr half minus_three = half::from_bits(0b1'10000'1000000000_u16);
			static constexpr half minus_four = half::from_bits(0b1'10001'0000000000_u16);
			static constexpr half minus_five = half::from_bits(0b1'10001'0100000000_u16);
			static constexpr half minus_six = half::from_bits(0b1'10001'1000000000_u16);
			static constexpr half minus_seven = half::from_bits(0b1'10001'1100000000_u16);
			static constexpr half minus_eight = half::from_bits(0b1'10010'0000000000_u16);
			static constexpr half minus_nine = half::from_bits(0b1'10010'0010000000_u16);
			static constexpr half minus_ten = half::from_bits(0b1'10010'0100000000_u16);
		};

		template <>
		struct floating_point_special_constants<half>
		{
			static constexpr half nan = half::from_bits(0b1'11111'1000000001_u16);
			static constexpr half snan = half::from_bits(0b1'11111'0000000001_u16);
			static constexpr half infinity = half::from_bits(0b0'11111'0000000000_u16);
			static constexpr half negative_infinity = half::from_bits(0b1'11111'0000000000_u16);
		};

		template <>
		struct floating_point_named_constants<half>
		{
			static constexpr half minus_zero = half::from_bits(0b1'00000'0000000000_u16);
			static constexpr half one_over_two = half::from_bits(0b0'01110'0000000000_u16);
			static constexpr half three_over_two = half::from_bits(0b0'01111'1000000000_u16);
		};

	}

	/// \brief	16-bit half-precision float constants.
	/// \ingroup		constants
	template <> struct constants<half> : impl::floating_point_constants<half> {};
}
MUU_NAMESPACE_END

MUU_IMPL_NAMESPACE_START
{
	#if MUU_F16_USE_INTRINSICS

	MUU_PUSH_WARNINGS
	MUU_PRAGMA_MSVC(warning(disable: 4556)) //value of intrinsic immediate argument '8' is out of range '0 - 7'
	MUU_PRAGMA_GCC("GCC diagnostic ignored \"-Wold-style-cast\"") // false positive with _mm_set_ss

	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(const)
	uint16_t MUU_VECTORCALL f32_to_f16_intrinsic(float val) noexcept
	{
		//_mm_set_ss			store a single float in a m128
		//_mm_cvtps_ph			convert floats in a m128 to half-precision floats in an m128i
		//_mm_cvtsi128_si32		returns the first int from an m128i

		return static_cast<::uint16_t>(_mm_cvtsi128_si32(
			_mm_cvtps_ph(_mm_set_ss(val), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)
		));
	}

	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(const)
	float MUU_VECTORCALL f16_to_f32_intrinsic(uint16_t val) noexcept
	{
		//_mm_cvtsi32_si128		store a single int in an m128i
		//_mm_cvtph_ps			convert half-precision floats in a m128i to floats in an m128
		//_mm_cvtss_f32			returns the first float from an m128

		return _mm_cvtss_f32(_mm_cvtph_ps(
			_mm_cvtsi32_si128(static_cast<int>(val))
		));
	}

	MUU_POP_WARNINGS

	#endif //MUU_F16_USE_INTRINSICS

	inline constexpr int8_t f16_single_exp_bias = 127;
	inline constexpr int8_t f16_half_exp_bias = 15;

	[[nodiscard]]
	MUU_ATTR(const)
	constexpr uint16_t MUU_VECTORCALL f32_to_f16_native(float val) noexcept
	{
		const uint32_t bits32 = bit_cast<uint32_t>(val);
		const uint16_t s16 = static_cast<uint16_t>((bits32 & 0x80000000u) >> 16);

		// 0000 0000 0111 1111 1111 1111 1111 1111
		const uint32_t frac32 = bits32 & 0x007FFFFFu;

		// 0111 1111 1000 0000 0000 0000 0000 0000
		const uint32_t exp32 = (bits32 & 0x7F800000u) >> 23;

		// unbiased exp32
		const int8_t exp32ub = static_cast<int8_t>(exp32) - f16_single_exp_bias;

		uint16_t exp16 = static_cast<uint16_t>(exp32ub + 15);

		// clamp the exponent
		if MUU_UNLIKELY(exp32 == 0xFF /*|| exp32ub > single_exp_bias*/)
		{
			// 0001 1111, use all five bits
			exp16 = 0x1F_u16;
		}
		else if MUU_UNLIKELY(exp32 == 0 /*|| exp32ub < -(single_exp_bias - 1)*/)
		{
			exp16 = {};
		}

		uint16_t frac16 = static_cast<uint16_t>(frac32 >> 13);

		// nan
		if MUU_UNLIKELY(exp32 == 0xFF && frac32 != 0 && frac16 == 0)
		{
			// 0000 0010 0000 0000
			frac16 = 0x0200_u16;
		}
		// denormal overflow
		else if (exp32 == 0 || (exp16 == 0x1F && exp32 != 0xFF))
		{
			frac16 = {};
		}
		// denormal underflow
		else if MUU_UNLIKELY(exp16 == 0 && exp32 != 0)
		{
			// 0000 0001 0000 0000
			frac16 = 0x0100 | (frac16 >> 2);
		}
		exp16 = static_cast<uint16_t>(exp16 << 10);
		return s16 | exp16 | frac16;
	}

	[[nodiscard]]
	MUU_ATTR(const)
	constexpr float MUU_VECTORCALL f16_to_f32_native(uint16_t val) noexcept
	{
		// 1000 0000 0000 0000 ->
		// 1000 0000 0000 0000 0000 0000 0000 0000
		const uint32_t s32 = static_cast<uint32_t>(val & 0x8000_u16) << 16;

		// 0111 1100 0000 0000
		const uint16_t exp16 = (val & 0x7C00_u16) >> 10;

		uint32_t exp32 = static_cast<uint32_t>(exp16) + (f16_single_exp_bias - f16_half_exp_bias);

		// 0000 0011 1111 1111
		const uint16_t frac16 = val & 0x03FF_u16;

		uint32_t frac32 = static_cast<uint32_t>(frac16);

		// the number is denormal if exp16 == 0 and frac16 != 0
		if MUU_UNLIKELY(exp16 == 0 && frac16 != 0)
		{
			uint32_t offset = {};
			do
			{
				++offset;
				frac32 = frac32 << 1;
			}
			while ((frac32 & 0x0400u) != 0x0400u); // 0100 0000 0000

			frac32 &= 0x03FFu;
			exp32 = 113u - offset; // 113 = 127-14
		}
		// +-0
		else if MUU_UNLIKELY(exp16 == 0 && frac16 == 0)
		{
			exp32 = {};
		}
		// +- inf
		else if MUU_UNLIKELY(exp16 == 31_u16 && frac16 == 0_u16)
		{
			exp32 = 0xFFu;
		}
		// +- nan
		else if MUU_UNLIKELY(exp16 == 31_u16 && frac16 != 0_u16)
		{
			exp32 = 0xFFu;
		}

		exp32 = exp32 << 23;
		frac32 = frac32 << 13;
		return bit_cast<float>(s32 | exp32 | frac32);
	}

	[[nodiscard]]
	MUU_ATTR(const)
	MUU_ATTR(flatten)
	constexpr uint16_t MUU_VECTORCALL f32_to_f16(float val) noexcept
	{
		#if MUU_F16_USE_INTRINSICS
		if constexpr (build::supports_is_constant_evaluated)
		{
			if (!is_constant_evaluated())
				return f32_to_f16_intrinsic(val);
		}
		#endif

		return f32_to_f16_native(val);
	}

	[[nodiscard]]
	MUU_ATTR(const)
	MUU_ATTR(flatten)
	constexpr float MUU_VECTORCALL f16_to_f32(uint16_t val) noexcept
	{
		#if MUU_F16_USE_INTRINSICS
		if constexpr (build::supports_is_constant_evaluated)
		{
			if (!is_constant_evaluated())
				return f16_to_f32_intrinsic(val);
		}
		#endif

		return f16_to_f32_native(val);
	}
}
MUU_IMPL_NAMESPACE_END

namespace std
{
	/// \brief	Specialization of std::numeric_limits for muu::half.
	/// \ingroup constants
	template<>
	struct numeric_limits<muu::half>
	{
		using half = muu::half;

		static constexpr auto is_specialized	= true;
		static constexpr auto is_signed			= true;
		static constexpr auto is_integer		= false;
		static constexpr auto is_exact			= false;
		static constexpr auto has_infinity		= true;
		static constexpr auto has_quiet_NaN		= true;
		static constexpr auto has_signaling_NaN	= true;
		static constexpr auto has_denorm		= float_denorm_style::denorm_present;
		static constexpr auto has_denorm_loss	= false;
		static constexpr auto round_style		= float_round_style::round_to_nearest;
		static constexpr auto is_iec559			= true;
		static constexpr auto is_bounded		= true;
		static constexpr auto is_modulo			= false;
		static constexpr auto radix				= 2;
		static constexpr auto digits			= 11;	// equivalent to __FLT16_MANT_DIG__
		static constexpr auto digits10			= 3;	// equivalent to __FLT16_DIG__
		static constexpr auto min_exponent		= -13;	// equivalent to __FLT16_MIN_EXP__
		static constexpr auto min_exponent10	= -4;	// equivalent to __FLT16_MIN_10_EXP__
		static constexpr auto max_exponent		= 16;	// equivalent to __FLT16_MAX_EXP__
		static constexpr auto max_exponent10	= 4;	// equivalent to __FLT16_MAX_10_EXP__
		static constexpr auto max_digits10		= 5;
		static constexpr auto traps				= false;
		static constexpr auto tinyness_before	= false;
		
		[[nodiscard]]
		MUU_ATTR(const)
		static MUU_CONSTEVAL half min() noexcept
		{
			using namespace muu::literals;
			return half::from_bits(0x0400_u16); // 0.000061035 (ish)
		}

		[[nodiscard]]
		MUU_ATTR(const)
		static MUU_CONSTEVAL half lowest() noexcept
		{
			return half::constants::lowest;
		}

		[[nodiscard]]
		MUU_ATTR(const)
		static MUU_CONSTEVAL half max() noexcept
		{
			return half::constants::highest;
		}

		[[nodiscard]]
		MUU_ATTR(const)
		static MUU_CONSTEVAL half epsilon() noexcept
		{
			using namespace muu::literals;
			return half::from_bits(0b0'00101'0000000000_u16); // 0.00097656
		}

		[[nodiscard]]
		MUU_ATTR(const)
		static MUU_CONSTEVAL half round_error() noexcept
		{
			using namespace muu::literals;
			return half::from_bits(0b0'00100'0000000000_u16); // epsilon / 2
		}

		[[nodiscard]]
		MUU_ATTR(const)
		static MUU_CONSTEVAL half infinity() noexcept
		{
			return half::constants::infinity;
		}

		[[nodiscard]]
		MUU_ATTR(const)
		static MUU_CONSTEVAL half quiet_NaN() noexcept
		{
			return half::constants::nan;
		}

		[[nodiscard]]
		MUU_ATTR(const)
		static MUU_CONSTEVAL half signaling_NaN() noexcept
		{
			return half::constants::snan;
		}

		[[nodiscard]]
		MUU_ATTR(const)
		static MUU_CONSTEVAL half denorm_min() noexcept
		{
			using namespace muu::literals;
			return half::from_bits(0x0001_u16); // 0.000000059605 (ish)
		}
	};

	template <> struct numeric_limits<const muu::half>			: numeric_limits<muu::half> {};
	template <> struct numeric_limits<volatile muu::half>		: numeric_limits<muu::half> {};
	template <> struct numeric_limits<const volatile muu::half>	: numeric_limits<muu::half> {};
}

MUU_POP_WARNINGS // MUU_DISABLE_ARITHMETIC_WARNINGS

#undef MUU_F16_USE_INTRINSICS
