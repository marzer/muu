// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::quaternion.

#include "vector.h"
#include "axis_angle.h"
#include "euler_angles.h"
#include "impl/std_initializer_list.h"
#include "impl/std_iosfwd.h"
#include "impl/matrix_base.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_DISABLE_SHADOW_WARNINGS;
MUU_DISABLE_ARITHMETIC_WARNINGS;
MUU_PRAGMA_MSVC(float_control(except, off))
MUU_PRAGMA_MSVC(float_control(precise, off))

//======================================================================================================================
// IMPLEMENTATION DETAILS
//======================================================================================================================
/// \cond

namespace muu::impl
{
	template <typename Scalar>
	struct MUU_TRIVIAL_ABI quaternion_
	{
		Scalar s;
		vector<Scalar, 3> v;
	};

	template <typename Scalar>
	inline constexpr bool is_hva<quaternion_<Scalar>> = can_be_hva_of<Scalar, quaternion_<Scalar>>;

	template <typename Scalar>
	inline constexpr bool is_hva<quaternion<Scalar>> = is_hva<quaternion_<Scalar>>;

	template <typename Scalar>
	struct vectorcall_param_<quaternion<Scalar>>
	{
		using type = std::conditional_t<pass_vectorcall_by_value<quaternion_<Scalar>>, //
										quaternion<Scalar>,
										const quaternion<Scalar>&>;
	};
}

namespace muu
{
	template <typename From, typename Scalar>
	inline constexpr bool allow_implicit_bit_cast<From, impl::quaternion_<Scalar>> =
		allow_implicit_bit_cast<From, quaternion<Scalar>>;
}

/// \endcond

//======================================================================================================================
// QUATERNION
//======================================================================================================================

