// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::oriented_bounding_box.

#include "sat_tester.h"
#include "impl/geometry_common.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(float_control(except, off))

//======================================================================================================================
// ORIENTED BOUNDING BOX CLASS
#if 1

namespace muu
{
	/// \brief An oriented bounding box.
	/// \ingroup math
	///
	/// \tparam	Scalar      The oriented bounding box's scalar component type. Must be a floating-point type.
	///
	/// \see [Oriented Bounding Box](https://www.sciencedirect.com/topics/computer-science/oriented-bounding-box)
	template <typename Scalar>
	struct MUU_TRIVIAL_ABI oriented_bounding_box //
		MUU_HIDDEN_BASE(impl::storage_base<oriented_bounding_box<Scalar>>)
	{
		static_assert(!std::is_reference_v<Scalar>, "Oriented bounding box scalar type cannot be a reference");
		static_assert(!is_cv<Scalar>, "Oriented bounding box scalar type cannot be const- or volatile-qualified");
		static_assert(std::is_trivially_constructible_v<Scalar>	  //
						  && std::is_trivially_copyable_v<Scalar> //
						  && std::is_trivially_destructible_v<Scalar>,
					  "Oriented bounding box scalar type must be trivially constructible, copyable and destructible");
		static_assert(is_floating_point<Scalar>, "Oriented bounding box scalar type must be a floating-point type");
		static_assert(is_signed<Scalar>, "Oriented bounding box scalar type must be signed");
		static_assert(is_floating_point<Scalar>, "Oriented bounding scalar type must be a floating-point type");

		/// \brief The oriented bounding box's scalar type.
		using scalar_type = Scalar;

		/// \brief The three-dimensional #muu::vector with the same #scalar_type as the oriented bounding box.
		using vector_type = vector<scalar_type, 3>;

		/// \brief The 3x3 #muu::matrix with the same #scalar_type as the oriented bounding box.
		using axes_type = matrix<scalar_type, 3, 3>;

		/// \brief Compile-time bounding box constants.
		using constants = muu::constants<oriented_bounding_box>;

	  private:
		using base = impl::storage_base<oriented_bounding_box<Scalar>>;
		static_assert(sizeof(base) == (sizeof(vector_type) * 2 + sizeof(axes_type)),
					  "Oriented bounding boxes should not have padding");

		using obbs		 = impl::obb_common<Scalar>;
		using sat_tester = muu::sat_tester<Scalar, 3>;

		using promoted_scalar					 = promote_if_small_float<scalar_type>;
		using promoted_vec3						 = vector<promoted_scalar, 3>;
		using promoted_box						 = oriented_bounding_box<promoted_scalar>;
		static constexpr bool requires_promotion = impl::is_small_float_<scalar_type>;

		using axes_constants = muu::constants<axes_type>;

	  public:
	#if MUU_DOXYGEN

		/// \brief	The center of the box.
		vector_type center;

		/// \brief	The half-lengths of box (i.e. distances from the center to the sides).
		vector_type extents;

		/// \brief	The axes of the box's orientation.
		axes_type axes;

	#endif // DOXYGEN

	#if 1 // constructors ----------------------------------------------------------------------------------------------

		/// \brief Default constructor. Values are not initialized.
		oriented_bounding_box() noexcept = default;

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr oriented_bounding_box(const oriented_bounding_box&) noexcept = default;

		/// \brief Copy-assigment operator.
		constexpr oriented_bounding_box& operator=(const oriented_bounding_box&) noexcept = default;

		/// \brief	Constructs a oriented bounding box from center, extents and axes values.
		///
		/// \param	cen		The center point.
		/// \param	ext		The extents.
		/// \param	rot		The rotation axes.
		MUU_NODISCARD_CTOR
		constexpr oriented_bounding_box(const vector_type& cen,
										const vector_type& ext,
										const axes_type& rot = axes_constants::identity) noexcept
			: base{ cen, ext, rot }
		{}

		/// \brief	Constructs a oriented bounding box from center, extents and axes values.
		///
		/// \param	cen		The center point.
		/// \param	ext_x	The length of the X extent.
		/// \param	ext_y	The length of the Y extent.
		/// \param	ext_z	The length of the Z extent.
		/// \param	rot		The rotation axes.
		MUU_NODISCARD_CTOR
		constexpr oriented_bounding_box(const vector_type& cen,
										scalar_type ext_x,
										scalar_type ext_y,
										scalar_type ext_z,
										const axes_type& rot = axes_constants::identity) noexcept
			: base{ cen, { ext_x, ext_y, ext_z }, rot }
		{}

		/// \brief	Constructs a uniformly-sized oriented bounding box.
		///
		/// \param	cen		The center point.
		/// \param	ext		The length of all three extents.
		/// \param	rot		The rotation axes.
		MUU_NODISCARD_CTOR
		constexpr oriented_bounding_box(const vector_type& cen,
										scalar_type ext,
										const axes_type& rot = axes_constants::identity) noexcept
			: base{ cen, { ext, ext, ext }, rot }
		{}

		/// \brief	Constructs an oriented bounding box at the origin.
		///
		/// \param	ext		The extents.
		/// \param	rot		The rotation axes.
		MUU_NODISCARD_CTOR
		explicit constexpr oriented_bounding_box(const vector_type& ext,
												 const axes_type& rot = axes_constants::identity) noexcept
			: base{ vector_type::constants::zero, ext, rot }
		{}

		/// \brief	Constructs an oriented bounding box from center and extent values.
		///
		/// \param	cen_x	The X component of the center point.
		/// \param	cen_y	The Y component of the center point.
		/// \param	cen_z	The Z component of the center point.
		/// \param	ext		The extents.
		/// \param	rot		The rotation axes.
		MUU_NODISCARD_CTOR
		constexpr oriented_bounding_box(scalar_type cen_x,
										scalar_type cen_y,
										scalar_type cen_z,
										const vector_type& ext,
										const axes_type& rot = axes_constants::identity) noexcept
			: base{ { cen_x, cen_y, cen_z }, ext, rot }
		{}

		/// \brief	Constructs an oriented bounding box from center and extent values.
		///
		/// \param	cen_x	The X component of the center point.
		/// \param	cen_y	The Y component of the center point.
		/// \param	cen_z	The Z component of the center point.
		/// \param	ext_x	The length of the X extent.
		/// \param	ext_y	The length of the Y extent.
		/// \param	ext_z	The length of the Z extent.
		/// \param	rot		The rotation axes.
		MUU_NODISCARD_CTOR
		constexpr oriented_bounding_box(scalar_type cen_x,
										scalar_type cen_y,
										scalar_type cen_z,
										scalar_type ext_x,
										scalar_type ext_y,
										scalar_type ext_z,
										const axes_type& rot = axes_constants::identity) noexcept
			: base{ { cen_x, cen_y, cen_z }, { ext_x, ext_y, ext_z }, rot }
		{}

		/// \brief	Constructs a uniformly-sized oriented bounding box at the origin.
		///
		/// \param	ext		The length of all three extents.
		/// \param	rot		The rotation axes.
		MUU_NODISCARD_CTOR
		explicit constexpr oriented_bounding_box(scalar_type ext,
												 const axes_type& rot = axes_constants::identity) noexcept
			: base{ vector_type::constants::zero, { ext, ext, ext }, rot }
		{}

		/// \brief	Converting constructor.
		template <typename S>
		MUU_NODISCARD_CTOR
		explicit constexpr oriented_bounding_box(const oriented_bounding_box<S>& bb) noexcept //
			: base{ vector_type{ bb.center }, vector_type{ bb.extents }, axes_type{ bb.axes } }
		{}

		/// \brief Constructs a oriented bounding box from an implicitly bit-castable type.
		///
		/// \tparam T	A bit-castable type.
		///
		/// \see muu::allow_implicit_bit_cast
		MUU_CONSTRAINED_TEMPLATE((allow_implicit_bit_cast<T, oriented_bounding_box>), typename T)
		MUU_NODISCARD_CTOR
		/*implicit*/
		constexpr oriented_bounding_box(const T& blittable) noexcept //
			: base{ muu::bit_cast<base>(blittable) }
		{
			static_assert(sizeof(T) == sizeof(base), "Bit-castable types must be the same size");
			static_assert(std::is_trivially_copyable_v<T>, "Bit-castable types must be trivially-copyable");
		}

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
			return obbs::width(base::extents);
		}

