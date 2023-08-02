// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the definition of muu::half.

#include "math.h"
#include "impl/std_iosfwd.h"

// see if we can just wrap a 'real' fp16 type (better codegen while still being binary-compatible)
#if MUU_HAS_FLOAT16 && MUU_FLOAT16_LIMITS_IEEE754
	#define MUU_HALF_IMPL_TYPE _Float16
#endif
#ifdef MUU_HALF_IMPL_TYPE
	#define MUU_HALF_EMULATED		0
	#define MUU_HALF_USE_INTRINSICS 0
#else
	#define MUU_HALF_IMPL_TYPE uint16_t
	#define MUU_HALF_EMULATED  1

	// detect SSE, SSE2 and FP16C
	#if MUU_ISET_SSE && MUU_ISET_SSE2
		#define MUU_HALF_USE_INTRINSICS 1
	#endif
	#if (MUU_GCC && !defined(__F16C__)) || (MUU_CLANG && !MUU_HAS_FEATURE(f16c))
		#undef MUU_HALF_USE_INTRINSICS
	#endif
	#ifndef MUU_HALF_USE_INTRINSICS
		#define MUU_HALF_USE_INTRINSICS 0
	#endif
#endif

MUU_DISABLE_WARNINGS;
#if MUU_HALF_USE_INTRINSICS
	#if MUU_MSVC
		#include <intrin.h>
	#elif MUU_LINUX
		#include <immintrin.h>
	#endif
#endif
MUU_ENABLE_WARNINGS;

#include "impl/header_start.h"
MUU_DISABLE_ARITHMETIC_WARNINGS;
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(warning(disable : 4556)) // value of intrinsic immediate argument '8' is out of range '0 - 7'
MUU_PRAGMA_MSVC(float_control(except, off))
MUU_PRAGMA_GCC(diagnostic ignored "-Wold-style-cast") // false positive with _mm_load_ss

namespace muu
{
	/// \cond

	namespace impl
	{
		struct half_from_bits_tag
		{};

#if MUU_HALF_EMULATED

		MUU_CONST_GETTER
		constexpr uint16_t MUU_VECTORCALL f32_to_u16(float) noexcept;

		MUU_CONST_GETTER
		constexpr float MUU_VECTORCALL u16_to_f32(uint16_t) noexcept;

#endif
	}

	MUU_CONST_GETTER
	constexpr bool MUU_VECTORCALL infinity_or_nan(half) noexcept;
	MUU_CONST_GETTER
	constexpr half MUU_VECTORCALL abs(half) noexcept;
	MUU_CONST_GETTER
	constexpr bool MUU_VECTORCALL approx_equal(half, half, half) noexcept;
	MUU_CONST_GETTER
	constexpr half MUU_VECTORCALL floor(half) noexcept;
	MUU_CONST_GETTER
	constexpr half MUU_VECTORCALL ceil(half) noexcept;
	MUU_CONST_GETTER
	constexpr half MUU_VECTORCALL sqrt(half) noexcept;
	MUU_CONST_GETTER
	constexpr half MUU_VECTORCALL cos(half) noexcept;
	MUU_CONST_GETTER
	constexpr half MUU_VECTORCALL sin(half) noexcept;
	MUU_CONST_GETTER
	constexpr half MUU_VECTORCALL tan(half) noexcept;
	MUU_CONST_GETTER
	constexpr half MUU_VECTORCALL acos(half) noexcept;
	MUU_CONST_GETTER
	constexpr half MUU_VECTORCALL asin(half) noexcept;
	MUU_CONST_GETTER
	constexpr half MUU_VECTORCALL atan(half) noexcept;
	MUU_CONST_GETTER
	constexpr half MUU_VECTORCALL atan2(half, half) noexcept;
	MUU_CONST_GETTER
	constexpr half MUU_VECTORCALL lerp(half, half, half) noexcept;

	/// \endcond

	/// \brief	A 16-bit "half-precision" IEEE754 floating point type.
	/// \ingroup core
	///
	/// \details This type is equipped with the full set of operators you'd expect from a float type,
	/// 		and is capable of being converted to other floats and integers, as well as direct construction
	/// 		using the `_f16` literal: \cpp
	///
	/// // creation from other floats and integers is explicit:
	/// auto f1 = muu::half{ 1.0f };
	/// auto f2 = static_cast<muu::half>(2);
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
	/// \availability	Older compilers won't provide the necessary machinery for arithmetic and conversions to/from
	///					muu::half to work in constexpr contexts. Check for constexpr support by examining
	/// 				#build::supports_constexpr_half.
	///
	/// \attention	Despite the arithmetic operations being implemented as you'd expect, 16-bit floating-point
	/// 			arithmetic is _very_ lossy and should be avoided for all but the most trivial cases.
	/// 			In general it's better to do your arithmetic in a higher-precision type (e.g. float) and convert
	/// 			back to half when you're finished.
	///
	/// \see [Half-precision floating-point](https://en.wikipedia.org/wiki/Half-precision_floating-point_format)
	struct MUU_TRIVIAL_ABI half
	{
	  private:
		using impl_type = MUU_HALF_IMPL_TYPE;
		impl_type impl_;

#if MUU_HALF_EMULATED

		explicit constexpr half(impl::half_from_bits_tag, uint16_t bits) noexcept //
			: impl_{ bits }
		{}
#else

		explicit constexpr half(impl::half_from_bits_tag, uint16_t bits) noexcept //
			: impl_{ muu::bit_cast<impl_type>(bits) }
		{}
#endif

	  public:
		/// \brief	Static constants for this type.
		using constants = muu::constants<half>;

		half() noexcept = default;

		/// \brief	Creates a half-precision float from its raw bit equivalent.
		MUU_CONST_INLINE_GETTER
		static constexpr half from_bits(uint16_t val) noexcept
		{
			return half{ impl::half_from_bits_tag{}, val };
		}

#if MUU_HALF_EMULATED

		MUU_NODISCARD_CTOR
		explicit constexpr half(bool val) noexcept //
			: impl_{ val ? 0x3c00_u16 : 0_u16 }
		{}

#else

		MUU_NODISCARD_CTOR
		explicit constexpr half(bool val) noexcept //
			: impl_{ static_cast<impl_type>(val ? 1.0f : 0.0f) }
		{}

#endif

#if MUU_HALF_EMULATED
	#define HALF_EXPLICIT_CONSTRUCTOR(type)                                                                            \
		MUU_NODISCARD_CTOR                                                                                             \
		explicit constexpr half(type val) noexcept : impl_{ impl::f32_to_u16(static_cast<float>(val)) }                \
		{}                                                                                                             \
		static_assert(true)
#else
	#define HALF_EXPLICIT_CONSTRUCTOR(type)                                                                            \
		MUU_NODISCARD_CTOR                                                                                             \
		explicit constexpr half(type val) noexcept : impl_{ static_cast<impl_type>(val) }                              \
		{}                                                                                                             \
		static_assert(true)
#endif

		HALF_EXPLICIT_CONSTRUCTOR(float);
		HALF_EXPLICIT_CONSTRUCTOR(double);
		HALF_EXPLICIT_CONSTRUCTOR(long double);
#if MUU_HAS_FLOAT128
		HALF_EXPLICIT_CONSTRUCTOR(float128_t);
#endif
		HALF_EXPLICIT_CONSTRUCTOR(char);
		HALF_EXPLICIT_CONSTRUCTOR(signed char);
		HALF_EXPLICIT_CONSTRUCTOR(unsigned char);
		HALF_EXPLICIT_CONSTRUCTOR(signed short);
		HALF_EXPLICIT_CONSTRUCTOR(unsigned short);
		HALF_EXPLICIT_CONSTRUCTOR(signed int);
		HALF_EXPLICIT_CONSTRUCTOR(unsigned int);
		HALF_EXPLICIT_CONSTRUCTOR(signed long);
		HALF_EXPLICIT_CONSTRUCTOR(unsigned long);
		HALF_EXPLICIT_CONSTRUCTOR(signed long long);
		HALF_EXPLICIT_CONSTRUCTOR(unsigned long long);
#if MUU_HAS_INT128
		HALF_EXPLICIT_CONSTRUCTOR(int128_t);
		HALF_EXPLICIT_CONSTRUCTOR(uint128_t);
#endif
#undef HALF_EXPLICIT_CONSTRUCTOR

#if MUU_HAS_FLOAT16
		explicit constexpr half(_Float16 val) noexcept //
			: impl_{ static_cast<impl_type>(val) }
		{
			static_assert(!std::is_same_v<impl_type, uint16_t>);
		}
#endif