namespace muu
{
	/// \brief A quaternion.
	/// \ingroup math
	///
	/// \tparam	Scalar      The quaternion's scalar component type. Must be a floating-point type.
	///
	/// \see
	/// 	 - muu::vector
	/// 	 - muu::axis_angle
	/// 	 - muu::euler_angles
	template <typename Scalar>
	struct MUU_TRIVIAL_ABI quaternion //
		MUU_HIDDEN_BASE(impl::quaternion_<Scalar>)
	{
		static_assert(!std::is_reference_v<Scalar>, "Quaternion scalar type cannot be a reference");
		static_assert(!is_cv<Scalar>, "Quaternion scalar type cannot be const- or volatile-qualified");
		static_assert(std::is_trivially_constructible_v<Scalar>	  //
						  && std::is_trivially_copyable_v<Scalar> //
						  && std::is_trivially_destructible_v<Scalar>,
					  "Quaternion scalar type must be trivially constructible, copyable and destructible");
		static_assert(is_floating_point<Scalar>, "Quaternion scalar type must be a floating-point type");

		/// \brief The type of each scalar component stored in this quaternion.
		using scalar_type = Scalar;

		/// \brief The three-dimensional #muu::vector with the same #scalar_type as this quaternion.
		using vector_type = vector<scalar_type, 3>;

		/// \brief The #muu::axis_angle with the same #scalar_type as this quaternion.
		using axis_angle_type = axis_angle<scalar_type>;

		/// \brief The #muu::euler_angles with the same #scalar_type as this quaternion.
		using euler_type = euler_angles<scalar_type>;

		/// \brief Compile-time constants for this quaternion.
		using constants = muu::constants<quaternion>;

	  private:
		/// \cond

		template <typename>
		friend struct quaternion;

		using base = impl::quaternion_<Scalar>;
		static_assert(sizeof(base) == (sizeof(scalar_type) * 4), "Quaternions should not have padding");

		static constexpr bool is_small_float = impl::is_small_float_<scalar_type>;
		using promoted_scalar				 = promote_if_small_float<scalar_type>;
		using promoted_quat					 = quaternion<promoted_scalar>;
		using promoted_vec					 = vector<promoted_scalar, 3>;

		using scalar_constants = muu::constants<scalar_type>;

		/// \endcond

	  public:
#ifdef DOXYGEN

		/// \brief The quaternion's scalar (real) part.
		scalar_type s;
		/// \brief The quaternion's vector (imaginary) part.
		vector_type v;

#endif // DOXYGEN

#if 1 // constructors ---------------------------------------------------------------------------------------------

		/// \brief Default constructor. Values are not initialized.
		quaternion() noexcept = default;

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr quaternion(const quaternion&) noexcept = default;

		/// \brief Copy-assigment operator.
		constexpr quaternion& operator=(const quaternion&) noexcept = default;

		/// \brief	Constructs a quaternion from raw scalar values.
		///
		/// \param	s	Initial value for the scalar (real) part.
		/// \param	vx	Initial value for the vector (imaginary) part's X component.
		/// \param	vy	Initial value for the vector (imaginary) part's Y component.
		/// \param	vz	Initial value for the vector (imaginary) part's Z component.
		MUU_NODISCARD_CTOR
		constexpr quaternion(scalar_type s, scalar_type vx, scalar_type vy, scalar_type vz) noexcept
			: base{ s, vector_type{ vx, vy, vz } }
		{}

		/// \brief	Constructs a quaternion from a scalar and a vector.
		///
		/// \param	s	Initial value for the scalar (real) part.
		/// \param	v	Initial value for the vector (imaginary) part.
		MUU_NODISCARD_CTOR
		constexpr quaternion(scalar_type s, const vector_type& v) noexcept //
			: base{ s, v }
		{}

		/// \brief Converting constructor.
		template <typename T>
		MUU_NODISCARD_CTOR
		explicit constexpr quaternion(const quaternion<T>& quat) noexcept
			: base{ static_cast<scalar_type>(quat.s), vector_type{ quat.v } }
		{}

		/// \brief	Constructs a quaternion from an axis-angle rotation.
		MUU_NODISCARD_CTOR
		explicit constexpr quaternion(const axis_angle_type& aa) noexcept //
			: quaternion{ from_axis_angle(aa) }
		{}

		/// \brief	Constructs a quaternion from a set of euler angles.
		MUU_NODISCARD_CTOR
		explicit constexpr quaternion(const euler_type& euler) noexcept //
			: quaternion{ from_euler(euler) }
		{}

		/// \brief Constructs a quaternion from a 3x3 3D rotation matrix.
		MUU_NODISCARD_CTOR
		explicit constexpr quaternion(MUU_VC_PARAM(matrix<scalar_type, 3, 3>) rot) noexcept //
			: quaternion{ from_rotation(rot) }
		{}

		/// \brief Constructs a quaternion from the lower 3x3 part of a 3x4 3D transform matrix.
		MUU_NODISCARD_CTOR
		explicit constexpr quaternion(MUU_VC_PARAM(matrix<scalar_type, 3, 4>) rot) noexcept //
			: quaternion{ from_rotation(rot) }
		{}

		/// \brief Constructs a quaternion from the lower 3x3 part of a 4x4 3D transform matrix.
		MUU_NODISCARD_CTOR
		explicit constexpr quaternion(MUU_VC_PARAM(matrix<scalar_type, 4, 4>) rot) noexcept //
			: quaternion{ from_rotation(rot) }
		{}

		/// \brief Constructs a quaternion from an implicitly bit-castable type.
		///
		/// \tparam T	A bit-castable type.
		///
		/// \see muu::allow_implicit_bit_cast
		MUU_CONSTRAINED_TEMPLATE((allow_implicit_bit_cast<T, quaternion> && !impl::is_quaternion_<T>), typename T)
		MUU_NODISCARD_CTOR
		/*implicit*/
		constexpr quaternion(const T& blittable) noexcept //
			: base{ muu::bit_cast<base>(blittable) }
		{
			static_assert(sizeof(T) == sizeof(base), "Bit-castable types must be the same size as the quaternion");
			static_assert(std::is_trivially_copyable_v<T>, "Bit-castable types must be trivially-copyable");
		}

#endif // constructors

#if 1 // value accessors ------------------------------------------------------------------------------------------

	  private:
		/// \cond

		template <size_t Index, typename T>
		MUU_PURE_INLINE_GETTER
		static constexpr auto& do_get(T& quat) noexcept
		{
			static_assert(Index < 2, "Element index out of range");

			if constexpr (Index == 0)
				return quat.s;
			if constexpr (Index == 1)
				return quat.v;
		}

		/// \endcond

	  public:
		/// \brief Gets a reference to the value at a specific index.
		///
		/// \tparam Index  The index of the value to retrieve, where s == 0, v == 1.
		///
		/// \return  A reference to the selected value.
		template <size_t Index>
		MUU_PURE_INLINE_GETTER
		MUU_ATTR(flatten)
		constexpr const auto& get() const noexcept
		{
			return do_get<Index>(*this);
		}

		/// \brief Gets a reference to the value at a specific index.
		///
		/// \tparam Index  The index of the value to retrieve, where s == 0, v == 1.
		///
		/// \return  A reference to the selected value.
		template <size_t Index>
		MUU_PURE_INLINE_GETTER
		MUU_ATTR(flatten)
		constexpr auto& get() noexcept
		{
			return do_get<Index>(*this);
		}

		/// \brief Returns a pointer to the first scalar component in the quaternion.
		MUU_PURE_INLINE_GETTER
		MUU_ATTR(flatten)
		constexpr const scalar_type* data() const noexcept
		{
			return &do_get<0>(*this);
		}

		/// \brief Returns a pointer to the first scalar component in the quaternion.
		MUU_PURE_INLINE_GETTER
		MUU_ATTR(flatten)
		constexpr scalar_type* data() noexcept
		{
			return &do_get<0>(*this);
		}

#endif // value accessors

#if 1 // equality (exact) ----------------------------------------------------------------------------------------------
		/// \name Equality (exact)
		/// @{

		/// \brief		Returns true if two quaternions are exactly equal.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		MUU_CONSTRAINED_TEMPLATE((!MUU_HAS_VECTORCALL || impl::pass_vectorcall_by_reference<quaternion, quaternion<T>>),
								 typename T)
		MUU_PURE_GETTER
		friend constexpr bool operator==(const quaternion& lhs, const quaternion<T>& rhs) noexcept
		{
			if constexpr (std::is_same_v<scalar_type, T>)
			{
				return lhs.s == rhs.s && lhs.v == rhs.v;
			}
			else
			{
				using type = impl::equality_check_type<scalar_type, T>;

				if (static_cast<type>(lhs.s) != static_cast<type>(rhs.s))
					return false;
				return lhs.v == rhs.v;
			}
		}

	#if MUU_HAS_VECTORCALL

