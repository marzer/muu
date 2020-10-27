// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::quaternion.

#pragma once
#include "../muu/vector.h"

MUU_DISABLE_WARNINGS
#include <iosfwd>
MUU_ENABLE_WARNINGS

MUU_PUSH_WARNINGS
MUU_DISABLE_SHADOW_WARNINGS
MUU_PRAGMA_GCC(diagnostic ignored "-Wsign-conversion")
MUU_PRAGMA_CLANG(diagnostic ignored "-Wdouble-promotion")

MUU_PRAGMA_MSVC(inline_recursion(on))
MUU_PRAGMA_MSVC(float_control(push))
MUU_PRAGMA_MSVC(float_control(except, off))
MUU_PRAGMA_MSVC(float_control(precise, off))
MUU_PRAGMA_MSVC(push_macro("min"))
MUU_PRAGMA_MSVC(push_macro("max"))
#if MUU_MSVC
	#undef min
	#undef max
#endif

//=====================================================================================================================
// IMPLEMENTATION DETAILS
#if 1
#ifndef DOXYGEN

MUU_IMPL_NAMESPACE_START
{
	template <typename Scalar>
	struct MUU_TRIVIAL_ABI quaternion_base
	{
		Scalar s;
		vector<Scalar, 3> v;
	};

	#if MUU_HAS_VECTORCALL

	template <typename Scalar>
	inline constexpr bool is_hva<quaternion_base<Scalar>> =
		(is_same_as_any<Scalar, float, double, long double> || is_simd_intrinsic<Scalar>)
		&& sizeof(quaternion_base<Scalar>) == sizeof(Scalar) * 4
		&& alignof(quaternion_base<Scalar>) == alignof(Scalar)
	;

	template <typename Scalar>
	inline constexpr bool is_hva<quaternion<Scalar>> = is_hva<quaternion_base<Scalar>>;

	#endif // MUU_HAS_VECTORCALL

	template <typename Scalar>
	inline constexpr bool pass_quaternion_by_reference = pass_object_by_reference<quaternion_base<Scalar>>;

	template <typename Scalar>
	inline constexpr bool pass_quaternion_by_value = !pass_quaternion_by_reference<Scalar>;

	MUU_API void print_quaternion_to_stream(std::ostream& stream, const half*);
	MUU_API void print_quaternion_to_stream(std::ostream& stream, const float*);
	MUU_API void print_quaternion_to_stream(std::ostream& stream, const double*);
	MUU_API void print_quaternion_to_stream(std::ostream& stream, const long double*);
	#if MUU_HAS_FLOAT16
	MUU_API void print_quaternion_to_stream(std::ostream& stream, const _Float16*);
	#endif
	#if MUU_HAS_FP16
	MUU_API void print_quaternion_to_stream(std::ostream& stream, const __fp16*);
	#endif
	#if MUU_HAS_FLOAT128
	MUU_API void print_quaternion_to_stream(std::ostream& stream, const float128_t*);
	#endif

	MUU_API void print_quaternion_to_stream(std::wostream& stream, const half*);
	MUU_API void print_quaternion_to_stream(std::wostream& stream, const float*);
	MUU_API void print_quaternion_to_stream(std::wostream& stream, const double*);
	MUU_API void print_quaternion_to_stream(std::wostream& stream, const long double*);
	#if MUU_HAS_FLOAT16
	MUU_API void print_quaternion_to_stream(std::wostream& stream, const _Float16*);
	#endif
	#if MUU_HAS_FP16
	MUU_API void print_quaternion_to_stream(std::wostream& stream, const __fp16*);
	#endif
	#if MUU_HAS_FLOAT128
	MUU_API void print_quaternion_to_stream(std::wostream& stream, const float128_t*);
	#endif

	template <typename T>
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr T MUU_VECTORCALL normalize_angle(T val) noexcept
	{
		if (val < T{} || val > constants<T>::two_pi)
			val -= constants<T>::two_pi * floor_(val * constants<T>::one_over_two_pi);
		return val;
	}

	template <typename T>
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr T MUU_VECTORCALL normalize_angle_signed(T val) noexcept
	{
		if (val < -constants<T>::pi || val > constants<T>::pi)
		{
			val += constants<T>::pi;
			val -= constants<T>::two_pi * floor_(val * constants<T>::one_over_two_pi);
			val -= constants<T>::pi;
		}
		return val;
	}
}
MUU_IMPL_NAMESPACE_END

#else // ^^^ !DOXYGEN / DOXYGEN vvv

#define ENABLE_PAIRED_FUNC_BY_REF(...)
#define ENABLE_PAIRED_FUNC_BY_VAL(...)
#define ENABLE_PAIRED_FUNCS 0

#endif // DOXYGEN

#if !defined(DOXYGEN) && !MUU_INTELLISENSE

	#define ENABLE_PAIRED_FUNCS 1

	#define ENABLE_PAIRED_FUNC_BY_REF(S, ...) \
		MUU_SFINAE(impl::pass_quaternion_by_reference<S> && (__VA_ARGS__))

	#define ENABLE_PAIRED_FUNC_BY_VAL(S, ...) \
		MUU_SFINAE_2(impl::pass_quaternion_by_value<S> && (__VA_ARGS__))

#else 
	#define ENABLE_PAIRED_FUNCS 0
	#define ENABLE_PAIRED_FUNC_BY_REF(...)
	#define ENABLE_PAIRED_FUNC_BY_VAL(...)
#endif

#endif // =============================================================================================================

