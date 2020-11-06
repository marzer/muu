// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::matrix.

#pragma once
#include "../muu/vector.h"
#include "../muu/quaternion.h"

MUU_DISABLE_WARNINGS
#include <iosfwd>
#include <tuple>
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

#if 1 // helper macros ------------------------------------------------------------------------------------------------

#define	REQUIRES_SIZE_AT_LEAST(n)			MUU_REQUIRES(Rows * Columns >= (n))
#define	REQUIRES_SIZE_AT_LEAST_AND(n, ...)	MUU_REQUIRES(Rows * Columns >= (n) && (__VA_ARGS__))
#define	REQUIRES_SIZE_EXACTLY(n)			MUU_REQUIRES(Rows * Columns == (n))
#define	REQUIRES_ROWS_AT_LEAST(n)			MUU_REQUIRES(Rows >= (n))
#define	REQUIRES_ROWS_EXACTLY(n)			MUU_REQUIRES(Rows == (n))
#define	REQUIRES_COLUMNS_AT_LEAST(n)		MUU_REQUIRES(Columns >= (n))
#define	REQUIRES_COLUMNS_EXACTLY(n)			MUU_REQUIRES(Columns == (n))
#define	REQUIRES_DIMENSIONS_AT_LEAST(r, c)	MUU_REQUIRES(Rows >= (r) && Columns >= (c))
#define	REQUIRES_DIMENSIONS_EXACTLY(r, c)	MUU_REQUIRES(Rows == (r) && Columns == (c))
#define	REQUIRES_FLOATING_POINT				MUU_REQUIRES(muu::is_floating_point<Scalar>)
#define	REQUIRES_INTEGRAL					MUU_REQUIRES(muu::is_integral<Scalar>)
#define	REQUIRES_SIGNED						MUU_REQUIRES(muu::is_signed<Scalar>)

#if !MUU_CONCEPTS

	#define	SFINAE_REQUIRES_SIZE_AT_LEAST(n)						\
		template <size_t SFINAE = Rows * Columns MUU_ENABLE_IF(		\
			SFINAE >= (n)											\
			&& SFINAE == (Rows * Columns)							\
		)>

	#define	SFINAE_REQUIRES_SIZE_AT_LEAST_AND(n, ...)				\
		template <size_t SFINAE = Rows * Columns MUU_ENABLE_IF(		\
			SFINAE >= (n)											\
			&& SFINAE == (Rows * Columns)							\
			&& (__VA_ARGS__)										\
		)>

	#define	SFINAE_REQUIRES_SIZE_EXACTLY(n)							\
		template <size_t SFINAE = Rows * Columns MUU_ENABLE_IF_2(	\
			SFINAE == (n)											\
			&& SFINAE == (Rows * Columns)							\
		)>

	#define	SFINAE_REQUIRES_ROWS_AT_LEAST(n)						\
		template <size_t SFINAE = Rows MUU_ENABLE_IF(				\
			SFINAE >= (n)											\
			&& SFINAE == Rows										\
		)>

	#define	SFINAE_REQUIRES_ROWS_EXACTLY(n)							\
		template <size_t SFINAE = Rows MUU_ENABLE_IF_2(				\
			SFINAE == (n)											\
			&& SFINAE == Rows										\
		)>

	#define	SFINAE_REQUIRES_COLUMNS_AT_LEAST(n)						\
		template <size_t SFINAE = Columns MUU_ENABLE_IF(			\
			SFINAE >= (n)											\
			&& SFINAE == Columns									\
		)>

	#define	SFINAE_REQUIRES_COLUMNS_EXACTLY(n)						\
		template <size_t SFINAE = Columns MUU_ENABLE_IF_2(			\
			SFINAE == (n)											\
			&& SFINAE == Columns									\
		)>

	#define	SFINAE_REQUIRES_DIMENSIONS_AT_LEAST(r, c)								\
		template <size_t SFINAE_R = Rows, size_t SFINAE_C = Columns MUU_ENABLE_IF(	\
			SFINAE_R >= (r)															\
			&& SFINAE_C >= (c)														\
			&& SFINAE_R == Rows														\
			&& SFINAE_C == Columns													\
		)>

	#define	SFINAE_REQUIRES_DIMENSIONS_EXACTLY(r, c)								\
		template <size_t SFINAE_R = Rows, size_t SFINAE_C = Columns MUU_ENABLE_IF_2(\
			SFINAE_R == (r)															\
			&& SFINAE_C == (c)														\
			&& SFINAE_R == Rows														\
			&& SFINAE_C == Columns													\
		)>

	#define	SFINAE_REQUIRES_FLOATING_POINT							\
		template <typename SFINAE = Scalar MUU_ENABLE_IF(			\
			muu::is_floating_point<SFINAE>							\
			&& std::is_same_v<SFINAE, Scalar>						\
		)>

	#define	SFINAE_REQUIRES_INTEGRAL								\
		template <typename SFINAE = Scalar MUU_ENABLE_IF(			\
			muu::is_integral<SFINAE>								\
			&& std::is_same_v<SFINAE, Scalar>						\
		)>

	#define	SFINAE_REQUIRES_SIGNED									\
		template <typename SFINAE = Scalar MUU_ENABLE_IF(			\
			muu::is_signed<SFINAE>									\
			&& std::is_same_v<SFINAE, Scalar>						\
		)>

