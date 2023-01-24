// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \cond
#include "../oriented_bounding_box.h"
#include "../bounding_sphere.h"
#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(float_control(except, off))

//----------------------------------------------------------------------------------------------------------------------
// oriented_bounding_box.h implementations
//----------------------------------------------------------------------------------------------------------------------

namespace muu
{
	template <typename Scalar>
	MUU_PURE_GETTER
	constexpr bool MUU_VECTORCALL oriented_bounding_box<Scalar>::intersects(MUU_VPARAM(oriented_bounding_box) bb,
																			MUU_VPARAM(bounding_sphere<Scalar>)
																				bs) noexcept
	{
		return vector_type::distance_squared(bb.closest_point(bs.center), bs.center) <= bs.radius * bs.radius;
	}
}

//----------------------------------------------------------------------------------------------------------------------
// bounding_sphere.h implementations
//----------------------------------------------------------------------------------------------------------------------

namespace muu
{
	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL bounding_sphere<Scalar>::intersects(MUU_VPARAM(bounding_sphere) bs,
																	  MUU_VPARAM(oriented_bounding_box<scalar_type>)
																		  bb) noexcept
	{
		return oriented_bounding_box<Scalar>::intersects(bb, bs);
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
/// \endcond
