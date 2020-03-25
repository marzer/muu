// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

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
	[[nodiscard]] constexpr uint16_t f32_to_f16(float) noexcept;
	[[nodiscard]] constexpr float f16_to_f32(uint16_t) noexcept;
	struct f16_from_bits_tag {};
}

namespace muu
{
	/// \brief	A 16-bit "half-precision" floating point type.
	/// 
	/// \see [Half-precision floating-point (wikipedia)](https://en.wikipedia.org/wiki/Half-precision_floating-point_format)
	struct MUU_TRIVIAL_ABI float16 final
	{
		/// \brief	The raw bits of the float.
		uint16_t bits;

		//====================================================
		// CONSTRUCTORS
		//====================================================

		/// \brief	Default 'uninitialized' constructor.
		float16() noexcept = default;

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

		/// \brief	Constructs a half-precision float from a single-precision float.
		explicit constexpr float16(float val) noexcept
			: bits{ impl::f32_to_f16(val) }
		{}

		/// \brief	Constructs a half-precision float from a double-precision float.
		explicit constexpr float16(double val) noexcept
			: float16{ static_cast<float>(val) }
		{ }

		/// \brief	Constructs a half-precision float from a long double.
		explicit constexpr float16(long double val) noexcept
			: float16{ static_cast<float>(val) }
		{ }

		/// \brief	Constructs a half-precision float from a boolean.
		explicit constexpr float16(bool val) noexcept
			: bits{ val ? 0x3c00_u16 : 0_u16 }
		{ }

		/// \brief	Constructs a half-precision float from an 8-bit integer.
		explicit constexpr float16(int8_t val) noexcept
			: float16{ static_cast<float>(val) }
		{ }

		/// \brief	Constructs a half-precision float from an 16-bit integer.
		explicit constexpr float16(int16_t val) noexcept
			: float16{ static_cast<float>(val) }
		{ }

		/// \brief	Constructs a half-precision float from an 32-bit integer.
		explicit constexpr float16(int32_t val) noexcept
			: float16{ static_cast<float>(val) }
		{ }

		/// \brief	Constructs a half-precision float from an 64-bit integer.
		explicit constexpr float16(int64_t val) noexcept
			: float16{ static_cast<float>(val) }
		{ }

		/// \brief	Constructs a half-precision float from an unsigned 8-bit integer.
		explicit constexpr float16(uint8_t val) noexcept
			: float16{ static_cast<float>(val) }
		{ }

		/// \brief	Constructs a half-precision float from an unsigned 16-bit integer.
		explicit constexpr float16(uint16_t val) noexcept
			: float16{ static_cast<float>(val) }
		{ }

		/// \brief	Constructs a half-precision float from an unsigned 32-bit integer.
		explicit constexpr float16(uint32_t val) noexcept
			: float16{ static_cast<float>(val) }
		{ }

		/// \brief	Constructs a half-precision float from an unsigned 64-bit integer.
		explicit constexpr float16(uint64_t val) noexcept
			: float16{ static_cast<float>(val) }
		{ }

		//====================================================
		// CONVERSIONS
		//====================================================

		/// \brief	Creates a single-precision float by expanding a half-precision float.
		[[nodiscard]] MUU_ALWAYS_INLINE
		/*explicit*/ constexpr operator float() const noexcept
		{
			return impl::f16_to_f32(bits);
		}

		/// \brief	Creates a double-precision float by expanding a half-precision float.
		[[nodiscard]] MUU_ALWAYS_INLINE
		/*explicit*/ constexpr operator double() const noexcept
		{
			return static_cast<double>(impl::f16_to_f32(bits));
		}

		/// \brief	Creates a long double-precision float by expanding a half-precision float.
		[[nodiscard]] MUU_ALWAYS_INLINE
		/*explicit*/ constexpr operator long double() const noexcept
		{
			return static_cast<long double>(impl::f16_to_f32(bits));
		}

		/// \brief	Converts a half-precision float to a boolean.
		[[nodiscard]] MUU_ALWAYS_INLINE
		explicit constexpr operator bool() const noexcept
		{
			return (bits & 0x7FFFu) != 0u;
		}

		/// \brief	Creates an 8-bit integer from a half-precision float.
		[[nodiscard]] MUU_ALWAYS_INLINE
		explicit constexpr operator int8_t() const noexcept
		{
			return static_cast<int8_t>(impl::f16_to_f32(bits));
		}