		/// \brief	Returns the height of the box (y-axis).
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type height() const noexcept
		{
			return obbs::height(base::extents);
		}

		/// \brief	Returns the depth of the box (z-axis).
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type depth() const noexcept
		{
			return obbs::depth(base::extents);
		}

		/// \brief	Calculates the length of the line connecting the min and max points.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type diagonal() const noexcept
		{
			return obbs::diagonal(base::extents);
		}

		/// \brief	Returns the shortest of the box's three extents.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type& shortest_extent() noexcept
		{
			return obbs::shortest_extent(base::extents);
		}

		/// \brief	Returns the longest of the box's three extents.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type& longest_extent() noexcept
		{
			return obbs::longest_extent(base::extents);
		}

		/// \brief	Returns the shortest of the box's three extents (const overload).
		MUU_PURE_INLINE_GETTER
		constexpr const scalar_type& shortest_extent() const noexcept
		{
			return obbs::shortest_extent(base::extents);
		}

		/// \brief	Returns the longest of the box's three extents (const overload).
		MUU_PURE_INLINE_GETTER
		constexpr const scalar_type& longest_extent() const noexcept
		{
			return obbs::longest_extent(base::extents);
		}

		/// \brief	Returns the length of the shortest of the box's three sides.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type shortest_side() const noexcept
		{
			return obbs::shortest_side(base::extents);
		}

