// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::float16.
#pragma once
#include "../muu/common.h"

MUU_PUSH_WARNINGS
MUU_DISABLE_ALL_WARNINGS

#include <limits>

#define MUU_F16_USE_INTRINSICS 1
#if !__has_include(<immintrin.h>)
	#undef MUU_F16_USE_INTRINSICS
	#define MUU_F16_USE_INTRINSICS 0
#endif
#if MUU_F16_USE_INTRINSICS && defined(__clang__)
	#if !__has_feature(f16c)
		#undef MUU_F16_USE_INTRINSICS
		#define MUU_F16_USE_INTRINSICS 0
	#endif
#endif
#if MUU_F16_USE_INTRINSICS
	#include <immintrin.h>
#endif

MUU_POP_WARNINGS

namespace muu::impl
{
	[[nodiscard]] constexpr uint16_t MUU_VECTORCALL f32_to_f16(float) noexcept;
	[[nodiscard]] constexpr float MUU_VECTORCALL f16_to_f32(uint16_t) noexcept;
	struct f16_from_bits_tag {};
}

namespace muu
{
	/// \brief	A 16-bit "half-precision" floating point type.
	/// 
	/// \detail This type is equipped with the full set of operators you'd expect from a float type,
	/// 		and is capable of being converted to other floats and integers, as well as direct construction
	/// 		using the `_f16` literal: \cpp
	/// 
	/// // creation from other floats and integers is explicit:
	/// auto f1 = float16{ 1.0f };
	/// auto f2 = static_cast<float16>(2);
	/// 
	/// // promotion to larger float types is implicit (to mimic the behaviour of the built-ins):
	/// double f3 = f2;
	/// 
	/// // using the _f16 literal
	/// using namespace muu::literals;
	/// auto f4 = 4.0_f16;
	/// 
	/// // arithmetic operators
	/// auto f5 = 4.0_f16 * 5.0_f16; // result is a float16
	/// auto f6 = 4.0_f16 * 5.0;     // result is a double because of promotion
	/// auto f7 = 4.0_f16 * 5;       // result is a float16 because of promotion
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
	/// 		 back to float16 when you're finished.
	/// 
	/// \see [Half-precision floating-point (wikipedia)](https://en.wikipedia.org/wiki/Half-precision_floating-point_format)
	struct MUU_TRIVIAL_ABI float16
	{
		/// \brief	The raw bits of the float.
		uint16_t bits;

		//====================================================
		// CONSTRUCTORS
		//====================================================

		float16() noexcept = default;
		constexpr float16(const float16&) noexcept = default;

		private:

			constexpr float16(uint16_t val, impl::f16_from_bits_tag) noexcept
				: bits{ val }
			{}


		public:

		/// \brief	Creates a half-precision float from its raw bit equivalent.
		[[nodiscard]] MUU_ALWAYS_INLINE
		static constexpr float16 from_bits(uint16_t val) noexcept
		{
			return float16{ val, impl::f16_from_bits_tag{} };
		}

		MUU_NODISCARD_CTOR
		explicit constexpr float16(bool val) noexcept
			: bits{ val ? 0x3c00_u16 : 0_u16 }
		{ }

		#define MUU_F16_EXPLICIT_CONSTRUCTOR(type)					\
			MUU_NODISCARD_CTOR										\
			explicit constexpr float16(type val) noexcept			\
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

		#undef MUU_F16_EXPLICIT_CONSTRUCTOR

		//====================================================
		// CONVERSIONS
		//====================================================

		[[nodiscard]] MUU_ALWAYS_INLINE
		constexpr operator float() const noexcept
		{
			return impl::f16_to_f32(bits);
		}

		[[nodiscard]] MUU_ALWAYS_INLINE
		constexpr operator double() const noexcept
		{
			return static_cast<double>(impl::f16_to_f32(bits));
		}

		[[nodiscard]] MUU_ALWAYS_INLINE
		constexpr operator long double() const noexcept
		{
			return static_cast<long double>(impl::f16_to_f32(bits));
		}

