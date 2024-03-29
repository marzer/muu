// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#ifndef MUU_BOUNDING_BOX_H
#define MUU_BOUNDING_BOX_H

/// \file
/// \brief  Contains the definition of muu::bounding_box.

#include "sat_tester.h"
#include "impl/geometry_common.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(float_control(except, off))

//======================================================================================================================
// BOUNDING BOX CLASS
#if 1

namespace muu
{
	/// \brief An axis-aligned bounding box.
	/// \ingroup math
	/// \image html diag_bounding_box.svg
	///
	/// \tparam	Scalar      The bounding box's scalar component type. Must be a floating-point type.
	///
	/// \see [Aligned Bounding Box](https://www.sciencedirect.com/topics/computer-science/aligned-bounding-box)
	template <typename Scalar>
	struct MUU_TRIVIAL_ABI bounding_box //
		MUU_HIDDEN_BASE(impl::storage_base<bounding_box<Scalar>>)
	{
		static_assert(!std::is_reference_v<Scalar>, "Bounding box scalar type cannot be a reference");
		static_assert(!is_cv<Scalar>, "Bounding box scalar type cannot be const- or volatile-qualified");
		static_assert(std::is_trivially_constructible_v<Scalar>	  //
						  && std::is_trivially_copyable_v<Scalar> //
						  && std::is_trivially_destructible_v<Scalar>,
					  "Bounding box scalar type must be trivially constructible, copyable and destructible");
		static_assert(is_signed<Scalar>, "Bounding box scalar type must be signed");
		static_assert(is_floating_point<Scalar>, "Bounding box scalar type must be a floating-point type");

		/// \brief The bounding box's scalar type.
		using scalar_type = Scalar;

		/// \brief The three-dimensional #muu::vector with the same #scalar_type as the bounding box.
		using vector_type = vector<scalar_type, 3>;

		/// \brief Compile-time bounding box constants.
		using constants = muu::constants<bounding_box>;

	  private:
		/// \cond
		using base = impl::storage_base<bounding_box<Scalar>>;
		static_assert(sizeof(base) == (sizeof(vector_type) * 2), "Bounding boxes should not have padding");

		using boxes		 = impl::boxes_common<Scalar>;
		using aabbs		 = impl::aabbs_common<Scalar>;
		using triangles	 = impl::triangles_common<Scalar>;
		using sat_tester = muu::sat_tester<Scalar, 3>;

		using promoted_scalar					 = promote_if_small_float<scalar_type>;
		using promoted_vec3						 = vector<promoted_scalar, 3>;
		using promoted_box						 = bounding_box<promoted_scalar>;
		static constexpr bool requires_promotion = impl::is_small_float_<scalar_type>;

		/// \endcond

	  public:
	#if MUU_DOXYGEN

		/// \brief	The center of the box.
		vector_type center;

		/// \brief	The half-lengths of box (i.e. distances from the center to the sides).
		vector_type extents;

	#endif // DOXYGEN

	#if 1 // constructors ----------------------------------------------------------------------------------------------

		/// \brief Default constructor. Values are not initialized.
		bounding_box() noexcept = default;

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr bounding_box(const bounding_box&) noexcept = default;

		/// \brief Copy-assigment operator.
		constexpr bounding_box& operator=(const bounding_box&) noexcept = default;

		/// \brief	Constructs a bounding box from center and extent values.
		///
		/// \param	cen		The center point.
		/// \param	ext		The extents.
		MUU_NODISCARD_CTOR
		constexpr bounding_box(const vector_type& cen, const vector_type& ext) noexcept //
			: base{ cen, ext }
		{}

		/// \brief	Constructs a bounding box from center and extent values.
		///
		/// \param	cen		The center point.
		/// \param	ext_x	The length of the X extent.
		/// \param	ext_y	The length of the Y extent.
		/// \param	ext_z	The length of the Z extent.
		MUU_NODISCARD_CTOR
		constexpr bounding_box(const vector_type& cen, scalar_type ext_x, scalar_type ext_y, scalar_type ext_z) noexcept
			: base{ cen, { ext_x, ext_y, ext_z } }
		{}

		/// \brief	Constructs a uniformly-sized bounding box.
		///
		/// \param	cen		The center point.
		/// \param	ext		The length of all three extents.
		MUU_NODISCARD_CTOR
		constexpr bounding_box(const vector_type& cen, scalar_type ext) noexcept //
			: base{ cen, { ext, ext, ext } }
		{}

		/// \brief	Constructs a bounding box at the origin.
		///
		/// \param	ext		The extents.
		MUU_NODISCARD_CTOR
		explicit constexpr bounding_box(const vector_type& ext) noexcept //
			: base{ vector_type::constants::zero, ext }
		{}

		/// \brief	Constructs a bounding box from center and extent values.
		///
		/// \param	cen_x	The X component of the center point.
		/// \param	cen_y	The Y component of the center point.
		/// \param	cen_z	The Z component of the center point.
		/// \param	ext		The extents.
		MUU_NODISCARD_CTOR
		constexpr bounding_box(scalar_type cen_x, scalar_type cen_y, scalar_type cen_z, const vector_type& ext) noexcept
			: base{ { cen_x, cen_y, cen_z }, ext }
		{}

		/// \brief	Constructs a bounding box from center and extent values.
		///
		/// \param	cen_x	The X component of the center point.
		/// \param	cen_y	The Y component of the center point.
		/// \param	cen_z	The Z component of the center point.
		/// \param	ext_x	The length of the X extent.
		/// \param	ext_y	The length of the Y extent.
		/// \param	ext_z	The length of the Z extent.
		MUU_NODISCARD_CTOR
		constexpr bounding_box(scalar_type cen_x,
							   scalar_type cen_y,
							   scalar_type cen_z,
							   scalar_type ext_x,
							   scalar_type ext_y,
							   scalar_type ext_z) noexcept
			: base{ { cen_x, cen_y, cen_z }, { ext_x, ext_y, ext_z } }
		{}

		/// \brief	Constructs a uniformly-sized bounding box at the origin.
		///
		/// \param	ext		The length of all three extents.
		MUU_NODISCARD_CTOR
		explicit constexpr bounding_box(scalar_type ext) noexcept //
			: base{ vector_type::constants::zero, { ext, ext, ext } }
		{}

		/// \brief	Converting constructor.
		template <typename S>
		MUU_NODISCARD_CTOR
		explicit constexpr bounding_box(const bounding_box<S>& bb) noexcept //
			: base{ vector_type{ bb.center }, vector_type{ bb.extents } }
		{}

		/// \brief Constructs a bounding box from an implicitly bit-castable type.
		///
		/// \tparam T	A bit-castable type.
		///
		/// \see muu::allow_implicit_bit_cast
		MUU_CONSTRAINED_TEMPLATE((allow_implicit_bit_cast<T, bounding_box>), typename T)
		MUU_NODISCARD_CTOR
		/*implicit*/
		constexpr bounding_box(const T& obj) noexcept //
			: base{ muu::bit_cast<base>(obj) }
		{
			static_assert(sizeof(T) == sizeof(base), "Bit-castable types must be the same size");
			static_assert(std::is_trivially_copyable_v<T>, "Bit-castable types must be trivially-copyable");
		}

		/// \brief	Constructs a bounding box from min and max corners.
		MUU_PURE_GETTER
		static constexpr bounding_box MUU_VECTORCALL from_min_max(MUU_VPARAM(vector_type) min_,
																  MUU_VPARAM(vector_type) max_) noexcept
		{
			if constexpr (requires_promotion)
			{
				return bounding_box{
					promoted_box::from_min_max(promoted_vec3{ min_ }, promoted_vec3{ max_ }),
				};
			}
			else
			{
				return bounding_box{ aabbs::center(min_, max_), aabbs::extents(min_, max_) };
			}
		}

		/// \brief	Constructs a bounding box fitting two or more points.
		MUU_CONSTRAINED_TEMPLATE((sizeof...(T) == 0 || all_convertible_to<vector_type, const T&...>), typename... T)
		MUU_PURE_GETTER
		static constexpr bounding_box MUU_VECTORCALL from_points(MUU_VPARAM(vector_type) pt0,
																 MUU_VPARAM(vector_type) pt1,
																 const T&... pts) noexcept
		{
			if constexpr (sizeof...(T) && !all_same<vector_type, remove_cvref<T>...>)
			{
				return from_points(pt0, pt1, static_cast<vector_type>(pts)...);
			}
			else
			{
				return from_min_max(vector_type::min(pt0, pt1, pts...), vector_type::max(pt0, pt1, pts...));
			}
		}

		/// \brief	Constructs a bounding box fitting a range of points.
		MUU_PURE_GETTER
		MUU_ATTR(nonnull)
		static constexpr bounding_box MUU_VECTORCALL from_points(const vector_type* begin,
																 const vector_type* end) noexcept
		{
			if MUU_UNLIKELY(begin == end)
				return bounding_box{};

			return from_min_max(vector_type::min(begin, end), vector_type::max(begin, end));
		}

		/// \brief	Constructs a bounding box fitting an array of points.
		template <size_t N>
		MUU_PURE_INLINE_GETTER
		static constexpr bounding_box MUU_VECTORCALL from_points(const vector_type (&points)[N]) noexcept
		{
			return from_points(points, points + N);
		}

		/// \brief	Constructs a bounding box fitting a range of points.
		MUU_NODISCARD_CTOR
		MUU_ATTR(nonnull)
		explicit constexpr bounding_box(const vector_type* begin, const vector_type* end) noexcept //
			: bounding_box{ from_points(begin, end) }
		{}

		/// \brief	Constructs a bounding box fitting a range of points.
		MUU_NODISCARD_CTOR
		MUU_ATTR(nonnull)
		explicit constexpr bounding_box(const vector_type* points, size_t count) noexcept //
			: bounding_box{ from_points(points, points + count) }
		{}

		/// \brief	Constructs a bounding box fitting an array of points.
		template <size_t N>
		MUU_NODISCARD_CTOR
		explicit constexpr bounding_box(const vector_type (&points)[N]) noexcept //
			: bounding_box{ from_points(points) }
		{}

		/// \brief	Constructs a bounding box fitting an array of points.
		template <size_t N>
		MUU_NODISCARD_CTOR
		explicit constexpr bounding_box(const span<vector_type, N>& vals) noexcept //
			: bounding_box{ from_points(vals.data(), vals.data() + vals.size()) }
		{}

		/// \brief	Constructs a bounding box fitting an array of points.
		template <size_t N>
		MUU_NODISCARD_CTOR
		explicit constexpr bounding_box(const const_span<vector_type, N>& vals) noexcept //
			: bounding_box{ from_points(vals.data(), vals.data() + vals.size()) }
		{}

		/// \brief	Constructs a bounding box completely containing an oriented bounding-box.
		MUU_PURE_GETTER
		static constexpr bounding_box MUU_VECTORCALL from_obb(MUU_VPARAM(oriented_bounding_box<scalar_type>)
																  obb) noexcept;

		/// \brief	Constructs a bounding box completely containing an oriented bounding-box.
		MUU_NODISCARD_CTOR
		explicit constexpr bounding_box(MUU_VPARAM(oriented_bounding_box<scalar_type>) obb) noexcept //
			: bounding_box{ from_obb(obb) }
		{}

		/// \brief	Constructs a bounding box completely containing a bounding sphere.
		MUU_PURE_GETTER
		static constexpr bounding_box MUU_VECTORCALL from_sphere(MUU_VPARAM(bounding_sphere<scalar_type>) bs) noexcept;

		/// \brief	Constructs a bounding box completely containing a bounding sphere.
		MUU_NODISCARD_CTOR
		explicit constexpr bounding_box(MUU_VPARAM(bounding_sphere<scalar_type>) bs) noexcept //
			: bounding_box{ from_sphere(bs) }
		{}

	#endif // constructors

	#if 1 // scalar accessors ------------------------------------------------------------------------------------------
		/// \name Scalar accessors
		/// @{

		/// \brief Returns a pointer to the first scalar component in the bounding box.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type* data() noexcept
		{
			return base::center.data();
		}

		/// \brief Returns a pointer to the first scalar component in the bounding box.
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

		/// \brief	Returns the width of the box (x-axis).
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type width() const noexcept
		{
			return aabbs::width(base::extents);
		}

		/// \brief	Returns the height of the box (y-axis).
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type height() const noexcept
		{
			return aabbs::height(base::extents);
		}

		/// \brief	Returns the depth of the box (z-axis).
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type depth() const noexcept
		{
			return aabbs::depth(base::extents);
		}

		/// \brief	Calculates the length of the line connecting the min and max points.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type diagonal() const noexcept
		{
			return aabbs::diagonal(base::extents);
		}

		/// \brief	Returns the shortest of the box's three extents.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type& shortest_extent() noexcept
		{
			return aabbs::shortest_extent(base::extents);
		}

		/// \brief	Returns the longest of the box's three extents.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type& longest_extent() noexcept
		{
			return aabbs::longest_extent(base::extents);
		}

		/// \brief	Returns the shortest of the box's three extents (const overload).
		MUU_PURE_INLINE_GETTER
		constexpr const scalar_type& shortest_extent() const noexcept
		{
			return aabbs::shortest_extent(base::extents);
		}

		/// \brief	Returns the longest of the box's three extents (const overload).
		MUU_PURE_INLINE_GETTER
		constexpr const scalar_type& longest_extent() const noexcept
		{
			return aabbs::longest_extent(base::extents);
		}

		/// \brief	Returns the length of the shortest of the box's three sides.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type shortest_side() const noexcept
		{
			return aabbs::shortest_side(base::extents);
		}

		/// \brief	Returns the length of the longest of the box's three sides.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type longest_side() const noexcept
		{
			return aabbs::longest_side(base::extents);
		}

		/// \brief	Calculates the volume of this bounding box.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type volume() const noexcept
		{
			return aabbs::volume(base::extents);
		}

		/// \brief	Calculates the mass of this box if it had a given density.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type MUU_VECTORCALL mass(scalar_type density) const noexcept
		{
			return density * volume();
		}

		/// \brief	Calculates the density of this box if it had a given mass.
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

		/// \brief		Returns true if two bounding boxes are exactly equal.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		template <typename T>
		MUU_PURE_GETTER
		friend constexpr bool MUU_VECTORCALL operator==(MUU_VPARAM(bounding_box) lhs,
														const bounding_box<T>& rhs) noexcept
		{
			return lhs.center == rhs.center && lhs.extents == rhs.extents;
		}

		/// \brief	Returns true if two bounding boxes are not exactly equal.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		template <typename T>
		MUU_PURE_GETTER
		friend constexpr bool MUU_VECTORCALL operator!=(MUU_VPARAM(bounding_box) lhs,
														const bounding_box<T>& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		/// \brief	Returns true if all the scalar components of a bounding box are exactly zero.
		///
		/// \remarks	This is an exact check;
		///				use #approx_zero() if you want an epsilon-based "near-enough" check.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL zero(MUU_VPARAM(bounding_box) bb) noexcept
		{
			return vector_type::zero(bb.center) && vector_type::zero(bb.extents);
		}

		/// \brief	Returns true if all the scalar components of the bounding box are exactly zero.
		///
		/// \remarks	This is an exact check;
		///				use #approx_zero() if you want an epsilon-based "near-enough" check.
		MUU_PURE_INLINE_GETTER
		constexpr bool zero() const noexcept
		{
			return zero(*this);
		}

		/// \brief	Returns true if a bounding box has exactly zero volume.
		///
		/// \remarks	This is an exact check;
		///				use #approx_empty() if you want an epsilon-based "near-enough" check.
		MUU_PURE_INLINE_GETTER
		static constexpr bool MUU_VECTORCALL empty(MUU_VPARAM(bounding_box) bb) noexcept
		{
			return vector_type::zero(bb.extents);
		}

		/// \brief	Returns true if the bounding box has exactly zero volume.
		///
		/// \remarks	This is an exact check;
		///				use #approx_empty() if you want an epsilon-based "near-enough" check.
		MUU_PURE_INLINE_GETTER
		constexpr bool empty() const noexcept
		{
			return vector_type::zero(base::extents);
		}

		/// \brief	Returns true if any of the scalar components of a bounding box are infinity or NaN.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL infinity_or_nan(MUU_VPARAM(bounding_box) bb) noexcept
		{
			return vector_type::infinity_or_nan(bb.center) || vector_type::infinity_or_nan(bb.extents);
		}

		/// \brief	Returns true if any of the scalar components of the bounding box are infinity or NaN.
		MUU_PURE_INLINE_GETTER
		constexpr bool infinity_or_nan() const noexcept
		{
			return infinity_or_nan(*this);
		}

		/// \brief	Returns true if a box is degenerate (i.e. any of its extents are less than or equal to zero).
		MUU_PURE_INLINE_GETTER
		static constexpr bool MUU_VECTORCALL degenerate(MUU_VPARAM(bounding_box) bb) noexcept
		{
			return aabbs::degenerate(bb.extents);
		}

		/// \brief	Returns true if the box is degenerate (i.e. any of its extents are less than or equal to zero).
		MUU_PURE_INLINE_GETTER
		constexpr bool degenerate() const noexcept
		{
			return aabbs::degenerate(base::extents);
		}

				/// @}
	#endif // equality (exact)

	#if 1 // equality (approx) -----------------------------------------------------------------------------------------
		/// \name Equality (approximate)
		/// @{

		/// \brief	Returns true if two bounding boxes are approximately equal.
		template <typename T>
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_equal(
			MUU_VPARAM(bounding_box) bb1,
			const bounding_box<T>& bb2,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) noexcept
		{
			return vector_type::approx_equal(bb1.center, bb2.center, epsilon)
				&& vector_type::approx_equal(bb1.extents, bb2.extents, epsilon);
		}

		/// \brief	Returns true if the bounding box is approximately equal to another.
		template <typename T>
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL approx_equal(
			const bounding_box<T>& bb,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) const noexcept
		{
			return approx_equal(*this, bb, epsilon);
		}

		/// \brief	Returns true if all the scalar components in a bounding box are approximately equal to zero.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_zero(MUU_VPARAM(bounding_box) bb,
														 scalar_type epsilon = default_epsilon<scalar_type>) noexcept
		{
			return vector_type::approx_zero(bb.center, epsilon) && vector_type::approx_zero(bb.extents, epsilon);
		}

		/// \brief	Returns true if all the scalar components in the bounding box are approximately equal to zero.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL approx_zero(scalar_type epsilon = default_epsilon<scalar_type>) const noexcept
		{
			return approx_zero(*this, epsilon);
		}

		/// \brief	Returns true if a bounding box has approximately zero volume.
		MUU_PURE_INLINE_GETTER
		static constexpr bool MUU_VECTORCALL approx_empty(MUU_VPARAM(bounding_box) bb,
														  scalar_type epsilon = default_epsilon<scalar_type>) noexcept
		{
			return vector_type::approx_zero(bb.extents, epsilon);
		}

		/// \brief	Returns true if the bounding box has approximately zero volume.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL approx_empty(scalar_type epsilon = default_epsilon<scalar_type>) const noexcept
		{
			return vector_type::approx_zero(base::extents, epsilon);
		}

				/// @}
	#endif // equality (approx)

	#if 1 // corners ---------------------------------------------------------------------------------------------------
		/// \name Corners
		/// @{

		/// \brief	Returns a specific corner of a bounding box.
		template <box_corner Corner>
		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL corner(MUU_VPARAM(bounding_box) bb) noexcept
		{
			return aabbs::template corner<Corner>(bb.center, bb.extents);
		}

		/// \brief	Returns a specific corner of the bounding box.
		template <box_corner Corner>
		MUU_PURE_INLINE_GETTER
		constexpr vector_type corner() const noexcept
		{
			return aabbs::template corner<Corner>(base::center, base::extents);
		}

		/// \brief	Returns a specific corner of a bounding box.
		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL corner(MUU_VPARAM(bounding_box) bb, box_corner which) noexcept
		{
			return aabbs::corner(bb.center, bb.extents, which);
		}

		/// \brief	Returns a specific corner of the bounding box.
		MUU_PURE_INLINE_GETTER
		constexpr vector_type corner(box_corner which) const noexcept
		{
			return aabbs::corner(base::center, base::extents, which);
		}

		/// \brief	Returns the 'min' corner of a bounding box.
		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL min_corner(MUU_VPARAM(bounding_box) bb) noexcept
		{
			return aabbs::template corner<box_corner::min>(bb.center, bb.extents);
		}

		/// \brief	Returns the 'min' corner of the bounding box.
		MUU_PURE_INLINE_GETTER
		constexpr vector_type min_corner() const noexcept
		{
			return aabbs::template corner<box_corner::min>(base::center, base::extents);
		}

		/// \brief	Returns the 'max' corner of a bounding box.
		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL max_corner(MUU_VPARAM(bounding_box) bb) noexcept
		{
			return aabbs::template corner<box_corner::max>(bb.center, bb.extents);
		}

		/// \brief	Returns the 'max' corner of the bounding box.
		MUU_PURE_INLINE_GETTER
		constexpr vector_type max_corner() const noexcept
		{
			return aabbs::template corner<box_corner::max>(base::center, base::extents);
		}

				/// @}
	#endif // corners

	#if 1  // translation -------------------------------------------------------------------
		/// \name Translation
		/// @{

		/// \brief	Translates a bounding box.
		///
		/// \param	bb		The bounding box to translate.
		/// \param	offset	An offset to add to the box's center position.
		///
		/// \returns	A copy of the input box translated by the given offset.
		MUU_PURE_GETTER
		static constexpr bounding_box MUU_VECTORCALL translate(MUU_VPARAM(bounding_box) bb,
															   MUU_VPARAM(vector_type) offset) noexcept
		{
			return bounding_box{ bb.center + offset, bb.extents };
		}

		/// \brief	Translates the bounding box (in-place).
		///
		/// \param	offset	An offset to add to the box's center position.
		///
		/// \return	A reference to the box_.
		constexpr bounding_box& MUU_VECTORCALL translate(MUU_VPARAM(vector_type) offset) noexcept
		{
			base::center += offset;
			return *this;
		}

				/// @}
	#endif // translation

	#if 1  // scaling -------------------------------------------------------------------
		/// \name Scaling
		/// @{

		/// \brief	Scales a bounding box.
		///
		/// \param	bb		The bounding box to scale.
		/// \param	scale_	The amount to scale the box extents by on each axis.
		///
		/// \returns	A copy of the input box scaled by the given amounts.
		MUU_PURE_GETTER
		static constexpr bounding_box MUU_VECTORCALL scale(MUU_VPARAM(bounding_box) bb,
														   MUU_VPARAM(vector_type) scale_) noexcept
		{
			return bounding_box{ bb.center, bb.extents * scale_ };
		}

		/// \brief	Scales the bounding box (in-place).
		///
		/// \param	scale_	The amount to scale the box extents by on each axis.
		///
		/// \return	A reference to the box.
		constexpr bounding_box& MUU_VECTORCALL scale(MUU_VPARAM(vector_type) scale_) noexcept
		{
			base::extents *= scale_;
			return *this;
		}

				/// @}
	#endif // scaling

	#if 1  // transformation -------------------------------------------------------------------
		/// \name Transformation
		/// @{

		/// \brief Transforms an axis-aligned bounding box from one coordinate space to another.
		///
		/// \param	bb		The bounding box to transform.
		/// \param	tx		The transform to apply.
		///
		/// \return	Returns an axis-aligned bounding box containing all the points of the
		///			input bounding box after being transformed.
		MUU_PURE_GETTER
		static constexpr bounding_box MUU_VECTORCALL transform(MUU_VPARAM(bounding_box) bb,
															   MUU_VPARAM(matrix<scalar_type, 4, 4>) tx) noexcept
		{
			if constexpr (requires_promotion)
			{
				return bounding_box{ promoted_box::transform(promoted_box{ bb }, matrix<promoted_scalar, 4, 4>{ tx }) };
			}
			else
			{
				// http://dev.theomader.com/transform-bounding-boxes/

				MUU_FMA_BLOCK;

				auto min	  = bb.center - bb.extents;
				auto max	  = bb.center + bb.extents;
				const auto xa = vector_type{ tx.m[0] } * min.x;
				const auto xb = vector_type{ tx.m[0] } * max.x;
				const auto ya = vector_type{ tx.m[1] } * min.y;
				const auto yb = vector_type{ tx.m[1] } * max.y;
				const auto za = vector_type{ tx.m[2] } * min.z;
				const auto zb = vector_type{ tx.m[2] } * max.z;
				min			  = muu::min(xa, xb) + muu::min(ya, yb) + muu::min(za, zb);
				max			  = muu::max(xa, xb) + muu::max(ya, yb) + muu::max(za, zb);
				max			  = (max - min) / scalar_type{ 2 }; // extents
				return { min + max + vector_type{ tx.m[3] }, max };
			}
		}

		/// \brief Transforms the axis-aligned bounding box from one coordinate space to another (in-place).
		///
		/// \param	tx		The transform to apply.
		///
		/// \return	A reference to the box.
		constexpr bounding_box& transform(MUU_VPARAM(matrix<scalar_type, 4, 4>) tx) noexcept
		{
			return *this = transform(*this, tx);
		}

				/// @}
	#endif // transformation

	#if 1  // appending -------------------------------------------------------------------
		/// \name Appending
		/// @{

		/// \brief Adds a point to the bounding box (in-place), expanding the bounded region to contain it if necessary.
		///
		/// \param	pt		The point being added to the box's volume.
		///
		/// \return	A reference to the box.
		constexpr bounding_box& MUU_VECTORCALL append(MUU_VPARAM(vector_type) pt) noexcept
		{
			if constexpr (requires_promotion)
			{
				return *this = bounding_box{ promoted_box{ *this }.append(promoted_vec3{ pt }) };
			}
			else
			{
				return *this = from_min_max(vector_type::min(min_corner(*this), pt),
											vector_type::max(max_corner(*this), pt));
			}
		}

		/// \brief Adds two or more points to the bounding box (in-place), expanding the bounded region to contain them if necessary.
		///
		/// \param	pt1		The first point being added to the box's volume.
		/// \param	pt2		The second point being added to the box's volume.
		/// \param	pts		The remaining points being added to the box's volume.
		///
		/// \return	A reference to the box.
		MUU_CONSTRAINED_TEMPLATE((sizeof...(T) == 0 || all_convertible_to<vector_type, const T&...>), typename... T)
		constexpr bounding_box& MUU_VECTORCALL append(MUU_VPARAM(vector_type) pt1,
													  MUU_VPARAM(vector_type) pt2,
													  const T&... pts) noexcept
		{
			if constexpr (sizeof...(T) && !all_same<vector_type, remove_cvref<T>...>)
			{
				return append(pt1, pt2, static_cast<vector_type>(pts)...);
			}
			else if constexpr (requires_promotion)
			{
				return *this = bounding_box{
					promoted_box{ *this }.append(promoted_vec3{ pt1 }, promoted_vec3{ pt2 }, promoted_vec3{ pts }...)
				};
			}
			else
			{
				return *this = from_min_max(vector_type::min(min_corner(*this), pt1, pt2, pts...),
											vector_type::max(max_corner(*this), pt1, pt2, pts...));
			}
		}

		/// \brief Adds another box to the bounding box (in-place), expanding the bounded region to contain it if necessary.
		///
		/// \param	bb		The bounding box being added to the box's volume.
		///
		/// \return	A reference to the box.
		constexpr bounding_box& MUU_VECTORCALL append(MUU_VPARAM(bounding_box) bb) noexcept
		{
			return append(min_corner(bb), max_corner(bb));
		}

		/// \brief Adds a line segment to the bounding box (in-place), expanding the bounded region to contain it if necessary.
		///
		/// \param	seg		The line segment being added to the box's volume.
		///
		/// \return	A reference to the box.
		constexpr bounding_box& MUU_VECTORCALL append(MUU_VPARAM(line_segment<scalar_type>) seg) noexcept;

		/// \brief Adds a triangle to the bounding box (in-place), expanding the bounded region to contain it if necessary.
		///
		/// \param	tri		The triangle being added to the box's volume.
		///
		/// \return	A reference to the box.
		constexpr bounding_box& MUU_VECTORCALL append(MUU_VPARAM(triangle<scalar_type>) tri) noexcept;

	/// @}
	#endif // appending

	#if 1  // point queries ------------------------------------------------------------------------------
		/// \name Point queries
		/// @{

		/// \brief Gets point contained by the bounding box closest to another arbitrary point.
		MUU_PURE_GETTER
		constexpr vector_type MUU_VECTORCALL closest_point(MUU_VPARAM(vector_type) point) const noexcept
		{
			return aabbs::closest_min_max(base::center - base::extents, base::center + base::extents, point);
		}

				/// @}
	#endif // point queries

	#if 1  // collision detection ------------------------------------------------------------------------------
		/// \name Collision detection
		/// @{

		/// \brief Creates an #muu::collision_tester for this bounding box.
		MUU_PURE_GETTER
		constexpr muu::collision_tester<bounding_box> MUU_VECTORCALL collision_tester() const noexcept;

		//--------------------------------
		// aabb x point
		//--------------------------------

		/// \brief	Returns true if a bounding box contains a point.
		MUU_PURE_INLINE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(bounding_box) bb,
													  MUU_VPARAM(vector_type) point) noexcept
		{
			return aabbs::contains_point(bb.center, bb.extents, point);
		}

