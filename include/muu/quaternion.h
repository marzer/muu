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

MUU_NAMESPACE_START
{
	/// \brief	An axis+angle rotation.
	///
	/// \tparam	T	The scalar component type of the data members.
	template <typename T>
	struct MUU_TRIVIAL_ABI axis_angle_rotation
	{
		/// \brief	The axis being rotated around.
		vector<T, 3> axis;

		/// \brief	The angle of rotation, in radians.
		T angle;
	};
}
MUU_NAMESPACE_END

#ifndef DOXYGEN

#if 1 // helper macros ------------------------------------------------------------------------------------------------

#define ENABLE_PAIRED_FUNC_BY_REF(S, ...) \
	MUU_SFINAE((MUU_INTELLISENSE || impl::pass_quaternion_by_reference<S>) && (__VA_ARGS__))

#define ENABLE_PAIRED_FUNC_BY_VAL(S, ...) \
	MUU_SFINAE_2(!MUU_INTELLISENSE && impl::pass_quaternion_by_value<S> && (__VA_ARGS__))

#define ENABLE_PAIRED_FUNCS !MUU_INTELLISENSE

#endif // helper macros

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

	template <typename Scalar>
	inline constexpr bool is_hva<axis_angle_rotation<Scalar>> =
		(is_same_as_any<Scalar, float, double, long double> || is_simd_intrinsic<Scalar>)
		&& sizeof(axis_angle_rotation<Scalar>) == sizeof(Scalar) * 4
		&& alignof(axis_angle_rotation<Scalar>) == alignof(Scalar)
	;

	#endif // MUU_HAS_VECTORCALL

	template <typename Scalar>
	inline constexpr bool pass_quaternion_by_reference =
		std::is_reference_v<maybe_pass_readonly_by_value<quaternion_base<Scalar>>>;

	template <typename Scalar>
	inline constexpr bool pass_quaternion_by_value = !pass_quaternion_by_reference<Scalar>;
}
MUU_IMPL_NAMESPACE_END

#else // ^^^ !DOXYGEN / DOXYGEN vvv

#define ENABLE_PAIRED_FUNC_BY_REF(...)
#define ENABLE_PAIRED_FUNC_BY_VAL(...)
#define ENABLE_PAIRED_FUNCS 0

#endif // DOXYGEN

#endif // =============================================================================================================

//=====================================================================================================================
// QUATERNION CLASS
#if 1