		[[nodiscard]] MUU_ALWAYS_INLINE
		explicit constexpr operator bool() const noexcept
		{
			return (bits & 0x7FFFu) != 0u;
		}

		#define MUU_F16_EXPLICIT_CONVERSION(type)				\
			[[nodiscard]] MUU_ALWAYS_INLINE							\
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

		#undef MUU_F16_EXPLICIT_CONVERSION

		//====================================================
		// INFINITY AND NAN
		//====================================================

		/// \brief	Returns true if the value of a float16 is infinity or NaN.
		[[nodiscard]] MUU_ALWAYS_INLINE
		constexpr bool is_infinity_or_nan() const noexcept
		{
			return (0b0111110000000000_u16 & bits) == 0b0111110000000000_u16;
		}

		/// \brief	Returns true if the value of a float16 is NaN.
		[[nodiscard]] MUU_ALWAYS_INLINE
		constexpr bool is_nan() const noexcept
		{
			return (0b0111110000000000_u16 & bits) == 0b0111110000000000_u16
				&& (0b0000001111111111_u16 & bits) != 0_u16;
		}

		//====================================================
		// COMPARISONS
		//====================================================

		[[nodiscard]] MUU_ALWAYS_INLINE
		friend constexpr bool MUU_VECTORCALL operator == (float16 lhs, float16 rhs) noexcept
		{
			return static_cast<float>(lhs) == static_cast<float>(rhs);
		}

		[[nodiscard]] MUU_ALWAYS_INLINE
		friend constexpr bool MUU_VECTORCALL operator != (float16 lhs, float16 rhs) noexcept
		{
			return static_cast<float>(lhs) != static_cast<float>(rhs);
		}

		[[nodiscard]] MUU_ALWAYS_INLINE
		friend constexpr bool MUU_VECTORCALL operator < (float16 lhs, float16 rhs) noexcept
		{
			return static_cast<float>(lhs) < static_cast<float>(rhs);
		}

		[[nodiscard]] MUU_ALWAYS_INLINE
		friend constexpr bool MUU_VECTORCALL operator <= (float16 lhs, float16 rhs) noexcept
		{
			return static_cast<float>(lhs) <= static_cast<float>(rhs);
		}

		[[nodiscard]] MUU_ALWAYS_INLINE
		friend constexpr bool MUU_VECTORCALL operator > (float16 lhs, float16 rhs) noexcept
		{
			return static_cast<float>(lhs) > static_cast<float>(rhs);
		}

		[[nodiscard]] MUU_ALWAYS_INLINE
		friend constexpr bool MUU_VECTORCALL operator >= (float16 lhs, float16 rhs) noexcept
		{
			return static_cast<float>(lhs) >= static_cast<float>(rhs);
		}

		#define MUU_F16_PROMOTING_BINARY_OP(return_type, input_type, op)					\
			[[nodiscard]] MUU_ALWAYS_INLINE friend constexpr return_type MUU_VECTORCALL		\
			operator op (float16 lhs, input_type rhs) noexcept								\
			{																				\
				return static_cast<input_type>(lhs) op rhs;									\
			}																				\
			[[nodiscard]] MUU_ALWAYS_INLINE friend constexpr return_type MUU_VECTORCALL		\
			operator op (input_type rhs, float16 lhs) noexcept								\
			{																				\
				return lhs op static_cast<input_type>(rhs);									\
			}

		MUU_F16_PROMOTING_BINARY_OP(bool, float, ==)
		MUU_F16_PROMOTING_BINARY_OP(bool, float, !=)
		MUU_F16_PROMOTING_BINARY_OP(bool, float, < )
		MUU_F16_PROMOTING_BINARY_OP(bool, float, <=)
		MUU_F16_PROMOTING_BINARY_OP(bool, float, > )
		MUU_F16_PROMOTING_BINARY_OP(bool, float, >=)
		MUU_F16_PROMOTING_BINARY_OP(bool, double, ==)
		MUU_F16_PROMOTING_BINARY_OP(bool, double, !=)
		MUU_F16_PROMOTING_BINARY_OP(bool, double, < )
		MUU_F16_PROMOTING_BINARY_OP(bool, double, <=)
		MUU_F16_PROMOTING_BINARY_OP(bool, double, > )
		MUU_F16_PROMOTING_BINARY_OP(bool, double, >=)
		MUU_F16_PROMOTING_BINARY_OP(bool, long double, ==)
		MUU_F16_PROMOTING_BINARY_OP(bool, long double, !=)
		MUU_F16_PROMOTING_BINARY_OP(bool, long double, < )
		MUU_F16_PROMOTING_BINARY_OP(bool, long double, <=)
		MUU_F16_PROMOTING_BINARY_OP(bool, long double, > )
		MUU_F16_PROMOTING_BINARY_OP(bool, long double, >=)

