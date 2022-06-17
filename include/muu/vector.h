// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::vector.

/*
	VECTOR CLASS DESIGN - KEY POINTS

	a.k.a. pre-emptive answers to "why is _____ implemented like that?"

	-	Vectors of <= 4 dimensions have named members x,y,z and w, whereas those with dimensions > 4
		have a single values[] array. This is accomplished by the use of specialized base class templates.
		While complicating the main class implementation somewhat, it is a great usability boon since in general being
		able to refer to component 0 like "vec.x" is much more ergonomic than vec[0] or vec.get<0>().
		Implementation complexity is largely addressed by the use of some generic function body implementation macros.

	-	"Why not just make them a union?"
		Because unions in C++ are a complicated mess, and almost impossible to use if you value any sort of constexpr.

	-	"Macros??"
		Yes. I could have gone all FP and used a bunch of lambdas but they only get optimized out when the optimizer is
		actually run; debug builds still have a bunch of nested calls and lambda instantiations. Macros + 'if constexpr'
		end up being much more debug friendly.

	-	Some functions use constraints to select overloads that take things either by reference or value, with the value
		overloads being 'hidden' from doxygen and intellisense. The value overloads are optimizations to take
		advantage of __vectorcall on windows, and only apply to float, double and long double vectors of <= 4 dimensions
		since the're considered "Homogeneous Vector Aggreggates" and must be passed by value to be properly vectorized.
		- __vectorcall: https://docs.microsoft.com/en-us/cpp/cpp/vectorcall?view=vs-2019
		- vectorizer sandbox: https://godbolt.org/z/vn8aKv (change vector_param_mode to see the effect in the MSVC tab)

	-	You'll see promoted_delta used instead of scalar_type or delta_scalar_type in a few places. It's a 'better'
		type used for intermediate floating-point values where precision loss or unnecessary cast round-tripping is to
		be avoided. generally:
			- when scalar_type is a float >= 32 bits: promoted_delta == scalar_type
			- when scalar_type is a float < 32 bits: promoted_delta == float
			- when scalar_type is integral: promoted_delta == double.

	-	Some code is statically switched/branched according to whether a static_cast<> is necessary; this is to avoid
		unnecessary codegen and improve debug build performance for non-trivial scalar_types (e.g. muu::half).
*/

#include "impl/std_initializer_list.h"
#include "impl/vector_types_common.h"
#include "impl/vector_base.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_DISABLE_SHADOW_WARNINGS;
MUU_DISABLE_ARITHMETIC_WARNINGS;
MUU_PRAGMA_MSVC(float_control(except, off))

//======================================================================================================================
// IMPLEMENTATION DETAILS
//======================================================================================================================
/// \cond

// helper macros -------------------------------------------------------------------------------------------------------

#define COMPONENTWISE_AND(func)                                                                                        \
	do                                                                                                                 \
	{                                                                                                                  \
		MUU_FMA_BLOCK;                                                                                                 \
		if constexpr (Dimensions == 1)                                                                                 \
			return func(x);                                                                                            \
		if constexpr (Dimensions == 2)                                                                                 \
			return (func(x)) && (func(y));                                                                             \
		if constexpr (Dimensions == 3)                                                                                 \
			return (func(x)) && (func(y)) && (func(z));                                                                \
		if constexpr (Dimensions == 4)                                                                                 \
			return (func(x)) && (func(y)) && (func(z)) && (func(w));                                                   \
		if constexpr (Dimensions > 4)                                                                                  \
		{                                                                                                              \
			for (size_t i = 0; i < Dimensions; i++)                                                                    \
				if (!(func(values[i])))                                                                                \
					return false;                                                                                      \
			return true;                                                                                               \
		}                                                                                                              \
	}                                                                                                                  \
	while (false)

#define COMPONENTWISE_OR(func)                                                                                         \
	do                                                                                                                 \
	{                                                                                                                  \
		MUU_FMA_BLOCK;                                                                                                 \
		if constexpr (Dimensions == 1)                                                                                 \
			return func(x);                                                                                            \
		if constexpr (Dimensions == 2)                                                                                 \
			return (func(x)) || (func(y));                                                                             \
		if constexpr (Dimensions == 3)                                                                                 \
			return (func(x)) || (func(y)) || (func(z));                                                                \
		if constexpr (Dimensions == 4)                                                                                 \
			return (func(x)) || (func(y)) || (func(z)) || (func(w));                                                   \
		if constexpr (Dimensions > 4)                                                                                  \
		{                                                                                                              \
			for (size_t i = 0; i < Dimensions; i++)                                                                    \
				if (func(values[i]))                                                                                   \
					return true;                                                                                       \
			return false;                                                                                              \
		}                                                                                                              \
	}                                                                                                                  \
	while (false)

#define COMPONENTWISE_ACCUMULATE(func, op)                                                                             \
	do                                                                                                                 \
	{                                                                                                                  \
		MUU_FMA_BLOCK;                                                                                                 \
		if constexpr (Dimensions == 1)                                                                                 \
			return func(x);                                                                                            \
		if constexpr (Dimensions == 2)                                                                                 \
			return (func(x))op(func(y));                                                                               \
		if constexpr (Dimensions == 3)                                                                                 \
			return (func(x))op(func(y)) op(func(z));                                                                   \
		if constexpr (Dimensions == 4)                                                                                 \
			return (func(x))op(func(y)) op(func(z)) op(func(w));                                                       \
		if constexpr (Dimensions > 4)                                                                                  \
		{                                                                                                              \
			auto val = (func(values[0]))op(func(values[1])) op(func(values[2])) op(func(values[3]));                   \
			for (size_t i = 4; i < Dimensions; i++)                                                                    \
				val op## = func(values[i]);                                                                            \
			return val;                                                                                                \
		}                                                                                                              \
	}                                                                                                                  \
	while (false)

#define IDENTITY_TRANSFORM(x) x

#define COMPONENTWISE_CASTING_OP_BRANCH(func, transformer, x_selector)                                                 \
	using func_type = decltype(func(x_selector));                                                                      \
	if constexpr (!std::is_same_v<func_type, Scalar>)                                                                  \
	{                                                                                                                  \
		transformer(func, static_cast<Scalar>);                                                                        \
	}                                                                                                                  \
	else                                                                                                               \
	{                                                                                                                  \
		transformer(func, IDENTITY_TRANSFORM);                                                                         \
	}                                                                                                                  \
	static_assert(true)

#define COMPONENTWISE_CASTING_OP(func, transformer)                                                                    \
	if constexpr (Dimensions <= 4)                                                                                     \
	{                                                                                                                  \
		COMPONENTWISE_CASTING_OP_BRANCH(func, transformer, x);                                                         \
	}                                                                                                                  \
	else                                                                                                               \
	{                                                                                                                  \
		COMPONENTWISE_CASTING_OP_BRANCH(func, transformer, values[0]);                                                 \
	}                                                                                                                  \
	static_assert(true)

#define COMPONENTWISE_CONSTRUCT_WITH_TRANSFORM(func, xform)                                                            \
	do                                                                                                                 \
	{                                                                                                                  \
		MUU_FMA_BLOCK;                                                                                                 \
		if constexpr (Dimensions == 1)                                                                                 \
			return vector{ xform(func(x)) };                                                                           \
		if constexpr (Dimensions == 2)                                                                                 \
			return vector{ xform(func(x)), xform(func(y)) };                                                           \
		if constexpr (Dimensions == 3)                                                                                 \
			return vector{ xform(func(x)), xform(func(y)), xform(func(z)) };                                           \
		if constexpr (Dimensions == 4)                                                                                 \
			return vector{ xform(func(x)), xform(func(y)), xform(func(z)), xform(func(w)) };                           \
		if constexpr (Dimensions > 4)                                                                                  \
		{                                                                                                              \
			return vector{ impl::componentwise_func_tag{},                                                             \
						   [&](size_t i) noexcept                                                                      \
						   {                                                                                           \
							   MUU_FMA_BLOCK;                                                                          \
							   return xform(func(values[i]));                                                          \
						   } };                                                                                        \
		}                                                                                                              \
	}                                                                                                                  \
	while (false)

#define COMPONENTWISE_CONSTRUCT(func) COMPONENTWISE_CASTING_OP(func, COMPONENTWISE_CONSTRUCT_WITH_TRANSFORM)

#define COMPONENTWISE_ASSIGN_WITH_TRANSFORM(func, xform)                                                               \
	do                                                                                                                 \
	{                                                                                                                  \
		MUU_FMA_BLOCK;                                                                                                 \
		if constexpr (Dimensions <= 4)                                                                                 \
		{                                                                                                              \
			base::x = xform(func(x));                                                                                  \
			if constexpr (Dimensions >= 2)                                                                             \
				base::y = xform(func(y));                                                                              \
			if constexpr (Dimensions >= 3)                                                                             \
				base::z = xform(func(z));                                                                              \
			if constexpr (Dimensions == 4)                                                                             \
				base::w = xform(func(w));                                                                              \
		}                                                                                                              \
		else                                                                                                           \
		{                                                                                                              \
			for (size_t i = 0; i < Dimensions; i++)                                                                    \
				base::values[i] = xform(func(values[i]));                                                              \
		}                                                                                                              \
		return *this;                                                                                                  \
	}                                                                                                                  \
	while (false)

#define COMPONENTWISE_ASSIGN(func) COMPONENTWISE_CASTING_OP(func, COMPONENTWISE_ASSIGN_WITH_TRANSFORM)

#define SPECIALIZED_IF(cond) , bool = (cond)

/// \endcond

#ifndef SPECIALIZED_IF
	#define SPECIALIZED_IF(cond)
#endif

//======================================================================================================================
// VECTOR CLASS
//======================================================================================================================

namespace muu
{
	/// \brief An N-dimensional vector.
	/// \ingroup math
	///
	/// \tparam	Scalar      The vector's scalar component type.
	/// \tparam Dimensions  The number of dimensions.
	template <typename Scalar, size_t Dimensions>
	struct MUU_TRIVIAL_ABI vector MUU_HIDDEN_BASE(impl::vector_<Scalar, Dimensions>)
	{
		static_assert(!is_cvref<Scalar>, "Vector scalar type cannot be const, volatile, or a reference");
		static_assert(std::is_trivially_constructible_v<Scalar>	  //
						  && std::is_trivially_copyable_v<Scalar> //
						  && std::is_trivially_destructible_v<Scalar>,
					  "Vector scalar type must be trivially constructible, copyable and destructible");
		static_assert(Dimensions >= 1, "Vectors must have at least one dimension");

		/// \brief The number of scalar components stored in this vector.
		static constexpr size_t dimensions = Dimensions;

		/// \brief The type of each scalar component stored in this vector.
		using scalar_type = Scalar;

		/// \brief The scalar type used for length, distance, blend factors, etc. Always floating-point.
		using delta_scalar_type = std::conditional_t<is_integral<scalar_type>, double, scalar_type>;

		/// \brief The vector type with #scalar_type == #delta_scalar_type and the same number of #dimensions as this one.
		using delta_type = vector<delta_scalar_type, dimensions>;

		/// \brief The scalar type used for products (dot, cross, etc.). Always signed.
		using product_scalar_type = std::
			conditional_t<is_integral<scalar_type>, impl::highest_ranked<make_signed<scalar_type>, int>, scalar_type>;

		/// \brief The vector type with #scalar_type == #product_scalar_type and the same number of #dimensions as this one.
		using product_type = vector<product_scalar_type, dimensions>;

		/// \brief Compile-time constants for this vector type.
		using constants = muu::constants<vector>;

		/// \brief A LegacyRandomAccessIterator for the scalar components in the vector.
		using iterator = scalar_type*;

		/// \brief A const LegacyRandomAccessIterator for the scalar components in the vector.
		using const_iterator = const scalar_type*;

	  private:
		/// \cond

		template <typename, size_t>
		friend struct vector;

		using base = impl::vector_<scalar_type, Dimensions>;
		static_assert(sizeof(base) == (sizeof(scalar_type) * Dimensions), "Vectors should not have padding");

		using promoted_scalar				 = promote_if_small_float<scalar_type>;
		using promoted_vec					 = vector<promoted_scalar, Dimensions>;
		static constexpr bool is_small_float = impl::is_small_float_<scalar_type>;

		static_assert(is_signed<product_scalar_type>);
		using promoted_product	   = promote_if_small_float<product_scalar_type>;
		using promoted_product_vec = vector<promoted_product, Dimensions>;
		static constexpr bool product_requires_promotion =
			!all_same<product_scalar_type, scalar_type, promoted_product>;

		static_assert(is_floating_point<delta_scalar_type>);
		using promoted_delta						   = promote_if_small_float<delta_scalar_type>;
		using promoted_delta_vec					   = vector<promoted_delta, Dimensions>;
		static constexpr bool delta_requires_promotion = !all_same<delta_scalar_type, scalar_type, promoted_delta>;

		using scalar_constants = muu::constants<scalar_type>;

		/// \endcond

	  public:
#if MUU_DOXYGEN

		/// \brief The vector's 0th scalar component (when #dimensions &lt;= 4).
		scalar_type x;
		/// \brief The vector's 1st scalar component (when #dimensions == 2, 3 or 4).
		scalar_type y;
		/// \brief The vector's 2nd scalar component (when #dimensions == 3 or 4).
		scalar_type z;
		/// \brief The vector's 3rd scalar component (when #dimensions == 4).
		scalar_type w;
		/// \brief The vector's scalar component array (when #dimensions &gt;= 5).
		scalar_type values[dimensions];

#endif // DOXYGEN

#if 1 // constructors ----------------------------------------------------------------------------------------------

	  private:
		/// \cond

		template <typename Func>
		explicit constexpr vector(impl::componentwise_func_tag, Func&& func) noexcept
			: base{ impl::componentwise_func_tag{}, std::make_index_sequence<Dimensions>{}, static_cast<Func&&>(func) }
		{}

		/// \endcond

	  public:
		/// \brief Default constructor. Scalar components are not initialized.
		MUU_NODISCARD_CTOR
		vector() noexcept = default;

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr vector(const vector&) noexcept = default;

		/// \brief Copy-assigment operator.
		constexpr vector& operator=(const vector&) noexcept = default;

		/// \brief	Constructs a vector with all scalar components set to the same value.
		///
		/// \param	fill	The value used to initialize each of the vector's scalar components.
		MUU_NODISCARD_CTOR
		explicit constexpr vector(scalar_type fill) noexcept //
			: base{ impl::value_fill_tag{}, fill }
		{}

		/// \brief		Constructs a vector from two scalar values.
		/// \details	Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \param	x		Initial value for the vector's first scalar component.
		/// \param	y		Initial value for the vector's second scalar component.
		///
		/// \availability		This constructor is only available when #dimensions &gt;= 2.
		MUU_HIDDEN_CONSTRAINT(Dims >= 2, size_t Dims = Dimensions)
		MUU_NODISCARD_CTOR
		constexpr vector(scalar_type x, scalar_type y) noexcept //
			: base{ x, y }
		{}

		/// \brief		Constructs a vector from three scalar values.
		/// \details	Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \param	x		Initial value for the vector's first scalar component.
		/// \param	y		Initial value for the vector's second scalar component.
		/// \param	z		Initial value for the vector's third scalar component.
		///
		/// \availability		This constructor is only available when #dimensions &gt;= 3.
		MUU_HIDDEN_CONSTRAINT(Dims >= 3, size_t Dims = Dimensions)
		MUU_NODISCARD_CTOR
		constexpr vector(scalar_type x, scalar_type y, scalar_type z) noexcept //
			: base{ x, y, z }
		{}

		/// \brief		Constructs a vector from four scalar values.
		/// \details	Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \param	x		Initial value for the vector's first scalar component.
		/// \param	y		Initial value for the vector's second scalar component.
		/// \param	z		Initial value for the vector's third scalar component.
		/// \param	w		Initial value for the vector's fourth scalar component.
		///
		/// \availability			This constructor is only available when #dimensions &gt;= 4.
		MUU_HIDDEN_CONSTRAINT(Dims >= 4, size_t Dims = Dimensions)
		MUU_NODISCARD_CTOR
		constexpr vector(scalar_type x, scalar_type y, scalar_type z, scalar_type w) noexcept //
			: base{ x, y, z, w }
		{}

		/// \brief		Constructs a vector from five or more scalar values.
		/// \details	Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \tparam T		Types convertible to #scalar_type.
		/// \param	x		Initial value for the vector's first scalar component.
		/// \param	y		Initial value for the vector's second scalar component.
		/// \param	z		Initial value for the vector's third scalar component.
		/// \param	w		Initial value for the vector's fourth scalar component.
		/// \param	vals	Initial values for the vector's remaining scalar components.
		///
		/// \availability			This constructor is only available when #dimensions &gt;= 5.
		MUU_CONSTRAINED_TEMPLATE((Dims >= (4 + sizeof...(T)) //
								  && all_convertible_to<Scalar, const T&...>),
								 typename... T //
									 MUU_HIDDEN_PARAM(size_t Dims = Dimensions))
		MUU_NODISCARD_CTOR
		constexpr vector(scalar_type x, scalar_type y, scalar_type z, scalar_type w, const T&... vals) noexcept
			: base{ x, y, z, w, vals... }
		{}

