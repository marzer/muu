// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once
/// \cond

#include "../bounding_box.h"
#include "../triangle.h"
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
																   MUU_VC_PARAM(triangle<Scalar>) tri) noexcept
	{
		return intersects(bb, tri.points[0], tri.points[1], tri.points[2]);
	}

	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL bounding_box<Scalar>::intersects(
		MUU_VC_PARAM(triangle<scalar_type>) tri) const noexcept
	{
		return intersects(*this, tri.points[0], tri.points[1], tri.points[2]);
	}

	template <typename Scalar>
	constexpr bounding_box<Scalar>& MUU_VECTORCALL bounding_box<Scalar>::append(
		MUU_VC_PARAM(triangle<scalar_type>) tri) noexcept
	{
		return append(tri.points[0], tri.points[1], tri.points[2]);
	}
}

//----------------------------------------------------------------------------------------------------------------------
// triangle.h implementations
//----------------------------------------------------------------------------------------------------------------------

namespace muu
{
	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL triangle<Scalar>::intersects(MUU_VC_PARAM(vector_type) p0,
															   MUU_VC_PARAM(vector_type) p1,
															   MUU_VC_PARAM(vector_type) p2,
															   MUU_VC_PARAM(bounding_box<scalar_type>) bb) noexcept
	{
		return bounding_box<scalar_type>::intersects(bb, p0, p1, p2);
	}

	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL triangle<Scalar>::intersects(MUU_VC_PARAM(triangle) tri,
															   MUU_VC_PARAM(bounding_box<scalar_type>) bb) noexcept
	{
		return bounding_box<scalar_type>::intersects(bb, tri.points[0], tri.points[1], tri.points[2]);
	}

	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL triangle<Scalar>::intersects(
		MUU_VC_PARAM(bounding_box<scalar_type>) bb) const noexcept
	{
		return bounding_box<scalar_type>::intersects(bb, base::points[0], base::points[1], base::points[2]);
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
/// \endcond