		#define MUU_F16_CONVERTING_BINARY_OP(return_type, input_type, op)					\
			[[nodiscard]] MUU_ALWAYS_INLINE friend constexpr return_type MUU_VECTORCALL		\
			operator op (float16 lhs, input_type rhs) noexcept								\
			{																				\
				return return_type{ static_cast<float>(lhs) op static_cast<float>(rhs) };	\
			}																				\
			[[nodiscard]] MUU_ALWAYS_INLINE	friend constexpr return_type MUU_VECTORCALL		\
			operator op (input_type rhs, float16 lhs) noexcept								\
			{																				\
				return return_type{ static_cast<float>(lhs) op static_cast<float>(rhs) };	\
			}

		MUU_F16_CONVERTING_BINARY_OP(bool, char, ==)
		MUU_F16_CONVERTING_BINARY_OP(bool, char, !=)
		MUU_F16_CONVERTING_BINARY_OP(bool, char, < )
		MUU_F16_CONVERTING_BINARY_OP(bool, char, <=)
		MUU_F16_CONVERTING_BINARY_OP(bool, char, > )
		MUU_F16_CONVERTING_BINARY_OP(bool, char, >=)
		MUU_F16_CONVERTING_BINARY_OP(bool, signed char, ==)
		MUU_F16_CONVERTING_BINARY_OP(bool, signed char, !=)
		MUU_F16_CONVERTING_BINARY_OP(bool, signed char, < )
		MUU_F16_CONVERTING_BINARY_OP(bool, signed char, <=)
		MUU_F16_CONVERTING_BINARY_OP(bool, signed char, > )
		MUU_F16_CONVERTING_BINARY_OP(bool, signed char, >=)
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned char, ==)
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned char, !=)
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned char, < )
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned char, <=)
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned char, > )
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned char, >=)
		MUU_F16_CONVERTING_BINARY_OP(bool, short, ==)
		MUU_F16_CONVERTING_BINARY_OP(bool, short, !=)
		MUU_F16_CONVERTING_BINARY_OP(bool, short, < )
		MUU_F16_CONVERTING_BINARY_OP(bool, short, <=)
		MUU_F16_CONVERTING_BINARY_OP(bool, short, > )
		MUU_F16_CONVERTING_BINARY_OP(bool, short, >=)
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned short, ==)
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned short, !=)
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned short, < )
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned short, <=)
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned short, > )
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned short, >=)
		MUU_F16_CONVERTING_BINARY_OP(bool, int, ==)
		MUU_F16_CONVERTING_BINARY_OP(bool, int, !=)
		MUU_F16_CONVERTING_BINARY_OP(bool, int, < )
		MUU_F16_CONVERTING_BINARY_OP(bool, int, <=)
		MUU_F16_CONVERTING_BINARY_OP(bool, int, > )
		MUU_F16_CONVERTING_BINARY_OP(bool, int, >=)
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned int, ==)
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned int, !=)
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned int, < )
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned int, <=)
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned int, > )
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned int, >=)
		MUU_F16_CONVERTING_BINARY_OP(bool, long, ==)
		MUU_F16_CONVERTING_BINARY_OP(bool, long, !=)
		MUU_F16_CONVERTING_BINARY_OP(bool, long, < )
		MUU_F16_CONVERTING_BINARY_OP(bool, long, <=)
		MUU_F16_CONVERTING_BINARY_OP(bool, long, > )
		MUU_F16_CONVERTING_BINARY_OP(bool, long, >=)
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned long, ==)
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned long, !=)
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned long, < )
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned long, <=)
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned long, > )
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned long, >=)
		MUU_F16_CONVERTING_BINARY_OP(bool, long long, ==)
		MUU_F16_CONVERTING_BINARY_OP(bool, long long, !=)
		MUU_F16_CONVERTING_BINARY_OP(bool, long long, < )
		MUU_F16_CONVERTING_BINARY_OP(bool, long long, <=)
		MUU_F16_CONVERTING_BINARY_OP(bool, long long, > )
		MUU_F16_CONVERTING_BINARY_OP(bool, long long, >=)
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned long long, ==)
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned long long, !=)
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned long long, < )
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned long long, <=)
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned long long, > )
		MUU_F16_CONVERTING_BINARY_OP(bool, unsigned long long, >=)

		//====================================================
		// ARITHMETIC OPERATORS
		//====================================================

		[[nodiscard]]
		friend constexpr float16 MUU_VECTORCALL operator + (float16 lhs, float16 rhs) noexcept
		{
			return float16{ static_cast<float>(lhs) + static_cast<float>(rhs) };
		}

		[[nodiscard]]
		friend constexpr float16 MUU_VECTORCALL operator - (float16 lhs, float16 rhs) noexcept
		{
			return float16{ static_cast<float>(lhs) - static_cast<float>(rhs) };
		}

		[[nodiscard]]
		friend constexpr float16 MUU_VECTORCALL operator * (float16 lhs, float16 rhs) noexcept
		{
			return float16{ static_cast<float>(lhs) * static_cast<float>(rhs) };
		}

		[[nodiscard]]
		friend constexpr float16 MUU_VECTORCALL operator / (float16 lhs, float16 rhs) noexcept
		{
			return float16{ static_cast<float>(lhs) / static_cast<float>(rhs) };
		}

		MUU_F16_PROMOTING_BINARY_OP(float, float, +)
		MUU_F16_PROMOTING_BINARY_OP(float, float, -)
		MUU_F16_PROMOTING_BINARY_OP(float, float, *)
		MUU_F16_PROMOTING_BINARY_OP(float, float, /)
		MUU_F16_PROMOTING_BINARY_OP(double, double, +)
		MUU_F16_PROMOTING_BINARY_OP(double, double, -)
		MUU_F16_PROMOTING_BINARY_OP(double, double, *)
		MUU_F16_PROMOTING_BINARY_OP(double, double, /)
		MUU_F16_PROMOTING_BINARY_OP(long double, long double, +)
		MUU_F16_PROMOTING_BINARY_OP(long double, long double, -)
		MUU_F16_PROMOTING_BINARY_OP(long double, long double, *)
		MUU_F16_PROMOTING_BINARY_OP(long double, long double, /)

		#undef MUU_F16_PROMOTING_BINARY_OP

		MUU_F16_CONVERTING_BINARY_OP(float16, char, +)
		MUU_F16_CONVERTING_BINARY_OP(float16, char, -)
		MUU_F16_CONVERTING_BINARY_OP(float16, char, *)
		MUU_F16_CONVERTING_BINARY_OP(float16, char, /)
		MUU_F16_CONVERTING_BINARY_OP(float16, signed char, +)
		MUU_F16_CONVERTING_BINARY_OP(float16, signed char, -)
		MUU_F16_CONVERTING_BINARY_OP(float16, signed char, *)
		MUU_F16_CONVERTING_BINARY_OP(float16, signed char, /)
		MUU_F16_CONVERTING_BINARY_OP(float16, unsigned char, +)
		MUU_F16_CONVERTING_BINARY_OP(float16, unsigned char, -)
		MUU_F16_CONVERTING_BINARY_OP(float16, unsigned char, *)
		MUU_F16_CONVERTING_BINARY_OP(float16, unsigned char, /)
		MUU_F16_CONVERTING_BINARY_OP(float16, short, +)
		MUU_F16_CONVERTING_BINARY_OP(float16, short, -)
		MUU_F16_CONVERTING_BINARY_OP(float16, short, *)
		MUU_F16_CONVERTING_BINARY_OP(float16, short, /)
		MUU_F16_CONVERTING_BINARY_OP(float16, unsigned short, +)
		MUU_F16_CONVERTING_BINARY_OP(float16, unsigned short, -)
		MUU_F16_CONVERTING_BINARY_OP(float16, unsigned short, *)
		MUU_F16_CONVERTING_BINARY_OP(float16, unsigned short, /)
		MUU_F16_CONVERTING_BINARY_OP(float16, int, +)
		MUU_F16_CONVERTING_BINARY_OP(float16, int, -)
		MUU_F16_CONVERTING_BINARY_OP(float16, int, *)
		MUU_F16_CONVERTING_BINARY_OP(float16, int, /)
		MUU_F16_CONVERTING_BINARY_OP(float16, unsigned int, +)
		MUU_F16_CONVERTING_BINARY_OP(float16, unsigned int, -)
		MUU_F16_CONVERTING_BINARY_OP(float16, unsigned int, *)
		MUU_F16_CONVERTING_BINARY_OP(float16, unsigned int, /)
		MUU_F16_CONVERTING_BINARY_OP(float16, long, +)
		MUU_F16_CONVERTING_BINARY_OP(float16, long, -)
		MUU_F16_CONVERTING_BINARY_OP(float16, long, *)
		MUU_F16_CONVERTING_BINARY_OP(float16, long, /)
		MUU_F16_CONVERTING_BINARY_OP(float16, unsigned long, +)
		MUU_F16_CONVERTING_BINARY_OP(float16, unsigned long, -)
		MUU_F16_CONVERTING_BINARY_OP(float16, unsigned long, *)
		MUU_F16_CONVERTING_BINARY_OP(float16, unsigned long, /)
		MUU_F16_CONVERTING_BINARY_OP(float16, long long, +)
		MUU_F16_CONVERTING_BINARY_OP(float16, long long, -)
		MUU_F16_CONVERTING_BINARY_OP(float16, long long, *)
		MUU_F16_CONVERTING_BINARY_OP(float16, long long, /)
		MUU_F16_CONVERTING_BINARY_OP(float16, unsigned long long, +)
		MUU_F16_CONVERTING_BINARY_OP(float16, unsigned long long, -)
		MUU_F16_CONVERTING_BINARY_OP(float16, unsigned long long, *)
		MUU_F16_CONVERTING_BINARY_OP(float16, unsigned long long, /)

		#undef MUU_F16_CONVERTING_BINARY_OP

		/// \brief	Performs a fused-multiply-add.
		/// 
		/// \details The FMA operation is performed on the inputs as if they were higher-precision
		/// 		 types so the intermediate precision loss is minimized.
		/// 
		/// \param 	m1	The first multiplication operand.
		/// \param 	m2	The second multiplication operand.
		/// \param 	a 	The addition operand.
		///
		/// \returns	The result of `(m1 * m2) + a`.
		[[nodiscard]]
		static constexpr float16 MUU_VECTORCALL fma(float16 m1, float16 m2, float16 a) noexcept
		{
			#if defined(__GNUC__) || defined(__clang__)
				if (!is_constant_evaluated())
					return float16{ __builtin_fmaf(m1, m2, a) };
			#endif
			return float16{ (static_cast<long double>(m1) * m2) + a };
		}

		//====================================================
		// ARITHMETIC ASSIGNMENTS
		//====================================================

		friend constexpr float16& operator += (float16& lhs, float16 rhs) noexcept
		{
			lhs.bits = impl::f32_to_f16(static_cast<float>(lhs) + static_cast<float>(rhs));
			return lhs;
		}

		friend constexpr float16& operator -= (float16& lhs, float16 rhs) noexcept
		{
			lhs.bits = impl::f32_to_f16(static_cast<float>(lhs) - static_cast<float>(rhs));
			return lhs;
		}

		friend constexpr float16& operator *= (float16& lhs, float16 rhs) noexcept
		{
			lhs.bits = impl::f32_to_f16(static_cast<float>(lhs) * static_cast<float>(rhs));
			return lhs;
		}

		friend constexpr float16& operator /= (float16& lhs, float16 rhs) noexcept
		{
			lhs.bits = impl::f32_to_f16(static_cast<float>(lhs) / static_cast<float>(rhs));
			return lhs;
		}

		#define MUU_F16_ASSIGN_OP(input_type, op)												\
			friend constexpr float16& operator op##= (float16& lhs, input_type rhs) noexcept	\
			{																					\
				lhs.bits = impl::f32_to_f16(static_cast<float>(lhs op rhs));					\
				return lhs;																		\
			}

		MUU_F16_ASSIGN_OP(float, +)
		MUU_F16_ASSIGN_OP(float, -)
		MUU_F16_ASSIGN_OP(float, *)
		MUU_F16_ASSIGN_OP(float, /)
		MUU_F16_ASSIGN_OP(double, +)
		MUU_F16_ASSIGN_OP(double, -)
		MUU_F16_ASSIGN_OP(double, *)
		MUU_F16_ASSIGN_OP(double, /)
		MUU_F16_ASSIGN_OP(long double, +)
		MUU_F16_ASSIGN_OP(long double, -)
		MUU_F16_ASSIGN_OP(long double, *)
		MUU_F16_ASSIGN_OP(long double, /)

		#undef MUU_F16_ASSIGN_OP
		#define MUU_F16_ASSIGN_OP(input_type, op)									\
			friend constexpr float16& operator op##= (float16& lhs, input_type rhs) noexcept	\
			{																					\
				lhs.bits = impl::f32_to_f16(static_cast<float>(lhs) op rhs);					\
				return lhs;																		\
			}

		MUU_F16_ASSIGN_OP(char, +)
		MUU_F16_ASSIGN_OP(char, -)
		MUU_F16_ASSIGN_OP(char, *)
		MUU_F16_ASSIGN_OP(char, /)
		MUU_F16_ASSIGN_OP(signed char, +)
		MUU_F16_ASSIGN_OP(signed char, -)
		MUU_F16_ASSIGN_OP(signed char, *)
		MUU_F16_ASSIGN_OP(signed char, /)
		MUU_F16_ASSIGN_OP(unsigned char, +)
		MUU_F16_ASSIGN_OP(unsigned char, -)
		MUU_F16_ASSIGN_OP(unsigned char, *)
		MUU_F16_ASSIGN_OP(unsigned char, /)
		MUU_F16_ASSIGN_OP(short, +)
		MUU_F16_ASSIGN_OP(short, -)
		MUU_F16_ASSIGN_OP(short, *)
		MUU_F16_ASSIGN_OP(short, /)
		MUU_F16_ASSIGN_OP(unsigned short, +)
		MUU_F16_ASSIGN_OP(unsigned short, -)
		MUU_F16_ASSIGN_OP(unsigned short, *)
		MUU_F16_ASSIGN_OP(unsigned short, /)
		MUU_F16_ASSIGN_OP(int, +)
		MUU_F16_ASSIGN_OP(int, -)
		MUU_F16_ASSIGN_OP(int, *)
		MUU_F16_ASSIGN_OP(int, /)
		MUU_F16_ASSIGN_OP(unsigned int, +)
		MUU_F16_ASSIGN_OP(unsigned int, -)
		MUU_F16_ASSIGN_OP(unsigned int, *)
		MUU_F16_ASSIGN_OP(unsigned int, /)
		MUU_F16_ASSIGN_OP(long, +)
		MUU_F16_ASSIGN_OP(long, -)
		MUU_F16_ASSIGN_OP(long, *)
		MUU_F16_ASSIGN_OP(long, /)
		MUU_F16_ASSIGN_OP(unsigned long, +)
		MUU_F16_ASSIGN_OP(unsigned long, -)
		MUU_F16_ASSIGN_OP(unsigned long, *)
		MUU_F16_ASSIGN_OP(unsigned long, /)
		MUU_F16_ASSIGN_OP(long long, +)
		MUU_F16_ASSIGN_OP(long long, -)
		MUU_F16_ASSIGN_OP(long long, *)
		MUU_F16_ASSIGN_OP(long long, /)
		MUU_F16_ASSIGN_OP(unsigned long long, +)
		MUU_F16_ASSIGN_OP(unsigned long long, -)
		MUU_F16_ASSIGN_OP(unsigned long long, *)
		MUU_F16_ASSIGN_OP(unsigned long long, /)

		#undef MUU_F16_ASSIGN_OP

		//====================================================
		// INCREMENTS AND DECREMENTS
		//====================================================

		constexpr float16& operator++() noexcept
		{
			bits = impl::f32_to_f16(static_cast<float>(*this) + 1.0f);
			return *this;
		}

		constexpr float16& operator--() noexcept
		{
			bits = impl::f32_to_f16(static_cast<float>(*this) - 1.0f);
			return *this;
		}

		constexpr float16 operator++(int) noexcept
		{
			float16 prev = *this;
			bits = impl::f32_to_f16(static_cast<float>(*this) + 1.0f);
			return prev;
		}

		constexpr float16 operator--(int) noexcept
		{
			float16 prev = *this;
			bits = impl::f32_to_f16(static_cast<float>(*this) - 1.0f);
			return prev;
		}

		//====================================================
		// MISC
		//====================================================

		[[nodiscard]]
		constexpr float16 operator + () const noexcept
		{
			return *this;
		}

		[[nodiscard]]
		constexpr float16 operator - () const noexcept
		{
			return from_bits(bits ^ 0b1000000000000000_u16);
		}
	};

	inline namespace literals
	{
		/// \brief	Literal for creating a half-precision float.
		[[nodiscard]] MUU_ALWAYS_INLINE
		MUU_CONSTEVAL float16 operator "" _f16(long double val) noexcept
		{
			return float16{ val };
		}
	}

	template <>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr bool MUU_VECTORCALL is_infinity_or_nan<float16, void>(float16 val) noexcept
	{
		return val.is_infinity_or_nan();
	}

	template <>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr float16 MUU_VECTORCALL abs<float16, void>(float16 val) noexcept
	{
		return static_cast<float>(val) < 0.0f ? -val : val;
	}
}

