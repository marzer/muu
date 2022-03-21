// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::bounding_box.

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
		MUU_HIDDEN_BASE(impl::bounding_box_<Scalar>)
	{
		static_assert(!std::is_reference_v<Scalar>, "Bounding box scalar type cannot be a reference");
		static_assert(!is_cv<Scalar>, "Bounding box scalar type cannot be const- or volatile-qualified");
		static_assert(std::is_trivially_constructible_v<Scalar>	  //
						  && std::is_trivially_copyable_v<Scalar> //
						  && std::is_trivially_destructible_v<Scalar>,
					  "Bounding box scalar type must be trivially constructible, copyable and destructible");
		static_assert(is_signed<Scalar>, "Bounding box scalar type must be signed");
		static_assert(is_floating_point<Scalar>, "Bounding scalar type must be a floating-point type");

		/// \brief The bounding box's scalar type.
		using scalar_type = Scalar;

		/// \brief The three-dimensional #muu::vector with the same #scalar_type as the bounding box.
		using vector_type = vector<scalar_type, 3>;

		/// \brief Compile-time bounding box constants.
		using constants = muu::constants<bounding_box>;

	  private:
		/// \cond
		using base = impl::bounding_box_<Scalar>;
		static_assert(sizeof(base) == (sizeof(vector_type) * 2), "Bounding boxes should not have padding");

		using boxes				 = impl::boxes_common<Scalar>;
		using aabbs				 = impl::aabb_common<Scalar>;
		using intermediate_float = promote_if_small_float<scalar_type>;
		static_assert(is_floating_point<intermediate_float>);

		/// \endcond

	  public:
	#ifdef DOXYGEN

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
		constexpr bounding_box(const T& blittable) noexcept //
			: base{ muu::bit_cast<base>(blittable) }
		{
			static_assert(sizeof(T) == sizeof(base), "Bit-castable types must be the same size");
			static_assert(std::is_trivially_copyable_v<T>, "Bit-castable types must be trivially-copyable");
		}

		/// \brief	Constructs a bounding box fitting a range of points.
		MUU_PURE_GETTER
		MUU_ATTR(nonnull)
		static constexpr bounding_box MUU_VECTORCALL from_points(const vector_type* begin,
																 const vector_type* end) noexcept
		{
			if MUU_UNLIKELY(begin == end)
				return bounding_box{ *begin, vector_type{} };

			if constexpr (std::is_same_v<scalar_type, intermediate_float>)
			{
				const auto min = vector_type::min(begin, end);
				const auto max = vector_type::max(begin, end);
				return bounding_box{ aabbs::center(min, max), aabbs::extents(min, max) };
			}
			else
			{
				using ivec3	   = vector<intermediate_float, 3>;
				const auto min = ivec3{ vector_type::min(begin, end) };
				const auto max = ivec3{ vector_type::max(begin, end) };

				return bounding_box{ vector_type{ impl::aabb_common<intermediate_float>::center(min, max) },
									 vector_type{ impl::aabb_common<intermediate_float>::extents(min, max) } };
			}
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
			: bounding_box{ from_points(points, points + N) }
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
		static constexpr bounding_box MUU_VECTORCALL from_obb(
			MUU_VC_PARAM(oriented_bounding_box<scalar_type>) obb) noexcept;

		/// \brief	Constructs a bounding box completely containing an oriented bounding-box.
		MUU_NODISCARD_CTOR
		explicit constexpr bounding_box(MUU_VC_PARAM(oriented_bounding_box<scalar_type>) obb) noexcept //
			: bounding_box{ from_obb(obb) }
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
		MUU_PURE_GETTER
		constexpr scalar_type width() const noexcept
		{
			return aabbs::width(base::extents);
		}

		/// \brief	Returns the height of the box (y-axis).
		MUU_PURE_GETTER
		constexpr scalar_type height() const noexcept
		{
			return aabbs::height(base::extents);
		}

		/// \brief	Returns the depth of the box (z-axis).
		MUU_PURE_GETTER
		constexpr scalar_type depth() const noexcept
		{
			return aabbs::depth(base::extents);
		}

		/// \brief	Calculates the length of the line connecting the min and max points.
		MUU_PURE_GETTER
		constexpr scalar_type diagonal() const noexcept
		{
			return aabbs::diagonal(base::extents);
		}

		/// \brief	Returns the shortest of the box's three extents.
		MUU_PURE_GETTER
		constexpr scalar_type& shortest_extent() noexcept
		{
			return aabbs::shortest_extent(base::extents);
		}

		/// \brief	Returns the longest of the box's three extents.
		MUU_PURE_GETTER
		constexpr scalar_type& longest_extent() noexcept
		{
			return aabbs::longest_extent(base::extents);
		}

		/// \brief	Returns the shortest of the box's three extents (const overload).
		MUU_PURE_GETTER
		constexpr const scalar_type& shortest_extent() const noexcept
		{
			return aabbs::shortest_extent(base::extents);
		}

		/// \brief	Returns the longest of the box's three extents (const overload).
		MUU_PURE_GETTER
		constexpr const scalar_type& longest_extent() const noexcept
		{
			return aabbs::longest_extent(base::extents);
		}

		/// \brief	Returns the length of the shortest of the box's three sides.
		MUU_PURE_GETTER
		constexpr scalar_type shortest_side() const noexcept
		{
			return aabbs::sortest_side(base::extents);
		}

		/// \brief	Returns the length of the longest of the box's three sides.
		MUU_PURE_GETTER
		constexpr scalar_type longest_side() const noexcept
		{
			return aabbs::longest_side(base::extents);
		}

		/// \brief	Calculates the volume of this bounding box.
		MUU_PURE_GETTER
		constexpr scalar_type volume() const noexcept
		{
			return aabbs::volume(base::extents);
		}

		/// \brief	Calculates the mass of this box if it had a given density.
		MUU_PURE_GETTER
		constexpr scalar_type MUU_VECTORCALL mass(scalar_type density) const noexcept
		{
			return aabbs::mass(base::extents, density);
		}

		/// \brief	Calculates the density of this box if it had a given mass.
		MUU_PURE_GETTER
		constexpr scalar_type MUU_VECTORCALL density(scalar_type mass) const noexcept
		{
			return aabbs::density(base::extents, mass);
		}

		/// \brief	Returns true if the box is degenerate (i.e. any of its extents are less than or equal to zero).
		MUU_PURE_GETTER
		constexpr bool degenerate() const noexcept
		{
			return aabbs::degenerate(base::extents);
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
		friend constexpr bool MUU_VECTORCALL operator==(MUU_VC_PARAM(bounding_box) lhs,
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
		friend constexpr bool MUU_VECTORCALL operator!=(MUU_VC_PARAM(bounding_box) lhs,
														const bounding_box<T>& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		/// \brief	Returns true if all the scalar components of a bounding box are exactly zero.
		///
		/// \remarks	This is an exact check;
		///				use #approx_zero() if you want an epsilon-based "near-enough" check.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL zero(MUU_VC_PARAM(bounding_box) bb) noexcept
		{
			return vector_type::zero(bb.center) && vector_type::zero(bb.extents);
		}

		/// \brief	Returns true if all the scalar components of the bounding box are exactly zero.
		///
		/// \remarks	This is an exact check;
		///				use #approx_zero() if you want an epsilon-based "near-enough" check.
		MUU_PURE_GETTER
		constexpr bool zero() const noexcept
		{
			return zero(*this);
		}

		/// \brief	Returns true if a bounding box has exactly zero volume.
		///
		/// \remarks	This is an exact check;
		///				use #approx_empty() if you want an epsilon-based "near-enough" check.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL empty(MUU_VC_PARAM(bounding_box) bb) noexcept
		{
			return vector_type::zero(bb.extents);
		}

		/// \brief	Returns true if the bounding box has exactly zero volume.
		///
		/// \remarks	This is an exact check;
		///				use #approx_empty() if you want an epsilon-based "near-enough" check.
		MUU_PURE_GETTER
		constexpr bool empty() const noexcept
		{
			return vector_type::zero(base::extents);
		}

		/// \brief	Returns true if any of the scalar components of a bounding box are infinity or NaN.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL infinity_or_nan(MUU_VC_PARAM(bounding_box) bb) noexcept
		{
			return vector_type::infinity_or_nan(bb.center) || vector_type::infinity_or_nan(bb.extents);
		}

		/// \brief	Returns true if any of the scalar components of the bounding box are infinity or NaN.
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

		/// \brief	Returns true if two bounding boxes are approximately equal.
		template <typename T>
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_equal(
			MUU_VC_PARAM(bounding_box) bb1,
			const bounding_box<T>& bb2,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) noexcept
		{
			return vector_type::approx_equal(bb1.center, bb2.center, epsilon)
				&& vector_type::approx_equal(bb1.extents, bb2.extents, epsilon);
		}

		/// \brief	Returns true if the bounding box is approximately equal to another.
		template <typename T>
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL approx_equal(
			const bounding_box<T>& bb,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) const noexcept
		{
			return approx_equal(*this, bb, epsilon);
		}

		/// \brief	Returns true if all the scalar components in a bounding box are approximately equal to zero.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_zero(MUU_VC_PARAM(bounding_box) bb,
														 scalar_type epsilon = default_epsilon<scalar_type>) noexcept
		{
			return vector_type::approx_zero(bb.center, epsilon) && vector_type::approx_zero(bb.extents, epsilon);
		}

		/// \brief	Returns true if all the scalar components in the bounding box are approximately equal to zero.
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL approx_zero(scalar_type epsilon = default_epsilon<scalar_type>) const noexcept
		{
			return approx_zero(*this, epsilon);
		}

		/// \brief	Returns true if a bounding box has approximately zero volume.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_empty(MUU_VC_PARAM(bounding_box) bb,
														  scalar_type epsilon = default_epsilon<scalar_type>) noexcept
		{
			return vector_type::approx_zero(bb.extents, epsilon);
		}

		/// \brief	Returns true if the bounding box has approximately zero volume.
		MUU_PURE_GETTER
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
		template <box_corners Corner>
		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL corner(MUU_VC_PARAM(bounding_box) bb) noexcept
		{
			return aabbs::template corner<Corner>(bb.center, bb.extents);
		}

		/// \brief	Returns a specific corner of the bounding box.
		template <box_corners Corner>
		MUU_PURE_GETTER
		constexpr vector_type corner() const noexcept
		{
			return aabbs::template corner<Corner>(base::center, base::extents);
		}

		/// \brief	Returns a specific corner of a bounding box.
		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL corner(MUU_VC_PARAM(bounding_box) bb, box_corners which) noexcept
		{
			return aabbs::corner(bb.center, bb.extents, which);
		}

		/// \brief	Returns a specific corner of the bounding box.
		MUU_PURE_GETTER
		constexpr vector_type corner(box_corners which) const noexcept
		{
			return aabbs::corner(base::center, base::extents, which);
		}

		/// \brief	Returns the 'min' corner of a bounding box.
		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL min_corner(MUU_VC_PARAM(bounding_box) bb) noexcept
		{
			return aabbs::template corner<box_corners::min>(bb.center, bb.extents);
		}

		/// \brief	Returns the 'min' corner of the bounding box.
		MUU_PURE_GETTER
		constexpr vector_type min_corner() const noexcept
		{
			return aabbs::template corner<box_corners::min>(base::center, base::extents);
		}

		/// \brief	Returns the 'max' corner of a bounding box.
		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL max_corner(MUU_VC_PARAM(bounding_box) bb) noexcept
		{
			return aabbs::template corner<box_corners::max>(bb.center, bb.extents);
		}

		/// \brief	Returns the 'max' corner of the bounding box.
		MUU_PURE_GETTER
		constexpr vector_type max_corner() const noexcept
		{
			return aabbs::template corner<box_corners::max>(base::center, base::extents);
		}

			/// @}
	#endif // corners

	#if 1 // transformation, translation and scaling -------------------------------------------------------------------
		/// \name Transformation, translation and scaling
		/// @{

		/// \brief	Translates a bounding box.
		///
		/// \param	bb		The bounding box to translate.
		/// \param	offset	An offset to add to the box's center position.
		///
		/// \returns	A copy of the input box translated by the given offset.
		MUU_PURE_GETTER
		static constexpr bounding_box MUU_VECTORCALL translate(MUU_VC_PARAM(bounding_box) bb,
															   MUU_VC_PARAM(vector_type) offset) noexcept
		{
			return bounding_box{ bb.center + offset, bb.extents };
		}

		/// \brief	Translates the bounding box (in-place).
		///
		/// \param	offset	An offset to add to the box's center position.
		///
		/// \return	A reference to the box_.
		constexpr bounding_box& MUU_VECTORCALL translate(MUU_VC_PARAM(vector_type) offset) noexcept
		{
			base::center += offset;
			return *this;
		}

		/// \brief	Scales a bounding box.
		///
		/// \param	bb		The bounding box to scale.
		/// \param	scale_	The amount to scale the box extents by on each axis.
		///
		/// \returns	A copy of the input box scaled by the given amounts.
		MUU_PURE_GETTER
		static constexpr bounding_box MUU_VECTORCALL scale(MUU_VC_PARAM(bounding_box) bb,
														   MUU_VC_PARAM(vector_type) scale_) noexcept
		{
			return bounding_box{ bb.center, bb.extents * scale_ };
		}

		/// \brief	Scales the bounding box (in-place).
		///
		/// \param	scale_	The amount to scale the box extents by on each axis.
		///
		/// \return	A reference to the box.
		constexpr bounding_box& MUU_VECTORCALL scale(MUU_VC_PARAM(vector_type) scale_) noexcept
		{
			base::extents *= scale_;
			return *this;
		}

		/// \brief Transforms an axis-aligned bounding box from one coordinate space to another.
		///
		/// \param	bb		The bounding box to transform.
		/// \param	tx		The transform to apply.
		///
		/// \return	Returns an axis-aligned bounding box containing all the points of the
		///			input bounding box after being transformed.
		MUU_PURE_GETTER
		static constexpr bounding_box MUU_VECTORCALL transform(MUU_VC_PARAM(bounding_box) bb,
															   MUU_VC_PARAM(matrix<scalar_type, 4, 4>) tx) noexcept
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

		/// \brief Transforms the axis-aligned bounding box from one coordinate space to another (in-place).
		///
		/// \param	tx		The transform to apply.
		///
		/// \return	A reference to the box.
		constexpr bounding_box& transform(MUU_VC_PARAM(matrix<scalar_type, 4, 4>) tx) noexcept
		{
			return *this = transform(*this, tx);
		}

			/// @}
	#endif // transformation, translation and scaling

	#if 1 // intersection and containment ------------------------------------------------------------------------------
		/// \name Intersection and containment
		/// @{

		/// \brief	Returns true if a bounding box contains a point.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains(MUU_VC_PARAM(bounding_box) bb,
													  MUU_VC_PARAM(vector_type) point) noexcept
		{
			return aabbs::contains_point(bb.center, bb.extents, point);
		}

		/// \brief	Returns true if the bounding box contains a point.
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL contains(MUU_VC_PARAM(vector_type) point) const noexcept
		{
			return contains(*this, point);
		}

		/// \brief	Returns true if two bounding boxes intersect.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VC_PARAM(bounding_box) bb1,
														MUU_VC_PARAM(bounding_box) bb2) noexcept
		{
			return aabbs::intersects_aabb(bb1.center, bb1.extents, bb2.center, bb2.extents);
		}

		/// \brief	Returns true if two bounding boxes intersect.
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL intersects(MUU_VC_PARAM(bounding_box) bb) const noexcept
		{
			return intersects(*this, bb);
		}

		/// \brief	Returns true if a bounding box intersects a plane.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VC_PARAM(bounding_box) bb,
														MUU_VC_PARAM(plane<scalar_type>) p) noexcept;

		/// \brief	Returns true if the bounding box intersects a plane.
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL intersects(MUU_VC_PARAM(plane<scalar_type>) p) const noexcept
		{
			return intersects(*this, p);
		}

		/// \brief	Returns true if a bounding box intersects a triangle.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VC_PARAM(bounding_box) bb,
														MUU_VC_PARAM(triangle<scalar_type>) tri) noexcept;

		/// \brief	Returns true if the bounding box intersects a triangle.
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL intersects(MUU_VC_PARAM(triangle<scalar_type>) tri) const noexcept
		{
			return intersects(*this, tri);
		}

		/// \cond

		[[deprecated]] static constexpr bool MUU_VECTORCALL intersects_triangle(MUU_VC_PARAM(bounding_box) bb,
																				MUU_VC_PARAM(vector_type) p0,
																				MUU_VC_PARAM(vector_type) p1,
																				MUU_VC_PARAM(vector_type) p2) noexcept
		{
			return aabbs::intersects_triangle(bb.center, bb.extents, p0, p1, p2);
		}

		[[deprecated]] constexpr bool MUU_VECTORCALL intersects_triangle(MUU_VC_PARAM(vector_type) p0,
																		 MUU_VC_PARAM(vector_type) p1,
																		 MUU_VC_PARAM(vector_type) p2) const noexcept
		{
			return intersects_triangle(*this, p0, p1, p2);
		}

			/// \endcond

			/// @}
	#endif // intersection and containment

	#if 1 // misc ------------------------------------------------------------------------------------------------------

		/// \brief Writes a vector out to a text stream.
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
	bounding_box(CX, CY, CZ, vector<E, 3>)->bounding_box<impl::highest_ranked<CX, CY, CZ, E>>;

	MUU_CONSTRAINED_TEMPLATE((all_arithmetic<EX, EY, EZ>), typename C, typename EX, typename EY, typename EZ)
	bounding_box(vector<C, 3>, EX, EY, EZ)->bounding_box<impl::highest_ranked<C, EX, EY, EZ>>;

	MUU_CONSTRAINED_TEMPLATE(is_arithmetic<E>, typename C, typename E)
	bounding_box(vector<C, 3>, E)->bounding_box<impl::highest_ranked<C, E>>;

	template <typename E>
	bounding_box(vector<E, 3>) -> bounding_box<E>;

	MUU_CONSTRAINED_TEMPLATE(is_arithmetic<E>, typename E)
	bounding_box(E)->bounding_box<std::remove_cv_t<E>>;

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
	/// \ingroup		infinity_or_nan
	/// \relatesalso	muu::bounding_box
	///
	/// \brief	Returns true if any of the scalar components of a bounding_box are infinity or NaN.
	template <typename S>
	MUU_PURE_GETTER
	constexpr bool infinity_or_nan(const bounding_box<S>& q) noexcept
	{
		return bounding_box<S>::infinity_or_nan(q);
	}

	/// \ingroup		approx_equal
	/// \relatesalso	muu::bounding_box
	///
	/// \brief		Returns true if two bounding_boxes are approximately equal.
	template <typename S, typename T>
	MUU_PURE_GETTER
	constexpr bool MUU_VECTORCALL approx_equal(const bounding_box<S>& q1,
											   const bounding_box<T>& q2,
											   epsilon_type<S, T> epsilon = default_epsilon<S, T>) noexcept
	{
		return bounding_box<S>::approx_equal(q1, q2, epsilon);
	}

	/// \ingroup		approx_zero
	/// \relatesalso	muu::bounding_box
	///
	/// \brief		Returns true if all the scalar components of a bounding_box are approximately equal to zero.
	template <typename S>
	MUU_PURE_GETTER
	constexpr bool MUU_VECTORCALL approx_zero(const bounding_box<S>& q, S epsilon = default_epsilon<S>) noexcept
	{
		return bounding_box<S>::approx_zero(q, epsilon);
	}
}

#endif //===============================================================================================================

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"

/// \cond
#include "impl/bounding_box_x_oriented_bounding_box.h"
#include "impl/bounding_box_x_triangle.h"
#include "impl/plane_x_bounding_box.h"
/// \endcond
