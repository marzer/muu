// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#ifndef MUU_RAY_H
#define MUU_RAY_H

/// \file
/// \brief  Contains the definition of muu::ray.

#include "impl/geometry_common.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(float_control(except, off))

//======================================================================================================================
// RAY CLASS
#if 1

namespace muu
{
	/// \brief A ray
	/// \ingroup math
	///
	/// \tparam	Scalar      The ray's scalar component type. Must be a floating-point type.
	template <typename Scalar>
	struct MUU_TRIVIAL_ABI ray //
		MUU_HIDDEN_BASE(impl::storage_base<ray<Scalar>>)
	{
		static_assert(!std::is_reference_v<Scalar>, "Ray scalar type cannot be a reference");
		static_assert(!is_cv<Scalar>, "Ray scalar type cannot be const- or volatile-qualified");
		static_assert(std::is_trivially_constructible_v<Scalar>	  //
						  && std::is_trivially_copyable_v<Scalar> //
						  && std::is_trivially_destructible_v<Scalar>,
					  "Ray scalar type must be trivially constructible, copyable and destructible");
		static_assert(is_signed<Scalar>, "Ray scalar type must be signed");
		static_assert(is_floating_point<Scalar>, "Ray type must be a floating-point type");

		/// \brief The ray's scalar type.
		using scalar_type = Scalar;

		/// \brief The three-dimensional #muu::vector with the same #scalar_type as the ray.
		using vector_type = vector<scalar_type, 3>;

		/// \brief The result type returned by a raycast test.
		using result_type = std::optional<scalar_type>;

	  private:
		/// \cond

		using base = impl::storage_base<ray<Scalar>>;
		static_assert(sizeof(base) == (sizeof(vector_type) * 2), "Rays should not have padding");

		using rays = impl::rays_common<Scalar>;

		using promoted_scalar				 = promote_if_small_float<scalar_type>;
		using promoted_vec					 = vector<promoted_scalar, 3>;
		static constexpr bool is_small_float = impl::is_small_float_<scalar_type>;

		/// \endcond

	  public:
	#if MUU_DOXYGEN

		/// \brief	The ray's origin point.
		vector_type origin;

		/// \brief	The ray's direction.
		vector_type direction;

	#endif // DOXYGEN

	#if 1 // constructors ----------------------------------------------------------------------------------------------

		/// \brief Default constructor. Values are not initialized.
		ray() noexcept = default;

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr ray(const ray&) noexcept = default;

		/// \brief Copy-assigment operator.
		constexpr ray& operator=(const ray&) noexcept = default;

		/// \brief	Constructs a ray from an origin and direction.
		MUU_NODISCARD_CTOR
		constexpr ray(const vector_type& o, const vector_type& dir) noexcept //
			: base{ o, dir }
		{}

		/// \brief	Constructs a ray from an origin and direction (in scalar form).
		MUU_NODISCARD_CTOR
		constexpr ray(scalar_type o_x,
					  scalar_type o_y,
					  scalar_type o_z,
					  scalar_type dir_x,
					  scalar_type dir_y,
					  scalar_type dir_z) noexcept //
			: base{ vector_type{ o_x, o_y, o_z }, vector_type{ dir_x, dir_y, dir_z } }
		{}

		/// \brief	Converting constructor.
		template <typename S>
		MUU_NODISCARD_CTOR
		explicit constexpr ray(const ray<S>& r) noexcept //
			: base{ vector_type{ r.origin }, vector_type{ r.direction } }
		{}

		/// \brief Constructs a ray from an implicitly bit-castable type.
		///
		/// \tparam T	A bit-castable type.
		///
		/// \see muu::allow_implicit_bit_cast
		MUU_CONSTRAINED_TEMPLATE((allow_implicit_bit_cast<T, ray>), typename T)
		MUU_NODISCARD_CTOR
		/*implicit*/
		constexpr ray(const T& obj) noexcept //
			: base{ muu::bit_cast<base>(obj) }
		{
			static_assert(sizeof(T) == sizeof(base), "Bit-castable types must be the same size");
			static_assert(std::is_trivially_copyable_v<T>, "Bit-castable types must be trivially-copyable");
		}

	#endif // constructors

	#if 1 // equality (exact) -------------------------------------------------------------------------------------
		/// \name Equality (exact)
		/// @{

		/// \brief		Returns true if two rays are exactly equal.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		template <typename T>
		MUU_PURE_GETTER
		friend constexpr bool MUU_VECTORCALL operator==(MUU_VPARAM(ray) lhs, const ray<T>& rhs) noexcept
		{
			return lhs.origin == rhs.origin //
				&& lhs.direction == rhs.direction;
		}

		/// \brief	Returns true if two rays are not exactly equal.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		template <typename T>
		MUU_PURE_INLINE_GETTER
		friend constexpr bool MUU_VECTORCALL operator!=(MUU_VPARAM(ray) lhs, const ray<T>& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		/// \brief	Returns true if any of the scalars in a ray are infinity or NaN.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL infinity_or_nan(MUU_VPARAM(ray) r) noexcept
		{
			return vector_type::infinity_or_nan(r.origin) //
				&& vector_type::infinity_or_nan(r.direction);
		}

		/// \brief	Returns true if any of the scalars in the ray are infinity or NaN.
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

		/// \brief	Returns true if two rays are approximately equal.
		template <typename T>
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_equal(
			MUU_VPARAM(ray) ray1,
			const ray<T>& ray2,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) noexcept
		{
			return vector_type::approx_equal(ray1.origin, ray2.origin, epsilon) //
				&& vector_type::approx_equal(ray1.direction, ray2.direction, epsilon);
		}

		/// \brief	Returns true if the ray is approximately equal to another.
		template <typename T>
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL approx_equal(
			const ray<T>& r,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) const noexcept
		{
			return approx_equal(*this, r, epsilon);
		}

				/// @}
	#endif // equality (approx)

	#if 1 // hit tests -------------------------------------------------------------------------------------
		/// \name Hit tests
		/// @{

		//--------------------------------
		// ray x plane
		//--------------------------------

		MUU_PURE_GETTER
		static constexpr result_type MUU_VECTORCALL hits(MUU_VPARAM(ray) r, MUU_VPARAM(plane<scalar_type>) p) noexcept;

		MUU_PURE_INLINE_GETTER
		constexpr result_type MUU_VECTORCALL hits(MUU_VPARAM(plane<scalar_type>) p) const noexcept
		{
			return hits(*this, p);
		}

		//--------------------------------
		// ray x triangle
		//--------------------------------

		MUU_PURE_GETTER
		static constexpr result_type MUU_VECTORCALL hits(MUU_VPARAM(ray) r,
														 MUU_VPARAM(triangle<scalar_type>) tri) noexcept;

		MUU_PURE_INLINE_GETTER
		constexpr result_type MUU_VECTORCALL hits(MUU_VPARAM(triangle<scalar_type>) tri) const noexcept
		{
			return hits(*this, tri);
		}

				/// @}
	#endif // hit tests

	#if 1 // misc ---------------------------------------------------------------------------------------------------

		/// \brief Writes a ray out to a text stream.
		template <typename Char, typename Traits>
		friend std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os, const ray& r)
		{
			const impl::compound_vector_elem<Scalar> elems[]{ { &r.origin.x, 3 }, //
															  { &r.direction.x, 3 } };
			impl::print_compound_vector(os, elems);
			return os;
		}

	#endif // misc
	};