		/// \brief Constructs a vector from a raw array of scalars.
		/// \details	Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \tparam N		The number of elements in the array.
		/// \param	arr		Array of values used to initialize the vector's scalar components.
		MUU_CONSTRAINED_TEMPLATE((Dimensions > N || (Dimensions == N && !build::supports_constexpr_bit_cast)), size_t N)
		MUU_NODISCARD_CTOR
		explicit constexpr vector(const scalar_type (&arr)[N]) noexcept
			: base{ impl::array_cast_tag{}, std::make_index_sequence<N>{}, arr }
		{}

		/// \cond

		MUU_CONSTRAINED_TEMPLATE((Dimensions == N && build::supports_constexpr_bit_cast), size_t N)
		MUU_NODISCARD_CTOR
		explicit constexpr vector(const scalar_type (&arr)[N]) noexcept //
			: base{ muu::bit_cast<base>(arr) }
		{}

		/// \endcond

		/// \brief Constructs a vector from a raw array of scalars.
		/// \details	Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \tparam T		A type convertible to #scalar_type.
		/// \tparam N		The number of elements in the array.
		/// \param	arr		Array of values used to initialize the vector's scalar components.
		MUU_CONSTRAINED_TEMPLATE((Dimensions >= N						  //
								  && all_convertible_to<Scalar, const T&> //
								  && !std::is_same_v<remove_cv<T>, Scalar>),
								 typename T,
								 size_t N)
		MUU_NODISCARD_CTOR
		explicit constexpr vector(const T (&arr)[N]) noexcept
			: base{ impl::array_cast_tag{}, std::make_index_sequence<N>{}, arr }
		{}

		/// \brief Constructs a vector from a std::array of scalars.
		/// \details	Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \tparam N		The number of elements in the array.
		/// \param	arr		Array of values used to initialize the vector's scalar components.
		MUU_CONSTRAINED_TEMPLATE((Dimensions > N || (Dimensions == N && !build::supports_constexpr_bit_cast)), size_t N)
		MUU_NODISCARD_CTOR
		explicit constexpr vector(const std::array<scalar_type, N>& arr) noexcept
			: base{ impl::array_cast_tag{}, std::make_index_sequence<N>{}, arr }
		{}

		/// \cond

		MUU_CONSTRAINED_TEMPLATE((Dimensions == N && build::supports_constexpr_bit_cast), size_t N)
		MUU_NODISCARD_CTOR
		explicit constexpr vector(const std::array<scalar_type, N>& arr) noexcept //
			: base{ muu::bit_cast<base>(arr) }
		{}

		/// \endcond

		/// \brief Constructs a vector from a std::array of scalars.
		/// \details	Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \tparam T		A type convertible to #scalar_type.
		/// \tparam N		The number of elements in the array.
		/// \param	arr		Array of values used to initialize the vector's scalar components.
		MUU_CONSTRAINED_TEMPLATE((Dimensions >= N						  //
								  && all_convertible_to<Scalar, const T&> //
								  && !std::is_same_v<remove_cv<T>, Scalar>),
								 typename T,
								 size_t N)
		MUU_NODISCARD_CTOR
		explicit constexpr vector(const std::array<T, N>& arr) noexcept
			: base{ impl::array_cast_tag{}, std::make_index_sequence<N>{}, arr }
		{}

		/// \brief Constructs a vector from any tuple-like or implicitly bit-castable type.
		///
		/// \tparam T	A tuple-like or bit-castable type.
		///
		/// \details	Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \note		This constructor is implicit when given an implicitly bit-castable type, `explicit` otherwise.
		///
		/// \see muu::allow_implicit_bit_cast
		MUU_CONSTRAINED_TEMPLATE((!impl::is_vector_<T>							//
								  && (std::is_class_v<T> || std::is_union_v<T>) //
								  &&is_tuple_like<T>							//
								  && Dimensions >= tuple_size<T>				//
								  && !allow_implicit_bit_cast<T, vector>),
								 typename T)
		MUU_NODISCARD_CTOR
		explicit constexpr vector(const T& tuple_or_bitcastable) noexcept
			: base{ impl::tuple_cast_tag{}, std::make_index_sequence<tuple_size<T>>{}, tuple_or_bitcastable }
		{}

		/// \cond

		MUU_CONSTRAINED_TEMPLATE((!impl::is_vector_<T>							//
								  && (std::is_class_v<T> || std::is_union_v<T>) //
								  &&allow_implicit_bit_cast<T, vector>			//
								  && (!is_tuple_like<T> || (tuple_size<T> > Dimensions))),
								 typename T)
		MUU_NODISCARD_CTOR
		/*implicit*/
		constexpr vector(const T& bitcastable) noexcept //
			: base{ muu::bit_cast<base>(bitcastable) }
		{
			static_assert(sizeof(T) == sizeof(base), "Bit-castable types must be the same size");
			static_assert(std::is_trivially_copyable_v<T>, "Bit-castable types must be trivially-copyable");
		}

		/// \endcond

		/// \brief Enlarging/truncating/converting constructor.
		/// \details Copies source vector's scalar components, casting if necessary:
		/// \cpp
		/// vector<float, 3> xyz  = { 1, 2, 3 };
		/// vector<float, 2> xy   = { xyz }; // { 1, 2 }
		/// vector<float, 4> xyzw = { xyz }; // { 1, 2, 3, 0 }
		/// auto rect   = vector{ point1, point2 }; // { 1, 2, 10, 15 }
		/// \ecpp
		/// Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \tparam	S		Source vector's #scalar_type.
		/// \tparam	D		Source vector's dimensions.
		/// \param 	vec		Source vector.
		template <typename S, size_t D>
		MUU_NODISCARD_CTOR
		explicit constexpr vector(const vector<S, D>& vec) noexcept
			: base{ impl::tuple_cast_tag{}, std::make_index_sequence<(D < Dimensions ? D : Dimensions)>{}, vec }
		{}

		/// \brief	Concatenating constructor.
		/// \details Copies the scalar components from vector 1 and then vector 2 contiguously into the new vector:
		/// \cpp
		/// vector<float, 2> point1 = { 1, 2 };
		/// vector<float, 2> point2 = { 10, 15 };
		/// vector<float, 4> rect   = { point1, point2 }; // { 1, 2, 10, 15 }
		/// \ecpp
		/// Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \tparam	S1  	Vector 1's #scalar_type.
		/// \tparam	D1		Vector 1's #dimensions.
		/// \tparam	S2  	Vector 2's #scalar_type.
		/// \tparam	D2		Vector 2's #dimensions.
		/// \param 	vec1	Vector 1.
		/// \param 	vec2	Vector 2.
		MUU_CONSTRAINED_TEMPLATE(Dimensions >= D1 + D2, typename S1, size_t D1, typename S2, size_t D2)
		MUU_NODISCARD_CTOR
		constexpr vector(const vector<S1, D1>& vec1, const vector<S2, D2>& vec2) noexcept
			: base{ impl::tuple_concat_tag{},
					std::make_index_sequence<D1>{},
					vec1,
					std::make_index_sequence<D2>{},
					vec2 }
		{}

		/// \brief	Appending constructor.
		/// \details Copies the scalar components from the vector and then the
		/// 		 list of scalars contiguously into the new vector:
		/// \cpp
		/// vector<float, 2> point = { 1, 2 };
		/// vector<float, 4> rect  = { point, 10, 15 }; // { 1, 2, 10, 15 }
		/// \ecpp
		/// Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \tparam	S	  	Vector's #scalar_type.
		/// \tparam	D		Vector's #dimensions.
		/// \tparam T		Types convertible to #scalar_type.
		/// \param 	vec		A vector.
		/// \param 	vals	Scalar values.
		MUU_CONSTRAINED_TEMPLATE((Dimensions >= D + sizeof...(T) //
								  && all_convertible_to<Scalar, const T&...>),
								 typename S,
								 size_t D,
								 typename... T)
		MUU_NODISCARD_CTOR
		constexpr vector(const vector<S, D>& vec, const T&... vals) noexcept
			: base{ impl::tuple_concat_tag{}, std::make_index_sequence<D>{}, vec, vals... }
		{}

		/// \brief Constructs a vector from a pointer to scalars.
		/// \details	Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \tparam T			Type convertible to #scalar_type.
		/// \param	vals		Pointer to values to copy.
		/// \param	num			Number of values to copy.
		MUU_CONSTRAINED_TEMPLATE((all_convertible_to<Scalar, const T&>), typename T)
		MUU_NODISCARD_CTOR
		MUU_ATTR(nonnull)
		vector(const T* vals, size_t num) noexcept
		{
			MUU_ASSUME(vals != nullptr);
			MUU_ASSUME(num <= Dimensions);

			if constexpr (std::is_same_v<remove_cv<T>, Scalar>)
				MUU_MEMCPY(this, vals, sizeof(scalar_type) * num);
			else
			{
				for (size_t i = 0; i < num; i++)
					operator[](i) = static_cast<scalar_type>(vals[i]);
			}

			if (num < Dimensions)
				MUU_MEMSET(pointer_cast<std::byte*>(this) + num * sizeof(scalar_type),
						   0,
						   (Dimensions - num) * sizeof(scalar_type));
		}

		/// \brief Constructs a vector from a pointer to scalars.
		///
		/// \tparam T			Type convertible to #scalar_type.
		/// \param	vals		Pointer to values to copy.
		MUU_CONSTRAINED_TEMPLATE((all_convertible_to<Scalar, const T&>), typename T)
		MUU_NODISCARD_CTOR
		MUU_ATTR(nonnull)
		explicit constexpr vector(const T* MUU_HIDDEN(const&) vals) noexcept
			: base{ impl::array_cast_tag{}, std::make_index_sequence<Dimensions>{}, vals }
		{}

		/// \brief Constructs a vector from a statically-sized muu::span.
		/// \details	Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \tparam T			Type convertible to #scalar_type.
		/// \tparam N			The number of elements covered by the span.
		/// \param	vals		A span representing the values to copy.
		MUU_CONSTRAINED_TEMPLATE((Dimensions >= N						  //
								  && all_convertible_to<Scalar, const T&> //
								  && N != dynamic_extent),
								 typename T,
								 size_t N)
		MUU_NODISCARD_CTOR
		explicit constexpr vector(const muu::span<T, N>& vals) noexcept
			: base{ impl::array_cast_tag{}, std::make_index_sequence<N>{}, vals }
		{}

		/// \brief Constructs a vector from a dynamically-sized muu::span.
		/// \details			Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \tparam T			Type convertible to #scalar_type.
		/// \param	vals		A span representing the values to copy.
		MUU_CONSTRAINED_TEMPLATE((all_convertible_to<Scalar, const T&>), typename T)
		MUU_NODISCARD_CTOR
		explicit constexpr vector(const muu::span<T>& vals) noexcept //
			: vector{ vals.data(), vals.size() }
		{}

#endif // constructors

#if 1	// scalar component accessors --------------------------------------------------------------------------------
		/// \name Scalar accessors
		/// @{

	  private:
		/// \cond

		template <size_t Dimension, typename T>
		MUU_PURE_INLINE_GETTER
		static constexpr auto& do_get(T& vec) noexcept
		{
			static_assert(Dimension < Dimensions, "Dimension index out of range");

			if constexpr (Dimensions <= 4)
			{
				if constexpr (Dimension == 0)
					return vec.x;
				if constexpr (Dimension == 1)
					return vec.y;
				if constexpr (Dimension == 2)
					return vec.z;
				if constexpr (Dimension == 3)
					return vec.w;
			}
			else
				return vec.values[Dimension];
		}

		template <typename T>
		MUU_PURE_GETTER
		static constexpr auto& do_array_operator(T& vec, size_t dim) noexcept
		{
			MUU_ASSUME(dim < Dimensions);

			if constexpr (Dimensions <= 4)
			{
				if (/*!build::supports_is_constant_evaluated ||*/ is_constant_evaluated())
				{
					switch (dim)
					{
						case 0: return vec.x;
						case 1:
							if constexpr (Dimensions > 1)
								return vec.y;
							else
								MUU_UNREACHABLE;
						case 2:
							if constexpr (Dimensions > 2)
								return vec.z;
							else
								MUU_UNREACHABLE;
						case 3:
							if constexpr (Dimensions > 3)
								return vec.w;
							else
								MUU_UNREACHABLE;
						default: MUU_UNREACHABLE;
					}
				}
				else
					return *(&vec.x + dim);
			}
			else
				return vec.values[dim];
		}

		/// \endcond

	  public:
		/// \brief Gets a reference to the scalar component at a specific index.
		///
		/// \tparam Dimension The index of the dimension to retrieve, where X == 0, Y == 1, etc.
		///
		/// \return  A reference to the selected scalar component.
		template <size_t Dimension>
		MUU_PURE_INLINE_GETTER
		MUU_ATTR(flatten)
		constexpr const scalar_type& get() const noexcept
		{
			return do_get<Dimension>(*this);
		}

		/// \brief Gets a reference to the scalar component at a specific index.
		///
		/// \tparam Dimension The index of the dimension to retrieve, where X == 0, Y == 1, etc.
		///
		/// \return  A reference to the selected scalar component.
		template <size_t Dimension>
		MUU_PURE_INLINE_GETTER
		MUU_ATTR(flatten)
		constexpr scalar_type& get() noexcept
		{
			return do_get<Dimension>(*this);
		}

		/// \brief Gets a reference to the Nth scalar component.
		///
		/// \tparam dim The index of the dimension to retrieve, where X == 0, Y == 1, etc.
		///
		/// \return  A reference to the selected scalar component.
		MUU_PURE_INLINE_GETTER
		constexpr const scalar_type& operator[](size_t dim) const noexcept
		{
			return do_array_operator(*this, dim);
		}

		/// \brief Gets a reference to the Nth scalar component.
		///
		/// \tparam dim The index of the dimension to retrieve, where X == 0, Y == 1, etc.
		///
		/// \return  A reference to the selected scalar component.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type& operator[](size_t dim) noexcept
		{
			return do_array_operator(*this, dim);
		}

		/// \brief Returns a pointer to the first scalar component in the vector.
		MUU_PURE_INLINE_GETTER
		MUU_ATTR(flatten)
		constexpr const scalar_type* data() const noexcept
		{
			return &do_get<0>(*this);
		}

		/// \brief Returns a pointer to the first scalar component in the vector.
		MUU_PURE_INLINE_GETTER
		MUU_ATTR(flatten)
		constexpr scalar_type* data() noexcept
		{
			return &do_get<0>(*this);
		}

		/// @}
#endif // scalar component accessors

#if 1 // equality (exact) ----------------------------------------------------------------------------------------------
		/// \name Equality (exact)
		/// @{

		/// \brief		Returns true if two vectors are exactly equal.
		///
		/// \remarks	This is a componentwise exact equality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point vectors, use #approx_equal().
		MUU_CONSTRAINED_TEMPLATE((!MUU_HAS_VECTORCALL
								  || impl::pass_vectorcall_by_reference<vector, vector<T, Dimensions>>),
								 typename T)
		MUU_PURE_GETTER
		friend constexpr bool operator==(const vector& lhs, const vector<T, dimensions>& rhs) noexcept
		{
			// clang-format off

			if constexpr (std::is_same_v<scalar_type, T>)
			{
				#define VEC_FUNC(member) lhs.member == rhs.member
				COMPONENTWISE_AND(VEC_FUNC);
				#undef VEC_FUNC
			}
			else
			{
				using type = impl::equality_check_type<scalar_type, T>;

				#define VEC_FUNC(member) static_cast<type>(lhs.member) == static_cast<type>(rhs.member)
				COMPONENTWISE_AND(VEC_FUNC);
				#undef VEC_FUNC
			}

			// clang-format on
		}

	#if MUU_HAS_VECTORCALL

		MUU_CONSTRAINED_TEMPLATE((impl::pass_vectorcall_by_value<vector, vector<T, Dimensions>>), typename T)
		MUU_CONST_GETTER
		friend constexpr bool MUU_VECTORCALL operator==(vector lhs, vector<T, dimensions> rhs) noexcept
		{
			// clang-format off

			if constexpr (std::is_same_v<scalar_type, T>)
			{
				#define VEC_FUNC(member) lhs.member == rhs.member
				COMPONENTWISE_AND(VEC_FUNC);
				#undef VEC_FUNC
			}
			else
			{
				using type = impl::equality_check_type<scalar_type, T>;

				#define VEC_FUNC(member) static_cast<type>(lhs.member) == static_cast<type>(rhs.member)
				COMPONENTWISE_AND(VEC_FUNC);
				#undef VEC_FUNC
			}

			// clang-format on
		}

