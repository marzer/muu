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
		using aabbs		 = impl::aabbs_common<scalar_type>;
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

	#if 1 // collision detection ------------------------------------------------------------------------------
		/// \name Collision detection
		/// @{

		/// \brief Creates an #muu::intersection_tester for this bounding sphere.
		MUU_PURE_GETTER
		constexpr muu::intersection_tester<bounding_sphere> MUU_VECTORCALL intersection_tester() noexcept;

		//--------------------------------
		// sphere x point
		//--------------------------------

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

		/// \brief	Returns true if the bounding sphere contains all the points in an arbitrary collection.
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL contains(const vector_type* begin, const vector_type* end) noexcept
		{
			if (begin == end)
				return false;

			MUU_ASSUME(begin != nullptr);
			MUU_ASSUME(end != nullptr);
			MUU_ASSUME(begin < end);

			for (; begin != end; begin++)
				if (!contains(*begin))
					return false;

			return true;
		}

		//--------------------------------
		// sphere x line segment
		//--------------------------------

		/// \brief	Returns true if a bounding sphere contains a line segment.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(bounding_sphere) bs,
													  MUU_VPARAM(vector_type) start,
													  MUU_VPARAM(vector_type) end) noexcept
		{
			return contains(bs, start) && contains(bs, end);
		}

		/// \brief	Returns true if a bounding sphere contains a line segment.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(bounding_sphere) bb,
													  MUU_VPARAM(line_segment<scalar_type>) seg) noexcept;

		/// \brief	Returns true if the bounding sphere contains a line segment.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(line_segment<scalar_type>) seg) const noexcept
		{
			return contains(*this, seg);
		}

		//--------------------------------
		// sphere x plane
		//--------------------------------

		//--------------------------------
		// sphere x triangle
		//--------------------------------

		/// \brief	Returns true if a bounding sphere contains a triangle.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(bounding_sphere) bs,
													  MUU_VPARAM(vector_type) p0,
													  MUU_VPARAM(vector_type) p1,
													  MUU_VPARAM(vector_type) p2) noexcept
		{
			return contains(bs, p0) && contains(bs, p1) && contains(bs, p2);
		}

		/// \brief	Returns true if a bounding sphere contains a triangle.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(bounding_sphere) bs,
													  MUU_VPARAM(triangle<scalar_type>) tri) noexcept;

		/// \brief	Returns true if the bounding sphere contains a triangle.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(triangle<scalar_type>) tri) const noexcept
		{
			return contains(*this, tri);
		}

		/// \brief	Returns true if a bounding sphere intersects a triangle.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(bounding_sphere) bs,
														MUU_VPARAM(vector_type) p0,
														MUU_VPARAM(vector_type) p1,
														MUU_VPARAM(vector_type) p2) noexcept
		{
			// https://realtimecollisiondetection.net/blog/?p=103

			p0 -= bs.center;
			p1 -= bs.center;
			p2 -= bs.center;
			const auto rr = bs.radius * bs.radius;
			const auto V  = vector_type::cross(p1 - p0, p2 - p0);
			const auto d  = vector_type::dot(p0, V);
			const auto e  = vector_type::dot(V, V);
			if (d * d > rr * e)
				return false;

			const auto aa	= vector_type::dot(p0, p0);
			const auto ab	= vector_type::dot(p0, p1);
			const auto ac	= vector_type::dot(p0, p2);
			const auto bb	= vector_type::dot(p1, p1);
			const auto bc	= vector_type::dot(p1, p2);
			const auto cc	= vector_type::dot(p2, p2);
			const auto sep2 = (aa > rr) && (ab > aa) && (ac > aa);
			const auto sep3 = (bb > rr) && (ab > bb) && (bc > bb);
			const auto sep4 = (cc > rr) && (ac > cc) && (bc > cc);
			if (sep2 || sep3 || sep4)
				return false;

			const auto AB	= p1 - p0;
			const auto BC	= p2 - p1;
			const auto CA	= p0 - p2;
			const auto d1	= ab - aa;
			const auto d2	= bc - bb;
			const auto d3	= ac - cc;
			const auto e1	= vector_type::dot(AB, AB);
			const auto e2	= vector_type::dot(BC, BC);
			const auto e3	= vector_type::dot(CA, CA);
			const auto Q1	= p0 * e1 - d1 * AB;
			const auto Q2	= p1 * e2 - d2 * BC;
			const auto Q3	= p2 * e3 - d3 * CA;
			const auto QC	= p2 * e1 - Q1;
			const auto QA	= p0 * e2 - Q2;
			const auto QB	= p1 * e3 - Q3;
			const auto sep5 = (vector_type::dot(Q1, Q1) > rr * e1 * e1) && (vector_type::dot(Q1, QC) > scalar_type{});
			const auto sep6 = (vector_type::dot(Q2, Q2) > rr * e2 * e2) && (vector_type::dot(Q2, QA) > scalar_type{});
			const auto sep7 = (vector_type::dot(Q3, Q3) > rr * e3 * e3) && (vector_type::dot(Q3, QB) > scalar_type{});
			if (sep5 || sep6 || sep7)
				return false;

			return true;
		}

		/// \brief	Returns true if a bounding sphere intersects a triangle.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(bounding_sphere) bs,
														MUU_VPARAM(triangle<scalar_type>) tri) noexcept;

		/// \brief	Returns true if the bounding sphere intersects a triangle.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(triangle<scalar_type>) tri) const noexcept
		{
			return intersects(*this, tri);
		}

		//--------------------------------
		// sphere x sphere
		//--------------------------------

		/// \brief	Returns true if a bounding sphere contains all the points of another bounding sphere.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(bounding_sphere) outer,
													  MUU_VPARAM(bounding_sphere) inner) noexcept
		{
			const auto dist_squared		 = vector_type::distance_squared(outer.center, inner.center);
			const auto outer_rad_squared = outer.radius * outer.radius;
			const auto inner_rad_squared = inner.radius * inner.radius;

			if (dist_squared > (outer_rad_squared + inner_rad_squared))
				return false;

			return outer_rad_squared >= dist_squared + inner_rad_squared;
		}

		/// \brief	Returns true if the bounding sphere contains all the points of another bounding sphere.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(bounding_sphere) bs) const noexcept
		{
			return contains(*this, bs);
		}

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

		//--------------------------------
		// sphere x aabb
		//--------------------------------

		/// \brief	Returns true if a bounding sphere contains all the points of an axis-aligned bounding box.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(bounding_sphere) outer,
													  MUU_VPARAM(bounding_box<scalar_type>) inner) noexcept;

		/// \brief	Returns true if the bounding sphere contains all the points of an axis-aligned bounding box.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(bounding_box<scalar_type>) bb) const noexcept
		{
			return contains(*this, bb);
		}

		/// \brief	Returns true if a bounding sphere intersects a bounding box.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(bounding_sphere) bs,
														MUU_VPARAM(bounding_box<scalar_type>) bb) noexcept;

		/// \brief	Returns true if the bounding sphere intersects a bounding box.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(bounding_box<scalar_type>) bb) const noexcept
		{
			return intersects(*this, bb);
		}

			//--------------------------------
			// sphere x obb
			//--------------------------------

			/// @}
	#endif // collision detection

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
// INTERSECTION TESTER
#if 1