MUU_NAMESPACE_START
{
	/// \brief A quaternion.
	///
	/// \tparam	Scalar      The type of the quaternion's scalar components. Must be a floating-point type.
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

		/// \brief The three-dimensional vector type with the same #scalar_type as this quaternion.
		using vector_type = vector<scalar_type, 3>;

		/// \brief `vector_type` or `const vector_type&`, depending on depending on size, triviality, simd-friendliness, etc.
		using vector_param = typename vector_type::vector_param;

		/// \brief Compile-time constants for this quaternion's #vector_type.
		using vector_constants = muu::constants<vector_type>;

		/// \brief The axis-angle rotation with the same #scalar_type as this quaternion.
		using axis_angle_type = axis_angle_rotation<scalar_type>;

		/// \brief `quaternion` or `const quaternion&`, depending on depending on size, triviality, simd-friendliness, etc.
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
		constexpr quaternion(scalar_type s, scalar_type vx, scalar_type vy, scalar_type vz) noexcept
			: base{ s, vector_type{ vx, vy, vz }  }
		{}

		/// \brief	Constructs a quaternion from a scalar and a vector.
		///
		/// \param	s	Initial value for the scalar (real) part.
		/// \param	v	Initial value for the vector (imaginary) part.
		constexpr quaternion(scalar_type s, const vector_type& v) noexcept
			: base{ s, v }
		{}

		/// \brief Converting constructor.
		template <typename T>
		explicit constexpr quaternion(const quaternion<T>& quat) noexcept
			: base{ static_cast<scalar_type>(quat.s), vector_type{ quat.v } }
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
			return lhs.s == rhs.s
				&& lhs.v == rhs.v;
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
			return lhs.s == rhs.s
				&& lhs.v == rhs.v;
		}

		template <typename T ENABLE_PAIRED_FUNC_BY_VAL(T, true)>
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr bool MUU_VECTORCALL operator != (quaternion_param lhs, quaternion<T> rhs) noexcept
		{
			return !(lhs == rhs);
		}

		#endif // ENABLE_PAIRED_FUNCS

		/// \brief	Returns true if all the scalar components of the quaternion are exactly zero.
		/// 
		/// \remarks	This is a componentwise exact equality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point quaternions, use #approx_zero().
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool zero() const noexcept
		{
			return base::s == scalar_constants::zero
				&& base::v == vector_constants::zero;
		}

		/// \brief	Returns true if any of the scalar components of the quaternion are infinity or NaN.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool infinity_or_nan() const noexcept
		{
			if constexpr (is_floating_point<scalar_type>)
			{
				return muu::infinity_or_nan(base::s)
					|| base::v.infinity_or_nan();
			}
			else
				return false;
		}

		/// \brief Returns true if the quaternion is unit-length (i.e. has a length of 1).
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool unit_length() const noexcept
		{
			constexpr auto epsilon = intermediate_type{ 1 } / (
				100ull
				* (sizeof(scalar_type) >= sizeof(float) ? 10000ull : 1ull)
				* (sizeof(scalar_type) >= sizeof(double) ? 10000ull : 1ull)
			);

			return muu::approx_equal(
				raw_dot(*this, *this),
				intermediate_type{ 1 },
				epsilon
			);
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
			Epsilon epsilon = muu::constants<Epsilon>::approx_equal_epsilon
		) noexcept
		{
			static_assert(is_same_as_any<Epsilon, T, scalar_type>);

			using type = impl::promote_if_small_float<impl::highest_ranked<scalar_type, T>>;

			return muu::approx_equal(static_cast<type>(q1.s), static_cast<type>(q2.s), static_cast<type>(epsilon))
				&& vector_type::approx_equal(q1.v, q2.v, epsilon);
		}

		/// \brief	Returns true if the quaternion is approximately equal to another.
		template <typename T, typename Epsilon = impl::highest_ranked<scalar_type, T> ENABLE_PAIRED_FUNC_BY_REF(T, true)>
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool MUU_VECTORCALL approx_equal(
			const quaternion<T>& q,
			Epsilon epsilon = muu::constants<Epsilon>::approx_equal_epsilon
		) const noexcept
		{
			static_assert(is_same_as_any<Epsilon, T, scalar_type>);

			return approx_equal(*this, q, epsilon);
		}

		#if ENABLE_PAIRED_FUNCS

		template <typename T, typename Epsilon = impl::highest_ranked<scalar_type, T> ENABLE_PAIRED_FUNC_BY_VAL(T, true)>
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool MUU_VECTORCALL approx_equal(
			quaternion_param q1,
			quaternion<T> q2,
			Epsilon epsilon = muu::constants<Epsilon>::approx_equal_epsilon
		) noexcept
		{
			static_assert(is_same_as_any<Epsilon, T, scalar_type>);

			using type = impl::promote_if_small_float<impl::highest_ranked<scalar_type, T>>;

			return muu::approx_equal(static_cast<type>(q1.s), static_cast<type>(q2.s), static_cast<type>(epsilon))
				&& muu::approx_equal(static_cast<type>(q1.v.x), static_cast<type>(q2.v.x), static_cast<type>(epsilon))
				&& muu::approx_equal(static_cast<type>(q1.v.y), static_cast<type>(q2.v.y), static_cast<type>(epsilon))
				&& muu::approx_equal(static_cast<type>(q1.v.z), static_cast<type>(q2.v.z), static_cast<type>(epsilon));
		}

		template <typename T, typename Epsilon = impl::highest_ranked<scalar_type, T> ENABLE_PAIRED_FUNC_BY_VAL(T, true)>
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool MUU_VECTORCALL approx_equal(
			quaternion<T> q,
			Epsilon epsilon = muu::constants<Epsilon>::approx_equal_epsilon
		) const noexcept
		{
			static_assert(is_same_as_any<Epsilon, T, scalar_type>);

			return approx_equal(*this, q, epsilon);
		}

		#endif // ENABLE_PAIRED_FUNCS

		/// \brief	Returns true if all the scalar components in the quaternion are approximately equal to zero.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool MUU_VECTORCALL approx_zero(
			scalar_type epsilon = muu::constants<scalar_type>::approx_equal_epsilon
		) const noexcept
		{
			return approx_equal(*this, constants::zero, epsilon);
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
		/// \param v	The quaternion to normalize.
		/// 
		/// \return		A normalized copy of the input quaternion.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr quaternion MUU_VECTORCALL normalize(quaternion_param v) noexcept
		{
			const intermediate_type inv_length = intermediate_type{ 1 } / muu::sqrt(raw_dot(v, v));
			return quaternion{
				static_cast<scalar_type>(v.s * inv_length),
				vector_type::raw_multiply_scalar(v.v, inv_length)
			};
		}

		/// \brief		Normalizes the quaternion (in-place).
		///
		/// \return		A reference to the quaternion.
		constexpr quaternion& normalize() noexcept
		{
			return (*this = normalize(*this));
		}

	#endif // normalization
	
	#if 1 // inversion ------------------------------------------------------------------------------------------------

		/// \brief		Inverts a quaternion.
		///
		/// \param q	The quaternion to invert.
		/// 
		/// \return		An inverted copy of the input quaternion.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr quaternion MUU_VECTORCALL invert(quaternion_param q) noexcept
		{
			return { q.s, -q.v };
		}

		/// \brief		Inverts the quaternion (in-place).
		///
		/// \return		A reference to the quaternion.
		constexpr quaternion& invert() noexcept
		{
			base::v = -base::v;
			return *this;
		}

	#endif // inversion

	#if 1 // axis-angle conversions -----------------------------------------------------------------------------------

		/// \brief Creates a quaternion from an angle and axis.
		/// 
		/// \param axis		Axis to rotate around. Must be unit-length.
		/// \param angle	Angle in radians to rotate by.
		/// 
		/// \return A quaternion encoding the given angle-axis rotation.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr quaternion MUU_VECTORCALL from_axis_angle(vector_param axis, scalar_type angle) noexcept
		{
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

		/// \brief	Extracts an angle and axis rotation from a quaternion.
		///
		/// \param	quat			The quaternion to convert.
		/// \param	shortest_path	Should the returned values be derived from the shortest path?
		/// 						`true` is more intuitive but means that the returned values may not convert back
		/// 						to the original quaternion if used with #from_axis_angle().
		///
		/// \returns	An axis and angle representing the rotation stored in the given quaternion.
		[[nodiscard]]
		MUU_ATTR(pure)
		static axis_angle_type MUU_VECTORCALL to_axis_angle(quaternion quat, bool shortest_path = true) noexcept
		{
			if (shortest_path && quat.s < scalar_constants::zero)
			{
				quat.s = -quat.s;
				quat.v *= scalar_constants::minus_one;
			}

			// intermediate calcs are done using doubles because anything else is far too imprecise
			using calc_type = impl::highest_ranked<intermediate_type, double>;

			const calc_type length = vector_type::template raw_length<calc_type>(quat.v);
			if (length == calc_type{})
			{
				// This happens at angle = 0 and 360. All axes are correct, so any will do.
				return
				{
					vector_constants::x_axis,
					static_cast<scalar_type>(calc_type{ 2 } * std::atan2(length, static_cast<calc_type>(quat.s))),
				};
			}
			else
			{
				return
				{

					quat.i.raw_divide_assign_scalar(length),
					static_cast<scalar_type>(calc_type{ 2 } * std::atan2(length, static_cast<calc_type>(quat.s))),
				};
			}
		}

	#endif // axis-angle conversions

	#if 1 // multiplication -------------------------------------------------------------------------------------------

		/// \brief Multiplies two quaternions.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr quaternion MUU_VECTORCALL operator * (quaternion_param lhs, quaternion_param rhs) noexcept
		{
			using mult_type = decltype(scalar_type{} * scalar_type{});

			if constexpr (all_same<scalar_type, mult_type, intermediate_type>)
			{
				return
				{
					lhs.s * rhs.s - vector_type::template raw_dot<intermediate_type>(lhs.v, rhs.v),
					lhs.s * rhs.v + rhs.s * lhs.v + vector_type::template raw_cross<intermediate_type>(lhs.v, rhs.v)
				};

			}
			else
			{
				return
				{
					static_cast<scalar_type>(
						static_cast<intermediate_type>(lhs.s) * static_cast<intermediate_type>(rhs.s)
							- vector_type::template raw_dot<intermediate_type>(lhs.v, rhs.v)
					),

					vector_type{
						static_cast<intermediate_type>(lhs.s) * static_cast<intermediate_type>(rhs.v)
							+ static_cast<intermediate_type>(rhs.s) * static_cast<intermediate_type>(lhs.v)
							+ vector_type::template raw_cross<intermediate_type>(lhs.v, rhs.v)
					}
				};
			}
		}

		/// \brief Multiplies this quaternion with another.
		constexpr quaternion& MUU_VECTORCALL operator *= (quaternion_param rhs) noexcept
		{
			return (*this = *this * rhs);
		}

		/// \brief Rotates a three-dimensional vector by the rotation encoded in a quaternion.
		[[nodiscard]]
		friend constexpr vector_type MUU_VECTORCALL operator * (quaternion_param lhs, vector_param rhs) noexcept
		{
			using mult_type = decltype(scalar_type{} * scalar_type{});

			if constexpr (all_same<scalar_type, mult_type, intermediate_type>)
			{
				const auto two_s = lhs.s + lhs.s;
				return two_s * vector_type::template raw_cross<intermediate_type>(lhs.v, rhs)
					+ (two_s * lhs.s - intermediate_type{ 1 }) * rhs
					+ intermediate_type{ 2 } * vector_type::template raw_cross<intermediate_type>(lhs.v, rhs) * lhs.v;
			}
			else
			{
				const auto two_s = static_cast<intermediate_type>(lhs.s) * intermediate_type{ 2 };

				return two_s * vector_type::template raw_cross<intermediate_type>(lhs.v, rhs)
					+ (two_s * static_cast<intermediate_type>(lhs.s) - intermediate_type{ 1 }) * rhs
					+ intermediate_type{ 2 } * vector_type::template raw_dot<intermediate_type>(lhs.v, rhs) * lhs.v;
			}


		}

		/// \brief Scales the shortest-path rotation equivalent of a quaternion by a scalar.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr quaternion MUU_VECTORCALL operator * (quaternion_param lhs, scalar_type rhs) noexcept
		{
			auto aa = to_axis_angle(lhs);
			aa.angle *= rhs * scalar_constants::one_over_two;
			aa.axis.normalize(); //todo: unnecessary?
			return
			{
				muu::cos(aa.angle),
				muu::sin(aa.angle) * aa.axis
			};
		}

	#endif // multiplication
	};

	#ifndef DOXYGEN // deduction guides -------------------------------------------------------------------------------

	template <typename R, typename IX, typename IY, typename IZ MUU_SFINAE(all_arithmetic<R, IX, IY, IZ>)>
	quaternion(R, IX, IY, IZ) -> quaternion<impl::highest_ranked<R, IX, IY, IZ>>;

	template <typename R, typename I MUU_SFINAE(all_arithmetic<R, I>)>
	quaternion(R, vector<I, 3>) -> quaternion<impl::highest_ranked<R, I>>;

	#endif // deduction guides
}
MUU_NAMESPACE_END

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

#endif // =============================================================================================================

//=====================================================================================================================
// CONSTANTS
#if 1

#endif // =============================================================================================================

//=====================================================================================================================
// FREE FUNCTIONS
#if 1

#endif // =============================================================================================================

#undef ENABLE_PAIRED_FUNC_BY_REF
#undef ENABLE_PAIRED_FUNC_BY_VAL
#undef ENABLE_PAIRED_FUNCS

MUU_PRAGMA_MSVC(pop_macro("min"))
MUU_PRAGMA_MSVC(pop_macro("max"))
MUU_PRAGMA_MSVC(float_control(pop))
MUU_PRAGMA_MSVC(inline_recursion(off))

MUU_POP_WARNINGS	// MUU_DISABLE_SHADOW_WARNINGS
