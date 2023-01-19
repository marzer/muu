// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::euler_angles.

#include "vector.h"
#include "impl/std_iosfwd.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_DISABLE_SHADOW_WARNINGS;
MUU_PRAGMA_MSVC(float_control(except, off))

//======================================================================================================================
// IMPLEMENTATION DETAILS
//======================================================================================================================
/// \cond

namespace muu::impl
{
	template <typename Scalar>
	struct MUU_TRIVIAL_ABI storage_base<euler_angles<Scalar>>
	{
		Scalar yaw;
		Scalar pitch;
		Scalar roll;
	};

	template <typename Scalar>
	inline constexpr bool is_hva<storage_base<euler_angles<Scalar>>> =
		can_be_hva_of<Scalar, storage_base<euler_angles<Scalar>>>;

	template <typename Scalar>
	inline constexpr bool is_hva<euler_angles<Scalar>> = is_hva<storage_base<euler_angles<Scalar>>>;

	template <typename Scalar>
	struct vector_param_<euler_angles<Scalar>>
	{
		using type = copy_cvref<euler_angles<Scalar>, vector_param<storage_base<euler_angles<Scalar>>>>;
	};
}

namespace muu
{

	template <typename From, typename Scalar>
	inline constexpr bool allow_implicit_bit_cast<From, impl::storage_base<euler_angles<Scalar>>> =
		allow_implicit_bit_cast<From, euler_angles<Scalar>>;
}

/// \endcond

//======================================================================================================================
// EULER ANGLES
//======================================================================================================================

namespace muu
{
	/// \brief	A set of euler angles used for rotation.
	/// \ingroup math
	///
	/// \details This type models a specific form of Euler angles relating to the Aircraft Principal Axes,
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
	struct MUU_TRIVIAL_ABI euler_angles //
		MUU_HIDDEN_BASE(impl::storage_base<euler_angles<Scalar>>)
	{
		static_assert(!std::is_reference_v<Scalar>, "Euler rotation scalar type cannot be a reference");
		static_assert(!is_cv<Scalar>, "Euler rotation scalar type cannot be const- or volatile-qualified");
		static_assert(is_floating_point<Scalar>, "Euler angles scalar type must be a floating-point type");

		/// \brief The scalar type of the rotation's angles.
		using scalar_type = Scalar;

	  private:
		/// \cond

		using base = impl::storage_base<euler_angles<Scalar>>;

		/// \endcond

	  public:
#if MUU_DOXYGEN

		/// \brief The rotation around the local up axis (in radians).
		/// \remark A positive yaw means "turn the nose of the aircraft to the right".
		scalar_type yaw;

		/// \brief The rotation around the local right axis (in radians).
		/// \remark A positive pitch means "point the nose of the aircraft up toward the sky".
		scalar_type pitch;

		/// \brief The rotation around the local forward axis (in radians).
		/// \remark A positive roll means "tilt the aircraft so the right wing points toward the ground".
		scalar_type roll;

#endif // DOXYGEN

		/// \brief Default constructor. Values are not initialized.
		MUU_NODISCARD_CTOR
		euler_angles() noexcept = default;

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr euler_angles(const euler_angles&) noexcept = default;

		/// \brief Copy-assigment operator.
		constexpr euler_angles& operator=(const euler_angles&) noexcept = default;

		/// \brief Initializes from three separate euler angles.
		MUU_NODISCARD_CTOR
		constexpr euler_angles(scalar_type yaw, scalar_type pitch, scalar_type roll = scalar_type{}) noexcept //
			: base{ yaw, pitch, roll }
		{}

		/// \brief Initializes from a vec3, where x, y and z are yaw, pitch and roll, respectively.
		MUU_NODISCARD_CTOR
		explicit constexpr euler_angles(const vector<scalar_type, 3>& v) noexcept //
			: base{ v.x, v.y, v.z }
		{}

		/// \brief Converting constructor.
		template <typename T>
		MUU_NODISCARD_CTOR
		explicit constexpr euler_angles(const euler_angles<T>& other) noexcept //
			: base{ static_cast<scalar_type>(other.yaw),
					static_cast<scalar_type>(other.pitch),
					static_cast<scalar_type>(other.roll) }
		{}

		/// \brief Constructs a set of euler angles from an implicitly bit-castable type.
		///
		/// \tparam T	A bit-castable type.
		///
		/// \see muu::allow_implicit_bit_cast
		MUU_CONSTRAINED_TEMPLATE((allow_implicit_bit_cast<T, euler_angles>), typename T)
		MUU_NODISCARD_CTOR
		/*implicit*/
		constexpr euler_angles(const T& obj) noexcept //
			: base{ muu::bit_cast<base>(obj) }
		{
			static_assert(sizeof(T) == sizeof(base), "Bit-castable types must be the same size");
			static_assert(std::is_trivially_copyable_v<T>, "Bit-castable types must be trivially-copyable");
		}

		/// \brief Converts the euler angles to a vec3, where x, y and z are yaw, pitch and roll, respectively.
		MUU_PURE_INLINE_GETTER
		explicit constexpr operator vector<scalar_type, 3>() const noexcept
		{
			return vector<scalar_type, 3>{ base::yaw, base::pitch, base::roll };
		}

		/// \brief	Scales a set of euler angles.
		MUU_PURE_GETTER
		friend constexpr euler_angles MUU_VECTORCALL operator*(MUU_VPARAM(euler_angles) lhs, scalar_type rhs) noexcept
		{
			return euler_angles{ lhs.yaw * rhs, lhs.pitch * rhs, lhs.roll * rhs };
		}

		/// \brief	Scales a set of euler angles.
		MUU_PURE_GETTER
		friend constexpr euler_angles MUU_VECTORCALL operator*(scalar_type lhs, MUU_VPARAM(euler_angles) rhs) noexcept
		{
			return rhs * lhs;
		}

		/// \brief	Scales the euler angles.
		constexpr euler_angles& MUU_VECTORCALL operator*=(scalar_type rhs) noexcept
		{
			return *this = *this * rhs;
		}

		/// \brief Writes a set of euler angles out to a text stream.
		template <typename Char, typename Traits>
		friend std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os,
															const euler_angles& rot)
		{
			static_assert(sizeof(euler_angles) == sizeof(scalar_type) * 3);
			impl::print_vector(os, &rot.yaw, 3u);
			return os;
		}
	};

	/// \cond

	MUU_CONSTRAINED_TEMPLATE((all_arithmetic<Yaw, Pitch>), typename Yaw, typename Pitch)
	euler_angles(Yaw, Pitch)->euler_angles<impl::std_math_common_type<impl::highest_ranked<Yaw, Pitch>>>;

	MUU_CONSTRAINED_TEMPLATE((all_arithmetic<Yaw, Pitch, Roll>), typename Yaw, typename Pitch, typename Roll)
	euler_angles(Yaw, Pitch, Roll)->euler_angles<impl::std_math_common_type<impl::highest_ranked<Yaw, Pitch, Roll>>>;

	template <typename S, size_t D>
	euler_angles(vector<S, D>) -> euler_angles<impl::std_math_common_type<S>>;

	/// \endcond
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