		/// \brief	Creates a 16-bit integer from a half-precision float.
		[[nodiscard]] MUU_ALWAYS_INLINE
		explicit constexpr operator int16_t() const noexcept
		{
			return static_cast<int16_t>(impl::f16_to_f32(bits));
		}

		/// \brief	Creates a 32-bit integer from a half-precision float.
		[[nodiscard]] MUU_ALWAYS_INLINE
		explicit constexpr operator int32_t() const noexcept
		{
			return static_cast<int32_t>(impl::f16_to_f32(bits));
		}

		/// \brief	Creates a 64-bit integer from a half-precision float.
		[[nodiscard]] MUU_ALWAYS_INLINE
		explicit constexpr operator int64_t() const noexcept
		{
			return static_cast<int64_t>(impl::f16_to_f32(bits));
		}

		/// \brief	Creates an unsigned 8-bit integer from a half-precision float.
		[[nodiscard]] MUU_ALWAYS_INLINE
		explicit constexpr operator uint8_t() const noexcept
		{
			return static_cast<uint8_t>(impl::f16_to_f32(bits));
		}

		/// \brief	Creates an unsigned 16-bit integer from a half-precision float.
		[[nodiscard]] MUU_ALWAYS_INLINE
		explicit constexpr operator uint16_t() const noexcept
		{
			return static_cast<uint16_t>(impl::f16_to_f32(bits));
		}

		/// \brief	Creates an unsigned 32-bit integer from a half-precision float.
		[[nodiscard]] MUU_ALWAYS_INLINE
		explicit constexpr operator uint32_t() const noexcept
		{
			return static_cast<uint32_t>(impl::f16_to_f32(bits));
		}

		/// \brief	Creates an unsigned 64-bit integer from a half-precision float.
		[[nodiscard]] MUU_ALWAYS_INLINE
		explicit constexpr operator uint64_t() const noexcept
		{
			return static_cast<uint64_t>(impl::f16_to_f32(bits));
		}

		//====================================================
		// INFINITY AND NAN
		//====================================================

		/// \brief	Returns true if the value of a half-precision float is infinity or NaN.
		[[nodiscard]] MUU_ALWAYS_INLINE
		constexpr bool is_inf_or_nan() const noexcept
		{
			return (0b0111110000000000_u16 & bits) == 0b0111110000000000_u16;
		}

		/// \brief	Returns true if the value of a half-precision float is NaN.
		[[nodiscard]] MUU_ALWAYS_INLINE
		constexpr bool is_nan() const noexcept
		{
			return (0b0111110000000000_u16 & bits) == 0b0111110000000000_u16
				&& (0b0000001111111111_u16 & bits) != 0_u16;
		}

		//====================================================
		// COMPARISONS
		//====================================================

		/// \brief	Equality operator.
		[[nodiscard]] MUU_ALWAYS_INLINE
		friend constexpr bool operator == (float16 lhs, float16 rhs) noexcept
		{
			return static_cast<float>(lhs) == static_cast<float>(rhs);
		}

		/// \brief	Inequality operator.
		[[nodiscard]] MUU_ALWAYS_INLINE
		friend constexpr bool operator != (float16 lhs, float16 rhs) noexcept
		{
			return static_cast<float>(lhs) != static_cast<float>(rhs);
		}

		/// \brief	Less-than operator.
		[[nodiscard]] MUU_ALWAYS_INLINE
		friend constexpr bool operator < (float16 lhs, float16 rhs) noexcept
		{
			return static_cast<float>(lhs) < static_cast<float>(rhs);
		}

		/// \brief	Less-than-or-equal-to operator.
		[[nodiscard]] MUU_ALWAYS_INLINE
		friend constexpr bool operator <= (float16 lhs, float16 rhs) noexcept
		{
			return static_cast<float>(lhs) <= static_cast<float>(rhs);
		}

		/// \brief	Greater-than operator.
		[[nodiscard]] MUU_ALWAYS_INLINE
		friend constexpr bool operator > (float16 lhs, float16 rhs) noexcept
		{
			return static_cast<float>(lhs) > static_cast<float>(rhs);
		}

		/// \brief	Greater-than-or-equal-to operator.
		[[nodiscard]] MUU_ALWAYS_INLINE
		friend constexpr bool operator >= (float16 lhs, float16 rhs) noexcept
		{
			return static_cast<float>(lhs) >= static_cast<float>(rhs);
		}

		#if 1 // DEMO_INTELLISENSE_BUG

		#define MUU_F16_PROMOTING_BINARY_OP(input_type, return_type, op)

		#else

