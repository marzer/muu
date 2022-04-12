// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once
/// \cond

#include "../bounding_box.h"
#include "../line_segment.h"
#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(float_control(except, off))

//----------------------------------------------------------------------------------------------------------------------
// bounding_box.h implementations
//----------------------------------------------------------------------------------------------------------------------

namespace muu
{
	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL bounding_box<Scalar>::intersects(MUU_VC_PARAM(bounding_box) bb,
																   MUU_VC_PARAM(line_segment<scalar_type>) seg) noexcept
	{
		return aabbs::intersects_line_segment(bb.center, bb.extents, seg.points[0], seg.points[1]);
	}

	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL bounding_box<Scalar>::intersects(
		MUU_VC_PARAM(line_segment<scalar_type>) seg) const noexcept
	{
		return aabbs::intersects_line_segment(base::center, base::extents, seg.points[0], seg.points[1]);
	}

	template <typename Scalar>
	MUU_PURE_GETTER
	constexpr bounding_box<Scalar> MUU_VECTORCALL bounding_box<Scalar>::append(
		MUU_VC_PARAM(bounding_box) bb,
		MUU_VC_PARAM(line_segment<scalar_type>) seg) noexcept
	{
		if constexpr (requires_promotion)
		{
			return bounding_box{ promoted_box::append(promoted_box{ bb }, line_segment<promoted_scalar>{ seg }) };
		}
		else
		{
			return from_points(vector_type::min(bounding_box::min_corner(bb), seg.points[0], seg.points[1]),
							   vector_type::max(bounding_box::max_corner(bb), seg.points[0], seg.points[1]));
		}
	}

	template <typename Scalar>
	constexpr bounding_box<Scalar>& bounding_box<Scalar>::append(MUU_VC_PARAM(line_segment<scalar_type>) seg) noexcept
	{
		return *this = append(*this, seg);
	}

}

//----------------------------------------------------------------------------------------------------------------------
// line_segment.h implementations
//----------------------------------------------------------------------------------------------------------------------

namespace muu
{
	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL line_segment<Scalar>::intersects(MUU_VC_PARAM(line_segment) seg,
																   MUU_VC_PARAM(bounding_box<scalar_type>) bb) noexcept
	{
		return aabbs::intersects_line_segment(bb.center, bb.extents, seg.points[0], seg.points[1]);
	}

	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL line_segment<Scalar>::intersects(
		MUU_VC_PARAM(bounding_box<scalar_type>) bb) const noexcept
	{
		return aabbs::intersects_line_segment(bb.center, bb.extents, base::points[0], base::points[1]);
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
/// \endcond
