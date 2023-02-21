// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#ifndef MUU_PLANE_H
#define MUU_PLANE_H

/// \file
/// \brief  Contains the definition of muu::plane.

#include "impl/geometry_common.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_DISABLE_SHADOW_WARNINGS;
MUU_PRAGMA_MSVC(float_control(except, off))

//======================================================================================================================
// PLANE CLASS
#if 1

namespace muu
{
	/// \brief A plane.
	/// \ingroup math
	///
	/// \tparam	Scalar      The plane's scalar component type. Must be a floating-point type.
	///
	/// \see [Plane](https://en.wikipedia.org/wiki/Plane_%28geometry%29)
	template <typename Scalar>
	struct MUU_TRIVIAL_ABI plane //
		MUU_HIDDEN_BASE(impl::storage_base<plane<Scalar>>)
	{
		static_assert(!std::is_reference_v<Scalar>, "Plane scalar type cannot be a reference");
		static_assert(!is_cv<Scalar>, "Plane scalar type cannot be const- or volatile-qualified");
		static_assert(std::is_trivially_constructible_v<Scalar>	  //
						  && std::is_trivially_copyable_v<Scalar> //
						  && std::is_trivially_destructible_v<Scalar>,
					  "Plane scalar type must be trivially constructible, copyable and destructible");
		static_assert(is_signed<Scalar>, "Plane scalar type must be signed");
		static_assert(is_floating_point<Scalar>, "Plane scalar type must be a floating-point type");

		/// \brief The plane's scalar type.
		using scalar_type = Scalar;

		/// \brief The three-dimensional #muu::vector with the same #scalar_type as the plane.
		using vector_type = vector<scalar_type, 3>;

		/// \brief Compile-time plane constants.
		using constants = muu::constants<plane>;

	  private:
		using base = impl::storage_base<plane<Scalar>>;
		static_assert(sizeof(base) == (sizeof(scalar_type) * 4), "Planes should not have padding");

		using planes		   = impl::planes_common<Scalar>;
		using triangles		   = impl::triangles_common<Scalar>;
		using aabbs			   = impl::aabbs_common<Scalar>;
		using scalar_constants = muu::constants<scalar_type>;

		using promoted_scalar					 = promote_if_small_float<scalar_type>;
		using promoted_vec						 = vector<promoted_scalar, 3>;
		using promoted_plane					 = plane<promoted_scalar>;
		static constexpr bool requires_promotion = impl::is_small_float_<scalar_type>;

	  public:
	#if MUU_DOXYGEN

		/// \brief	The plane's normal direction.
		/// \attention	All plane math assumes this normal to be unit-length;
		///				if you assign to this member directly and do not ensure unit-length, there be dragons!
		vector_type normal;

		/// \brief	The `d` term of the plane equation.
		scalar_type d;

	#endif // DOXYGEN

	#if 1 // constructors ----------------------------------------------------------------------------------------------

		/// \brief Default constructor. Values are not initialized.
		plane() noexcept = default;

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr plane(const plane&) noexcept = default;

		/// \brief Copy-assigment operator.
		constexpr plane& operator=(const plane&) noexcept = default;

		/// \brief	Constructs a plane from `n` and `d` terms.
		MUU_NODISCARD_CTOR
		constexpr plane(const vector_type& n, scalar_type d) noexcept //
			: base{ n, d }
		{}

		/// \brief	Constructs a plane from a position and normal direction.
		MUU_NODISCARD_CTOR
		constexpr plane(const vector_type& position, const vector_type& direction) noexcept //
			: base{ direction, planes::d_term(position, direction) }
		{}

		/// \brief	Constructs a plane from a triangle.
		MUU_PURE_INLINE_GETTER
		static constexpr plane MUU_VECTORCALL from_triangle(MUU_VPARAM(vector_type) p0,
															MUU_VPARAM(vector_type) p1,
															MUU_VPARAM(vector_type) p2) noexcept
		{
			return plane{ p0, triangles::normal(p0, p1, p2) };
		}

		/// \brief	Constructs a plane from a triangle.
		MUU_NODISCARD_CTOR
		constexpr plane(const vector_type& p0, const vector_type& p1, const vector_type& p2) noexcept //
			: plane{ from_triangle(p0, p1, p2) }
		{}

		/// \brief	Constructs a plane from a triangle.
		MUU_PURE_INLINE_GETTER
		static constexpr plane MUU_VECTORCALL from_triangle(MUU_VPARAM(triangle<scalar_type>) tri) noexcept;

		/// \brief	Constructs a plane from a triangle.
		MUU_NODISCARD_CTOR
		explicit constexpr plane(const triangle<scalar_type>& tri) noexcept //
			: plane{ from_triangle(tri) }
		{}

		/// \brief	Converting constructor.
		template <typename S>
		MUU_NODISCARD_CTOR
		explicit constexpr plane(const plane<S>& p) noexcept //
			: base{ vector_type{ p.normal }, static_cast<scalar_type>(p.d) }
		{}

		/// \brief Constructs a plane from an implicitly bit-castable type.
		///
		/// \tparam T	A bit-castable type.
		///
		/// \see muu::allow_implicit_bit_cast
		MUU_CONSTRAINED_TEMPLATE((allow_implicit_bit_cast<T, plane>), typename T)
		MUU_NODISCARD_CTOR
		/*implicit*/
		constexpr plane(const T& obj) noexcept //
			: base{ muu::bit_cast<base>(obj) }
		{
			static_assert(sizeof(T) == sizeof(base), "Bit-castable types must be the same size");
			static_assert(std::is_trivially_copyable_v<T>, "Bit-castable types must be trivially-copyable");
		}

	#endif // constructors

	#if 1 // scalar accessors ------------------------------------------------------------------------------------------
		/// \name Scalar accessors
		/// @{

		/// \brief Returns a pointer to the first scalar component in the plane.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type* data() noexcept
		{
			return base::normal.data();
		}

		/// \brief Returns a pointer to the first scalar component in the plane.
		MUU_PURE_INLINE_GETTER
		constexpr const scalar_type* data() const noexcept
		{
			return base::normal.data();
		}

				/// @}
	#endif // scalar accessors

	#if 1 // equality (exact) ------------------------------------------------------------------------------------------
		/// \name Equality (exact)
		/// @{

		/// \brief		Returns true if two planes are exactly equal.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		template <typename T>
		MUU_PURE_GETTER
		friend constexpr bool MUU_VECTORCALL operator==(MUU_VPARAM(plane) lhs, const plane<T>& rhs) noexcept
		{
			return lhs.normal == rhs.normal && lhs.d == rhs.d;
		}

		/// \brief	Returns true if two planes are not exactly equal.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		template <typename T>
		MUU_PURE_GETTER
		friend constexpr bool MUU_VECTORCALL operator!=(MUU_VPARAM(plane) lhs, const plane<T>& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		/// \brief	Returns true if all the scalar components of a plane are exactly zero.
		///
		/// \remarks	This is an exact check;
		///				use #approx_zero() if you want an epsilon-based "near-enough" check.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL zero(MUU_VPARAM(plane) p) noexcept
		{
			return vector_type::zero(p.normal) && p.d == scalar_constants::zero;
		}

		/// \brief	Returns true if all the scalar components of the plane are exactly zero.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		MUU_PURE_GETTER
		constexpr bool zero() const noexcept
		{
			return zero(*this);
		}

		/// \brief	Returns true if any of the scalar components of a plane are infinity or NaN.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL infinity_or_nan(MUU_VPARAM(plane) p) noexcept
		{
			return vector_type::infinity_or_nan(p.normal) || muu::infinity_or_nan(p.d);
		}

		/// \brief	Returns true if any of the scalar components of the plane are infinity or NaN.
		MUU_PURE_GETTER
		constexpr bool infinity_or_nan() const noexcept
		{
			return infinity_or_nan(*this);
		}

				/// @}
	#endif // equality (exact)

	#if 1 // equality (approx) -----------------------------------------------------------------------------------------
		  /// \name Equality (approximate)
		  /// @{

		/// \brief	Returns true if two planes are approximately equal.
		template <typename T>
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_equal(
			MUU_VPARAM(plane) p1,
			const plane<T>& p2,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) noexcept
		{
			return vector_type::approx_equal(p1.normal, p2.normal, epsilon) && muu::approx_equal(p1.d, p2.d, epsilon);
		}

		/// \brief	Returns true if the plane is approximately equal to another.
		template <typename T>
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL approx_equal(
			const plane<T>& p,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) const noexcept
		{
			return approx_equal(*this, p, epsilon);
		}

		/// \brief	Returns true if all the scalar components in a plane are approximately equal to zero.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_zero(MUU_VPARAM(plane) p,
														 scalar_type epsilon = default_epsilon<scalar_type>) noexcept
		{
			return vector_type::approx_zero(p.normal, epsilon) && muu::approx_zero(p.d, epsilon);
		}

		/// \brief	Returns true if all the scalar components in the plane are approximately equal to zero.
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL approx_zero(scalar_type epsilon = default_epsilon<scalar_type>) const noexcept
		{
			return approx_zero(*this, epsilon);
		}

				/// @}
	#endif // equality (approx)

	#if 1 // normalization --------------------------------------------------------------------------------------------
		/// \name Normalization
		/// @{

		/// \brief		Normalizes a plane.
		///
		/// \param p	The plane to normalize.
		///
		/// \return		A normalized copy of the input plane.
		MUU_PURE_GETTER
		static constexpr plane MUU_VECTORCALL normalize(MUU_VPARAM(plane) p) noexcept
		{
			if constexpr (requires_promotion)
			{
				return plane{ promoted_plane::normalize(promoted_plane{ p }) };
			}
			else
			{
				const auto inv_len = scalar_type{ 1 } / vector_type::length(p.normal);
				return plane{ p.normal * inv_len, p.d * inv_len };
			}
		}

		/// \brief		Normalizes the plane (in-place).
		///
		/// \return		A reference to the plane.
		constexpr plane& normalize() noexcept
		{
			return *this = normalize(*this);
		}

		/// \brief Returns true if a plane is normalized.
		MUU_PURE_INLINE_GETTER
		static constexpr bool MUU_VECTORCALL normalized(MUU_VPARAM(plane) p) noexcept
		{
			return vector_type::normalized(p.normal);
		}

		/// \brief Returns true if the plane is normalized.
		MUU_PURE_INLINE_GETTER
		constexpr bool normalized() const noexcept
		{
			return vector_type::normalized(base::normal);
		}

				/// @}
	#endif // normalization

	#if 1 // distance and projection -----------------------------------------------------------------------------------
		/// \name Distance and projection
		/// @{

		/// \brief	Returns the signed distance of a point from a plane.
		MUU_PURE_INLINE_GETTER
		static constexpr scalar_type MUU_VECTORCALL signed_distance(MUU_VPARAM(plane) p,
																	MUU_VPARAM(vector_type) point) noexcept
		{
			return planes::signed_distance(p.normal, p.d, point);
		}

		/// \brief	Returns the signed distance of a point from the plane.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type MUU_VECTORCALL signed_distance(MUU_VPARAM(vector_type) point) const noexcept
		{
			return planes::signed_distance(base::normal, base::d, point);
		}

		/// \brief	Returns the unsigned distance of a point from a plane.
		MUU_PURE_INLINE_GETTER
		static constexpr scalar_type MUU_VECTORCALL distance(MUU_VPARAM(plane) p,
															 MUU_VPARAM(vector_type) point) noexcept
		{
			return planes::unsigned_distance(p.normal, p.d, point);
		}

		/// \brief	Returns the unsigned distance of a point from the plane.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type MUU_VECTORCALL distance(MUU_VPARAM(vector_type) point) const noexcept
		{
			return planes::unsigned_distance(base::normal, base::d, point);
		}

		/// \brief	Returns the projection of a point onto a plane.
		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL project(MUU_VPARAM(plane) p, MUU_VPARAM(vector_type) point) noexcept
		{
			return planes::project(p.normal, p.d, point);
		}

		/// \brief	Returns the projection of a point onto the plane.
		MUU_PURE_INLINE_GETTER
		constexpr vector_type MUU_VECTORCALL project(MUU_VPARAM(vector_type) point) const noexcept
		{
			return planes::project(base::normal, base::d, point);
		}

		/// \brief	Returns the projection of a line segment onto a plane.
		MUU_PURE_GETTER
		static constexpr line_segment<scalar_type> MUU_VECTORCALL project(MUU_VPARAM(plane) p,
																		  MUU_VPARAM(line_segment<scalar_type>)
																			  seg) noexcept;

		/// \brief	Returns the projection of a line segment onto the plane.
		MUU_PURE_INLINE_GETTER
		constexpr line_segment<scalar_type> MUU_VECTORCALL project(MUU_VPARAM(line_segment<scalar_type>)
																	   seg) const noexcept
		{
			return project(*this, seg);
		}

		/// \brief	Returns the 'origin' (basis point) of a plane.
		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL origin(MUU_VPARAM(plane) p) noexcept
		{
			return planes::origin(p.normal, p.d);
		}

		/// \brief	Returns the 'origin' (basis point) of the plane.
		MUU_PURE_INLINE_GETTER
		constexpr vector_type MUU_VECTORCALL origin() const noexcept
		{
			return planes::origin(base::normal, base::d);
		}

		/// \brief	Returns true if all the given points lie on the same side of a plane.
		/// \attention	For the purposes of this function, for all points to lie on the same side
		///				is for all points to have the same result for the expression
		///				`plane::signed_distance(plane, point) >= 0`.
		MUU_CONSTRAINED_TEMPLATE((sizeof...(T) == 0 || all_convertible_to<vector_type, const T&...>), typename... T)
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL same_side(MUU_VPARAM(plane) p,
													   MUU_VPARAM(vector_type) point1,
													   MUU_VPARAM(vector_type) point2,
													   const T&... pointN) noexcept
		{
			if constexpr (sizeof...(T) > 0)
				return (signed_distance(p, point1) >= scalar_type{}) == (signed_distance(p, point2) >= scalar_type{});
			else
			{
				const auto p0_side = signed_distance(p, point1) >= scalar_type{};
				return ((p0_side == (signed_distance(p, point2) >= scalar_type{})) && ...
						&& (p0_side == (signed_distance(p, pointN) >= scalar_type{})));
			}
		}

		/// \brief	Returns true if all the given points lie on the same side of the plane.
		/// \attention	For the purposes of this function, for all points to lie on the same side
		///				is for all points to have the same result for the expression
		///				`plane.signed_distance(point) >= 0`.
		MUU_CONSTRAINED_TEMPLATE((sizeof...(T) == 0 || all_convertible_to<vector_type, const T&...>), typename... T)
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL same_side(MUU_VPARAM(vector_type) point1,
												MUU_VPARAM(vector_type) point2,
												const T&... pointN) const noexcept
		{
			return same_side(*this, point1, point2, pointN...);
		}

				/// @}
	#endif // distances and projection

	#if 1 // containment ------------------------------------------------------------------------------
		  /// \name Containment
		  /// @{

		/// \brief	Returns true if a plane contains a point.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(plane) p,
													  MUU_VPARAM(vector_type) point,
													  scalar_type epsilon = default_epsilon<scalar_type>) noexcept
		{
			return planes::contains_point(p.normal, p.d, point, epsilon);
		}