		#define MUU_F16_PROMOTING_BINARY_OP(input_type, return_type, op)					\
			[[nodiscard]] MUU_ALWAYS_INLINE													\
			friend constexpr return_type operator op (float16 lhs, input_type rhs) noexcept	\
			{																				\
				return static_cast<input_type>(lhs) op rhs;									\
			}																				\
			friend constexpr return_type operator op (input_type rhs, float16 lhs) noexcept	\
			{																				\
				return lhs op static_cast<input_type>(rhs);									\
			}

		#endif // DEMO_INTELLISENSE_BUG

		MUU_F16_PROMOTING_BINARY_OP(float, bool, ==)
		MUU_F16_PROMOTING_BINARY_OP(float, bool, !=)
		MUU_F16_PROMOTING_BINARY_OP(float, bool, < )
		MUU_F16_PROMOTING_BINARY_OP(float, bool, <=)
		MUU_F16_PROMOTING_BINARY_OP(float, bool, > )
		MUU_F16_PROMOTING_BINARY_OP(float, bool, >=)
		MUU_F16_PROMOTING_BINARY_OP(double, bool, ==)
		MUU_F16_PROMOTING_BINARY_OP(double, bool, !=)
		MUU_F16_PROMOTING_BINARY_OP(double, bool, < )
		MUU_F16_PROMOTING_BINARY_OP(double, bool, <=)
		MUU_F16_PROMOTING_BINARY_OP(double, bool, > )
		MUU_F16_PROMOTING_BINARY_OP(double, bool, >=)
		MUU_F16_PROMOTING_BINARY_OP(long double, bool, ==)
		MUU_F16_PROMOTING_BINARY_OP(long double, bool, !=)
		MUU_F16_PROMOTING_BINARY_OP(long double, bool, < )
		MUU_F16_PROMOTING_BINARY_OP(long double, bool, <=)
		MUU_F16_PROMOTING_BINARY_OP(long double, bool, > )
		MUU_F16_PROMOTING_BINARY_OP(long double, bool, >=)

		//====================================================
		// ARITHMETIC OPERATORS
		//====================================================

		/// \brief	Adds two half-precision floats.
		[[nodiscard]]
		friend constexpr float16 operator + (float16 lhs, float16 rhs) noexcept
		{
			return float16{ static_cast<float>(lhs) + static_cast<float>(rhs) };
		}

		/// \brief	Subtracts one half-precision float from another.
		[[nodiscard]]
		friend constexpr float16 operator - (float16 lhs, float16 rhs) noexcept
		{
			return float16{ static_cast<float>(lhs) - static_cast<float>(rhs) };
		}

		/// \brief	Multiplies two half-precision floats.
		[[nodiscard]]
		friend constexpr float16 operator * (float16 lhs, float16 rhs) noexcept
		{
			return float16{ static_cast<float>(lhs) * static_cast<float>(rhs) };
		}

		/// \brief	Divides one half-precision float by another.
		[[nodiscard]]
		friend constexpr float16 operator / (float16 lhs, float16 rhs) noexcept
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

		/// \brief	Performs a fused-multiply-add.
		/// 
		/// \details The FMA operation is performed on the inputs after promoting them to floats
		/// 		 so the intermediate precision loss is minimized.
		/// 
		/// \param 	m1	The first multiplication operand.
		/// \param 	m2	The second multiplication operand.
		/// \param 	a 	The addition operand.
		///
		/// \returns	The result of `(m1 * m2) + a`.
		[[nodiscard]]
		static constexpr float16 fma(float16 m1, float16 m2, float16 a) noexcept
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

		/// \brief	Addition-assignment operator.
		friend constexpr float16& operator += (float16& lhs, float16 rhs) noexcept
		{
			lhs.bits = impl::f32_to_f16(static_cast<float>(lhs) + static_cast<float>(rhs));
			return lhs;
		}

		/// \brief	Subtraction-assignment operator.
		friend constexpr float16& operator -= (float16& lhs, float16 rhs) noexcept
		{
			lhs.bits = impl::f32_to_f16(static_cast<float>(lhs) - static_cast<float>(rhs));
			return lhs;
		}

		/// \brief	Multiplication-assignment operator.
		friend constexpr float16& operator *= (float16& lhs, float16 rhs) noexcept
		{
			lhs.bits = impl::f32_to_f16(static_cast<float>(lhs) * static_cast<float>(rhs));
			return lhs;
		}

		/// \brief	Division-assignment operator.
		friend constexpr float16& operator /= (float16& lhs, float16 rhs) noexcept
		{
			lhs.bits = impl::f32_to_f16(static_cast<float>(lhs) / static_cast<float>(rhs));
			return lhs;
		}

