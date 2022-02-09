// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::triangle.

#include "impl/geometry_common.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_DISABLE_SHADOW_WARNINGS;
MUU_PRAGMA_MSVC(float_control(except, off))

//======================================================================================================================
// TRIANGLE CLASS
#if 1

namespace muu
{
	/// \brief A triangle
	/// \ingroup math
	///
	/// \tparam	Scalar      The triangle's scalar component type. Must be a floating-point type.
	template <typename Scalar>
	struct MUU_TRIVIAL_ABI triangle //
		MUU_HIDDEN_BASE(impl::triangle_<Scalar>)
	{
		static_assert(!std::is_reference_v<Scalar>, "Triangle scalar type cannot be a reference");
		static_assert(!is_cv<Scalar>, "Triangle scalar type cannot be const- or volatile-qualified");
		static_assert(std::is_trivially_constructible_v<Scalar>	  //
						  && std::is_trivially_copyable_v<Scalar> //
						  && std::is_trivially_destructible_v<Scalar>,
					  "Triangle scalar type must be trivially constructible, copyable and destructible");
		static_assert(is_signed<Scalar>, "Triangle scalar type must be signed");
		static_assert(is_floating_point<Scalar>, "Triangle type must be a floating-point type");

		/// \brief The triangle's scalar type.
		using scalar_type = Scalar;

		/// \brief The three-dimensional #muu::vector with the same #scalar_type as the triangle.
		using vector_type = vector<scalar_type, 3>;

		/// \brief Compile-time triangle constants.
		using constants = muu::constants<triangle>;

		/// \brief A LegacyRandomAccessIterator for the points in the triangle.
		using iterator = vector_type*;

		/// \brief A const LegacyRandomAccessIterator for the points in the triangle.
		using const_iterator = const vector_type*;

	  private:
		/// \cond
		using base = impl::triangle_<Scalar>;
		static_assert(sizeof(base) == (sizeof(vector_type) * 3), "Triangles should not have padding");

		using triangles			 = impl::triangles_common<Scalar>;
		using aabbs				 = impl::aabb_common<Scalar>;
		using intermediate_float = promote_if_small_float<scalar_type>;
		static_assert(is_floating_point<intermediate_float>);

		/// \endcond

	  public:
	#ifdef DOXYGEN

		/// \brief	the points in the triangle.
		vector_type points[3];

	#endif // DOXYGEN

	#if 1 // constructors ----------------------------------------------------------------------------------------------

		/// \brief Default constructor. Values are not initialized.
		triangle() noexcept = default;

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr triangle(const triangle&) noexcept = default;

		/// \brief Copy-assigment operator.
		constexpr triangle& operator=(const triangle&) noexcept = default;

		/// \brief	Constructs a triangle from three points.
		MUU_NODISCARD_CTOR
		constexpr triangle(const vector_type& p0, const vector_type& p1, const vector_type& p2) noexcept //
			: base{ p0, p1, p2 }
		{}

		/// \brief	Constructs a triangle from three points.
		MUU_NODISCARD_CTOR
		explicit constexpr triangle(const vector_type (&points)[3]) noexcept //
			: base{ points[0], points[0], points[0] }
		{}

		/// \brief	Constructs a triangle from three points (in scalar form).
		MUU_NODISCARD_CTOR
		constexpr triangle(scalar_type p0_x,
						   scalar_type p0_y,
						   scalar_type p0_z,
						   scalar_type p1_x,
						   scalar_type p1_y,
						   scalar_type p1_z,
						   scalar_type p2_x,
						   scalar_type p2_y,
						   scalar_type p2_z) noexcept //
			: base{ vector_type{ p0_x, p0_y, p0_z }, vector_type{ p1_x, p1_y, p1_z }, vector_type{ p2_x, p2_y, p2_z } }
		{}

		/// \brief	Converting constructor.
		template <typename S>
		MUU_NODISCARD_CTOR
		explicit constexpr triangle(const triangle<S>& tri) noexcept //
			: base{ vector_type{ tri.points[0] }, vector_type{ tri.points[1] }, vector_type{ tri.points[2] } }
		{}

		/// \brief Constructs a triangle from an implicitly bit-castable type.
		///
		/// \tparam T	A bit-castable type.
		///
		/// \see muu::allow_implicit_bit_cast
		MUU_CONSTRAINED_TEMPLATE((allow_implicit_bit_cast<T, triangle>), typename T)
		MUU_NODISCARD_CTOR
		/*implicit*/
		constexpr triangle(const T& blittable) noexcept //
			: base{ muu::bit_cast<base>(blittable) }
		{
			static_assert(sizeof(T) == sizeof(base), "Bit-castable types must be the same size");
			static_assert(std::is_trivially_copyable_v<T>, "Bit-castable types must be trivially-copyable");
		}

	#endif // constructors

	#if 1 // point accessors -------------------------------------------------------------------------------------------
		/// \name Point accessors
		/// @{

		/// \brief Returns a reference to one of the triangle's points.
		template <size_t P>
		MUU_PURE_INLINE_GETTER
		constexpr vector_type& get() noexcept
		{
			static_assert(P < 3, "Point index out of range");

			return base::points[P];
		}

		/// \brief Returns a reference to one of the triangle's points.
		template <size_t P>
		MUU_PURE_INLINE_GETTER
		constexpr const vector_type& get() const noexcept
		{
			static_assert(P < 3, "Point index out of range");

			return base::points[P];
		}

		/// \brief Returns a reference to one of the triangle's points.
		MUU_PURE_INLINE_GETTER
		constexpr vector_type& operator[](size_t p) noexcept
		{
			MUU_ASSUME(p < 3);

			return base::points[p];
		}

		/// \brief Returns a reference to one of the triangle's points.
		MUU_PURE_INLINE_GETTER
		constexpr const vector_type& operator[](size_t p) const noexcept
		{
			MUU_ASSUME(p < 3);

			return base::points[p];
		}

			/// @}
	#endif // point accessors

	#if 1 // scalar accessors ------------------------------------------------------------------------------------------
		  /// \name Scalar accessors
		  /// @{

		/// \brief Returns a pointer to the first scalar component in the triangle.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type* data() noexcept
		{
			return base::points[0].data();
		}

		/// \brief Returns a pointer to the first scalar component in the triangle.
		MUU_PURE_INLINE_GETTER
		constexpr const scalar_type* data() const noexcept
		{
			return base::points[0].data();
		}

			/// @}
	#endif // scalar accessors

	#if 1 // geometric properties --------------------------------------------------------------------------------------
		  /// \name Geometric properties
		  /// @{

		/// \brief	Returns the normal of a triangle's plane.
		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL normal(MUU_VC_PARAM(vector_type) p0,
														   MUU_VC_PARAM(vector_type) p1,
														   MUU_VC_PARAM(vector_type) p2) noexcept
		{
			return triangles::normal(p0, p1, p2);
		}

		/// \brief	Returns the normal of the triangle's plane.
		MUU_PURE_GETTER
		constexpr vector_type normal() const noexcept
		{
			return triangles::normal(base::points[0], base::points[1], base::points[2]);
		}

		/// \brief	Returns a triangle's centroid point.
		///
		/// \see https://en.wikipedia.org/wiki/Centroid
		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL centroid(MUU_VC_PARAM(vector_type) p0,
															 MUU_VC_PARAM(vector_type) p1,
															 MUU_VC_PARAM(vector_type) p2) noexcept
		{
			return triangles::centroid(p0, p1, p2);
		}

		/// \brief	Returns the triangle's centroid point.
		///
		/// \see https://en.wikipedia.org/wiki/Centroid
		MUU_PURE_GETTER
		constexpr vector_type centroid() const noexcept
		{
			return triangles::centroid(base::points[0], base::points[1], base::points[2]);
		}

		/// \brief	Returns the length of a triangle's perimeter.
		MUU_PURE_GETTER
		static constexpr scalar_type MUU_VECTORCALL perimeter(MUU_VC_PARAM(vector_type) p0,
															  MUU_VC_PARAM(vector_type) p1,
															  MUU_VC_PARAM(vector_type) p2) noexcept
		{
			return triangles::perimeter(p0, p1, p2);
		}

		/// \brief	Returns the length of the triangle's perimeter.
		MUU_PURE_GETTER
		constexpr scalar_type perimeter() const noexcept
		{
			return triangles::perimeter(base::points[0], base::points[1], base::points[2]);
		}

		/// \brief	Returns the area of a triangle.
		MUU_PURE_GETTER
		static constexpr scalar_type area(MUU_VC_PARAM(vector_type) p0,
										  MUU_VC_PARAM(vector_type) p1,
										  MUU_VC_PARAM(vector_type) p2) noexcept
		{
			return triangles::area(p0, p1, p2);
		}

		/// \brief	Returns the area of the triangle.
		MUU_PURE_GETTER
		constexpr scalar_type area() const noexcept
		{
			return triangles::area(base::points[0], base::points[1], base::points[2]);
		}

		/// \brief	Returns true if a triangle is degenerate (i.e. two or more of its points are coincident).
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL degenerate(MUU_VC_PARAM(vector_type) p0,
														MUU_VC_PARAM(vector_type) p1,
														MUU_VC_PARAM(vector_type) p2) noexcept
		{
			return triangles::degenerate(p0, p1, p2);
		}

		/// \brief	Returns true if the triangle is degenerate (i.e. two or more of its points are coincident).
		MUU_PURE_GETTER
		constexpr bool degenerate() const noexcept
		{
			return triangles::degenerate(base::points[0], base::points[1], base::points[2]);
		}

		/// \brief	Returns the plane on which a triangle lies.
		MUU_PURE_GETTER
		static constexpr muu::plane<scalar_type> MUU_VECTORCALL plane(MUU_VC_PARAM(vector_type) p0,
																	  MUU_VC_PARAM(vector_type) p1,
																	  MUU_VC_PARAM(vector_type) p2) noexcept;

		/// \brief	Returns the plane on which the triangle lies.
		MUU_PURE_GETTER
		constexpr muu::plane<scalar_type> plane() const noexcept
		{
			return plane(base::points);
		}

		/// \brief	Returns the plane on which the triangle lies.
		MUU_PURE_GETTER
		explicit constexpr operator muu::plane<scalar_type>() const noexcept
		{
			return plane();
		}

			/// @}
	#endif // geometric properties

	#if 1 // equality (exact) -------------------------------------------------------------------------------------
		/// \name Equality (exact)
		/// @{

		/// \brief		Returns true if two triangles are exactly equal.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		template <typename T>
		MUU_PURE_GETTER
		friend constexpr bool MUU_VECTORCALL operator==(MUU_VC_PARAM(triangle) lhs, const triangle<T>& rhs) noexcept
		{
			return lhs.n == rhs.n && lhs.d == rhs.d;
		}

		/// \brief	Returns true if two triangles are not exactly equal.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		template <typename T>
		MUU_PURE_GETTER
		friend constexpr bool MUU_VECTORCALL operator!=(MUU_VC_PARAM(triangle) lhs, const triangle<T>& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		/// \brief	Returns true if all the points of a triangle are exactly zero.
		///
		/// \remarks	This is an exact check;
		///				use #approx_zero() if you want an epsilon-based "near-enough" check.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL zero(MUU_VC_PARAM(triangle) tri) noexcept
		{
			return vector_type::zero(tri.points[0]) //
				&& vector_type::zero(tri.points[1]) //
				&& vector_type::zero(tri.points[2]);
		}

		/// \brief	Returns true if all the points of the triangle are exactly zero.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		MUU_PURE_GETTER
		constexpr bool zero() const noexcept
		{
			return zero(*this);
		}

		/// \brief	Returns true if any of the points of a triangle are infinity or NaN.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL infinity_or_nan(MUU_VC_PARAM(triangle) tri) noexcept
		{
			return vector_type::infinity_or_nan(tri.points[0]) //
				&& vector_type::infinity_or_nan(tri.points[1]) //
				&& vector_type::infinity_or_nan(tri.points[2]);
		}

		/// \brief	Returns true if any of the points in the triangle are infinity or NaN.
		MUU_PURE_GETTER
		constexpr bool infinity_or_nan() const noexcept
		{
			return infinity_or_nan(*this);
		}

			/// @}
	#endif // equality (exact)

	#if 1 // equality (approx) -----------------------------------------------------------------------------------
		/// \name Equality (approximate)
		/// @{

		/// \brief	Returns true if two triangles are approximately equal.
		template <typename T>
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_equal(
			MUU_VC_PARAM(triangle) tri1,
			const triangle<T>& tri2,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) noexcept
		{
			return vector_type::approx_equal(tri1.points[0], tri2.points[0], epsilon) //
				&& vector_type::approx_equal(tri1.points[1], tri2.points[1], epsilon) //
				&& vector_type::approx_equal(tri1.points[2], tri2.points[2], epsilon);
		}

		/// \brief	Returns true if the triangle is approximately equal to another.
		template <typename T>
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL approx_equal(
			const triangle<T>& tri,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) const noexcept
		{
			return approx_equal(*this, tri, epsilon);
		}

		/// \brief	Returns true if all the scalar components in a triangle are approximately equal to zero.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_zero(MUU_VC_PARAM(triangle) tri,
														 scalar_type epsilon = default_epsilon<scalar_type>) noexcept
		{
			return vector_type::approx_equal(tri.points[0], epsilon) //
				&& vector_type::approx_equal(tri.points[1], epsilon) //
				&& vector_type::approx_equal(tri.points[2], epsilon);
		}

		/// \brief	Returns true if all the scalar components in the triangle are approximately equal to zero.
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL approx_zero(scalar_type epsilon = default_epsilon<scalar_type>) const noexcept
		{
			return approx_zero(*this, epsilon);
		}

			/// @}
	#endif // equality (approx)

	#if 1 // intersection and containment ------------------------------------------------------------------------------
		  /// \name Intersection and containment
		  /// @{

		/// \brief	Returns true if a triangle and a point are coplanar.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL coplanar(MUU_VC_PARAM(vector_type) p0,
													  MUU_VC_PARAM(vector_type) p1,
													  MUU_VC_PARAM(vector_type) p2,
													  MUU_VC_PARAM(vector_type) point) noexcept
		{
			return triangles::coplanar(p0, p1, p2, point);
		}