		/// \brief	Returns the length of the longest of the box's three sides.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type longest_side() const noexcept
		{
			return obbs::longest_side(base::extents);
		}

		/// \brief	Calculates the volume of this bounding box.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type volume() const noexcept
		{
			return obbs::volume(base::extents);
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

		/// \brief		Returns true if two oriented bounding boxes are exactly equal.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		template <typename T>
		MUU_PURE_GETTER
		friend constexpr bool MUU_VECTORCALL operator==(MUU_VPARAM(oriented_bounding_box) lhs,
														const oriented_bounding_box<T>& rhs) noexcept
		{
			return lhs.center == rhs.center && lhs.extents == rhs.extents && lhs.axes == rhs.axes;
		}

		/// \brief	Returns true if two oriented bounding boxes are not exactly equal.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		template <typename T>
		MUU_PURE_GETTER
		friend constexpr bool MUU_VECTORCALL operator!=(MUU_VPARAM(oriented_bounding_box) lhs,
														const oriented_bounding_box<T>& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		/// \brief	Returns true if all the scalar components of an oriented bounding box are exactly zero.
		///
		/// \remarks	This is an exact check;
		///				use #approx_zero() if you want an epsilon-based "near-enough" check.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL zero(MUU_VPARAM(oriented_bounding_box) bb) noexcept
		{
			return vector_type::zero(bb.center) && vector_type::zero(bb.extents) && axes_type::zero(bb.axes);
		}

		/// \brief	Returns true if all the scalar components of the oriented bounding box are exactly zero.
		///
		/// \remarks	This is an exact check;
		///				use #approx_zero() if you want an epsilon-based "near-enough" check.
		MUU_PURE_INLINE_GETTER
		constexpr bool zero() const noexcept
		{
			return zero(*this);
		}

		/// \brief	Returns true if an oriented bounding box has exactly zero volume.
		///
		/// \remarks	This is an exact check;
		///				use #approx_empty() if you want an epsilon-based "near-enough" check.
		MUU_PURE_INLINE_GETTER
		static constexpr bool MUU_VECTORCALL empty(MUU_VPARAM(oriented_bounding_box) bb) noexcept
		{
			return vector_type::zero(bb.extents);
		}

		/// \brief	Returns true if the oriented bounding box has exactly zero volume.
		///
		/// \remarks	This is an exact check;
		///				use #approx_empty() if you want an epsilon-based "near-enough" check.
		MUU_PURE_INLINE_GETTER
		constexpr bool empty() const noexcept
		{
			return vector_type::zero(base::extents);
		}

		/// \brief	Returns true if any of the scalar components of an oriented bounding box are infinity or NaN.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL infinity_or_nan(MUU_VPARAM(oriented_bounding_box) bb) noexcept
		{
			return vector_type::infinity_or_nan(bb.center) || vector_type::infinity_or_nan(bb.extents)
				|| axes_type::infinity_or_nan(bb.axes);
		}

		/// \brief	Returns true if any of the scalar components of the oriented bounding box are infinity or NaN.
		MUU_PURE_INLINE_GETTER
		constexpr bool infinity_or_nan() const noexcept
		{
			return infinity_or_nan(*this);
		}

		/// \brief	Returns true if the box is degenerate (i.e. any of its extents are less than or equal to zero).
		MUU_PURE_INLINE_GETTER
		static constexpr bool MUU_VECTORCALL degenerate(MUU_VPARAM(oriented_bounding_box) bb) noexcept
		{
			return obbs::degenerate(bb.extents);
		}

		/// \brief	Returns true if the box is degenerate (i.e. any of its extents are less than or equal to zero).
		MUU_PURE_INLINE_GETTER
		constexpr bool degenerate() const noexcept
		{
			return obbs::degenerate(base::extents);
		}

			/// @}
	#endif // equality (exact)

	#if 1 // equality (approx) -----------------------------------------------------------------------------------------
		  /// \name Equality (approximate)
		  /// @{

		/// \brief	Returns true if two oriented bounding boxes are approximately equal.
		template <typename T>
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_equal(
			MUU_VPARAM(oriented_bounding_box) bb1,
			const oriented_bounding_box<T>& bb2,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) noexcept
		{
			return vector_type::approx_equal(bb1.center, bb2.center, epsilon)
				&& vector_type::approx_equal(bb1.extents, bb2.extents, epsilon)
				&& axes_type::approx_equal(bb1.axes, bb2.axes, epsilon);
		}

		/// \brief	Returns true if the oriented bounding box is approximately equal to another.
		template <typename T>
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL approx_equal(
			const oriented_bounding_box<T>& bb,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) const noexcept
		{
			return approx_equal(*this, bb, epsilon);
		}

		/// \brief	Returns true if all the scalar components in an oriented bounding box are approximately equal to
		/// zero.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_zero(MUU_VPARAM(oriented_bounding_box) bb,
														 scalar_type epsilon = default_epsilon<scalar_type>) noexcept
		{
			return vector_type::approx_zero(bb.center, epsilon) && vector_type::approx_zero(bb.extents, epsilon)
				&& axes_type::approx_zero(bb.axes, epsilon);
		}

		/// \brief	Returns true if all the scalar components in the oriented bounding box are approximately equal to
		/// zero.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL approx_zero(scalar_type epsilon = default_epsilon<scalar_type>) const noexcept
		{
			return approx_zero(*this, epsilon);
		}

		/// \brief	Returns true if an oriented bounding box has approximately zero volume.
		MUU_PURE_INLINE_GETTER
		static constexpr bool MUU_VECTORCALL approx_empty(MUU_VPARAM(oriented_bounding_box) bb,
														  scalar_type epsilon = default_epsilon<scalar_type>) noexcept
		{
			return vector_type::approx_zero(bb.extents, epsilon);
		}

		/// \brief	Returns true if the oriented bounding box has approximately zero volume.
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

		/// \brief	Returns a specific corner of an oriented bounding box.
		template <box_corner Corner>
		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL corner(MUU_VPARAM(oriented_bounding_box) bb) noexcept
		{
			return obbs::template corner<Corner>(bb.center, bb.extents, bb.axes);
		}

		/// \brief	Returns a specific corner of the oriented bounding box.
		template <box_corner Corner>
		MUU_PURE_INLINE_GETTER
		constexpr vector_type corner() const noexcept
		{
			return obbs::template corner<Corner>(base::center, base::extents, base::axes);
		}

		/// \brief	Returns a specific corner of an oriented bounding box.
		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL corner(MUU_VPARAM(oriented_bounding_box) bb,
														   box_corner which) noexcept
		{
			return obbs::corner(bb.center, bb.extents, bb.axes, which);
		}

		/// \brief	Returns a specific corner of the oriented bounding box.
		MUU_PURE_INLINE_GETTER
		constexpr vector_type corner(box_corner which) const noexcept
		{
			return obbs::corner(base::center, base::extents, base::axes, which);
		}

		/// \brief	Returns the 'min' corner of an oriented bounding box.
		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL min_corner(MUU_VPARAM(oriented_bounding_box) bb) noexcept
		{
			return obbs::template corner<box_corner::min>(bb.center, bb.extents, bb.axes);
		}

		/// \brief	Returns the 'min' corner of the oriented bounding box.
		MUU_PURE_INLINE_GETTER
		constexpr vector_type min_corner() const noexcept
		{
			return obbs::template corner<box_corner::min>(base::center, base::extents, base::axes);
		}

		/// \brief	Returns the 'max' corner of an oriented bounding box.
		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL max_corner(MUU_VPARAM(oriented_bounding_box) bb) noexcept
		{
			return obbs::template corner<box_corner::max>(bb.center, bb.extents, bb.axes);
		}

		/// \brief	Returns the 'max' corner of the oriented bounding box.
		MUU_PURE_INLINE_GETTER
		constexpr vector_type max_corner() const noexcept
		{
			return obbs::template corner<box_corner::max>(base::center, base::extents, base::axes);
		}

			/// @}
	#endif // corners

	#if 1 // translation -----------------------------------------------------------------------------------
		/// \name Translation
		/// @{

		/// \brief	Translates an oriented bounding box.
		///
		/// \param	bb		The oriented bounding box to translate.
		/// \param	offset	An offset to add to the box's center position.
		///
		/// \returns	A copy of the input box translated by the given offset.
		MUU_PURE_GETTER
		static constexpr oriented_bounding_box MUU_VECTORCALL translate(MUU_VPARAM(oriented_bounding_box) bb,
																		MUU_VPARAM(vector_type) offset) noexcept
		{
			return oriented_bounding_box{ bb.center + offset, bb.extents, bb.axes };
		}

		/// \brief	Translates the oriented bounding box (in-place).
		///
		/// \param	offset	An offset to add to the box's center position.
		///
		/// \return	A reference to the box_.
		constexpr oriented_bounding_box& MUU_VECTORCALL translate(MUU_VPARAM(vector_type) offset) noexcept
		{
			base::center += offset;
			return *this;
		}

			/// @}
	#endif // translation

	#if 1 // scaling -----------------------------------------------------------------------------------
		  /// \name Scaling
		  /// @{

		/// \brief	Scales an oriented bounding box.
		///
		/// \param	bb		The oriented bounding box to scale.
		/// \param	scale_	The amount to scale the box extents by on each axis.
		///
		/// \returns	A copy of the input box scaled by the given amounts.
		MUU_PURE_GETTER
		static constexpr oriented_bounding_box MUU_VECTORCALL scale(MUU_VPARAM(oriented_bounding_box) bb,
																	MUU_VPARAM(vector_type) scale_) noexcept
		{
			return oriented_bounding_box{ bb.center, bb.extents * scale_, bb.axes };
		}

		/// \brief	Scales the oriented bounding box (in-place).
		///
		/// \param	scale_	The amount to scale the box extents by on each axis.
		///
		/// \return	A reference to the box_.
		constexpr oriented_bounding_box& MUU_VECTORCALL scale(MUU_VPARAM(vector_type) scale_) noexcept
		{
			base::extents *= scale_;
			return *this;
		}

			/// @}
	#endif // scaling

	#if 1 // transformation -------------------------------------------------------------------
		  /// \name Transformation
		  /// @{

		/// \brief Transforms an oriented bounding box from one coordinate space to another.
		///
		/// \param	bb		The bounding box to transform.
		/// \param	tx		The transform to apply.
		///
		/// \return	Returns an oriented bounding box containing all the points of the
		///			input bounding box after being transformed.
		MUU_PURE_GETTER
		static constexpr oriented_bounding_box MUU_VECTORCALL transform(MUU_VPARAM(oriented_bounding_box) bb,
																		MUU_VPARAM(matrix<scalar_type, 4, 4>)
																			tx) noexcept
		{
			if constexpr (requires_promotion)
			{
				return oriented_bounding_box{ promoted_box::transform(promoted_box{ bb },
																	  matrix<promoted_scalar, 4, 4>{ tx }) };
			}
			else
			{
				oriented_bounding_box out{ tx * bb.center, vector_type{} };

				out.axes.x_column() =
					vector_type::normalize(tx * (bb.center + bb.axes.x_column() * bb.extents.x) - out.center,
										   out.extents.x);
				out.axes.y_column() =
					vector_type::normalize(tx * (bb.center + bb.axes.y_column() * bb.extents.y) - out.center,
										   out.extents.y);
				out.axes.z_column() =
					vector_type::normalize(tx * (bb.center + bb.axes.z_column() * bb.extents.z) - out.center,
										   out.extents.z);

				return out;
			}
		}

		/// \brief Transforms the oriented bounding box from one coordinate space to another (in-place).
		///
		/// \param	tx		The transform to apply.
		///
		/// \return	A reference to the box.
		constexpr oriented_bounding_box& transform(MUU_VPARAM(matrix<scalar_type, 4, 4>) tx) noexcept
		{
			return *this = transform(*this, tx);
		}

			/// @}
	#endif // transformation

	#if 1 // intersection ------------------------------------------------------------------------------
		  /// \name Intersection
		  /// @{

		//--------------------------------
		// obb x aabb
		//--------------------------------

		/// \brief	Returns true if an oriented bounding box intersects an axis-aligned bounding_box.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(oriented_bounding_box) obb,
														MUU_VPARAM(bounding_box<scalar_type>) aabb) noexcept;