		//====================================================
		// CONVERSIONS
		//====================================================

		MUU_PURE_INLINE_GETTER
		explicit constexpr operator bool() const noexcept
		{
#if MUU_HALF_EMULATED
			return (impl_ & 0x7FFF) != 0u; // !(anything but sign bit)
#else
			return static_cast<bool>(impl_);
#endif
		}

#if MUU_HAS_FLOAT16
		MUU_PURE_INLINE_GETTER
		constexpr operator _Float16() const noexcept
		{
			return static_cast<_Float16>(impl_);
		}
#endif

		MUU_PURE_INLINE_GETTER
		constexpr operator float() const noexcept
		{
#if MUU_HALF_EMULATED
			return impl::u16_to_f32(impl_);
#else
			return static_cast<float>(impl_);
#endif
		}

#define HALF_CAST_CONVERSION(type, explicit)                                                                           \
	MUU_NODISCARD                                                                                                      \
	explicit constexpr operator type() const noexcept                                                                  \
	{                                                                                                                  \
		return static_cast<type>(static_cast<float>(*this));                                                           \
	}                                                                                                                  \
	static_assert(true)

		HALF_CAST_CONVERSION(double, );
		HALF_CAST_CONVERSION(long double, );
#if MUU_HAS_FLOAT128
		HALF_CAST_CONVERSION(float128_t, );
#endif
		HALF_CAST_CONVERSION(char, explicit);
		HALF_CAST_CONVERSION(signed char, explicit);
		HALF_CAST_CONVERSION(unsigned char, explicit);
		HALF_CAST_CONVERSION(signed short, explicit);
		HALF_CAST_CONVERSION(unsigned short, explicit);
		HALF_CAST_CONVERSION(signed int, explicit);
		HALF_CAST_CONVERSION(unsigned int, explicit);
		HALF_CAST_CONVERSION(signed long, explicit);
		HALF_CAST_CONVERSION(unsigned long, explicit);
		HALF_CAST_CONVERSION(signed long long, explicit);
		HALF_CAST_CONVERSION(unsigned long long, explicit);
#if MUU_HAS_INT128
		HALF_CAST_CONVERSION(int128_t, explicit);
		HALF_CAST_CONVERSION(uint128_t, explicit);
#endif

#undef HALF_CAST_CONVERSION

		//====================================================
		// COMPARISONS
		//====================================================

		MUU_CONST_INLINE_GETTER
		friend constexpr bool MUU_VECTORCALL operator==(half lhs, half rhs) noexcept
		{
#if MUU_HALF_EMULATED
			return static_cast<float>(lhs) == static_cast<float>(rhs);
#else
			return lhs.impl_ == rhs.impl_;
#endif
		}

		MUU_CONST_INLINE_GETTER
		friend constexpr bool MUU_VECTORCALL operator!=(half lhs, half rhs) noexcept
		{
#if MUU_HALF_EMULATED
			return static_cast<float>(lhs) != static_cast<float>(rhs);
#else
			return lhs.impl_ != rhs.impl_;
#endif
		}

		MUU_CONST_INLINE_GETTER
		friend constexpr bool MUU_VECTORCALL operator<(half lhs, half rhs) noexcept
		{
#if MUU_HALF_EMULATED
			return static_cast<float>(lhs) < static_cast<float>(rhs);
#else
			return lhs.impl_ < rhs.impl_;
#endif
		}

		MUU_CONST_INLINE_GETTER
		friend constexpr bool MUU_VECTORCALL operator<=(half lhs, half rhs) noexcept
		{
#if MUU_HALF_EMULATED
			return static_cast<float>(lhs) <= static_cast<float>(rhs);
#else
			return lhs.impl_ <= rhs.impl_;
#endif
		}

		MUU_CONST_INLINE_GETTER
		friend constexpr bool MUU_VECTORCALL operator>(half lhs, half rhs) noexcept
		{
#if MUU_HALF_EMULATED
			return static_cast<float>(lhs) > static_cast<float>(rhs);
#else
			return lhs.impl_ > rhs.impl_;
#endif
		}

		MUU_CONST_INLINE_GETTER
		friend constexpr bool MUU_VECTORCALL operator>=(half lhs, half rhs) noexcept
		{
#if MUU_HALF_EMULATED
			return static_cast<float>(lhs) >= static_cast<float>(rhs);
#else
			return lhs.impl_ >= rhs.impl_;
#endif
		}

#define HALF_PROMOTING_BINARY_OP(return_type, input_type, op)                                                          \
	MUU_CONST_GETTER                                                                                                   \
	friend constexpr return_type MUU_VECTORCALL operator op(half lhs, input_type rhs) noexcept                         \
	{                                                                                                                  \
		return static_cast<input_type>(lhs) op rhs;                                                                    \
	}                                                                                                                  \
	MUU_CONST_GETTER                                                                                                   \
	friend constexpr return_type MUU_VECTORCALL operator op(input_type lhs, half rhs) noexcept                         \
	{                                                                                                                  \
		return lhs op static_cast<input_type>(rhs);                                                                    \
	}                                                                                                                  \
	static_assert(true)

#define HALF_CONVERTING_BINARY_OP(return_type, input_type, op)                                                         \
	MUU_CONST_GETTER                                                                                                   \
	friend constexpr return_type MUU_VECTORCALL operator op(half lhs, input_type rhs) noexcept                         \
	{                                                                                                                  \
		return static_cast<return_type>(static_cast<float>(lhs) op static_cast<float>(rhs));                           \
	}                                                                                                                  \
	MUU_CONST_GETTER                                                                                                   \
	friend constexpr return_type MUU_VECTORCALL operator op(input_type lhs, half rhs) noexcept                         \
	{                                                                                                                  \
		return static_cast<return_type>(static_cast<float>(lhs) op static_cast<float>(rhs));                           \
	}                                                                                                                  \
	static_assert(true)

#define HALF_BINARY_OPS(func, input_type)                                                                              \
	func(bool, input_type, ==);                                                                                        \
	func(bool, input_type, !=);                                                                                        \
	func(bool, input_type, <);                                                                                         \
	func(bool, input_type, <=);                                                                                        \
	func(bool, input_type, >);                                                                                         \
	func(bool, input_type, >=)

#if MUU_HAS_FLOAT16
		HALF_BINARY_OPS(HALF_PROMOTING_BINARY_OP, _Float16);
#endif
		HALF_BINARY_OPS(HALF_PROMOTING_BINARY_OP, float);
		HALF_BINARY_OPS(HALF_PROMOTING_BINARY_OP, double);
		HALF_BINARY_OPS(HALF_PROMOTING_BINARY_OP, long double);
#if MUU_HAS_FLOAT128
		HALF_BINARY_OPS(HALF_PROMOTING_BINARY_OP, float128_t);
#endif
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, char);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, signed char);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, unsigned char);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, signed short);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, unsigned short);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, signed int);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, unsigned int);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, signed long);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, unsigned long);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, signed long long);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, unsigned long long);
#if MUU_HAS_INT128
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, int128_t);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, uint128_t);
#endif
#undef HALF_BINARY_OPS

		//====================================================
		// ARITHMETIC OPERATORS
		//====================================================

		MUU_CONST_GETTER
		friend constexpr half MUU_VECTORCALL operator+(half lhs, half rhs) noexcept
		{
			return half{ static_cast<float>(lhs) + static_cast<float>(rhs) };
		}

		MUU_CONST_GETTER
		friend constexpr half MUU_VECTORCALL operator-(half lhs, half rhs) noexcept
		{
			return half{ static_cast<float>(lhs) - static_cast<float>(rhs) };
		}

		MUU_CONST_GETTER
		friend constexpr half MUU_VECTORCALL operator*(half lhs, half rhs) noexcept
		{
			return half{ static_cast<float>(lhs) * static_cast<float>(rhs) };
		}

		MUU_CONST_GETTER
		friend constexpr half MUU_VECTORCALL operator/(half lhs, half rhs) noexcept
		{
			return half{ static_cast<float>(lhs) / static_cast<float>(rhs) };
		}

