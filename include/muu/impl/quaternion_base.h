// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

#include "../vector.h"
#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_DISABLE_SHADOW_WARNINGS;
MUU_DISABLE_SUGGEST_WARNINGS;
MUU_DISABLE_ARITHMETIC_WARNINGS;
MUU_PRAGMA_MSVC(float_control(except, off))
MUU_PRAGMA_MSVC(float_control(precise, off))
/// \cond

namespace muu::impl
{
	//--- axis_angle_rotation ------------------------------------------------------------------------------------------

	template <typename Scalar>
	struct MUU_TRIVIAL_ABI axis_angle_rotation_
	{
		vector<Scalar, 3> axis;
		Scalar angle;
	};

	template <typename Scalar>
	inline constexpr bool is_hva<axis_angle_rotation_<Scalar>> = can_be_hva_of<Scalar, axis_angle_rotation_<Scalar>>;

	template <typename Scalar>
	inline constexpr bool is_hva<axis_angle_rotation<Scalar>> = is_hva<axis_angle_rotation_<Scalar>>;

	template <typename Scalar>
	struct vectorcall_param_<axis_angle_rotation<Scalar>>
	{
		using type = std::conditional_t<pass_vectorcall_by_value<axis_angle_rotation_<Scalar>>,
										axis_angle_rotation<Scalar>,
										const axis_angle_rotation<Scalar>&>;
	};

	//--- euler_rotation ------------------------------------------------------------------------------------------

	template <typename Scalar>
	struct MUU_TRIVIAL_ABI euler_rotation_
	{
		Scalar yaw;
		Scalar pitch;
		Scalar roll;
	};

	template <typename Scalar>
	inline constexpr bool is_hva<euler_rotation_<Scalar>> = can_be_hva_of<Scalar, euler_rotation_<Scalar>>;

	template <typename Scalar>
	inline constexpr bool is_hva<euler_rotation<Scalar>> = is_hva<euler_rotation_<Scalar>>;

	template <typename Scalar>
	struct vectorcall_param_<euler_rotation<Scalar>>
	{
		using type = std::conditional_t<pass_vectorcall_by_value<euler_rotation_<Scalar>>,
										euler_rotation<Scalar>,
										const euler_rotation<Scalar>&>;
	};

	//--- quaternion ---------------------------------------------------------------------------------------------------

	template <typename Scalar>
	struct MUU_TRIVIAL_ABI quaternion_
	{
		Scalar s;
		vector<Scalar, 3> v;
	};

	template <typename Scalar>
	inline constexpr bool is_hva<quaternion_<Scalar>> = can_be_hva_of<Scalar, quaternion_<Scalar>>;

	template <typename Scalar>
	inline constexpr bool is_hva<quaternion<Scalar>> = is_hva<quaternion_<Scalar>>;

	template <typename Scalar>
	struct vectorcall_param_<quaternion<Scalar>>
	{
		using type = std::
			conditional_t<pass_vectorcall_by_value<quaternion_<Scalar>>, quaternion<Scalar>, const quaternion<Scalar>&>;
	};
}

namespace muu
{

	template <typename From, typename Scalar>
	inline constexpr bool allow_implicit_bit_cast<From, impl::axis_angle_rotation_<Scalar>> =
		allow_implicit_bit_cast<From, axis_angle_rotation<Scalar>>;

	template <typename From, typename Scalar>
	inline constexpr bool allow_implicit_bit_cast<From, impl::euler_rotation_<Scalar>> =
		allow_implicit_bit_cast<From, euler_rotation<Scalar>>;

	template <typename From, typename Scalar>
	inline constexpr bool allow_implicit_bit_cast<From, impl::quaternion_<Scalar>> =
		allow_implicit_bit_cast<From, quaternion<Scalar>>;
}

/// \endcond
MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
