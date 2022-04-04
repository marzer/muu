// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::line_segment.

#include "impl/geometry_common.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_DISABLE_SHADOW_WARNINGS;
MUU_PRAGMA_MSVC(float_control(except, off))

//======================================================================================================================
// LINE SEGMENT CLASS
#if 1

namespace muu
{
	/// \brief A line segment.
	/// \ingroup math
	///
	/// \tparam	Scalar      The line segment's scalar component type. Must be a floating-point type.
	template <typename Scalar>
	struct MUU_TRIVIAL_ABI line_segment //
		MUU_HIDDEN_BASE(impl::line_segment_<Scalar>)
	{
		static_assert(!std::is_reference_v<Scalar>, "Line segment scalar type cannot be a reference");
		static_assert(!is_cv<Scalar>, "Line segment scalar type cannot be const- or volatile-qualified");
		static_assert(std::is_trivially_constructible_v<Scalar>	  //
						  && std::is_trivially_copyable_v<Scalar> //
						  && std::is_trivially_destructible_v<Scalar>,
					  "Line segment scalar type must be trivially constructible, copyable and destructible");
		static_assert(is_signed<Scalar>, "Line segment scalar type must be signed");
		static_assert(is_floating_point<Scalar>, "Line segment type must be a floating-point type");

		/// \brief The line segment's scalar type.
		using scalar_type = Scalar;

		/// \brief The three-dimensional #muu::vector with the same #scalar_type as the line segment.
		using vector_type = vector<scalar_type, 3>;

		/// \brief Compile-time line segment constants.
		using constants = muu::constants<line_segment>;

		/// \brief A LegacyRandomAccessIterator for the points in the line segment.
		using iterator = vector_type*;

		/// \brief A const LegacyRandomAccessIterator for the points in the line segment.
		using const_iterator = const vector_type*;

	  private:
		/// \cond
		using base = impl::line_segment_<Scalar>;
		static_assert(sizeof(base) == (sizeof(vector_type) * 2), "Line segments should not have padding");

		using aabbs		= impl::aabb_common<Scalar>;
		using lines		= impl::lines_common<Scalar>;
		using segments	= impl::line_segments_common<Scalar>;
		using triangles = impl::triangles_common<Scalar>;
		using planes	= impl::planes_common<Scalar>;

		using promoted_scalar				 = promote_if_small_float<scalar_type>;
		using promoted_vec					 = vector<promoted_scalar, 3>;
		static constexpr bool is_small_float = impl::is_small_float_<scalar_type>;

		/// \endcond

	  public:
	#ifdef DOXYGEN

		/// \brief	the points in the line segment.
		vector_type points[2];

	#endif // DOXYGEN

	#if 1 // constructors ----------------------------------------------------------------------------------------------

		/// \brief Default constructor. Values are not initialized.
		line_segment() noexcept = default;

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr line_segment(const line_segment&) noexcept = default;

		/// \brief Copy-assigment operator.
		constexpr line_segment& operator=(const line_segment&) noexcept = default;

		/// \brief	Constructs a line segment from two points.
		MUU_NODISCARD_CTOR
		constexpr line_segment(const vector_type& p0, const vector_type& p1) noexcept //
			: base{ p0, p1 }
		{}

		/// \brief	Constructs a line segment from two points.
		MUU_NODISCARD_CTOR
		explicit constexpr line_segment(const vector_type (&points)[2]) noexcept //
			: base{ points[0], points[1] }
		{}

		/// \brief	Constructs a line segment from two points (in scalar form).
		MUU_NODISCARD_CTOR
		constexpr line_segment(scalar_type p0_x,
							   scalar_type p0_y,
							   scalar_type p0_z,
							   scalar_type p1_x,
							   scalar_type p1_y,
							   scalar_type p1_z) noexcept //
			: base{ vector_type{ p0_x, p0_y, p0_z }, vector_type{ p1_x, p1_y, p1_z } }
		{}

		/// \brief	Converting constructor.
		template <typename S>
		MUU_NODISCARD_CTOR
		explicit constexpr line_segment(const line_segment<S>& seg) noexcept //
			: base{ vector_type{ seg.points[0] }, vector_type{ seg.points[1] } }
		{}

		/// \brief Constructs a line segment from an implicitly bit-castable type.
		///
		/// \tparam T	A bit-castable type.
		///
		/// \see muu::allow_implicit_bit_cast
		MUU_CONSTRAINED_TEMPLATE((allow_implicit_bit_cast<T, line_segment>), typename T)
		MUU_NODISCARD_CTOR
		/*implicit*/
		constexpr line_segment(const T& blittable) noexcept //
			: base{ muu::bit_cast<base>(blittable) }
		{
			static_assert(sizeof(T) == sizeof(base), "Bit-castable types must be the same size");
			static_assert(std::is_trivially_copyable_v<T>, "Bit-castable types must be trivially-copyable");
		}

	#endif // constructors

	#if 1 // point accessors -------------------------------------------------------------------------------------------
		/// \name Point accessors
		/// @{

		/// \brief Returns a reference to one of the line segment's points.
		template <size_t P>
		MUU_PURE_INLINE_GETTER
		constexpr vector_type& get() noexcept
		{
			static_assert(P < 2, "Point index out of range");

			return base::points[P];
		}

		/// \brief Returns a reference to one of the line segment's points.
		template <size_t P>
		MUU_PURE_INLINE_GETTER
		constexpr const vector_type& get() const noexcept
		{
			static_assert(P < 2, "Point index out of range");

			return base::points[P];
		}

		/// \brief Returns a reference to one of the line segment's points.
		MUU_PURE_INLINE_GETTER
		constexpr vector_type& operator[](size_t p) noexcept
		{
			MUU_ASSUME(p < 2);

			return base::points[p];
		}

		/// \brief Returns a reference to one of the line segment's points.
		MUU_PURE_INLINE_GETTER
		constexpr const vector_type& operator[](size_t p) const noexcept
		{
			MUU_ASSUME(p < 2);

			return base::points[p];
		}

			/// @}
	#endif // point accessors

	#if 1 // scalar accessors ------------------------------------------------------------------------------------------
		  /// \name Scalar accessors
		  /// @{

		/// \brief Returns a pointer to the first scalar component in the line segment.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type* data() noexcept
		{
			return base::points[0].data();
		}

		/// \brief Returns a pointer to the first scalar component in the line segment.
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

		/// \brief	Returns the direction of a line segment (from point 0 to point 1).
		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL direction(MUU_VC_PARAM(vector_type) p0,
															  MUU_VC_PARAM(vector_type) p1,
															  scalar_type& length_out) noexcept
		{
			return vector_type::direction(p0, p1, length_out);
		}

		/// \brief	Returns the direction of a line segment (from point 0 to point 1).
		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL direction(MUU_VC_PARAM(vector_type) p0,
															  MUU_VC_PARAM(vector_type) p1) noexcept
		{
			return vector_type::direction(p0, p1);
		}

		/// \brief	Returns the direction of the line segment (from point 0 to point 1).
		MUU_PURE_INLINE_GETTER
		constexpr vector_type direction(scalar_type& length_out) const noexcept
		{
			return vector_type::direction(base::points[0], base::points[1], length_out);
		}

		/// \brief	Returns the direction of the line segment (from point 0 to point 1).
		MUU_PURE_INLINE_GETTER
		constexpr vector_type direction() const noexcept
		{
			return vector_type::direction(base::points[0], base::points[1]);
		}

		/// \brief	Returns a line segment's midpoint.
		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL midpoint(MUU_VC_PARAM(vector_type) p0,
															 MUU_VC_PARAM(vector_type) p1) noexcept
		{
			if constexpr (is_small_float)
				return vector_type{ line_segment<promoted_scalar>::midpoint(promoted_vec{ p0 }, promoted_vec{ p1 }) };
			else
				return (p0 + p1) / scalar_type{ 2 };
		}

		/// \brief	Returns the line segment's midpoint.
		MUU_PURE_INLINE_GETTER
		constexpr vector_type midpoint() const noexcept
		{
			return midpoint(base::points[0], base::points[1]);
		}

		/// \brief	Returns the squared length of a line segment.
		MUU_PURE_INLINE_GETTER
		static constexpr scalar_type MUU_VECTORCALL length_squared(MUU_VC_PARAM(vector_type) p0,
																   MUU_VC_PARAM(vector_type) p1) noexcept
		{
			return vector_type::length_squared(p0, p1);
		}

		/// \brief	Returns the squared length of a line segment.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type length_squared() const noexcept
		{
			return vector_type::length_squared(base::points[0], base::points[1]);
		}

		/// \brief	Returns the length of a line segment.
		MUU_PURE_INLINE_GETTER
		static constexpr scalar_type MUU_VECTORCALL length(MUU_VC_PARAM(vector_type) p0,
														   MUU_VC_PARAM(vector_type) p1) noexcept
		{
			return vector_type::length(p0, p1);
		}

		/// \brief	Returns the length of a line segment.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type length() const noexcept
		{
			return vector_type::length(base::points[0], base::points[1]);
		}

		/// \brief	Returns true if a line segment is degenerate (i.e. its points are coincident).
		MUU_PURE_INLINE_GETTER
		static constexpr bool MUU_VECTORCALL degenerate(MUU_VC_PARAM(vector_type) p0,
														MUU_VC_PARAM(vector_type) p1) noexcept
		{
			return p0 == p1;
		}

		/// \brief	Returns true if a line segment is degenerate (i.e. its points are coincident).
		MUU_PURE_INLINE_GETTER
		constexpr bool degenerate() const noexcept
		{
			return base::points[0] == base::points[1];
		}

			/// @}
	#endif // geometric properties

	#if 1 // equality (exact) -------------------------------------------------------------------------------------
		  /// \name Equality (exact)
		  /// @{

		/// \brief		Returns true if two line segments are exactly equal.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		template <typename T>
		MUU_PURE_GETTER
		friend constexpr bool MUU_VECTORCALL operator==(MUU_VC_PARAM(line_segment) lhs,
														const line_segment<T>& rhs) noexcept
		{
			return lhs.points[0] == rhs.points[0] //
				&& lhs.points[1] == rhs.points[1];
		}

		/// \brief	Returns true if two line segments are not exactly equal.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		template <typename T>
		MUU_PURE_GETTER
		friend constexpr bool MUU_VECTORCALL operator!=(MUU_VC_PARAM(line_segment) lhs,
														const line_segment<T>& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		/// \brief	Returns true if all the points of a line segment are exactly zero.
		///
		/// \remarks	This is an exact check;
		///				use #approx_zero() if you want an epsilon-based "near-enough" check.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL zero(MUU_VC_PARAM(line_segment) seg) noexcept
		{
			return vector_type::zero(seg.points[0]) //
				&& vector_type::zero(seg.points[1]);
		}

		/// \brief	Returns true if all the points of the line segment are exactly zero.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		MUU_PURE_INLINE_GETTER
		constexpr bool zero() const noexcept
		{
			return zero(*this);
		}

		/// \brief	Returns true if any of the points of a line segment are infinity or NaN.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL infinity_or_nan(MUU_VC_PARAM(line_segment) seg) noexcept
		{
			return vector_type::infinity_or_nan(seg.points[0]) //
				&& vector_type::infinity_or_nan(seg.points[1]);
		}

		/// \brief	Returns true if any of the points in the line segment are infinity or NaN.
		MUU_PURE_INLINE_GETTER
		constexpr bool infinity_or_nan() const noexcept
		{
			return infinity_or_nan(*this);
		}

			/// @}
	#endif // equality (exact)

	#if 1 // equality (approx) -----------------------------------------------------------------------------------
		  /// \name Equality (approximate)
		  /// @{

		/// \brief	Returns true if two line segments are approximately equal.
		template <typename T>
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_equal(
			MUU_VC_PARAM(line_segment) seg1,
			const line_segment<T>& seg2,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) noexcept
		{
			return vector_type::approx_equal(seg1.points[0], seg2.points[0], epsilon) //
				&& vector_type::approx_equal(seg1.points[1], seg2.points[1], epsilon);
		}

		/// \brief	Returns true if the line segment is approximately equal to another.
		template <typename T>
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL approx_equal(
			const line_segment<T>& seg,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) const noexcept
		{
			return approx_equal(*this, seg, epsilon);
		}

		/// \brief	Returns true if all the scalar components in a line segment are approximately equal to zero.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_zero(MUU_VC_PARAM(line_segment) seg,
														 scalar_type epsilon = default_epsilon<scalar_type>) noexcept
		{
			return vector_type::approx_equal(seg.points[0], epsilon) //
				&& vector_type::approx_equal(seg.points[1], epsilon);
		}

		/// \brief	Returns true if all the scalar components in the line segment are approximately equal to zero.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL approx_zero(scalar_type epsilon = default_epsilon<scalar_type>) const noexcept
		{
			return approx_zero(*this, epsilon);
		}

			/// @}
	#endif // equality (approx)

	#if 1 // intersection and containment ------------------------------------------------------------------------------
		  /// \name Intersection and containment
		  /// @{

		/// \brief	Returns true if a line segment and a point are colinear (i.e. they lie on the same infinite line).
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL colinear(MUU_VC_PARAM(vector_type) p0,
													  MUU_VC_PARAM(vector_type) p1,
													  MUU_VC_PARAM(vector_type) point) noexcept
		{
			return lines::contains_point(p0, vector_type::direction(p0, p1), point);
		}

		/// \brief	Returns true if a line segment and a point are colinear (i.e. they lie on the same infinite line).
		MUU_PURE_INLINE_GETTER
		static constexpr bool MUU_VECTORCALL colinear(MUU_VC_PARAM(line_segment) seg,
													  MUU_VC_PARAM(vector_type) point) noexcept
		{
			return colinear(seg.points[0], seg.points[1], point);
		}

		/// \brief	Returns true if a line segment and a point are colinear (i.e. they lie on the same infinite line).
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL colinear(MUU_VC_PARAM(vector_type) point) const noexcept
		{
			return colinear(*this, point);
		}

		/// \brief	Returns true if a point lies on a line segment.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VC_PARAM(vector_type) p0,
													  MUU_VC_PARAM(vector_type) p1,
													  MUU_VC_PARAM(vector_type) point) noexcept
		{
			return segments::contains_point(p0, vector_type::direction(p0, p1), point);
		}

