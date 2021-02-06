// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "../../muu/vector.h"
#include "../../muu/matrix.h"

namespace muu
{
	/// \brief Corner masks/indices for three-dimensional bounding-boxes.
	/// \ingroup math
	/// 	 
	/// \see
	/// 	- muu::bounding_box  
	/// 	- muu::oriented_bounding_box
	enum class box_corners : uint8_t
	{
		min = 0, ///< The 'min' corner (negative offset on all axes).
		x = 1, ///< The corner with a positive offset on the X axis, negative on Y and Z.
		y = 2, ///< The corner with a positive offset on the Y axis, negative on X and Z.
		xy = x | y, ///< The corner with a positive offset on the X and Y axes, negative on Z.
		z = 4, ///< The corner with a positive offset on the Z axis, negative on X and Y.
		xz = x | z, ///< The corner with a positive offset on the X and Z axes, negative on Y.
		yz = y | z, ///< The corner with a positive offset on the Y and Z axes, negative on X.
		max = x | y | z ///< The 'max' corner (positive offset on all axes).
	};
	MUU_MAKE_FLAGS(box_corners);
}

/// \cond

MUU_PUSH_WARNINGS;
MUU_PRAGMA_MSVC(inline_recursion(on))
MUU_PRAGMA_MSVC(float_control(push))
MUU_PRAGMA_MSVC(float_control(except, off))
MUU_PRAGMA_MSVC(float_control(precise, off))
MUU_PRAGMA_MSVC(push_macro("min"))
MUU_PRAGMA_MSVC(push_macro("max"))
#if MUU_MSVC
	#undef min
	#undef max
#endif

namespace muu::impl
{
	MUU_ABI_VERSION_START(0);

	template <typename Scalar>
	struct MUU_TRIVIAL_ABI plane_
	{
		vector<Scalar, 3> normal;
		Scalar D;
	};

	template <typename Scalar, size_t Dimensions>
	struct MUU_TRIVIAL_ABI triangle_
	{
		vector<Scalar, Dimensions> points[3];
	};

	template <typename Scalar>
	struct MUU_TRIVIAL_ABI bounding_sphere_
	{
		vector<Scalar, 3> center;
		Scalar radius;
	};

	template <typename Scalar>
	struct MUU_TRIVIAL_ABI bounding_box_
	{
		vector<Scalar, 3> center;
		vector<Scalar, 3> extents;
	};

	template <typename Scalar>
	struct MUU_TRIVIAL_ABI oriented_bounding_box_
	{
		vector<Scalar, 3> center;
		vector<Scalar, 3> extents;
		matrix<Scalar, 3, 3> axes;
	};

	MUU_ABI_VERSION_END;

	template <typename Scalar>
	inline constexpr bool is_hva<plane_<Scalar>> = can_be_hva_of<Scalar, plane_<Scalar>>;

	template <typename Scalar>
	inline constexpr bool is_hva<plane<Scalar>> = is_hva<plane_<Scalar>>;

	template <typename Scalar>
	struct readonly_param_<plane<Scalar>>
	{
		using type = std::conditional_t<
			pass_readonly_by_value<plane_<Scalar>>,
			plane<Scalar>,
			const plane<Scalar>&
		>;
	};

	template <typename Scalar, size_t Dimensions>
	inline constexpr bool is_hva<triangle_<Scalar, Dimensions>> = can_be_hva_of<Scalar, triangle_<Scalar, Dimensions>>;

	template <typename Scalar, size_t Dimensions>
	inline constexpr bool is_hva<triangle<Scalar, Dimensions>> = is_hva<triangle_<Scalar, Dimensions>>;

	template <typename Scalar, size_t Dimensions>
	struct readonly_param_<triangle<Scalar, Dimensions>>
	{
		using type = std::conditional_t<
			pass_readonly_by_value<triangle_<Scalar, Dimensions>>,
			triangle<Scalar, Dimensions>,
			const triangle<Scalar, Dimensions>&
		>;
	};

	template <typename Scalar>
	inline constexpr bool is_hva<bounding_sphere_<Scalar>> = can_be_hva_of<Scalar, bounding_sphere_<Scalar>>;

	template <typename Scalar>
	inline constexpr bool is_hva<bounding_sphere<Scalar>> = is_hva<bounding_sphere_<Scalar>>;

	template <typename Scalar>
	struct readonly_param_<bounding_sphere<Scalar>>
	{
		using type = std::conditional_t<
			pass_readonly_by_value<bounding_sphere_<Scalar>>,
			bounding_sphere<Scalar>,
			const bounding_sphere<Scalar>&
		>;
	};

	template <typename Scalar>
	inline constexpr bool is_hva<bounding_box_<Scalar>> = can_be_hva_of<Scalar, bounding_box_<Scalar>>;

	template <typename Scalar>
	inline constexpr bool is_hva<bounding_box<Scalar>> = is_hva<bounding_box_<Scalar>>;

	template <typename Scalar>
	struct readonly_param_<bounding_box<Scalar>>
	{
		using type = std::conditional_t<
			pass_readonly_by_value<bounding_box_<Scalar>>,
			bounding_box<Scalar>,
			const bounding_box<Scalar>&
		>;
	};

	template <typename Scalar>
	inline constexpr bool is_hva<oriented_bounding_box_<Scalar>> = can_be_hva_of<Scalar, oriented_bounding_box_<Scalar>>;

	template <typename Scalar>
	inline constexpr bool is_hva<oriented_bounding_box<Scalar>> = is_hva<oriented_bounding_box_<Scalar>>;

	template <typename Scalar>
	struct readonly_param_<oriented_bounding_box<Scalar>>
	{
		using type = std::conditional_t<
			pass_readonly_by_value<oriented_bounding_box_<Scalar>>,
			oriented_bounding_box<Scalar>,
			const oriented_bounding_box<Scalar>&
		>;
	};
}

namespace muu
{
	template <typename From, typename Scalar>
	inline constexpr bool allow_implicit_bit_cast<From, impl::plane_<Scalar>>
		= allow_implicit_bit_cast<From, plane<Scalar>>;

	template <typename From, typename Scalar, size_t Dimensions>
	inline constexpr bool allow_implicit_bit_cast<From, impl::triangle_<Scalar, Dimensions>>
		= allow_implicit_bit_cast<From, triangle<Scalar, Dimensions>>;

	template <typename From, typename Scalar>
	inline constexpr bool allow_implicit_bit_cast<From, impl::bounding_sphere_<Scalar>>
		= allow_implicit_bit_cast<From, bounding_sphere<Scalar>>;

	template <typename From, typename Scalar>
	inline constexpr bool allow_implicit_bit_cast<From, impl::bounding_box_<Scalar>>
		= allow_implicit_bit_cast<From, bounding_box<Scalar>>;

	template <typename From, typename Scalar>
	inline constexpr bool allow_implicit_bit_cast<From, impl::oriented_bounding_box_<Scalar>>
		= allow_implicit_bit_cast<From, oriented_bounding_box<Scalar>>;
}

MUU_PRAGMA_MSVC(pop_macro("min"))
MUU_PRAGMA_MSVC(pop_macro("max"))
MUU_PRAGMA_MSVC(float_control(pop))
MUU_PRAGMA_MSVC(inline_recursion(off))
MUU_POP_WARNINGS;

/// \endcond