//=====================================================================================================================
// HELPER CLASSES
#if 1
MUU_NAMESPACE_START
{
	/// \brief	An axis+angle rotation.
	/// 
	/// \tparam	Scalar	The scalar component type of the data members.
	/// 
	/// \see		muu::quaternion
	template <typename Scalar>
	struct MUU_TRIVIAL_ABI axis_angle_rotation
	{
		static_assert(
			!std::is_reference_v<Scalar>,
			"Axis-angle rotation scalar type cannot be a reference"
		);
		static_assert(
			!std::is_const_v<Scalar> && !std::is_volatile_v<Scalar>,
			"Axis-angle rotation scalar type cannot be const- or volatile-qualified"
		);

		/// \brief The scalar type of the axis and angle.
		using scalar_type = Scalar;

		/// \brief	The axis being rotated around.
		vector<scalar_type, 3> axis;

		/// \brief	The angle of rotation (in radians).
		scalar_type angle;
	};
	#ifndef DOXYGEN

	template <typename S MUU_SFINAE(is_arithmetic<S>)>
	axis_angle_rotation(S, S, S, S) -> axis_angle_rotation<impl::std_math_common_type<S>>;

	template <typename X, typename Y, typename Z, typename Angle MUU_SFINAE(all_arithmetic<X, Y, Z, Angle>)>
	axis_angle_rotation(X, Y, Z, Angle) -> axis_angle_rotation<impl::std_math_common_type<impl::highest_ranked<X, Y, Z, Angle>>>;

	template <typename Axis, typename Angle MUU_SFINAE(all_arithmetic<Axis, Angle>)>
	axis_angle_rotation(vector<Axis, 3>, Angle) -> axis_angle_rotation<impl::std_math_common_type<impl::highest_ranked<Axis, Angle>>>;

	#if MUU_HAS_VECTORCALL
	namespace impl
	{
		template <typename Scalar>
		inline constexpr bool is_hva<axis_angle_rotation<Scalar>> =
			(is_same_as_any<Scalar, float, double, long double> || is_simd_intrinsic<Scalar>)
			&& sizeof(axis_angle_rotation<Scalar>) == sizeof(Scalar) * 4
			&& alignof(axis_angle_rotation<Scalar>) == alignof(Scalar);
	}
	#endif // MUU_HAS_VECTORCALL
	#endif

	/// \brief	A set of euler angles used for rotation.
	/// 
	/// \detail This type models a specific form of Euler angles relating to the Aircraft Principal Axes,
	/// 		and observes the following conventions:
	/// 		<table>
	/// 		<tr><td><strong><em>Yaw</em></strong></td>
	/// 		<td><ul>
	/// 		<li>Applied <em>first</em>
	/// 		<li>Corresponds to a rotation around the local up axis
	/// 		<li>A positive yaw means "turn the nose of the aircraft to the right"  
	/// 		</ul></td></tr>
	/// 		<tr><td><strong><em>Pitch</em></strong></td>
	/// 		<td><ul>
	/// 		<li>Applied <em>second</em>
	/// 		<li>Corresponds to a rotation around the local right axis
	/// 		<li>A positive pitch means "point the nose of the aircraft up toward the sky".
	/// 		</ul></td></tr>
	/// 		<tr><td><strong><em>Roll</em></strong></td>
	/// 		<td><ul>
	/// 		<li>Applied <em>third</em>
	/// 		<li>Corresponds to a rotation around the local forward axis
	/// 		<li>A positive roll means "tilt the right wing of the aircraft toward the ground".
	/// 		</ul></td></tr>
	/// 		</table>
	/// 		
	/// 		The angles are always in radians.
	/// 
	/// \tparam	Scalar	The scalar component type of the data members.
	/// 
	/// \see
	/// 	 - muu::quaternion
	/// 	 - [Euler Angles](https://en.wikipedia.org/wiki/Euler_angles)
	/// 	 - [Aircraft Principal Axes](https://en.wikipedia.org/wiki/Aircraft_principal_axes)
	/// 	 - [Euler Angles (math)](https://www.euclideanspace.com/maths/geometry/rotations/euler/index.htm)
	template <typename Scalar>
	struct MUU_TRIVIAL_ABI euler_rotation
	{
		static_assert(
			!std::is_reference_v<Scalar>,
			"Euler rotation scalar type cannot be a reference"
		);
		static_assert(
			!std::is_const_v<Scalar> && !std::is_volatile_v<Scalar>,
			"Euler rotation scalar type cannot be const- or volatile-qualified"
		);

		/// \brief The scalar type of the rotation's angles.
		using scalar_type = Scalar;

		/// \brief The rotation around the local up axis (in radians).
		/// \remark A positive yaw means "turn the nose of the aircraft to the right".
		scalar_type yaw;

		/// \brief The rotation around the local right axis (in radians).
		/// \remark A positive pitch means "point the nose of the aircraft up toward the sky".
		scalar_type pitch;

		/// \brief The rotation around the local forward axis (in radians).
		/// \remark A positive roll means "tilt the aircraft so the right wing points toward the ground".
		scalar_type roll;

		/// \brief	Scales an euler rotation.
		[[nodiscard]]
		MUU_ATTR(const)
		friend constexpr euler_rotation MUU_VECTORCALL operator * (euler_rotation lhs, scalar_type rhs) noexcept
		{
			return euler_rotation{ lhs.yaw * rhs, lhs.pitch * rhs, lhs.roll * rhs };
		}

		/// \brief	Scales an euler rotation.
		[[nodiscard]]
		MUU_ATTR(const)
		friend constexpr euler_rotation MUU_VECTORCALL operator * (scalar_type lhs, euler_rotation rhs) noexcept
		{
			return rhs * lhs;
		}

		/// \brief	Scales an euler rotation.
		constexpr euler_rotation& MUU_VECTORCALL operator *= (scalar_type rhs) noexcept
		{
			return *this = *this * rhs;
		}

		/// \brief Writes a euler rotation out to a text stream.
		template <typename Char, typename Traits>
		friend std::basic_ostream<Char, Traits>& operator << (std::basic_ostream<Char, Traits>& os, const euler_rotation& rot)
		{
			static_assert(sizeof(euler_rotation) == sizeof(scalar_type) * 3);
			impl::print_vector_to_stream(os, &rot.yaw, 3u);
			return os;
		}
	};
	#ifndef DOXYGEN

	template <typename S MUU_SFINAE(is_arithmetic<S>)>
	euler_rotation(S, S, S) -> euler_rotation<impl::std_math_common_type<S>>;

	template <typename Yaw, typename Pitch, typename Roll MUU_SFINAE(all_arithmetic<Yaw, Pitch, Roll>)>
	euler_rotation(Yaw, Pitch, Roll) -> euler_rotation<impl::std_math_common_type<impl::highest_ranked<Yaw, Pitch, Roll>>>;

	#if MUU_HAS_VECTORCALL
	namespace impl
	{
		template <typename Scalar>
		inline constexpr bool is_hva<euler_rotation<Scalar>> =
			(is_same_as_any<Scalar, float, double, long double> || is_simd_intrinsic<Scalar>)
			&& sizeof(euler_rotation<Scalar>) == sizeof(Scalar) * 3
			&& alignof(euler_rotation<Scalar>) == alignof(Scalar);
	}
	#endif // MUU_HAS_VECTORCALL
	#endif
}
MUU_NAMESPACE_END
#endif // =============================================================================================================

//=====================================================================================================================
// QUATERNION CLASS
#if 1