		MUU_CONSTRAINED_TEMPLATE((impl::pass_vectorcall_by_value<quaternion, quaternion<T>>), typename T)
		MUU_CONST_GETTER
		friend constexpr bool MUU_VECTORCALL operator==(quaternion lhs, quaternion<T> rhs) noexcept
		{
			if constexpr (std::is_same_v<scalar_type, T>)
			{
				return lhs.s == rhs.s && lhs.v == rhs.v;
			}
			else
			{
				using type = impl::equality_check_type<scalar_type, T>;

				if (static_cast<type>(lhs.s) != static_cast<type>(rhs.s))
					return false;
				return lhs.v == rhs.v;
			}
		}

	#endif // MUU_HAS_VECTORCALL

		/// \brief	Returns true if two quaternions are not exactly equal.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		MUU_CONSTRAINED_TEMPLATE((!MUU_HAS_VECTORCALL || impl::pass_vectorcall_by_reference<quaternion, quaternion<T>>),
								 typename T)
		MUU_PURE_GETTER
		friend constexpr bool operator!=(const quaternion& lhs, const quaternion<T>& rhs) noexcept
		{
			return !(lhs == rhs);
		}

	#if MUU_HAS_VECTORCALL

		MUU_CONSTRAINED_TEMPLATE((impl::pass_vectorcall_by_value<quaternion, quaternion<T>>), typename T)
		MUU_CONST_GETTER
		friend constexpr bool MUU_VECTORCALL operator!=(quaternion lhs, quaternion<T> rhs) noexcept
		{
			return !(lhs == rhs);
		}

	#endif // MUU_HAS_VECTORCALL

		/// \brief	Returns true if all the scalar components of a quaternion are exactly zero.
		///
		/// \remarks	This is an exact check;
		///				use #approx_zero() if you want an epsilon-based "near-enough" check.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL zero(MUU_VC_PARAM(quaternion) q) noexcept
		{
			return q.s == scalar_constants::zero && vector_type::zero(q.v);
		}

		/// \brief	Returns true if all the scalar components of the quaternion are exactly zero.
		///
		/// \remarks	This is an exact check;
		///				use #approx_zero() if you want an epsilon-based "near-enough" check.
		MUU_PURE_GETTER
		constexpr bool zero() const noexcept
		{
			return zero(*this);
		}

		/// \brief	Returns true if any of the scalar components of a quaternion are infinity or NaN.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL infinity_or_nan(MUU_VC_PARAM(quaternion) q) noexcept
		{
			return muu::infinity_or_nan(q.s) || vector_type::infinity_or_nan(q.v);
		}

		/// \brief	Returns true if any of the scalar components of the quaternion are infinity or NaN.
		MUU_PURE_GETTER
		constexpr bool infinity_or_nan() const noexcept
		{
			return infinity_or_nan(*this);
		}

		/// @}
#endif // equality (exact)

#if 1 // equality (approx) -------------------------------------------------------------------------------------
		/// \name Equality (approximate)
		/// @{

		/// \brief	Returns true if two quaternions are approximately equal.
		MUU_CONSTRAINED_TEMPLATE((!MUU_HAS_VECTORCALL || impl::pass_vectorcall_by_reference<quaternion, quaternion<T>>),
								 typename T)
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_equal(
			const quaternion& q1,
			const quaternion<T>& q2,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) noexcept
		{
			if constexpr (std::is_same_v<scalar_type, T>)
			{
				return muu::approx_equal(q1.s, q2.s, epsilon) && vector_type::approx_equal(q1.v, q2.v, epsilon);
			}
			else
			{
				using type = impl::equality_check_type<scalar_type, T>;

				return muu::approx_equal(static_cast<type>(q1.s), static_cast<type>(q2.s), static_cast<type>(epsilon))
					&& vector_type::approx_equal(q1.v, q2.v, epsilon);
			}
		}

	#if MUU_HAS_VECTORCALL

		MUU_CONSTRAINED_TEMPLATE((impl::pass_vectorcall_by_value<quaternion, quaternion<T>>), typename T)
		MUU_CONST_GETTER
		static constexpr bool MUU_VECTORCALL approx_equal(
			quaternion q1,
			quaternion<T> q2,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) noexcept
		{
			if constexpr (std::is_same_v<scalar_type, T>)
			{
				return muu::approx_equal(q1.s, q2.s, epsilon) && vector_type::approx_equal(q1.v, q2.v, epsilon);
			}
			else
			{
				using type = impl::equality_check_type<scalar_type, T>;

				return muu::approx_equal(static_cast<type>(q1.s), static_cast<type>(q2.s), static_cast<type>(epsilon))
					&& vector_type::approx_equal(q1.v, q2.v, epsilon);
			}
		}

	#endif // MUU_HAS_VECTORCALL

		/// \brief	Returns true if the quaternion is approximately equal to another.
		MUU_CONSTRAINED_TEMPLATE((!MUU_HAS_VECTORCALL || impl::pass_vectorcall_by_reference<quaternion<T>>), typename T)
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL approx_equal(
			const quaternion<T>& q,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) const noexcept
		{
			return approx_equal(*this, q, epsilon);
		}

	#if MUU_HAS_VECTORCALL

		MUU_CONSTRAINED_TEMPLATE(impl::pass_vectorcall_by_value<quaternion<T>>, typename T)
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL approx_equal(
			quaternion<T> q,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) const noexcept
		{
			return approx_equal(*this, q, epsilon);
		}

	#endif // MUU_HAS_VECTORCALL

		/// \brief	Returns true if all the scalar components in a quaternion are approximately equal to zero.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_zero(MUU_VC_PARAM(quaternion) q,
														 scalar_type epsilon = default_epsilon<scalar_type>) noexcept
		{
			return muu::approx_zero(q.s, epsilon) && vector_type::approx_zero(q.v, epsilon);
		}

		/// \brief	Returns true if all the scalar components in the quaternion are approximately equal to zero.
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL approx_zero(scalar_type epsilon = default_epsilon<scalar_type>) const noexcept
		{
			return approx_zero(*this, epsilon);
		}

		/// @}