		/// \brief	Returns true if the oriented bounding box intersects an axis-aligned bounding_box.
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(bounding_box<scalar_type>) aabb) const noexcept;

		//--------------------------------
		// obb x obb
		//--------------------------------

		/// \brief	Returns true if two oriented bounding boxes intersect.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(oriented_bounding_box) bb1,
														MUU_VPARAM(oriented_bounding_box) bb2) noexcept
		{
			const vector_type corners1[] = {
				bb1.template corner<box_corner::min>(), bb1.template corner<box_corner::x>(),
				bb1.template corner<box_corner::xy>(),	bb1.template corner<box_corner::xz>(),
				bb1.template corner<box_corner::y>(),	bb1.template corner<box_corner::yz>(),
				bb1.template corner<box_corner::z>(),	bb1.template corner<box_corner::max>()
			};

			const vector_type corners2[] = {
				bb2.template corner<box_corner::min>(), bb2.template corner<box_corner::x>(),
				bb2.template corner<box_corner::xy>(),	bb2.template corner<box_corner::xz>(),
				bb2.template corner<box_corner::y>(),	bb2.template corner<box_corner::yz>(),
				bb2.template corner<box_corner::z>(),	bb2.template corner<box_corner::max>()
			};

			for (auto* bb : { &bb1, &bb2 })
			{
				MUU_ASSUME(bb);

				for (size_t i = 0; i < 3; i++)
				{
					sat_tester tester1{ bb->axes.m[i], corners1[0] };
					for (size_t c = 1; c < 8; c++)
						tester1.add(bb->axes.m[i], corners1[c]);

					sat_tester tester2{ bb->axes.m[i], corners2[0] };
					for (size_t c = 1; c < 8; c++)
						tester2.add(bb->axes.m[i], corners2[c]);

					if (!tester1(tester2))
						return false;
				}
			}
			return true;
		}

		/// \brief	Returns true if two oriented bounding boxes intersect.
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL intersects(MUU_VPARAM(oriented_bounding_box) bb) const noexcept
		{
			return intersects(*this, bb);
		}

			/// @}
	#endif // intersection
	};

	/// \cond

	template <typename CX, typename CY, typename CZ, typename EX, typename EY, typename EZ>
	oriented_bounding_box(CX, CY, CZ, EX, EY, EZ)
		-> oriented_bounding_box<impl::highest_ranked<CX, CY, CZ, EX, EY, EZ>>;

	template <typename C, typename E>
	oriented_bounding_box(vector<C, 3>, vector<E, 3>) -> oriented_bounding_box<impl::highest_ranked<C, E>>;

	MUU_CONSTRAINED_TEMPLATE((all_arithmetic<CX, CY, CZ>), typename CX, typename CY, typename CZ, typename E)
	oriented_bounding_box(CX, CY, CZ, vector<E, 3>)->oriented_bounding_box<impl::highest_ranked<CX, CY, CZ, E>>;

	MUU_CONSTRAINED_TEMPLATE((all_arithmetic<EX, EY, EZ>), typename C, typename EX, typename EY, typename EZ)
	oriented_bounding_box(vector<C, 3>, EX, EY, EZ)->oriented_bounding_box<impl::highest_ranked<C, EX, EY, EZ>>;

	MUU_CONSTRAINED_TEMPLATE(is_arithmetic<E>, typename C, typename E)
	oriented_bounding_box(vector<C, 3>, E)->oriented_bounding_box<impl::highest_ranked<C, E>>;

	template <typename E>
	oriented_bounding_box(vector<E, 3>) -> oriented_bounding_box<E>;

	MUU_CONSTRAINED_TEMPLATE(is_arithmetic<E>, typename E)
	oriented_bounding_box(E)->oriented_bounding_box<std::remove_cv_t<E>>;

	/// \endcond
}