namespace muu::impl
{
	#if MUU_F16_USE_INTRINSICS

	MUU_PUSH_WARNINGS
	MUU_PRAGMA_MSVC(warning(disable: 4556)) //value of intrinsic immediate argument '8' is out of range '0 - 7'

	[[nodiscard]] MUU_ALWAYS_INLINE
	uint16_t MUU_VECTORCALL f32_to_f16_intrinsic(float val) noexcept
	{
		//_mm_set_ss			store a single float in a m128
		//_mm_cvtps_ph			convert floats in a m128 to half-precision floats in an m128i
		//_mm_cvtsi128_si32		returns the first int from an m128i

		return static_cast<::uint16_t>(_mm_cvtsi128_si32(
			_mm_cvtps_ph(_mm_set_ss(val), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)
		));
	}

	[[nodiscard]] MUU_ALWAYS_INLINE
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
	constexpr uint16_t MUU_VECTORCALL f32_to_f16_native(float val) noexcept
	{
		const uint32_t bits32 = bit_cast<uint32_t>(val);
		const uint16_t s16 = (bits32 & 0x80000000u) >> 16;

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
			exp16 = 0x1F;
		}
		else if MUU_UNLIKELY(exp32 == 0 /*|| exp32ub < -(single_exp_bias - 1)*/)
		{
			exp16 = 0;
		}