	/// \cond

	template <typename T1, typename T2, typename T3>
	ray(vector<T1, 3>, vector<T2, 3>) -> ray<impl::highest_ranked<T1, T2, T3>>;

	MUU_CONSTRAINED_TEMPLATE((all_arithmetic<X1, Y1, Z1, X2, Y2, Z2>),
							 typename X1,
							 typename Y1,
							 typename Z1,
							 typename X2,
							 typename Y2,
							 typename Z2)
	ray(X1, Y1, Z1, X2, Y2, Z2)->ray<impl::highest_ranked<X1, Y1, Z1, X2, Y2, Z2>>;

	template <typename T>
	ray(const ray<T>&) -> ray<T>;

	/// \endcond
}

namespace std
{
	/// \brief Specialization of std::tuple_size for muu::ray.
	template <typename Scalar>
	struct tuple_size<muu::ray<Scalar>>
	{
		static constexpr size_t value = 2;
	};

	/// \brief Specialization of std::tuple_element for muu::ray.
	template <size_t P, typename Scalar>
	struct tuple_element<P, muu::ray<Scalar>>
	{
		static_assert(P < 2);

		using type = muu::vector<Scalar, 3>;
	};
}

#endif //===============================================================================================================

//======================================================================================================================
// FREE FUNCTIONS
#if 1

namespace muu
{
	/// \ingroup		infinity_or_nan
	/// \relatesalso	muu::ray
	///
	/// \brief	Returns true if any of the scalars in a ray are infinity or NaN.
	template <typename S>
	MUU_PURE_INLINE_GETTER
	constexpr bool infinity_or_nan(const ray<S>& r) noexcept
	{
		return ray<S>::infinity_or_nan(r);
	}

	/// \ingroup		approx_equal
	/// \relatesalso	muu::ray
	///
	/// \brief		Returns true if two rays are approximately equal.
	template <typename S, typename T>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL approx_equal(const ray<S>& ray1,
											   const ray<T>& ray2,
											   epsilon_type<S, T> epsilon = default_epsilon<S, T>) noexcept
	{
		return ray<S>::approx_equal(ray1, ray2, epsilon);
	}
}

#endif //===============================================================================================================

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"

/// \cond
#ifdef MUU_PLANE_H
	#include "impl/ray_x_plane.h"
#endif
#ifdef MUU_TRIANGLE_H
	#include "impl/ray_x_triangle.h"
#endif
/// \endcond

#endif // MUU_RAY_H