		/// \brief	Returns true if the plane contains a point.
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(vector_type) point,
											   scalar_type epsilon = default_epsilon<scalar_type>) const noexcept
		{
			return contains(*this, point, epsilon);
		}

				/// @}
	#endif // containment

	#if 1 // intersection ------------------------------------------------------------------------------
		/// \name Intersection
		/// @{

		/// \brief	Returns true if a plane intersects a line segment.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(plane) p,
														MUU_VPARAM(line_segment<scalar_type>) seg) noexcept;

		/// \brief	Returns true if the plane intersects a line segment.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(line_segment<scalar_type>) seg) const noexcept
		{
			return intersects(*this, seg);
		}

		/// \brief	Returns true if a plane intersects a bounding box.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(plane) p,
														MUU_VPARAM(bounding_box<scalar_type>) bb) noexcept;

		/// \brief	Returns true if the plane intersects a bounding box.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(bounding_box<scalar_type>) bb) const noexcept
		{
			return intersects(*this, bb);
		}

				/// @}
	#endif // intersection

	#if 1 // transformation -------------------------------------------------------------------
		  /// \name Transformation
		  /// @{

		/// \brief Transforms a plane from one coordinate space to another.
		///
		/// \param	p		The plane to transform.
		/// \param	tx		The transform to apply.
		///
		/// \return	Returns the plane transformed into the new coordinate space.
		MUU_PURE_GETTER
		static constexpr plane MUU_VECTORCALL transform(MUU_VPARAM(plane) p,
														MUU_VPARAM(matrix<scalar_type, 4, 4>) tx) noexcept
		{
			if constexpr (requires_promotion)
			{
				return plane{ promoted_plane::transform(promoted_plane{ p }, matrix<promoted_scalar, 4, 4>{ tx }) };
			}
			else
			{
				return plane{ tx * p.origin(), tx.transform_direction(p.normal) };
			}
		}

		/// \brief Transforms the plane from one coordinate space to another (in-place).
		///
		/// \param	tx		The transform to apply.
		///
		/// \return	A reference to the plane.
		constexpr plane& transform(MUU_VPARAM(matrix<scalar_type, 4, 4>) tx) noexcept
		{
			return *this = transform(*this, tx);
		}

				/// @}
	#endif // transformation

	#if 1 // misc ------------------------------------------------------------------------------------------------------

		/// \brief Writes a plane out to a text stream.
		template <typename Char, typename Traits>
		friend std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os, const plane& p)
		{
			const impl::compound_vector_elem<Scalar> elems[]{ { &p.normal.x, 3 }, //
															  { &p.d, 1 } };
			impl::print_compound_vector(os, elems);
			return os;
		}

	#endif // misc
	};

	/// \cond

	MUU_CONSTRAINED_TEMPLATE((all_arithmetic<N, D>), typename N, typename D)
	plane(vector<N, 3>, D)->plane<impl::highest_ranked<N, D>>;

	template <typename P, typename N>
	plane(vector<P, 3>, vector<N, 3>) -> plane<impl::highest_ranked<P, N>>;

	template <typename T, typename U, typename V>
	plane(vector<T, 3>, vector<U, 3>, vector<V, 3>) -> plane<impl::highest_ranked<T, U, V>>;

	template <typename T>
	plane(const triangle<T>&) -> plane<T>;

	/// \endcond
}

