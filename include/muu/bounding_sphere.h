// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::bounding_sphere.

#include "sat_tester.h"
#include "impl/geometry_common.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(float_control(except, off))

//======================================================================================================================
// BOUNDING SPHERE CLASS
#if 1

namespace muu
{
	/// \brief A bounding sphere.
	/// \ingroup math
	///
	/// \tparam	Scalar      The bounding sphere's scalar component type. Must be a floating-point type.
	///
	/// \see [Bounding Sphere](https://en.wikipedia.org/wiki/Bounding_sphere)
	template <typename Scalar>
	struct MUU_TRIVIAL_ABI bounding_sphere //
		MUU_HIDDEN_BASE(impl::storage_base<bounding_sphere<Scalar>>)
	{
		static_assert(!std::is_reference_v<Scalar>, "Bounding sphere scalar type cannot be a reference");
		static_assert(!is_cv<Scalar>, "Bounding sphere scalar type cannot be const- or volatile-qualified");
		static_assert(std::is_trivially_constructible_v<Scalar>	  //
						  && std::is_trivially_copyable_v<Scalar> //
						  && std::is_trivially_destructible_v<Scalar>,
					  "Bounding sphere scalar type must be trivially constructible, copyable and destructible");
		static_assert(is_signed<Scalar>, "Bounding sphere scalar type must be signed");
		static_assert(is_floating_point<Scalar>, "Bounding sphere scalar type must be a floating-point type");

		/// \brief The bounding sphere's scalar type.
		using scalar_type = Scalar;

		/// \brief The three-dimensional #muu::vector with the same #scalar_type as the bounding sphere.
		using vector_type = vector<scalar_type, 3>;

		/// \brief Compile-time bounding sphere constants.
		using constants = muu::constants<bounding_sphere>;

	  private:
		/// \cond
		using base = impl::storage_base<bounding_sphere<scalar_type>>;
		static_assert(sizeof(base) == (sizeof(vector_type) + sizeof(scalar_type)),
					  "Bounding spheres should not have padding");

		using spheres	 = impl::spheres_common<scalar_type>;
		using boxes		 = impl::boxes_common<scalar_type>;
		using aabbs		 = impl::aabb_common<scalar_type>;
		using triangles	 = impl::triangles_common<scalar_type>;
		using sat_tester = muu::sat_tester<scalar_type, 3>;

		using promoted_scalar					 = promote_if_small_float<scalar_type>;
		using promoted_vec3						 = vector<promoted_scalar, 3>;
		using promoted_sphere					 = bounding_sphere<promoted_scalar>;
		static constexpr bool requires_promotion = impl::is_small_float_<scalar_type>;

		/// \endcond

	  public:
	#if MUU_DOXYGEN

		/// \brief	The center of the sphere.
		vector_type center;

		/// \brief	The radius of the sphere.
		scalar_type radius;

	#endif // DOXYGEN

	#if 1 // constructors ----------------------------------------------------------------------------------------------

		/// \brief Default constructor. Values are not initialized.
		bounding_sphere() noexcept = default;

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr bounding_sphere(const bounding_sphere&) noexcept = default;

		/// \brief Copy-assigment operator.
		constexpr bounding_sphere& operator=(const bounding_sphere&) noexcept = default;

		/// \brief	Constructs a bounding sphere from center and radius values.
		///
		/// \param	cen		The center point.
		/// \param	rad		The radius.
		MUU_NODISCARD_CTOR
		constexpr bounding_sphere(const vector_type& cen, scalar_type rad) noexcept //
			: base{ cen, rad }
		{}

		/// \brief	Constructs a bounding sphere from center and radius values.
		///
		/// \param	cen_x	The X component of the center point.
		/// \param	cen_y	The Y component of the center point.
		/// \param	cen_z	The Z component of the center point.
		/// \param	rad		The radius.
		MUU_NODISCARD_CTOR
		constexpr bounding_sphere(scalar_type cen_x, scalar_type cen_y, scalar_type cen_z, scalar_type rad) noexcept
			: base{ { cen_x, cen_y, cen_z }, rad }
		{}

		/// \brief	Constructs a bounding sphere at the origin.
		///
		/// \param	rad		The radius.
		MUU_NODISCARD_CTOR
		explicit constexpr bounding_sphere(scalar_type rad) noexcept //
			: base{ vector_type::constants::zero, rad }
		{}

		/// \brief	Converting constructor.
		template <typename S>
		MUU_NODISCARD_CTOR
		explicit constexpr bounding_sphere(const bounding_sphere<S>& bs) noexcept //
			: base{ vector_type{ bs.center }, static_cast<scalar_type>(bs.radius) }
		{}

		/// \brief Constructs a bounding sphere from an implicitly bit-castable type.
		///
		/// \tparam T	A bit-castable type.
		///
		/// \see muu::allow_implicit_bit_cast
		MUU_CONSTRAINED_TEMPLATE((allow_implicit_bit_cast<T, bounding_sphere>), typename T)
		MUU_NODISCARD_CTOR
		/*implicit*/
		constexpr bounding_sphere(const T& blittable) noexcept //
			: base{ muu::bit_cast<base>(blittable) }
		{
			static_assert(sizeof(T) == sizeof(base), "Bit-castable types must be the same size");
			static_assert(std::is_trivially_copyable_v<T>, "Bit-castable types must be trivially-copyable");
		}

	#endif // constructors

	#if 1 // scalar accessors ------------------------------------------------------------------------------------------
		/// \name Scalar accessors
		/// @{

		/// \brief Returns a pointer to the first scalar component in the bounding sphere.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type* data() noexcept
		{
			return base::center.data();
		}

		/// \brief Returns a pointer to the first scalar component in the bounding sphere.
		MUU_PURE_INLINE_GETTER
		constexpr const scalar_type* data() const noexcept
		{
			return base::center.data();
		}

			/// @}
	#endif // scalar accessors

	#if 1 // geometric properties --------------------------------------------------------------------------------------
		/// \name Geometric properties
		/// @{

		/// \brief	Calculates the diameter of the sphere.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type diameter() const noexcept
		{
			return base::radius * muu::constants<scalar_type>::two;
		}

		/// \brief	Calculates the volume of the sphere.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type volume() const noexcept
		{
			if constexpr (requires_promotion)
			{
				return static_cast<scalar_type>(impl::spheres_common<promoted_scalar>::volume(base::radius));
			}
			else
				return spheres::volume(base::radius);
		}

		/// \brief	Calculates the mass of this sphere if it had a given density.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type MUU_VECTORCALL mass(scalar_type density) const noexcept
		{
			return density * volume();
		}

		/// \brief	Calculates the density of this sphere if it had a given mass.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type MUU_VECTORCALL density(scalar_type mass) const noexcept
		{
			return mass / volume();
		}

			/// @}
	#endif // geometric properties

	#if 1 // equality (exact) ------------------------------------------------------------------------------------------
		  /// \name Equality (exact)
		  /// @{

		/// \brief		Returns true if two bounding spheres are exactly equal.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		template <typename T>
		MUU_PURE_GETTER
		friend constexpr bool MUU_VECTORCALL operator==(MUU_VPARAM(bounding_sphere) lhs,
														const bounding_sphere<T>& rhs) noexcept
		{
			return lhs.center == rhs.center && lhs.radius == rhs.radius;
		}

		/// \brief	Returns true if two bounding spheres are not exactly equal.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		template <typename T>
		MUU_PURE_GETTER
		friend constexpr bool MUU_VECTORCALL operator!=(MUU_VPARAM(bounding_sphere) lhs,
														const bounding_sphere<T>& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		/// \brief	Returns true if all the scalar components of a bounding sphere are exactly zero.
		///
		/// \remarks	This is an exact check;
		///				use #approx_zero() if you want an epsilon-based "near-enough" check.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL zero(MUU_VPARAM(bounding_sphere) bs) noexcept
		{
			return vector_type::zero(bs.center) && bs.radius == scalar_type{};
		}

		/// \brief	Returns true if all the scalar components of the bounding sphere are exactly zero.
		///
		/// \remarks	This is an exact check;
		///				use #approx_zero() if you want an epsilon-based "near-enough" check.
		MUU_PURE_INLINE_GETTER
		constexpr bool zero() const noexcept
		{
			return zero(*this);
		}

		/// \brief	Returns true if a bounding sphere has exactly zero volume.
		///
		/// \remarks	This is an exact check;
		///				use #approx_empty() if you want an epsilon-based "near-enough" check.
		MUU_PURE_INLINE_GETTER
		static constexpr bool MUU_VECTORCALL empty(MUU_VPARAM(bounding_sphere) bs) noexcept
		{
			return bs.radius == scalar_type{};
		}

		/// \brief	Returns true if the bounding sphere has exactly zero volume.
		///
		/// \remarks	This is an exact check;
		///				use #approx_empty() if you want an epsilon-based "near-enough" check.
		MUU_PURE_INLINE_GETTER
		constexpr bool empty() const noexcept
		{
			return base::radius == scalar_type{};
		}

		/// \brief	Returns true if any of the scalar components of a bounding sphere are infinity or NaN.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL infinity_or_nan(MUU_VPARAM(bounding_sphere) bs) noexcept
		{
			return vector_type::infinity_or_nan(bs.center) || muu::infinity_or_nan(bs.radius);
		}

		/// \brief	Returns true if any of the scalar components of the bounding sphere are infinity or NaN.
		MUU_PURE_INLINE_GETTER
		constexpr bool infinity_or_nan() const noexcept
		{
			return infinity_or_nan(*this);
		}

		/// \brief	Returns true if a sphere is degenerate (i.e. its radius is less than or equal to zero).
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL degenerate(MUU_VPARAM(bounding_sphere) bs) noexcept
		{
			return bs.radius <= scalar_type{};
		}

		/// \brief	Returns true if the sphere is degenerate (i.e. its radius is less than or equal to zero).
		MUU_PURE_INLINE_GETTER
		constexpr bool degenerate() const noexcept
		{
			return degenerate(*this);
		}

			/// @}
	#endif // equality (exact)

	#if 1 // equality (approx) -----------------------------------------------------------------------------------------
		  /// \name Equality (approximate)
		  /// @{

		/// \brief	Returns true if two bounding spheres are approximately equal.
		template <typename T>
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_equal(
			MUU_VPARAM(bounding_sphere) bs1,
			const bounding_sphere<T>& bs2,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) noexcept
		{
			return vector_type::approx_equal(bs1.center, bs2.center, epsilon)
				&& muu::approx_equal(bs1.radius, bs2.radius, epsilon);
		}

		/// \brief	Returns true if the bounding sphere is approximately equal to another.
		template <typename T>
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL approx_equal(
			const bounding_sphere<T>& bs,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) const noexcept
		{
			return approx_equal(*this, bs, epsilon);
		}

		/// \brief	Returns true if all the scalar components in a bounding sphere are approximately equal to zero.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_zero(MUU_VPARAM(bounding_sphere) bs,
														 scalar_type epsilon = default_epsilon<scalar_type>) noexcept
		{
			return vector_type::approx_zero(bs.center, epsilon) && vector_type::approx_zero(bs.radius, epsilon);
		}

		/// \brief	Returns true if all the scalar components in the bounding sphere are approximately equal to zero.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL approx_zero(scalar_type epsilon = default_epsilon<scalar_type>) const noexcept
		{
			return approx_zero(*this, epsilon);
		}

		/// \brief	Returns true if a bounding sphere has approximately zero volume.
		MUU_PURE_INLINE_GETTER
		static constexpr bool MUU_VECTORCALL approx_empty(MUU_VPARAM(bounding_sphere) bs,
														  scalar_type epsilon = default_epsilon<scalar_type>) noexcept
		{
			return muu::approx_zero(bs.radius, epsilon);
		}

		/// \brief	Returns true if the bounding sphere has approximately zero volume.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL approx_empty(scalar_type epsilon = default_epsilon<scalar_type>) const noexcept
		{
			return muu::approx_zero(base::radius, epsilon);
		}

			/// @}
	#endif // equality (approx)

	#if 1 // translation -------------------------------------------------------------------
		/// \name Translation
		/// @{

		/// \brief	Translates a bounding sphere.
		///
		/// \param	bs		The bounding sphere to translate.
		/// \param	offset	An offset to add to the sphere's center position.
		///
		/// \returns	A copy of the input sphere translated by the given offset.
		MUU_PURE_GETTER
		static constexpr bounding_sphere MUU_VECTORCALL translate(MUU_VPARAM(bounding_sphere) bs,
																  MUU_VPARAM(vector_type) offset) noexcept
		{
			return bounding_sphere{ bs.center + offset, bs.radius };
		}

		/// \brief	Translates the bounding sphere (in-place).
		///
		/// \param	offset	An offset to add to the sphere's center position.
		///
		/// \return	A reference to the sphere.
		constexpr bounding_sphere& MUU_VECTORCALL translate(MUU_VPARAM(vector_type) offset) noexcept
		{
			base::center += offset;
			return *this;
		}

			/// @}
	#endif // translation

	#if 1 // scaling -------------------------------------------------------------------
		  /// \name Scaling
		  /// @{

		/// \brief	Scales a bounding sphere.
		///
		/// \param	bs		The bounding sphere to scale.
		/// \param	scale_	The amount to scale the sphere radius by.
		///
		/// \returns	A copy of the input sphere scaled by the given amount.
		MUU_PURE_GETTER
		static constexpr bounding_sphere MUU_VECTORCALL scale(MUU_VPARAM(bounding_sphere) bs,
															  scalar_type scale_) noexcept
		{
			return bounding_sphere{ bs.center, bs.radius * scale_ };
		}

		/// \brief	Scales the bounding sphere (in-place).
		///
		/// \param	scale_	The amount to scale the sphere radius by.
		///
		/// \return	A reference to the sphere.
		constexpr bounding_sphere& MUU_VECTORCALL scale(scalar_type scale_) noexcept
		{
			base::radius *= scale_;
			return *this;
		}

			/// @}
	#endif // scaling

	#if 1 // containment ------------------------------------------------------------------------------
		  /// \name Containment
		  /// @{

		/// \brief	Returns true if a bounding sphere contains a point.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(bounding_sphere) bs,
													  MUU_VPARAM(vector_type) point) noexcept
		{
			return vector_type::distance_squared(bs.center, point) <= (bs.radius * bs.radius);
		}

