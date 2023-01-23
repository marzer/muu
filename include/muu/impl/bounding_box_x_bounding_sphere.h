// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
/// \cond
#pragma once

#include "../bounding_box.h"
#include "../bounding_sphere.h"
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
	constexpr bounding_box<Scalar> MUU_VECTORCALL bounding_box<Scalar>::from_sphere(MUU_VPARAM(bounding_sphere<Scalar>)
																						bs) noexcept
	{
		return { bs.center, vector_type{ bs.radius } };
	}

	template <typename Scalar>
	MUU_PURE_GETTER
	constexpr bool MUU_VECTORCALL bounding_box<Scalar>::contains(MUU_VPARAM(bounding_box) bb,
																 MUU_VPARAM(bounding_sphere<Scalar>) bs) noexcept
	{
		const auto min = bb.min_corner();
		const auto max = bb.max_corner();
		if (!aabbs::intersects_sphere_min_max_radsq(min, max, bs.center, bs.radius * bs.radius))
			return false;

		return aabbs::contains_aabb_min_max(min,
											max,
											bs.center - vector_type{ bs.radius },
											bs.center + vector_type{ bs.radius });
	}

	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL bounding_box<Scalar>::intersects(MUU_VPARAM(bounding_box) bb,
																   MUU_VPARAM(bounding_sphere<Scalar>) bs) noexcept
	{
		return aabbs::intersects_sphere(bb.center, bb.extents, bs.center, bs.radius);
	}

	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL collision_tester<bounding_box<Scalar>>::operator()(
		MUU_VPARAM(bounding_sphere<scalar_type>) bs) const noexcept
	{
		return aabbs::intersects_sphere_min_max_radsq(min, max, bs.center, bs.radius * bs.radius);
	}

	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL collision_tester<bounding_box<Scalar>>::operator()(
		const collision_tester<bounding_sphere<scalar_type>>& bs_tester) const noexcept
	{
		return aabbs::intersects_sphere_min_max_radsq(min, max, bs_tester.center, bs_tester.radius_squared);
	}

}

//----------------------------------------------------------------------------------------------------------------------
// bounding_sphere.h implementations
//----------------------------------------------------------------------------------------------------------------------

namespace muu
{
	template <typename Scalar>
	MUU_PURE_GETTER
	constexpr bool MUU_VECTORCALL bounding_sphere<Scalar>::contains(MUU_VPARAM(bounding_sphere) outer,
																	MUU_VPARAM(bounding_box<Scalar>) inner) noexcept
	{
		const auto outer_radius_squared = outer.radius * outer.radius;
		const auto inner_min			= inner.min_corner();
		const auto inner_max			= inner.max_corner();

		if (!aabbs::intersects_sphere_min_max_radsq(inner_min, inner_max, outer.center, outer_radius_squared))
			return false;

		return vector_type::distance_squared(
				   aabbs::furthest_center_min_max(inner.center, inner_min, inner_max, outer.center),
				   outer.center)
			<= outer_radius_squared;
	}

	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL bounding_sphere<Scalar>::intersects(MUU_VPARAM(bounding_sphere) bs,
																	  MUU_VPARAM(bounding_box<Scalar>) bb) noexcept
	{
		return aabbs::intersects_sphere(bb.center, bb.extents, bs.center, bs.radius);
	}

	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL collision_tester<bounding_sphere<Scalar>>::operator()(
		MUU_VPARAM(bounding_box<scalar_type>) bb) const noexcept
	{
		return aabbs::intersects_sphere_min_max_radsq(bb.min_corner(), bb.max_corner(), center, radius_squared);
	}

	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL collision_tester<bounding_sphere<Scalar>>::operator()(
		const collision_tester<bounding_box<scalar_type>>& tester) const noexcept
	{
		return aabbs::intersects_sphere_min_max_radsq(tester.min, tester.max, center, radius_squared);
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
