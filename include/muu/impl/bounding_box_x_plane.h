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
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL bounding_box<Scalar>::intersects(MUU_VC_PARAM(bounding_box) bb,
																   MUU_VC_PARAM(plane<Scalar>) p) noexcept
	{
		return aabbs::intersects_plane(bb.center, bb.extents, p.n, p.d);
	}

	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL bounding_box<Scalar>::intersects(MUU_VC_PARAM(plane<scalar_type>) p) const noexcept
	{
		return aabbs::intersects_plane(base::center, base::extents, p.n, p.d);
	}
}

//----------------------------------------------------------------------------------------------------------------------
// plane.h implementations
//----------------------------------------------------------------------------------------------------------------------

namespace muu
{
	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL plane<Scalar>::intersects(MUU_VC_PARAM(plane) p,
															MUU_VC_PARAM(bounding_box<Scalar>) bb) noexcept
	{
		return aabbs::intersects_plane(bb.center, bb.extents, p.n, p.d);
	}

	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL plane<Scalar>::intersects(MUU_VC_PARAM(bounding_box<scalar_type>) bb) const noexcept
	{
		return aabbs::intersects_plane(bb.center, bb.extents, base::n, base::d);
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
/// \endcond