namespace std
{
	/// \brief Specialization of std::tuple_size for muu::bounding_box.
	template <typename Scalar>
	struct tuple_size<muu::oriented_bounding_box<Scalar>>
	{
		static constexpr size_t value = 3;
	};

	/// \brief Specialization of std::tuple_element for muu::bounding_box.
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
	/// \ingroup	constants
	/// \see		muu::oriented_bounding_box
	///
	/// \brief		Oriented bounding box constants.
	template <typename Scalar>
	struct constants<oriented_bounding_box<Scalar>>
	{
		using scalars = constants<Scalar>;
		using vectors = constants<vector<Scalar, 3>>;

		/// \brief An oriented bounding box with all members initialized to zero.
		static constexpr oriented_bounding_box<Scalar> zero = { vectors::zero, vectors::zero };

		/// \brief A oriented bounding box centered at the origin with sides of length 1.
		static constexpr oriented_bounding_box<Scalar> unit = { vectors::zero, vectors::one_over_two };
	};
}

MUU_POP_PRECISE_MATH;

#endif //===============================================================================================================

//======================================================================================================================
// FREE FUNCTIONS
#if 1

namespace muu
{
	/// \ingroup	infinity_or_nan
	/// \relatesalso	muu::oriented_bounding_box
	///
	/// \brief	Returns true if any of the scalar components of an oriented_bounding_box are infinity or NaN.
	template <typename S>
	MUU_PURE_INLINE_GETTER
	constexpr bool infinity_or_nan(const oriented_bounding_box<S>& bb) noexcept
	{
		return oriented_bounding_box<S>::infinity_or_nan(bb);
	}

