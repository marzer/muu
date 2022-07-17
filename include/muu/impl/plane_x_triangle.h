// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once
/// \cond

#include "../plane.h"
#include "../triangle.h"
#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(float_control(except, off))

//----------------------------------------------------------------------------------------------------------------------
// plane.h implementations
//----------------------------------------------------------------------------------------------------------------------

namespace muu
{
	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr plane<Scalar> MUU_VECTORCALL plane<Scalar>::from_triangle(MUU_VPARAM(triangle<Scalar>) tri) noexcept
	{
		return from_triangle(tri.points[0], tri.points[1], tri.points[2]);
	}
}

//----------------------------------------------------------------------------------------------------------------------
// triangle.h implementations
//----------------------------------------------------------------------------------------------------------------------

namespace muu
{
	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr muu::plane<Scalar> MUU_VECTORCALL triangle<Scalar>::plane(MUU_VPARAM(vector_type) p0,
																		MUU_VPARAM(vector_type) p1,
																		MUU_VPARAM(vector_type) p2) noexcept
	{
		return muu::plane<Scalar>{ p0, normal(p0, p1, p2) };
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
/// \endcond
