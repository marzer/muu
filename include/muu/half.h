// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief Contains the definition of muu::half.

#pragma once
#include "../muu/core.h"
MUU_DISABLE_WARNINGS
#include <iosfwd>
MUU_ENABLE_WARNINGS

// see if we can just wrap a 'real' fp16 type (better codegen while still being binary-compatible)
#if MUU_HAS_FLOAT16
	#define HALF_IMPL_TYPE		_Float16
#elif MUU_HAS_FP16
	#define HALF_IMPL_TYPE		__fp16
#endif
#ifdef MUU_HALF_TYPE
	#define HALF_EMULATED		0
	#define HALF_USE_INTRINSICS	0
#else
	#define HALF_IMPL_TYPE		uint16_t
	#define HALF_EMULATED		1

	// detect SSE, SSE2 and FP16C
	#define HALF_USE_INTRINSICS (MUU_ISET_SSE && MUU_ISET_SSE2)
	#if MUU_GCC && !defined(__F16C__)
		#undef HALF_USE_INTRINSICS
	#endif
	#if MUU_CLANG
		#if !__has_feature(f16c)
			#undef HALF_USE_INTRINSICS
		#endif
	#endif
	#ifndef HALF_USE_INTRINSICS
		#define HALF_USE_INTRINSICS 0
	#endif

#endif

MUU_PUSH_WARNINGS
MUU_DISABLE_ARITHMETIC_WARNINGS
MUU_DISABLE_SPAM_WARNINGS