		uint16_t frac16 = static_cast<uint16_t>(frac32 >> 13);

		// nan
		if MUU_UNLIKELY(exp32 == 0xFF && frac32 != 0 && frac16 == 0)
		{
			// 0000 0010 0000 0000
			frac16 = 0x0200;
		}
		// denormal overflow
		else if (exp32 == 0 || (exp16 == 0x1F && exp32 != 0xFF))
		{
			frac16 = 0;
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
			uint32_t offset = 0;
			do
			{
				++offset;
				frac32 = frac32 << 1;
			}
			while ((frac32 & 0x0400u) != 0x0400u); // 0100 0000 0000

			frac32 &= 0x03FFu;
			exp32 = 113 - offset; // 113 = 127-14
		}
		// +-0
		else if MUU_UNLIKELY(exp16 == 0 && frac16 == 0)
		{
			exp32 = 0;
		}
		// +- inf
		else if MUU_UNLIKELY(exp16 == 31 && frac16 == 0)
		{
			exp32 = 0xFF;
		}
		// +- nan
		else if MUU_UNLIKELY(exp16 == 31 && frac16 != 0)
		{
			exp32 = 0xFF;
		}

		exp32 = exp32 << 23;
		frac32 = frac32 << 13;
		return bit_cast<float>(s32 | exp32 | frac32);
	}

	[[nodiscard]] MUU_ALWAYS_INLINE
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

	[[nodiscard]] MUU_ALWAYS_INLINE
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

