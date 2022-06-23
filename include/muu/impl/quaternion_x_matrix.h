// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once
/// \cond

#include "../quaternion.h"
#include "../matrix.h"
#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(float_control(except, off))

namespace muu
{
	namespace impl
	{
		template <typename Scalar, size_t R, size_t C>
		MUU_PURE_GETTER
		inline constexpr quaternion<Scalar> MUU_VECTORCALL quat_from_rotation_matrix(MUU_VPARAM(matrix<Scalar, R, C>)
																									rot) noexcept
		{
#define MAT_GET(r, c) rot.m[c].template get<r>()

			MUU_FMA_BLOCK;
			static_assert(R >= 3);
			static_assert(C >= 3);

			const auto trace = MAT_GET(0, 0) + MAT_GET(1, 1) + MAT_GET(2, 2);

			if (trace > Scalar{})
			{
				const auto s	 = muu::sqrt(trace + Scalar{ 1 }) * Scalar{ 2 };
				const auto inv_s = Scalar{ 1 } / s;
				return {
					Scalar{ 0.25 } * s,						 // scalar
					(MAT_GET(2, 1) - MAT_GET(1, 2)) * inv_s, // vector.x
					(MAT_GET(0, 2) - MAT_GET(2, 0)) * inv_s, // vector.y
					(MAT_GET(1, 0) - MAT_GET(0, 1)) * inv_s	 // vector.z
				};
			}
			else if (MAT_GET(0, 0) > MAT_GET(1, 1) && MAT_GET(0, 0) > MAT_GET(2, 2))
			{
				const auto s	 = muu::sqrt(MAT_GET(0, 0) - MAT_GET(1, 1) - MAT_GET(2, 2) + Scalar{ 1 }) * Scalar{ 2 };
				const auto inv_s = Scalar{ 1 } / s;
				return {
					(MAT_GET(2, 1) - MAT_GET(1, 2)) * inv_s, // scalar
					Scalar{ 0.25 } * s,						 // vector.x
					(MAT_GET(0, 1) + MAT_GET(1, 0)) * inv_s, // vector.y
					(MAT_GET(0, 2) + MAT_GET(2, 0)) * inv_s	 // vector.z
				};
			}
			else if (MAT_GET(1, 1) > MAT_GET(2, 2))
			{
				const auto s	 = muu::sqrt(MAT_GET(1, 1) - MAT_GET(0, 0) - MAT_GET(2, 2) + Scalar{ 1 }) * Scalar{ 2 };
				const auto inv_s = Scalar{ 1 } / s;
				return {
					(MAT_GET(0, 2) - MAT_GET(2, 0)) * inv_s, // scalar
					(MAT_GET(0, 1) + MAT_GET(1, 0)) * inv_s, // vector.x
					Scalar{ 0.25 } * s,						 // vector.y
					(MAT_GET(2, 1) + MAT_GET(1, 2)) * inv_s	 // vector.z
				};
			}
			else
			{
				const auto s	 = muu::sqrt(MAT_GET(2, 2) - MAT_GET(0, 0) - MAT_GET(1, 1) + Scalar{ 1 }) * Scalar{ 2 };
				const auto inv_s = Scalar{ 1 } / s;
				return {
					(MAT_GET(1, 0) - MAT_GET(0, 1)) * inv_s, // scalar
					(MAT_GET(0, 2) + MAT_GET(2, 0)) * inv_s, // vector.x
					(MAT_GET(2, 1) + MAT_GET(1, 2)) * inv_s, // vector.y
					Scalar{ 0.25 } * s						 // vector.z
				};
			}
#undef MAT_GET
		}

	}

	template <typename Scalar>
	MUU_PURE_GETTER
	inline constexpr quaternion<Scalar> quaternion<Scalar>::from_rotation(MUU_VPARAM(matrix<Scalar, 3, 3>) rot) noexcept
	{
		if constexpr (is_small_float)
			return quaternion{ promoted_quat::from_rotation(matrix<promoted_scalar, 3, 3>{ rot }) };
		else
			return impl::quat_from_rotation_matrix<Scalar, 3, 3>(rot);
	}

	template <typename Scalar>
	MUU_PURE_GETTER
	inline constexpr quaternion<Scalar> quaternion<Scalar>::from_rotation(MUU_VPARAM(matrix<Scalar, 3, 4>) rot) noexcept
	{
		if constexpr (is_small_float)
			return quaternion{ promoted_quat::from_rotation(matrix<promoted_scalar, 3, 3>{ rot }) };
		else
			return impl::quat_from_rotation_matrix<Scalar, 3, 4>(rot);
	}

	template <typename Scalar>
	MUU_PURE_GETTER
	inline constexpr quaternion<Scalar> quaternion<Scalar>::from_rotation(MUU_VPARAM(matrix<Scalar, 4, 4>) rot) noexcept
	{
		if constexpr (is_small_float)
			return quaternion{ promoted_quat::from_rotation(matrix<promoted_scalar, 3, 3>{ rot }) };
		else
			return impl::quat_from_rotation_matrix<Scalar, 4, 4>(rot);
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
/// \endcond