		//====================================================
		// INCREMENTS AND DECREMENTS
		//====================================================

		/// \brief	Pre-increment operator.
		constexpr float16& operator++() noexcept
		{
			bits = impl::f32_to_f16(static_cast<float>(*this) + 1.0f);
			return *this;
		}

		/// \brief	Pre-decrement operator.
		constexpr float16& operator--() noexcept
		{
			bits = impl::f32_to_f16(static_cast<float>(*this) - 1.0f);
			return *this;
		}

		/// \brief	Post-increment operator.
		constexpr float16 operator++(int) noexcept
		{
			float16 prev = *this;
			bits = impl::f32_to_f16(static_cast<float>(*this) + 1.0f);
			return prev;
		}

		/// \brief	Post-decrement operator.
		constexpr float16 operator--(int) noexcept
		{
			float16 prev = *this;
			bits = impl::f32_to_f16(static_cast<float>(*this) - 1.0f);
			return prev;
		}

		//====================================================
		// MISC
		//====================================================

		/// \brief	Unary plus operator.
		[[nodiscard]]
		constexpr float16 operator + () const noexcept
		{
			return *this;
		}

		/// \brief	Negation operator.
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
}

namespace muu::impl
{
	#if MUU_F16_USE_INTRINSICS

	MUU_PUSH_WARNINGS
	MUU_PRAGMA_MSVC(warning(disable: 4556)) //value of intrinsic immediate argument '8' is out of range '0 - 7'

	[[nodiscard]] MUU_ALWAYS_INLINE
	uint16_t f32_to_f16_intrinsic(float val) noexcept
	{
		//_mm_set_ss			store a single float in a m128
		//_mm_cvtps_ph			convert floats in a m128 to half-precision floats in an m128i
		//_mm_cvtsi128_si32		returns the first int from an m128i

		return static_cast<::uint16_t>(_mm_cvtsi128_si32(
			_mm_cvtps_ph(_mm_set_ss(val), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)
		));
	}

	[[nodiscard]] MUU_ALWAYS_INLINE
	float f16_to_f32_intrinsic(uint16_t val) noexcept
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
	constexpr uint16_t f32_to_f16_native(float val) noexcept
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
		if (exp32 == 0xFF /*|| exp32ub > single_exp_bias*/) MUU_UNLIKELY
		{
			// 0001 1111, use all five bits
			exp16 = 0x1F;
		}
		else if (exp32 == 0 /*|| exp32ub < -(single_exp_bias - 1)*/) MUU_UNLIKELY
		{
			exp16 = 0;
		}

		uint16_t frac16 = static_cast<uint16_t>(frac32 >> 13);

		// nan
		if (exp32 == 0xFF && frac32 != 0 && frac16 == 0) MUU_UNLIKELY
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
		else if (exp16 == 0 && exp32 != 0) MUU_UNLIKELY
		{
			// 0000 0001 0000 0000
			frac16 = 0x0100 | (frac16 >> 2);
		}
		exp16 = static_cast<uint16_t>(exp16 << 10);
		return s16 | exp16 | frac16;
	}

	[[nodiscard]]
	constexpr float f16_to_f32_native(uint16_t val) noexcept
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
		if (exp16 == 0 && frac16 != 0)  MUU_UNLIKELY
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
		else if (exp16 == 0 && frac16 == 0) MUU_UNLIKELY
		{
			exp32 = 0;
		}
		// +- inf
		else if (exp16 == 31 && frac16 == 0) MUU_UNLIKELY
		{
			exp32 = 0xFF;
		}
		// +- nan
		else if (exp16 == 31 && frac16 != 0) MUU_UNLIKELY
		{
			exp32 = 0xFF;
		}

		exp32 = exp32 << 23;
		frac32 = frac32 << 13;
		return bit_cast<float>(s32 | exp32 | frac32);
	}

	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr uint16_t f32_to_f16(float val) noexcept
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
	constexpr float f16_to_f32(uint16_t val) noexcept
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
		static constexpr bool is_bounded = false;
		static constexpr bool is_modulo = false;
		static constexpr int digits = 11;
		static constexpr int digits10 = 3;
		static constexpr int max_digits10 = 5;
		static constexpr int radix = 2;
		static constexpr int min_exponent = -13;
		static constexpr int min_exponent10 = -4;
		static constexpr int max_exponent = 16;
		static constexpr int max_exponent10 = 4;
		static constexpr bool traps = true;
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