#define HALF_BINARY_OPS(func, return_type, input_type)                                                                 \
	func(return_type, input_type, +);                                                                                  \
	func(return_type, input_type, -);                                                                                  \
	func(return_type, input_type, *);                                                                                  \
	func(return_type, input_type, /)

#if MUU_HAS_FLOAT16
		HALF_BINARY_OPS(HALF_PROMOTING_BINARY_OP, _Float16, _Float16);
#endif
		HALF_BINARY_OPS(HALF_PROMOTING_BINARY_OP, float, float);
		HALF_BINARY_OPS(HALF_PROMOTING_BINARY_OP, double, double);
		HALF_BINARY_OPS(HALF_PROMOTING_BINARY_OP, long double, long double);
#if MUU_HAS_FLOAT128
		HALF_BINARY_OPS(HALF_PROMOTING_BINARY_OP, float128_t, float128_t);
#endif
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, half, char);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, half, signed char);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, half, unsigned char);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, half, signed short);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, half, unsigned short);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, half, signed int);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, half, unsigned int);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, half, signed long);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, half, unsigned long);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, half, signed long long);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, half, unsigned long long);
#if MUU_HAS_INT128
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, half, int128_t);
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, half, uint128_t);
#endif
#undef HALF_BINARY_OPS
#undef HALF_PROMOTING_BINARY_OP
#undef HALF_CONVERTING_BINARY_OP

		//====================================================
		// ARITHMETIC ASSIGNMENTS
		//====================================================

		friend constexpr half& operator+=(half& lhs, half rhs) noexcept
		{
#if MUU_HALF_EMULATED
			lhs.impl_ = impl::f32_to_u16(static_cast<float>(lhs) + static_cast<float>(rhs));
#else
			lhs.impl_ = static_cast<impl_type>(lhs.impl_ + rhs.impl_);
#endif
			return lhs;
		}

		friend constexpr half& operator-=(half& lhs, half rhs) noexcept
		{
#if MUU_HALF_EMULATED
			lhs.impl_ = impl::f32_to_u16(static_cast<float>(lhs) - static_cast<float>(rhs));
#else
			lhs.impl_ = static_cast<impl_type>(lhs.impl_ - rhs.impl_);
#endif
			return lhs;
		}

		friend constexpr half& operator*=(half& lhs, half rhs) noexcept
		{
#if MUU_HALF_EMULATED
			lhs.impl_ = impl::f32_to_u16(static_cast<float>(lhs) * static_cast<float>(rhs));
#else
			lhs.impl_ = static_cast<impl_type>(lhs.impl_ * rhs.impl_);
#endif
			return lhs;
		}

		friend constexpr half& operator/=(half& lhs, half rhs) noexcept
		{
#if MUU_HALF_EMULATED
			lhs.impl_ = impl::f32_to_u16(static_cast<float>(lhs) / static_cast<float>(rhs));
#else
			lhs.impl_ = static_cast<impl_type>(lhs.impl_ / rhs.impl_);
#endif
			return lhs;
		}

#if MUU_HALF_EMULATED
	#define HALF_DEMOTING_ASSIGN_OP(input_type, op)                                                                    \
		friend constexpr half& operator op##=(half& lhs, input_type rhs) noexcept                                      \
		{                                                                                                              \
			lhs.impl_ = impl::f32_to_u16(static_cast<float>(lhs op rhs));                                              \
			return lhs;                                                                                                \
		}                                                                                                              \
		static_assert(true)

	#define HALF_CONVERTING_ASSIGN_OP(input_type, op)                                                                  \
		friend constexpr half& operator op##=(half& lhs, input_type rhs) noexcept                                      \
		{                                                                                                              \
			lhs.impl_ = impl::f32_to_u16(static_cast<float>(lhs) op rhs);                                              \
			return lhs;                                                                                                \
		}                                                                                                              \
		static_assert(true)

	#define HALF_CASTING_ASSIGN_OP(input_type, op)                                                                     \
		friend constexpr half& operator op##=(half& lhs, input_type rhs) noexcept                                      \
		{                                                                                                              \
			lhs.impl_ = muu::bit_cast<impl_type>(lhs op rhs);                                                          \
			return lhs;                                                                                                \
		}                                                                                                              \
		static_assert(true)
#else
	#define HALF_DEMOTING_ASSIGN_OP(input_type, op)                                                                    \
		friend constexpr half& operator op##=(half& lhs, input_type rhs) noexcept                                      \
		{                                                                                                              \
			lhs.impl_ = static_cast<impl_type>(lhs op rhs);                                                            \
			return lhs;                                                                                                \
		}                                                                                                              \
		static_assert(true)

	#define HALF_CONVERTING_ASSIGN_OP(input_type, op)                                                                  \
		friend constexpr half& operator op##=(half& lhs, input_type rhs) noexcept                                      \
		{                                                                                                              \
			lhs.impl_ = static_cast<impl_type>(static_cast<float>(lhs) op rhs);                                        \
			return lhs;                                                                                                \
		}                                                                                                              \
		static_assert(true)

	#define HALF_CASTING_ASSIGN_OP(input_type, op)                                                                     \
		friend constexpr half& operator op##=(half& lhs, input_type rhs) noexcept                                      \
		{                                                                                                              \
			lhs.impl_ = static_cast<impl_type>(lhs op rhs);                                                            \
			return lhs;                                                                                                \
		}                                                                                                              \
		static_assert(true)
#endif

#define HALF_BINARY_OPS(func, input_type)                                                                              \
	func(input_type, +);                                                                                               \
	func(input_type, -);                                                                                               \
	func(input_type, *);                                                                                               \
	func(input_type, /)

#if MUU_HAS_FLOAT16
		HALF_BINARY_OPS(HALF_CASTING_ASSIGN_OP, _Float16);
#endif
		HALF_BINARY_OPS(HALF_DEMOTING_ASSIGN_OP, float);
		HALF_BINARY_OPS(HALF_DEMOTING_ASSIGN_OP, double);
		HALF_BINARY_OPS(HALF_DEMOTING_ASSIGN_OP, long double);
#if MUU_HAS_FLOAT128
		HALF_BINARY_OPS(HALF_DEMOTING_ASSIGN_OP, float128_t);