#endif // equality (approx)

#if 1 // dot product ------------------------------------------------------------------------------------
		/// \name Dot product
		/// @{

		/// \brief	Returns the dot product of two quaternions.
		MUU_PURE_GETTER
		static constexpr scalar_type MUU_VECTORCALL dot(MUU_VC_PARAM(quaternion) q1,
														MUU_VC_PARAM(quaternion) q2) noexcept
		{
			if constexpr (is_small_float)
			{
				return static_cast<scalar_type>(promoted_quat::dot(promoted_quat{ q1 }, promoted_quat{ q2 }));
			}
			else
			{
				return q1.s * q2.s + vector_type::dot(q1.v, q2.v);
			}
		}

		/// \brief	Returns the dot product of this and another quaternion.
		MUU_PURE_GETTER
		constexpr scalar_type MUU_VECTORCALL dot(MUU_VC_PARAM(quaternion) q) const noexcept
		{
			return dot(*this, q);
		}

		/// @}
#endif // dot product

#if 1 // normalization --------------------------------------------------------------------------------------------
		/// \name Normalization
		/// @{

		/// \brief		Normalizes a quaternion.
		///
		/// \param q	The quaternion to normalize.
		///
		/// \return		A normalized copy of the input quaternion.
		MUU_PURE_GETTER
		static constexpr quaternion MUU_VECTORCALL normalize(MUU_VC_PARAM(quaternion) q) noexcept
		{
			if constexpr (is_small_float)
			{
				return quaternion{ promoted_quat::normalize(promoted_quat{ q }) };
			}
			else
			{
				const auto inv_length = scalar_type{ 1 } / muu::sqrt(dot(q, q));

				return quaternion{ q.s * inv_length, q.v * inv_length };
			}
		}

		/// \brief		Normalizes the quaternion (in-place).
		///
		/// \return		A reference to the quaternion.
		constexpr quaternion& normalize() noexcept
		{
			return *this = normalize(*this);
		}

		/// \brief Returns true if a quaternion is normalized (i.e. has a length of 1).
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL normalized(MUU_VC_PARAM(quaternion) q) noexcept
		{
			constexpr promoted_scalar epsilon = promoted_scalar{ 1 }
											  / (100ull * (sizeof(scalar_type) >= sizeof(float) ? 10000ull : 1ull)
												 * (sizeof(scalar_type) >= sizeof(double) ? 10000ull : 1ull));

			if constexpr (is_small_float)
			{
				return muu::approx_equal(promoted_quat::dot(promoted_quat{ q }, promoted_quat{ q }),
										 promoted_scalar{ 1 },
										 epsilon);
			}
			else
			{
				return muu::approx_equal(dot(q, q), scalar_type{ 1 }, epsilon);
			}
		}

		/// \brief Returns true if the quaternion is normalized (i.e. has a length of 1).
		MUU_PURE_GETTER
		constexpr bool normalized() const noexcept
		{
			return normalized(*this);
		}

		/// @}
#endif // normalization

#if 1 // conjugate --------------------------------------------------------------------------------------------
		/// \name Conjugate
		/// @{

		/// \brief		Returns the conjugate of a quaternion.
		MUU_PURE_GETTER
		static constexpr quaternion MUU_VECTORCALL conjugate(MUU_VC_PARAM(quaternion) q) noexcept
		{
			return { q.s, -q.v };
		}

		/// \brief		Returns the conjugate of the quaternion.
		MUU_PURE_GETTER
		constexpr quaternion conjugate() const noexcept
		{
			return { base::s, -base::v };
		}

		/// @}
#endif // conjugate