namespace std
{
	/// \brief Specialization of std::tuple_size for muu::plane.
	template <typename Scalar>
	struct tuple_size<muu::plane<Scalar>>
	{
		static constexpr size_t value = 2;
	};

	/// \brief Specialization of std::tuple_element for muu::plane.
	template <size_t I, typename Scalar>
	struct tuple_element<I, muu::plane<Scalar>>
	{
		static_assert(I < 2);
		using type = std::conditional_t<I == 0, muu::vector<Scalar, 3>, Scalar>;
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
	/// \see		muu::plane
	///
	/// \brief		Plane constants.
	template <typename Scalar>
	struct constants<plane<Scalar>>
	{
		using scalars = constants<Scalar>;
		using vectors = constants<vector<Scalar, 3>>;

		/// \brief A plane with all members initialized to zero.
		static constexpr plane<Scalar> zero = { vectors::zero, scalars::zero };

		/// \brief The XY plane.
		static constexpr plane<Scalar> xy = { vectors::z_axis, scalars::zero };

		/// \brief The XZ plane.
		static constexpr plane<Scalar> xz = { vectors::y_axis, scalars::zero };

		/// \brief The YZ plane.
		static constexpr plane<Scalar> yz = { vectors::x_axis, scalars::zero };
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
	/// \relatesalso	muu::plane
	///
	/// \brief	Returns true if any of the scalar components of a plane are infinity or NaN.
	template <typename S>
	MUU_PURE_INLINE_GETTER
	constexpr bool infinity_or_nan(const plane<S>& p) noexcept
	{
		return plane<S>::infinity_or_nan(p);
	}