#endif
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP, char);
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP, signed char);
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP, unsigned char);
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP, signed short);
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP, unsigned short);
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP, signed int);
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP, unsigned int);
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP, signed long);
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP, unsigned long);
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP, signed long long);
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP, unsigned long long);
#if MUU_HAS_INT128
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP, int128_t);
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP, uint128_t);
#endif
#undef HALF_BINARY_OPS
#undef HALF_DEMOTING_ASSIGN_OP
#undef HALF_CONVERTING_ASSIGN_OP
#undef HALF_CASTING_ASSIGN_OP

		//====================================================
		// INCREMENTS AND DECREMENTS
		//====================================================

		constexpr half& operator++() noexcept
		{
#if MUU_HALF_EMULATED
			impl_ = impl::f32_to_u16(static_cast<float>(*this) + 1.0f);
#else
			impl_++;
#endif
			return *this;
		}

		constexpr half& operator--() noexcept
		{
#if MUU_HALF_EMULATED
			impl_ = impl::f32_to_u16(static_cast<float>(*this) - 1.0f);
#else
			impl_--;
#endif
			return *this;
		}

		constexpr half operator++(int) noexcept
		{
			half prev = *this;
#if MUU_HALF_EMULATED
			impl_ = impl::f32_to_u16(static_cast<float>(*this) + 1.0f);
#else
			impl_++;
#endif
			return prev;
		}

		constexpr half operator--(int) noexcept
		{
			half prev = *this;
#if MUU_HALF_EMULATED
			impl_ = impl::f32_to_u16(static_cast<float>(*this) - 1.0f);
#else
			impl_--;
#endif
			return prev;
		}

		//====================================================
		// MISC
		//====================================================

		MUU_PURE_GETTER
		constexpr half operator+() const noexcept
		{
			return *this;
		}

		MUU_PURE_GETTER
		constexpr half operator-() const noexcept
		{
#if MUU_HALF_EMULATED
			return from_bits(impl_ ^ 0b1000000000000000_u16);
#else
			return half{ -static_cast<float>(impl_) };
#endif
		}

		template <typename Char, typename Traits>
		friend std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& lhs, half rhs)
		{
			return lhs << static_cast<float>(rhs);
		}

		template <typename Char, typename Traits>
		friend std::basic_istream<Char, Traits>& operator>>(std::basic_istream<Char, Traits>& lhs, half& rhs)
		{
			float val;
			if (lhs >> val)
				rhs = static_cast<half>(val);
			return lhs;
		}

		friend constexpr bool MUU_VECTORCALL muu::infinity_or_nan(half) noexcept;
		friend constexpr half MUU_VECTORCALL muu::abs(half) noexcept;
		friend constexpr bool MUU_VECTORCALL muu::approx_equal(half, half, half) noexcept;
		friend constexpr half MUU_VECTORCALL muu::floor(half) noexcept;
		friend constexpr half MUU_VECTORCALL muu::ceil(half) noexcept;
		friend constexpr half MUU_VECTORCALL muu::sqrt(half) noexcept;
		friend constexpr half MUU_VECTORCALL muu::cos(half) noexcept;
		friend constexpr half MUU_VECTORCALL muu::sin(half) noexcept;
		friend constexpr half MUU_VECTORCALL muu::tan(half) noexcept;
		friend constexpr half MUU_VECTORCALL muu::acos(half) noexcept;
		friend constexpr half MUU_VECTORCALL muu::asin(half) noexcept;
		friend constexpr half MUU_VECTORCALL muu::atan(half) noexcept;
		friend constexpr half MUU_VECTORCALL muu::atan2(half, half) noexcept;
		friend constexpr half MUU_VECTORCALL muu::lerp(half, half, half) noexcept;
	};

	namespace build
	{
		/// \brief	True if using #muu::half in constexpr contexts is fully supported on this compiler.
		inline constexpr bool supports_constexpr_half = !MUU_HALF_EMULATED || supports_constexpr_bit_cast;
	}

	/// \cond
	namespace impl
	{
		template <>
		struct integer_limits<half>
		{
#if MUU_HALF_EMULATED
			static constexpr half lowest  = half::from_bits(0b1'11110'1111111111_u16);
			static constexpr half highest = half::from_bits(0b0'11110'1111111111_u16);
#else
			static constexpr half lowest				 = half{ integer_limits<MUU_HALF_IMPL_TYPE>::lowest };
			static constexpr half highest				 = half{ integer_limits<MUU_HALF_IMPL_TYPE>::highest };
#endif
		};

		template <>
		struct integer_positive_constants<half>
		{
#if MUU_HALF_EMULATED
			static constexpr half zero		  = half::from_bits(0b0'00000'0000000000_u16);
			static constexpr half one		  = half::from_bits(0b0'01111'0000000000_u16);
			static constexpr half two		  = half::from_bits(0b0'10000'0000000000_u16);
			static constexpr half three		  = half::from_bits(0b0'10000'1000000000_u16);
			static constexpr half four		  = half::from_bits(0b0'10001'0000000000_u16);
			static constexpr half five		  = half::from_bits(0b0'10001'0100000000_u16);
			static constexpr half six		  = half::from_bits(0b0'10001'1000000000_u16);
			static constexpr half seven		  = half::from_bits(0b0'10001'1100000000_u16);
			static constexpr half eight		  = half::from_bits(0b0'10010'0000000000_u16);
			static constexpr half nine		  = half::from_bits(0b0'10010'0010000000_u16);
			static constexpr half ten		  = half::from_bits(0b0'10010'0100000000_u16);
			static constexpr half one_hundred = half::from_bits(0b0'10101'1001000000_u16);
#else
			using impl_type								 = integer_positive_constants<MUU_HALF_IMPL_TYPE>;
			static constexpr half zero					 = half{ impl_type::zero };
			static constexpr half one					 = half{ impl_type::one };
			static constexpr half two					 = half{ impl_type::two };
			static constexpr half three					 = half{ impl_type::three };
			static constexpr half four					 = half{ impl_type::four };
			static constexpr half five					 = half{ impl_type::five };
			static constexpr half six					 = half{ impl_type::six };
			static constexpr half seven					 = half{ impl_type::seven };
			static constexpr half eight					 = half{ impl_type::eight };
			static constexpr half nine					 = half{ impl_type::nine };
			static constexpr half ten					 = half{ impl_type::ten };
			static constexpr half one_hundred			 = half{ impl_type::one_hundred };
#endif
		};

		template <>
		struct floating_point_traits<half>
		{
			static constexpr int significand_digits = 11;
			static constexpr int decimal_digits		= 3;

#if MUU_HALF_EMULATED
			static constexpr half default_epsilon = half::from_bits(0b0'00101'0000011001_u16); // 0.001
#else
			using impl_type								 = floating_point_traits<MUU_HALF_IMPL_TYPE>;
			static constexpr half default_epsilon		 = half{ impl_type::default_epsilon };
#endif
		};

		template <>
		struct floating_point_special_constants<half>
		{
#if MUU_HALF_EMULATED
			static constexpr half nan				= half::from_bits(0b1'11111'1000000001_u16);
			static constexpr half signaling_nan		= half::from_bits(0b1'11111'0000000001_u16);
			static constexpr half infinity			= half::from_bits(0b0'11111'0000000000_u16);
			static constexpr half negative_infinity = half::from_bits(0b1'11111'0000000000_u16);
			static constexpr half negative_zero		= half::from_bits(0b1'00000'0000000000_u16);
#else
			using impl_type								 = floating_point_special_constants<MUU_HALF_IMPL_TYPE>;
			static constexpr half nan					 = half{ impl_type::nan };
			static constexpr half signaling_nan			 = half{ impl_type::signaling_nan };
			static constexpr half infinity				 = half{ impl_type::infinity };
			static constexpr half negative_infinity		 = half{ impl_type::negative_infinity };
			static constexpr half negative_zero			 = half{ impl_type::negative_zero };
#endif
		};

		template <>
		struct floating_point_named_constants<half>
		{
#if MUU_HALF_EMULATED
			static constexpr half one_over_two			 = half::from_bits(0b0'01110'0000000000_u16);
			static constexpr half one_over_three		 = half::from_bits(0b0'01101'0101010110_u16);
			static constexpr half one_over_four			 = half::from_bits(0b0'01101'0000000000_u16);
			static constexpr half one_over_five			 = half::from_bits(0b0'01100'1001100111_u16);
			static constexpr half one_over_six			 = half::from_bits(0b0'01100'0101010110_u16);
			static constexpr half one_over_seven		 = half::from_bits(0b0'01100'0010010011_u16);
			static constexpr half one_over_eight		 = half::from_bits(0b0'01100'0000000000_u16);
			static constexpr half sqrt_two				 = half::from_bits(0b0'01111'0110101000_u16);
			static constexpr half one_over_sqrt_two		 = half::from_bits(0b0'01110'0110101000_u16);
			static constexpr half two_over_three		 = half::from_bits(0b0'01110'0101010110_u16);
			static constexpr half two_over_five			 = half::from_bits(0b0'01101'1001100111_u16);
			static constexpr half two_over_seven		 = half::from_bits(0b0'01101'0010010011_u16);
			static constexpr half sqrt_three			 = half::from_bits(0b0'01111'1011101110_u16);
			static constexpr half one_over_sqrt_three	 = half::from_bits(0b0'01110'0010011111_u16);
			static constexpr half three_over_two		 = half::from_bits(0b0'01111'1000000000_u16);
			static constexpr half three_over_four		 = half::from_bits(0b0'01110'1000000000_u16);
			static constexpr half three_over_five		 = half::from_bits(0b0'01110'0011001101_u16);
			static constexpr half three_over_seven		 = half::from_bits(0b0'01101'1011011100_u16);
			static constexpr half three_over_eight		 = half::from_bits(0b0'01101'1000000000_u16);
			static constexpr half sqrt_four				 = half::from_bits(0b0'10000'0000000000_u16);
			static constexpr half four_over_three		 = half::from_bits(0b0'01111'0101010110_u16);
			static constexpr half four_over_five		 = half::from_bits(0b0'01110'1001100111_u16);
			static constexpr half four_over_seven		 = half::from_bits(0b0'01110'0010010011_u16);
			static constexpr half pi					 = half::from_bits(0b0'10000'1001001001_u16);
			static constexpr half one_over_pi			 = half::from_bits(0b0'01101'0100011000_u16);
			static constexpr half sqrt_pi				 = half::from_bits(0b0'01111'1100010111_u16);
			static constexpr half one_over_sqrt_pi		 = half::from_bits(0b0'01110'0010000100_u16);
			static constexpr half pi_over_two			 = half::from_bits(0b0'01111'1001001001_u16);
			static constexpr half pi_over_three			 = half::from_bits(0b0'01111'0000110001_u16);
			static constexpr half pi_over_four			 = half::from_bits(0b0'01110'1001001001_u16);
			static constexpr half pi_over_five			 = half::from_bits(0b0'01110'0100000111_u16);
			static constexpr half pi_over_six			 = half::from_bits(0b0'01110'0000110001_u16);
			static constexpr half pi_over_seven			 = half::from_bits(0b0'01101'1100101111_u16);
			static constexpr half pi_over_eight			 = half::from_bits(0b0'01101'1001001001_u16);
			static constexpr half two_over_pi			 = half::from_bits(0b0'01110'0100011000_u16);
			static constexpr half three_over_pi			 = half::from_bits(0b0'01110'1110100100_u16);
			static constexpr half four_over_pi			 = half::from_bits(0b0'01111'0100011000_u16);
			static constexpr half five_over_pi			 = half::from_bits(0b0'01111'1001011110_u16);
			static constexpr half six_over_pi			 = half::from_bits(0b0'01111'1110100100_u16);
			static constexpr half seven_over_pi			 = half::from_bits(0b0'10000'0001110101_u16);
			static constexpr half eight_over_pi			 = half::from_bits(0b0'10000'0100011000_u16);
			static constexpr half two_pi				 = half::from_bits(0b0'10001'1001001001_u16);
			static constexpr half one_over_two_pi		 = half::from_bits(0b0'01100'0100011000_u16);
			static constexpr half sqrt_two_pi			 = half::from_bits(0b0'10000'0100000100_u16);
			static constexpr half one_over_sqrt_two_pi	 = half::from_bits(0b0'01101'1001100010_u16);
			static constexpr half two_pi_over_three		 = half::from_bits(0b0'10000'0000110001_u16);
			static constexpr half two_pi_over_five		 = half::from_bits(0b0'01111'0100000111_u16);
			static constexpr half two_pi_over_seven		 = half::from_bits(0b0'01110'1100101111_u16);
			static constexpr half three_over_two_pi		 = half::from_bits(0b0'01101'1110100100_u16);
			static constexpr half five_over_two_pi		 = half::from_bits(0b0'01110'1001011110_u16);
			static constexpr half seven_over_two_pi		 = half::from_bits(0b0'01111'0001110101_u16);
			static constexpr half three_pi				 = half::from_bits(0b0'10010'0010110111_u16);
			static constexpr half one_over_three_pi		 = half::from_bits(0b0'01011'1011001011_u16);
			static constexpr half sqrt_three_pi			 = half::from_bits(0b0'10000'1000100100_u16);
			static constexpr half one_over_sqrt_three_pi = half::from_bits(0b0'01101'0100110110_u16);
			static constexpr half three_pi_over_two		 = half::from_bits(0b0'10001'0010110111_u16);
			static constexpr half three_pi_over_four	 = half::from_bits(0b0'10000'0010110111_u16);
			static constexpr half three_pi_over_five	 = half::from_bits(0b0'01111'1110001010_u16);
			static constexpr half three_pi_over_seven	 = half::from_bits(0b0'01111'0101100011_u16);
			static constexpr half three_pi_over_eight	 = half::from_bits(0b0'01111'0010110111_u16);
			static constexpr half two_over_three_pi		 = half::from_bits(0b0'01100'1011001011_u16);
			static constexpr half four_over_three_pi	 = half::from_bits(0b0'01101'1011001011_u16);
			static constexpr half five_over_three_pi	 = half::from_bits(0b0'01110'0000111111_u16);
			static constexpr half seven_over_three_pi	 = half::from_bits(0b0'01110'0111110001_u16);
			static constexpr half eight_over_three_pi	 = half::from_bits(0b0'01110'1011001011_u16);
			static constexpr half e						 = half::from_bits(0b0'10000'0101110000_u16);
			static constexpr half one_over_e			 = half::from_bits(0b0'01101'0111100011_u16);
			static constexpr half sqrt_e				 = half::from_bits(0b0'01111'1010011001_u16);
			static constexpr half one_over_sqrt_e		 = half::from_bits(0b0'01110'0011011010_u16);
			static constexpr half e_over_two			 = half::from_bits(0b0'01111'0101110000_u16);
			static constexpr half e_over_three			 = half::from_bits(0b0'01110'1101000000_u16);
			static constexpr half e_over_four			 = half::from_bits(0b0'01110'0101110000_u16);
			static constexpr half e_over_five			 = half::from_bits(0b0'01110'0001011010_u16);
			static constexpr half e_over_six			 = half::from_bits(0b0'01101'1101000000_u16);
			static constexpr half e_over_seven			 = half::from_bits(0b0'01101'1000110111_u16);
			static constexpr half e_over_eight			 = half::from_bits(0b0'01101'0101110000_u16);
			static constexpr half two_over_e			 = half::from_bits(0b0'01110'0111100011_u16);
			static constexpr half three_over_e			 = half::from_bits(0b0'01111'0001101010_u16);
			static constexpr half four_over_e			 = half::from_bits(0b0'01111'0111100011_u16);
			static constexpr half five_over_e			 = half::from_bits(0b0'01111'1101011100_u16);
			static constexpr half six_over_e			 = half::from_bits(0b0'10000'0001101010_u16);
			static constexpr half seven_over_e			 = half::from_bits(0b0'10000'0100100111_u16);
			static constexpr half eight_over_e			 = half::from_bits(0b0'10000'0111100011_u16);
			static constexpr half phi					 = half::from_bits(0b0'01111'1001111001_u16);
			static constexpr half one_over_phi			 = half::from_bits(0b0'01110'0011110010_u16);
			static constexpr half sqrt_phi				 = half::from_bits(0b0'01111'0100010111_u16);
			static constexpr half one_over_sqrt_phi		 = half::from_bits(0b0'01110'1001001010_u16);
			static constexpr half phi_over_two			 = half::from_bits(0b0'01110'1001111001_u16);
			static constexpr half phi_over_three		 = half::from_bits(0b0'01110'0001010001_u16);
			static constexpr half phi_over_four			 = half::from_bits(0b0'01101'1001111001_u16);
			static constexpr half phi_over_five			 = half::from_bits(0b0'01101'0100101110_u16);
			static constexpr half phi_over_six			 = half::from_bits(0b0'01101'0001010001_u16);
			static constexpr half phi_over_seven		 = half::from_bits(0b0'01100'1101100110_u16);
			static constexpr half phi_over_eight		 = half::from_bits(0b0'01100'1001111001_u16);
			static constexpr half two_over_phi			 = half::from_bits(0b0'01111'0011110010_u16);
			static constexpr half three_over_phi		 = half::from_bits(0b0'01111'1101101011_u16);
			static constexpr half four_over_phi			 = half::from_bits(0b0'10000'0011110010_u16);
			static constexpr half five_over_phi			 = half::from_bits(0b0'10000'1000101110_u16);
			static constexpr half six_over_phi			 = half::from_bits(0b0'10000'1101101011_u16);
			static constexpr half seven_over_phi		 = half::from_bits(0b0'10001'0001010100_u16);
			static constexpr half eight_over_phi		 = half::from_bits(0b0'10001'0011110010_u16);
			static constexpr half one_over_ten			 = half::from_bits(0b0'01011'1001100111_u16);
			static constexpr half one_over_one_hundred	 = half::from_bits(0b0'01000'0100011111_u16);
			static constexpr half one_over_one_thousand	 = half::from_bits(0b0'00101'0000011001_u16);
			static constexpr half degrees_to_radians	 = half::from_bits(0b0'01001'0001111000_u16);
			static constexpr half radians_to_degrees	 = half::from_bits(0b0'10100'1100101010_u16);
#else
			using impl_type								 = floating_point_named_constants<MUU_HALF_IMPL_TYPE>;
			static constexpr half one_over_two			 = half{ impl_type::one_over_two };
			static constexpr half one_over_three		 = half{ impl_type::one_over_three };
			static constexpr half one_over_four			 = half{ impl_type::one_over_four };
			static constexpr half one_over_five			 = half{ impl_type::one_over_five };
			static constexpr half one_over_six			 = half{ impl_type::one_over_six };
			static constexpr half one_over_seven		 = half{ impl_type::one_over_seven };
			static constexpr half one_over_eight		 = half{ impl_type::one_over_eight };
			static constexpr half sqrt_two				 = half{ impl_type::sqrt_two };
			static constexpr half one_over_sqrt_two		 = half{ impl_type::one_over_sqrt_two };
			static constexpr half two_over_three		 = half{ impl_type::two_over_three };
			static constexpr half two_over_five			 = half{ impl_type::two_over_five };
			static constexpr half two_over_seven		 = half{ impl_type::two_over_seven };
			static constexpr half sqrt_three			 = half{ impl_type::sqrt_three };
			static constexpr half one_over_sqrt_three	 = half{ impl_type::one_over_sqrt_three };
			static constexpr half three_over_two		 = half{ impl_type::three_over_two };
			static constexpr half three_over_four		 = half{ impl_type::three_over_four };
			static constexpr half three_over_five		 = half{ impl_type::three_over_five };
			static constexpr half three_over_seven		 = half{ impl_type::three_over_seven };
			static constexpr half three_over_eight		 = half{ impl_type::three_over_eight };
			static constexpr half sqrt_four				 = half{ impl_type::sqrt_four };
			static constexpr half four_over_three		 = half{ impl_type::four_over_three };
			static constexpr half four_over_five		 = half{ impl_type::four_over_five };
			static constexpr half four_over_seven		 = half{ impl_type::four_over_seven };
			static constexpr half pi					 = half{ impl_type::pi };
			static constexpr half one_over_pi			 = half{ impl_type::one_over_pi };
			static constexpr half sqrt_pi				 = half{ impl_type::sqrt_pi };
			static constexpr half one_over_sqrt_pi		 = half{ impl_type::one_over_sqrt_pi };
			static constexpr half pi_over_two			 = half{ impl_type::pi_over_two };
			static constexpr half pi_over_three			 = half{ impl_type::pi_over_three };
			static constexpr half pi_over_four			 = half{ impl_type::pi_over_four };
			static constexpr half pi_over_five			 = half{ impl_type::pi_over_five };
			static constexpr half pi_over_six			 = half{ impl_type::pi_over_six };
			static constexpr half pi_over_seven			 = half{ impl_type::pi_over_seven };
			static constexpr half pi_over_eight			 = half{ impl_type::pi_over_eight };
			static constexpr half two_over_pi			 = half{ impl_type::two_over_pi };
			static constexpr half three_over_pi			 = half{ impl_type::three_over_pi };
			static constexpr half four_over_pi			 = half{ impl_type::four_over_pi };
			static constexpr half five_over_pi			 = half{ impl_type::five_over_pi };
			static constexpr half six_over_pi			 = half{ impl_type::six_over_pi };
			static constexpr half seven_over_pi			 = half{ impl_type::seven_over_pi };
			static constexpr half eight_over_pi			 = half{ impl_type::eight_over_pi };
			static constexpr half two_pi				 = half{ impl_type::two_pi };
			static constexpr half one_over_two_pi		 = half{ impl_type::one_over_two_pi };
			static constexpr half sqrt_two_pi			 = half{ impl_type::sqrt_two_pi };
			static constexpr half one_over_sqrt_two_pi	 = half{ impl_type::one_over_sqrt_two_pi };
			static constexpr half two_pi_over_three		 = half{ impl_type::two_pi_over_three };
			static constexpr half two_pi_over_five		 = half{ impl_type::two_pi_over_five };
			static constexpr half two_pi_over_seven		 = half{ impl_type::two_pi_over_seven };
			static constexpr half three_over_two_pi		 = half{ impl_type::three_over_two_pi };
			static constexpr half five_over_two_pi		 = half{ impl_type::five_over_two_pi };
			static constexpr half seven_over_two_pi		 = half{ impl_type::seven_over_two_pi };
			static constexpr half three_pi				 = half{ impl_type::three_pi };
			static constexpr half one_over_three_pi		 = half{ impl_type::one_over_three_pi };
			static constexpr half sqrt_three_pi			 = half{ impl_type::sqrt_three_pi };
			static constexpr half one_over_sqrt_three_pi = half{ impl_type::one_over_sqrt_three_pi };
			static constexpr half three_pi_over_two		 = half{ impl_type::three_pi_over_two };
			static constexpr half three_pi_over_four	 = half{ impl_type::three_pi_over_four };
			static constexpr half three_pi_over_five	 = half{ impl_type::three_pi_over_five };
			static constexpr half three_pi_over_seven	 = half{ impl_type::three_pi_over_seven };
			static constexpr half three_pi_over_eight	 = half{ impl_type::three_pi_over_eight };
			static constexpr half two_over_three_pi		 = half{ impl_type::two_over_three_pi };
			static constexpr half four_over_three_pi	 = half{ impl_type::four_over_three_pi };
			static constexpr half five_over_three_pi	 = half{ impl_type::five_over_three_pi };
			static constexpr half seven_over_three_pi	 = half{ impl_type::seven_over_three_pi };
			static constexpr half eight_over_three_pi	 = half{ impl_type::eight_over_three_pi };
			static constexpr half e						 = half{ impl_type::e };
			static constexpr half one_over_e			 = half{ impl_type::one_over_e };
			static constexpr half sqrt_e				 = half{ impl_type::sqrt_e };
			static constexpr half one_over_sqrt_e		 = half{ impl_type::one_over_sqrt_e };
			static constexpr half e_over_two			 = half{ impl_type::e_over_two };
			static constexpr half e_over_three			 = half{ impl_type::e_over_three };
			static constexpr half e_over_four			 = half{ impl_type::e_over_four };
			static constexpr half e_over_five			 = half{ impl_type::e_over_five };
			static constexpr half e_over_six			 = half{ impl_type::e_over_six };
			static constexpr half e_over_seven			 = half{ impl_type::e_over_seven };
			static constexpr half e_over_eight			 = half{ impl_type::e_over_eight };
			static constexpr half two_over_e			 = half{ impl_type::two_over_e };
			static constexpr half three_over_e			 = half{ impl_type::three_over_e };
			static constexpr half four_over_e			 = half{ impl_type::four_over_e };
			static constexpr half five_over_e			 = half{ impl_type::five_over_e };
			static constexpr half six_over_e			 = half{ impl_type::six_over_e };
			static constexpr half seven_over_e			 = half{ impl_type::seven_over_e };
			static constexpr half eight_over_e			 = half{ impl_type::eight_over_e };
			static constexpr half phi					 = half{ impl_type::phi };
			static constexpr half one_over_phi			 = half{ impl_type::one_over_phi };
			static constexpr half sqrt_phi				 = half{ impl_type::sqrt_phi };
			static constexpr half one_over_sqrt_phi		 = half{ impl_type::one_over_sqrt_phi };
			static constexpr half phi_over_two			 = half{ impl_type::phi_over_two };
			static constexpr half phi_over_three		 = half{ impl_type::phi_over_three };
			static constexpr half phi_over_four			 = half{ impl_type::phi_over_four };
			static constexpr half phi_over_five			 = half{ impl_type::phi_over_five };
			static constexpr half phi_over_six			 = half{ impl_type::phi_over_six };
			static constexpr half phi_over_seven		 = half{ impl_type::phi_over_seven };
			static constexpr half phi_over_eight		 = half{ impl_type::phi_over_eight };
			static constexpr half two_over_phi			 = half{ impl_type::two_over_phi };
			static constexpr half three_over_phi		 = half{ impl_type::three_over_phi };
			static constexpr half four_over_phi			 = half{ impl_type::four_over_phi };
			static constexpr half five_over_phi			 = half{ impl_type::five_over_phi };
			static constexpr half six_over_phi			 = half{ impl_type::six_over_phi };
			static constexpr half seven_over_phi		 = half{ impl_type::seven_over_phi };
			static constexpr half eight_over_phi		 = half{ impl_type::eight_over_phi };
			static constexpr half one_over_ten			 = half{ impl_type::one_over_ten };
			static constexpr half one_over_one_hundred	 = half{ impl_type::one_over_one_hundred };
			static constexpr half one_over_one_thousand	 = half{ impl_type::one_over_one_thousand };
			static constexpr half degrees_to_radians	 = half{ impl_type::degrees_to_radians };
			static constexpr half radians_to_degrees	 = half{ impl_type::radians_to_degrees };
#endif
		};
	}
	/// \endcond

	/// \brief	16-bit half-precision float constants.
	///
	/// \ingroup	constants
	template <>
	struct constants<half> : impl::floating_point_constants<half>
	{};

	inline namespace literals
	{
		/// \brief	Literal for creating a half-precision float.
		/// \detail \cpp
		/// const half val = 42.5_f16;
		/// \ecpp
		///
		/// \relatesalso	muu::half
		MUU_CONST_GETTER
		MUU_CONSTEVAL
		half operator"" _f16(long double val) noexcept
		{
			return half{ val };
		}

		/// \brief	Literal for creating a half-precision float.
		/// \detail \cpp
		/// const half val = 42_f16;
		/// \ecpp
		///
		/// \relatesalso	muu::half
		MUU_CONST_GETTER
		MUU_CONSTEVAL
		half operator"" _f16(unsigned long long val) noexcept
		{
			return half{ val };
		}
	}

	/// \brief	Returns true if a half-precision float is infinity or NaN.
	///
	/// \ingroup	infinity_or_nan
	/// \relatesalso	muu::half
	MUU_CONST_INLINE_GETTER
	constexpr bool MUU_VECTORCALL infinity_or_nan(half x) noexcept
	{
#if MUU_HALF_EMULATED
		return (x.impl_ & 0b0111110000000000_u16) == 0b0111110000000000_u16;
#else
		return impl::infinity_or_nan_(x.impl_);
#endif
	}

	/// \brief	Returns the absolute value of a half-precision float.
	///
	/// \ingroup	abs
	/// \relatesalso	muu::half
	MUU_CONST_GETTER
	constexpr half MUU_VECTORCALL abs(half x) noexcept
	{
#if MUU_HALF_EMULATED

		if ((x.impl_ & 0b0111110000000000_u16) == 0b0111110000000000_u16 // inf or nan
			&& (x.impl_ & 0b0000001111111111_u16) != 0_u16)				 // any manitssa bits set == nan
			return x;
		return half::from_bits(x.impl_ & 0b0111111111111111_u16);

#else
		return half{ muu::abs(x.impl_) };
#endif
	}

	/// \brief	Returns true if two half-precision floats are approximately equal.
	///
	/// \ingroup	approx_equal
	/// \relatesalso	muu::half
	MUU_CONST_GETTER
	constexpr bool MUU_VECTORCALL approx_equal(half a, half b, half epsilon = constants<half>::default_epsilon) noexcept
	{
		return muu::approx_equal(static_cast<float>(a), static_cast<float>(b), static_cast<float>(epsilon));
	}

	/// \brief	Returns true a half-precision float is approximately equal to zero.
	///
	/// \ingroup	approx_zero
	/// \relatesalso	muu::half
	MUU_CONST_INLINE_GETTER
	constexpr bool MUU_VECTORCALL approx_zero(half x, half epsilon = constants<half>::default_epsilon) noexcept
	{
		return muu::approx_equal(static_cast<float>(x), 0.0f, static_cast<float>(epsilon));
	}

	/// \brief	Returns the floor of a half-precision float.
	///
	/// \ingroup	floor
	/// \relatesalso	muu::half
	MUU_CONST_INLINE_GETTER
	constexpr half MUU_VECTORCALL floor(half x) noexcept
	{
		return impl::floor_(x);
	}

	/// \brief	Returns the ceiling of a half-precision float.
	///
	/// \ingroup	ceil
	/// \relatesalso	muu::half
	MUU_CONST_INLINE_GETTER
	constexpr half MUU_VECTORCALL ceil(half x) noexcept
	{
		return impl::ceil_(x);
	}

	/// \brief	Returns the square-root of a half-precision float.
	///
	/// \ingroup	consteval_sqrt
	/// \relatesalso	muu::half
	MUU_CONST_INLINE_GETTER
	constexpr half consteval_sqrt(half x) noexcept
	{
		return half{ muu::consteval_sqrt(static_cast<float>(x)) };
	}

	/// \brief	Returns the square-root of a half-precision float.
	///
	/// \ingroup	sqrt
	/// \relatesalso	muu::half
	MUU_CONST_INLINE_GETTER
	constexpr half MUU_VECTORCALL sqrt(half x) noexcept
	{
		return half{ muu::sqrt(static_cast<float>(x)) };
	}

	/// \brief	Returns the cosine of a half-precision float.
	///
	/// \ingroup	cos
	/// \relatesalso	muu::half
	MUU_CONST_INLINE_GETTER
	constexpr half MUU_VECTORCALL cos(half x) noexcept
	{
		return half{ muu::cos(static_cast<float>(x)) };
	}

	/// \brief	Returns the sine of a half-precision float.
	///
	/// \ingroup	sin
	/// \relatesalso	muu::half
	MUU_CONST_INLINE_GETTER
	constexpr half MUU_VECTORCALL sin(half x) noexcept
	{
		return half{ muu::sin(static_cast<float>(x)) };
	}

	/// \brief	Returns the tangent of a half-precision float.
	///
	/// \ingroup	tan
	/// \relatesalso	muu::half
	MUU_CONST_INLINE_GETTER
	constexpr half MUU_VECTORCALL tan(half x) noexcept
	{
		return half{ muu::tan(static_cast<float>(x)) };
	}

	/// \brief	Returns the arc cosine of a half-precision float.
	///
	/// \ingroup	acos
	/// \relatesalso	muu::half
	MUU_CONST_INLINE_GETTER
	constexpr half MUU_VECTORCALL acos(half x) noexcept
	{
		return half{ muu::acos(static_cast<float>(x)) };
	}

	/// \brief	Returns the arc sine of a half-precision float.
	///
	/// \ingroup	asin
	/// \relatesalso	muu::half
	MUU_CONST_INLINE_GETTER
	constexpr half MUU_VECTORCALL asin(half x) noexcept
	{
		return half{ muu::asin(static_cast<float>(x)) };
	}

	/// \brief	Returns the arc tangent of a half-precision float.
	///
	/// \ingroup	atan
	/// \relatesalso	muu::half
	MUU_CONST_INLINE_GETTER
	constexpr half MUU_VECTORCALL atan(half x) noexcept
	{
		return half{ muu::atan(static_cast<float>(x)) };
	}

	/// \brief	Returns the arc tangent of a half-precision float.
	///
	/// \ingroup	atan2
	/// \relatesalso	muu::half
	MUU_CONST_INLINE_GETTER
	constexpr half MUU_VECTORCALL atan2(half y, half x) noexcept
	{
		return half{ muu::atan2(static_cast<float>(x), static_cast<float>(y)) };
	}

	/// \brief	Returns a linear interpolation between two half-precision floats.
	///
	/// \ingroup	lerp
	/// \relatesalso	muu::half
	MUU_CONST_INLINE_GETTER
	constexpr half MUU_VECTORCALL lerp(half start, half finish, half alpha) noexcept
	{
		return half{ muu::lerp(static_cast<float>(start), static_cast<float>(finish), static_cast<float>(alpha)) };
	}

	/// \brief	Normalizes the angle value of a half-precision float.
	///
	/// \ingroup	normalize_angle
	/// \relatesalso	muu::half
	MUU_CONST_INLINE_GETTER
	constexpr half MUU_VECTORCALL normalize_angle(half x) noexcept
	{
		return half{ muu::normalize_angle(static_cast<float>(x)) };
	}

	/// \brief	Normalizes the angle value of a half-precision float.
	///
	/// \ingroup	normalize_angle_signed
	/// \relatesalso	muu::half
	MUU_CONST_INLINE_GETTER
	constexpr half MUU_VECTORCALL normalize_angle_signed(half x) noexcept
	{
		return half{ muu::normalize_angle_signed(static_cast<float>(x)) };
	}
}

