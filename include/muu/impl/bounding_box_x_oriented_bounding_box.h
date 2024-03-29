// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \cond
#include "../sat_tester.h"
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
	constexpr bool MUU_VECTORCALL bounding_box<Scalar>::contains(MUU_VPARAM(bounding_box) bb,
																 MUU_VPARAM(oriented_bounding_box<Scalar>) obb) noexcept
	{
		// todo: very likely a more efficient way of doing this

		return bounding_box::contains(bb, obb.template corner<box_corner::min>())
			&& bounding_box::contains(bb, obb.template corner<muu::box_corner::x>())
			&& bounding_box::contains(bb, obb.template corner<box_corner::y>())
			&& bounding_box::contains(bb, obb.template corner<muu::box_corner::xy>())
			&& bounding_box::contains(bb, obb.template corner<box_corner::z>())
			&& bounding_box::contains(bb, obb.template corner<muu::box_corner::xz>())
			&& bounding_box::contains(bb, obb.template corner<box_corner::yz>())
			&& bounding_box::contains(bb, obb.template corner<muu::box_corner::max>());
	}

	template <typename Scalar>
	MUU_PURE_GETTER
	constexpr bounding_box<Scalar> MUU_VECTORCALL bounding_box<Scalar>::from_obb(
		MUU_VPARAM(oriented_bounding_box<Scalar>) obb) noexcept
	{
		const vector_type corners[] = {
			obb.template corner<box_corner::min>(), obb.template corner<muu::box_corner::x>(),
			obb.template corner<box_corner::y>(),	obb.template corner<muu::box_corner::xy>(),
			obb.template corner<box_corner::z>(),	obb.template corner<muu::box_corner::xz>(),
			obb.template corner<box_corner::yz>(),	obb.template corner<muu::box_corner::max>()
		};

		return bounding_box{ corners };
	}

	template <typename Scalar>
	MUU_PURE_GETTER
	constexpr bool MUU_VECTORCALL bounding_box<Scalar>::intersects(MUU_VPARAM(bounding_box) aabb,
																   MUU_VPARAM(oriented_bounding_box<Scalar>)
																	   obb) noexcept
	{
		const vector_type obb_corners[] = {
			obb.template corner<box_corner::min>(), obb.template corner<box_corner::x>(),
			obb.template corner<box_corner::xy>(),	obb.template corner<box_corner::xz>(),
			obb.template corner<box_corner::y>(),	obb.template corner<box_corner::yz>(),
			obb.template corner<box_corner::z>(),	obb.template corner<box_corner::max>()
		};

		// test aabb axes first - cheaper since the axes and ranges require less work
		const auto aabb_min	 = aabb.min_corner();
		const auto aabb_max	 = aabb.max_corner();
		const auto test_aabb = [&](auto idx) noexcept -> bool
		{
			constexpr auto axis_index = remove_cvref<decltype(idx)>::value;
			sat_tester sat{ index_tag<axis_index>{}, obb_corners[0] };
			for (size_t c = 1; c < 8; c++)
				sat.add(index_tag<axis_index>{}, obb_corners[c]);
			return sat(aabb_min.template get<axis_index>(), aabb_max.template get<axis_index>());
		};
		if (!test_aabb(x_axis_tag{}) || !test_aabb(y_axis_tag{}) || !test_aabb(z_axis_tag{}))
			return false;

		const vector_type aabb_corners[] = {
			aabb.template corner<box_corner::min>(), aabb.template corner<box_corner::x>(),
			aabb.template corner<box_corner::xy>(),	 aabb.template corner<box_corner::xz>(),
			aabb.template corner<box_corner::y>(),	 aabb.template corner<box_corner::yz>(),
			aabb.template corner<box_corner::z>(),	 aabb.template corner<box_corner::max>()
		};

		// test obb axes
		for (size_t i = 0; i < 3; i++)
		{
			sat_tester tester1{ obb.axes.m[i], obb_corners[0] };
			for (size_t c = 1; c < 8; c++)
				tester1.add(obb.axes.m[i], obb_corners[c]);

			sat_tester tester2{ obb.axes.m[i], aabb_corners[0] };
			for (size_t c = 1; c < 8; c++)
				tester2.add(obb.axes.m[i], aabb_corners[c]);

			if (!tester1(tester2))
				return false;
		}

		return true;
	}
}

//----------------------------------------------------------------------------------------------------------------------
// oriented_bounding_box.h implementations
//----------------------------------------------------------------------------------------------------------------------

namespace muu
{
	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL oriented_bounding_box<Scalar>::intersects(MUU_VPARAM(oriented_bounding_box) obb,
																			MUU_VPARAM(bounding_box<Scalar>)
																				aabb) noexcept
	{
		return bounding_box<Scalar>::intersects(aabb, obb);
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
/// \endcond
