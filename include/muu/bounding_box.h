// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::bounding_box.

#pragma once
#include "../muu/vector.h"

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

//======================================================================================================================
// IMPLEMENTATION DETAILS
#if 1

/// \cond
namespace muu::impl
{
	MUU_ABI_VERSION_START(0);

	template <typename Scalar>
	struct MUU_TRIVIAL_ABI bounding_box_
	{
		vector<Scalar, 3> center;
		vector<Scalar, 3> extents;
	};

	MUU_ABI_VERSION_END;

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
}

namespace muu
{
	template <typename From, typename Scalar>
	inline constexpr bool allow_implicit_bit_cast<From, impl::bounding_box_<Scalar>>
		= allow_implicit_bit_cast<From, bounding_box<Scalar>>;
}

/// \endcond

#endif //===============================================================================================================

namespace muu
{
	MUU_ABI_VERSION_START(0);

	/// \brief An axis-aligned bounding box.
	/// \ingroup math
	/// 
	/// \tparam	Scalar      The type of the bounding box's scalar components.
	/// 
	/// \see [Axis-aligned bounding box](https://en.wikipedia.org/w/index.php?title=Axis-aligned_bounding_box)
	template <typename Scalar>
	struct MUU_TRIVIAL_ABI bounding_box
		#ifndef DOXYGEN
		: impl::bounding_box_<Scalar>
		#endif
	{
		static_assert(
			!std::is_reference_v<Scalar>,
			"Bounding box scalar type cannot be a reference"
		);
		static_assert(
			!std::is_const_v<Scalar> && !std::is_volatile_v<Scalar>,
			"Bounding box scalar type cannot be const- or volatile-qualified"
		);
		static_assert(
			std::is_trivially_constructible_v<Scalar>
			&& std::is_trivially_copyable_v<Scalar>
			&& std::is_trivially_destructible_v<Scalar>,
			"Bounding box scalar type must be trivially constructible, copyable and destructible"
		);

		/// \brief The bounding box's scalar type.
		using scalar_type = Scalar;

		/// \brief The scalar type used for length, distance, volume, etc. Always floating-point.
		using delta_type = std::conditional_t<is_integral<scalar_type>, double, scalar_type>;

		/// \brief The three-dimensional #muu::vector with the same #scalar_type as the bounding box.
		using vector_type = vector<scalar_type, 3>;

		/// \brief Compile-time constants for this bounding box's #vector_type.
		using vector_constants = muu::constants<vector_type>;

		/// \brief Compile-time bounding box constants.
		using constants = muu::constants<bounding_box>;

	private:

		using base = impl::bounding_box_<Scalar>;

		using intermediate_float = impl::promote_if_small_float<delta_type>;
		static_assert(is_floating_point<delta_type>);
		static_assert(is_floating_point<intermediate_float>);

	public:

		#ifdef DOXYGEN

		/// \brief	The center of the box.
		vector_type center;

		/// \brief	The half-lengths of box (i.e. distances from the center to the sides).
		vector_type extents;

		#endif //DOXYGEN

	#if 1 // constructors ----------------------------------------------------------------------------------------------

		/// \brief Default constructor. Values are not initialized.
		bounding_box() noexcept = default;

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr bounding_box(const bounding_box&) noexcept = default;

		/// \brief Copy-assigment operator.
		constexpr bounding_box& operator = (const bounding_box&) noexcept = default;

		/// \brief	Constructs a bounding box from center and extent values.
		///
		/// \param	cen_x	The X component of the center point.
		/// \param	cen_y	The Y component of the center point.
		/// \param	cen_z	The Z component of the center point.
		/// \param	ext_x	The length of the X extent.
		/// \param	ext_y	The length of the Y extent.
		/// \param	ext_z	The length of the Z extent.
		MUU_NODISCARD_CTOR
		constexpr bounding_box(
			scalar_type cen_x, scalar_type cen_y, scalar_type cen_z,
			scalar_type ext_x, scalar_type ext_y, scalar_type ext_z
		) noexcept
			: base{ { cen_x, cen_y, cen_z }, { ext_x, ext_y, ext_z } }
		{ }

		/// \brief	Constructs a bounding box from center and extent values.
		///
		/// \param	cen		The center point.
		/// \param	ext_x	The length of the X extent.
		/// \param	ext_y	The length of the Y extent.
		/// \param	ext_z	The length of the Z extent.
		MUU_NODISCARD_CTOR
		constexpr bounding_box(const vector_type& cen, scalar_type ext_x, scalar_type ext_y, scalar_type ext_z) noexcept
			: base{ { cen }, { ext_x, ext_y, ext_z } }
		{ }

		/// \brief	Constructs a bounding box from center and extent values.
		///
		/// \param	cen_x	The X component of the center point.
		/// \param	cen_y	The Y component of the center point.
		/// \param	cen_z	The Z component of the center point.
		/// \param	ext		The extents.
		MUU_NODISCARD_CTOR
		constexpr bounding_box(scalar_type cen_x, scalar_type cen_y, scalar_type cen_z, const vector_type& ext) noexcept
			: base { { cen_x, cen_y, cen_z }, ext }
		{ }

		/// \brief	Constructs a uniformly-sized bounding box.
		///
		/// \param	cen		The center point.
		/// \param	ext		The length of all three extents.
		MUU_NODISCARD_CTOR
		constexpr bounding_box(const vector_type& cen, scalar_type ext) noexcept
			: base{ { cen }, { ext, ext, ext } }
		{ }

		/// \brief	Constructs a uniformly-sized bounding box at the origin.
		///
		/// \param	ext		The length of all three extents.
		MUU_NODISCARD_CTOR
		explicit
		constexpr bounding_box(scalar_type ext) noexcept
			: base{ { vector_constants::zero }, { ext, ext, ext } }
		{ }

		/// \brief	Constructs a bounding box from center and extent values.
		///
		/// \param	cen		The center point.
		/// \param	ext		The extents.
		MUU_NODISCARD_CTOR
		constexpr bounding_box(const vector_type& cen, const vector_type& ext) noexcept
			: base{ cen, ext }
		{ }

		/// \brief Constructs a bounding box from an implicitly bit-castable type.
		/// 
		/// \tparam T	A bit-castable type.
		/// 
		/// \see muu::allow_implicit_bit_cast
		MUU_CONSTRAINED_TEMPLATE(
			(allow_implicit_bit_cast<T, bounding_box>),
			typename T
		)
		MUU_NODISCARD_CTOR
		/*implicit*/
		constexpr bounding_box(const T& blittable) noexcept
			: base{ bit_cast<base>(blittable) }
		{
			static_assert(
				sizeof(T) == sizeof(base),
				"Bit-castable types must be the same size as the bounding box"
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
			return static_cast<delta_type>(base::extents.x * intermediate_float{ 2 });
		}

		/// \brief	Returns the height of the box (y-axis).
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr delta_type height() const noexcept
		{
			return static_cast<delta_type>(base::extents.y * intermediate_float{ 2 });
		}

		/// \brief	Returns the depth of the box (z-axis).
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr delta_type depth() const noexcept
		{
			return static_cast<delta_type>(base::extents.z * intermediate_float{ 2 });
		}

		/// \brief	Calculates the length of the line connecting the min and max points.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr delta_type diagonal() const noexcept
		{
			return static_cast<delta_type>(vector_type::raw_length(base::extents) * intermediate_float{ 2 });
		}

		/// \brief	Returns the shortest of the box's three extents.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr scalar_type& shortest_extent() noexcept
		{
			return min(base::extents.x, min(base::extents.y, base::extents.z));
		}

		/// \brief	Returns the longest of the box's three extents.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr scalar_type& longest_extent() noexcept
		{
			return max(base::extents.x, max(base::extents.y, base::extents.z));
		}

		/// \brief	Returns the shortest of the box's three extents (const overload).
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr const scalar_type& shortest_extent() const noexcept
		{
			return min(base::extents.x, min(base::extents.y, base::extents.z));
		}

		/// \brief	Returns the longest of the box's three extents (const overload).
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr const scalar_type& longest_extent() const noexcept
		{
			return max(base::extents.x, max(base::extents.y, base::extents.z));
		}

		/// \brief	Returns the length of the shortest of the box's three sides.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr delta_type shortest_side() const noexcept
		{
			return static_cast<delta_type>(shortest_extent() * intermediate_float{ 2 });
		}

		/// \brief	Returns the length of the longest of the box's three sides.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr delta_type longest_side() const noexcept
		{
			return static_cast<delta_type>(longest_extent() * intermediate_float{ 2 });
		}

	private:

		template <typename T = intermediate_float>
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr T raw_volume() const noexcept
		{
			return static_cast<T>(base::extents.x)
				* static_cast<T>(base::extents.y)
				* static_cast<T>(base::extents.z)
				* T { 8 };
		}

	public:

		/// \brief	Calculates the volume of this bounding box.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr delta_type volume() const noexcept
		{
			return static_cast<delta_type>(raw_volume());
		}

		/// \brief	Calculates the mass of this box if it had a given density.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr delta_type MUU_VECTORCALL mass(delta_type density) const noexcept
		{
			return static_cast<delta_type>(density * raw_volume());
		}

		/// \brief	Calculates the density of this box if it had a given mass.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr delta_type MUU_VECTORCALL density(delta_type mass) const noexcept
		{
			return static_cast<delta_type>(mass / raw_volume());
		}


	#endif // geometric properties

	#if 1 // equality --------------------------------------------------------------------------------------------------

		/// \brief		Returns true if two bounding boxes are exactly equal.
		/// 
		/// \remarks	This is a componentwise exact equality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point bounding boxes, use #approx_equal().
		template <typename T>
		[[nodiscard]]
		MUU_ATTR(pure)
		friend
		constexpr bool operator == (const bounding_box& lhs, const bounding_box<T>& rhs) noexcept
		{
			return lhs.center == rhs.center
				&& lhs.extents == rhs.extents;
		}

		/// \brief	Returns true if two bounding boxes are not exactly equal.
		/// 
		/// \remarks	This is a componentwise exact inequality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point bounding boxes, use #approx_equal().
		template <typename T>
		[[nodiscard]]
		MUU_ATTR(pure)
		friend
		constexpr bool operator != (const bounding_box& lhs, const bounding_box<T>& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		/// \brief	Returns true if all the scalar components of a bounding box are exactly zero.
		/// 
		/// \remarks	This is a componentwise exact equality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point bounding boxes, use #approx_zero().
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool zero(const bounding_box& bb) noexcept
		{
			return vector_type::zero(bb.center)
				&& vector_type::zero(bb.extents);
		}

		/// \brief	Returns true if all the scalar components of the bounding box are exactly zero.
		/// 
		/// \remarks	This is a componentwise exact equality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point bounding boxes, use #approx_zero().
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool zero() const noexcept
		{
			return zero(*this);
		}

		/// \brief	Returns true if any of the scalar components of a bounding box are infinity or NaN.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool infinity_or_nan(const bounding_box& bb) noexcept
		{
			if constexpr (is_floating_point<scalar_type>)
				return vector_type::infinity_or_nan(bb.center)
				|| vector_type::infinity_or_nan(bb.extents);
			else
				return false;
		}

		/// \brief	Returns true if any of the scalar components of the bounding box are infinity or NaN.
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

		/// \brief	Returns true if two bounding boxes are approximately equal.
		/// 
		/// \note		This function is only available when at least one of #scalar_type and `T` is a floating-point type.
		MUU_CONSTRAINED_TEMPLATE(
			(any_floating_point<scalar_type, T>),
			typename T
		)
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool approx_equal(
			const bounding_box& bb1,
			const bounding_box<T>& bb2,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>
		) noexcept
		{
			return vector_type::approx_equal(bb1.center, bb2.center, epsilon)
				&& vector_type::approx_equal(bb1.extents, bb2.extents, epsilon);
		}

		/// \brief	Returns true if the bounding box is approximately equal to another.
		/// 
		/// \note		This function is only available when at least one of #scalar_type and `T` is a floating-point type.
		MUU_CONSTRAINED_TEMPLATE(
			(any_floating_point<scalar_type, T>),
			typename T
		)
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool approx_equal(
			const bounding_box<T>& bb,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>
		) const noexcept
		{
			return approx_equal(*this, bb, epsilon);
		}

		/// \brief	Returns true if all the scalar components in a bounding box are approximately equal to zero.
		/// 
		/// \note		This function is only available when #scalar_type is a floating-point type.
		MUU_LEGACY_REQUIRES(is_floating_point<T>, typename T = scalar_type)
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool approx_zero(const bounding_box& bb, scalar_type epsilon = default_epsilon<scalar_type>) noexcept
			MUU_REQUIRES(is_floating_point<scalar_type>)
		{
			return vector_type::approx_zero(bb.center, epsilon)
				&& vector_type::approx_zero(bb.extents, epsilon);
		}

		/// \brief	Returns true if all the scalar components in the bounding box are approximately equal to zero.
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

	#endif // approx_equal

	#if 1 // misc -----------------------------------------------------------------------------------------------------

		/// \brief Writes a vector out to a text stream.
		template <typename Char, typename Traits>
		friend
		std::basic_ostream<Char, Traits>& operator << (std::basic_ostream<Char, Traits>& os, const bounding_box& bb)
		{
			impl::print_compound_vector(os,
				&bb.center.x, 3_sz, true,
				&bb.extents.x, 3_sz, true
			);
			return os;
		}

	#endif // misc

	};

	/// \cond deduction_guides

	template <
		typename CX, typename CY, typename CZ,
		typename EX, typename EY, typename EZ
	>
	bounding_box(CX, CY, CZ, EX, EY, EZ) -> bounding_box<impl::highest_ranked<CX, CY, CZ, EX, EY, EZ>>;

	template <typename C, typename E>
	bounding_box(vector<C, 3>, vector<E, 3>) -> bounding_box<impl::highest_ranked<C, E>>;

	MUU_CONSTRAINED_TEMPLATE(
		(all_arithmetic<CX, CY, CZ>),
		typename CX, typename CY, typename CZ, typename E
	)
	bounding_box(CX, CY, CZ, vector<E, 3>) -> bounding_box<impl::highest_ranked<CX, CY, CZ, E>>;

	MUU_CONSTRAINED_TEMPLATE(
		(all_arithmetic<EX, EY, EZ>),
		typename C, typename EX, typename EY, typename EZ
	)
	bounding_box(vector<C, 3>, EX, EY, EZ) -> bounding_box<impl::highest_ranked<C, EX, EY, EZ>>;

	MUU_CONSTRAINED_TEMPLATE(
		is_arithmetic<E>,
		typename C, typename E
	)
	bounding_box(vector<C, 3>, E) -> bounding_box<impl::highest_ranked<C, E>>;

	MUU_CONSTRAINED_TEMPLATE(
		is_arithmetic<E>,
		typename E
	)
	bounding_box(E) -> bounding_box<std::remove_cv_t<E>>;

	/// \endcond

	MUU_ABI_VERSION_END;
}

namespace std
{
	/// \brief Specialization of std::tuple_size for muu::bounding_box.
	/// \related	muu::bounding_box
	template <typename Scalar>
	struct tuple_size<muu::bounding_box<Scalar>>
	{
		static constexpr size_t value = 2;
	};

	/// \brief Specialization of std::tuple_element for muu::bounding_box.
	/// \related	muu::bounding_box
	template <size_t I, typename Scalar>
	struct tuple_element<I, muu::bounding_box<Scalar>>
	{
		static_assert(I < 2);
		using type = muu::vector<Scalar, 3>;
	};
}

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

MUU_PRAGMA_MSVC(pop_macro("min"))
MUU_PRAGMA_MSVC(pop_macro("max"))
MUU_PRAGMA_MSVC(float_control(pop))
MUU_PRAGMA_MSVC(inline_recursion(off))
MUU_POP_WARNINGS;