/// \cond
#if MUU_HALF_EMULATED
namespace muu::impl
{
	inline constexpr int8_t f16_single_exp_bias = 127;
	inline constexpr int8_t f16_half_exp_bias	= 15;

	MUU_CONST_GETTER
	constexpr uint16_t MUU_VECTORCALL f32_to_u16_naive(float val) noexcept
	{
		const uint32_t bits32 = muu::bit_cast<uint32_t>(val);
		const uint16_t s16	  = static_cast<uint16_t>((bits32 & 0x80000000u) >> 16);

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

	MUU_CONST_GETTER
	constexpr float MUU_VECTORCALL u16_to_f32_naive(uint16_t val) noexcept
	{
		// 1000 0000 0000 0000 ->
		// 1000 0000 0000 0000 0000 0000 0000 0000
		const uint32_t s32 = static_cast<uint32_t>(val & 0x8000u) << 16;

		// 0111 1100 0000 0000
		const uint16_t exp16 = static_cast<uint16_t>((val & 0x7C00u) >> 10);

		uint32_t exp32 = static_cast<uint32_t>(exp16) + (f16_single_exp_bias - f16_half_exp_bias);

		// 0000 0011 1111 1111
		const uint16_t frac16 = static_cast<uint16_t>(val & 0x03FFu);

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

		exp32  = exp32 << 23;
		frac32 = frac32 << 13;
		return muu::bit_cast<float>(s32 | exp32 | frac32);
	}

	#if MUU_HALF_USE_INTRINSICS

	MUU_CONST_INLINE_GETTER
	uint16_t MUU_VECTORCALL f32_to_u16_intrinsic(float val) noexcept
	{
		//_mm_load_ss			load a single float into a m128 (sse)
		//_mm_cvtps_ph			convert floats in a m128 to half-precision floats in an m128i (FP16C)
		//_mm_cvtsi128_si32		returns the first int from an m128i (sse2)

		return static_cast<uint16_t>(
			_mm_cvtsi128_si32(_mm_cvtps_ph(_mm_load_ss(&val), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)));
	}

	MUU_CONST_INLINE_GETTER
	float MUU_VECTORCALL u16_to_f32_intrinsic(uint16_t val) noexcept
	{
		//_mm_cvtsi32_si128		store a single int in an m128i (sse2)
		//_mm_cvtph_ps			convert half-precision floats in a m128i to floats in an m128 (FP16C)
		//_mm_cvtss_f32			returns the first float from an m128 (sse)

		return _mm_cvtss_f32(_mm_cvtph_ps(_mm_cvtsi32_si128(static_cast<int>(val))));
	}

	#endif // MUU_HALF_USE_INTRINSICS

	MUU_CONST_INLINE_GETTER
	constexpr uint16_t MUU_VECTORCALL f32_to_u16(float val) noexcept
	{
	#if MUU_HALF_USE_INTRINSICS
		if constexpr (build::supports_is_constant_evaluated)
		{
			MUU_IF_CONSTEVAL
			{
				return f32_to_u16_naive(val);
			}
			else
			{
				return f32_to_u16_intrinsic(val);
			}
		}
		else
	#endif
			return f32_to_u16_naive(val);
	}

	MUU_CONST_INLINE_GETTER
	constexpr float MUU_VECTORCALL u16_to_f32(uint16_t val) noexcept
	{
	#if MUU_HALF_USE_INTRINSICS
		if constexpr (build::supports_is_constant_evaluated)
		{
			MUU_IF_CONSTEVAL
			{
				return u16_to_f32_naive(val);
			}
			else
			{
				return u16_to_f32_intrinsic(val);
			}
		}
		else
	#endif
			return u16_to_f32_naive(val);
	}
}
#endif // MUU_HALF_EMULATED
/// \endcond

namespace std
{
	/// \brief	Specialization of std::numeric_limits for muu::half.
	///
	/// \ingroup constants
	template <>
	struct numeric_limits<muu::half>
	{
		using half = muu::half;