#if 1 // rotations ----------------------------------------------------------------------------------------
		/// \name Rotations
		/// @{

		/// \brief Creates a quaternion from an axis-angle rotation.
		///
		/// \param axis		Axis to rotate around. Must be normalized.
		/// \param angle	Angle to rotate by (in radians).
		///
		/// \return A quaternion encoding the given axis-angle rotation.
		MUU_PURE_GETTER
		static constexpr quaternion MUU_VECTORCALL from_axis_angle(MUU_VC_PARAM(vector_type) axis,
																   scalar_type angle) noexcept
		{
			if constexpr (is_small_float)
			{
				return quaternion{ promoted_quat::from_axis_angle(promoted_vec{ axis },
																  static_cast<promoted_scalar>(angle)) };
			}
			else
			{
				MUU_FMA_BLOCK;

				angle *= scalar_constants::one_over_two;
				return quaternion{
					muu::cos(angle),	   // scalar
					muu::sin(angle) * axis // vector
				};
			}
		}

		/// \brief Creates a quaternion from an axis-angle rotation.
		MUU_PURE_GETTER
		static constexpr quaternion MUU_VECTORCALL from_axis_angle(MUU_VC_PARAM(axis_angle_type) aa) noexcept
		{
			return from_axis_angle(aa.axis, aa.angle);
		}

		/// \brief	Extracts an axis-angle rotation from a quaternion.
		///
		/// \param	quat			The quaternion to convert. Must be normalized.
		/// \param	shortest_path	Should the returned values be derived from the shortest path?
		/// 						`true` is more intuitive but means that the returned values may not convert back
		/// 						to the original quaternion if used with #from_axis_angle().
		///
		/// \returns	An axis-angle rotation representing the rotation stored in the given quaternion.
		MUU_PURE_GETTER
		static constexpr axis_angle_type MUU_VECTORCALL to_axis_angle(MUU_VC_PARAM(quaternion) quat,
																	  bool shortest_path = true) noexcept
		{
			if constexpr (is_small_float)
			{
				return axis_angle_type{ promoted_quat::to_axis_angle(promoted_quat{ quat }, shortest_path) };
			}
			else
			{
				MUU_FMA_BLOCK;
				const auto len = vector_type::length(quat.v);
				const auto correction =
					shortest_path && quat.s < scalar_constants::zero ? scalar_type{ -1 } : scalar_type{ 1 };

				if MUU_UNLIKELY(len == scalar_type{})
				{
					// This happens at angle = 0 and 360. All axes are correct, so any will do.
					return {
						vector_type::constants::x_axis,
						scalar_type{ 2 } * muu::atan2(len, quat.s * correction),
					};
				}
				else
				{
					return {
						vector_type{ (quat.v.x * correction) / len,
									 (quat.v.y * correction) / len,
									 (quat.v.z * correction) / len },
						scalar_type{ 2 } * muu::atan2(len, quat.s * correction),
					};
				}
			}
		}

		/// \brief	Extracts an axis-angle rotation from this quaternion.
		///
		/// \param	shortest_path	Should the returned values be derived from the shortest path?
		/// 						`true` is more intuitive but means that the returned values may not convert back
		/// 						to the original quaternion if used with #from_axis_angle().
		///
		/// \returns	An axis-angle rotation representing the rotation stored in the quaternion.
		MUU_PURE_GETTER
		constexpr axis_angle_type to_axis_angle(bool shortest_path = true) const noexcept
		{
			return to_axis_angle(*this, shortest_path);
		}

		/// \brief Creates a quaternion from a set of euler angles.
		///
		/// \param yaw		The rotation around the local up axis (in radians).
		/// \param pitch	The rotation around the local right axis (in radians).
		/// \param roll		The rotation around the local forward axis (in radians).
		///
		/// \return A quaternion encoding the given axis-angle rotation.
		MUU_CONST_GETTER
		static constexpr quaternion MUU_VECTORCALL from_euler(scalar_type yaw,
															  scalar_type pitch,
															  scalar_type roll) noexcept
		{
			if constexpr (is_small_float)
			{
				return quaternion{ promoted_quat::from_euler(static_cast<promoted_scalar>(yaw),
															 static_cast<promoted_scalar>(pitch),
															 static_cast<promoted_scalar>(roll)) };
			}
			else
			{
				MUU_FMA_BLOCK;

				// ensure rotation signs correspond with the aircraft principal axes:
				//	yaw - positive turns toward the right (nose turns east)
				//	pitch - positive pitches upward (nose points up away from ground)
				//	roll - positive rolls to the right (right wing tilts down)
				//	(see https://en.wikipedia.org/wiki/Flight_dynamics_(fixed-wing_aircraft)
				//
				yaw	 = -yaw;
				roll = -roll;

				const auto c1 = muu::cos(pitch * scalar_constants::one_over_two);
				const auto s1 = muu::sin(pitch * scalar_constants::one_over_two);
				const auto c2 = muu::cos(yaw * scalar_constants::one_over_two);
				const auto s2 = muu::sin(yaw * scalar_constants::one_over_two);
				const auto c3 = muu::cos(roll * scalar_constants::one_over_two);
				const auto s3 = muu::sin(roll * scalar_constants::one_over_two);

				return {
					c1 * c2 * c3 - s1 * s2 * s3, // scalar
					s1 * c2 * c3 - c1 * s2 * s3, // vector (x)
					c1 * s2 * c3 + s1 * c2 * s3, // vector (y)
					c1 * c2 * s3 + s1 * s2 * c3	 // vector (z)
				};
			}
		}

		/// \brief Creates a quaternion from an axis-angle rotation.
		MUU_PURE_GETTER
		static constexpr quaternion MUU_VECTORCALL from_euler(MUU_VC_PARAM(euler_type) euler) noexcept
		{
			return from_euler(euler.yaw, euler.pitch, euler.roll);
		}

		/// \brief	Extracts a set of euler angles from a quaternion.
		///
		/// \param	q	The quaternion to convert.
		///
		/// \returns	A set of euler angles representing the rotation stored in the given quaternion.
		MUU_PURE_GETTER
		static constexpr euler_type MUU_VECTORCALL to_euler(MUU_VC_PARAM(quaternion) q) noexcept
		{
			if constexpr (is_small_float)
			{
				return euler_type{ promoted_quat::to_euler(promoted_quat{ q }) };
			}
			else
			{
				MUU_FMA_BLOCK;

				const auto sqw		  = q.s * q.s;
				const auto sqx		  = q.v.x * q.v.x;
				const auto sqy		  = q.v.y * q.v.y;
				const auto sqz		  = q.v.z * q.v.z;
				const auto test		  = q.v.y * q.v.z + q.s * q.v.x;
				const auto correction = sqx + sqy + sqz + sqw;

				// https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/index.htm
				// (note that they pitch around Z, not X; pitch and roll are swapped here)

				constexpr auto threshold = static_cast<scalar_type>(0.49995);

				// singularity at north pole
				if MUU_UNLIKELY(test > threshold * correction)
				{
					return { scalar_type{ -2 } * muu::atan2(q.v.z, q.s),
							 scalar_constants::pi_over_two,
							 scalar_constants::zero };
				}

				// singularity at south pole
				else if MUU_UNLIKELY(test < -threshold * correction)
				{
					return { scalar_type{ 2 } * muu::atan2(q.v.z, q.s),
							 -scalar_constants::pi_over_two,
							 scalar_constants::zero };
				}

				else
				{
					// note the sign of yaw and roll are flipped;
					// see from_euler() for an explanation

					return { -muu::atan2(scalar_type{ -2 } * (q.v.x * q.v.z - q.s * q.v.y), sqw - sqx - sqy + sqz),
							 muu::asin(scalar_type{ 2 } * test / correction),
							 -muu::atan2(scalar_type{ -2 } * (q.v.x * q.v.y - q.s * q.v.z), sqw - sqx + sqy - sqz) };
				}
			}
		}

		/// \brief	Extracts a set of euler angles from this quaternion.
		MUU_PURE_GETTER
		constexpr euler_type to_euler() const noexcept
		{
			return to_euler(*this);
		}

		/// \brief	Creates a quaternion encoding a rotation from one direction vector to another.
		///
		/// \param	from	The source direction.
		/// \param	to		The destination direction.
		///
		/// \returns	A quaternion encoding a rotation between the two direction vectors.
		MUU_PURE_GETTER
		static constexpr quaternion MUU_VECTORCALL from_rotation(MUU_VC_PARAM(vector_type) from,
																 MUU_VC_PARAM(vector_type) to) noexcept
		{
			if constexpr (is_small_float)
			{
				return quaternion{ promoted_quat::from_rotation(promoted_vec{ from }, promoted_vec{ to }) };
			}
			else
			{
				MUU_FMA_BLOCK;

				const auto k_cos_theta = vector_type::dot(from, to);
				const auto k = muu::sqrt(vector_type::length_squared(from) * vector_type::length_squared(from));

				if (muu::approx_equal(k_cos_theta / k, scalar_type{ -1 }))
				{
					// 180 degree rotation around any orthogonal vector
					return quaternion{ scalar_type{}, vector_type::normalize(vector_type::orthogonal(from)) };
				}

				return normalize(quaternion(k_cos_theta + k, vector_type::cross(from, to)));
			}
		}

		/// \brief Creates a quaternion from a 3x3 3D rotation matrix.
		MUU_PURE_GETTER
		static constexpr quaternion MUU_VECTORCALL from_rotation(MUU_VC_PARAM(matrix<Scalar, 3, 3>) rot) noexcept;

		/// \brief Creates a quaternion from the lower 3x3 part of a 3x4 3D transform matrix.
		MUU_PURE_GETTER
		static constexpr quaternion MUU_VECTORCALL from_rotation(MUU_VC_PARAM(matrix<Scalar, 3, 4>) rot) noexcept;

		/// \brief Creates a quaternion from the lower 3x3 part of a 4x4 3D transform matrix.
		MUU_PURE_GETTER
		static constexpr quaternion MUU_VECTORCALL from_rotation(MUU_VC_PARAM(matrix<Scalar, 4, 4>) rot) noexcept;

		/// @}