namespace std
{
	/// \brief	Specialization of std::numeric_limits for muu::float16.
	template<>
	struct numeric_limits<::muu::float16>
	{
		using float16 = ::muu::float16;

		static constexpr bool is_specialized = true;
		static constexpr bool is_signed = true;
		static constexpr bool is_integer = false;
		static constexpr bool is_exact = false;
		static constexpr bool has_infinity = true;
		static constexpr bool has_quiet_NaN = true;
		static constexpr bool has_signaling_NaN = true;
		static constexpr float_denorm_style has_denorm = denorm_present;
		static constexpr bool has_denorm_loss = false;
		static constexpr std::float_round_style round_style = std::round_to_nearest;
		static constexpr bool is_iec559 = true;
		static constexpr bool is_bounded = true;
		static constexpr bool is_modulo = false;
		static constexpr int digits = 11;
		static constexpr int digits10 = 3;
		static constexpr int max_digits10 = 5;
		static constexpr int radix = 2;
		static constexpr int min_exponent = -13;
		static constexpr int min_exponent10 = -4;
		static constexpr int max_exponent = 16;
		static constexpr int max_exponent10 = 4;
		static constexpr bool traps = false;
		static constexpr bool tinyness_before = false;
		
		[[nodiscard]]
		static constexpr float16 min() noexcept
		{
			using namespace ::muu::literals;
			return float16::from_bits(0x0400_u16); // 0.000061035 (ish)
		}