		static constexpr auto is_specialized	= true;
		static constexpr auto is_signed			= true;
		static constexpr auto is_integer		= false;
		static constexpr auto is_exact			= false;
		static constexpr auto has_infinity		= true;
		static constexpr auto has_quiet_NaN		= true;
		static constexpr auto has_signaling_NaN = true;
		static constexpr auto round_style		= float_round_style::round_to_nearest;
		static constexpr auto is_iec559			= true;
		static constexpr auto is_bounded		= true;
		static constexpr auto is_modulo			= false;
		static constexpr auto radix				= 2;
		static constexpr auto digits			= 11;  // IEE754-compliant FLT16_MANT_DIG
		static constexpr auto digits10			= 3;   // IEE754-compliant FLT16_DIG
		static constexpr auto min_exponent		= -13; // IEE754-compliant FLT16_MIN_EXP
		static constexpr auto min_exponent10	= -4;  // IEE754-compliant FLT16_MIN_10_EXP
		static constexpr auto max_exponent		= 16;  // IEE754-compliant FLT16_MAX_EXP
		static constexpr auto max_exponent10	= 4;   // IEE754-compliant FLT16_MAX_10_EXP
		static constexpr auto max_digits10		= 5;
		static constexpr auto traps				= false;
		static constexpr auto tinyness_before	= false;