#endif // rotations

#if 1 // multiplication ------------------------------------------------------------------------------------------------
		/// \name Multiplication
		/// @{

	  private:
		/// \cond

		MUU_PURE_GETTER
		static constexpr quaternion MUU_VECTORCALL multiply(MUU_VC_PARAM(quaternion) lhs,
															MUU_VC_PARAM(quaternion) rhs) noexcept
		{
			if constexpr (is_small_float)
			{
				return quaternion{ promoted_quat::multiply(promoted_quat{ lhs }, promoted_quat{ rhs }) };
			}
			else
			{
				MUU_FMA_BLOCK;

				return { lhs.s * rhs.s - vector_type::dot(lhs.v, rhs.v),
						 lhs.s * rhs.v + rhs.s * lhs.v + vector_type::cross(lhs.v, rhs.v) };
			}
		}

		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL rotate_vector(MUU_VC_PARAM(quaternion) lhs,
																  MUU_VC_PARAM(vector_type) rhs) noexcept
		{
			if constexpr (is_small_float)
			{
				return vector_type{ promoted_quat::rotate_vector(promoted_quat{ lhs }, promoted_vec{ rhs }) };
			}
			else
			{
				MUU_FMA_BLOCK;

				auto t		 = scalar_type{ 2 } * vector_type::cross(lhs.v, rhs);
				const auto u = vector_type::cross(lhs.v, t);
				t *= lhs.s;
				return { rhs.x + t.x + u.x, rhs.y + t.y + u.y, rhs.z + t.z + u.z };
			}
		}

		/// \endcond

	  public:
		/// \brief Multiplies two quaternions.
		MUU_PURE_GETTER
		friend constexpr quaternion MUU_VECTORCALL operator*(MUU_VC_PARAM(quaternion) lhs,
															 MUU_VC_PARAM(quaternion) rhs) noexcept
		{
			return multiply(lhs, rhs);
		}

		/// \brief Multiplies this quaternion with another.
		constexpr quaternion& MUU_VECTORCALL operator*=(MUU_VC_PARAM(quaternion) rhs) noexcept
		{
			return *this = multiply(*this, rhs);
		}

		/// \brief Rotates a three-dimensional vector by the rotation encoded in a quaternion.
		MUU_NODISCARD
		friend constexpr vector_type MUU_VECTORCALL operator*(MUU_VC_PARAM(quaternion) lhs,
															  MUU_VC_PARAM(vector_type) rhs) noexcept
		{
			return rotate_vector(lhs, rhs);
		}

		/// \brief Rotates a three-dimensional vector by the rotation encoded in a quaternion.
		MUU_NODISCARD
		friend constexpr vector_type MUU_VECTORCALL operator*(MUU_VC_PARAM(vector_type) lhs,
															  MUU_VC_PARAM(quaternion) rhs) noexcept
		{
			return rotate_vector(rhs, lhs);
		}

		/// \brief Scales the shortest-path rotation equivalent of a quaternion by a scalar.
		MUU_PURE_GETTER
		friend constexpr quaternion MUU_VECTORCALL operator*(MUU_VC_PARAM(quaternion) lhs, scalar_type rhs) noexcept
		{
			if constexpr (is_small_float)
			{
				return quaternion{ promoted_quat{ lhs } * static_cast<promoted_scalar>(rhs) };
			}
			else
			{
				MUU_FMA_BLOCK;

				auto aa = to_axis_angle(lhs);
				aa.angle *= rhs * scalar_constants::one_over_two;
				aa.axis.normalize(); // todo: unnecessary?
				return { muu::cos(aa.angle), muu::sin(aa.angle) * aa.axis };
			}
		}

		/// \brief Scales the shortest-path rotation equivalent of a quaternion by a scalar.
		MUU_NODISCARD
		friend constexpr quaternion MUU_VECTORCALL operator*(scalar_type lhs, MUU_VC_PARAM(quaternion) rhs) noexcept
		{
			return rhs * lhs;
		}

		/// \brief Scales the shortest-path rotation equivalent of this quaternion by a scalar (in-place).
		constexpr quaternion& MUU_VECTORCALL operator*=(scalar_type rhs) noexcept
		{
			return *this = (*this * rhs);
		}

		/// @}
