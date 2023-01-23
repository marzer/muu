// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
/// \cond
#pragma once

#include "../bounding_sphere.h"
#include "../line_segment.h"
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
	constexpr bool MUU_VECTORCALL bounding_sphere<Scalar>::contains(MUU_VPARAM(bounding_sphere) bb,
																	MUU_VPARAM(line_segment<Scalar>) seg) noexcept
	{
		return contains(bb, seg.points[0], seg.points[1]);
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