		/// \brief	Returns true if the bounding sphere contains a point.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(vector_type) point) const noexcept
		{
			return contains(*this, point);
		}

			/// @}
	#endif // containment

	#if 1 // intersection ------------------------------------------------------------------------------
		  /// \name Intersection
		  /// @{

		//--------------------------------
		// sphere x sphere
		//--------------------------------

		/// \brief	Returns true if two bounding spheres intersect.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(bounding_sphere) bs1,
														MUU_VPARAM(bounding_sphere) bs2) noexcept
		{
			return vector_type::distance_squared(bs1.center, bs2.center)
				<= ((bs1.radius * bs1.radius) + (bs2.radius * bs2.radius));
		}

		/// \brief	Returns true if two bounding spheres intersect.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(bounding_sphere) bs) const noexcept
		{
			return intersects(*this, bs);
		}

			/// @}
	#endif // intersection

	#if 1 // misc ---------------------------------------------------------------------------------------------------

		/// \brief Writes a bounding_sphere out to a text stream.
		template <typename Char, typename Traits>
		friend std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os,
															const bounding_sphere& bs)
		{
			const impl::compound_vector_elem<Scalar> elems[]{ { &bs.center.x, 3 }, //
															  { &bs.radius, 1 } };
			impl::print_compound_vector(os, elems);
			return os;
		}

	#endif // misc
	};

	/// \cond

	MUU_CONSTRAINED_TEMPLATE((all_arithmetic<C, R>), typename C, typename R)
	bounding_sphere(vector<C, 3>, R)->bounding_sphere<impl::highest_ranked<C, R>>;

	MUU_CONSTRAINED_TEMPLATE((all_arithmetic<CX, CY, CZ>), typename CX, typename CY, typename CZ, typename R)
	bounding_sphere(CX, CY, CZ, R)->bounding_sphere<impl::highest_ranked<CX, CY, CZ, R>>;

	MUU_CONSTRAINED_TEMPLATE(is_arithmetic<R>, typename R)
	bounding_sphere(R)->bounding_sphere<std::remove_cv_t<R>>;

	/// \endcond
}