#endif // !MUU_CONCEPTS

#endif // helper macros

MUU_IMPL_NAMESPACE_START
{
	struct columnwise_init_tag {};
	struct row_major_tuple_tag {};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct MUU_TRIVIAL_ABI matrix_base
	{
		vector<Scalar, Rows> m[Columns];

		matrix_base() noexcept = default;

		template <size_t... ColumnIndices>
		explicit
		constexpr matrix_base(Scalar fill, std::index_sequence<ColumnIndices...>) noexcept
			: m{ ((void)ColumnIndices, vector<Scalar, Rows>{ fill })... }
		{
			static_assert(sizeof...(ColumnIndices) <= Columns);
		}

		explicit
		constexpr matrix_base(value_fill_tag, Scalar fill) noexcept
			: matrix_base{ fill, std::make_index_sequence<Columns>{} }
		{}

		template <typename... T>
		explicit
		constexpr matrix_base(columnwise_init_tag, T... cols) noexcept
			: m{ cols... }
		{
			static_assert(sizeof...(T) <= Columns);
		}

	private:

		template <size_t Index, typename T>
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(pure)
		static constexpr decltype(auto) get_tuple_value_or_zero(const T& tpl) noexcept
		{
			if constexpr (Index < tuple_size<T>)
			{
				return get_from_tuple_like<Index>(tpl);
			}
			else
			{
				(void)tpl;
				return Scalar{};
			}
		}

		template <size_t Column, typename T, size_t... RowIndices>
		[[nodiscard]]
		MUU_ATTR(pure)
		MUU_ATTR(flatten)
		static constexpr vector<Scalar, Rows> column_from_row_major_tuple(const T& tpl, std::index_sequence<RowIndices...>) noexcept
		{
			static_assert(sizeof...(RowIndices) == Rows);

			return vector<Scalar, Rows>
			{
				static_cast<Scalar>(get_tuple_value_or_zero<Column + (Columns * RowIndices)>(tpl))...
			};
		}

	public:

		template <typename T, size_t... ColumnIndices>
		explicit
		constexpr matrix_base(const T& tpl, std::index_sequence<ColumnIndices...>) noexcept
			: m{ column_from_row_major_tuple<ColumnIndices>(tpl, std::make_index_sequence<Rows>{})... }
		{
			static_assert(tuple_size<T> <= Rows * Columns);
			static_assert(sizeof...(ColumnIndices) == Columns);
		}

		template <typename T>
		explicit
		constexpr matrix_base(row_major_tuple_tag, const T& tpl) noexcept
			: matrix_base{ tpl, std::make_index_sequence<Columns>{} }
		{
			static_assert(tuple_size<T> <= Rows * Columns);
		}
	};

	#if MUU_HAS_VECTORCALL

	template <typename Scalar, size_t Rows, size_t Columns>
	inline constexpr bool is_hva<matrix_base<Scalar, Rows, Columns>> = can_be_hva_of<matrix_base<Scalar, Rows, Columns>, Scalar>;

	template <typename Scalar, size_t Rows, size_t Columns>
	inline constexpr bool is_hva<matrix<Scalar, Rows, Columns>> = is_hva<matrix_base<Scalar, Rows, Columns>>;

	#endif // MUU_HAS_VECTORCALL

	template <typename Scalar, size_t Rows, size_t Columns>
	struct readonly_param_<matrix<Scalar, Rows, Columns>>
	{
		using type = std::conditional_t<
			pass_readonly_by_value<matrix_base<Scalar, Rows, Columns>>,
			matrix<Scalar, Rows, Columns>,
			const matrix<Scalar, Rows, Columns>&
		>;
	};

	MUU_API void print_matrix_to_stream(std::ostream& stream, const half*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::ostream& stream, const float*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::ostream& stream, const double*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::ostream& stream, const long double*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::ostream& stream, const signed char*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::ostream& stream, const signed short*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::ostream& stream, const signed int*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::ostream& stream, const signed long*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::ostream& stream, const signed long long*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::ostream& stream, const unsigned char*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::ostream& stream, const unsigned short*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::ostream& stream, const unsigned int*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::ostream& stream, const unsigned long*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::ostream& stream, const unsigned long long*, size_t, size_t);
	#if MUU_HAS_FLOAT16
	MUU_API void print_matrix_to_stream(std::ostream& stream, const _Float16*, size_t, size_t);
	#endif
	#if MUU_HAS_FP16
	MUU_API void print_matrix_to_stream(std::ostream& stream, const __fp16*, size_t, size_t);
	#endif
	#if MUU_HAS_FLOAT128
	MUU_API void print_matrix_to_stream(std::ostream& stream, const float128_t*, size_t, size_t);
	#endif
	#if MUU_HAS_INT128
	MUU_API void print_matrix_to_stream(std::ostream& stream, const int128_t*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::ostream& stream, const uint128_t*, size_t, size_t);
	#endif

	MUU_API void print_matrix_to_stream(std::wostream& stream, const half*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::wostream& stream, const float*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::wostream& stream, const double*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::wostream& stream, const long double*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::wostream& stream, const signed char*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::wostream& stream, const signed short*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::wostream& stream, const signed int*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::wostream& stream, const signed long*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::wostream& stream, const signed long long*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::wostream& stream, const unsigned char*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::wostream& stream, const unsigned short*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::wostream& stream, const unsigned int*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::wostream& stream, const unsigned long*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::wostream& stream, const unsigned long long*, size_t, size_t);
	#if MUU_HAS_FLOAT16
	MUU_API void print_matrix_to_stream(std::wostream& stream, const _Float16*, size_t, size_t);
	#endif
	#if MUU_HAS_FP16
	MUU_API void print_matrix_to_stream(std::wostream& stream, const __fp16*, size_t, size_t);
	#endif
	#if MUU_HAS_FLOAT128
	MUU_API void print_matrix_to_stream(std::wostream& stream, const float128_t*, size_t, size_t);
	#endif
	#if MUU_HAS_INT128
	MUU_API void print_matrix_to_stream(std::wostream& stream, const int128_t*, size_t, size_t);
	MUU_API void print_matrix_to_stream(std::wostream& stream, const uint128_t*, size_t, size_t);
	#endif
}
MUU_IMPL_NAMESPACE_END

