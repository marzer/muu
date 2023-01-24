// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \cond
#include "../triangle.h"
#include "../line_segment.h"
#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(float_control(except, off))

//----------------------------------------------------------------------------------------------------------------------
// triangle.h implementations
//----------------------------------------------------------------------------------------------------------------------

namespace muu
{
	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL triangle<Scalar>::contains(MUU_VPARAM(triangle) tri,
															 MUU_VPARAM(line_segment<Scalar>) seg,
															 Scalar epsilon) noexcept
	{
		return contains(tri.points[0], tri.points[1], tri.points[2], seg.points[0], seg.points[1], epsilon);
	}
}

//----------------------------------------------------------------------------------------------------------------------
// line_segment.h implementations
//----------------------------------------------------------------------------------------------------------------------

namespace muu
{

}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
/// \endcond