		/// \brief	Returns true if a triangle and a point are coplanar.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL coplanar(MUU_VC_PARAM(triangle) tri,
													  MUU_VC_PARAM(vector_type) point) noexcept
		{
			return triangles::coplanar(tri.points[0], tri.points[1], tri.points[2], point);
		}

		/// \brief	Returns true if the triangle and a point are coplanar.
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL coplanar(MUU_VC_PARAM(vector_type) point) const noexcept
		{
			return triangles::coplanar(base::points[0], base::points[1], base::points[2], point);
		}

		/// \brief	Returns true if a triangle contains a point.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VC_PARAM(vector_type) p0,
													  MUU_VC_PARAM(vector_type) p1,
													  MUU_VC_PARAM(vector_type) p2,
													  MUU_VC_PARAM(vector_type) point) noexcept
		{
			return triangles::contains_point(p0, p1, p2, point);
		}

		/// \brief	Returns true if a triangle contains a point.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VC_PARAM(triangle) tri,
													  MUU_VC_PARAM(vector_type) point) noexcept
		{
			return triangles::contains_point(tri.points[0], tri.points[1], tri.points[2], point);
		}

		/// \brief	Returns true if the triangle contains a point.
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL contains(MUU_VC_PARAM(vector_type) point) const noexcept
		{
			return triangles::contains_point(base::points[0], base::points[1], base::points[2], point);
		}

		/// \brief	Returns true if a triangle intersects a bounding box.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VC_PARAM(vector_type) p0,
														MUU_VC_PARAM(vector_type) p1,
														MUU_VC_PARAM(vector_type) p2,
														MUU_VC_PARAM(bounding_box<scalar_type>) bb) noexcept;

		/// \brief	Returns true if a triangle intersects a bounding box.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VC_PARAM(triangle) tri,
														MUU_VC_PARAM(bounding_box<scalar_type>) bb) noexcept;

		/// \brief	Returns true if the triangle intersects a bounding box.
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL intersects(MUU_VC_PARAM(bounding_box<scalar_type>) bb) const noexcept
		{
			return intersects(*this, bb);
		}

			/// @}
	#endif // intersection and containment

	#if 1 // barycentric coordinates ----------------------------------------------------------------------
		  /// \name Barycentric coordinates
		  /// @{

		/// \brief Returns the barycentric coordinates of a point within a triangle.
		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL barycentric(MUU_VC_PARAM(vector_type) p0,
																MUU_VC_PARAM(vector_type) p1,
																MUU_VC_PARAM(vector_type) p2,
																MUU_VC_PARAM(vector_type) point) noexcept
		{
			return triangles::barycentric(p0, p1, p2, point);
		}

		/// \brief Returns the barycentric coordinates of a point within the triangle.
		MUU_PURE_GETTER
		constexpr vector_type MUU_VECTORCALL barycentric(MUU_VC_PARAM(vector_type) point) const noexcept
		{
			return barycentric(base::points[0], base::points[1], base::points[2], point);
		}

		/// \brief Returns a generator for calculating multiple barycentric coordinates from the same triangle.
		MUU_PURE_GETTER
		static constexpr auto MUU_VECTORCALL barycentric_generator(const vector_type& p0,
																   const vector_type& p1,
																   const vector_type& p2) noexcept
		{
			return typename triangles::barycentric_generator{ p0, p1, p2 };
		}

		/// \brief Returns a generator for calculating multiple barycentric coordinates from the triangle.
		MUU_PURE_GETTER
		constexpr auto MUU_VECTORCALL barycentric_generator() const noexcept
		{
			return barycentric_generator(base::points[0], base::points[1], base::points[2]);
		}

			/// @}
	#endif // barycentric coordinates

	#if 1 // iterators -----------------------------------------------------------------------------------
		/// \name Iterators
		/// @{

		/// \brief Returns an iterator to the first point in the triangle.
		MUU_PURE_INLINE_GETTER
		constexpr iterator begin() noexcept
		{
			return base::points;
		}

		/// \brief Returns an iterator to the one-past-the-last point in the triangle.
		MUU_PURE_INLINE_GETTER
		constexpr iterator end() noexcept
		{
			return base::points + 3u;
		}

		/// \brief Returns a const iterator to the first point in the triangle.
		MUU_PURE_INLINE_GETTER
		constexpr const_iterator begin() const noexcept
		{
			return base::points;
		}

		/// \brief Returns a const iterator to the one-past-the-last point in the triangle.
		MUU_PURE_INLINE_GETTER
		constexpr const_iterator end() const noexcept
		{
			return base::points + 3u;
		}

		/// \brief Returns a const iterator to the first point in the triangle.
		MUU_PURE_INLINE_GETTER
		constexpr const_iterator cbegin() const noexcept
		{
			return base::points;
		}

		/// \brief Returns a const iterator to the one-past-the-last point in the triangle.
		MUU_PURE_INLINE_GETTER
		constexpr const_iterator cend() const noexcept
		{
			return base::points + 3u;
		}

		/// @}

		/// \name Iterators (ADL)
		/// @{

		/// \brief Returns an iterator to the first point in a triangle.
		MUU_PURE_INLINE_GETTER
		friend constexpr iterator begin(triangle& v) noexcept
		{
			return v.begin();
		}

		/// \brief Returns an iterator to the one-past-the-last point in a triangle.
		MUU_PURE_INLINE_GETTER
		friend constexpr iterator end(triangle& v) noexcept
		{
			return v.end();
		}

		/// \brief Returns a const iterator to the first point in a triangle.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_iterator begin(const triangle& v) noexcept
		{
			return v.begin();
		}

		/// \brief Returns a const iterator to the one-past-the-last point in a triangle.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_iterator end(const triangle& v) noexcept
		{
			return v.end();
		}

		/// \brief Returns a const iterator to the first point in a triangle.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_iterator cbegin(const triangle& v) noexcept
		{
			return v.begin();
		}

		/// \brief Returns a const iterator to the one-past-the-last point in a triangle.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_iterator cend(const triangle& v) noexcept
		{
			return v.end();
		}

			/// @}
	#endif // iterators

	#if 1 // misc -----------------------------------------------------------------------------------------

		/// \brief Writes a triangle out to a text stream.
		template <typename Char, typename Traits>
		friend std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os, const triangle& tri)
		{
			const impl::compound_vector_elem<Scalar> elems[]{ { &tri.points[0].x, 3 }, //
															  { &tri.points[1].x, 3 }, //
															  { &tri.points[2].x, 3 } };
			impl::print_compound_vector(os, elems);
			return os;
		}

	#endif // misc
	};

	/// \cond

	template <typename T1, typename T2, typename T3>
	triangle(vector<T1, 3>, vector<T2, 3>, vector<T3, 3>) -> triangle<impl::highest_ranked<T1, T2, T3>>;

	template <typename T>
	triangle(const vector<T, 3> (&)[3]) -> triangle<T>;

	MUU_CONSTRAINED_TEMPLATE((all_arithmetic<X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3>),
							 typename X1,
							 typename Y1,
							 typename Z1,
							 typename X2,
							 typename Y2,
							 typename Z2,
							 typename X3,
							 typename Y3,
							 typename Z3)
	triangle(X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3)->triangle<impl::highest_ranked<X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3>>;

	template <typename T>
	triangle(const triangle<T>&) -> triangle<T>;

	/// \endcond
}