MUU_NAMESPACE_START
{
	/// \brief A quaternion.
	///
	/// \tparam	Scalar      The type of the quaternion's scalar components. Must be a floating-point type.
	/// 
	/// \see
	/// 	 - muu::vector
	/// 	 - muu::axis_angle_rotation
	/// 	 - muu::euler_rotation
	template <typename Scalar>
	struct MUU_TRIVIAL_ABI quaternion
		#ifndef DOXYGEN
		: impl::quaternion_base<Scalar>
		#endif
	{
		static_assert(
			!std::is_reference_v<Scalar>,
			"Quaternion scalar type cannot be a reference"
		);
		static_assert(
			!std::is_const_v<Scalar> && !std::is_volatile_v<Scalar>,
			"Quaternion scalar type cannot be const- or volatile-qualified"
		);
		static_assert(
			std::is_trivially_constructible_v<Scalar>
			&& std::is_trivially_copyable_v<Scalar>
			&& std::is_trivially_destructible_v<Scalar>,
			"Quaternion scalar type must be trivially constructible, copyable and destructible"
		);
		static_assert(
			is_floating_point<Scalar>,
			"Quaternion scalar type must be a floating-point type"
		);

		/// \brief The type of each scalar component stored in this quaternion.
		using scalar_type = Scalar;

		/// \brief Compile-time constants for this quaternion's #scalar_type.
		using scalar_constants = muu::constants<scalar_type>;

		/// \brief The three-dimensional #muu::vector with the same #scalar_type as this quaternion.
		using vector_type = vector<scalar_type, 3>;

		/// \brief `vector_type` or `const vector_type&`, depending on size, triviality, simd-friendliness, etc.
		using vector_param = typename vector_type::vector_param;

		/// \brief Compile-time constants for this quaternion's #vector_type.
		using vector_constants = muu::constants<vector_type>;

		/// \brief The #muu::axis_angle_rotation with the same #scalar_type as this quaternion.
		using axis_angle_type = axis_angle_rotation<scalar_type>;

		/// \brief `axis_angle_type` or `const axis_angle_type&`, depending on size, triviality, simd-friendliness, etc.
		using axis_angle_param = impl::maybe_pass_readonly_by_value<axis_angle_type>;

		/// \brief The #muu::euler_rotation with the same #scalar_type as this quaternion.
		using euler_type = euler_rotation<scalar_type>;

		/// \brief `euler_type` or `const euler_type&`, depending on size, triviality, simd-friendliness, etc.
		using euler_param = impl::maybe_pass_readonly_by_value<euler_type>;

		/// \brief `quaternion` or `const quaternion&`, depending on size, triviality, simd-friendliness, etc.
		using quaternion_param = std::conditional_t<
			impl::pass_quaternion_by_value<scalar_type>,
			quaternion,
			const quaternion&
		>;

		/// \brief Compile-time constants for this quaternion.
		using constants = muu::constants<quaternion>;

	private:

		using base = impl::quaternion_base<Scalar>;
		static_assert(
			sizeof(base) == (sizeof(scalar_type) * 4),
			"Quaternions should not have padding"
		);
		using intermediate_type = impl::promote_if_small_float<scalar_type>;

	public:

		#ifdef DOXYGEN
		/// \brief The quaternion's scalar (real) part.
		scalar_type s;
		/// \brief The quaternion's vector (imaginary) part.
		vector_type v;
		#endif

	#if 1 // constructors ---------------------------------------------------------------------------------------------
	
		/// \brief Default constructor. Values are not initialized.
		quaternion() noexcept = default;

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr quaternion(const quaternion&) noexcept = default;

		/// \brief Copy-assigment operator.
		constexpr quaternion& operator = (const quaternion&) noexcept = default;

		/// \brief	Constructs a quaternion from raw scalar values.
		///
		/// \param	s	Initial value for the scalar (real) part.
		/// \param	vx	Initial value for the vector (imaginary) part's X component.
		/// \param	vy	Initial value for the vector (imaginary) part's Y component.
		/// \param	vz	Initial value for the vector (imaginary) part's Z component.
		MUU_NODISCARD_CTOR
		constexpr quaternion(scalar_type s, scalar_type vx, scalar_type vy, scalar_type vz) noexcept
			: base{ s, vector_type{ vx, vy, vz }  }
		{}

		/// \brief	Constructs a quaternion from a scalar and a vector.
		///
		/// \param	s	Initial value for the scalar (real) part.
		/// \param	v	Initial value for the vector (imaginary) part.
		MUU_NODISCARD_CTOR
		constexpr quaternion(scalar_type s, const vector_type& v) noexcept
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
		explicit constexpr quaternion(const axis_angle_type& aa) noexcept
			: quaternion{ from_axis_angle(aa) }
		{}

		/// \brief	Constructs a quaternion from a set of euler angles.
		MUU_NODISCARD_CTOR
			explicit constexpr quaternion(const euler_type& euler) noexcept
			: quaternion{ from_euler(euler) }
		{}

	#endif // constructors

	#if 1 // value accessors ------------------------------------------------------------------------------------------

	private:

		template <size_t Index, typename T>
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(pure)
		static constexpr auto& do_get(T& quat) noexcept
		{
			static_assert(
				Index < 2,
				"Element index out of range"
			);

			if constexpr (Index == 0) return quat.s;
			if constexpr (Index == 1) return quat.v;
		}

	public:

		/// \brief Gets a reference to the value at a specific index.
		/// 
		/// \tparam Index  The index of the value to retrieve, where r == 0, i == 1.
		///
		/// \return  A reference to the selected value.
		template <size_t Index>
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(pure)
		MUU_ATTR(flatten)
		constexpr const auto& get() const noexcept
		{
			return do_get<Index>(*this);
		}

		/// \brief Gets a reference to the value at a specific index.
		/// 
		/// \tparam Index  The index of the value to retrieve, where r == 0, i == 1.
		///
		/// \return  A reference to the selected value.
		template <size_t Index>
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(pure)
		MUU_ATTR(flatten)
		constexpr auto& get() noexcept
		{
			return do_get<Index>(*this);
		}

	#endif // value accessors

	#if 1 // equality -------------------------------------------------------------------------------------------------

		/// \brief		Returns true if two quaternions are exactly equal.
		/// 
		/// \remarks	This is a componentwise exact equality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point quaternions, use #approx_equal().
		template <typename T ENABLE_PAIRED_FUNC_BY_REF(T, true)>
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr bool MUU_VECTORCALL operator == (quaternion_param lhs, const quaternion<T>& rhs) noexcept
		{
			if constexpr (std::is_same_v<scalar_type, T>)
			{
				return lhs.s == rhs.s
					&& lhs.v == rhs.v;
			}
			else
			{
				using type = impl::equality_check_type<scalar_type, T>;

				if (static_cast<type>(lhs.s) != static_cast<type>(rhs.s))
					return false;
				return lhs.v == rhs.v;
			}
		}

		/// \brief	Returns true if two quaternions are not exactly equal.
		/// 
		/// \remarks	This is a componentwise exact inequality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point quaternions, use #approx_equal().
		template <typename T ENABLE_PAIRED_FUNC_BY_REF(T, true)>
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr bool MUU_VECTORCALL operator != (quaternion_param lhs, const quaternion<T>& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		#if ENABLE_PAIRED_FUNCS

		template <typename T ENABLE_PAIRED_FUNC_BY_VAL(T, true)>
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr bool MUU_VECTORCALL operator == (quaternion_param lhs, quaternion<T> rhs) noexcept
		{
			if constexpr (std::is_same_v<scalar_type, T>)
			{
				return lhs.s == rhs.s
					&& lhs.v == rhs.v;
			}
			else
			{
				using type = impl::equality_check_type<scalar_type, T>;

				if (static_cast<type>(lhs.s) != static_cast<type>(rhs.s))
					return false;
				return lhs.v == rhs.v;
			}
		}

		template <typename T ENABLE_PAIRED_FUNC_BY_VAL(T, true)>
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr bool MUU_VECTORCALL operator != (quaternion_param lhs, quaternion<T> rhs) noexcept
		{
			return !(lhs == rhs);
		}

		#endif // ENABLE_PAIRED_FUNCS

		/// \brief	Returns true if all the scalar components of a quaternion are exactly zero.
		/// 
		/// \remarks	This is a componentwise exact equality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point quaternions, use #approx_zero().
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool MUU_VECTORCALL zero(quaternion_param q) noexcept
		{
			return q.s == scalar_constants::zero
				&& vector_type::zero(q.v);
		}

		/// \brief	Returns true if all the scalar components of the quaternion are exactly zero.
		/// 
		/// \remarks	This is a componentwise exact equality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point quaternions, use #approx_zero().
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool zero() const noexcept
		{
			return zero(*this);
		}

		/// \brief	Returns true if any of the scalar components of a quaternion are infinity or NaN.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool MUU_VECTORCALL infinity_or_nan(quaternion_param q) noexcept
		{
			return muu::infinity_or_nan(q.s)
				|| vector_type::infinity_or_nan(q.v);
		}

		/// \brief	Returns true if any of the scalar components of the quaternion are infinity or NaN.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool infinity_or_nan() const noexcept
		{
			return infinity_or_nan(*this);
		}

		/// \brief Returns true if a quaternion is unit-length (i.e. has a length of 1).
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool MUU_VECTORCALL unit_length(quaternion_param q) noexcept
		{
			constexpr auto epsilon = intermediate_type{ 1 } / (
				100ull
				* (sizeof(scalar_type) >= sizeof(float) ? 10000ull : 1ull)
				* (sizeof(scalar_type) >= sizeof(double) ? 10000ull : 1ull)
			);

			return muu::approx_equal(
				raw_dot(q, q),
				intermediate_type{ 1 },
				epsilon
			);
		}

		/// \brief Returns true if the quaternion is unit-length (i.e. has a length of 1).
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool unit_length() const noexcept
		{
			return unit_length(*this);
		}

	#endif // equality

	#if 1 // approx_equal ---------------------------------------------------------------------------------------------

		/// \brief	Returns true if two quaternions are approximately equal.
		template <typename T, typename Epsilon = impl::highest_ranked<scalar_type, T> ENABLE_PAIRED_FUNC_BY_REF(T, true)>
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool MUU_VECTORCALL approx_equal(
			quaternion_param q1,
			const quaternion<T>& q2,
			dont_deduce<Epsilon> epsilon = muu::constants<Epsilon>::approx_equal_epsilon
		) noexcept
		{
			if constexpr (std::is_same_v<scalar_type, T>)
			{
				return muu::approx_equal(q1.s, q2.s, epsilon)
					&& vector_type::approx_equal(q1.v, q2.v, epsilon);
			}
			else
			{
				using type = impl::equality_check_type<scalar_type, T>;

				return muu::approx_equal(static_cast<type>(q1.s), static_cast<type>(q2.s), static_cast<type>(epsilon))
					&& vector_type::approx_equal(q1.v, q2.v, epsilon);
			}
		}

		/// \brief	Returns true if the quaternion is approximately equal to another.
		template <typename T, typename Epsilon = impl::highest_ranked<scalar_type, T> ENABLE_PAIRED_FUNC_BY_REF(T, true)>
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool MUU_VECTORCALL approx_equal(
			const quaternion<T>& q,
			dont_deduce<Epsilon> epsilon = muu::constants<Epsilon>::approx_equal_epsilon
		) const noexcept
		{
			return approx_equal(*this, q, epsilon);
		}

		#if ENABLE_PAIRED_FUNCS

		template <typename T, typename Epsilon = impl::highest_ranked<scalar_type, T> ENABLE_PAIRED_FUNC_BY_VAL(T, true)>
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool MUU_VECTORCALL approx_equal(
			quaternion_param q1,
			quaternion<T> q2,
			dont_deduce<Epsilon> epsilon = muu::constants<Epsilon>::approx_equal_epsilon
		) noexcept
		{
			if constexpr (std::is_same_v<scalar_type, T>)
			{
				return muu::approx_equal(q1.s, q2.s, epsilon)
					&& vector_type::approx_equal(q1.v, q2.v, epsilon);
			}
			else
			{
				using type = impl::equality_check_type<scalar_type, T>;

				return muu::approx_equal(static_cast<type>(q1.s), static_cast<type>(q2.s), static_cast<type>(epsilon))
					&& vector_type::approx_equal(q1.v, q2.v, epsilon);
			}
		}

		template <typename T, typename Epsilon = impl::highest_ranked<scalar_type, T> ENABLE_PAIRED_FUNC_BY_VAL(T, true)>
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool MUU_VECTORCALL approx_equal(
			quaternion<T> q,
			dont_deduce<Epsilon> epsilon = muu::constants<Epsilon>::approx_equal_epsilon
		) const noexcept
		{
			return approx_equal(*this, q, epsilon);
		}

		#endif // ENABLE_PAIRED_FUNCS

		/// \brief	Returns true if all the scalar components in a quaternion are approximately equal to zero.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool MUU_VECTORCALL approx_zero(
			quaternion_param q,
			scalar_type epsilon = muu::constants<scalar_type>::approx_equal_epsilon
		) noexcept
		{
			return muu::approx_zero(q.s, epsilon) && vector_type::approx_zero(q.v, epsilon);
		}

		/// \brief	Returns true if all the scalar components in the quaternion are approximately equal to zero.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool MUU_VECTORCALL approx_zero(
			scalar_type epsilon = muu::constants<scalar_type>::approx_equal_epsilon
		) const noexcept
		{
			return approx_zero(*this, epsilon);
		}

	#endif // approx_equal

	#if 1 // dot product ----------------------------------------------------------------------------------------------

	private:

		template <typename T = intermediate_type>
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr T MUU_VECTORCALL raw_dot(quaternion_param q1, quaternion_param q2) noexcept
		{
			MUU_FMA_BLOCK

			static_assert(std::is_same_v<impl::highest_ranked<T, intermediate_type>, T>); // non-truncating

			return static_cast<T>(q1.s) * static_cast<T>(q2.s)
				+ vector_type::template raw_dot<T>(q1.v, q2.v);
		}

	public:

		/// \brief	Returns the dot product of two quaternions.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr scalar_type MUU_VECTORCALL dot(quaternion_param q1, quaternion_param q2) noexcept
		{
			return static_cast<scalar_type>(raw_dot(q1, q2));
		}

		/// \brief	Returns the dot product of this and another quaternion.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr scalar_type MUU_VECTORCALL dot(quaternion_param q) const noexcept
		{
			return dot(*this, q);
		}

	#endif // dot product

	#if 1 // normalization --------------------------------------------------------------------------------------------

		/// \brief		Normalizes a quaternion.
		///
		/// \param q	The quaternion to normalize.
		/// 
		/// \return		A normalized copy of the input quaternion.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr quaternion MUU_VECTORCALL normalize(quaternion_param q) noexcept
		{
			const intermediate_type inv_length = intermediate_type{ 1 } / muu::sqrt(raw_dot(q, q));
			return quaternion{
				static_cast<scalar_type>(q.s * inv_length),
				vector_type::raw_multiply_scalar(q.v, inv_length)
			};
		}

		/// \brief		Normalizes the quaternion (in-place).
		///
		/// \return		A reference to the quaternion.
		constexpr quaternion& normalize() noexcept
		{
			return *this = normalize(*this);
		}

	#endif // normalization
	
	#if 1 // conjgate ------------------------------------------------------------------------------------------------

		/// \brief		Returns the conjugate of a quaternion.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr quaternion MUU_VECTORCALL conjugate(quaternion_param q) noexcept
		{
			return { q.s, -q.v };
		}

		/// \brief		Returns the conjugate of the quaternion.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr quaternion conjugate() const noexcept
		{
			return { base::s, -base::v };
		}

	#endif // conjugate

	#if 1 // axis-angle conversions -----------------------------------------------------------------------------------

		/// \brief Creates a quaternion from an axis-angle rotation.
		/// 
		/// \param axis		Axis to rotate around. Must be unit-length.
		/// \param angle	Angle to rotate by (in radians).
		/// 
		/// \return A quaternion encoding the given axis-angle rotation.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr quaternion MUU_VECTORCALL from_axis_angle(vector_param axis, scalar_type angle) noexcept
		{
			MUU_CONSTEXPR_SAFE_ASSERT(
				vector_type::unit_length(axis)
				&& "from_axis_angle() expects axis inputs to be unit-length"
			);

			if constexpr (impl::is_small_float<scalar_type>)
			{
				const auto angle_ = static_cast<intermediate_type>(angle) * muu::constants<intermediate_type>::one_over_two;
				return quaternion
				{
					static_cast<intermediate_type>(muu::cos(angle_)),	//scalar
					vector_type{ muu::sin(angle_) * axis }				//vector
				};
			}
			else
			{
				angle *= scalar_constants::one_over_two;
				return quaternion
				{
					muu::cos(angle),		//scalar
					muu::sin(angle) * axis	//vector
				};
			}
		}

		/// \brief Creates a quaternion from an axis-angle rotation.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr quaternion MUU_VECTORCALL from_axis_angle(axis_angle_param aa) noexcept
		{
			return from_axis_angle(aa.axis, aa.angle);
		}

	private:

		template <typename T = intermediate_type>
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr axis_angle_rotation<T> MUU_VECTORCALL raw_to_axis_angle(quaternion_param q, bool shortest_path = true) noexcept
		{
			MUU_CONSTEXPR_SAFE_ASSERT(
				unit_length(q)
				&& "to_axis_angle() expects a normalized quaternion"
			);

			using calc_type = impl::highest_ranked<intermediate_type, T>;
			const auto length = vector_type::template raw_length<calc_type>(q.v);
			const auto correction = calc_type{ shortest_path && q.s < scalar_constants::zero ? -1 : 1 };

			if MUU_UNLIKELY(length == calc_type{})
			{
				// This happens at angle = 0 and 360. All axes are correct, so any will do.
				return
				{
					muu::vector<T, 3>::constants::x_axis,
					static_cast<T>(calc_type{ 2 } * muu::atan2(length, static_cast<calc_type>(q.s) * correction)),
				};
			}
			else
			{
				return
				{
					muu::vector<T, 3>
					{
						static_cast<T>((static_cast<calc_type>(q.v.x) * correction) / length),
						static_cast<T>((static_cast<calc_type>(q.v.y) * correction) / length),
						static_cast<T>((static_cast<calc_type>(q.v.z) * correction) / length),
					},
					static_cast<T>(calc_type{ 2 } * muu::atan2(length, static_cast<calc_type>(q.s) * correction)),
				};
			}
		}

	public:

		/// \brief	Extracts an axis-angle rotation from a quaternion.
		///
		/// \param	quat			The quaternion to convert. Must be unit-length.
		/// \param	shortest_path	Should the returned values be derived from the shortest path?
		/// 						`true` is more intuitive but means that the returned values may not convert back
		/// 						to the original quaternion if used with #from_axis_angle().
		///
		/// \returns	An axis-angle rotation representing the rotation stored in the given quaternion.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr axis_angle_type MUU_VECTORCALL to_axis_angle(quaternion_param quat, bool shortest_path = true) noexcept
		{
			return raw_to_axis_angle<scalar_type>(quat, shortest_path);
		}

		/// \brief	Extracts an axis-angle rotation from this quaternion.
		///
		/// \param	shortest_path	Should the returned values be derived from the shortest path?
		/// 						`true` is more intuitive but means that the returned values may not convert back
		/// 						to the original quaternion if used with #from_axis_angle().
		///
		/// \returns	An axis-angle rotation representing the rotation stored in the quaternion.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr axis_angle_type MUU_VECTORCALL to_axis_angle(bool shortest_path = true) const noexcept
		{
			return raw_to_axis_angle<scalar_type>(*this, shortest_path);
		}

	#endif // axis-angle conversions

	#if 1 // euler conversions ----------------------------------------------------------------------------------------

		/// \brief Creates a quaternion from a set of euler angles.
		/// 
		/// \param yaw		The rotation around the local up axis (in radians).
		/// \param pitch	The rotation around the local right axis (in radians).
		/// \param roll		The rotation around the local forward axis (in radians).
		/// 
		/// \return A quaternion encoding the given axis-angle rotation.
		[[nodiscard]]
		MUU_ATTR(const)
		static constexpr quaternion MUU_VECTORCALL from_euler(scalar_type yaw, scalar_type pitch, scalar_type roll) noexcept
		{
			MUU_FMA_BLOCK
			using itype = intermediate_type;

			// ensure rotation signs correspond with the aircraft principal axes:
			//	yaw - positive turns toward the right (nose of the plane turns east)
			//	pitch - positive pitches upward (nose of the plane points up away from ground)
			//	roll - positive rolls to the right (right wing tilts down)
			//	(see https://en.wikipedia.org/wiki/Flight_dynamics_(fixed-wing_aircraft)
			// 
			yaw = -yaw; 
			roll = -roll; 

			const itype c1 = muu::cos(static_cast<itype>(pitch) * muu::constants<itype>::one_over_two);
			const itype s1 = muu::sin(static_cast<itype>(pitch) * muu::constants<itype>::one_over_two);
			const itype c2 = muu::cos(static_cast<itype>(yaw)   * muu::constants<itype>::one_over_two);
			const itype s2 = muu::sin(static_cast<itype>(yaw)   * muu::constants<itype>::one_over_two);
			const itype c3 = muu::cos(static_cast<itype>(roll)  * muu::constants<itype>::one_over_two);
			const itype s3 = muu::sin(static_cast<itype>(roll)  * muu::constants<itype>::one_over_two);

			return
			{
				static_cast<scalar_type>(c1 * c2 * c3 - s1 * s2 * s3), // scalar (w)
				static_cast<scalar_type>(s1 * c2 * c3 - c1 * s2 * s3), // vector (x)
				static_cast<scalar_type>(c1 * s2 * c3 + s1 * c2 * s3), // vector (y)
				static_cast<scalar_type>(c1 * c2 * s3 + s1 * s2 * c3)  // vector (z)
			};
		}

		/// \brief Creates a quaternion from an axis-angle rotation.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr quaternion MUU_VECTORCALL from_euler(euler_param euler) noexcept
		{
			return from_euler(euler.yaw, euler.pitch, euler.roll);
		}

		/// \brief	Extracts a set of euler angles from a quaternion.
		///
		/// \param	q	The quaternion to convert.
		///
		/// \returns	A set of euler angles representing the rotation stored in the given quaternion.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr euler_type MUU_VECTORCALL to_euler(quaternion_param q) noexcept
		{
			MUU_FMA_BLOCK
			using itype = intermediate_type;

			const itype sqw  = static_cast<itype>(q.s)   * q.s;
			const itype sqx  = static_cast<itype>(q.v.x) * q.v.x;
			const itype sqy  = static_cast<itype>(q.v.y) * q.v.y;
			const itype sqz  = static_cast<itype>(q.v.z) * q.v.z;
			const itype test = static_cast<itype>(q.v.y) * q.v.z + static_cast<itype>(q.s) * q.v.x;
			const itype correction = static_cast<itype>(sqx) + static_cast<itype>(sqy)
				+ static_cast<itype>(sqz) + static_cast<itype>(sqw);

			// https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/index.htm
			// (note that they pitch around Z, not X; pitch and roll are swapped here)
			
			constexpr itype threshold = static_cast<itype>(0.49995);

			// singularity at north pole
			if MUU_UNLIKELY(test > threshold * correction)
			{
				MUU_FMA_BLOCK
				return
				{
					static_cast<scalar_type>(itype{ -2 } * muu::atan2(q.v.z, q.s)),
					static_cast<scalar_type>(muu::constants<itype>::pi_over_two),
					scalar_constants::zero
				};
			}

			// singularity at south pole
			else if MUU_UNLIKELY(test < -threshold * correction)
			{ 
				MUU_FMA_BLOCK
				return
				{
					static_cast<scalar_type>(itype{ 2 } * muu::atan2(q.v.z, q.s)),
					static_cast<scalar_type>(-muu::constants<itype>::pi_over_two),
					scalar_constants::zero
				};
			}

			else
			{
				MUU_FMA_BLOCK
				// note the sign of yaw and roll are flipped;
				// see from_euler() for an explanation

				return
				{
					static_cast<scalar_type>(-muu::atan2(itype{ -2 } * (q.v.x * q.v.z - q.s * q.v.y), sqw - sqx - sqy + sqz)),
					static_cast<scalar_type>(muu::asin(itype{ 2 } * test / correction)),
					static_cast<scalar_type>(-muu::atan2(itype{ -2 } * (q.v.x * q.v.y - q.s * q.v.z), sqw - sqx + sqy - sqz))
				};
			}
		}

		/// \brief	Extracts a set of euler angles from this quaternion.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr euler_type MUU_VECTORCALL to_euler() const noexcept
		{
			return to_euler(*this);
		}

	#endif // euler conversions

	#if 1 // multiplication -------------------------------------------------------------------------------------------

	private:

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr quaternion MUU_VECTORCALL multiply(quaternion_param lhs, quaternion_param rhs) noexcept
		{
			using itype = intermediate_type;	

			if constexpr (all_same<scalar_type, itype>)
			{
				MUU_FMA_BLOCK
				return
				{
					lhs.s * rhs.s - vector_type::template raw_dot<itype>(lhs.v, rhs.v),
					lhs.s * rhs.v + rhs.s * lhs.v + impl::raw_cross<vector<itype, 3>>(lhs.v, rhs.v)
				};
			}
			else
			{
				MUU_FMA_BLOCK
				return
				{
					static_cast<scalar_type>(
						static_cast<itype>(lhs.s) * rhs.s - vector_type::template raw_dot<itype>(lhs.v, rhs.v)
					),

					vector_type{
						static_cast<itype>(lhs.s) * static_cast<itype>(rhs.v)
							+ static_cast<itype>(rhs.s) * static_cast<itype>(lhs.v)
							+ impl::raw_cross<vector<itype, 3>>(lhs.v, rhs.v)
					}
				};
			}
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr vector_type MUU_VECTORCALL rotate_vector(quaternion_param lhs, vector_param rhs) noexcept
		{
			MUU_FMA_BLOCK
			using itype = intermediate_type;

			auto t = itype{ 2 } * impl::raw_cross<vector<itype, 3>>(lhs.v, rhs);
			const auto u = impl::raw_cross<vector<itype, 3>>(lhs.v, t);
			t *= static_cast<itype>(lhs.s);
			return
			{
				static_cast<scalar_type>(rhs.x + t.x + u.x),
				static_cast<scalar_type>(rhs.y + t.y + u.y),
				static_cast<scalar_type>(rhs.z + t.z + u.z)
			};
		}

	public:

		/// \brief Multiplies two quaternions.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr quaternion MUU_VECTORCALL operator * (quaternion_param lhs, quaternion_param rhs) noexcept
		{
			return multiply(lhs, rhs);
		}

		/// \brief Multiplies this quaternion with another.
		constexpr quaternion& MUU_VECTORCALL operator *= (quaternion_param rhs) noexcept
		{
			return *this = multiply(*this, rhs);
		}

		/// \brief Rotates a three-dimensional vector by the rotation encoded in a quaternion.
		[[nodiscard]]
		friend constexpr vector_type MUU_VECTORCALL operator * (quaternion_param lhs, vector_param rhs) noexcept
		{
			return rotate_vector(lhs, rhs);
		}

		/// \brief Rotates a three-dimensional vector by the rotation encoded in a quaternion.
		[[nodiscard]]
		friend constexpr vector_type MUU_VECTORCALL operator * (vector_param lhs, quaternion_param rhs) noexcept
		{
			return rotate_vector(rhs, lhs);
		}

		/// \brief Scales the shortest-path rotation equivalent of a quaternion by a scalar.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr quaternion MUU_VECTORCALL operator * (quaternion_param lhs, scalar_type rhs) noexcept
		{
			MUU_FMA_BLOCK

			auto aa = raw_to_axis_angle<intermediate_type>(lhs);
			aa.angle *= rhs * muu::constants<intermediate_type>::one_over_two;
			aa.axis.normalize(); //todo: unnecessary?
			return
			{
				static_cast<scalar_type>(muu::cos(aa.angle)),
				static_cast<vector_type>(muu::sin(aa.angle) * aa.axis)
			};
		}

		/// \brief Scales the shortest-path rotation equivalent of a quaternion by a scalar.
		[[nodiscard]]
		friend constexpr quaternion MUU_VECTORCALL operator * (scalar_type lhs, quaternion_param rhs) noexcept
		{
			return rhs * lhs;
		}

		/// \brief Scales the shortest-path rotation equivalent of this quaternion by a scalar (in-place).
		constexpr quaternion& MUU_VECTORCALL operator *= (scalar_type rhs) noexcept
		{
			return *this = (*this * rhs);
		}

	#endif // multiplication

	#if 1 // streams --------------------------------------------------------------------------------------------------

		/// \brief Writes a quaternion out to a text stream.
		template <typename Char, typename Traits>
		friend std::basic_ostream<Char, Traits>& operator << (std::basic_ostream<Char, Traits>& os, const quaternion& q)
		{
			impl::print_quaternion_to_stream(os, &q.s);
			return os;
		}

	#endif // streams

	#if 1 // misc -----------------------------------------------------------------------------------------------------

		/// \brief	Performs a spherical-linear interpolation between two quaternions.
		///
		/// \param	start	The value at the start of the interpolation range.
		/// \param	finish	The value at the end of the interpolation range.
		/// \param	alpha 	The blend factor.
		///
		/// \returns	A quaternion with values derived from a spherical-linear interpolation from `start` to `finish`.
		/// 
		/// \see [Slerp](https://en.wikipedia.org/wiki/Slerp)
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr quaternion MUU_VECTORCALL slerp(quaternion_param start, quaternion_param finish, scalar_type alpha) noexcept
		{
			MUU_FMA_BLOCK
			using itype = intermediate_type;
			auto dot = raw_dot(start, finish);

			//map from { s, v } and { -s, -v } (they represent the same rotation)
			auto correction = itype{ 1 };
			if (dot < itype{})
			{
				correction = itype{ -1 };
				dot = -dot;
			}

			//they're extremely close, do a normal lerp
			if (dot >= static_cast<itype>(0.9995))
			{
				MUU_FMA_BLOCK
				const auto inv_alpha = itype{ 1 } - static_cast<itype>(alpha);

				return normalize(quaternion{
					static_cast<scalar_type>(start.s * inv_alpha + finish.s * static_cast<itype>(alpha) * correction),
					vector_type::raw_multiply_scalar(start.v, inv_alpha)
						+ vector_type::raw_multiply_scalar(finish.v, static_cast<itype>(alpha) * correction)
				});
			}

			const auto theta_0 = muu::acos(dot);
			const auto theta = theta_0 * static_cast<itype>(alpha);
			const auto sin_theta_div = muu::sin(theta) / muu::sin(theta_0);
			const auto s0 = muu::cos(theta) - dot * sin_theta_div;
			const auto s1 = sin_theta_div;
			return
			{
				static_cast<scalar_type>(start.s * s0 + finish.s * s1 * correction),
				vector_type::raw_multiply_scalar(start.v, s0)
					+ vector_type::raw_multiply_scalar(finish.v, static_cast<itype>(s1) * correction)
			};
		}

		/// \brief	Performs a spherical-linear interpolation on this quaternion (in-place).
		///
		/// \param	target	The 'target' value for the interpolation.
		/// \param	alpha 	The blend factor.
		///
		/// \return	A reference to the quaternion.
		/// 
		/// \see [Slerp](https://en.wikipedia.org/wiki/Slerp)
		constexpr quaternion& MUU_VECTORCALL slerp(quaternion_param target, scalar_type alpha) noexcept
		{
			return *this = slerp(*this, target, alpha);
		}

	#endif // misc
	};

	#ifndef DOXYGEN // deduction guides -------------------------------------------------------------------------------

	template <typename S MUU_SFINAE(is_arithmetic<S>)>
	quaternion(S, S, S, S) -> quaternion<impl::std_math_common_type<S>>;

	template <typename S, typename VX, typename VY, typename VZ MUU_SFINAE(all_arithmetic<S, VX, VY, VZ>)>
	quaternion(S, VX, VY, VZ) -> quaternion<impl::std_math_common_type<impl::highest_ranked<S, VX, VY, VZ>>>;

	template <typename R, typename I MUU_SFINAE(all_arithmetic<R, I>)>
	quaternion(R, vector<I, 3>) -> quaternion<impl::std_math_common_type<impl::highest_ranked<R, I>>>;

	template <typename S>
	quaternion(const axis_angle_rotation<S>&) -> quaternion<S>;

	template <typename S>
	quaternion(const euler_rotation<S>&) -> quaternion<S>;

	#endif // deduction guides
}
MUU_NAMESPACE_END