MUU_NAMESPACE_START
{
	namespace impl
	{
		#if !defined(DOXYGEN) && HALF_EMULATED
		[[nodiscard]] MUU_ATTR(const) constexpr uint16_t MUU_VECTORCALL f32_to_f16(float) noexcept;
		[[nodiscard]] MUU_ATTR(const) constexpr float MUU_VECTORCALL f16_to_f32(uint16_t) noexcept;
		#endif

		struct half_from_bits_tag {};
	}

	/// \brief	A 16-bit "half-precision" floating point type.
	/// \ingroup building_blocks
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
	private:

		using data_type = HALF_IMPL_TYPE;
		data_type data_;

		explicit constexpr half(impl::half_from_bits_tag, uint16_t bits) noexcept
			#if HALF_EMULATED
			: data_{ bits }
			#else
			: data_{ bit_cast<data_type>(bits) }
			#endif
		{}

	public:

		/// \brief	Static constants for this type.
		using constants = muu::constants<half>;

		half() noexcept = default;
		constexpr half(const half&) noexcept = default;
		constexpr half& operator = (const half&) noexcept = default;

		/// \brief	Creates a half-precision float from its raw bit equivalent.
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(const)
		static constexpr half from_bits(uint16_t val) noexcept
		{
			return half{ impl::half_from_bits_tag{}, val };
		}

		MUU_NODISCARD_CTOR
		explicit constexpr half(bool val) noexcept
			#if HALF_EMULATED
			: data_{ val ? 0x3c00_u16 : 0_u16 }
			#else
			: data_{ static_cast<data_type>(val ? 1.0f : 0.0f) }
			#endif
			
		{ }

		#if HALF_EMULATED
			#define HALF_EXPLICIT_CONSTRUCTOR(type)							\
				MUU_NODISCARD_CTOR											\
				explicit constexpr half(type val) noexcept					\
					: data_{ impl::f32_to_f16(static_cast<float>(val)) }	\
				{}
		#else
			#define HALF_EXPLICIT_CONSTRUCTOR(type)							\
				MUU_NODISCARD_CTOR											\
				explicit constexpr half(type val) noexcept					\
					: data_{ static_cast<data_type>(val) }					\
				{}
		#endif

		HALF_EXPLICIT_CONSTRUCTOR(float)
		HALF_EXPLICIT_CONSTRUCTOR(double)
		HALF_EXPLICIT_CONSTRUCTOR(long double)
		#if MUU_HAS_FLOAT128
		HALF_EXPLICIT_CONSTRUCTOR(quad)
		#endif
		HALF_EXPLICIT_CONSTRUCTOR(char)
		HALF_EXPLICIT_CONSTRUCTOR(signed char)
		HALF_EXPLICIT_CONSTRUCTOR(unsigned char)
		HALF_EXPLICIT_CONSTRUCTOR(signed short)
		HALF_EXPLICIT_CONSTRUCTOR(unsigned short)
		HALF_EXPLICIT_CONSTRUCTOR(signed int)
		HALF_EXPLICIT_CONSTRUCTOR(unsigned int)
		HALF_EXPLICIT_CONSTRUCTOR(signed long)
		HALF_EXPLICIT_CONSTRUCTOR(unsigned long)
		HALF_EXPLICIT_CONSTRUCTOR(signed long long)
		HALF_EXPLICIT_CONSTRUCTOR(unsigned long long)
		#if MUU_HAS_INT128
		HALF_EXPLICIT_CONSTRUCTOR(int128_t)
		HALF_EXPLICIT_CONSTRUCTOR(uint128_t)
		#endif
		#undef HALF_EXPLICIT_CONSTRUCTOR

		#if MUU_HAS_FP16
		/*explicit*/ constexpr half(__fp16 val) noexcept
			: data_{ static_cast<data_type>(val) }
		{
			static_assert(!std::is_same_v<data_type, uint16_t>);
		}
		#endif

		#if MUU_HAS_FLOAT16
		explicit constexpr half(_Float16 val) noexcept
			: data_{ static_cast<data_type>(val) }
		{
		
			static_assert(!std::is_same_v<data_type, uint16_t>);
		}
		#endif

		//====================================================
		// CONVERSIONS
		//====================================================

		[[nodiscard]]
		MUU_ATTR(pure)
		MUU_ALWAYS_INLINE
		explicit constexpr operator bool() const noexcept
		{
			#if HALF_EMULATED
				return (data_ & 0x7FFF) != 0u; // !(anything but sign bit)
			#else
				return static_cast<bool>(data_);
			#endif
		}

		#if MUU_HAS_FP16
		[[nodiscard]]
		MUU_ATTR(pure)
		MUU_ALWAYS_INLINE
		explicit constexpr operator __fp16() const noexcept
		{
			return static_cast<__fp16>(data_);
		}
		#endif

		#if MUU_HAS_FLOAT16
		[[nodiscard]]
		MUU_ATTR(pure)
		MUU_ALWAYS_INLINE
		constexpr operator _Float16() const noexcept
		{
			return static_cast<_Float16>(data_);
		}
		#endif

		[[nodiscard]]
		MUU_ATTR(pure)
		MUU_ALWAYS_INLINE
		constexpr operator float() const noexcept
		{
			#if HALF_EMULATED
				return impl::f16_to_f32(data_);
			#else
				return static_cast<float>(data_);
			#endif
		}

		#define HALF_CAST_CONVERSION(type, explicit)					\
			[[nodiscard]]												\
			explicit constexpr operator type() const noexcept			\
			{															\
				return static_cast<type>(static_cast<float>(*this));	\
			}

		HALF_CAST_CONVERSION(double,						)
		HALF_CAST_CONVERSION(long double,					)
		#if MUU_HAS_FLOAT128
		HALF_CAST_CONVERSION(quad,							)
		#endif
		HALF_CAST_CONVERSION(char,					explicit)
		HALF_CAST_CONVERSION(signed char,			explicit)
		HALF_CAST_CONVERSION(unsigned char,			explicit)
		HALF_CAST_CONVERSION(signed short,			explicit)
		HALF_CAST_CONVERSION(unsigned short,		explicit)
		HALF_CAST_CONVERSION(signed int,			explicit)
		HALF_CAST_CONVERSION(unsigned int,			explicit)
		HALF_CAST_CONVERSION(signed long,			explicit)
		HALF_CAST_CONVERSION(unsigned long,			explicit)
		HALF_CAST_CONVERSION(signed long long,		explicit)
		HALF_CAST_CONVERSION(unsigned long long,	explicit)
		#if MUU_HAS_INT128
		HALF_CAST_CONVERSION(int128_t,				explicit)
		HALF_CAST_CONVERSION(uint128_t,				explicit)
		#endif

		#undef HALF_CAST_CONVERSION

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

		#define HALF_PROMOTING_BINARY_OP(return_type, input_type, op)									\
			[[nodiscard]]																				\
			MUU_ATTR(const)																				\
			friend constexpr return_type MUU_VECTORCALL operator op (half lhs, input_type rhs) noexcept	\
			{																							\
				return static_cast<input_type>(lhs) op rhs;												\
			}																							\
			[[nodiscard]]																				\
			MUU_ATTR(const)																				\
			friend constexpr return_type MUU_VECTORCALL operator op(input_type lhs, half rhs) noexcept	\
			{																							\
				return lhs op static_cast<input_type>(rhs);												\
			}

		#define HALF_CONVERTING_BINARY_OP(return_type, input_type, op)									\
			[[nodiscard]]																				\
			MUU_ATTR(const)																				\
			friend constexpr return_type MUU_VECTORCALL operator op (half lhs, input_type rhs) noexcept	\
			{																							\
				return static_cast<return_type>( static_cast<float>(lhs) op static_cast<float>(rhs) );	\
			}																							\
			[[nodiscard]]																				\
			MUU_ATTR(const)																				\
			friend constexpr return_type MUU_VECTORCALL operator op (input_type lhs, half rhs) noexcept	\
			{																							\
				return static_cast<return_type>( static_cast<float>(lhs) op static_cast<float>(rhs) );	\
			}

		#define HALF_BINARY_OPS(func, input_type)	\
			func(bool, input_type, ==)				\
			func(bool, input_type, !=)				\
			func(bool, input_type, < )				\
			func(bool, input_type, <=)				\
			func(bool, input_type, > )				\
			func(bool, input_type, >=)

		#if MUU_HAS_FP16
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, __fp16)
		#endif
		#if MUU_HAS_FLOAT16
		HALF_BINARY_OPS(HALF_PROMOTING_BINARY_OP, _Float16)
		#endif
		HALF_BINARY_OPS(HALF_PROMOTING_BINARY_OP,  float)
		HALF_BINARY_OPS(HALF_PROMOTING_BINARY_OP,  double)
		HALF_BINARY_OPS(HALF_PROMOTING_BINARY_OP,  long double)
		#if MUU_HAS_FLOAT128
		HALF_BINARY_OPS(HALF_PROMOTING_BINARY_OP,  quad)
		#endif
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, char)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, signed char)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, unsigned char)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, signed short)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, unsigned short)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, signed int)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, unsigned int)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, signed long)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, unsigned long)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, signed long long)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, unsigned long long)
		#if MUU_HAS_INT128
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, int128_t)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP, uint128_t)
		#endif
		#undef HALF_BINARY_OPS

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

		#define HALF_BINARY_OPS(func, return_type, input_type)	\
			func(return_type, input_type, +)					\
			func(return_type, input_type, -)					\
			func(return_type, input_type, *)					\
			func(return_type, input_type, /)

		#if MUU_HAS_FP16
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP,	half,			__fp16)
		#endif
		#if MUU_HAS_FLOAT16
		HALF_BINARY_OPS(HALF_PROMOTING_BINARY_OP,	_Float16,		_Float16)
		#endif
		HALF_BINARY_OPS(HALF_PROMOTING_BINARY_OP,	float,			float)
		HALF_BINARY_OPS(HALF_PROMOTING_BINARY_OP,	double,			double)
		HALF_BINARY_OPS(HALF_PROMOTING_BINARY_OP,	long double,	long double)
		#if MUU_HAS_FLOAT128
		HALF_BINARY_OPS(HALF_PROMOTING_BINARY_OP,	quad,			quad)
		#endif
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP,	half,			char)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP,	half,			signed char)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP,	half,			unsigned char)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP,	half,			signed short)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP,	half,			unsigned short)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP,	half,			signed int)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP,	half,			unsigned int)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP,	half,			signed long)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP,	half,			unsigned long)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP,	half,			signed long long)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP,	half,			unsigned long long)
		#if MUU_HAS_INT128
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP,	half,			int128_t)
		HALF_BINARY_OPS(HALF_CONVERTING_BINARY_OP,	half,			uint128_t)
		#endif
		#undef HALF_BINARY_OPS
		#undef HALF_PROMOTING_BINARY_OP
		#undef HALF_CONVERTING_BINARY_OP

		//====================================================
		// ARITHMETIC ASSIGNMENTS
		//====================================================

		friend constexpr half& operator += (half& lhs, half rhs) noexcept
		{
			#if HALF_EMULATED
				lhs.data_ = impl::f32_to_f16(static_cast<float>(lhs) + static_cast<float>(rhs));
			#else
				lhs.data_ += rhs.data_;
			#endif
			return lhs;
		}

		friend constexpr half& operator -= (half& lhs, half rhs) noexcept
		{
			#if HALF_EMULATED
				lhs.data_ = impl::f32_to_f16(static_cast<float>(lhs) - static_cast<float>(rhs));
			#else
				lhs.data_ -= rhs.data_;
			#endif
			return lhs;
		}

		friend constexpr half& operator *= (half& lhs, half rhs) noexcept
		{
			#if HALF_EMULATED
				lhs.data_ = impl::f32_to_f16(static_cast<float>(lhs) * static_cast<float>(rhs));
			#else
				lhs.data_ *= rhs.data_;
			#endif
			return lhs;
		}

		friend constexpr half& operator /= (half& lhs, half rhs) noexcept
		{
			#if HALF_EMULATED
				lhs.data_ = impl::f32_to_f16(static_cast<float>(lhs) / static_cast<float>(rhs));
			#else
				lhs.data_ /= rhs.data_;
			#endif
			return lhs;
		}

		#if HALF_EMULATED
			#define HALF_DEMOTING_ASSIGN_OP(input_type, op)									\
				friend constexpr half& operator op##= (half& lhs, input_type rhs) noexcept	\
				{																			\
					lhs.data_ = impl::f32_to_f16(static_cast<float>(lhs op rhs));			\
					return lhs;																\
				}

			#define HALF_CONVERTING_ASSIGN_OP(input_type, op)								\
				friend constexpr half& operator op##= (half& lhs, input_type rhs) noexcept	\
				{																			\
					lhs.data_ = impl::f32_to_f16(static_cast<float>(lhs) op rhs);			\
					return lhs;																\
				}

			#define HALF_CASTING_ASSIGN_OP(input_type, op)									\
				friend constexpr half& operator op##= (half& lhs, input_type rhs) noexcept	\
				{																			\
					lhs.data_ = bit_cast<data_type>(lhs op rhs);							\
					return lhs;																\
				}
		#else
			#define HALF_DEMOTING_ASSIGN_OP(input_type, op)									\
				friend constexpr half& operator op##= (half& lhs, input_type rhs) noexcept	\
				{																			\
					lhs.data_ = static_cast<data_type>(lhs op rhs);							\
					return lhs;																\
				}

			#define HALF_CONVERTING_ASSIGN_OP(input_type, op)								\
				friend constexpr half& operator op##= (half& lhs, input_type rhs) noexcept	\
				{																			\
					lhs.data_ = static_cast<data_type>(static_cast<float>(lhs) op rhs);		\
					return lhs;																\
				}

			#define HALF_CASTING_ASSIGN_OP(input_type, op)									\
				friend constexpr half& operator op##= (half& lhs, input_type rhs) noexcept	\
				{																			\
					lhs.data_ = static_cast<data_type>(lhs op rhs);							\
					return lhs;																\
				}
		#endif

		#define HALF_BINARY_OPS(func, input_type)	\
			func(input_type, +)						\
			func(input_type, -)						\
			func(input_type, *)						\
			func(input_type, /)

		#if MUU_HAS_FP16
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP,	__fp16)
		#endif
		#if MUU_HAS_FLOAT16
		HALF_BINARY_OPS(HALF_CASTING_ASSIGN_OP,		_Float16)
		#endif
		HALF_BINARY_OPS(HALF_DEMOTING_ASSIGN_OP,	float)
		HALF_BINARY_OPS(HALF_DEMOTING_ASSIGN_OP,	double)
		HALF_BINARY_OPS(HALF_DEMOTING_ASSIGN_OP,	long double)
		#if MUU_HAS_FLOAT128
		HALF_BINARY_OPS(HALF_DEMOTING_ASSIGN_OP,	quad)
		#endif
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP,	char)
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP,	signed char)
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP,	unsigned char)
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP,	signed short)
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP,	unsigned short)
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP,	signed int)
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP,	unsigned int)
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP,	signed long)
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP,	unsigned long)
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP,	signed long long)
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP,	unsigned long long)
		#if MUU_HAS_INT128
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP,	int128_t)
		HALF_BINARY_OPS(HALF_CONVERTING_ASSIGN_OP,	uint128_t)
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
			#if HALF_EMULATED
				data_ = impl::f32_to_f16(static_cast<float>(*this) + 1.0f);
			#else
				data_++;
			#endif
			return *this;
		}

		constexpr half& operator--() noexcept
		{
			#if HALF_EMULATED
				data_ = impl::f32_to_f16(static_cast<float>(*this) - 1.0f);
			#else
				data_--;
			#endif
			return *this;
		}

		constexpr half operator++(int) noexcept
		{
			half prev = *this;
			#if HALF_EMULATED
				data_ = impl::f32_to_f16(static_cast<float>(*this) + 1.0f);
			#else
				data_++;
			#endif
			return prev;
		}

		constexpr half operator--(int) noexcept
		{
			half prev = *this;
			#if HALF_EMULATED
				data_ = impl::f32_to_f16(static_cast<float>(*this) - 1.0f);
			#else
				data_--;
			#endif
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
			#if HALF_EMULATED
				return from_bits(data_ ^ 0b1000000000000000_u16);
			#else
				return half{ -static_cast<float>(data_) };
			#endif
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


		/// \brief	Returns true if the value of the half is positive/negative infinity or Not-A-Number.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool infinity_or_nan() const noexcept
		{
			#if HALF_EMULATED
				return (0b0111110000000000_u16 & data_) == 0b0111110000000000_u16;
			#else
				return muu::infinity_or_nan(data_);
			#endif
		}

		/// \brief	Returns true if the value of the half is positive or negative infinity.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool infinity() const noexcept
		{
			#if HALF_EMULATED
				return (0b0111110000000000_u16 & data_) == 0b0111110000000000_u16
					&& (0b0000001111111111_u16 & data_) == 0_u16;
			#else
				return infinity_or_nan() && data_ == data_;
			#endif
		}

		/// \brief	Returns true if the value of the half is Not-A-Number.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool nan() const noexcept
		{
			#if HALF_EMULATED
				return (0b0111110000000000_u16 & data_) == 0b0111110000000000_u16
					&& (0b0000001111111111_u16 & data_) != 0_u16;
			#else
				return infinity_or_nan() && data_ != data_;
			#endif
		}
	};

	#ifndef DOXYGEN
	namespace impl
	{
		#if HALF_EMULATED

		template <>
		struct integer_limits<half>
		{
			static constexpr half lowest		= half::from_bits(0b1'11110'1111111111_u16);
			static constexpr half highest		= half::from_bits(0b0'11110'1111111111_u16);
		};

		template <>
		struct integer_positive_constants<half>
		{
			static constexpr half zero			= half::from_bits(0b0'00000'0000000000_u16);
			static constexpr half one			= half::from_bits(0b0'01111'0000000000_u16);
			static constexpr half two			= half::from_bits(0b0'10000'0000000000_u16);
			static constexpr half three			= half::from_bits(0b0'10000'1000000000_u16);
			static constexpr half four			= half::from_bits(0b0'10001'0000000000_u16);
			static constexpr half five			= half::from_bits(0b0'10001'0100000000_u16);
			static constexpr half six			= half::from_bits(0b0'10001'1000000000_u16);
			static constexpr half seven			= half::from_bits(0b0'10001'1100000000_u16);
			static constexpr half eight			= half::from_bits(0b0'10010'0000000000_u16);
			static constexpr half nine			= half::from_bits(0b0'10010'0010000000_u16);
			static constexpr half ten			= half::from_bits(0b0'10010'0100000000_u16);
		};

		template <>
		struct integer_negative_constants<half>
		{
			static constexpr half minus_one		= half::from_bits(0b1'01111'0000000000_u16);
			static constexpr half minus_two		= half::from_bits(0b1'10000'0000000000_u16);
			static constexpr half minus_three	= half::from_bits(0b1'10000'1000000000_u16);
			static constexpr half minus_four	= half::from_bits(0b1'10001'0000000000_u16);
			static constexpr half minus_five	= half::from_bits(0b1'10001'0100000000_u16);
			static constexpr half minus_six		= half::from_bits(0b1'10001'1000000000_u16);
			static constexpr half minus_seven	= half::from_bits(0b1'10001'1100000000_u16);
			static constexpr half minus_eight	= half::from_bits(0b1'10010'0000000000_u16);
			static constexpr half minus_nine	= half::from_bits(0b1'10010'0010000000_u16);
			static constexpr half minus_ten		= half::from_bits(0b1'10010'0100000000_u16);
		};

		template <>
		struct floating_point_special_constants<half>
		{
			static constexpr half nan				= half::from_bits(0b1'11111'1000000001_u16);
			static constexpr half snan				= half::from_bits(0b1'11111'0000000001_u16);
			static constexpr half infinity			= half::from_bits(0b0'11111'0000000000_u16);
			static constexpr half negative_infinity	= half::from_bits(0b1'11111'0000000000_u16);
			static constexpr half minus_zero		= half::from_bits(0b1'00000'0000000000_u16);
		};

		template <>
		struct floating_point_named_constants<half>
		{
			static constexpr half one_over_two          = half::from_bits( 0b0'01110'0000000000_u16 );
			static constexpr half two_over_three        = half::from_bits( 0b0'01110'0101010110_u16 );
			static constexpr half two_over_five         = half::from_bits( 0b0'01101'1001100111_u16 );
			static constexpr half sqrt_two              = half::from_bits( 0b0'01111'0110101000_u16 );
			static constexpr half one_over_sqrt_two     = half::from_bits( 0b0'01110'0110101000_u16 );
			static constexpr half one_over_three        = half::from_bits( 0b0'01101'0101010110_u16 );
			static constexpr half three_over_two        = half::from_bits( 0b0'01111'1000000000_u16 );
			static constexpr half three_over_four       = half::from_bits( 0b0'01110'1000000000_u16 );
			static constexpr half three_over_five       = half::from_bits( 0b0'01110'0011001101_u16 );
			static constexpr half sqrt_three            = half::from_bits( 0b0'01111'1011101110_u16 );
			static constexpr half one_over_sqrt_three   = half::from_bits( 0b0'01110'0010011111_u16 );
			static constexpr half pi                    = half::from_bits( 0b0'10000'1001001001_u16 );
			static constexpr half one_over_pi           = half::from_bits( 0b0'01101'0100011000_u16 );
			static constexpr half pi_over_two           = half::from_bits( 0b0'01111'1001001001_u16 );
			static constexpr half pi_over_three         = half::from_bits( 0b0'01111'0000110001_u16 );
			static constexpr half pi_over_four          = half::from_bits( 0b0'01110'1001001001_u16 );
			static constexpr half pi_over_five          = half::from_bits( 0b0'01110'0100000111_u16 );
			static constexpr half pi_over_six           = half::from_bits( 0b0'01110'0000110001_u16 );
			static constexpr half sqrt_pi               = half::from_bits( 0b0'01111'1100010111_u16 );
			static constexpr half one_over_sqrt_pi      = half::from_bits( 0b0'01110'0010000100_u16 );
			static constexpr half two_pi                = half::from_bits( 0b0'10001'1001001001_u16 );
			static constexpr half sqrt_two_pi           = half::from_bits( 0b0'10000'0100000100_u16 );
			static constexpr half one_over_sqrt_two_pi  = half::from_bits( 0b0'01101'1001100010_u16 );
			static constexpr half e                     = half::from_bits( 0b0'10000'0101110000_u16 );
			static constexpr half one_over_e            = half::from_bits( 0b0'01101'0111100011_u16 );
			static constexpr half e_over_two            = half::from_bits( 0b0'01111'0101110000_u16 );
			static constexpr half e_over_three          = half::from_bits( 0b0'01110'1101000000_u16 );
			static constexpr half e_over_four           = half::from_bits( 0b0'01110'0101110000_u16 );
			static constexpr half e_over_five           = half::from_bits( 0b0'01110'0001011010_u16 );
			static constexpr half e_over_six            = half::from_bits( 0b0'01101'1101000000_u16 );
			static constexpr half sqrt_e                = half::from_bits( 0b0'01111'1010011001_u16 );
			static constexpr half one_over_sqrt_e       = half::from_bits( 0b0'01110'0011011010_u16 );
			static constexpr half phi                   = half::from_bits( 0b0'01111'1001111001_u16 );
			static constexpr half one_over_phi          = half::from_bits( 0b0'01110'0011110010_u16 );
			static constexpr half phi_over_two          = half::from_bits( 0b0'01110'1001111001_u16 );
			static constexpr half phi_over_three        = half::from_bits( 0b0'01110'0001010001_u16 );
			static constexpr half phi_over_four         = half::from_bits( 0b0'01101'1001111001_u16 );
			static constexpr half phi_over_five         = half::from_bits( 0b0'01101'0100101110_u16 );
			static constexpr half phi_over_six          = half::from_bits( 0b0'01101'0001010001_u16 );
			static constexpr half sqrt_phi              = half::from_bits( 0b0'01111'0100010111_u16 );
			static constexpr half one_over_sqrt_phi     = half::from_bits( 0b0'01110'1001001010_u16 );
		};

		#endif // HALF_EMULATED

		template <>
		struct floating_point_limits<half>
		{
			static constexpr int significand_digits = 11;
			
			#if HALF_EMULATED
			static constexpr half approx_equal_epsilon = half::from_bits(0b0'00101'0000011001_u16); // 0.001
			#else
			static constexpr half approx_equal_epsilon = half{ floating_point_limits<HALF_IMPL_TYPE>::approx_equal_epsilon };
			#endif
		};
	}
	#endif // !DOXYGEN

	/// \brief	16-bit half-precision float constants.
	/// \ingroup		constants
	template <> struct constants<half> : impl::floating_point_constants<half> {};

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

	/// \addtogroup 	intrinsics
	/// @{

	/// \brief	Returns a linear interpolation between two halfs.
	/// \ingroup	lerp
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr half MUU_VECTORCALL lerp(half start, half finish, half alpha) noexcept
	{
		return static_cast<half>(lerp(static_cast<float>(start), static_cast<float>(finish), static_cast<float>(alpha)));
	}

	/// \brief	Returns true if two halfs are approximately equal.
	/// \ingroup	approx_equal
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool MUU_VECTORCALL approx_equal(half lhs, half rhs, half epsilon = constants<half>::approx_equal_epsilon) noexcept
	{
		return abs(rhs - lhs) < epsilon;
	}

	/// @}
}
MUU_NAMESPACE_END