#else // ^^^ !DOXYGEN / DOXYGEN vvv

#define	REQUIRES_SIZE_AT_LEAST(n)
#define	REQUIRES_SIZE_AT_LEAST_AND(n, ...)
#define	REQUIRES_SIZE_EXACTLY(n)
#define	REQUIRES_ROWS_AT_LEAST(n)
#define	REQUIRES_ROWS_EXACTLY(n)
#define	REQUIRES_COLUMNS_AT_LEAST(n)
#define	REQUIRES_COLUMNS_EXACTLY(n)
#define	REQUIRES_DIMENSIONS_AT_LEAST(r, c)
#define	REQUIRES_DIMENSIONS_EXACTLY(r, c)
#define	REQUIRES_FLOATING_POINT
#define	REQUIRES_INTEGRAL
#define	REQUIRES_SIGNED

#endif // DOXYGEN

#ifndef SFINAE_REQUIRES_SIZE_AT_LEAST
	#define SFINAE_REQUIRES_SIZE_AT_LEAST(n)
#endif
#ifndef SFINAE_REQUIRES_SIZE_AT_LEAST_AND
	#define SFINAE_REQUIRES_SIZE_AT_LEAST_AND(n, ...)
#endif
#ifndef SFINAE_REQUIRES_SIZE_EXACTLY
	#define SFINAE_REQUIRES_SIZE_EXACTLY(n)
#endif
#ifndef SFINAE_REQUIRES_ROWS_AT_LEAST
	#define SFINAE_REQUIRES_ROWS_AT_LEAST(n)
#endif
#ifndef SFINAE_REQUIRES_ROWS_EXACTLY
	#define SFINAE_REQUIRES_ROWS_EXACTLY(n)
#endif
#ifndef SFINAE_REQUIRES_COLUMNS_AT_LEAST
	#define SFINAE_REQUIRES_COLUMNS_AT_LEAST(n)
#endif
#ifndef SFINAE_REQUIRES_COLUMNS_EXACTLY
	#define SFINAE_REQUIRES_COLUMNS_EXACTLY(n)
#endif
#ifndef SFINAE_REQUIRES_DIMENSIONS_AT_LEAST
	#define	SFINAE_REQUIRES_DIMENSIONS_AT_LEAST(r, c)
#endif
#ifndef SFINAE_REQUIRES_DIMENSIONS_EXACTLY
	#define	SFINAE_REQUIRES_DIMENSIONS_EXACTLY(r, c)
#endif
#ifndef SFINAE_REQUIRES_FLOATING_POINT
	#define SFINAE_REQUIRES_FLOATING_POINT
#endif
#ifndef SFINAE_REQUIRES_INTEGRAL
	#define SFINAE_REQUIRES_INTEGRAL
#endif
#ifndef SFINAE_REQUIRES_SIGNED
	#define SFINAE_REQUIRES_SIGNED