namespace std
{
	/// \related	muu::quaternion
	///
	/// \brief Specialization of std::tuple_size for muu::quaternion.
	template <typename Scalar>
	struct tuple_size<muu::quaternion<Scalar>>
	{
		static constexpr size_t value = 2;
	};

	/// \related	muu::quaternion
	///
	/// \brief Specialization of std::tuple_element for muu::quaternion.
	template <size_t I, typename Scalar>
	struct tuple_element<I, muu::quaternion<Scalar>>
	{
		static_assert(I < 2);
		using type = std::conditional_t<I == 1, muu::vector<Scalar, 3>, Scalar>;
	};
}

#endif // =============================================================================================================

//=====================================================================================================================
// CONSTANTS
#if 1

MUU_PUSH_PRECISE_MATH

MUU_NAMESPACE_START
{
	/// \ingroup	constants
	/// \related	muu::quaternion
	/// \see		muu::quaternion
	/// 
	/// \brief		Quaternion constants.
	template <typename Scalar>
	struct constants<quaternion<Scalar>>
	{
		using scalars = constants<Scalar>;
		using vectors = constants<vector<Scalar, 3>>;

		/// \brief A quaternion with all members initialized to zero.
		static constexpr auto zero     = quaternion<Scalar>{ scalars::zero, vectors::zero };
		
		/// \brief The identity quaternion.
		static constexpr auto identity = quaternion<Scalar>{ scalars::one, vectors::zero };
	};
}
MUU_NAMESPACE_END

