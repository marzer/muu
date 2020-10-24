// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "muu/half.h"
#if MUU_HALF_USE_INTRINSICS

#if !MUU_MSVC
	MUU_DISABLE_WARNINGS
	#include <immintrin.h>
	MUU_ENABLE_WARNINGS
#endif

MUU_PUSH_WARNINGS
MUU_PRAGMA_MSVC(warning(disable: 4556)) // value of intrinsic immediate argument '8' is out of range '0 - 7'
MUU_PRAGMA_GCC(diagnostic ignored "-Wold-style-cast") // false positive with _mm_load_ss

MUU_IMPL_NAMESPACE_START
{
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
}
MUU_IMPL_NAMESPACE_END

MUU_POP_WARNINGS

#endif // MUU_HALF_USE_INTRINSICS