#endif // multiplication

#if 1 // misc ----------------------------------------------------------------------------------------------------------

		/// \brief Writes a quaternion out to a text stream.
		template <typename Char, typename Traits>
		friend std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os, const quaternion& q)
		{
			impl::print_compound_vector(os, &q.s, 1_sz, false, &q.v.x, 3_sz, true);
			return os;
		}

		/// \brief	Performs a spherical-linear interpolation between two quaternion orientations.
		///
		/// \param	start	The start orientation.
		/// \param	finish	The finish orientation.
		/// \param	alpha 	The blend factor.
		///
		/// \returns	A quaternion orientation derived from a spherical-linear interpolation between `start` and
		/// `finish`.
		///
		/// \see [Slerp](https://en.wikipedia.org/wiki/Slerp)
		MUU_PURE_GETTER
		static constexpr quaternion MUU_VECTORCALL slerp(MUU_VC_PARAM(quaternion) start,
														 MUU_VC_PARAM(quaternion) finish,
														 scalar_type alpha) noexcept
		{
			if constexpr (is_small_float)
			{
				return quaternion{ promoted_quat::slerp(promoted_quat{ start },
														promoted_quat{ finish },
														static_cast<promoted_scalar>(alpha)) };
			}
			else
			{
				MUU_FMA_BLOCK;

				auto dot = quaternion::dot(start, finish);

				// map from { s, v } and { -s, -v } (they represent the same rotation)
				auto correction = scalar_type{ 1 };
				if (dot < scalar_type{})
				{
					correction = scalar_type{ -1 };
					dot		   = -dot;
				}

				// they're extremely close, do a normal lerp
				if (dot >= static_cast<scalar_type>(0.9995))
				{
					const auto inv_alpha = scalar_type{ 1 } - alpha;

					return normalize(quaternion{ start.s * inv_alpha + finish.s * alpha * correction,
												 start.v * inv_alpha + finish.v * alpha * correction });
				}

				const auto theta_0		 = muu::acos(dot);
				const auto theta		 = theta_0 * alpha;
				const auto sin_theta_div = muu::sin(theta) / muu::sin(theta_0);
				const auto s0			 = muu::cos(theta) - dot * sin_theta_div;
				const auto s1			 = sin_theta_div;
				return { start.s * s0 + finish.s * s1 * correction, start.v * s0 + finish.v * s1 * correction };
			}
		}

		/// \brief	Performs a spherical-linear interpolation on this quaternion (in-place).
		///
		/// \param	target	The 'target' orientation.
		/// \param	alpha 	The blend factor.
		///
		/// \return	A reference to the quaternion.
		///
		/// \see [Slerp](https://en.wikipedia.org/wiki/Slerp)
		constexpr quaternion& MUU_VECTORCALL slerp(MUU_VC_PARAM(quaternion) target, scalar_type alpha) noexcept
		{
			return *this = slerp(*this, target, alpha);
		}