MUU_POP_PRECISE_MATH

#endif // =============================================================================================================

//=====================================================================================================================
// FREE FUNCTIONS
#if 1

MUU_NAMESPACE_START
{
	/// \ingroup	infinity_or_nan
	/// \related	muu::quaternion
	///
	/// \brief	Returns true if any of the scalar components of a #quaternion are infinity or NaN.
	template <typename S ENABLE_PAIRED_FUNC_BY_REF(S, true)>
	[[nodiscard]]
	MUU_ATTR(pure)
	MUU_ALWAYS_INLINE
	constexpr bool infinity_or_nan(const quaternion<S>& q) noexcept
	{
		return quaternion<S>::infinity_or_nan(q);
	}

	/// \ingroup	approx_equal
	/// \related	muu::quaternion
	///
	/// \brief		Returns true if two quaternions are approximately equal.
	template <typename S, typename T,
		typename Epsilon = impl::highest_ranked<S, T>
		ENABLE_PAIRED_FUNC_BY_REF(S, impl::pass_quaternion_by_reference<T>)
	>
	[[nodiscard]]
	MUU_ATTR(pure)
	MUU_ALWAYS_INLINE
	constexpr bool approx_equal(
		const quaternion<S>& q1,
		const quaternion<T>& q2,
		dont_deduce<Epsilon> epsilon = muu::constants<Epsilon>::approx_equal_epsilon
	) noexcept
	{
		static_assert(is_same_as_any<Epsilon, S, T>);

		return quaternion<S>::approx_equal(q1, q2, epsilon);
	}

	/// \ingroup	approx_zero
	/// \related	muu::quaternion
	///
	/// \brief		Returns true if all the scalar components of a #quaternion are approximately equal to zero.
	template <typename S ENABLE_PAIRED_FUNC_BY_REF(S, true)>
	[[nodiscard]]
	MUU_ATTR(pure)
	MUU_ALWAYS_INLINE
	constexpr bool approx_zero(
		const quaternion<S>& q,
		S epsilon = muu::constants<S>::approx_equal_epsilon
	) noexcept
	{
		return quaternion<S>::approx_zero(q, epsilon);
	}

	/// \ingroup	unit_length
	/// \related	muu::quaternion
	///
	/// \brief Returns true if a #quaternion is unit-length (i.e. has a length of 1).
	template <typename S ENABLE_PAIRED_FUNC_BY_REF(S, true)>
	[[nodiscard]]
	MUU_ATTR(pure)
	MUU_ALWAYS_INLINE
	constexpr bool MUU_VECTORCALL unit_length(const quaternion<S>& q) noexcept
	{
		return quaternion<S>::unit_length(q);
	}

	/// \related muu::quaternion
	///
	/// \brief	Returns the dot product of two quaternions.
	template <typename S ENABLE_PAIRED_FUNC_BY_REF(S, true)>
	[[nodiscard]]
	MUU_ATTR(pure)
	MUU_ALWAYS_INLINE
	constexpr S dot(const quaternion<S>& q1, const quaternion<S>& q2) noexcept
	{
		return quaternion<S>::dot(q1, q2);
	}

	/// \related muu::quaternion
	///
	/// \brief	Normalizes a #quaternion.
	///
	/// \param q	The quaternion to normalize.
	/// 
	/// \return		A normalized copy of the input quaternion.
	template <typename S ENABLE_PAIRED_FUNC_BY_REF(S, true)>
	[[nodiscard]]
	MUU_ATTR(pure)
	MUU_ALWAYS_INLINE
	constexpr quaternion<S> normalize(const quaternion<S>& q) noexcept
	{
		return quaternion<S>::normalize(q);
	}

	/// \related	muu::quaternion
	/// 			
	/// \brief	Performs a spherical-linear interpolation between two quaternions.
	template <typename S ENABLE_PAIRED_FUNC_BY_REF(S, true)>
	[[nodiscard]]
	MUU_ATTR(pure)
	MUU_ALWAYS_INLINE
	constexpr quaternion<S> slerp(const quaternion<S>& start, const quaternion<S>& finish, S alpha) noexcept
	{
		return quaternion<S>::slerp(start, finish, alpha);
	}

	#if ENABLE_PAIRED_FUNCS

	template <typename S ENABLE_PAIRED_FUNC_BY_VAL(S, true)>
	[[nodiscard]]
	MUU_ATTR(const)
	MUU_ALWAYS_INLINE
	constexpr bool infinity_or_nan(quaternion<S> q) noexcept
	{
		return quaternion<S>::infinity_or_nan(q);
	}

	template <typename S, typename T,
		typename Epsilon = impl::highest_ranked<S, T>
		ENABLE_PAIRED_FUNC_BY_VAL(S, impl::pass_quaternion_by_value<T>)
	>
	[[nodiscard]]
	MUU_ATTR(const)
	MUU_ALWAYS_INLINE
	constexpr bool MUU_VECTORCALL approx_equal(
		quaternion<S> q1,
		quaternion<T> q2,
		dont_deduce<Epsilon> epsilon = muu::constants<Epsilon>::approx_equal_epsilon
	) noexcept
	{
		static_assert(is_same_as_any<Epsilon, S, T>);

		return quaternion<S>::approx_equal(q1, q2, epsilon);
	}

	template <typename S ENABLE_PAIRED_FUNC_BY_VAL(S, true)>
	[[nodiscard]]
	MUU_ATTR(const)
	MUU_ALWAYS_INLINE
	constexpr bool MUU_VECTORCALL approx_zero(
		quaternion<S> q,
		S epsilon = muu::constants<S>::approx_equal_epsilon
	) noexcept
	{
		return quaternion<S>::approx_zero(q, epsilon);
	}

	template <typename S ENABLE_PAIRED_FUNC_BY_VAL(S, true)>
	[[nodiscard]]
	MUU_ATTR(const)
	MUU_ALWAYS_INLINE
	constexpr bool MUU_VECTORCALL unit_length(quaternion<S> q) noexcept
	{
		return quaternion<S>::unit_length(q);
	}

	template <typename S ENABLE_PAIRED_FUNC_BY_VAL(S, true)>
	[[nodiscard]]
	MUU_ATTR(const)
	MUU_ALWAYS_INLINE
	constexpr S MUU_VECTORCALL dot(quaternion<S> q1, quaternion<S> q2) noexcept
	{
		return quaternion<S>::dot(q1, q2);
	}

	template <typename S ENABLE_PAIRED_FUNC_BY_VAL(S, true)>
	[[nodiscard]]
	MUU_ATTR(const)
	MUU_ALWAYS_INLINE
	constexpr quaternion<S> MUU_VECTORCALL normalize(quaternion<S> q) noexcept
	{
		return quaternion<S>::normalize(q);
	}

	template <typename S ENABLE_PAIRED_FUNC_BY_VAL(S, true)>
	[[nodiscard]]
	MUU_ATTR(const)
	MUU_ALWAYS_INLINE
	constexpr quaternion<S> MUU_VECTORCALL slerp(quaternion<S> start, quaternion<S> finish, S alpha) noexcept
	{
		return quaternion<S>::slerp(start, finish, alpha);
	}

	#endif // ENABLE_PAIRED_FUNCS
}
MUU_NAMESPACE_END

#endif // =============================================================================================================

#undef ENABLE_PAIRED_FUNC_BY_REF
#undef ENABLE_PAIRED_FUNC_BY_VAL
#undef ENABLE_PAIRED_FUNCS

MUU_PRAGMA_MSVC(pop_macro("min"))
MUU_PRAGMA_MSVC(pop_macro("max"))
MUU_PRAGMA_MSVC(float_control(pop))
MUU_PRAGMA_MSVC(inline_recursion(off))

MUU_POP_WARNINGS	// MUU_DISABLE_SHADOW_WARNINGS