		/// \brief	Returns true if the bounding box contains a point.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(vector_type) point) const noexcept
		{
			return contains(*this, point);
		}

		/// \brief	Returns true if the bounding box contains all the points in an arbitrary collection.
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
		// aabb x line segment
		//--------------------------------

		/// \brief	Returns true if a bounding box contains a line segment.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(bounding_box) bb,
													  MUU_VPARAM(vector_type) start,
													  MUU_VPARAM(vector_type) end) noexcept
		{
			return contains(bb, start) && contains(bb, end);
		}

		/// \brief	Returns true if a bounding box contains a line segment.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(bounding_box) bb,
													  MUU_VPARAM(line_segment<scalar_type>) seg) noexcept;

		/// \brief	Returns true if the bounding box contains a line segment.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(line_segment<scalar_type>) seg) const noexcept
		{
			return contains(*this, seg);
		}

		/// \brief	Returns true if a bounding box intersects a line segment.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(bounding_box) bb,
														MUU_VPARAM(vector_type) start,
														MUU_VPARAM(vector_type) end) noexcept
		{
			MUU_FMA_BLOCK;

			const auto mid_delta = (end - start) / scalar_type{ 2 };
			const auto mid_point = start + mid_delta - bb.center;
			const auto abs_delta = vector_type::abs(mid_delta);

			if (muu::abs(mid_point.x) > bb.extents.x + abs_delta.x	  //
				|| muu::abs(mid_point.y) > bb.extents.y + abs_delta.y //
				|| muu::abs(mid_point.z) > bb.extents.z + abs_delta.z)
				return false;

			if (muu::abs(mid_delta.y * mid_point.z - mid_delta.z * mid_point.y)
					> bb.extents.y * abs_delta.z + bb.extents.z * abs_delta.y
				|| muu::abs(mid_delta.z * mid_point.x - mid_delta.x * mid_point.z)
					   > bb.extents.z * abs_delta.x + bb.extents.x * abs_delta.z
				|| muu::abs(mid_delta.x * mid_point.y - mid_delta.y * mid_point.x)
					   > bb.extents.x * abs_delta.y + bb.extents.y * abs_delta.x)
				return false;

			return true;
		}

		/// \brief	Returns true if a bounding box intersects a line segment.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(bounding_box) bb,
														MUU_VPARAM(line_segment<scalar_type>) seg) noexcept;

		/// \brief	Returns true if the bounding box intersects a line segment.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(line_segment<scalar_type>) seg) const noexcept
		{
			return intersects(*this, seg);
		}

		//--------------------------------
		// aabb x plane
		//--------------------------------

		/// \brief	Returns true if a bounding box intersects a plane.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(bounding_box) bb,
														MUU_VPARAM(plane<scalar_type>) p) noexcept;

		/// \brief	Returns true if the bounding box intersects a plane.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(plane<scalar_type>) p) const noexcept
		{
			return intersects(*this, p);
		}

		//--------------------------------
		// aabb x triangle
		//--------------------------------

		/// \brief	Returns true if a bounding box contains a triangle.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(bounding_box) bb,
													  MUU_VPARAM(vector_type) p0,
													  MUU_VPARAM(vector_type) p1,
													  MUU_VPARAM(vector_type) p2) noexcept
		{
			return contains(bb, p0) && contains(bb, p1) && contains(bb, p2);
		}

		/// \brief	Returns true if a bounding box contains a triangle.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(bounding_box) bb,
													  MUU_VPARAM(triangle<scalar_type>) tri) noexcept;

		/// \brief	Returns true if the bounding box contains a triangle.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(triangle<scalar_type>) tri) const noexcept
		{
			return contains(*this, tri);
		}

		/// \brief	Returns true if a bounding box intersects a triangle.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(bounding_box) bb,
														MUU_VPARAM(vector_type) p0,
														MUU_VPARAM(vector_type) p1,
														MUU_VPARAM(vector_type) p2) noexcept
		{
			return aabbs::intersects_tri(bb.center, bb.extents, p0, p1, p2);
		}

		/// \brief	Returns true if a bounding box intersects a triangle.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(bounding_box) bb,
														MUU_VPARAM(triangle<scalar_type>) tri) noexcept;

		/// \brief	Returns true if the bounding box intersects a triangle.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(triangle<scalar_type>) tri) const noexcept
		{
			return intersects(*this, tri);
		}

		//--------------------------------
		// aabb x sphere
		//--------------------------------

		/// \brief	Returns true if a bounding box contains all the points of a bounding sphere.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(bounding_box) bb,
													  MUU_VPARAM(bounding_sphere<scalar_type>) bs) noexcept;

		/// \brief	Returns true if a bounding box contains all the points of a bounding sphere.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(bounding_sphere<scalar_type>) bs) const noexcept
		{
			return contains(*this, bs);
		}

		/// \brief	Returns true if a bounding box intersects a bounding sphere.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(bounding_box) bb,
														MUU_VPARAM(bounding_sphere<scalar_type>) bs) noexcept;

		/// \brief	Returns true if the bounding box intersects a bounding sphere.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(bounding_sphere<scalar_type>) bs) const noexcept

		{
			return intersects(*this, bs);
		}

		//--------------------------------
		// aabb x aabb
		//--------------------------------

		/// \brief	Returns true if a bounding box contains all the points of another bounding box.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(bounding_box) outer,
													  MUU_VPARAM(bounding_box) inner) noexcept
		{
			return aabbs::contains_aabb_min_max(outer.min_corner(),
												outer.max_corner(),
												inner.min_corner(),
												inner.max_corner());
		}

		/// \brief	Returns true if a bounding box contains all the points of another bounding box.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(bounding_box) bb) const noexcept
		{
			return contains(*this, bb);
		}

		/// \brief	Returns true if two bounding boxes intersect.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(bounding_box) bb1,
														MUU_VPARAM(bounding_box) bb2) noexcept
		{
			return aabbs::intersects_aabb_min_max(bb1.min_corner(),
												  bb1.max_corner(),
												  bb2.min_corner(),
												  bb2.max_corner());
		}

		/// \brief	Returns true if two bounding boxes intersect.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(bounding_box) bb) const noexcept
		{
			return intersects(*this, bb);
		}

		//--------------------------------
		// aabb x obb
		//--------------------------------

		/// \brief	Returns true if an axis-aligned bounding box contains all the points of an oriented bounding box.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(bounding_box) bb,
													  MUU_VPARAM(oriented_bounding_box<scalar_type>) obb) noexcept;

		/// \brief	Returns true if an axis-aligned bounding box contains all the points of an oriented bounding box.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL contains(MUU_VPARAM(oriented_bounding_box<scalar_type>) obb) const noexcept
		{
			return contains(*this, obb);
		}

		/// \brief	Returns true if an axis-aligned bounding box intersects an oriented bounding box.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(bounding_box) aabb,
														MUU_VPARAM(oriented_bounding_box<scalar_type>) obb) noexcept;

		/// \brief	Returns true if the axis-aligned bounding box intersects an oriented bounding box.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(oriented_bounding_box<scalar_type>) obb) const noexcept

		{
			return intersects(*this, obb);
		}

				/// @}
	#endif // collision detection

	#if 1  // misc ---------------------------------------------------------------------------------------------------

		/// \brief Writes a bounding box out to a text stream.
		template <typename Char, typename Traits>
		friend std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os,
															const bounding_box& bb)
		{
			const impl::compound_vector_elem<Scalar> elems[]{ { &bb.center.x, 3 }, //
															  { &bb.extents.x, 3 } };
			impl::print_compound_vector(os, elems);
			return os;
		}

	#endif // misc
	};

	/// \cond

	template <typename CX, typename CY, typename CZ, typename EX, typename EY, typename EZ>
	bounding_box(CX, CY, CZ, EX, EY, EZ) -> bounding_box<impl::highest_ranked<CX, CY, CZ, EX, EY, EZ>>;

	template <typename C, typename E>
	bounding_box(vector<C, 3>, vector<E, 3>) -> bounding_box<impl::highest_ranked<C, E>>;

	MUU_CONSTRAINED_TEMPLATE((all_arithmetic<CX, CY, CZ>), typename CX, typename CY, typename CZ, typename E)
	bounding_box(CX, CY, CZ, vector<E, 3>) -> bounding_box<impl::highest_ranked<CX, CY, CZ, E>>;

	MUU_CONSTRAINED_TEMPLATE((all_arithmetic<EX, EY, EZ>), typename C, typename EX, typename EY, typename EZ)
	bounding_box(vector<C, 3>, EX, EY, EZ) -> bounding_box<impl::highest_ranked<C, EX, EY, EZ>>;

	MUU_CONSTRAINED_TEMPLATE(is_arithmetic<E>, typename C, typename E)
	bounding_box(vector<C, 3>, E) -> bounding_box<impl::highest_ranked<C, E>>;

	template <typename E>
	bounding_box(vector<E, 3>) -> bounding_box<E>;

	MUU_CONSTRAINED_TEMPLATE(is_arithmetic<E>, typename E)
	bounding_box(E) -> bounding_box<std::remove_cv_t<E>>;

	/// \endcond
}