namespace std
{
	/// \brief Specialization of std::tuple_size for muu::bounding_sphere.
	template <typename Scalar>
	struct tuple_size<muu::bounding_sphere<Scalar>>
	{
		static constexpr size_t value = 2;
	};

	/// \brief Specialization of std::tuple_element for muu::bounding_sphere.
	template <size_t I, typename Scalar>
	struct tuple_element<I, muu::bounding_sphere<Scalar>>
	{
		static_assert(I < 2);
		using type = std::conditional_t<I == 1, Scalar, muu::vector<Scalar, 3>>;
	};
}

#endif //===============================================================================================================

//======================================================================================================================
// CONSTANTS
#if 1

MUU_PUSH_PRECISE_MATH;

namespace muu
{

	/// \ingroup	constants
	/// \see		muu::bounding_sphere
	///
	/// \brief		Bounding spehere constants.
	template <typename Scalar>
	struct constants<bounding_sphere<Scalar>>
	{
		using scalars = constants<Scalar>;
		using vectors = constants<vector<Scalar, 3>>;

		/// \brief A bounding sphere with all members initialized to zero.
		static constexpr bounding_sphere<Scalar> zero = { vectors::zero, scalars::zero };

		/// \brief A bounding sphere centered at the origin with radius 1.
		static constexpr bounding_sphere<Scalar> unit = { vectors::zero, scalars::one };
	};

}

