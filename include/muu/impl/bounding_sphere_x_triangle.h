// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once
/// \cond

#include "../bounding_sphere.h"
#include "../triangle.h"
#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(float_control(except, off))

//----------------------------------------------------------------------------------------------------------------------
// bounding_sphere.h implementations
//----------------------------------------------------------------------------------------------------------------------

namespace muu
{
	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL bounding_sphere<Scalar>::intersects(MUU_VPARAM(bounding_sphere) bs,
																	  MUU_VPARAM(triangle<Scalar>) tri) noexcept
	{
		return intersects(bs, tri.points[0], tri.points[1], tri.points[2]);
	}

}

//----------------------------------------------------------------------------------------------------------------------
// triangle.h implementations
//----------------------------------------------------------------------------------------------------------------------

namespace muu
{
	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL triangle<Scalar>::intersects(MUU_VPARAM(vector_type) p0,
															   MUU_VPARAM(vector_type) p1,
															   MUU_VPARAM(vector_type) p2,
															   MUU_VPARAM(bounding_sphere<scalar_type>) bs) noexcept
	{
		return bounding_sphere<scalar_type>::intersects(bs, p0, p1, p2);
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
/// \endcond