	#endif // MUU_HAS_VECTORCALL

		/// \brief	Returns true if two vectors are not exactly equal.
		///
		/// \remarks	This is a componentwise exact inequality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point vectors, use #approx_equal().
		MUU_CONSTRAINED_TEMPLATE((!MUU_HAS_VECTORCALL
								  || impl::pass_vectorcall_by_reference<vector, vector<T, Dimensions>>),
								 typename T)
		MUU_PURE_GETTER
		friend constexpr bool operator!=(const vector& lhs, const vector<T, dimensions>& rhs) noexcept
		{
			return !(lhs == rhs);
		}

	#if MUU_HAS_VECTORCALL

		MUU_CONSTRAINED_TEMPLATE((impl::pass_vectorcall_by_value<vector, vector<T, Dimensions>>), typename T)
		MUU_CONST_INLINE_GETTER
		friend constexpr bool MUU_VECTORCALL operator!=(vector lhs, vector<T, dimensions> rhs) noexcept
		{
			return !(lhs == rhs);
		}

	#endif // MUU_HAS_VECTORCALL

		/// \brief	Returns true if all the scalar components of a vector are exactly zero.
		///
		/// \remarks	This is a componentwise exact equality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point vectors, use #approx_zero().
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL zero(MUU_VC_PARAM(vector) v) noexcept
		{
			// clang-format off

			#define VEC_FUNC(member) v.member == scalar_type{}
			COMPONENTWISE_AND(VEC_FUNC);
			#undef VEC_FUNC

			// clang-format on
		}

		/// \brief	Returns true if all the scalar components of the vector are exactly zero.
		///
		/// \remarks	This is a componentwise exact equality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point vectors, use #approx_zero().
		MUU_PURE_INLINE_GETTER
		constexpr bool zero() const noexcept
		{
			return zero(*this);
		}

		/// \brief	Returns true if any of the scalar components of a vector are infinity or NaN.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL infinity_or_nan(MUU_VC_PARAM(vector) v) noexcept
		{
			// clang-format off

			if constexpr (is_floating_point<scalar_type>)
			{
				#define VEC_FUNC(member) muu::infinity_or_nan(v.member)
				COMPONENTWISE_OR(VEC_FUNC);
				#undef VEC_FUNC
			}
			else
			{
				MUU_UNUSED(v);
				return false;
			}

			// clang-format on
		}

		/// \brief	Returns true if any of the scalar components of the vector are infinity or NaN.
		MUU_PURE_INLINE_GETTER
		constexpr bool infinity_or_nan() const noexcept
		{
			if constexpr (is_floating_point<scalar_type>)
				return infinity_or_nan(*this);
			else
				return false;
		}

		/// @}
#endif // equality (exact)

#if 1 // equality (approx) -------------------------------------------------------------------------------------
		/// \name Equality (approximate)
		/// @{

		/// \brief	Returns true if two vectors are approximately equal.
		///
		/// \availability		This function is only available when at least one of #scalar_type
		///						and `T` is a floating-point type.
		MUU_CONSTRAINED_TEMPLATE((any_floating_point<Scalar, T> //
								  && (!MUU_HAS_VECTORCALL
									  || impl::pass_vectorcall_by_reference<vector, vector<T, Dimensions>>)),
								 typename T)
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_equal(
			const vector& v1,
			const vector<T, dimensions>& v2,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) noexcept
		{
			// clang-format off

			if constexpr (std::is_same_v<scalar_type, T>)
			{
				#define VEC_FUNC(member) muu::approx_equal(v1.member, v2.member, epsilon)
				COMPONENTWISE_AND(VEC_FUNC);
				#undef VEC_FUNC
			}
			else
			{
				using type = impl::equality_check_type<scalar_type, T>;

				#define VEC_FUNC(member) muu::approx_equal(static_cast<type>(v1.member),\
															static_cast<type>(v2.member),\
															static_cast<type>(epsilon))
				COMPONENTWISE_AND(VEC_FUNC);
				#undef VEC_FUNC
			}

			// clang-format on
		}

	#if MUU_HAS_VECTORCALL

		MUU_CONSTRAINED_TEMPLATE((any_floating_point<Scalar, T> //
								  && (impl::pass_vectorcall_by_value<vector, vector<T, Dimensions>>)),
								 typename T)
		MUU_CONST_GETTER
		static constexpr bool MUU_VECTORCALL approx_equal(
			vector v1,
			vector<T, dimensions> v2,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) noexcept
		{
			// clang-format off

			if constexpr (std::is_same_v<scalar_type, T>)
			{
				#define VEC_FUNC(member) muu::approx_equal(v1.member, v2.member, epsilon)
				COMPONENTWISE_AND(VEC_FUNC);
				#undef VEC_FUNC
			}
			else
			{
				using type = impl::equality_check_type<scalar_type, T>;

				#define VEC_FUNC(member) muu::approx_equal(static_cast<type>(v1.member),\
															static_cast<type>(v2.member),\
															static_cast<type>(epsilon))
				COMPONENTWISE_AND(VEC_FUNC);
				#undef VEC_FUNC
			}

			// clang-format on
		}

	#endif // MUU_HAS_VECTORCALL

		/// \brief	Returns true if the vector is approximately equal to another.
		///
		/// \availability		This function is only available when at least one of #scalar_type
		///						and `T` is a floating-point type.
		MUU_CONSTRAINED_TEMPLATE((any_floating_point<Scalar, T> //
								  && (!MUU_HAS_VECTORCALL
									  || impl::pass_vectorcall_by_reference<vector<T, Dimensions>>)),
								 typename T)
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL approx_equal(
			const vector<T, dimensions>& v,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) const noexcept
		{
			return approx_equal(*this, v, epsilon);
		}

	#if MUU_HAS_VECTORCALL

		MUU_CONSTRAINED_TEMPLATE((any_floating_point<Scalar, T> //
								  && impl::pass_vectorcall_by_value<vector<T, Dimensions>>),
								 typename T)
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL approx_equal(
			vector<T, dimensions> v,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) const noexcept
		{
			return approx_equal(*this, v, epsilon);
		}

	#endif // MUU_HAS_VECTORCALL

		/// \brief	Returns true if all the scalar components in a vector are approximately equal to zero.
		///
		/// \availability		This function is only available when #scalar_type is a floating-point type.
		MUU_HIDDEN_CONSTRAINT(is_floating_point<T>, typename T = Scalar)
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_zero(MUU_VC_PARAM(vector) v,
														 scalar_type epsilon = default_epsilon<scalar_type>) noexcept
		{
			// clang-format off

			#define VEC_FUNC(member) muu::approx_zero(v.member, epsilon)
			COMPONENTWISE_AND(VEC_FUNC);
			#undef VEC_FUNC

			// clang-format on
		}

		/// \brief	Returns true if all the scalar components in the vector are approximately equal to zero.
		///
		/// \availability		This function is only available when #scalar_type is a floating-point type.
		MUU_HIDDEN_CONSTRAINT(is_floating_point<T>, typename T = Scalar)
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL approx_zero(scalar_type epsilon = default_epsilon<scalar_type>) const noexcept
		{
			return approx_zero(*this, epsilon);
		}

		/// @}
#endif // equality (approx)