		MUU_CONST_GETTER
		static constexpr half min() noexcept
		{
#if MUU_HALF_EMULATED
			using namespace muu::literals;
			return half::from_bits(0x0400_u16); // 0.000061035 (ish)
#else
			return half{ (std::numeric_limits<MUU_HALF_IMPL_TYPE>::min)() };
#endif
		}

		MUU_CONST_GETTER
		static constexpr half lowest() noexcept
		{
			return half::constants::lowest;
		}

		MUU_CONST_GETTER
		static constexpr half max() noexcept
		{
			return half::constants::highest;
		}

		MUU_CONST_GETTER
		static constexpr half epsilon() noexcept
		{
#if MUU_HALF_EMULATED
			using namespace muu::literals;
			return half::from_bits(0b0'00101'0000000000_u16); // 0.00097656
#else
			return half{ std::numeric_limits<MUU_HALF_IMPL_TYPE>::epsilon() };
#endif
		}

		MUU_CONST_GETTER
		static constexpr half round_error() noexcept
		{
#if MUU_HALF_EMULATED
			using namespace muu::literals;
			return half::from_bits(0b0'00100'0000000000_u16); // epsilon / 2
#else
			return half{ std::numeric_limits<MUU_HALF_IMPL_TYPE>::round_error() };
#endif
		}

		MUU_CONST_GETTER
		static constexpr half infinity() noexcept
		{
			return half::constants::infinity;
		}

		MUU_CONST_GETTER
		static constexpr half quiet_NaN() noexcept
		{
			return half::constants::nan;
		}

		MUU_CONST_GETTER
		static constexpr half signaling_NaN() noexcept
		{
			return half::constants::signaling_nan;
		}

		MUU_CONST_GETTER
		static constexpr half denorm_min() noexcept
		{
#if MUU_HALF_EMULATED
			using namespace muu::literals;
			return half::from_bits(0x0001_u16); // 0.000000059605 (ish)
#else
			return half{ std::numeric_limits<MUU_HALF_IMPL_TYPE>::denorm_min() };
#endif
		}
	};

	template <>
	struct numeric_limits<const muu::half> : numeric_limits<muu::half>
	{};
	template <>
	struct numeric_limits<volatile muu::half> : numeric_limits<muu::half>
	{};
	template <>
	struct numeric_limits<const volatile muu::half> : numeric_limits<muu::half>
	{};
}

#undef MUU_HALF_IMPL_TYPE
#undef MUU_HALF_EMULATED

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