		/// \brief	Returns true if a point lies on a line segment.
		MUU_PURE_INLINE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VC_PARAM(line_segment) seg,
													  MUU_VC_PARAM(vector_type) point) noexcept
		{
			return contains(seg.points[0], seg.points[1], point);
		}

		/// \brief	Returns true if a point lies on a line segment.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL contains(MUU_VC_PARAM(vector_type) point) const noexcept
		{
			return contains(*this, point);
		}

		/// \brief	Returns true if two line segments are coplanar.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL coplanar(MUU_VC_PARAM(vector_type) a0,
													  MUU_VC_PARAM(vector_type) a1,
													  MUU_VC_PARAM(vector_type) b0,
													  MUU_VC_PARAM(vector_type) b1) noexcept
		{
			const auto n = triangles::normal(a0, a1, b0);
			return planes::contains_point(n, planes::d_term(a0, n), b1);
		}

		/// \brief	Returns true if two line segments are coplanar.
		MUU_PURE_INLINE_GETTER
		static constexpr bool MUU_VECTORCALL coplanar(MUU_VC_PARAM(line_segment) seg1,
													  MUU_VC_PARAM(line_segment) seg2) noexcept
		{
			return coplanar(seg1.points[0], seg1.points[1], seg2.points[0], seg2.points[1]);
		}

		/// \brief	Returns true if two line segments are coplanar.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL coplanar(MUU_VC_PARAM(line_segment) seg) const noexcept
		{
			return coplanar(*this, seg);
		}

		/// \brief	Returns true if a line segment intersects a bounding box.
		MUU_PURE_INLINE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VC_PARAM(vector_type) p0,
														MUU_VC_PARAM(vector_type) p1,
														MUU_VC_PARAM(bounding_box<scalar_type>) bb) noexcept;

		/// \brief	Returns true if a line segment intersects a bounding box.
		MUU_PURE_INLINE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VC_PARAM(line_segment) seg,
														MUU_VC_PARAM(bounding_box<scalar_type>) bb) noexcept;

		/// \brief	Returns true if a line segment intersects a bounding box.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL intersects(MUU_VC_PARAM(bounding_box<scalar_type>) bb) const noexcept;

		/// \brief	Returns true if a line segment intersects a plane.
		MUU_PURE_INLINE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VC_PARAM(vector_type) p0,
														MUU_VC_PARAM(vector_type) p1,
														MUU_VC_PARAM(plane<scalar_type>) p) noexcept;

		/// \brief	Returns true if a line segment intersects a plane.
		MUU_PURE_INLINE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VC_PARAM(line_segment) seg,
														MUU_VC_PARAM(plane<scalar_type>) p) noexcept;

		/// \brief	Returns true if a line segment intersects a plane.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL intersects(MUU_VC_PARAM(plane<scalar_type>) p) const noexcept;

			/// @}
	#endif // intersection and containment

	#if 1 // iterators -----------------------------------------------------------------------------------
		  /// \name Iterators
		  /// @{

		/// \brief Returns an iterator to the first point in the line segment.
		MUU_PURE_INLINE_GETTER
		constexpr iterator begin() noexcept
		{
			return base::points;
		}

		/// \brief Returns an iterator to the one-past-the-last point in the line segment.
		MUU_PURE_INLINE_GETTER
		constexpr iterator end() noexcept
		{
			return base::points + 2u;
		}

		/// \brief Returns a const iterator to the first point in the line segment.
		MUU_PURE_INLINE_GETTER
		constexpr const_iterator begin() const noexcept
		{
			return base::points;
		}

		/// \brief Returns a const iterator to the one-past-the-last point in the line segment.
		MUU_PURE_INLINE_GETTER
		constexpr const_iterator end() const noexcept
		{
			return base::points + 2u;
		}

		/// \brief Returns a const iterator to the first point in the line segment.
		MUU_PURE_INLINE_GETTER
		constexpr const_iterator cbegin() const noexcept
		{
			return base::points;
		}

		/// \brief Returns a const iterator to the one-past-the-last point in the line segment.
		MUU_PURE_INLINE_GETTER
		constexpr const_iterator cend() const noexcept
		{
			return base::points + 2u;
		}

		/// @}

		/// \name Iterators (ADL)
		/// @{

		/// \brief Returns an iterator to the first point in a line segment.
		MUU_PURE_INLINE_GETTER
		friend constexpr iterator begin(line_segment& v) noexcept
		{
			return v.begin();
		}

		/// \brief Returns an iterator to the one-past-the-last point in a line segment.
		MUU_PURE_INLINE_GETTER
		friend constexpr iterator end(line_segment& v) noexcept
		{
			return v.end();
		}

		/// \brief Returns a const iterator to the first point in a line segment.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_iterator begin(const line_segment& v) noexcept
		{
			return v.begin();
		}

		/// \brief Returns a const iterator to the one-past-the-last point in a line segment.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_iterator end(const line_segment& v) noexcept
		{
			return v.end();
		}

		/// \brief Returns a const iterator to the first point in a line segment.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_iterator cbegin(const line_segment& v) noexcept
		{
			return v.begin();
		}

		/// \brief Returns a const iterator to the one-past-the-last point in a line segment.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_iterator cend(const line_segment& v) noexcept
		{
			return v.end();
		}

			/// @}
	#endif // iterators

	#if 1 // misc -----------------------------------------------------------------------------------------

		/// \brief Writes a line segment out to a text stream.
		template <typename Char, typename Traits>
		friend std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os,
															const line_segment& seg)
		{
			const impl::compound_vector_elem<Scalar> elems[]{ { &seg.points[0].x, 3 }, //
															  { &seg.points[1].x, 3 } };
			impl::print_compound_vector(os, elems);
			return os;
		}

	#endif // misc
	};

	/// \cond

	template <typename T1, typename T2>
	line_segment(vector<T1, 3>, vector<T2, 3>) -> line_segment<impl::highest_ranked<T1, T2>>;

	template <typename T>
	line_segment(const vector<T, 3> (&)[2]) -> line_segment<T>;

	MUU_CONSTRAINED_TEMPLATE((all_arithmetic<X1, Y1, Z1, X2, Y2, Z2>),
							 typename X1,
							 typename Y1,
							 typename Z1,
							 typename X2,
							 typename Y2,
							 typename Z2)
	line_segment(X1, Y1, Z1, X2, Y2, Z2)->line_segment<impl::highest_ranked<X1, Y1, Z1, X2, Y2, Z2>>;

	template <typename T>
	line_segment(const line_segment<T>&) -> line_segment<T>;

	/// \endcond
}

namespace std
{
	/// \brief Specialization of std::tuple_size for muu::line_segment.
	template <typename Scalar>
	struct tuple_size<muu::line_segment<Scalar>>
	{
		static constexpr size_t value = 2;
	};

	/// \brief Specialization of std::tuple_element for muu::line_segment.
	template <size_t P, typename Scalar>
	struct tuple_element<P, muu::line_segment<Scalar>>
	{
		static_assert(P < 2);

		using type = muu::vector<Scalar, 3>;
	};
}

#endif //===============================================================================================================

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"

/// \cond
#include "impl/bounding_box_x_line_segment.h"
#include "impl/plane_x_line_segment.h"
/// \endcond