#if !defined(DOXYGEN) && HALF_EMULATED
MUU_IMPL_NAMESPACE_START
{
	#if HALF_USE_INTRINSICS

	MUU_PUSH_WARNINGS
	MUU_PRAGMA_MSVC(warning(disable: 4556)) // value of intrinsic immediate argument '8' is out of range '0 - 7'
	MUU_PRAGMA_GCC("GCC diagnostic ignored \"-Wold-style-cast\"") // false positive with _mm_load_ss

	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(const)
	uint16_t MUU_VECTORCALL f32_to_f16_intrinsic(float val) noexcept
	{
		//_mm_load_ss			load a single float into a m128 (sse)
		//_mm_cvtps_ph			convert floats in a m128 to half-precision floats in an m128i (FP16C)
		//_mm_cvtsi128_si32		returns the first int from an m128i (sse2)

		return static_cast<uint16_t>(_mm_cvtsi128_si32(
			_mm_cvtps_ph(_mm_load_ss(&val), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)
		));
	}

	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(const)
	float MUU_VECTORCALL f16_to_f32_intrinsic(uint16_t val) noexcept
	{
		//_mm_cvtsi32_si128		store a single int in an m128i (sse2)
		//_mm_cvtph_ps			convert half-precision floats in a m128i to floats in an m128 (FP16C)
		//_mm_cvtss_f32			returns the first float from an m128 (sse)

		return _mm_cvtss_f32(_mm_cvtph_ps(
			_mm_cvtsi32_si128(static_cast<int>(val))
		));
	}

	MUU_POP_WARNINGS

	#endif //HALF_USE_INTRINSICS

	inline constexpr int8_t f16_single_exp_bias = 127;
	inline constexpr int8_t f16_half_exp_bias = 15;

	[[nodiscard]]
	MUU_ATTR(const)
	constexpr uint16_t MUU_VECTORCALL f32_to_f16_emulated(float val) noexcept
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
	constexpr float MUU_VECTORCALL f16_to_f32_emulated(uint16_t val) noexcept
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
		#if HALF_USE_INTRINSICS
		if constexpr (build::supports_is_constant_evaluated)
		{
			if (!is_constant_evaluated())
				return f32_to_f16_intrinsic(val);
		}
		#endif

		return f32_to_f16_emulated(val);
	}

	[[nodiscard]]
	MUU_ATTR(const)
	MUU_ATTR(flatten)
	constexpr float MUU_VECTORCALL f16_to_f32(uint16_t val) noexcept
	{
		#if HALF_USE_INTRINSICS
		if constexpr (build::supports_is_constant_evaluated)
		{
			if (!is_constant_evaluated())
				return f16_to_f32_intrinsic(val);
		}
		#endif

		return f16_to_f32_emulated(val);
	}
}
MUU_IMPL_NAMESPACE_END
#endif // !defined(DOXYGEN) && HALF_EMULATED

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
		static MUU_CONSTEVAL half(min)() noexcept
		{
			#if HALF_EMULATED
				using namespace muu::literals;
				return half::from_bits(0x0400_u16); // 0.000061035 (ish)
			#else
				return half{ (std::numeric_limits<HALF_IMPL_TYPE>::min)() };
			#endif
		}

		[[nodiscard]]
		MUU_ATTR(const)
		static MUU_CONSTEVAL half lowest() noexcept
		{
			return half::constants::lowest;
		}

		[[nodiscard]]
		MUU_ATTR(const)
		static MUU_CONSTEVAL half(max)() noexcept
		{
			return half::constants::highest;
		}

		[[nodiscard]]
		MUU_ATTR(const)
		static MUU_CONSTEVAL half epsilon() noexcept
		{
			#if HALF_EMULATED
				using namespace muu::literals;
				return half::from_bits(0b0'00101'0000000000_u16); // 0.00097656
			#else
				return half{ std::numeric_limits<HALF_IMPL_TYPE>::epsilon() };
			#endif
		}

		[[nodiscard]]
		MUU_ATTR(const)
		static MUU_CONSTEVAL half round_error() noexcept
		{
			#if HALF_EMULATED
				using namespace muu::literals;
				return half::from_bits(0b0'00100'0000000000_u16); // epsilon / 2
			#else
				return half{ std::numeric_limits<HALF_IMPL_TYPE>::round_error() };
			#endif
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
			#if HALF_EMULATED
				using namespace muu::literals;
				return half::from_bits(0x0001_u16); // 0.000000059605 (ish)
			#else
				return half{ std::numeric_limits<HALF_IMPL_TYPE>::denorm_min() };
			#endif
		}
	};

	template <> struct numeric_limits<const muu::half>			: numeric_limits<muu::half> {};
	template <> struct numeric_limits<volatile muu::half>		: numeric_limits<muu::half> {};
	template <> struct numeric_limits<const volatile muu::half>	: numeric_limits<muu::half> {};
}

MUU_POP_WARNINGS // MUU_DISABLE_ARITHMETIC_WARNINGS, MUU_DISABLE_SPAM_WARNINGS

#undef HALF_IMPL_TYPE
#undef HALF_EMULATED
#undef HALF_USE_INTRINSICS