#endif
#if !defined(DOXYGEN) && !MUU_INTELLISENSE
	#define ENABLE_PAIRED_FUNCS 1

	#define ENABLE_PAIRED_FUNC_BY_REF(S, R, C, ...) \
			MUU_ENABLE_IF(impl::pass_readonly_by_reference<matrix<S, R, C>> && (__VA_ARGS__))

	#define ENABLE_PAIRED_FUNC_BY_VAL(S, R, C, ...) \
			MUU_ENABLE_IF_2(impl::pass_readonly_by_value<matrix<S, R, C>> && (__VA_ARGS__))

	#define REQUIRES_PAIRED_FUNC_BY_REF(S, R, C, ...) \
			MUU_REQUIRES(impl::pass_readonly_by_reference<matrix<S, R, C>> && (__VA_ARGS__))

	#define REQUIRES_PAIRED_FUNC_BY_VAL(S, R, C, ...) \
			MUU_REQUIRES(impl::pass_readonly_by_value<matrix<S, R, C>> && (__VA_ARGS__))

#else
	#define ENABLE_PAIRED_FUNCS 0
	#define ENABLE_PAIRED_FUNC_BY_REF(S, R, C, ...)
	#define ENABLE_PAIRED_FUNC_BY_VAL(S, R, C, ...)
	#define REQUIRES_PAIRED_FUNC_BY_REF(S, R, C, ...)
	#define REQUIRES_PAIRED_FUNC_BY_VAL(S, R, C, ...)
#endif

#endif // =============================================================================================================

//=====================================================================================================================
// MATRIX CLASS
#if 1

