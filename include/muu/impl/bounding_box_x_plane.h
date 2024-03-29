// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \cond
#include "../plane.h"
#include "../bounding_box.h"
#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(float_control(except, off))

//----------------------------------------------------------------------------------------------------------------------
// bounding_box.h implementations
//----------------------------------------------------------------------------------------------------------------------

namespace muu
{
	template <typename Scalar>
	MUU_PURE_GETTER
	constexpr bool MUU_VECTORCALL bounding_box<Scalar>::intersects(MUU_VPARAM(bounding_box) bb,
																   MUU_VPARAM(plane<scalar_type>) p) noexcept
	{
		return plane<scalar_type>::distance(p, bb.center) <= vector_type::dot(bb.extents, vector_type::abs(p.normal));
	}
}

//----------------------------------------------------------------------------------------------------------------------
// plane.h implementations
//----------------------------------------------------------------------------------------------------------------------

namespace muu
{
	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL plane<Scalar>::intersects(MUU_VPARAM(plane) p,
															MUU_VPARAM(bounding_box<Scalar>) bb) noexcept
	{
		return bounding_box<Scalar>::intersects(bb, p);
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
/// \endcond
