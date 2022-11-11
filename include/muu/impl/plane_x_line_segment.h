// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once
/// \cond

#include "../plane.h"
#include "../line_segment.h"
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
	constexpr bool MUU_VECTORCALL plane<Scalar>::intersects(MUU_VPARAM(plane) p,
															MUU_VPARAM(line_segment<scalar_type>) seg) noexcept
	{
		return planes::intersects_line_segment(p.normal, p.d, seg.points[0], seg.points[1]);
	}
}

//----------------------------------------------------------------------------------------------------------------------
// line_segment.h implementations
//----------------------------------------------------------------------------------------------------------------------

namespace muu
{
	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL line_segment<Scalar>::intersects(MUU_VPARAM(line_segment) seg,
																   MUU_VPARAM(plane<scalar_type>) p) noexcept
	{
		return planes::intersects_line_segment(p.normal, p.d, seg.points[0], seg.points[1]);
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
/// \endcond