/// \cond
namespace muu
{
	template <typename Scalar>
	struct intersection_tester<bounding_sphere<Scalar>>
	{
		using scalar_type = Scalar;
		using vector_type = vector<scalar_type, 3>;
		using sphere_type = bounding_sphere<scalar_type>;

	  private:
		using sat_tester = muu::sat_tester<scalar_type, 3>;
		using aabbs		 = impl::aabbs_common<scalar_type>;
		using spheres	 = impl::spheres_common<scalar_type>;
		using triangles	 = impl::triangles_common<scalar_type>;

	  public:
		vector_type center;
		scalar_type radius;
		scalar_type radius_squared;
		vector_type min;
		vector_type max;

		MUU_NODISCARD_CTOR
		intersection_tester() noexcept = default;

		MUU_NODISCARD_CTOR
		explicit constexpr intersection_tester(const sphere_type& bs) noexcept //
			: center{ bs.center },
			  radius{ bs.radius },
			  radius_squared{ bs.radius * bs.radius },
			  min{ bs.center - vector_type{ radius } },
			  max{ bs.center + vector_type{ radius } }
		{}

		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL operator()(MUU_VPARAM(sphere_type) bs) const noexcept
		{
			return vector_type::distance_squared(center, bs.center) <= (radius_squared + bs.radius * bs.radius);
		}

		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL operator()(const intersection_tester& tester) const noexcept
		{
			return vector_type::distance_squared(center, tester.center) <= (radius_squared + tester.radius_squared);
		}

		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL operator()(MUU_VPARAM(bounding_box<scalar_type>)) const noexcept;

		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL operator()(const intersection_tester<bounding_box<scalar_type>>&) const noexcept;
	};

	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr muu::intersection_tester<bounding_sphere<Scalar>> MUU_VECTORCALL bounding_sphere<
		Scalar>::intersection_tester() noexcept
	{
		return muu::intersection_tester<bounding_sphere<Scalar>>{ *this };
	}
}
/// \endcond

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

/// \cond
#include "impl/bounding_sphere_x_triangle.h"
#include "impl/bounding_sphere_x_line_segment.h"
#include "impl/bounding_box_x_bounding_sphere.h"
/// \endcond