namespace std
{
	/// \brief Specialization of std::tuple_size for muu::bounding_box.
	template <typename Scalar>
	struct tuple_size<muu::bounding_box<Scalar>>
	{
		static constexpr size_t value = 2;
	};

	/// \brief Specialization of std::tuple_element for muu::bounding_box.
	template <size_t I, typename Scalar>
	struct tuple_element<I, muu::bounding_box<Scalar>>
	{
		static_assert(I < 2);
		using type = muu::vector<Scalar, 3>;
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
	struct collision_tester<bounding_box<Scalar>>
	{
		using scalar_type = Scalar;
		using vector_type = vector<scalar_type, 3>;
		using box_type	  = bounding_box<scalar_type>;

	  private:
		using sat_tester = muu::sat_tester<scalar_type, 3>;
		using aabbs		 = impl::aabbs_common<scalar_type>;
		using triangles	 = impl::triangles_common<scalar_type>;

	  public:
		box_type box;
		vector_type min;
		vector_type max;
		vector_type corners[8];

		MUU_NODISCARD_CTOR
		collision_tester() noexcept = default;

		MUU_NODISCARD_CTOR
		explicit constexpr collision_tester(const box_type& bb) noexcept //
			: box{ bb },
			  min{ bb.min_corner() },
			  max{ bb.max_corner() },
			  corners{ min,
					   aabbs::template corner_min_max<box_corner::x>(min, max),
					   aabbs::template corner_min_max<box_corner::xy>(min, max),
					   aabbs::template corner_min_max<box_corner::xz>(min, max),
					   aabbs::template corner_min_max<box_corner::y>(min, max),
					   aabbs::template corner_min_max<box_corner::yz>(min, max),
					   aabbs::template corner_min_max<box_corner::z>(min, max),
					   max }
		{}

		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL operator()(MUU_VPARAM(vector_type) point) const noexcept
		{
			return box_type::contains(box, point);
		}

		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL operator()(MUU_VPARAM(box_type) bb) const noexcept
		{
			return aabbs::intersects_aabb_min_max(min, max, bb.min_corner(), bb.max_corner());
		}

		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL operator()(const collision_tester& tester) const noexcept
		{
			return aabbs::intersects_aabb_min_max(min, max, tester.min, tester.max);
		}

		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL operator()(MUU_VPARAM(vector_type) p0,
												 MUU_VPARAM(vector_type) p1,
												 MUU_VPARAM(vector_type) p2) const noexcept
		{
			if (!aabbs::intersects_tri_akenine_moller_1(min, max, p0, p1, p2))
				return false;

			if (!aabbs::intersects_tri_akenine_moller_2(corners, p0, triangles::normal(p0, p1, p2)))
				return false;

			const vector_type tri_edges[] = { p1 - p0, p2 - p1, p0 - p2 };
			if (!aabbs::intersects_tri_akenine_moller_3(corners, p0, p1, p2, tri_edges))
				return false;

			return true;
		}

		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL operator()(MUU_VPARAM(triangle<scalar_type>)) const noexcept;

		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL operator()(const collision_tester<triangle<scalar_type>>&) const noexcept;

		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL operator()(MUU_VPARAM(bounding_sphere<scalar_type>)) const noexcept;

		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL operator()(const collision_tester<bounding_sphere<scalar_type>>&) const noexcept;
	};

	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr muu::collision_tester<bounding_box<Scalar>> MUU_VECTORCALL bounding_box<Scalar>::collision_tester()
		const noexcept
	{
		return muu::collision_tester<bounding_box<Scalar>>{ *this };
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
	/// \see		muu::bounding_box
	///
	/// \brief		Axis-aligned bounding box constants.
	template <typename Scalar>
	struct constants<bounding_box<Scalar>>
	{
		using scalars = constants<Scalar>;
		using vectors = constants<vector<Scalar, 3>>;

		/// \brief A bounding box with all members initialized to zero.
		static constexpr bounding_box<Scalar> zero = { vectors::zero, vectors::zero };

		/// \brief A bounding box centered at the origin with sides of length 1.
		static constexpr bounding_box<Scalar> unit = { vectors::zero, vectors::one_over_two };
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
	/// \relatesalso	muu::bounding_box
	///
	/// \brief	Returns true if any of the scalar components of a bounding box are infinity or NaN.
	template <typename S>
	MUU_PURE_INLINE_GETTER
	constexpr bool infinity_or_nan(const bounding_box<S>& bb) noexcept
	{
		return bounding_box<S>::infinity_or_nan(bb);
	}

	/// \ingroup		approx_equal
	/// \relatesalso	muu::bounding_box
	///
	/// \brief		Returns true if two bounding boxes are approximately equal.
	template <typename S, typename T>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL approx_equal(const bounding_box<S>& bb1,
											   const bounding_box<T>& bb2,
											   epsilon_type<S, T> epsilon = default_epsilon<S, T>) noexcept
	{
		return bounding_box<S>::approx_equal(bb1, bb2, epsilon);
	}

	/// \ingroup		approx_zero
	/// \relatesalso	muu::bounding_box
	///
	/// \brief		Returns true if all the scalar components of a bounding box are approximately equal to zero.
	template <typename S>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL approx_zero(const bounding_box<S>& bb, S epsilon = default_epsilon<S>) noexcept
	{
		return bounding_box<S>::approx_zero(bb, epsilon);
	}

	/// \ingroup		degenerate
	/// \relatesalso	muu::bounding_box
	///
	/// \brief	Returns true if a bounding box is degenerate (i.e. any of its extents are less than or equal to zero).
	template <typename S>
	MUU_PURE_INLINE_GETTER
	constexpr bool degenerate(const bounding_box<S>& bb) noexcept
	{
		return bounding_box<S>::degenerate(bb);
	}
}

#endif //===============================================================================================================

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"

/// \cond
#ifdef MUU_LINE_SEGMENT_H
	#include "impl/bounding_box_x_line_segment.h"
#endif
#ifdef MUU_PLANE_H
	#include "impl/bounding_box_x_plane.h"
#endif
#ifdef MUU_TRIANGLE_H
	#include "impl/bounding_box_x_triangle.h"
#endif
#ifdef MUU_BOUNDING_SPHERE_H
	#include "impl/bounding_box_x_bounding_sphere.h"
#endif
#ifdef MUU_ORIENTED_BOUNDING_BOX_H
	#include "impl/bounding_box_x_oriented_bounding_box.h"
#endif
#ifdef MUU_RAY_H
	#include "impl/ray_x_bounding_box.h"
#endif
/// \endcond

#endif // MUU_BOUNDING_BOX_H