namespace std
{
	/// \brief Specialization of std::tuple_size for muu::triangle.
	template <typename Scalar>
	struct tuple_size<muu::triangle<Scalar>>
	{
		static constexpr size_t value = 3;
	};

	/// \brief Specialization of std::tuple_element for muu::triangle.
	template <size_t P, typename Scalar>
	struct tuple_element<P, muu::triangle<Scalar>>
	{
		static_assert(P < 3);

		using type = muu::vector<Scalar, 3>;
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
	/// \see		muu::triangle
	///
	/// \brief		Triangle constants.
	template <typename Scalar>
	struct constants<triangle<Scalar>>
	{
		using scalars = constants<Scalar>;
		using vectors = constants<vector<Scalar, 3>>;

		/// \brief A triangle with all points initialized to zero.
		static constexpr triangle<Scalar> zero = { vectors::zero, vectors::zero, vectors::zero };
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
	/// \relatesalso	muu::triangle
	///
	/// \brief	Returns true if any of the points in a triangle are infinity or NaN.
	template <typename S>
	MUU_PURE_GETTER
	constexpr bool infinity_or_nan(const triangle<S>& tri) noexcept
	{
		return triangle<S>::infinity_or_nan(tri);
	}

	/// \ingroup		approx_equal
	/// \relatesalso	muu::triangle
	///
	/// \brief		Returns true if two triangles are approximately equal.
	template <typename S, typename T>
	MUU_PURE_GETTER
	constexpr bool MUU_VECTORCALL approx_equal(const triangle<S>& tri1,
											   const triangle<T>& tri2,
											   epsilon_type<S, T> epsilon = default_epsilon<S, T>) noexcept
	{
		return triangle<S>::approx_equal(tri1, tri2, epsilon);
	}

	/// \ingroup		approx_zero
	/// \relatesalso	muu::triangle
	///
	/// \brief		Returns true if all the points in triangle are approximately equal to zero.
	template <typename S>
	MUU_PURE_GETTER
	constexpr bool MUU_VECTORCALL approx_zero(const triangle<S>& tri, S epsilon = default_epsilon<S>) noexcept
	{
		return triangle<S>::approx_zero(tri, epsilon);
	}
}

#endif //===============================================================================================================

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"

/// \cond
#include "impl/bounding_box_x_triangle.h"
#include "impl/plane_x_triangle.h"
/// \endcond