MUU_POP_PRECISE_MATH;

#endif //===============================================================================================================

//======================================================================================================================
// FREE FUNCTIONS
#if 1

namespace muu
{
	/// \ingroup		infinity_or_nan
	/// \relatesalso	muu::bounding_sphere
	///
	/// \brief	Returns true if any of the scalar components of a bounding sphere are infinity or NaN.
	template <typename S>
	MUU_PURE_INLINE_GETTER
	constexpr bool infinity_or_nan(const bounding_sphere<S>& bs) noexcept
	{
		return bounding_sphere<S>::infinity_or_nan(bs);
	}

	/// \ingroup		approx_equal
	/// \relatesalso	muu::bounding_sphere
	///
	/// \brief		Returns true if two bounding spheres are approximately equal.
	template <typename S, typename T>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL approx_equal(const bounding_sphere<S>& bs1,
											   const bounding_sphere<T>& bs2,
											   epsilon_type<S, T> epsilon = default_epsilon<S, T>) noexcept
	{
		return bounding_sphere<S>::approx_equal(bs1, bs2, epsilon);
	}

	/// \ingroup		approx_zero
	/// \relatesalso	muu::bounding_sphere
	///
	/// \brief		Returns true if all the scalar components of a bounding sphere are approximately equal to zero.
	template <typename S>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL approx_zero(const bounding_sphere<S>& bs, S epsilon = default_epsilon<S>) noexcept
	{
		return bounding_sphere<S>::approx_zero(bs, epsilon);
	}

	/// \ingroup		degenerate
	/// \relatesalso	muu::bounding_sphere
	///
	/// \brief	Returns true if a bounding sphere is degenerate (i.e. its radius is less than or equal to zero).
	template <typename S>
	MUU_PURE_INLINE_GETTER
	constexpr bool degenerate(const bounding_sphere<S>& bs) noexcept
	{
		return bounding_sphere<S>::degenerate(bs);
	}
}

#endif //===============================================================================================================

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"