	/// \ingroup	approx_equal
	/// \relatesalso	muu::oriented_bounding_box
	///
	/// \brief		Returns true if two oriented bounding boxes are approximately equal.
	template <typename S, typename T>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL approx_equal(const oriented_bounding_box<S>& bb1,
											   const oriented_bounding_box<T>& bb2,
											   epsilon_type<S, T> epsilon = default_epsilon<S, T>) noexcept
	{
		return oriented_bounding_box<S>::approx_equal(bb1, bb2, epsilon);
	}

	/// \ingroup	approx_zero
	/// \relatesalso	muu::oriented_bounding_box
	///
	/// \brief		Returns true if all the scalar components of an oriented_bounding_box are approximately equal to
	/// zero.
	template <typename S>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL approx_zero(const oriented_bounding_box<S>& bb,
											  S epsilon = default_epsilon<S>) noexcept
	{
		return oriented_bounding_box<S>::approx_zero(bb, epsilon);
	}

	/// \ingroup		degenerate
	/// \relatesalso	muu::oriented_bounding_box
	///
	/// \brief	Returns true if a box is degenerate (i.e. any of its extents are less than or equal to zero).
	template <typename S>
	MUU_PURE_INLINE_GETTER
	constexpr bool degenerate(const oriented_bounding_box<S>& bb) noexcept
	{
		return oriented_bounding_box<S>::degenerate(bb);
	}
}

#endif //===============================================================================================================

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"

/// \cond
#include "impl/bounding_box_x_oriented_bounding_box.h"
/// \endcond