#endif // misc
	};

	/// \cond

	template <typename S, typename X, typename Y, typename Z>
	quaternion(S, X, Y, Z) -> quaternion<impl::std_math_common_type<impl::highest_ranked<S, X, Y, Z>>>;

	template <typename S>
	quaternion(S, S, S, S) -> quaternion<impl::std_math_common_type<S>>;

	template <typename R, typename I>
	quaternion(R, vector<I, 3>) -> quaternion<impl::std_math_common_type<impl::highest_ranked<R, I>>>;

	template <typename S>
	quaternion(const axis_angle<S>&) -> quaternion<S>;

	template <typename S>
	quaternion(const euler_angles<S>&) -> quaternion<S>;

	/// \endcond
}

namespace std
{
	/// \brief Specialization of std::tuple_size for muu::quaternion.
	template <typename Scalar>
	struct tuple_size<muu::quaternion<Scalar>>
	{
		static constexpr size_t value = 2;
	};

	/// \brief Specialization of std::tuple_element for muu::quaternion.
	template <size_t I, typename Scalar>
	struct tuple_element<I, muu::quaternion<Scalar>>
	{
		static_assert(I < 2);
		using type = std::conditional_t<I == 1, muu::vector<Scalar, 3>, Scalar>;
	};
}

//======================================================================================================================
// CONSTANTS
//======================================================================================================================

namespace muu
{
	/// \ingroup	constants
	/// \see		muu::quaternion
	///
	/// \brief		Quaternion constants.
	template <typename Scalar>
	struct constants<quaternion<Scalar>>
	{
		using scalars = constants<Scalar>;
		using vectors = constants<vector<Scalar, 3>>;

		/// \brief A quaternion with all members initialized to zero.
		static constexpr quaternion<Scalar> zero{ scalars::zero, vectors::zero };

		/// \brief The identity quaternion.
		static constexpr quaternion<Scalar> identity{ scalars::one, vectors::zero };
	};
}

//======================================================================================================================
// FREE FUNCTIONS
//======================================================================================================================

namespace muu
{
	/// \ingroup	infinity_or_nan
	/// \relatesalso	muu::quaternion
	///
	/// \brief	Returns true if any of the scalar components of a quaternion are infinity or NaN.
	template <typename S>
	MUU_PURE_GETTER
	constexpr bool infinity_or_nan(const quaternion<S>& q) noexcept
	{
		return quaternion<S>::infinity_or_nan(q);
	}

	/// \ingroup	approx_equal
	/// \relatesalso	muu::quaternion
	///
	/// \brief		Returns true if two quaternions are approximately equal.
	template <typename S, typename T>
	MUU_PURE_GETTER
	constexpr bool MUU_VECTORCALL approx_equal(const quaternion<S>& q1,
											   const quaternion<T>& q2,
											   epsilon_type<S, T> epsilon = default_epsilon<S, T>) noexcept
	{
		return quaternion<S>::approx_equal(q1, q2, epsilon);
	}

	/// \ingroup	approx_zero
	/// \relatesalso	muu::quaternion
	///
	/// \brief		Returns true if all the scalar components of a quaternion are approximately equal to zero.
	template <typename S>
	MUU_PURE_GETTER
	constexpr bool MUU_VECTORCALL approx_zero(const quaternion<S>& q, S epsilon = default_epsilon<S>) noexcept
	{
		return quaternion<S>::approx_zero(q, epsilon);
	}

	/// \ingroup	normalized
	/// \relatesalso	muu::quaternion
	///
	/// \brief Returns true if a quaternion is normalized (i.e. has a length of 1).
	template <typename S>
	MUU_PURE_GETTER
	constexpr bool normalized(const quaternion<S>& q) noexcept
	{
		return quaternion<S>::normalized(q);
	}

	/// \relatesalso muu::quaternion
	///
	/// \brief	Returns the dot product of two quaternions.
	template <typename S>
	MUU_PURE_GETTER
	constexpr S dot(const quaternion<S>& q1, const quaternion<S>& q2) noexcept
	{
		return quaternion<S>::dot(q1, q2);
	}

	/// \relatesalso muu::quaternion
	///
	/// \brief	Normalizes a quaternion.
	///
	/// \param q	The quaternion to normalize.
	///
	/// \return		A normalized copy of the input quaternion.
	template <typename S>
	MUU_PURE_GETTER
	constexpr quaternion<S> normalize(const quaternion<S>& q) noexcept
	{
		return quaternion<S>::normalize(q);
	}

	/// \relatesalso	muu::quaternion
	///
	/// \brief	Performs a spherical-linear interpolation between two quaternions.
	template <typename S>
	MUU_PURE_GETTER
	constexpr quaternion<S> MUU_VECTORCALL slerp(const quaternion<S>& start,
												 const quaternion<S>& finish,
												 S alpha) noexcept
	{
		return quaternion<S>::slerp(start, finish, alpha);
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"

/// \cond
#include "impl/quaternion_x_matrix.h"
/// \endcond