	/// \ingroup		approx_equal
	/// \relatesalso	muu::plane
	///
	/// \brief		Returns true if two planes are approximately equal.
	template <typename S, typename T>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL approx_equal(const plane<S>& p1,
											   const plane<T>& p2,
											   epsilon_type<S, T> epsilon = default_epsilon<S, T>) noexcept
	{
		return plane<S>::approx_equal(p1, p2, epsilon);
	}

	/// \ingroup		approx_zero
	/// \relatesalso	muu::plane
	///
	/// \brief		Returns true if all the scalar components of a plane are approximately equal to zero.
	template <typename S>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL approx_zero(const plane<S>& p, S epsilon = default_epsilon<S>) noexcept
	{
		return plane<S>::approx_zero(p, epsilon);
	}

	/// \ingroup	normalized
	/// \relatesalso	muu::plane
	///
	/// \brief Returns true if a plane is normalized.
	template <typename S>
	MUU_PURE_INLINE_GETTER
	constexpr bool normalized(const plane<S>& p) noexcept
	{
		return plane<S>::normalized(p);
	}
}

#endif //===============================================================================================================

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"

/// \cond
#ifdef MUU_LINE_SEGMENT_H
	#include "impl/plane_x_line_segment.h"
#endif
#ifdef MUU_TRIANGLE_H
	#include "impl/plane_x_triangle.h"
#endif
#ifdef MUU_BOUNDING_BOX_H
	#include "impl/bounding_box_x_plane.h"
#endif
#ifdef MUU_RAY_H
	#include "impl/ray_x_plane.h"
#endif
/// \endcond

#endif // MUU_PLANE_H