#if 1 // length and distance ---------------------------------------------------------------------------------------
		/// \name Length and Distance
		/// @{

		/// \brief	Returns the squared length (magnitude) of a vector.
		MUU_PURE_GETTER
		static constexpr delta_scalar_type MUU_VECTORCALL length_squared(MUU_VC_PARAM(vector) v) noexcept
		{
			if constexpr (delta_requires_promotion)
			{
				return static_cast<delta_scalar_type>(promoted_delta_vec::length_squared(promoted_delta_vec{ v }));
			}
			else
			{
				// clang-format off

				#define VEC_FUNC(member) v.member * v.member
				COMPONENTWISE_ACCUMULATE(VEC_FUNC, +);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief	Returns the squared length (magnitude) of the vector.
		MUU_PURE_INLINE_GETTER
		constexpr delta_scalar_type length_squared() const noexcept
		{
			return length_squared(*this);
		}

		/// \brief	Returns the length (magnitude) of a vector.
		MUU_PURE_GETTER
		static constexpr delta_scalar_type MUU_VECTORCALL length(MUU_VC_PARAM(vector) v) noexcept
		{
			if constexpr (Dimensions == 1)
			{
				return static_cast<delta_scalar_type>(v.x);
			}
			else if constexpr (delta_requires_promotion)
			{
				return static_cast<delta_scalar_type>(promoted_delta_vec::length(promoted_delta_vec{ v }));
			}
			else
			{
				return muu::sqrt(length_squared(v));
			}
		}

		/// \brief	Returns the length (magnitude) of a vector.
		///
		/// \warning	This function is implemented such that it is _always_ available at compile time,
		///				arriving at the result using very slow iterative machinery. Do not use it at runtime!
		MUU_PURE_GETTER
		MUU_CONSTEVAL
		static delta_scalar_type MUU_VECTORCALL consteval_length(MUU_VC_PARAM(vector) v) noexcept
		{
			if constexpr (Dimensions == 1)
			{
				return static_cast<delta_scalar_type>(v.x);
			}
			else if constexpr (delta_requires_promotion)
			{
				return static_cast<delta_scalar_type>(promoted_delta_vec::consteval_length(promoted_delta_vec{ v }));
			}
			else
			{
				return muu::consteval_sqrt(length_squared(v));
			}
		}

		/// \brief	Returns the length (magnitude) of the vector.
		MUU_PURE_INLINE_GETTER
		constexpr delta_scalar_type length() const noexcept
		{
			return length(*this);
		}

		/// \brief	Returns the squared distance between two points.
		MUU_PURE_GETTER
		static constexpr delta_scalar_type MUU_VECTORCALL distance_squared(MUU_VC_PARAM(vector) p1,
																		   MUU_VC_PARAM(vector) p2) noexcept
		{
			if constexpr (delta_requires_promotion)
			{
				return static_cast<delta_scalar_type>(
					promoted_delta_vec::distance_squared(promoted_delta_vec{ p1 }, promoted_delta_vec{ p2 }));
			}
			else
			{
				return length_squared(p2 - p1);
			}
		}

		/// \brief	Returns the squared distance between this and another point vector.
		MUU_PURE_INLINE_GETTER
		constexpr delta_scalar_type MUU_VECTORCALL distance_squared(MUU_VC_PARAM(vector) p) const noexcept
		{
			return distance_squared(*this, p);
		}

		/// \brief	Returns the distance between two points.
		MUU_PURE_GETTER
		static constexpr delta_scalar_type MUU_VECTORCALL distance(MUU_VC_PARAM(vector) p1,
																   MUU_VC_PARAM(vector) p2) noexcept
		{
			if constexpr (delta_requires_promotion)
			{
				return static_cast<delta_scalar_type>(
					promoted_delta_vec::distance(promoted_delta_vec{ p1 }, promoted_delta_vec{ p2 }));
			}
			else
			{
				return muu::sqrt(distance_squared(p1, p2));
			}
		}

		/// \brief	Returns the distance between two points.
		///
		/// \warning	This function is implemented such that it is _always_ available at compile time,
		///				arriving at the result using very slow iterative machinery. Do not use it at runtime!
		MUU_PURE_GETTER
		MUU_CONSTEVAL
		static delta_scalar_type MUU_VECTORCALL consteval_distance(MUU_VC_PARAM(vector) p1,
																   MUU_VC_PARAM(vector) p2) noexcept
		{
			if constexpr (delta_requires_promotion)
			{
				return static_cast<delta_scalar_type>(
					promoted_delta_vec::consteval_distance(promoted_delta_vec{ p1 }, promoted_delta_vec{ p2 }));
			}
			else
			{
				return muu::consteval_sqrt(distance_squared(p1, p2));
			}
		}

		/// \brief	Returns the distance between this and another point vector.
		MUU_PURE_INLINE_GETTER
		constexpr delta_scalar_type MUU_VECTORCALL distance(MUU_VC_PARAM(vector) p) const noexcept
		{
			return distance(*this, p);
		}

		/// @}
#endif // length and distance

#if 1 // dot product ---------------------------------------------------------------------------------------------------
		/// \name Dot product
		/// @{

		/// \brief	Returns the dot product of two vectors.
		MUU_PURE_GETTER
		static constexpr product_scalar_type MUU_VECTORCALL dot(MUU_VC_PARAM(vector) v1,
																MUU_VC_PARAM(vector) v2) noexcept
		{
			if constexpr (product_requires_promotion)
			{
				return static_cast<product_scalar_type>(
					promoted_product_vec::dot(promoted_product_vec{ v1 }, promoted_product_vec{ v2 }));
			}
			else
			{
				// clang-format off

				#define VEC_FUNC(member) v1.member * v2.member
				COMPONENTWISE_ACCUMULATE(VEC_FUNC, +);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief	Returns the dot product of this and another vector.
		MUU_PURE_INLINE_GETTER
		constexpr product_scalar_type MUU_VECTORCALL dot(MUU_VC_PARAM(vector) v) const noexcept
		{
			return dot(*this, v);
		}

		/// \brief	Returns the dot product of a vector and a principal axis.
		template <size_t Dimension>
		MUU_PURE_INLINE_GETTER
		static constexpr product_scalar_type MUU_VECTORCALL dot(MUU_VC_PARAM(vector) v,
																index_tag<Dimension> /*axis*/) noexcept
		{
			static_assert(Dimension < Dimensions, "Dimension index out of range");

			return v.template get<Dimension>();
		}

		/// \brief	Returns the dot product of a vector and a principal axis.
		template <size_t Dimension>
		MUU_PURE_INLINE_GETTER
		static constexpr product_scalar_type MUU_VECTORCALL dot(index_tag<Dimension> /*axis*/,
																MUU_VC_PARAM(vector) v) noexcept
		{
			static_assert(Dimension < Dimensions, "Dimension index out of range");

			return v.template get<Dimension>();
		}

		/// \brief	Returns the dot product of the vector and a principal axis.
		template <size_t Dimension>
		MUU_PURE_INLINE_GETTER
		constexpr product_scalar_type MUU_VECTORCALL dot(index_tag<Dimension> /*axis*/) const noexcept
		{
			static_assert(Dimension < Dimensions, "Dimension index out of range");

			return get<Dimension>();
		}

		/// @}
#endif // dot product

#if 1 // cross product // ----------------------------------------------------------------------------------------------
		/// \name Cross product
		/// \availability		These functions are only available when #dimensions == 3.
		/// @{

		/// \brief	Returns the cross product of two vectors.
		///
		/// \availability		This function is only available when #dimensions == 3.
		MUU_HIDDEN_CONSTRAINT(Dims == 3, size_t Dims = Dimensions)
		MUU_PURE_GETTER
		static constexpr vector<product_scalar_type, 3> MUU_VECTORCALL cross(MUU_VC_PARAM(vector) v1,
																			 MUU_VC_PARAM(vector) v2) noexcept
		{
			if constexpr (product_requires_promotion)
			{
				return vector<product_scalar_type, 3>{ promoted_product_vec::cross(promoted_product_vec{ v1 },
																				   promoted_product_vec{ v2 }) };
			}
			else
			{
				MUU_FMA_BLOCK;

				return { v1.y * v2.z - v1.z * v2.y, //
						 v1.z * v2.x - v1.x * v2.z,
						 v1.x * v2.y - v1.y * v2.x };
			}
		}

		/// \brief	Returns the cross product of a vector and a principal axis.
		///
		/// \availability		This function is only available when #dimensions == 3.
		MUU_CONSTRAINED_TEMPLATE(Dimension < 3 && Dimensions == 3, size_t Dimension)
		MUU_PURE_INLINE_GETTER
		static constexpr vector<product_scalar_type, 3> MUU_VECTORCALL cross(MUU_VC_PARAM(vector) v,
																			 index_tag<Dimension> /*axis*/) noexcept
		{
			static_assert(Dimension < Dimensions, "Dimension index out of range");

			// x axis
			if constexpr (Dimension == 0)
			{
				return { product_scalar_type{}, v.z, -v.y };
			}

			// y axis
			if constexpr (Dimension == 1)
			{
				return { -v.z, product_scalar_type{}, v.x };
			}

			// z axis
			if constexpr (Dimension == 2)
			{
				return { v.y, -v.x, product_scalar_type{} };
			}
		}

		/// \brief	Returns the cross product of a principal axis and a vector.
		///
		/// \availability		This function is only available when #dimensions == 3.
		MUU_CONSTRAINED_TEMPLATE(Dimension < 3 && Dimensions == 3, size_t Dimension)
		MUU_PURE_INLINE_GETTER
		static constexpr vector<product_scalar_type, 3> MUU_VECTORCALL cross(index_tag<Dimension> /*axis*/,
																			 MUU_VC_PARAM(vector) v) noexcept
		{
			static_assert(Dimension < Dimensions, "Dimension index out of range");

			// x axis
			if constexpr (Dimension == 0)
			{
				return { product_scalar_type{}, -v.z, v.y };
			}

			// y axis
			if constexpr (Dimension == 1)
			{
				return { v.z, product_scalar_type{}, -v.x };
			}

			// z axis
			if constexpr (Dimension == 2)
			{
				return { -v.y, v.x, product_scalar_type{} };
			}
		}

		/// \brief	Returns the cross product of this vector and another.
		///
		/// \availability		This function is only available when #dimensions == 3.
		MUU_HIDDEN_CONSTRAINT(Dims == 3, size_t Dims = Dimensions)
		MUU_PURE_INLINE_GETTER
		constexpr vector<product_scalar_type, 3> MUU_VECTORCALL cross(MUU_VC_PARAM(vector) v) const noexcept
		{
			return cross(*this, v);
		}

		/// \brief	Returns the cross product of this vector and a principal axis.
		///
		/// \availability		This function is only available when #dimensions == 3.
		MUU_CONSTRAINED_TEMPLATE(Dimension < 3 && Dimensions == 3, size_t Dimension)
		MUU_PURE_INLINE_GETTER
		constexpr vector<product_scalar_type, 3> MUU_VECTORCALL cross(index_tag<Dimension> /*axis*/) const noexcept
		{
			return cross(*this, index_tag<Dimension>{});
		}

		/// \brief	Returns a vector orthogonal to another.
		///
		/// \availability		This function is only available when #dimensions == 3.
		MUU_HIDDEN_CONSTRAINT(Dims == 3, size_t Dims = Dimensions)
		MUU_PURE_GETTER
		static constexpr vector<product_scalar_type, 3> orthogonal(MUU_VC_PARAM(vector) v) noexcept
		{
			const auto x = muu::abs(v.x);
			const auto y = muu::abs(v.y);
			const auto z = muu::abs(v.z);

			return cross(v,
						 x < y ? (x < z ? constants::x_axis : constants::z_axis)
							   : (y < z ? constants::y_axis : constants::z_axis));
		}

		/// \brief	Returns a vector orthogonal to this one.
		///
		/// \availability		This function is only available when #dimensions == 3.
		MUU_HIDDEN_CONSTRAINT(Dims == 3, size_t Dims = Dimensions)
		MUU_PURE_INLINE_GETTER
		constexpr vector<product_scalar_type, 3> orthogonal() const noexcept
		{
			return orthogonal(*this);
		}

		/// @}
#endif // cross product

#if 1 // addition --------------------------------------------------------------------------------------------------
		/// \name Addition
		/// @{

		/// \brief Returns the componentwise addition of two vectors.
		MUU_PURE_GETTER
		friend constexpr vector MUU_VECTORCALL operator+(MUU_VC_PARAM(vector) lhs, MUU_VC_PARAM(vector) rhs) noexcept
		{
			if constexpr (is_small_float)
			{
				return vector{ promoted_vec{ lhs } + promoted_vec{ rhs } };
			}
			else
			{
				// clang-format off

				#define VEC_FUNC(member) lhs.member + rhs.member
				COMPONENTWISE_CONSTRUCT(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief Componentwise adds another vector to this one.
		constexpr vector& MUU_VECTORCALL operator+=(MUU_VC_PARAM(vector) rhs) noexcept
		{
			if constexpr (is_small_float)
			{
				return *this = vector{ promoted_vec{ *this } + promoted_vec{ rhs } };
			}
			else
			{
				// clang-format off

				#define VEC_FUNC(member) base::member + rhs.member
				COMPONENTWISE_ASSIGN(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief Returns a componentwise copy of a vector.
		MUU_PURE_INLINE_GETTER
		constexpr vector operator+() const noexcept
		{
			return *this;
		}

		/// \brief Returns the sum of all the scalar components in a vector.
		MUU_PURE_GETTER
		static constexpr scalar_type MUU_VECTORCALL sum(MUU_VC_PARAM(vector) v) noexcept
		{
			if constexpr (dimensions == 1)
			{
				return v.x;
			}
			else
			{
				using scalar_sum_type  = decltype(scalar_type{} + scalar_type{});
				using float_arith_type = promoted_scalar;
				using integer_arith_type =
					std::conditional_t<std::is_same_v<scalar_sum_type, scalar_type>, scalar_type, scalar_sum_type>;
				using arith_type = std::conditional_t<is_integral<scalar_type>, integer_arith_type, float_arith_type>;

				if constexpr (!std::is_same_v<arith_type, scalar_type>)
				{
					using arith_vec = vector<arith_type, dimensions>;
					return static_cast<scalar_type>(arith_vec::sum(arith_vec{ v }));
				}
				else
				{
					// clang-format off

					#define VEC_FUNC(member) v.member
					COMPONENTWISE_ACCUMULATE(VEC_FUNC, +);
					#undef VEC_FUNC

					// clang-format on
				}
			}
		}

		/// \brief Returns the sum of all the scalar components in the vector.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type MUU_VECTORCALL sum() const noexcept
		{
			return sum(*this);
		}

		/// @}
#endif // addition

#if 1 // subtraction -----------------------------------------------------------------------------------------------
		/// \name Subtraction
		/// @{

		/// \brief Returns the componentwise subtraction of one vector from another.
		MUU_PURE_GETTER
		friend constexpr vector MUU_VECTORCALL operator-(MUU_VC_PARAM(vector) lhs, MUU_VC_PARAM(vector) rhs) noexcept
		{
			if constexpr (is_small_float)
			{
				return vector{ promoted_vec{ lhs } - promoted_vec{ rhs } };
			}
			else
			{
				// clang-format off

				#define VEC_FUNC(member) lhs.member - rhs.member
				COMPONENTWISE_CONSTRUCT(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief Componentwise subtracts another vector from this one.
		constexpr vector& MUU_VECTORCALL operator-=(MUU_VC_PARAM(vector) rhs) noexcept
		{
			if constexpr (is_small_float)
			{
				return *this = vector{ promoted_vec{ *this } - promoted_vec{ rhs } };
			}
			else
			{
				// clang-format off

				#define VEC_FUNC(member) base::member - rhs.member
				COMPONENTWISE_ASSIGN(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief Returns the componentwise negation of a vector.
		MUU_HIDDEN_CONSTRAINT(is_signed<T>, typename T = Scalar)
		MUU_PURE_GETTER
		constexpr vector operator-() const noexcept
		{
			// clang-format off

			#define VEC_FUNC(member) -base::member
			COMPONENTWISE_CONSTRUCT(VEC_FUNC);
			#undef VEC_FUNC

			// clang-format on
		}

		/// @}
#endif // subtraction

#if 1 // multiplication -------------------------------------------------------------------------------------------
		/// \name Multiplication
		/// @{

		/// \brief Returns the componentwise multiplication of two vectors.
		MUU_PURE_GETTER
		friend constexpr vector MUU_VECTORCALL operator*(MUU_VC_PARAM(vector) lhs, MUU_VC_PARAM(vector) rhs) noexcept
		{
			if constexpr (is_small_float)
			{
				return vector{ promoted_vec{ lhs } * promoted_vec{ rhs } };
			}
			else
			{
				// clang-format off

				#define VEC_FUNC(member) lhs.member * rhs.member
				COMPONENTWISE_CONSTRUCT(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief Componentwise multiplies this vector by another.
		constexpr vector& MUU_VECTORCALL operator*=(MUU_VC_PARAM(vector) rhs) noexcept
		{
			if constexpr (is_small_float)
			{
				return *this = vector{ promoted_vec{ *this } * promoted_vec{ rhs } };
			}
			else
			{
				// clang-format off

				#define VEC_FUNC(member) base::member * rhs.member
				COMPONENTWISE_ASSIGN(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief Returns the componentwise multiplication of a vector and a scalar.
		MUU_PURE_GETTER
		friend constexpr vector MUU_VECTORCALL operator*(MUU_VC_PARAM(vector) lhs, scalar_type rhs) noexcept
		{
			if constexpr (is_small_float)
			{
				return vector{ promoted_vec{ lhs } * static_cast<promoted_scalar>(rhs) };
			}
			else
			{
				// clang-format off

				#define VEC_FUNC(member) lhs.member * rhs
				COMPONENTWISE_CONSTRUCT(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief Returns the componentwise multiplication of a vector and a scalar.
		MUU_PURE_INLINE_GETTER
		friend constexpr vector MUU_VECTORCALL operator*(scalar_type lhs, MUU_VC_PARAM(vector) rhs) noexcept
		{
			return rhs * lhs;
		}

		/// \brief Componentwise multiplies this vector by a scalar.
		constexpr vector& MUU_VECTORCALL operator*=(scalar_type rhs) noexcept
		{
			if constexpr (is_small_float)
			{
				return *this = vector{ promoted_vec{ *this } * static_cast<promoted_scalar>(rhs) };
			}
			else
			{
				// clang-format off

				#define VEC_FUNC(member) base::member * rhs
				COMPONENTWISE_ASSIGN(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief Returns the product of all the scalar components in a vector.
		MUU_PURE_GETTER
		static constexpr scalar_type MUU_VECTORCALL product(MUU_VC_PARAM(vector) v) noexcept
		{
			if constexpr (dimensions == 1)
			{
				return v.x;
			}
			else
			{
				using scalar_mult_type = decltype(scalar_type{} * scalar_type{});
				using float_arith_type = promoted_scalar;
				using integer_arith_type =
					std::conditional_t<std::is_same_v<scalar_mult_type, scalar_type>, scalar_type, scalar_mult_type>;
				using arith_type = std::conditional_t<is_integral<scalar_type>, integer_arith_type, float_arith_type>;

				if constexpr (!std::is_same_v<arith_type, scalar_type>)
				{
					using arith_vec = vector<arith_type, dimensions>;
					return static_cast<scalar_type>(arith_vec::product(arith_vec{ v }));
				}
				else
				{
					// clang-format off

					#define VEC_FUNC(member) v.member
					COMPONENTWISE_ACCUMULATE(VEC_FUNC, *);
					#undef VEC_FUNC

					// clang-format on
				}
			}
		}

		/// \brief Returns the product of all the scalar components in the vector.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type MUU_VECTORCALL product() const noexcept
		{
			return product(*this);
		}

		/// @}
#endif // multiplication

#if 1 // division -------------------------------------------------------------------------------------------------
		/// \name Division
		/// @{

		/// \brief Returns the componentwise division of one vector by another.
		MUU_PURE_GETTER
		friend constexpr vector MUU_VECTORCALL operator/(MUU_VC_PARAM(vector) lhs, MUU_VC_PARAM(vector) rhs) noexcept
		{
			if constexpr (is_small_float)
			{
				return vector{ promoted_vec{ lhs } / promoted_vec{ rhs } };
			}
			else
			{
				// clang-format off

				#define VEC_FUNC(member) lhs.member / rhs.member
				COMPONENTWISE_CONSTRUCT(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief Componentwise divides this vector by another.
		constexpr vector& MUU_VECTORCALL operator/=(MUU_VC_PARAM(vector) rhs) noexcept
		{
			if constexpr (is_small_float)
			{
				return *this = vector{ promoted_vec{ *this } / promoted_vec{ rhs } };
			}
			else
			{
				// clang-format off

				#define VEC_FUNC(member) base::member / rhs.member
				COMPONENTWISE_ASSIGN(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief Returns the componentwise division of a vector and a scalar.
		MUU_PURE_GETTER
		friend constexpr vector MUU_VECTORCALL operator/(MUU_VC_PARAM(vector) lhs, scalar_type rhs) noexcept
		{
			if constexpr (is_small_float)
			{
				return vector{ promoted_vec{ lhs } * (promoted_scalar{ 1 } / static_cast<promoted_scalar>(rhs)) };
			}
			else if constexpr (is_floating_point<scalar_type>)
			{
				return lhs * (scalar_type{ 1 } / rhs);
			}
			else
			{
				// clang-format off

				#define VEC_FUNC(member) lhs.member / rhs
				COMPONENTWISE_CONSTRUCT(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief Componentwise divides this vector by a scalar.
		constexpr vector& MUU_VECTORCALL operator/=(scalar_type rhs) noexcept
		{
			if constexpr (is_small_float)
			{
				return *this =
						   vector{ promoted_vec{ *this } * (promoted_scalar{ 1 } / static_cast<promoted_scalar>(rhs)) };
			}
			else if constexpr (is_floating_point<scalar_type>)
			{
				return *this *= (scalar_type{ 1 } / rhs);
			}
			else
			{
				// clang-format off

				#define VEC_FUNC(member) base::member / rhs
				COMPONENTWISE_ASSIGN(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// @}
#endif // division

#if 1 // modulo ---------------------------------------------------------------------------------------------------
		/// \name Modulo
		/// @{

		/// \brief Returns the remainder of componentwise dividing of one vector by another.
		MUU_PURE_GETTER
		friend constexpr vector MUU_VECTORCALL operator%(MUU_VC_PARAM(vector) lhs, MUU_VC_PARAM(vector) rhs) noexcept
		{
			if constexpr (is_small_float)
			{
				return vector{ promoted_vec{ lhs } % promoted_vec{ rhs } };
			}
			else
			{
				// clang-format off

				#define VEC_FUNC(member) impl::raw_modulo(lhs.member, rhs.member)
				COMPONENTWISE_CONSTRUCT(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief Assigns the result of componentwise dividing this vector by another.
		constexpr vector& MUU_VECTORCALL operator%=(MUU_VC_PARAM(vector) rhs) noexcept
		{
			if constexpr (is_small_float)
			{
				return *this = vector{ promoted_vec{ *this } % promoted_vec{ rhs } };
			}
			else
			{
				// clang-format off

				#define VEC_FUNC(member) impl::raw_modulo(base::member, rhs.member)
				COMPONENTWISE_ASSIGN(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief Returns the remainder of componentwise dividing vector by a scalar.
		MUU_PURE_GETTER
		friend constexpr vector MUU_VECTORCALL operator%(MUU_VC_PARAM(vector) lhs, scalar_type rhs) noexcept
		{
			if constexpr (is_small_float)
			{
				return vector{ promoted_vec{ lhs } % static_cast<promoted_scalar>(rhs) };
			}
			else
			{
				// clang-format off

				#define VEC_FUNC(member) impl::raw_modulo(lhs.member, rhs)
				COMPONENTWISE_CONSTRUCT(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief Assigns the result of componentwise dividing this vector by a scalar.
		constexpr vector& MUU_VECTORCALL operator%=(scalar_type rhs) noexcept
		{
			if constexpr (is_small_float)
			{
				return *this = vector{ promoted_vec{ *this } % static_cast<promoted_scalar>(rhs) };
			}
			else
			{
				// clang-format off

				#define VEC_FUNC(member) impl::raw_modulo(base::member, rhs)
				COMPONENTWISE_ASSIGN(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// @}
#endif // modulo

#if 1 // bitwise shifts ----------------------------------------------------------------------------------------
		/// \name Bitwise shifts
		/// \availability		These functions are only available when #scalar_type is an integral type.
		/// @{

		/// \brief Returns a vector with each scalar component left-shifted the given number of bits.
		///
		/// \availability		This function is only available when #scalar_type is an integral type.
		MUU_HIDDEN_CONSTRAINT(is_integral<T>, typename T = Scalar)
		MUU_PURE_GETTER
		friend constexpr vector MUU_VECTORCALL operator<<(MUU_VC_PARAM(vector) lhs, product_scalar_type rhs) noexcept
		{
			if constexpr (product_requires_promotion)
			{
				return vector{ promoted_product_vec{ lhs } << static_cast<promoted_product>(rhs) };
			}
			else
			{
				MUU_ASSUME(rhs >= 0);

				// clang-format off

				#define VEC_FUNC(member) lhs.member << rhs
				COMPONENTWISE_CONSTRUCT(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief Componentwise left-shifts each scalar component in the vector by the given number of bits.
		///
		/// \availability		This function is only available when #scalar_type is an integral type.
		MUU_HIDDEN_CONSTRAINT(is_integral<T>, typename T = Scalar)
		constexpr vector& MUU_VECTORCALL operator<<=(product_scalar_type rhs) noexcept
		{
			if constexpr (product_requires_promotion)
			{
				return *this = vector{ promoted_product_vec{ *this } << static_cast<promoted_product>(rhs) };
			}
			else
			{
				MUU_ASSUME(rhs >= 0);

				// clang-format off

				#define VEC_FUNC(member) base::member << rhs
				COMPONENTWISE_ASSIGN(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief Returns a vector with each scalar component right-shifted the given number of bits.
		///
		/// \availability		This function is only available when #scalar_type is an integral type.
		MUU_HIDDEN_CONSTRAINT(is_integral<T>, typename T = Scalar)
		MUU_PURE_GETTER
		friend constexpr vector MUU_VECTORCALL operator>>(MUU_VC_PARAM(vector) lhs, product_scalar_type rhs) noexcept
		{
			if constexpr (product_requires_promotion)
			{
				return vector{ promoted_product_vec{ lhs } >> static_cast<promoted_product>(rhs) };
			}
			else
			{
				MUU_ASSUME(rhs >= 0);

				// clang-format off

				#define VEC_FUNC(member) lhs.member >> rhs
				COMPONENTWISE_CONSTRUCT(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief Componentwise right-shifts each scalar component in the vector by the given number of bits.
		///
		/// \availability		This function is only available when #scalar_type is an integral type.
		MUU_HIDDEN_CONSTRAINT(is_integral<T>, typename T = Scalar)
		constexpr vector& MUU_VECTORCALL operator>>=(product_scalar_type rhs) noexcept
		{
			if constexpr (product_requires_promotion)
			{
				return *this = vector{ promoted_product_vec{ *this } >> static_cast<promoted_product>(rhs) };
			}
			else
			{
				MUU_ASSUME(rhs >= 0);

				// clang-format off

				#define VEC_FUNC(member) base::member >> rhs
				COMPONENTWISE_ASSIGN(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// @}
#endif // bitwise shifts

#if 1 // normalization --------------------------------------------------------------------------------------------
		/// \name Normalization
		/// \availability These functions are only available when #scalar_type is a floating-point type.
		/// @{

		/// \brief	Normalizes a vector.
		///
		/// \param v			The vector to normalize.
		/// \param length_out	An output param to receive the length of the vector pre-normalization.
		///
		/// \return		A normalized copy of the input vector.
		///
		/// \availability This function is only available when #scalar_type is a floating-point type.
		MUU_HIDDEN_CONSTRAINT(is_floating_point<T>, typename T = Scalar)
		MUU_NODISCARD
		static constexpr vector MUU_VECTORCALL normalize(MUU_VC_PARAM(vector) v, delta_scalar_type& length_out) noexcept
		{
			if constexpr (Dimensions == 1)
			{
				length_out = static_cast<delta_scalar_type>(v.x);
				return vector{ scalar_constants::one };
			}
			else if constexpr (delta_requires_promotion)
			{
				promoted_delta lo{};
				auto out   = vector{ promoted_delta_vec::normalize(promoted_delta_vec{ v }, lo) };
				length_out = static_cast<delta_scalar_type>(lo);
				return out;
			}
			else
			{
				const auto len = length(v);
				length_out	   = len;
				return v * (delta_scalar_type{ 1 } / len);
			}
		}

		/// \brief	Normalizes a vector.
		///
		/// \param v	The vector to normalize.
		///
		/// \return		A normalized copy of the input vector.
		///
		/// \availability This function is only available when #scalar_type is a floating-point type.
		MUU_HIDDEN_CONSTRAINT(is_floating_point<T>, typename T = Scalar)
		MUU_PURE_GETTER
		static constexpr vector MUU_VECTORCALL normalize(MUU_VC_PARAM(vector) v) noexcept
		{
			if constexpr (Dimensions == 1)
			{
				MUU_UNUSED(v);
				return vector{ scalar_constants::one };
			}
			else if constexpr (delta_requires_promotion)
			{
				return vector{ promoted_delta_vec::normalize(promoted_delta_vec{ v }) };
			}
			else
			{
				return v * (delta_scalar_type{ 1 } / length(v));
			}
		}

		/// \brief	Normalizes a vector.
		///
		/// \param v	The vector to normalize.
		///
		/// \return		A normalized copy of the input vector.
		///
		/// \availability This function is only available when #scalar_type is a floating-point type.
		///
		/// \warning	This function is implemented such that it is _always_ available at compile time,
		///				arriving at the result using very slow iterative machinery. Do not use it at runtime!
		MUU_HIDDEN_CONSTRAINT(is_floating_point<T>, typename T = Scalar)
		MUU_PURE_GETTER
		MUU_CONSTEVAL
		static vector MUU_VECTORCALL consteval_normalize(MUU_VC_PARAM(vector) v) noexcept
		{
			if constexpr (Dimensions == 1)
			{
				MUU_UNUSED(v);
				return vector{ scalar_constants::one };
			}
			else if constexpr (delta_requires_promotion)
			{
				return vector{ promoted_delta_vec::consteval_normalize(promoted_delta_vec{ v }) };
			}
			else
			{
				return v * (delta_scalar_type{ 1 } / consteval_length(v));
			}
		}

		/// \brief	Normalizes the vector (in-place).
		///
		/// \param length_out	An output param to receive the length of the vector pre-normalization.
		///
		/// \return	A reference to the vector.
		///
		/// \availability This function is only available when #scalar_type is a floating-point type.
		MUU_HIDDEN_CONSTRAINT(is_floating_point<T>, typename T = Scalar)
		constexpr vector& normalize(delta_scalar_type& length_out) noexcept
		{
			return *this = normalize(*this, length_out);
		}

		/// \brief	Normalizes the vector (in-place).
		///
		/// \return	A reference to the vector.
		///
		/// \availability This function is only available when #scalar_type is a floating-point type.
		MUU_HIDDEN_CONSTRAINT(is_floating_point<T>, typename T = Scalar)
		constexpr vector& normalize() noexcept
		{
			return *this = normalize(*this);
		}

		/// \brief	Normalizes a vector using a pre-calculated squared-length.
		///
		/// \param v			The vector to normalize.
		/// \param v_lensq		The pre-calculated squared-length of `v`.
		///
		/// \return		A normalized copy of the input vector.
		///
		/// \availability This function is only available when #scalar_type is a floating-point type.
		MUU_HIDDEN_CONSTRAINT(is_floating_point<T>, typename T = Scalar)
		MUU_NODISCARD
		static constexpr vector MUU_VECTORCALL normalize_lensq(MUU_VC_PARAM(vector) v,
															   delta_scalar_type v_lensq) noexcept
		{
			if constexpr (delta_requires_promotion)
			{
				return vector{ promoted_delta_vec::normalize_lensq(promoted_delta_vec{ v },
																   static_cast<promoted_delta>(v_lensq)) };
			}
			else
			{
				return v * (delta_scalar_type{ 1 } / muu::sqrt(v_lensq));
			}
		}

		/// \brief	Normalizes the vector using a pre-calculated squared-length (in-place).
		///
		/// \param lensq		The pre-calculated squared-length of the vector.
		///
		/// \return	A reference to the vector.
		///
		/// \availability This function is only available when #scalar_type is a floating-point type.
		MUU_HIDDEN_CONSTRAINT(is_floating_point<T>, typename T = Scalar)
		constexpr vector& MUU_VECTORCALL normalize_lensq(delta_scalar_type lensq) noexcept
		{
			return *this = normalize_lensq(*this, lensq);
		}

		/// \brief Returns true if a vector is normalized (i.e. has a length of 1).
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL normalized(MUU_VC_PARAM(vector) v) noexcept
		{
			if constexpr (is_integral<scalar_type>)
			{
				if constexpr (Dimensions == 1)
					return v.x == scalar_type{ 1 };
				else
				{
					using sum_type = decltype(scalar_type{} + scalar_type{});
					sum_type sum{};
					for (size_t i = 0u; i < dimensions && sum > sum_type{ 1 }; i++)
					{
						if constexpr (muu::is_signed<scalar_type>)
							sum += muu::abs(v[i]);
						else
							sum += v[i];
					}
					return sum == sum_type{ 1 };
				}
			}
			else
			{
				constexpr promoted_delta epsilon =
					promoted_delta{ 1 }
					/ (100ull																		//
					   * (muu::constants<scalar_type>::significand_digits >= 24u ? 10000ull : 1ull) // float32
					   * (muu::constants<scalar_type>::significand_digits >= 53u ? 10000ull : 1ull) // float64
					);

				if constexpr (delta_requires_promotion)
				{
					return muu::approx_equal(promoted_delta_vec::length_squared(promoted_delta_vec{ v }),
											 promoted_delta{ 1 },
											 epsilon);
				}
				else
				{
					return muu::approx_equal(length_squared(v), promoted_delta{ 1 }, epsilon);
				}
			}
		}

		/// \brief Returns true if the vector is normalized (i.e. has a length of 1).
		MUU_PURE_INLINE_GETTER
		constexpr bool normalized() const noexcept
		{
			return normalized(*this);
		}

		/// @}
#endif // normalization

#if 1 // direction ------------------------------------------------------------------------------------------------
		/// \name Direction
		/// \availability	These functions are only available when #dimensions == 2 or 3.
		/// @{

		/// \brief		Returns the normalized direction vector from one position to another.
		///
		/// \param	from			The start position.
		/// \param	to				The end position.
		/// \param	distance_out	An output param to receive the distance between the two points.
		///
		/// \return		A normalized direction vector pointing from the start position to the end position.
		///
		/// \availability	This function is only available when #dimensions == 2 or 3.
		MUU_HIDDEN_CONSTRAINT(Dims == 2 || Dims == 3, size_t Dims = Dimensions)
		MUU_PURE_GETTER
		static constexpr delta_type MUU_VECTORCALL direction(MUU_VC_PARAM(vector) from,
															 MUU_VC_PARAM(vector) to,
															 delta_scalar_type& distance_out) noexcept
		{
			if constexpr (delta_requires_promotion)
			{
				promoted_delta dist_out{};
				const auto out =
					promoted_delta_vec::direction(promoted_delta_vec{ from }, promoted_delta_vec{ to }, dist_out);
				distance_out = static_cast<delta_scalar_type>(dist_out);
				return delta_type{ out };
			}
			else
			{
				return delta_type::normalize(to - from, distance_out);
			}
		}

		/// \brief		Returns the normalized direction vector from one position to another.
		///
		/// \param	from			The start position.
		/// \param	to				The end position.
		///
		/// \return		A normalized direction vector pointing from the start position to the end position.
		///
		/// \availability	This function is only available when #dimensions == 2 or 3.
		MUU_HIDDEN_CONSTRAINT(Dims == 2 || Dims == 3, size_t Dims = Dimensions)
		MUU_PURE_GETTER
		static constexpr delta_type MUU_VECTORCALL direction(MUU_VC_PARAM(vector) from,
															 MUU_VC_PARAM(vector) to) noexcept
		{
			if constexpr (delta_requires_promotion)
			{
				return delta_type{ promoted_delta_vec::direction(promoted_delta_vec{ from },
																 promoted_delta_vec{ to }) };
			}
			else
			{
				return delta_type::normalize(to - from);
			}
		}

		/// \brief		Returns the normalized direction vector from this position to another.
		///
		/// \param	to				The end position.
		/// \param	distance_out	An output param to receive the distance between the two points.
		///
		/// \availability	This function is only available when #dimensions == 2 or 3.
		MUU_HIDDEN_CONSTRAINT(Dims == 2 || Dims == 3, size_t Dims = Dimensions)
		MUU_PURE_INLINE_GETTER
		constexpr delta_type MUU_VECTORCALL direction(MUU_VC_PARAM(vector) to,
													  delta_scalar_type& distance_out) const noexcept
		{
			return direction(*this, to, distance_out);
		}

		/// \brief		Returns the normalized direction vector from this position to another.
		///
		/// \param	to				The end position.
		///
		/// \availability	This function is only available when #dimensions == 2 or 3.
		MUU_HIDDEN_CONSTRAINT(Dims == 2 || Dims == 3, size_t Dims = Dimensions)
		MUU_PURE_INLINE_GETTER
		constexpr delta_type MUU_VECTORCALL direction(MUU_VC_PARAM(vector) to) const noexcept
		{
			return direction(*this, to);
		}

		/// @}
#endif // direction

#if 1 // iterators ------------------------------------------------------------------------------------------------
		/// \name Iterators
		/// @{

		/// \brief Returns an iterator to the first scalar component in the vector.
		MUU_PURE_INLINE_GETTER
		constexpr iterator begin() noexcept
		{
			return data();
		}

		/// \brief Returns an iterator to the one-past-the-last scalar component in the vector.
		MUU_PURE_INLINE_GETTER
		constexpr iterator end() noexcept
		{
			return begin() + Dimensions;
		}

		/// \brief Returns a const iterator to the first scalar component in the vector.
		MUU_PURE_INLINE_GETTER
		constexpr const_iterator begin() const noexcept
		{
			return data();
		}

		/// \brief Returns a const iterator to the one-past-the-last scalar component in the vector.
		MUU_PURE_INLINE_GETTER
		constexpr const_iterator end() const noexcept
		{
			return begin() + Dimensions;
		}

		/// \brief Returns a const iterator to the first scalar component in the vector.
		MUU_PURE_INLINE_GETTER
		constexpr const_iterator cbegin() const noexcept
		{
			return begin();
		}

		/// \brief Returns a const iterator to the one-past-the-last scalar component in the vector.
		MUU_PURE_INLINE_GETTER
		constexpr const_iterator cend() const noexcept
		{
			return end();
		}

		/// @}

		/// \name Iterators (ADL)
		/// @{

		/// \brief Returns an iterator to the first scalar component in a vector.
		MUU_PURE_INLINE_GETTER
		friend constexpr iterator begin(vector& v) noexcept
		{
			return v.begin();
		}

		/// \brief Returns an iterator to the one-past-the-last scalar component in a vector.
		MUU_PURE_INLINE_GETTER
		friend constexpr iterator end(vector& v) noexcept
		{
			return v.end();
		}

		/// \brief Returns a const iterator to the first scalar component in a vector.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_iterator begin(const vector& v) noexcept
		{
			return v.begin();
		}

		/// \brief Returns a const iterator to the one-past-the-last scalar component in a vector.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_iterator end(const vector& v) noexcept
		{
			return v.end();
		}

		/// \brief Returns a const iterator to the first scalar component in a vector.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_iterator cbegin(const vector& v) noexcept
		{
			return v.begin();
		}

		/// \brief Returns a const iterator to the one-past-the-last scalar component in a vector.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_iterator cend(const vector& v) noexcept
		{
			return v.end();
		}

		/// @}
#endif // iterators

#if 1 // min, max and clamp ---------------------------------------------------------------------------------------
		/// \name Min, Max and Clamp
		/// @{

		/// \brief	Returns the componentwise minimum of two or more vectors.
		MUU_CONSTRAINED_TEMPLATE((sizeof...(T) == 0 || all_convertible_to<vector, const T&...>), typename... T)
		MUU_PURE_GETTER
		static constexpr vector MUU_VECTORCALL min(MUU_VC_PARAM(vector) v1,
												   MUU_VC_PARAM(vector) v2,
												   const T&... vecs) noexcept
		{
			if constexpr (sizeof...(T) > 0)
				return vector::min(v1, vector::min(v2, vecs...));
			else
			{
				// clang-format off

				#define VEC_FUNC(member) muu::min(v1.member, v2.member)
				COMPONENTWISE_CONSTRUCT(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief	Returns the componentwise minimum of a range of vectors.
		MUU_PURE_GETTER
		static constexpr vector MUU_VECTORCALL min(const vector* begin_, const vector* end_) noexcept
		{
			if (begin_ == end_)
				return vector{};

			MUU_ASSUME(begin_ != nullptr);
			MUU_ASSUME(end_ != nullptr);
			MUU_ASSUME(begin_ < end_);

			vector out = *begin_;
			for (auto v = begin_ + 1u; v != end_; v++)
				out = vector::min(out, *v);
			return out;
		}

		/// \brief	Returns the componentwise minimum of a list of vectors.
		MUU_PURE_GETTER
		static constexpr vector MUU_VECTORCALL min(std::initializer_list<vector> vecs) noexcept
		{
			switch (vecs.size())
			{
				case 0: return vector{};
				case 1: return *vecs.begin();
				case 2: return vector::min(*vecs.begin(), *(vecs.begin() + 1u));
				default: return vector::min(vecs.begin(), vecs.end());
			}
		}

		/// \brief	Returns the componentwise maximum of two or more vectors.
		MUU_CONSTRAINED_TEMPLATE((sizeof...(T) == 0 || all_convertible_to<vector, const T&...>), typename... T)
		MUU_PURE_GETTER
		static constexpr vector MUU_VECTORCALL max(MUU_VC_PARAM(vector) v1,
												   MUU_VC_PARAM(vector) v2,
												   const T&... vecs) noexcept
		{
			if constexpr (sizeof...(T) > 0)
				return vector::max(v1, vector::max(v2, vecs...));
			else
			{
				// clang-format off

				#define VEC_FUNC(member) muu::max(v1.member, v2.member)
				COMPONENTWISE_CONSTRUCT(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief	Returns the componentwise maximum of a range of vectors.
		MUU_PURE_GETTER
		static constexpr vector MUU_VECTORCALL max(const vector* begin_, const vector* end_) noexcept
		{
			if (begin_ == end_)
				return vector{};

			MUU_ASSUME(begin_ != nullptr);
			MUU_ASSUME(end_ != nullptr);

			vector out = *begin_;
			for (auto v = begin_ + 1u; v != end_; v++)
				out = vector::max(out, *v);
			return out;
		}

		/// \brief	Returns the componentwise maximum of a list of vectors.
		MUU_PURE_GETTER
		static constexpr vector MUU_VECTORCALL max(std::initializer_list<vector> vecs) noexcept
		{
			switch (vecs.size())
			{
				case 0: return vector{};
				case 1: return *vecs.begin();
				case 2: return vector::max(*vecs.begin(), *(vecs.begin() + 1u));
				default: return vector::max(vecs.begin(), vecs.end());
			}
		}

		/// \brief	Componentwise clamps a vector between two others.
		///
		/// \param v		The vector being clamped.
		/// \param low		The low bound of the clamp operation.
		/// \param high		The high bound of the clamp operation.
		///
		/// \return	A vector containing the scalar components from `v` clamped inside the given bounds.
		MUU_PURE_GETTER
		static constexpr vector MUU_VECTORCALL clamp(MUU_VC_PARAM(vector) v,
													 MUU_VC_PARAM(vector) low,
													 MUU_VC_PARAM(vector) high) noexcept
		{
			// clang-format off

			#define VEC_FUNC(member) muu::clamp(v.member, low.member, high.member)
			COMPONENTWISE_CONSTRUCT(VEC_FUNC);
			#undef VEC_FUNC

			// clang-format on
		}

		/// \brief	Componentwise clamps the vector between two others (in-place).
		///
		/// \param low		The low bound of the clamp operation.
		/// \param high		The high bound of the clamp operation.
		///
		/// \return	A reference to the vector.
		constexpr vector& MUU_VECTORCALL clamp(MUU_VC_PARAM(vector) low, MUU_VC_PARAM(vector) high) noexcept
		{
			// clang-format off

			#define VEC_FUNC(member) muu::clamp(base::member, low.member, high.member)
			COMPONENTWISE_ASSIGN(VEC_FUNC);
			#undef VEC_FUNC

			// clang-format on
		}

		/// @}
#endif // min, max and clamp

#if 1 // swizzles -------------------------------------------------------------------------------------------------
		/// \name Swizzles
		///@{

		/// \brief Creates a vector by selecting and re-packing scalar components from
		/// 	   another vector in an abitrary order.
		///
		/// \tparam	Indices		Indices of the scalar components from the source vector
		/// 					in the order they're to be re-packed.
		/// \param v			The source vector.
		///
		/// \details \cpp
		///
		/// auto v = vector{ 10, 7, 5, 9 };
		/// //                ^  ^  ^  ^
		/// // indices:       0  1  2  3
		///
		/// using vec4 = decltype(v);
		///
		///	std::cout << "swizzle<0>:          " << vec4::swizzle<0>(v)          << "\n";
		///	std::cout << "swizzle<1, 0>:       " << vec4::swizzle<1, 0>(v)       << "\n";
		///	std::cout << "swizzle<3, 2, 3>:    " << vec4::swizzle<3, 2, 3>(v)    << "\n";
		///	std::cout << "swizzle<0, 1, 0, 1>: " << vec4::swizzle<0, 1, 0, 1>(v) << "\n";
		/// \ecpp
		///
		/// \out
		/// swizzle<0>:          { 10 }
		/// swizzle<1, 0>:       { 7, 10 }
		/// swizzle<3, 2, 3>:    { 9, 5, 9 }
		/// swizzle<0, 1, 0, 1>: { 10, 7, 10, 7 }
		/// \eout
		///
		/// \return  A vector composed from the desired 'swizzle' of the source vector.
		template <size_t... Indices>
		MUU_PURE_GETTER
		static constexpr vector<scalar_type, sizeof...(Indices)> MUU_VECTORCALL swizzle(MUU_VC_PARAM(vector) v) noexcept
		{
			static_assert(sizeof...(Indices) > 0_sz, "At least one scalar index must be specified.");
			static_assert((true && ... && (Indices < Dimensions)),
						  "One or more of the scalar indices was out-of-range");

			return vector<scalar_type, sizeof...(Indices)>{ v.template get<Indices>()... };
		}

		/// \brief Creates a vector by selecting and re-packing the
		/// 	   scalar components from this vector in an abitrary order.
		///
		/// \tparam	Indices		Indices of the scalar components in the order they're to be re-packed.
		///
		/// \details \cpp
		///
		/// auto v = vector{ 10, 7, 5, 9 };
		/// //                ^  ^  ^  ^
		/// // indices:       0  1  2  3
		///
		///	std::cout << "swizzle<0>:          " << v.swizzle<0>(v)          << "\n";
		///	std::cout << "swizzle<1, 0>:       " << v.swizzle<1, 0>(v)       << "\n";
		///	std::cout << "swizzle<3, 2, 3>:    " << v.swizzle<3, 2, 3>(v)    << "\n";
		///	std::cout << "swizzle<0, 1, 0, 1>: " << v.swizzle<0, 1, 0, 1>(v) << "\n";
		/// \ecpp
		///
		/// \out
		/// swizzle<0>:          { 10 }
		/// swizzle<1, 0>:       { 7, 10 }
		/// swizzle<3, 2, 3>:    { 9, 5, 9 }
		/// swizzle<0, 1, 0, 1>: { 10, 7, 10, 7 }
		/// \eout
		///
		/// \return  A vector composed from the desired 'swizzle' of this one.
		template <size_t... Indices>
		MUU_PURE_INLINE_GETTER
		constexpr vector<scalar_type, sizeof...(Indices)> swizzle() const noexcept
		{
			return swizzle<Indices...>(*this);
		}

		/// \brief Returns a two-dimensional vector containing `{ x, y }`.
		MUU_HIDDEN_CONSTRAINT(Dims >= 2, size_t Dims = Dimensions)
		MUU_PURE_INLINE_GETTER
		constexpr vector<scalar_type, 2> xy() const noexcept
		{
			return vector<scalar_type, 2>{ get<0>(), get<1>() };
		}

		/// \brief Returns a two-dimensional vector containing `{ x, z }`.
		MUU_HIDDEN_CONSTRAINT(Dims >= 3, size_t Dims = Dimensions)
		MUU_PURE_INLINE_GETTER
		constexpr vector<scalar_type, 2> xz() const noexcept
		{
			return vector<scalar_type, 2>{ get<0>(), get<2>() };
		}

		/// \brief Returns a two-dimensional vector containing `{ y, x }`.
		MUU_HIDDEN_CONSTRAINT(Dims >= 2, size_t Dims = Dimensions)
		MUU_PURE_INLINE_GETTER
		constexpr vector<scalar_type, 2> yx() const noexcept
		{
			return vector<scalar_type, 2>{ get<1>(), get<0>() };
		}

		/// \brief Returns a three-dimensional vector containing `{ x, y, z }`.
		MUU_HIDDEN_CONSTRAINT(Dims >= 3, size_t Dims = Dimensions)
		MUU_PURE_INLINE_GETTER
		constexpr vector<scalar_type, 3> xyz() const noexcept
		{
			return vector<scalar_type, 3>{ get<0>(), get<1>(), get<2>() };
		}

		/// \brief Returns a four-dimensional vector containing `{ x, y, z, 1 }`.
		MUU_HIDDEN_CONSTRAINT(Dims >= 3, size_t Dims = Dimensions)
		MUU_PURE_INLINE_GETTER
		constexpr vector<scalar_type, 4> xyz1() const noexcept
		{
			return vector<scalar_type, 4>{ get<0>(), get<1>(), get<2>(), scalar_constants::one };
		}

		/// \brief Returns a four-dimensional vector containing `{ x, y, z, 0 }`.
		MUU_HIDDEN_CONSTRAINT(Dims >= 3, size_t Dims = Dimensions)
		MUU_PURE_INLINE_GETTER
		constexpr vector<scalar_type, 4> xyz0() const noexcept
		{
			return vector<scalar_type, 4>{ get<0>(), get<1>(), get<2>(), scalar_constants::zero };
		}

///@}
#endif // swizzles

#if 1 // misc -----------------------------------------------------------------------------------------------------

		/// \brief Writes a vector out to a text stream.
		template <typename Char, typename Traits>
		friend std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os, const vector& v)
		{
			impl::print_vector(os, v.data(), Dimensions);
			return os;
		}

		/// \brief	Performs a linear interpolation between two vectors.
		///
		/// \param	start	The value at the start of the interpolation range.
		/// \param	finish	The value at the end of the interpolation range.
		/// \param	alpha 	The blend factor.
		///
		/// \returns	A vector with values derived from a linear interpolation from `start` to `finish`.
		MUU_PURE_GETTER
		static constexpr vector MUU_VECTORCALL lerp(MUU_VC_PARAM(vector) start,
													MUU_VC_PARAM(vector) finish,
													delta_scalar_type alpha) noexcept
		{
			if constexpr (delta_requires_promotion)
			{
				return vector{ promoted_delta_vec::lerp(promoted_delta_vec{ start },
														promoted_delta_vec{ finish },
														static_cast<promoted_delta>(alpha)) };
			}
			else
			{
				const auto inv_alpha = delta_scalar_type{ 1 } - alpha;

				// clang-format off

				#define VEC_FUNC(member)     start.member * inv_alpha + finish.member * alpha
				COMPONENTWISE_CONSTRUCT(VEC_FUNC);
				#undef VEC_FUNC

				// clang-format on
			}
		}

		/// \brief	Linearly interpolates this vector towards another (in-place).
		///
		/// \param	target	The 'target' value for the interpolation.
		/// \param	alpha 	The blend factor.
		///
		/// \return	A reference to the vector.
		constexpr vector& MUU_VECTORCALL lerp(MUU_VC_PARAM(vector) target, delta_scalar_type alpha) noexcept
		{
			return *this = lerp(*this, target, alpha);
		}

		/// \brief	Calculates the angle between two vectors.
		///
		/// \param	v1		A vector.
		/// \param	v2		A vector.
		///
		/// \returns	The angle between `v1` and `v2` (in radians).
		///
		/// \remarks	The angle returned is the unsigned angle between the two vectors;
		/// 			the smaller of the two possible angles between the two vectors is used.
		/// 			The result is never greater than `pi` radians (180 degrees).
		///
		/// \availability		This function is only available when #dimensions == 2 or 3.
		MUU_HIDDEN_CONSTRAINT((Dims == 2 || Dims == 3), size_t Dims = Dimensions)
		MUU_PURE_GETTER
		static constexpr delta_scalar_type MUU_VECTORCALL angle(MUU_VC_PARAM(vector) v1,
																MUU_VC_PARAM(vector) v2) noexcept
		{
			// intermediate calcs are done using doubles because anything else is far too imprecise
			using angle_type = impl::highest_ranked<promoted_delta, double>;

			if constexpr (!all_same<delta_scalar_type, scalar_type, angle_type>)
			{
				using angle_vec = vector<angle_type, Dimensions>;
				return static_cast<delta_scalar_type>(angle_vec::angle(angle_vec{ v1 }, angle_vec{ v2 }));
			}
			else
			{
				// law of cosines
				// https://stackoverflow.com/questions/10507620/finding-the-angle-between-vectors

				const auto divisor = length(v1) * length(v2);
				if (divisor == delta_scalar_type{})
					return delta_scalar_type{};

				return muu::acos(muu::clamp(dot(v1, v2) / divisor, delta_scalar_type{ -1 }, delta_scalar_type{ 1 }));
			}
		}

		/// \brief	Calculates the angle between this vector and another.
		///
		/// \param	v	A vector.
		///
		/// \returns	The angle between this vector and `v` (in radians).
		///
		/// \remarks	The angle returned is the unsigned angle between the two vectors;
		/// 			the smaller of the two possible angles between the two vectors is used.
		/// 			The result is never greater than `pi` radians (180 degrees).
		///
		/// \availability		This function is only available when #dimensions == 2 or 3.
		MUU_HIDDEN_CONSTRAINT((Dims == 2 || Dims == 3), size_t Dims = Dimensions)
		MUU_PURE_INLINE_GETTER
		constexpr delta_scalar_type MUU_VECTORCALL angle(MUU_VC_PARAM(vector) v) const noexcept
		{
			return angle(*this, v);
		}

		/// \brief	Returns a vector with all scalar components set to their absolute values.
		MUU_PURE_GETTER
		static constexpr vector MUU_VECTORCALL abs(MUU_VC_PARAM(vector) v) noexcept
		{
			// clang-format off

			if constexpr (is_signed<scalar_type>)
			{
				#define VEC_FUNC(member) muu::abs(v.member)
				COMPONENTWISE_CONSTRUCT(VEC_FUNC);
				#undef VEC_FUNC
			}
			else
				return v;

			// clang-format on
		}

		/// \brief	Returns a copy of this vector with all scalar components set to their absolute values.
		MUU_PURE_INLINE_GETTER
		constexpr vector abs() const noexcept
		{
			if constexpr (is_signed<scalar_type>)
				return abs(*this);
			else
				return *this;
		}

		/// \brief	Returns a vector with all scalar components set to the lowest integer not less than their original
		/// values.
		MUU_PURE_GETTER
		static constexpr vector MUU_VECTORCALL ceil(MUU_VC_PARAM(vector) v) noexcept
		{
			// clang-format off

			if constexpr (is_floating_point<scalar_type>)
			{
				#define VEC_FUNC(member) muu::ceil(v.member)
				COMPONENTWISE_CONSTRUCT(VEC_FUNC);
				#undef VEC_FUNC
			}
			else
				return v;

			// clang-format on
		}

		/// \brief	Returns a copy of the vector with all scalar components set to the lowest integer not less than
		/// their original values.
		MUU_PURE_INLINE_GETTER
		constexpr vector ceil() const noexcept
		{
			if constexpr (is_floating_point<scalar_type>)
				return ceil(*this);
			else
				return *this;
		}

		/// \brief	Returns a vector with all scalar components set to the highest integer not greater than their
		/// original values.
		MUU_PURE_GETTER
		static constexpr vector MUU_VECTORCALL floor(MUU_VC_PARAM(vector) v) noexcept
		{
			// clang-format off

			if constexpr (is_floating_point<scalar_type>)
			{
				#define VEC_FUNC(member) muu::floor(v.member)
				COMPONENTWISE_CONSTRUCT(VEC_FUNC);
				#undef VEC_FUNC
			}
			else
				return v;

			// clang-format on
		}

		/// \brief	Returns a copy of the vector with all scalar components set to the highest integer not greater than
		/// their original values.
		MUU_PURE_INLINE_GETTER
		constexpr vector floor() const noexcept
		{
			if constexpr (is_floating_point<scalar_type>)
				return floor(*this);
			else
				return *this;
		}

#endif // misc
	};

	/// \cond

	MUU_CONSTRAINED_TEMPLATE((all_arithmetic<T, U, V...>), typename T, typename U, typename... V)
	vector(T, U, V...)->vector<impl::highest_ranked<T, U, V...>, 2 + sizeof...(V)>;

	MUU_CONSTRAINED_TEMPLATE(is_arithmetic<T>, typename T)
	vector(T)->vector<std::remove_cv_t<T>, 1>;

	template <typename T, size_t N>
	vector(const T (&)[N]) -> vector<std::remove_cv_t<T>, N>;

	template <typename T, size_t N>
	vector(const std::array<T, N>&) -> vector<std::remove_cv_t<T>, N>;

	template <typename T, typename U>
	vector(const std::pair<T, U>&) -> vector<impl::highest_ranked<T, U>, 2>;

	template <typename... T>
	vector(const std::tuple<T...>&) -> vector<impl::highest_ranked<T...>, sizeof...(T)>;

	template <typename S1, size_t D1, typename S2, size_t D2>
	vector(const vector<S1, D1>&, const vector<S2, D2>&) -> vector<impl::highest_ranked<S1, S2>, D1 + D2>;

	template <typename S, size_t D, typename... T>
	vector(const vector<S, D>&, T...) -> vector<impl::highest_ranked<S, T...>, D + sizeof...(T)>;

	MUU_CONSTRAINED_TEMPLATE(N != dynamic_extent, typename T, size_t N)
	vector(const muu::span<T, N>&)->vector<T, N>;

	template <typename I, size_t D>
	vector(const packed_unit_vector<I, D>&) -> vector<float, D>;

	/// \endcond
}

namespace std
{
	/// \brief Specialization of std::tuple_size for muu::vector.
	template <typename Scalar, size_t Dimensions>
	struct tuple_size<muu::vector<Scalar, Dimensions>>
	{
		static constexpr size_t value = Dimensions;
	};

	/// \brief Specialization of std::tuple_element for muu::vector.
	template <size_t I, typename Scalar, size_t Dimensions>
	struct tuple_element<I, muu::vector<Scalar, Dimensions>>
	{
		static_assert(I < Dimensions);
		using type = Scalar;
	};
}

//======================================================================================================================
// CONSTANTS
//======================================================================================================================

namespace muu
{
	MUU_PUSH_PRECISE_MATH;

	namespace impl
	{
		/// \cond

		template <typename Scalar, size_t Dimensions>
		struct integer_limits<vector<Scalar, Dimensions>>
		{
			using type	  = vector<Scalar, Dimensions>;
			using scalars = integer_limits<Scalar>;

			static constexpr type lowest  = type{ scalars::lowest };
			static constexpr type highest = type{ scalars::highest };
		};

		template <typename Scalar, size_t Dimensions>
		struct integer_positive_constants<vector<Scalar, Dimensions>>
		{
			using type	  = vector<Scalar, Dimensions>;
			using scalars = integer_positive_constants<Scalar>;

			static constexpr type zero		  = type{ scalars::zero };
			static constexpr type one		  = type{ scalars::one };
			static constexpr type two		  = type{ scalars::two };
			static constexpr type three		  = type{ scalars::three };
			static constexpr type four		  = type{ scalars::four };
			static constexpr type five		  = type{ scalars::five };
			static constexpr type six		  = type{ scalars::six };
			static constexpr type seven		  = type{ scalars::seven };
			static constexpr type eight		  = type{ scalars::eight };
			static constexpr type nine		  = type{ scalars::nine };
			static constexpr type ten		  = type{ scalars::ten };
			static constexpr type one_hundred = type{ scalars::one_hundred };
		};

		template <typename Scalar, size_t Dimensions>
		struct floating_point_traits<vector<Scalar, Dimensions>> : floating_point_traits<Scalar>
		{};

		template <typename Scalar, size_t Dimensions>
		struct floating_point_special_constants<vector<Scalar, Dimensions>>
		{
			using type	  = vector<Scalar, Dimensions>;
			using scalars = floating_point_special_constants<Scalar>;

			static constexpr type nan				= type{ scalars::nan };
			static constexpr type signaling_nan		= type{ scalars::signaling_nan };
			static constexpr type infinity			= type{ scalars::infinity };
			static constexpr type negative_infinity = type{ scalars::negative_infinity };
			static constexpr type negative_zero		= type{ scalars::negative_zero };
		};

		template <typename Scalar, size_t Dimensions>
		struct floating_point_named_constants<vector<Scalar, Dimensions>>
		{
			using type	  = vector<Scalar, Dimensions>;
			using scalars = floating_point_named_constants<Scalar>;

			static constexpr type one_over_two		   = type{ scalars::one_over_two };
			static constexpr type two_over_three	   = type{ scalars::two_over_three };
			static constexpr type two_over_five		   = type{ scalars::two_over_five };
			static constexpr type sqrt_two			   = type{ scalars::sqrt_two };
			static constexpr type one_over_sqrt_two	   = type{ scalars::one_over_sqrt_two };
			static constexpr type one_over_three	   = type{ scalars::one_over_three };
			static constexpr type three_over_two	   = type{ scalars::three_over_two };
			static constexpr type three_over_four	   = type{ scalars::three_over_four };
			static constexpr type three_over_five	   = type{ scalars::three_over_five };
			static constexpr type sqrt_three		   = type{ scalars::sqrt_three };
			static constexpr type one_over_sqrt_three  = type{ scalars::one_over_sqrt_three };
			static constexpr type pi				   = type{ scalars::pi };
			static constexpr type one_over_pi		   = type{ scalars::one_over_pi };
			static constexpr type pi_over_two		   = type{ scalars::pi_over_two };
			static constexpr type pi_over_three		   = type{ scalars::pi_over_three };
			static constexpr type pi_over_four		   = type{ scalars::pi_over_four };
			static constexpr type pi_over_five		   = type{ scalars::pi_over_five };
			static constexpr type pi_over_six		   = type{ scalars::pi_over_six };
			static constexpr type pi_over_seven		   = type{ scalars::pi_over_seven };
			static constexpr type pi_over_eight		   = type{ scalars::pi_over_eight };
			static constexpr type sqrt_pi			   = type{ scalars::sqrt_pi };
			static constexpr type one_over_sqrt_pi	   = type{ scalars::one_over_sqrt_pi };
			static constexpr type two_pi			   = type{ scalars::two_pi };
			static constexpr type sqrt_two_pi		   = type{ scalars::sqrt_two_pi };
			static constexpr type one_over_sqrt_two_pi = type{ scalars::one_over_sqrt_two_pi };
			static constexpr type one_over_three_pi	   = type{ scalars::one_over_three_pi };
			static constexpr type three_pi_over_two	   = type{ scalars::three_pi_over_two };
			static constexpr type three_pi_over_four   = type{ scalars::three_pi_over_four };
			static constexpr type three_pi_over_five   = type{ scalars::three_pi_over_five };
			static constexpr type e					   = type{ scalars::e };
			static constexpr type one_over_e		   = type{ scalars::one_over_e };
			static constexpr type e_over_two		   = type{ scalars::e_over_two };
			static constexpr type e_over_three		   = type{ scalars::e_over_three };
			static constexpr type e_over_four		   = type{ scalars::e_over_four };
			static constexpr type e_over_five		   = type{ scalars::e_over_five };
			static constexpr type e_over_six		   = type{ scalars::e_over_six };
			static constexpr type sqrt_e			   = type{ scalars::sqrt_e };
			static constexpr type one_over_sqrt_e	   = type{ scalars::one_over_sqrt_e };
			static constexpr type phi				   = type{ scalars::phi };
			static constexpr type one_over_phi		   = type{ scalars::one_over_phi };
			static constexpr type phi_over_two		   = type{ scalars::phi_over_two };
			static constexpr type phi_over_three	   = type{ scalars::phi_over_three };
			static constexpr type phi_over_four		   = type{ scalars::phi_over_four };
			static constexpr type phi_over_five		   = type{ scalars::phi_over_five };
			static constexpr type phi_over_six		   = type{ scalars::phi_over_six };
			static constexpr type sqrt_phi			   = type{ scalars::sqrt_phi };
			static constexpr type one_over_sqrt_phi	   = type{ scalars::one_over_sqrt_phi };
			static constexpr type degrees_to_radians   = type{ scalars::degrees_to_radians };
			static constexpr type radians_to_degrees   = type{ scalars::radians_to_degrees };
		};

		template <typename Scalar,
				  size_t Dimensions,
				  int = (is_floating_point<Scalar> ? 2 : (is_signed<Scalar> ? 1 : 0))>
		struct vector_constants_base : unsigned_integral_constants<vector<Scalar, Dimensions>>
		{};
		template <typename Scalar, size_t Dimensions>
		struct vector_constants_base<Scalar, Dimensions, 1> : signed_integral_constants<vector<Scalar, Dimensions>>
		{};
		template <typename Scalar, size_t Dimensions>
		struct vector_constants_base<Scalar, Dimensions, 2> : floating_point_constants<vector<Scalar, Dimensions>>
		{};

		/// \endcond

		template <typename Scalar, size_t Dimensions>
		struct unit_length_ge_1d_vector_constants
		{
			using scalars = muu::constants<Scalar>;

			/// \name Axes
			/// @{

			/// \brief	A unit-length vector representing the X axis.
			static constexpr vector<Scalar, Dimensions> x_axis{ scalars::one, scalars::zero };

			/// \brief	A unit-length vector representing the principal axis of the given dimension.
			template <size_t Dimension>
			static constexpr vector<Scalar, Dimensions> axis = POXY_IMPLEMENTATION_DETAIL( //
				[](auto dim_idx) constexpr noexcept {
					constexpr auto dim = remove_cvref<decltype(dim_idx)>::value;
					static_assert(dim < Dimensions, "Dimension index out of range");

					vector<Scalar, Dimensions> out{ scalars::zero };
					out.template get<dim>() = scalars::one;
					return out;
				}(index_tag<Dimension>{}));

			/// @}
		};

		template <typename Scalar, size_t Dimensions SPECIALIZED_IF(Dimensions >= 2)>
		struct unit_length_ge_2d_vector_constants
		{
			using scalars = muu::constants<Scalar>;

			/// \name Axes
			/// @{

			/// \brief	A unit-length vector representing the Y axis.
			static constexpr vector<Scalar, Dimensions> y_axis{ scalars::zero, scalars::one };

			/// @}
		};

		template <typename Scalar, size_t Dimensions SPECIALIZED_IF(Dimensions >= 3)>
		struct unit_length_ge_3d_vector_constants
		{
			using scalars = muu::constants<Scalar>;

			/// \name Axes
			/// @{

			/// \brief	A unit-length vector representing the Z axis.
			static constexpr vector<Scalar, Dimensions> z_axis{ scalars::zero, scalars::zero, scalars::one };

			/// @}
		};

		template <typename Scalar, size_t Dimensions SPECIALIZED_IF(Dimensions >= 4)>
		struct unit_length_ge_4d_vector_constants
		{
			using scalars = muu::constants<Scalar>;

			/// \name Axes
			/// @{

			/// \brief	A unit-length vector representing the W axis.
			static constexpr vector<Scalar, Dimensions> w_axis{ scalars::zero,
																scalars::zero,
																scalars::zero,
																scalars::one };

			/// @}
		};

		template <typename Scalar, size_t Dimensions SPECIALIZED_IF(Dimensions == 2)>
		struct unit_length_2d_vector_constants
		{
			using scalars = muu::constants<Scalar>;

			/// \name Directions (screen space)
			/// @{

			/// \brief	Right direction (in a top-down screen coordinate system).
			static constexpr vector<Scalar, Dimensions> screen_right{ scalars::one, scalars::zero };

			/// \brief	Down direction (in a top-down screen coordinate system).
			static constexpr vector<Scalar, Dimensions> screen_down{ scalars::zero, scalars::one };

			/// @}
		};

		template <typename Scalar, size_t Dimensions SPECIALIZED_IF(Dimensions == 2 && is_signed<Scalar>)>
		struct unit_length_2d_signed_vector_constants
		{
			using scalars = muu::constants<Scalar>;

			/// \name Directions (screen space)
			/// @{

			/// \brief	Left direction (in a top-down screen coordinate system).
			static constexpr vector<Scalar, Dimensions> screen_left{ -scalars::one, scalars::zero };

			/// \brief	Up direction (in a top-down screen coordinate system).
			static constexpr vector<Scalar, Dimensions> screen_up{ scalars::zero, -scalars::one };

			/// @}
		};

		template <typename Scalar, size_t Dimensions SPECIALIZED_IF(Dimensions == 3)>
		struct unit_length_3d_vector_constants
		{
			using scalars = muu::constants<Scalar>;

			/// \name Directions (world space)
			/// @{

			/// \brief	Backward direction (in a right-handed coordinate system).
			static constexpr vector<Scalar, Dimensions> backward{ scalars::zero, scalars::zero, scalars::one };

			/// @}
		};

		template <typename Scalar, size_t Dimensions SPECIALIZED_IF(Dimensions == 3 && is_signed<Scalar>)>
		struct unit_length_3d_signed_vector_constants
		{
			using scalars = muu::constants<Scalar>;

			/// \name Directions (world space)
			/// @{

			/// \brief	Forward direction (in a right-handed coordinate system).
			static constexpr vector<Scalar, Dimensions> forward{ scalars::zero, scalars::zero, -scalars::one };

			/// @}
		};

		template <typename Scalar, size_t Dimensions SPECIALIZED_IF(Dimensions == 2 || Dimensions == 3)>
		struct unit_length_2d_or_3d_vector_constants
		{
			using scalars = muu::constants<Scalar>;

			/// \name Directions (world space)
			/// @{

			/// \brief	Right direction (in a right-handed coordinate system).
			static constexpr vector<Scalar, Dimensions> right{ scalars::one, scalars::zero };

			/// \brief	Up direction (in a right-handed coordinate system).
			static constexpr vector<Scalar, Dimensions> up{ scalars::zero, scalars::one };

			/// @}
		};

		template <typename Scalar,
				  size_t Dimensions SPECIALIZED_IF((Dimensions == 2 || Dimensions == 3) && is_signed<Scalar>)>
		struct unit_length_2d_or_3d_signed_vector_constants
		{
			using scalars = muu::constants<Scalar>;

			/// \name Directions (world space)
			/// @{

			/// \brief	Left direction (in a right-handed coordinate system).
			static constexpr vector<Scalar, Dimensions> left{ -scalars::one, scalars::zero };

			/// \brief	Down direction (in a right-handed coordinate system).
			static constexpr vector<Scalar, Dimensions> down{ scalars::zero, -scalars::one };

			/// @}
		};

		template <typename Scalar, size_t Dimensions>
		struct unit_length_vector_constants : unit_length_ge_1d_vector_constants<Scalar, Dimensions>,
											  unit_length_ge_2d_vector_constants<Scalar, Dimensions>,
											  unit_length_ge_3d_vector_constants<Scalar, Dimensions>,
											  unit_length_ge_4d_vector_constants<Scalar, Dimensions>,
											  unit_length_2d_vector_constants<Scalar, Dimensions>,
											  unit_length_2d_signed_vector_constants<Scalar, Dimensions>,
											  unit_length_3d_vector_constants<Scalar, Dimensions>,
											  unit_length_3d_signed_vector_constants<Scalar, Dimensions>,
											  unit_length_2d_or_3d_vector_constants<Scalar, Dimensions>,
											  unit_length_2d_or_3d_signed_vector_constants<Scalar, Dimensions>
		{};

		/// \cond

		template <typename Scalar>
		struct unit_length_ge_1d_vector_constants<Scalar, 1>
		{
			using scalars = muu::constants<Scalar>;

			static constexpr vector<Scalar, 1> x_axis{ scalars::one };
		};

		template <typename Scalar, size_t Dimensions>
		struct unit_length_ge_2d_vector_constants<Scalar, Dimensions, false>
		{};

		template <typename Scalar, size_t Dimensions>
		struct unit_length_ge_3d_vector_constants<Scalar, Dimensions, false>
		{};

		template <typename Scalar, size_t Dimensions>
		struct unit_length_ge_4d_vector_constants<Scalar, Dimensions, false>
		{};

		template <typename Scalar, size_t Dimensions>
		struct unit_length_2d_vector_constants<Scalar, Dimensions, false>
		{};

		template <typename Scalar, size_t Dimensions>
		struct unit_length_2d_signed_vector_constants<Scalar, Dimensions, false>
		{};

		template <typename Scalar, size_t Dimensions>
		struct unit_length_3d_vector_constants<Scalar, Dimensions, false>
		{};

		template <typename Scalar, size_t Dimensions>
		struct unit_length_3d_signed_vector_constants<Scalar, Dimensions, false>
		{};

		template <typename Scalar, size_t Dimensions>
		struct unit_length_2d_or_3d_vector_constants<Scalar, Dimensions, false>
		{};

		template <typename Scalar, size_t Dimensions>
		struct unit_length_2d_or_3d_signed_vector_constants<Scalar, Dimensions, false>
		{};

		/// \endcond
	}

#if MUU_DOXYGEN
	#define VECTOR_CONSTANTS_BASES                                                                                     \
		impl::unit_length_vector_constants<Scalar, Dimensions>, impl::integer_limits<vector<Scalar, Dimensions>>,      \
			impl::integer_positive_constants<vector<Scalar, Dimensions>>,                                              \
			impl::floating_point_traits<vector<Scalar, Dimensions>>,                                                   \
			impl::floating_point_special_constants<vector<Scalar, Dimensions>>,                                        \
			impl::floating_point_named_constants<vector<Scalar, Dimensions>>
#else
	#define VECTOR_CONSTANTS_BASES                                                                                     \
		impl::unit_length_vector_constants<Scalar, Dimensions>, impl::vector_constants_base<Scalar, Dimensions>
#endif

	/// \ingroup	constants
	/// \see		muu::vector
	///
	/// \brief		Vector constants.
	template <typename Scalar, size_t Dimensions>
	struct constants<vector<Scalar, Dimensions>> : VECTOR_CONSTANTS_BASES
	{};

#undef VECTOR_CONSTANTS_BASES

	MUU_POP_PRECISE_MATH;
}

//======================================================================================================================
// ACCUMULATOR
//======================================================================================================================

namespace muu::impl
{
	template <typename Scalar, size_t Dimensions>
	struct vector_accumulator
	{
		using value_type		 = vector<Scalar, Dimensions>;
		using value_param		 = vectorcall_param<value_type>;
		using scalar_accumulator = typename default_accumulator<Scalar>::type;

		scalar_accumulator accumulators[Dimensions];

		constexpr void MUU_VECTORCALL start(value_param sample) noexcept
		{
			accumulators[0].start(sample.template get<0>());
			if constexpr (Dimensions > 1)
				accumulators[1].start(sample.template get<1>());
			if constexpr (Dimensions > 2)
				accumulators[2].start(sample.template get<2>());
			if constexpr (Dimensions > 3)
				accumulators[3].start(sample.template get<3>());
			if constexpr (Dimensions > 4)
			{
				for (size_t i = 4; i < Dimensions; i++)
					accumulators[i].start(sample[i]);
			}
		}

		constexpr void MUU_VECTORCALL add(value_param sample) noexcept
		{
			accumulators[0].add(sample.template get<0>());
			if constexpr (Dimensions > 1)
				accumulators[1].add(sample.template get<1>());
			if constexpr (Dimensions > 2)
				accumulators[2].add(sample.template get<2>());
			if constexpr (Dimensions > 3)
				accumulators[3].add(sample.template get<3>());
			if constexpr (Dimensions > 4)
			{
				for (size_t i = 4; i < Dimensions; i++)
					accumulators[i].add(sample[i]);
			}
		}

		constexpr void add(const vector_accumulator& other) noexcept
		{
			for (size_t i = 0; i < Dimensions; i++)
				accumulators[i].add(other.accumulators[i]);
		}

	  private:
		/// \cond

		template <size_t... Indices, typename Func>
		MUU_PURE_GETTER
		constexpr auto componentwise(std::index_sequence<Indices...>, Func&& func) const noexcept
		{
			return vector{ func(accumulators[Indices])... };
		}

	  public:
		MUU_PURE_GETTER
		constexpr value_type min() const noexcept
		{
			return componentwise(std::make_index_sequence<Dimensions>{}, [](auto& acc) noexcept { return acc.min(); });
		}

		MUU_PURE_GETTER
		constexpr value_type max() const noexcept
		{
			return componentwise(std::make_index_sequence<Dimensions>{}, [](auto& acc) noexcept { return acc.max(); });
		}

		using sum_type = vector<decltype(std::declval<scalar_accumulator>().sum()), Dimensions>;

		MUU_PURE_GETTER
		constexpr sum_type sum() const noexcept
		{
			return componentwise(std::make_index_sequence<Dimensions>{}, [](auto& acc) noexcept { return acc.sum(); });
		}

		/// \endcond
	};
}

//======================================================================================================================
// FREE FUNCTIONS
//======================================================================================================================

namespace muu
{
	/// \ingroup	infinity_or_nan
	/// \relatesalso	muu::vector
	///
	/// \brief	Returns true if any of the scalar components of a vector are infinity or NaN.
	template <typename S, size_t D>
	MUU_PURE_INLINE_GETTER
	constexpr bool infinity_or_nan(const vector<S, D>& v) noexcept
	{
		if constexpr (is_floating_point<S>)
			return vector<S, D>::infinity_or_nan(v);
		else
		{
			MUU_UNUSED(v);
			return false;
		}
	}

	/// \ingroup	approx_equal
	/// \relatesalso	muu::vector
	///
	/// \brief		Returns true if two vectors are approximately equal.
	///
	/// \availability		This function is only available when at least one of `S` and `T` is a floating-point type.
	MUU_CONSTRAINED_TEMPLATE((any_floating_point<S, T>), typename S, typename T, size_t D)
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL approx_equal(const vector<S, D>& v1,
											   const vector<T, D>& v2,
											   epsilon_type<S, T> epsilon = default_epsilon<S, T>) noexcept
	{
		return vector<S, D>::approx_equal(v1, v2, epsilon);
	}

	/// \ingroup	approx_zero
	/// \relatesalso	muu::vector
	///
	/// \brief		Returns true if all the scalar components of a vector are approximately equal to zero.
	///
	/// \availability		This function is only available when `S` is a floating-point type.
	MUU_CONSTRAINED_TEMPLATE((is_floating_point<S>), typename S, size_t D)
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL approx_zero(const vector<S, D>& v, S epsilon = default_epsilon<S>) noexcept
	{
		return vector<S, D>::approx_zero(v, epsilon);
	}

	/// \ingroup	normalized
	/// \relatesalso	muu::vector
	///
	/// \brief Returns true if a vector is normalized (i.e. has a length of 1).
	template <typename S, size_t D>
	MUU_PURE_INLINE_GETTER
	constexpr bool normalized(const vector<S, D>& v) noexcept
	{
		return vector<S, D>::normalized(v);
	}

	/// \relatesalso muu::vector
	///
	/// \brief	Returns the squared length of a vector.
	template <typename S,
			  size_t D //
				  MUU_HIDDEN_PARAM(typename delta_scalar_type = typename vector<S, D>::delta_scalar_type)>
	MUU_PURE_INLINE_GETTER
	constexpr delta_scalar_type length_squared(const vector<S, D>& v) noexcept
	{
		return vector<S, D>::length_squared(v);
	}

	/// \relatesalso muu::vector
	///
	/// \brief	Returns the length (magnitude) of a vector.
	template <typename S,
			  size_t D //
				  MUU_HIDDEN_PARAM(typename delta_scalar_type = typename vector<S, D>::delta_scalar_type)>
	MUU_PURE_INLINE_GETTER
	constexpr delta_scalar_type length(const vector<S, D>& v) noexcept
	{
		return vector<S, D>::length(v);
	}

	/// \relatesalso muu::vector
	///
	/// \brief	Returns the length (magnitude) of a vector.
	///
	/// \warning	This function is implemented such that it is _always_ available at compile time,
	///				arriving at the result using very slow iterative machinery. Do not use it at runtime!
	template <typename S,
			  size_t D //
				  MUU_HIDDEN_PARAM(typename delta_scalar_type = typename vector<S, D>::delta_scalar_type)>
	MUU_PURE_INLINE_GETTER
	MUU_CONSTEVAL
	delta_scalar_type consteval_length(const vector<S, D>& v) noexcept
	{
		return vector<S, D>::consteval_length(v);
	}

	/// \relatesalso muu::vector
	///
	/// \brief	Returns the squared distance between two points.
	template <typename S,
			  size_t D //
				  MUU_HIDDEN_PARAM(typename delta_scalar_type = typename vector<S, D>::delta_scalar_type)>
	MUU_PURE_INLINE_GETTER
	constexpr delta_scalar_type distance_squared(const vector<S, D>& p1, const vector<S, D>& p2) noexcept
	{
		return vector<S, D>::distance_squared(p1, p2);
	}

	/// \relatesalso muu::vector
	///
	/// \brief	Returns the distance between two points.
	template <typename S,
			  size_t D //
				  MUU_HIDDEN_PARAM(typename delta_scalar_type = typename vector<S, D>::delta_scalar_type)>
	MUU_PURE_INLINE_GETTER
	constexpr delta_scalar_type distance(const vector<S, D>& p1, const vector<S, D>& p2) noexcept
	{
		return vector<S, D>::distance(p1, p2);
	}

	/// \relatesalso muu::vector
	///
	/// \brief	Returns the distance between two points.
	///
	/// \warning	This function is implemented such that it is _always_ available at compile time,
	///				arriving at the result using very slow iterative machinery. Do not use it at runtime!
	template <typename S,
			  size_t D //
				  MUU_HIDDEN_PARAM(typename delta_scalar_type = typename vector<S, D>::delta_scalar_type)>
	MUU_PURE_INLINE_GETTER
	MUU_CONSTEVAL
	delta_scalar_type consteval_distance(const vector<S, D>& p1, const vector<S, D>& p2) noexcept
	{
		return vector<S, D>::consteval_distance(p1, p2);
	}

	/// \relatesalso muu::vector
	///
	/// \brief	Returns the dot product of two vectors.
	template <typename S,
			  size_t D //
				  MUU_HIDDEN_PARAM(typename product_scalar_type = typename vector<S, D>::product_scalar_type)>
	MUU_PURE_INLINE_GETTER
	constexpr product_scalar_type dot(const vector<S, D>& v1, const vector<S, D>& v2) noexcept
	{
		return vector<S, D>::dot(v1, v2);
	}

	/// \relatesalso muu::vector
	///
	/// \brief	Returns the cross product of two three-dimensional vectors.
	template <typename S //
				  MUU_HIDDEN_PARAM(typename product_type = typename vector<S, 3>::product_type)>
	MUU_PURE_INLINE_GETTER
	constexpr product_type cross(const vector<S, 3>& lhs, const vector<S, 3>& rhs) noexcept
	{
		return vector<S, 3>::cross(lhs, rhs);
	}

	/// \relatesalso muu::vector
	///
	/// \brief	Normalizes a vector.
	///
	/// \param v			The vector to normalize.
	/// \param length_out	An output param to receive the length of the vector pre-normalization.
	///
	/// \return		A normalized copy of the input vector.
	///
	/// \availability This function is only available when `S` is a floating-point type.
	MUU_CONSTRAINED_TEMPLATE(
		is_floating_point<S>,
		typename S,
		size_t D //
			MUU_HIDDEN_PARAM(typename delta_scalar_type = typename vector<S, D>::delta_scalar_type))
	MUU_NODISCARD
	MUU_ALWAYS_INLINE
	constexpr vector<S, D> normalize(const vector<S, D>& v, delta_scalar_type& length_out) noexcept
	{
		return vector<S, D>::normalize(v, length_out);
	}

	/// \relatesalso muu::vector
	///
	/// \brief	Normalizes a vector.
	///
	/// \param v	The vector to normalize.
	///
	/// \return		A normalized copy of the input vector.
	///
	/// \availability This function is only available when `S` is a floating-point type.
	MUU_CONSTRAINED_TEMPLATE(is_floating_point<S>, typename S, size_t D)
	MUU_PURE_INLINE_GETTER
	constexpr vector<S, D> normalize(const vector<S, D>& v) noexcept
	{
		return vector<S, D>::normalize(v);
	}

	/// \relatesalso muu::vector
	///
	/// \brief	Normalizes a vector.
	///
	/// \param v	The vector to normalize.
	///
	/// \return		A normalized copy of the input vector.
	///
	/// \availability This function is only available when `S` is a floating-point type.
	///
	/// \warning	This function is implemented such that it is _always_ available at compile time,
	///				arriving at the result using very slow iterative machinery. Do not use it at runtime!
	MUU_CONSTRAINED_TEMPLATE(is_floating_point<S>, typename S, size_t D)
	MUU_PURE_INLINE_GETTER
	MUU_CONSTEVAL
	vector<S, D> consteval_normalize(const vector<S, D>& v) noexcept
	{
		return vector<S, D>::consteval_normalize(v);
	}

	/// \relatesalso muu::vector
	///
	/// \brief		Returns the normalized direction vector from one position to another.
	///
	/// \param	from			The start position.
	/// \param	to				The end position.
	/// \param	distance_out	An output param to receive the distance between the two points.
	///
	/// \return		A normalized direction vector pointing from the start position to the end position.
	///
	/// \availability		This function is only available when `D` == 2 or 3.
	MUU_CONSTRAINED_TEMPLATE((D == 2 || D == 3),
							 typename S,
							 size_t D																	   //
								 MUU_HIDDEN_PARAM(typename delta_type = typename vector<S, D>::delta_type) //
							 MUU_HIDDEN_PARAM(typename delta_scalar_type = typename vector<S, D>::delta_scalar_type))
	MUU_NODISCARD
	MUU_ALWAYS_INLINE
	constexpr delta_type direction(const vector<S, D>& from,
								   const vector<S, D>& to,
								   delta_scalar_type& distance_out) noexcept
	{
		return vector<S, D>::direction(from, to, distance_out);
	}

	/// \relatesalso muu::vector
	///
	/// \brief		Returns the normalized direction vector from one position to another.
	///
	/// \param	from			The start position.
	/// \param	to				The end position.
	///
	/// \return		A normalized direction vector pointing from the start position to the end position.
	///
	/// \availability		This function is only available when `D` == 2 or 3.
	MUU_CONSTRAINED_TEMPLATE((D == 2 || D == 3),
							 typename S,
							 size_t D //
								 MUU_HIDDEN_PARAM(typename delta_type = typename vector<S, D>::delta_type))
	MUU_PURE_INLINE_GETTER
	constexpr delta_type direction(const vector<S, D>& from, const vector<S, D>& to) noexcept
	{
		return vector<S, D>::direction(from, to);
	}

	/// \relatesalso muu::vector
	///
	/// \brief	Returns the componentwise minimum of two or more vectors.
	MUU_CONSTRAINED_TEMPLATE((sizeof...(T) == 0 || all_convertible_to<vector<S, D>, const T&...>),
							 typename S,
							 size_t D,
							 typename... T)
	MUU_PURE_INLINE_GETTER
	constexpr vector<S, D> min(const vector<S, D>& v1, const vector<S, D>& v2, const T&... vecs) noexcept
	{
		return vector<S, D>::min(v1, v2, vecs...);
	}

	/// \relatesalso muu::vector
	///
	/// \brief	Returns the componentwise maximum of two or more vectors.
	MUU_CONSTRAINED_TEMPLATE((sizeof...(T) == 0 || all_convertible_to<vector<S, D>, const T&...>),
							 typename S,
							 size_t D,
							 typename... T)
	MUU_PURE_INLINE_GETTER
	constexpr vector<S, D> max(const vector<S, D>& v1, const vector<S, D>& v2, const T&... vecs) noexcept
	{
		return vector<S, D>::max(v1, v2, vecs...);
	}

	/// \relatesalso muu::vector
	///
	/// \brief	Componentwise clamps a vector between two others.
	template <typename S, size_t D>
	MUU_PURE_INLINE_GETTER
	constexpr vector<S, D> clamp(const vector<S, D>& v, const vector<S, D>& low, const vector<S, D>& high) noexcept
	{
		return vector<S, D>::clamp(v, low, high);
	}

	/// \ingroup lerp
	/// \relatesalso muu::vector
	///
	/// \brief	Performs a linear interpolation between two vectors.
	template <typename S,
			  size_t D //
				  MUU_HIDDEN_PARAM(typename delta_scalar_type = typename vector<S, D>::delta_scalar_type)>
	MUU_PURE_INLINE_GETTER
	constexpr vector<S, D> MUU_VECTORCALL lerp(const vector<S, D>& start,
											   const vector<S, D>& finish,
											   delta_scalar_type alpha) noexcept
	{
		return vector<S, D>::lerp(start, finish, alpha);
	}

	/// \relatesalso muu::vector
	///
	/// \brief	Calculates the angle between two vectors.
	///
	/// \param	v1		A vector.
	/// \param	v2		A vector.
	///
	/// \returns	The angle between `v1` and `v2` (in radians).
	///
	/// \remarks	The angle returned is the unsigned angle between the two vectors;
	/// 			the smaller of the two possible angles between the two vectors is used.
	/// 			The result is never greater than `pi` radians (180 degrees).
	///
	/// \availability		This function is only available when `D` == 2 or 3.
	MUU_CONSTRAINED_TEMPLATE(
		(D == 2 || D == 3),
		typename S,
		size_t D //
			MUU_HIDDEN_PARAM(typename delta_scalar_type = typename vector<S, D>::delta_scalar_type))
	MUU_PURE_INLINE_GETTER
	constexpr delta_scalar_type angle(const vector<S, D>& v1, const vector<S, D>& v2) noexcept
	{
		return vector<S, D>::angle(v1, v2);
	}

	/// \ingroup abs
	/// \relatesalso muu::vector
	///
	/// \brief	Returns a copy of a vector with all scalar components set to their absolute values.
	template <typename S, size_t D>
	MUU_PURE_INLINE_GETTER
	constexpr vector<S, D> abs(const vector<S, D>& v) noexcept
	{
		return vector<S, D>::abs(v);
	}

	/// \ingroup ceil
	/// \relatesalso muu::vector
	///
	/// \brief	Returns a copy of a vector with all scalar components set to the lowest integer not less than their
	/// original values.
	template <typename S, size_t D>
	MUU_PURE_INLINE_GETTER
	constexpr vector<S, D> ceil(const vector<S, D>& v) noexcept
	{
		return vector<S, D>::ceil(v);
	}

	/// \ingroup floor
	/// \relatesalso muu::vector
	///
	/// \brief	Returns a copy of a vector with all scalar components set to the highest integer not greater than their
	/// original values.
	template <typename S, size_t D>
	MUU_PURE_INLINE_GETTER
	constexpr vector<S, D> floor(const vector<S, D>& v) noexcept
	{
		return vector<S, D>::floor(v);
	}
}

#undef COMPONENTWISE_AND
#undef COMPONENTWISE_OR
#undef COMPONENTWISE_ACCUMULATE
#undef COMPONENTWISE_CASTING_OP_BRANCH
#undef COMPONENTWISE_CASTING_OP
#undef COMPONENTWISE_CONSTRUCT_WITH_TRANSFORM
#undef COMPONENTWISE_CONSTRUCT
#undef COMPONENTWISE_ASSIGN_WITH_TRANSFORM
#undef COMPONENTWISE_ASSIGN
#undef IDENTITY_TRANSFORM
#undef SPECIALIZED_IF

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
