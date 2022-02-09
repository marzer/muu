// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once
/// \cond

#include "../bounding_box.h"
#include "../oriented_bounding_box.h"
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
	inline constexpr bounding_box<Scalar> MUU_VECTORCALL bounding_box<Scalar>::from_obb(
		MUU_VC_PARAM(oriented_bounding_box<Scalar>) obb) noexcept
	{
		const vector_type corners[] = {
			obb.template corner<box_corners::min>(), obb.template corner<muu::box_corners::x>(),
			obb.template corner<box_corners::y>(),	 obb.template corner<muu::box_corners::xy>(),
			obb.template corner<box_corners::z>(),	 obb.template corner<muu::box_corners::xz>(),
			obb.template corner<box_corners::yz>(),	 obb.template corner<muu::box_corners::max>()
		};

		return bounding_box{ corners };
	}
}

//----------------------------------------------------------------------------------------------------------------------
// oriented_bounding_box.h implementations
//----------------------------------------------------------------------------------------------------------------------

namespace muu
{

}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
/// \endcond