MUU_NAMESPACE_START
{
	/// \brief Alias of `matrix` or `const matrix&`, depending on size, triviality, simd-friendliness, etc.
	/// \ingroup math
	///
	/// \related muu::matrix
	/// \see muu::matrix
	template <typename Scalar, size_t Rows, size_t Columns>
	using matrix_param = impl::readonly_param<matrix<Scalar, Rows, Columns>>;

	/// \brief A matrix.
	/// \ingroup math
	/// 
	/// \tparam	Scalar      The type of the matrix's scalar components.
	/// \tparam Rows		The number of rows in the matrix.
	/// \tparam Columns		The number of columns in the matrix.
	template <typename Scalar, size_t Rows, size_t Columns>
	struct MUU_TRIVIAL_ABI matrix
		#ifndef DOXYGEN
		: impl::matrix_base<Scalar, Rows, Columns>
		#endif
	{
		static_assert(
			!std::is_reference_v<Scalar>,
			"Matrix scalar type cannot be a reference"
		);
		static_assert(
			!std::is_const_v<Scalar> && !std::is_volatile_v<Scalar>,
			"Matrix scalar type cannot be const- or volatile-qualified"
		);
		static_assert(
			std::is_trivially_constructible_v<Scalar>
			&& std::is_trivially_copyable_v<Scalar>
			&& std::is_trivially_destructible_v<Scalar>,
			"Matrix scalar type must be trivially constructible, copyable and destructible"
		);
		static_assert(
			Rows >= 1,
			"Matrices must have at least one row"
		);
		static_assert(
			Columns >= 1,
			"Matrices must have at least one column"
		);

		/// \brief The number of rows in the matrix.
		static constexpr size_t rows = Rows;

		/// \brief The number of columns in the matrix.
		static constexpr size_t columns = Columns;

		/// \brief The type of each scalar component stored in this matrix.
		using scalar_type = Scalar;

		/// \brief Compile-time constants for this matrix's scalar type.
		using scalar_constants = muu::constants<scalar_type>;

		/// \brief The type of one row of this matrix.
		using row_type = vector<scalar_type, columns>;

		/// \brief The type of one column of this matrix.
		using column_type = vector<scalar_type, rows>;

		/// \brief Alias of `matrix` or `const matrix&`, depending on size, triviality, simd-friendliness, etc.
		using matrix_param = muu::matrix_param<scalar_type, rows, columns>;

		/// \brief Compile-time constants for this matrix.
		using constants = muu::constants<matrix>;

	private:

		using base = impl::matrix_base<Scalar, Rows, Columns>;
		static_assert(
			sizeof(base) == (sizeof(scalar_type) * Rows * Columns),
			"Matrices should not have padding"
		);
		using intermediate_type = impl::promote_if_small_float<scalar_type>;

		using data_type = column_type;
		static constexpr auto data_count = columns;

	public:

		#ifdef DOXYGEN
		/// \brief The values in the matrix (stored column-major).
		column_type m[columns];
		#endif

	#if 1 // constructors ---------------------------------------------------------------------------------------------

		/// \brief Default constructor. Values are not initialized.
		matrix() noexcept = default;

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr matrix(const matrix&) noexcept = default;

		/// \brief Copy-assigment operator.
		constexpr matrix& operator = (const matrix&) noexcept = default;

		/// \brief	Constructs a matrix with all scalar components set to the same value.
		/// 
		/// \details \cpp
		/// const auto mat = matrix<int, 3, 3>{ 1 };
		/// std::cout << mat;
		/// \ecpp
		/// 
		/// \out
		/// {
		///		1,	1,	1,
		///		1,	1,	1,
		///		1,	1,	1
		/// }
		/// \eout
		/// 
		/// \param	fill	The value used to initialize each of the matrix's scalar components.
		MUU_NODISCARD_CTOR
		explicit
		constexpr matrix(scalar_type fill) noexcept
			: base{ impl::value_fill_tag{}, fill }
		{}

		SFINAE_REQUIRES_DIMENSIONS_EXACTLY(2, 2)
		MUU_NODISCARD_CTOR
		explicit
		constexpr matrix(
			scalar_type v00, scalar_type v01,
			scalar_type v10, scalar_type v11
		) noexcept
			REQUIRES_DIMENSIONS_EXACTLY(2, 2)
			: base{
				impl::columnwise_init_tag{},
				column_type{ v00, v10 },
				column_type{ v01, v11 },
			}
		{}

		SFINAE_REQUIRES_DIMENSIONS_EXACTLY(3, 3)
		MUU_NODISCARD_CTOR
		explicit
		constexpr matrix(
			scalar_type v00, scalar_type v01, scalar_type v02,
			scalar_type v10, scalar_type v11, scalar_type v12,
			scalar_type v20, scalar_type v21, scalar_type v22
		) noexcept
			REQUIRES_DIMENSIONS_EXACTLY(3, 3)
			: base{
				impl::columnwise_init_tag{},
				column_type{ v00, v10, v20 },
				column_type{ v01, v11, v21 },
				column_type{ v02, v12, v22 },
			}
		{}

		SFINAE_REQUIRES_DIMENSIONS_EXACTLY(4, 4)
		MUU_NODISCARD_CTOR
		explicit
		constexpr matrix(
			scalar_type v00, scalar_type v01, scalar_type v02, scalar_type v03,
			scalar_type v10, scalar_type v11, scalar_type v12, scalar_type v13,
			scalar_type v20, scalar_type v21, scalar_type v22, scalar_type v23,
			scalar_type v30, scalar_type v31, scalar_type v32, scalar_type v33
		) noexcept
			REQUIRES_DIMENSIONS_EXACTLY(4, 4)
			: base{
				impl::columnwise_init_tag{},
				column_type{ v00, v10, v20, v30 },
				column_type{ v01, v11, v21, v31 },
				column_type{ v02, v12, v22, v32 },
				column_type{ v03, v13, v23, v33 },
			}
		{}

		/// \brief	Constructs a matrix from scalars in row-major order.
		/// 
		/// \details \cpp
		/// const auto mat = matrix<int, 3, 3>{
		///		1, 2, 3,
		///		4, 5, 6,
		///		7, 8, 9,
		///	 };
		/// std::cout << mat;
		/// \ecpp
		/// 
		/// \out
		/// {
		///		1,	2,	3,
		///		4,	5,	6,
		///		7,	8,	9
		/// }
		/// \eout
		/// 
		/// \param	v0		Initial value for the matrix's first scalar component.
		/// \param	v1		Initial value for the matrix's second scalar component.
		/// \param	vals	Initial values for the matrix's remaining scalar components.
		template <typename... T MUU_ENABLE_IF(
			(sizeof...(T) + 2) <= (Rows * Columns)
			&& (sizeof...(T) == 0 || all_convertible_to<scalar_type, T...>)
		)>
		MUU_REQUIRES(
			(sizeof...(T) + 2) <= (Rows * Columns)
			&& (sizeof...(T) == 0 || all_convertible_to<scalar_type, T...>)
		)
		MUU_NODISCARD_CTOR
		explicit
		constexpr matrix(scalar_type v0, scalar_type v1, const T&... vals) noexcept
			: base{
				impl::row_major_tuple_tag{},
				std::tuple<scalar_type, scalar_type, const T&...>{ v0, v1, vals... }
			}
		{}

	#endif // constructors

	#if 1 // scalar component accessors -------------------------------------------------------------------------------

	private:

		template <size_t R, size_t C, typename T>
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(pure)
		static constexpr auto& do_get(T& mat) noexcept
		{
			static_assert(
				R < Rows,
				"Row index out of range"
			);
			static_assert(
				C < Columns,
				"Column index out of range"
			);

			return mat.m[C].template get<R>();
		}

		template <typename T>
		[[nodiscard]]
		MUU_ATTR_NDEBUG(pure)
		static constexpr auto& do_lookup_operator(T& mat, size_t r, size_t c) noexcept
		{
			MUU_CONSTEXPR_SAFE_ASSERT(
				r < Rows
				&& "Row index out of range"
			);
			MUU_CONSTEXPR_SAFE_ASSERT(
				c < Columns
				&& "Column index out of range"
			);
			MUU_ASSUME(r < Rows);
			MUU_ASSUME(c < Columns);

			return mat.m[c][r];
		}

	public:

		/// \brief Gets a reference to the scalar component at a specific row and column.
		/// 
		/// \tparam R	The row of the scalar component to retrieve.
		/// \tparam C	The column of the scalar component to retrieve.
		///
		/// \return  A reference to the selected scalar component.
		template <size_t R, size_t C>
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(pure)
		MUU_ATTR(flatten)
		constexpr const scalar_type& get() const noexcept
		{
			return do_get<R, C>(*this);
		}

		/// \brief Gets a reference to the scalar component at a specific row and column.
		/// 
		/// \tparam R	The row of the scalar component to retrieve.
		/// \tparam C	The column of the scalar component to retrieve.
		///
		/// \return  A reference to the selected scalar component.
		template <size_t R, size_t C>
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(pure)
		MUU_ATTR(flatten)
		constexpr scalar_type& get() noexcept
		{
			return do_get<R, C>(*this);
		}

		/// \brief Gets a reference to the scalar component at a specific row and column.
		/// 
		/// \param r	The row of the scalar component to retrieve.
		/// \param c	The column of the scalar component to retrieve.
		///
		/// \return  A reference to the selected scalar component.
		[[nodiscard]]
		MUU_ATTR_NDEBUG(pure)
		constexpr const scalar_type& operator ()(size_t r, size_t c) const noexcept
		{
			return do_lookup_operator(*this, r, c);
		}

		/// \brief Gets a reference to the scalar component at a specific row and column.
		/// 
		/// \param r	The row of the scalar component to retrieve.
		/// \param c	The column of the scalar component to retrieve.
		///
		/// \return  A reference to the selected scalar component.
		[[nodiscard]]
		MUU_ATTR_NDEBUG(pure)
		constexpr scalar_type& operator ()(size_t r, size_t c) noexcept
		{
			return do_lookup_operator(*this, r, c);
		}

	#endif // scalar component accessors

	#if 1 // equality -------------------------------------------------------------------------------------------------

		/// \brief		Returns true if two matrices are exactly equal.
		/// 
		/// \remarks	This is a componentwise exact equality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point matrices, use #approx_equal().
		template <typename T
			ENABLE_PAIRED_FUNC_BY_REF(T, rows, columns, true)
		>
		REQUIRES_PAIRED_FUNC_BY_REF(T, rows, columns, true)
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr bool MUU_VECTORCALL operator == (matrix_param lhs, const matrix<T, rows, columns>& rhs) noexcept
		{
			for (size_t i = 0; i < data_count; i++)
				if (lhs.m[i] != rhs.m[i])
					return false;
			return true;
		}

		/// \brief	Returns true if two matrices are not exactly equal.
		/// 
		/// \remarks	This is a componentwise exact inequality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point matrices, use #approx_equal().
		template <typename T
			ENABLE_PAIRED_FUNC_BY_REF(T, rows, columns, true)
		>
		REQUIRES_PAIRED_FUNC_BY_REF(T, rows, columns, true)
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr bool MUU_VECTORCALL operator != (matrix_param lhs, const matrix<T, rows, columns>& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		#if ENABLE_PAIRED_FUNCS

		template <typename T
			ENABLE_PAIRED_FUNC_BY_VAL(T, rows, columns, true)
		>
		REQUIRES_PAIRED_FUNC_BY_VAL(T, rows, columns, true)
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr bool MUU_VECTORCALL operator == (matrix_param lhs, matrix<T, rows, columns> rhs) noexcept
		{
			for (size_t i = 0; i < data_count; i++)
				if (lhs.m[i] != rhs.m[i])
					return false;
			return true;
		}

		template <typename T
			ENABLE_PAIRED_FUNC_BY_VAL(T, rows, columns, true)
		>
		REQUIRES_PAIRED_FUNC_BY_VAL(T, rows, columns, true)
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr bool MUU_VECTORCALL operator != (matrix_param lhs, matrix<T, rows, columns> rhs) noexcept
		{
			return !(lhs == rhs);
		}

		#endif // ENABLE_PAIRED_FUNCS

		/// \brief	Returns true if all the scalar components of a matrix are exactly zero.
		/// 
		/// \remarks	This is a componentwise exact equality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point matrices, use #approx_zero().
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool MUU_VECTORCALL zero(matrix_param m) noexcept
		{
			for (size_t i = 0; i < data_count; i++)
				if (!data_type::zero(m.m[i]))
					return false;
			return true;
		}

		/// \brief	Returns true if all the scalar components of the matrix are exactly zero.
		/// 
		/// \remarks	This is a componentwise exact equality check;
		/// 			if you want an epsilon-based "near-enough" for floating-point matrices, use #approx_zero().
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool zero() const noexcept
		{
			return zero(*this);
		}

		/// \brief	Returns true if any of the scalar components of a matrix are infinity or NaN.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool MUU_VECTORCALL infinity_or_nan(matrix_param m) noexcept
		{
			if constexpr (is_floating_point<scalar_type>)
			{
				for (size_t i = 0; i < data_count; i++)
					if (data_type::infinity_or_nan(m.m[i]))
						return true;
				return false;
			}
			else
			{
				(void)m;
				return false;
			}
		}

		/// \brief	Returns true if any of the scalar components of the matrix are infinity or NaN.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool infinity_or_nan() const noexcept
		{
			if constexpr (is_floating_point<scalar_type>)
				return infinity_or_nan(*this);
			else
				return false;
		}

	#endif // equality

	#if 1 // approx_equal ---------------------------------------------------------------------------------------------

		/// \brief	Returns true if two matrices are approximately equal.
		/// 
		/// \note		This function is only available when at least one of #scalar_type and `T` is a floating-point type.
		template <typename T, typename Epsilon = impl::highest_ranked<scalar_type, T>
			ENABLE_PAIRED_FUNC_BY_REF(T, rows, columns, any_floating_point<scalar_type, T>)
		>
		REQUIRES_PAIRED_FUNC_BY_REF(T, rows, columns, any_floating_point<scalar_type, T>)
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool MUU_VECTORCALL approx_equal(
			matrix_param m1,
			const matrix<T, rows, columns>& m2,
			dont_deduce<Epsilon> epsilon = muu::constants<Epsilon>::approx_equal_epsilon
		) noexcept
		{
			for (size_t i = 0; i < data_count; i++)
				if (!data_type::approx_equal(m1.m[i], m2.m[i], epsilon))
					return false;
			return true;
		}

		/// \brief	Returns true if the matrix is approximately equal to another.
		/// 
		/// \note		This function is only available when at least one of #scalar_type and `T` is a floating-point type.
		template <typename T, typename Epsilon = impl::highest_ranked<scalar_type, T>
			ENABLE_PAIRED_FUNC_BY_REF(T, rows, columns, any_floating_point<scalar_type, T>)
		>
		REQUIRES_PAIRED_FUNC_BY_REF(T, rows, columns, any_floating_point<scalar_type, T>)
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool MUU_VECTORCALL approx_equal(
			const matrix<T, rows, columns>& m,
			dont_deduce<Epsilon> epsilon = muu::constants<Epsilon>::approx_equal_epsilon
		) const noexcept
		{
			return approx_equal(*this, m, epsilon);
		}

		#if ENABLE_PAIRED_FUNCS

		template <typename T, typename Epsilon = impl::highest_ranked<scalar_type, T>
			ENABLE_PAIRED_FUNC_BY_VAL(T, rows, columns, any_floating_point<scalar_type, T>)
		>
		REQUIRES_PAIRED_FUNC_BY_VAL(T, rows, columns, any_floating_point<scalar_type, T>)
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool MUU_VECTORCALL approx_equal(
			matrix_param m1,
			matrix<T, rows, columns> m2,
			dont_deduce<Epsilon> epsilon = muu::constants<Epsilon>::approx_equal_epsilon
		) noexcept
		{
			for (size_t i = 0; i < data_count; i++)
				if (!data_type::approx_equal(m1.m[i], m2.m[i], epsilon))
					return false;
			return true;
		}

		template <typename T, typename Epsilon = impl::highest_ranked<scalar_type, T>
			ENABLE_PAIRED_FUNC_BY_VAL(T, rows, columns, any_floating_point<scalar_type, T>)
		>
		REQUIRES_PAIRED_FUNC_BY_VAL(T, rows, columns, any_floating_point<scalar_type, T>)
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool MUU_VECTORCALL approx_equal(
			matrix<T, rows, columns> m,
			dont_deduce<Epsilon> epsilon = muu::constants<Epsilon>::approx_equal_epsilon
		) const noexcept
		{
			return approx_equal(*this, m, epsilon);
		}

		#endif // ENABLE_PAIRED_FUNCS

		/// \brief	Returns true if all the scalar components in a matrix are approximately equal to zero.
		/// 
		/// \note		This function is only available when #scalar_type is a floating-point type.
		SFINAE_REQUIRES_FLOATING_POINT
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool MUU_VECTORCALL approx_zero(
			matrix_param m,
			scalar_type epsilon = muu::constants<scalar_type>::approx_equal_epsilon
		) noexcept
			REQUIRES_FLOATING_POINT
		{
			for (size_t i = 0; i < data_count; i++)
				if (!data_type::approx_zero(m.m[i], epsilon))
					return false;
			return true;
		}

		/// \brief	Returns true if all the scalar components in the matrix are approximately equal to zero.
		/// 
		/// \note		This function is only available when #scalar_type is a floating-point type.
		SFINAE_REQUIRES_FLOATING_POINT
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool MUU_VECTORCALL approx_zero(
			scalar_type epsilon = muu::constants<scalar_type>::approx_equal_epsilon
		) const noexcept
			REQUIRES_FLOATING_POINT
		{
			return approx_zero(*this, epsilon);
		}

	#endif // approx_equal

	#if 1 // addition -------------------------------------------------------------------------------------------------

		/// \brief Returns the componentwise addition of two matrices.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr matrix MUU_VECTORCALL operator + (matrix_param lhs, matrix_param rhs) noexcept
		{
			matrix out{ lhs };
			MUU_PRAGMA_MSVC(omp simd)
			for (size_t i = 0; i < data_count; i++)
				out.m[i] += rhs.m[i];
			return out;
		}

		/// \brief Componentwise adds another atrix to this one.
		constexpr matrix& MUU_VECTORCALL operator += (matrix_param rhs) noexcept
		{
			MUU_PRAGMA_MSVC(omp simd)
			for (size_t i = 0; i < data_count; i++)
				base::m[i] += rhs.m[i];
			return *this;
		}

		/// \brief Returns a componentwise copy of a matrix.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr matrix operator + () const noexcept
		{
			return *this;
		}

	#endif // addition

	#if 1 // ___________ ----------------------------------------------------------------------------------------------
	#endif // ___________

	#if 1 // ___________ ----------------------------------------------------------------------------------------------
	#endif // ___________

	#if 1 // ___________ ----------------------------------------------------------------------------------------------
	#endif // ___________

	#if 1 // streams --------------------------------------------------------------------------------------------------

		/// \brief Writes a matrix out to a text stream.
		template <typename Char, typename Traits>
		friend std::basic_ostream<Char, Traits>& operator << (std::basic_ostream<Char, Traits>& os, const matrix& m)
		{
			impl::print_matrix_to_stream(os, &m.get<0, 0>(), Rows, Columns);
			return os;
		}

	#endif // streams

	#if 1 // misc -----------------------------------------------------------------------------------------------------
	#endif // misc
	};
}
MUU_NAMESPACE_END

#endif // =============================================================================================================

//=====================================================================================================================
// CONSTANTS
#if 1

#endif // =============================================================================================================

//=====================================================================================================================
// FREE FUNCTIONS
#if 1

#endif // =============================================================================================================

#undef REQUIRES_SIZE_AT_LEAST
#undef REQUIRES_SIZE_AT_LEAST_AND
#undef REQUIRES_SIZE_EXACTLY
#undef REQUIRES_ROWS_AT_LEAST
#undef REQUIRES_ROWS_EXACTLY
#undef REQUIRES_COLUMNS_AT_LEAST
#undef REQUIRES_COLUMNS_EXACTLY
#undef REQUIRES_DIMENSIONS_AT_LEAST
#undef REQUIRES_DIMENSIONS_EXACTLY
#undef REQUIRES_FLOATING_POINT
#undef REQUIRES_INTEGRAL
#undef REQUIRES_SIGNED
#undef SFINAE_REQUIRES_SIZE_AT_LEAST
#undef SFINAE_REQUIRES_SIZE_AT_LEAST_AND
#undef SFINAE_REQUIRES_SIZE_EXACTLY
#undef SFINAE_REQUIRES_ROWS_AT_LEAST
#undef SFINAE_REQUIRES_ROWS_EXACTLY
#undef SFINAE_REQUIRES_COLUMNS_AT_LEAST
#undef SFINAE_REQUIRES_COLUMNS_EXACTLY
#undef SFINAE_REQUIRES_DIMENSIONS_AT_LEAST
#undef SFINAE_REQUIRES_DIMENSIONS_EXACTLY
#undef SFINAE_REQUIRES_FLOATING_POINT
#undef SFINAE_REQUIRES_INTEGRAL
#undef SFINAE_REQUIRES_SIGNED
#undef ENABLE_PAIRED_FUNCS
#undef ENABLE_PAIRED_FUNC_BY_REF
#undef ENABLE_PAIRED_FUNC_BY_VAL
#undef REQUIRES_PAIRED_FUNC_BY_REF
#undef REQUIRES_PAIRED_FUNC_BY_VAL

MUU_PRAGMA_MSVC(pop_macro("min"))
MUU_PRAGMA_MSVC(pop_macro("max"))
MUU_PRAGMA_MSVC(float_control(pop))
MUU_PRAGMA_MSVC(inline_recursion(off))

MUU_POP_WARNINGS	// MUU_DISABLE_SHADOW_WARNINGS
