// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::axis_angle.

#include "vector.h"
#include "impl/std_iosfwd.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_DISABLE_SHADOW_WARNINGS;
MUU_PRAGMA_MSVC(float_control(except, off))

//======================================================================================================================
// IMPLEMENTATION DETAILS
//======================================================================================================================
/// \cond

namespace muu::impl
{
	template <typename Scalar>
	struct MUU_TRIVIAL_ABI storage_base<axis_angle<Scalar>>
	{
		vector<Scalar, 3> axis;
		Scalar angle;
	};

	template <typename Scalar>
	inline constexpr bool is_hva<storage_base<axis_angle<Scalar>>> =
		can_be_hva_of<Scalar, storage_base<axis_angle<Scalar>>>;

	template <typename Scalar>
	inline constexpr bool is_hva<axis_angle<Scalar>> = is_hva<storage_base<axis_angle<Scalar>>>;

	template <typename Scalar>
	struct vector_param_<axis_angle<Scalar>>
	{
		using type = copy_cvref<axis_angle<Scalar>, vector_param<storage_base<axis_angle<Scalar>>>>;
	};
}

namespace muu
{
	template <typename From, typename Scalar>
	inline constexpr bool allow_implicit_bit_cast<From, impl::storage_base<axis_angle<Scalar>>> =
		allow_implicit_bit_cast<From, axis_angle<Scalar>>;
}

/// \endcond

//======================================================================================================================
// AXIS-ANGLE ROTATION
//======================================================================================================================

namespace muu
{
	/// \brief	An axis-angle rotation.
	/// \ingroup math
	///
	/// \tparam	Scalar	The scalar component type of the data members.
	///
	/// \see		muu::quaternion
	template <typename Scalar>
	struct MUU_TRIVIAL_ABI axis_angle //
		MUU_HIDDEN_BASE(impl::storage_base<axis_angle<Scalar>>)
	{
		static_assert(!std::is_reference_v<Scalar>, "Axis-angle rotation scalar type cannot be a reference");
		static_assert(!is_cv<Scalar>, "Axis-angle rotation scalar type cannot be const- or volatile-qualified");
		static_assert(is_floating_point<Scalar>, "Axis-angle rotation scalar type must be a floating-point type");

		/// \brief The scalar type of the axis and angle.
		using scalar_type = Scalar;

		/// \brief The three-dimensional #muu::vector with the same #scalar_type as this axis-angle.
		using axis_type = vector<scalar_type, 3>;

	  private:
		/// \cond

		using base = impl::storage_base<axis_angle<Scalar>>;

		/// \endcond

	  public:
#if MUU_DOXYGEN

		/// \brief	The axis being rotated around.
		vector_type axis;

		/// \brief	The angle of rotation (in radians).
		scalar_type angle;

#endif // DOXYGEN

		/// \brief Default constructor. Values are not initialized.
		MUU_NODISCARD_CTOR
		axis_angle() noexcept = default;

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr axis_angle(const axis_angle&) noexcept = default;

		/// \brief Copy-assigment operator.
		constexpr axis_angle& operator=(const axis_angle&) noexcept = default;

		/// \brief Initializes from an axis and an angle.
		MUU_NODISCARD_CTOR
		constexpr axis_angle(const axis_type& axis, scalar_type angle) noexcept //
			: base{ axis, angle }
		{}

		/// \brief Initializes from an axis, setting angle to zero.
		MUU_NODISCARD_CTOR
		explicit constexpr axis_angle(const axis_type& axis) noexcept //
			: base{ axis_type{ axis }, scalar_type{} }
		{}

		/// \brief Initializes from a vec4, where x, y and z are the axis, and w is the angle.
		MUU_NODISCARD_CTOR
		explicit constexpr axis_angle(const vector<scalar_type, 4>& v) noexcept //
			: base{ axis_type{ v }, v.w }
		{}

		/// \brief Converting constructor.
		template <typename T>
		MUU_NODISCARD_CTOR
		explicit constexpr axis_angle(const axis_angle<T>& other) noexcept //
			: base{ axis_type{ other.axis }, static_cast<scalar_type>(other.angle) }
		{}

		/// \brief Returns a reference to the axis component.
		MUU_PURE_INLINE_GETTER
		explicit constexpr operator const axis_type&() const noexcept
		{
			return base::axis;
		}

		/// \brief Converts this axis-angle to a vec4, where x, y and z are the axis, and w is the angle.
		MUU_PURE_INLINE_GETTER
		explicit constexpr operator vector<scalar_type, 4>() const noexcept
		{
			return vector<scalar_type, 4>{ base::axis, base::angle };
		}

		/// \brief Writes an axis-angle rotation out to a text stream.
		template <typename Char, typename Traits>
		friend std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os, const axis_angle& rot)
		{
			impl::print_compound_vector(os, &rot.axis.x, 3_sz, true, &rot.angle, 1_sz, false);
			return os;
		}
	};

	/// \cond

	MUU_CONSTRAINED_TEMPLATE(is_arithmetic<Angle>, typename Axis, typename Angle)
	axis_angle(vector<Axis, 3>, Angle)->axis_angle<impl::std_math_common_type<impl::highest_ranked<Axis, Angle>>>;

	template <typename S, size_t D>
	axis_angle(vector<S, D>) -> axis_angle<impl::std_math_common_type<S>>;

	MUU_CONSTRAINED_TEMPLATE((all_arithmetic<Axis, Angle>), typename Axis, typename Angle)
	axis_angle(std::initializer_list<Axis>, Angle)
		->axis_angle<impl::std_math_common_type<impl::highest_ranked<Axis, Angle>>>;

	/// \endcond
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
