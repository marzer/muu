// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::oriented_bounding_box.

#pragma once
#include "../muu/impl/bounding_boxes_common.h"

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

#if MUU_ENABLE_PAIRED_FUNCS
	#define MUU_RO_VEC		muu::impl::readonly_param<vector_type>
#else
	#define MUU_RO_VEC		const vector_type&
#endif

//======================================================================================================================
// ORIENTED BOUNDING BOX CLASS
#if 1

namespace muu
{
	MUU_ABI_VERSION_START(0);

	/// \brief An oriented bounding box.
	/// \ingroup math
	/// 
	/// \tparam	Scalar      The type of the oriented bounding box's scalar components.
	/// 
	/// \see [Oriented Bounding Box](https://www.sciencedirect.com/topics/computer-science/oriented-bounding-box)
	template <typename Scalar>
	struct MUU_TRIVIAL_ABI oriented_bounding_box
		#ifndef DOXYGEN
		: impl::oriented_bounding_box_<Scalar>
		#endif
	{
		static_assert(
			!std::is_reference_v<Scalar>,
			"Oriented bounding box scalar type cannot be a reference"
		);
		static_assert(
			!std::is_const_v<Scalar> && !std::is_volatile_v<Scalar>,
			"Oriented bounding box scalar type cannot be const- or volatile-qualified"
		);
		static_assert(
			std::is_trivially_constructible_v<Scalar>
			&& std::is_trivially_copyable_v<Scalar>
			&& std::is_trivially_destructible_v<Scalar>,
			"Oriented bounding box scalar type must be trivially constructible, copyable and destructible"
		);
		static_assert(
			is_floating_point<Scalar>,
			"Oriented bounding box scalar type must be a floating-point type"
		);
		static_assert(
			is_signed<Scalar>,
			"Oriented bounding box scalar type must be signed"
		);

		/// \brief The oriented bounding box's scalar type.
		using scalar_type = Scalar;

		/// \brief The scalar type used for length, distance, volume, etc. Always floating-point.
		using delta_type = std::conditional_t<is_integral<scalar_type>, double, scalar_type>;

		/// \brief The three-dimensional #muu::vector with the same #scalar_type as the oriented bounding box.
		using vector_type = vector<scalar_type, 3>;

		/// \brief Compile-time constants for this bounding box's #vector_type.
		using vector_constants = muu::constants<vector_type>;

		/// \brief The 3x3 #muu::matrix with the same #scalar_type as the oriented bounding box.
		using axes_type = matrix<scalar_type, 3, 3>;

		/// \brief Compile-time constants for this bounding box's #axes_type.
		using axes_constants = muu::constants<axes_type>;

		/// \brief Compile-time bounding box constants.
		using constants = muu::constants<bounding_box>;

	private:

		using base = impl::oriented_bounding_box_<Scalar>;
		static_assert(
			sizeof(base) == (sizeof(vector_type) * 2 + sizeof(axes_type)),
			"Oriented bounding boxes should not have padding"
		);

		using boxes = impl::bounding_boxes_common<Scalar>;
		using intermediate_float = impl::promote_if_small_float<delta_type>;
		static_assert(is_floating_point<delta_type>);
		static_assert(is_floating_point<intermediate_float>);

	public:

		#ifdef DOXYGEN

		/// \brief	The center of the box.
		vector_type center;

		/// \brief	The half-lengths of box (i.e. distances from the center to the sides).
		vector_type extents;

		/// \brief	The axes of the box's orientation.
		axes_type axes;

		#endif //DOXYGEN

	#if 1 // constructors ----------------------------------------------------------------------------------------------

		/// \brief Default constructor. Values are not initialized.
		oriented_bounding_box() noexcept = default;

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr oriented_bounding_box(const oriented_bounding_box&) noexcept = default;

		/// \brief Copy-assigment operator.
		constexpr oriented_bounding_box& operator = (const oriented_bounding_box&) noexcept = default;

		/// \brief	Constructs a oriented bounding box from center and extent values.
		///
		/// \param	cen		The center point.
		/// \param	ext		The extents.
		MUU_NODISCARD_CTOR
		constexpr oriented_bounding_box(const vector_type& cen, const vector_type& ext) noexcept
			: base{ cen, ext, axes_constants::identity }
		{ }

		/// \brief	Constructs a oriented bounding box from center and extent values.
		///
		/// \param	cen		The center point.
		/// \param	ext_x	The length of the X extent.
		/// \param	ext_y	The length of the Y extent.
		/// \param	ext_z	The length of the Z extent.
		MUU_NODISCARD_CTOR
		constexpr oriented_bounding_box(const vector_type& cen, scalar_type ext_x, scalar_type ext_y, scalar_type ext_z) noexcept
			: base{ cen, { ext_x, ext_y, ext_z }, axes_constants::identity }
		{ }

		/// \brief	Constructs a uniformly-sized oriented bounding box.
		///
		/// \param	cen		The center point.
		/// \param	ext		The length of all three extents.
		MUU_NODISCARD_CTOR
		constexpr oriented_bounding_box(const vector_type& cen, scalar_type ext) noexcept
			: base{ cen, { ext, ext, ext }, axes_constants::identity }
		{ }

		/// \brief	Constructs an oriented bounding box at the origin.
		///
		/// \param	ext		The extents.
		MUU_NODISCARD_CTOR
			explicit
			constexpr oriented_bounding_box(const vector_type& ext) noexcept
			: base{ vector_constants::zero, ext, axes_constants::identity }
		{ }

		/// \brief	Constructs an oriented bounding box from center and extent values.
		///
		/// \param	cen_x	The X component of the center point.
		/// \param	cen_y	The Y component of the center point.
		/// \param	cen_z	The Z component of the center point.
		/// \param	ext		The extents.
		MUU_NODISCARD_CTOR
		constexpr oriented_bounding_box(scalar_type cen_x, scalar_type cen_y, scalar_type cen_z, const vector_type& ext) noexcept
			: base { { cen_x, cen_y, cen_z }, ext, axes_constants::identity }
		{ }

		/// \brief	Constructs an oriented bounding box from center and extent values.
		/// 
		/// \param	cen_x	The X component of the center point.
		/// \param	cen_y	The Y component of the center point.
		/// \param	cen_z	The Z component of the center point.
		/// \param	ext_x	The length of the X extent.
		/// \param	ext_y	The length of the Y extent.
		/// \param	ext_z	The length of the Z extent.
		MUU_NODISCARD_CTOR
		constexpr oriented_bounding_box(
			scalar_type cen_x, scalar_type cen_y, scalar_type cen_z,
			scalar_type ext_x, scalar_type ext_y, scalar_type ext_z
		) noexcept
			: base{ { cen_x, cen_y, cen_z }, { ext_x, ext_y, ext_z }, axes_constants::identity }
		{ }

		/// \brief	Constructs a uniformly-sized oriented bounding box at the origin.
		///
		/// \param	ext		The length of all three extents.
		MUU_NODISCARD_CTOR
		explicit
		constexpr oriented_bounding_box(scalar_type ext) noexcept
			: base{ vector_constants::zero, { ext, ext, ext }, axes_constants::identity }
		{ }

		/// \brief Constructs a oriented bounding box from an implicitly bit-castable type.
		/// 
		/// \tparam T	A bit-castable type.
		/// 
		/// \see muu::allow_implicit_bit_cast
		MUU_CONSTRAINED_TEMPLATE(
			(allow_implicit_bit_cast<T, oriented_bounding_box>),
			typename T
		)
		MUU_NODISCARD_CTOR
		/*implicit*/
		constexpr oriented_bounding_box(const T& blittable) noexcept
			: base{ bit_cast<base>(blittable) }
		{
			static_assert(
				sizeof(T) == sizeof(base),
				"Bit-castable types must be the same size as the oriented bounding box"
			);
			static_assert(
				std::is_trivially_copyable_v<T>,
				"Bit-castable types must be trivially-copyable"
			);
		}

	#endif // constructors

	#if 1 // geometric properties --------------------------------------------------------------------------------------

		/// \brief	Returns the width of the box (x-axis).
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr delta_type width() const noexcept
		{
			return boxes::width(base::extents);
		}

		/// \brief	Returns the height of the box (y-axis).
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr delta_type height() const noexcept
		{
			return boxes::height(base::extents);
		}

		/// \brief	Returns the depth of the box (z-axis).
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr delta_type depth() const noexcept
		{
			return boxes::depth(base::extents);
		}

		/// \brief	Calculates the length of the line connecting the min and max points.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr delta_type diagonal() const noexcept
		{
			return boxes::diagonal(base::extents);
		}

		/// \brief	Returns the shortest of the box's three extents.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr scalar_type& shortest_extent() noexcept
		{
			return boxes::shortest_extent(base::extents);
		}

		/// \brief	Returns the longest of the box's three extents.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr scalar_type& longest_extent() noexcept
		{
			return boxes::longest_extent(base::extents);
		}

		/// \brief	Returns the shortest of the box's three extents (const overload).
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr const scalar_type& shortest_extent() const noexcept
		{
			return boxes::shortest_extent(base::extents);
		}

		/// \brief	Returns the longest of the box's three extents (const overload).
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr const scalar_type& longest_extent() const noexcept
		{
			return boxes::longest_extent(base::extents);
		}

		/// \brief	Returns the length of the shortest of the box's three sides.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr delta_type shortest_side() const noexcept
		{
			return boxes::longest_side(base::extents);
		}

		/// \brief	Returns the length of the longest of the box's three sides.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr delta_type longest_side() const noexcept
		{
			return boxes::longest_side(base::extents);
		}

		/// \brief	Calculates the volume of this bounding box.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr delta_type volume() const noexcept
		{
			return boxes::volume(base::extents);
		}

		/// \brief	Calculates the mass of this box if it had a given density.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr delta_type MUU_VECTORCALL mass(delta_type density) const noexcept
		{
			return boxes::mass(base::extents, density);
		}

		/// \brief	Calculates the density of this box if it had a given mass.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr delta_type MUU_VECTORCALL density(delta_type mass) const noexcept
		{
			return boxes::density(base::extents, mass);
		}


	#endif // geometric properties

	#if 1 // equality --------------------------------------------------------------------------------------------------

		/// \brief		Returns true if two oriented bounding boxes are exactly equal.
		/// 
		/// \remarks	This is a componentwise exact equality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point oriented bounding boxes, use #approx_equal().
		template <typename T>
		[[nodiscard]]
		MUU_ATTR(pure)
		friend
		constexpr bool operator == (const oriented_bounding_box& lhs, const oriented_bounding_box<T>& rhs) noexcept
		{
			return lhs.center == rhs.center
				&& lhs.extents == rhs.extents
				&& lhs.axes == rhs.axes;
		}

		/// \brief	Returns true if two oriented bounding boxes are not exactly equal.
		/// 
		/// \remarks	This is a componentwise exact inequality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point oriented bounding boxes, use #approx_equal().
		template <typename T>
		[[nodiscard]]
		MUU_ATTR(pure)
		friend
		constexpr bool operator != (const oriented_bounding_box& lhs, const oriented_bounding_box<T>& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		/// \brief	Returns true if all the scalar components of an oriented bounding box are exactly zero.
		/// 
		/// \remarks	This is a componentwise exact equality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point oriented bounding boxes, use #approx_zero().
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool zero(const oriented_bounding_box& bb) noexcept
		{
			return vector_type::zero(bb.center)
				&& vector_type::zero(bb.extents)
				&& axes_type::zero(bb.axes);
		}

		/// \brief	Returns true if all the scalar components of the oriented bounding box are exactly zero.
		/// 
		/// \remarks	This is a componentwise exact equality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point oriented bounding boxes, use #approx_zero().
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool zero() const noexcept
		{
			return zero(*this);
		}

		/// \brief	Returns true if an oriented bounding box has exactly zero volume.
		/// 
		/// \remarks	This is a componentwise exact empty check;
		/// 			if you want an epsilon-based "near-enough" for floating-point oriented bounding boxes, use #approx_empty().
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool empty(const oriented_bounding_box& bb) noexcept
		{
			return vector_type::zero(bb.extents);
		}

		/// \brief	Returns true if the oriented bounding box has exactly zero volume.
		/// 
		/// \remarks	This is a componentwise exact equality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point oriented bounding boxes, use #approx_empty().
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool empty() const noexcept
		{
			return vector_type::zero(base::extents);
		}

		/// \brief	Returns true if any of the scalar components of an oriented bounding box are infinity or NaN.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool infinity_or_nan(const oriented_bounding_box& bb) noexcept
		{
			if constexpr (is_floating_point<scalar_type>)
				return vector_type::infinity_or_nan(bb.center)
				|| vector_type::infinity_or_nan(bb.extents)
				|| axes_type::infinity_or_nan(bb.axes);
			else
				return false;
		}

		/// \brief	Returns true if any of the scalar components of the oriented bounding box are infinity or NaN.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool infinity_or_nan() const noexcept
		{
			if constexpr (is_floating_point<scalar_type>)
				return infinity_or_nan(*this);
			else
				return false;
		}

	#endif // equality

	#if 1 // approx_equal ----------------------------------------------------------------------------------------------

		/// \brief	Returns true if two oriented bounding boxes are approximately equal.
		/// 
		/// \note		This function is only available when at least one of #scalar_type and `T` is a floating-point type.
		MUU_CONSTRAINED_TEMPLATE(
			(any_floating_point<scalar_type, T>),
			typename T
		)
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool approx_equal(
			const oriented_bounding_box& bb1,
			const oriented_bounding_box<T>& bb2,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>
		) noexcept
		{
			return vector_type::approx_equal(bb1.center, bb2.center, epsilon)
				&& vector_type::approx_equal(bb1.extents, bb2.extents, epsilon)
				&& axes_type::approx_equal(bb1.axes, bb2.axes, epsilon);
		}

		/// \brief	Returns true if the oriented bounding box is approximately equal to another.
		/// 
		/// \note		This function is only available when at least one of #scalar_type and `T` is a floating-point type.
		MUU_CONSTRAINED_TEMPLATE(
			(any_floating_point<scalar_type, T>),
			typename T
		)
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool approx_equal(
			const oriented_bounding_box<T>& bb,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>
		) const noexcept
		{
			return approx_equal(*this, bb, epsilon);
		}

		/// \brief	Returns true if all the scalar components in an oriented bounding box are approximately equal to zero.
		/// 
		/// \note		This function is only available when #scalar_type is a floating-point type.
		MUU_LEGACY_REQUIRES(is_floating_point<T>, typename T = scalar_type)
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool approx_zero(const oriented_bounding_box& bb, scalar_type epsilon = default_epsilon<scalar_type>) noexcept
			MUU_REQUIRES(is_floating_point<scalar_type>)
		{
			return vector_type::approx_zero(bb.center, epsilon)
				&& vector_type::approx_zero(bb.extents, epsilon)
				&& axes_type::approx_zero(bb.axes, epsilon);
		}

		/// \brief	Returns true if all the scalar components in the oriented bounding box are approximately equal to zero.
		/// 
		/// \note		This function is only available when #scalar_type is a floating-point type.
		MUU_LEGACY_REQUIRES(is_floating_point<T>, typename T = scalar_type)
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool MUU_VECTORCALL approx_zero(scalar_type epsilon = default_epsilon<scalar_type>) const noexcept
			MUU_REQUIRES(is_floating_point<scalar_type>)
		{
			return approx_zero(*this, epsilon);
		}

		/// \brief	Returns true if an oriented bounding box has approximately zero volume.
		/// 
		/// \note		This function is only available when #scalar_type is a floating-point type.
		MUU_LEGACY_REQUIRES(is_floating_point<T>, typename T = scalar_type)
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool approx_empty(const oriented_bounding_box& bb, scalar_type epsilon = default_epsilon<scalar_type>) noexcept
			MUU_REQUIRES(is_floating_point<scalar_type>)
		{
			return vector_type::approx_zero(bb.extents, epsilon);
		}

		/// \brief	Returns true if the oriented bounding box has approximately zero volume.
		/// 
		/// \note		This function is only available when #scalar_type is a floating-point type.
		MUU_LEGACY_REQUIRES(is_floating_point<T>, typename T = scalar_type)
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool MUU_VECTORCALL approx_empty(scalar_type epsilon = default_epsilon<scalar_type>) const noexcept
			MUU_REQUIRES(is_floating_point<scalar_type>)
		{
			return vector_type::approx_zero(base::extents, epsilon);
		}

	#endif // approx_equal

	#if 1 // corners ---------------------------------------------------------------------------------------------------

		/// \brief	Returns a specific corner of an oriented bounding box.
		template <box_corners Corner>
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr vector_type corner(const oriented_bounding_box& bb) noexcept
		{
			return boxes::corner<Corner>(bb.center, bb.extents);
		}

		/// \brief	Returns a specific corner of the oriented bounding box.
		template <box_corners Corner>
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr vector_type corner() const noexcept
		{
			return boxes::corner<Corner>(base::center, base::extents);
		}

		/// \brief	Returns a specific corner of an oriented bounding box.
		[[nodiscard]]
		MUU_ATTR_NDEBUG(pure)
		static constexpr vector_type corner(const oriented_bounding_box& bb, box_corners which) noexcept
		{
			return boxes::corner(bb.center, bb.extents, which);
		}

		/// \brief	Returns a specific corner of the oriented bounding box.
		[[nodiscard]]
		MUU_ATTR_NDEBUG(pure)
		constexpr vector_type corner(box_corners which) const noexcept
		{
			return boxes::corner(base::center, base::extents, which);
		}

		/// \brief	Returns the 'min' corner of an oriented bounding box.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr vector_type min_corner(const oriented_bounding_box& bb) noexcept
		{
			return boxes::corner<box_corners::min>(bb.center, bb.extents);
		}

		/// \brief	Returns the 'min' corner of the oriented bounding box.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr vector_type min_corner() const noexcept
		{
			return boxes::corner<box_corners::min>(base::center, base::extents);
		}

		/// \brief	Returns the 'max' corner of an oriented bounding box.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr vector_type max_corner(const oriented_bounding_box& bb) noexcept
		{
			return boxes::corner<box_corners::max>(bb.center, bb.extents);
		}

		/// \brief	Returns the 'max' corner of the oriented bounding box.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr vector_type max_corner() const noexcept
		{
			return boxes::corner<box_corners::max>(base::center, base::extents);
		}

	#endif // corners

	#if 1 // translation -----------------------------------------------------------------------------------------------

		/// \brief	Translates an oriented bounding box.
		///
		/// \param	bb		The oriented bounding box to translate.
		/// \param	offset	An offset to add to the box's center position.
		///
		/// \returns	A copy of the input box translated by the given offset.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr oriented_bounding_box MUU_VECTORCALL translate(const oriented_bounding_box& bb, MUU_RO_VEC offset) noexcept
		{
			return oriented_bounding_box{ bb.center + offset, bb.extents, bb.axes };
		}

		/// \brief	Translates the oriented bounding box (in-place).
		///
		/// \param	offset	An offset to add to the box's center position.
		///
		/// \return	A reference to the box_.
		constexpr oriented_bounding_box& MUU_VECTORCALL translate(MUU_RO_VEC offset) noexcept
		{
			base::center += offset;
			return *this;
		}

	#endif // translation

	#if 1 // scaling -----------------------------------------------------------------------------------------------

		/// \brief	Scales an oriented bounding box.
		///
		/// \param	bb		The oriented bounding box to scale.
		/// \param	scale_	The amount to scale the box extents by on each axis.
		///
		/// \returns	A copy of the input box scaled by the given amounts.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr oriented_bounding_box MUU_VECTORCALL scale(const oriented_bounding_box& bb, MUU_RO_VEC scale_) noexcept
		{
			return oriented_bounding_box{ bb.center, bb.extents * scale_, bb.axes };
		}

		/// \brief	Scales the oriented bounding box (in-place).
		///
		/// \param	scale_	The amount to scale the box extents by on each axis.
		///
		/// \return	A reference to the box_.
		constexpr oriented_bounding_box& MUU_VECTORCALL scale(MUU_RO_VEC scale_) noexcept
		{
			base::extents *= scale_;
			return *this;
		}

	#endif // scaling

	};

	/// \cond deduction_guides

	template <
		typename CX, typename CY, typename CZ,
		typename EX, typename EY, typename EZ
	>
	oriented_bounding_box(CX, CY, CZ, EX, EY, EZ) -> oriented_bounding_box<impl::highest_ranked<CX, CY, CZ, EX, EY, EZ>>;

	template <typename C, typename E>
	oriented_bounding_box(vector<C, 3>, vector<E, 3>) -> oriented_bounding_box<impl::highest_ranked<C, E>>;

	MUU_CONSTRAINED_TEMPLATE(
		(all_arithmetic<CX, CY, CZ>),
		typename CX, typename CY, typename CZ, typename E
	)
	oriented_bounding_box(CX, CY, CZ, vector<E, 3>) -> oriented_bounding_box<impl::highest_ranked<CX, CY, CZ, E>>;

	MUU_CONSTRAINED_TEMPLATE(
		(all_arithmetic<EX, EY, EZ>),
		typename C, typename EX, typename EY, typename EZ
	)
	oriented_bounding_box(vector<C, 3>, EX, EY, EZ) -> oriented_bounding_box<impl::highest_ranked<C, EX, EY, EZ>>;

	MUU_CONSTRAINED_TEMPLATE(
		is_arithmetic<E>,
		typename C, typename E
	)
	oriented_bounding_box(vector<C, 3>, E) -> oriented_bounding_box<impl::highest_ranked<C, E>>;

	template <typename E>
	oriented_bounding_box(vector<E, 3>) -> oriented_bounding_box<E>;

	MUU_CONSTRAINED_TEMPLATE(
		is_arithmetic<E>,
		typename E
	)
	oriented_bounding_box(E) -> oriented_bounding_box<std::remove_cv_t<E>>;

	/// \endcond

	MUU_ABI_VERSION_END;
}

namespace std
{
	/// \brief Specialization of std::tuple_size for muu::bounding_box.
	/// \related	muu::bounding_box
	template <typename Scalar>
	struct tuple_size<muu::oriented_bounding_box<Scalar>>
	{
		static constexpr size_t value = 3;
	};

	/// \brief Specialization of std::tuple_element for muu::bounding_box.
	/// \related	muu::bounding_box
	template <size_t I, typename Scalar>
	struct tuple_element<I, muu::oriented_bounding_box<Scalar>>
	{
		static_assert(I < 3);
		using type = std::conditional_t<(I < 2), muu::vector<Scalar, 3>, muu::matrix<Scalar, 3, 3>>;
	};
}

#endif //===============================================================================================================

//======================================================================================================================
// CONSTANTS
#if 1

MUU_PUSH_PRECISE_MATH;

namespace muu
{

}

MUU_POP_PRECISE_MATH;

#endif //===============================================================================================================

//======================================================================================================================
// FREE FUNCTIONS
#if 1

namespace muu
{

}

#endif //===============================================================================================================

#undef MUU_RO_VEC

MUU_PRAGMA_MSVC(pop_macro("min"))
MUU_PRAGMA_MSVC(pop_macro("max"))
MUU_PRAGMA_MSVC(float_control(pop))
MUU_PRAGMA_MSVC(inline_recursion(off))
MUU_POP_WARNINGS;