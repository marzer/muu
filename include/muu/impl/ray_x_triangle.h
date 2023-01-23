// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
/// \cond
#pragma once

#include "../ray.h"
#include "../triangle.h"
#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(float_control(except, off))

//----------------------------------------------------------------------------------------------------------------------
// ray.h implementations
//----------------------------------------------------------------------------------------------------------------------

namespace muu
{
	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr typename ray<Scalar>::result_type MUU_VECTORCALL ray<Scalar>::hits(MUU_VPARAM(ray) r,
																				 MUU_VPARAM(triangle<Scalar>)
																					 tri) noexcept
	{
		return rays::hits_triangle(r.origin, r.direction, tri[0], tri[1], tri[2]);
	}
}

//----------------------------------------------------------------------------------------------------------------------
// triangle.h implementations
//----------------------------------------------------------------------------------------------------------------------

namespace muu
{

}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