		[[nodiscard]]
		static constexpr float16 lowest() noexcept
		{
			using namespace ::muu::literals;
			return float16::from_bits(0xFBFF_u16); // -65504
		}
		[[nodiscard]]
		static constexpr float16 max() noexcept
		{
			using namespace ::muu::literals;
			return float16::from_bits(0x7BFF_u16); // 65504
		}

		[[nodiscard]]
		static constexpr float16 epsilon() noexcept
		{
			using namespace ::muu::literals;
			return float16::from_bits(0x0800_u16);
		}

		[[nodiscard]]
		static constexpr float16 round_error() noexcept
		{
			using namespace ::muu::literals;
			return float16::from_bits(0x3800_u16);
		}

		[[nodiscard]]
		static constexpr float16 infinity() noexcept
		{
			using namespace ::muu::literals;
			return float16::from_bits(0x7C00_u16);
		}

		[[nodiscard]]
		static constexpr float16 quiet_NaN() noexcept
		{
			using namespace ::muu::literals;
			return float16::from_bits(0x7E00_u16);
		}

		[[nodiscard]]
		static constexpr float16 signaling_NaN() noexcept
		{
			using namespace ::muu::literals;
			return float16::from_bits(0x7E00_u16);
		}

		[[nodiscard]]
		static constexpr float16 denorm_min() noexcept
		{
			using namespace ::muu::literals;
			return float16::from_bits(0x0001_u16); // 0.000000059605 (ish)
		}
	};

	template <> struct numeric_limits<const ::muu::float16> : numeric_limits<::muu::float16> {};
	template <> struct numeric_limits<volatile ::muu::float16> : numeric_limits<::muu::float16> {};
	template <> struct numeric_limits<const volatile ::muu::float16> : numeric_limits<::muu::float16> {};
}


#undef MUU_F16_USE_INTRINSICS
