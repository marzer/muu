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
#define	REQUIRES_SQUARE_AND(...)			MUU_REQUIRES(Rows == Columns && (__VA_ARGS__))
#define	REQUIRES_FLOATING_POINT				MUU_REQUIRES(muu::is_floating_point<Scalar>)
#define	REQUIRES_INTEGRAL					MUU_REQUIRES(muu::is_integral<Scalar>)
#define	REQUIRES_SIGNED						MUU_REQUIRES(muu::is_signed<Scalar>)

#if !MUU_CONCEPTS

	#define	ENABLE_IF_SIZE_AT_LEAST_AND(n, ...)						\
		, size_t SFINAE = (Rows * Columns) MUU_ENABLE_IF(			\
			SFINAE >= (n)											\
			&& SFINAE == (Rows * Columns)							\
			&& (__VA_ARGS__)										\
		)

	#define	LEGACY_REQUIRES_SIZE_AT_LEAST(n)						\
		template <size_t SFINAE = Rows * Columns MUU_ENABLE_IF(		\
			SFINAE >= (n)											\
			&& SFINAE == (Rows * Columns)							\
		)>

	#define	LEGACY_REQUIRES_SIZE_AT_LEAST_AND(n, ...)				\
		template <size_t SFINAE = Rows * Columns MUU_ENABLE_IF(		\
			SFINAE >= (n)											\
			&& SFINAE == (Rows * Columns)							\
			&& (__VA_ARGS__)										\
		)>

	#define	LEGACY_REQUIRES_SIZE_EXACTLY(n)							\
		template <size_t SFINAE = Rows * Columns MUU_ENABLE_IF_2(	\
			SFINAE == (n)											\
			&& SFINAE == (Rows * Columns)							\
		)>

	#define	LEGACY_REQUIRES_ROWS_AT_LEAST(n)						\
		template <size_t SFINAE = Rows MUU_ENABLE_IF(				\
			SFINAE >= (n)											\
			&& SFINAE == Rows										\
		)>

	#define	LEGACY_REQUIRES_ROWS_EXACTLY(n)							\
		template <size_t SFINAE = Rows MUU_ENABLE_IF_2(				\
			SFINAE == (n)											\
			&& SFINAE == Rows										\
		)>

	#define	LEGACY_REQUIRES_COLUMNS_AT_LEAST(n)						\
		template <size_t SFINAE = Columns MUU_ENABLE_IF(			\
			SFINAE >= (n)											\
			&& SFINAE == Columns									\
		)>

	#define	LEGACY_REQUIRES_COLUMNS_EXACTLY(n)						\
		template <size_t SFINAE = Columns MUU_ENABLE_IF_2(			\
			SFINAE == (n)											\
			&& SFINAE == Columns									\
		)>

	#define	LEGACY_REQUIRES_DIMENSIONS_AT_LEAST(r, c)								\
		template <size_t SFINAE_R = Rows, size_t SFINAE_C = Columns MUU_ENABLE_IF(	\
			SFINAE_R >= (r)															\
			&& SFINAE_C >= (c)														\
			&& SFINAE_R == Rows														\
			&& SFINAE_C == Columns													\
		)>

	#define	LEGACY_REQUIRES_DIMENSIONS_EXACTLY(r, c)								\
		template <size_t SFINAE_R = Rows, size_t SFINAE_C = Columns MUU_ENABLE_IF_2(\
			SFINAE_R == (r)															\
			&& SFINAE_C == (c)														\
			&& SFINAE_R == Rows														\
			&& SFINAE_C == Columns													\
		)>

	#define	LEGACY_REQUIRES_SQUARE_AND(...)											\
		template <size_t SFINAE_R = Rows, size_t SFINAE_C = Columns MUU_ENABLE_IF_2(\
			SFINAE_R == SFINAE_C													\
			&& SFINAE_R == Rows														\
			&& SFINAE_C == Columns													\
			&& (__VA_ARGS__)														\
		)>

	#define	LEGACY_REQUIRES_FLOATING_POINT							\
		template <typename SFINAE = Scalar MUU_ENABLE_IF(			\
			muu::is_floating_point<SFINAE>							\
			&& std::is_same_v<SFINAE, Scalar>						\
		)>

	#define	LEGACY_REQUIRES_INTEGRAL								\
		template <typename SFINAE = Scalar MUU_ENABLE_IF_2(			\
			muu::is_integral<SFINAE>								\
			&& std::is_same_v<SFINAE, Scalar>						\
		)>

	#define	LEGACY_REQUIRES_SIGNED									\
		template <typename SFINAE = Scalar MUU_ENABLE_IF(			\
			muu::is_signed<SFINAE>									\
			&& std::is_same_v<SFINAE, Scalar>						\
		)>

#endif // !MUU_CONCEPTS

#endif // helper macros

MUU_IMPL_NAMESPACE_START
{
	struct columnwise_init_tag {};
	struct columnwise_copy_tag {};
	struct row_major_tuple_tag {};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct MUU_TRIVIAL_ABI matrix_base
	{
		vector<Scalar, Rows> m[Columns];

		matrix_base() noexcept = default;

		template <size_t... ColumnIndices>
		explicit
		constexpr matrix_base(value_fill_tag, std::index_sequence<ColumnIndices...>, Scalar fill) noexcept
			: m{ ((void)ColumnIndices, vector<Scalar, Rows>{ fill })... }
		{
			static_assert(sizeof...(ColumnIndices) <= Columns);
		}

		template <typename... T>
		explicit
		constexpr matrix_base(columnwise_init_tag, T... cols) noexcept
			: m{ cols... }
		{
			static_assert(sizeof...(T) <= Columns);
		}

		template <typename T, size_t... ColumnIndices>
		explicit
		constexpr matrix_base(columnwise_copy_tag, std::index_sequence<ColumnIndices...>, const T& cols) noexcept
			: m{ vector<Scalar, Rows>{ cols[ColumnIndices] }... }
		{
			static_assert(sizeof...(ColumnIndices) <= Columns);
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

	#define MAT_GET(r, c) static_cast<type>(m.m[c].template get<r>())

	template<
		size_t Row0 = 0, size_t Row1 = 1,
		size_t Col0 = 0, size_t Col1 = 1,
		typename T
	>
	[[nodiscard]]
	MUU_ATTR(pure)
	static constexpr
	promote_if_small_float<typename T::determinant_type>
	MUU_VECTORCALL
	raw_determinant_2x2(const T& m) noexcept
	{
		MUU_FMA_BLOCK
		using type = promote_if_small_float<typename T::determinant_type>;

		return MAT_GET(Row0, Col0) * MAT_GET(Row1, Col1)
			 - MAT_GET(Row0, Col1) * MAT_GET(Row1, Col0);
	}

	template<
		size_t Row0 = 0, size_t Row1 = 1, size_t Row2 = 2,
		size_t Col0 = 0, size_t Col1 = 1, size_t Col2 = 2,
		typename T
	>
	[[nodiscard]]
	MUU_ATTR(pure)
	static constexpr
	promote_if_small_float<typename T::determinant_type>
	MUU_VECTORCALL
	raw_determinant_3x3(const T& m) noexcept
	{
		MUU_FMA_BLOCK
		using type = promote_if_small_float<typename T::determinant_type>;

		return MAT_GET(Row0, Col0) * raw_determinant_2x2<Row1, Row2, Col1, Col2>(m)
			 - MAT_GET(Row0, Col1) * raw_determinant_2x2<Row1, Row2, Col0, Col2>(m)
			 + MAT_GET(Row0, Col2) * raw_determinant_2x2<Row1, Row2, Col0, Col1>(m);
	}

	template<
		size_t Row0 = 0, size_t Row1 = 1, size_t Row2 = 2, size_t Row3 = 3,
		size_t Col0 = 0, size_t Col1 = 1, size_t Col2 = 2, size_t Col3 = 3,
		typename T
	>
	[[nodiscard]]
	MUU_ATTR(pure)
	static constexpr
	promote_if_small_float<typename T::determinant_type>
	MUU_VECTORCALL
	raw_determinant_4x4(const T& m) noexcept
	{
		MUU_FMA_BLOCK
		using type = promote_if_small_float<typename T::determinant_type>;

		return MAT_GET(Row0, Col0) * raw_determinant_3x3<Row1, Row2, Row3, Col1, Col2, Col3>(m)
			 - MAT_GET(Row0, Col1) * raw_determinant_3x3<Row1, Row2, Row3, Col0, Col2, Col3>(m)
			 + MAT_GET(Row0, Col2) * raw_determinant_3x3<Row1, Row2, Row3, Col0, Col1, Col3>(m)
			 - MAT_GET(Row0, Col3) * raw_determinant_3x3<Row1, Row2, Row3, Col0, Col1, Col2>(m);
	}

	#undef MAT_GET
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
#define	REQUIRES_SQUARE_AND(...)
#define	REQUIRES_FLOATING_POINT
#define	REQUIRES_INTEGRAL
#define	REQUIRES_SIGNED

#endif // DOXYGEN
#ifndef ENABLE_IF_SIZE_AT_LEAST_AND
	#define ENABLE_IF_SIZE_AT_LEAST_AND(n, ...)
#endif
#ifndef LEGACY_REQUIRES_SIZE_AT_LEAST
	#define LEGACY_REQUIRES_SIZE_AT_LEAST(n)
#endif
#ifndef LEGACY_REQUIRES_SIZE_AT_LEAST_AND
	#define LEGACY_REQUIRES_SIZE_AT_LEAST_AND(n, ...)
#endif
#ifndef LEGACY_REQUIRES_SIZE_EXACTLY
	#define LEGACY_REQUIRES_SIZE_EXACTLY(n)
#endif
#ifndef LEGACY_REQUIRES_ROWS_AT_LEAST
	#define LEGACY_REQUIRES_ROWS_AT_LEAST(n)
#endif
#ifndef LEGACY_REQUIRES_ROWS_EXACTLY
	#define LEGACY_REQUIRES_ROWS_EXACTLY(n)
#endif
#ifndef LEGACY_REQUIRES_COLUMNS_AT_LEAST
	#define LEGACY_REQUIRES_COLUMNS_AT_LEAST(n)
#endif
#ifndef LEGACY_REQUIRES_COLUMNS_EXACTLY
	#define LEGACY_REQUIRES_COLUMNS_EXACTLY(n)
#endif
#ifndef LEGACY_REQUIRES_DIMENSIONS_AT_LEAST
	#define	LEGACY_REQUIRES_DIMENSIONS_AT_LEAST(r, c)
#endif
#ifndef LEGACY_REQUIRES_DIMENSIONS_EXACTLY
	#define	LEGACY_REQUIRES_DIMENSIONS_EXACTLY(r, c)
#endif
#ifndef LEGACY_REQUIRES_SQUARE_AND
	#define LEGACY_REQUIRES_SQUARE_AND(...)
#endif
#ifndef LEGACY_REQUIRES_FLOATING_POINT
	#define LEGACY_REQUIRES_FLOATING_POINT
#endif
#ifndef LEGACY_REQUIRES_INTEGRAL
	#define LEGACY_REQUIRES_INTEGRAL
#endif
#ifndef LEGACY_REQUIRES_SIGNED
	#define LEGACY_REQUIRES_SIGNED
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

		/// \brief The scalar type used for determinants. Always signed.
		using determinant_type = std::conditional_t<
			is_integral<scalar_type>,
			impl::highest_ranked<make_signed<scalar_type>, int>,
			scalar_type
		>;

		/// \brief The scalar type used for matrix inversion. Always floating-point.
		using inverse_type = std::conditional_t<is_integral<scalar_type>, double, scalar_type>;

	private:

		template <typename S, size_t R, size_t C>
		friend struct matrix;
		template <typename T, size_t R, size_t C>
		friend struct Achilles::Math::Matrix;

		using base = impl::matrix_base<Scalar, Rows, Columns>;
		static_assert(
			sizeof(base) == (sizeof(scalar_type) * Rows * Columns),
			"Matrices should not have padding"
		);

		using data_type = column_type;
		static constexpr auto data_count = columns;

		using intermediate_float = impl::promote_if_small_float<inverse_type>;
		static_assert(is_floating_point<inverse_type>);
		static_assert(is_floating_point<intermediate_float>);

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
		/// std::cout << matrix<int, 3, 3>{ 1 } << "\n";
		/// \ecpp
		/// 
		/// \out
		/// {    1,    1,    1,
		///	     1,    1,    1,
		///	     1,    1,    1 }
		/// \eout
		/// 
		/// \param	fill	The value used to initialize each of the matrix's scalar components.
		MUU_NODISCARD_CTOR
		explicit
		constexpr matrix(scalar_type fill) noexcept
			: base{ impl::value_fill_tag{}, std::make_index_sequence<Columns>{}, fill }
		{}

		/// \brief	Constructs a matrix from (row-major-ordered) scalars.
		/// 
		/// \details \cpp
		/// // explicitly-sized matrices:
		/// std::cout << matrix<int, 2, 3>{ 1, 2, 3, 4, 5, 6 } << "\n\n";
		/// 
		/// // 2x2, 3x3, 3x4 and 4x4 matrices can be deduced automatically from 4, 9 12 and 16 inputs (respectively):
		/// std::cout << matrix{ 1, 2, 3, 4 } << "\n\n";
		/// std::cout << matrix{ 1, 2, 3, 4, 5, 6, 7, 8, 9 } << "\n\n";
		/// std::cout << matrix{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 } << "\n\n";
		/// std::cout << matrix{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } << "\n\n";
		/// \ecpp
		/// 
		/// \out
		/// {    1,    2,    3,
		///      4,    5,    6 }
		/// 
		/// {    1,    2,
		///      3,    4 }
		/// 
		/// {    1,    2,    3,
		///      4,    5,    6,
		///      7,    8,    9 }
		/// 
		/// {    1,    2,    3,    4,
		///      5,    6,    7,    8,
		///      9,   10,   11,   12 }
		/// 
		/// {    1,    2,    3,    4,
		///      5,    6,    7,    8,
		///      9,   10,   11,   12,
		///     13,   14,   15,   16 }
		/// \eout
		/// 
		/// Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \tparam T		Types convertible to #scalar_type.
		/// \param	v0		Initial value for the matrix's first scalar component.
		/// \param	v1		Initial value for the matrix's second scalar component.
		/// \param	vals	Initial values for the matrix's remaining scalar components.
		template <typename... T
			ENABLE_IF_SIZE_AT_LEAST_AND(sizeof...(T) + 2, all_convertible_to<scalar_type, scalar_type, T...>)
		>
		REQUIRES_SIZE_AT_LEAST_AND(sizeof...(T) + 2, all_convertible_to<scalar_type, scalar_type, T...>)
		MUU_NODISCARD_CTOR
		constexpr matrix(scalar_type v0, scalar_type v1, const T&... vals) noexcept
			: base{
				impl::row_major_tuple_tag{},
				std::tuple<scalar_type, scalar_type, const T&...>{ v0, v1, vals... }
			}
		{}

		/// \brief Enlarging/truncating/converting constructor.
		/// \details Copies source matrix's scalar components, casting if necessary:
		/// \cpp
		/// matrix<int, 3, 3> m33 = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		/// std::cout << m33 << "\n\n";
		/// 
		/// matrix<int, 2, 2> m22 = { m33 };
		/// std::cout << m22 << "\n\n";
		/// 
		/// matrix<int, 4, 4> m44 = { m33 };
		/// std::cout << m44 << "\n\n";
		/// \ecpp
		/// 
		/// \out
		/// {    1,    2,    3,
		///      4,    5,    6,
		///      7,    8,    9 }
		/// 
		/// {    1,    2,
		///      4,    5 }
		/// 
		/// {    1,    2,    3,    0,
		///      4,    5,    6,    0,
		///      7,    8,    9,    0,
		///      0,    0,    0,    0 }
		/// \eout
		/// 	 
		/// Any scalar components not covered by the constructor's parameters are initialized to zero.
		/// 
		/// \tparam	S		Source matrix's #scalar_type.
		/// \tparam	R		Source matrix's row count.
		/// \tparam	C		Source matrix's column count.
		/// \param 	mat		Source matrix.
		template <typename S, size_t R, size_t C>
		MUU_NODISCARD_CTOR
		explicit
		constexpr matrix(const matrix<S, R, C>& mat) noexcept
			: base{ impl::columnwise_copy_tag{}, std::make_index_sequence<(C < Columns ? C : Columns)>{}, mat.m }
		{}

		#if ENABLE_PAIRED_FUNCS

		// optimizations for 2x2, 3x3, 3x4 and 4x4 cases

		LEGACY_REQUIRES_DIMENSIONS_EXACTLY(2, 2)
		MUU_NODISCARD_CTOR
		constexpr matrix(
			scalar_type v00, scalar_type v01,
			scalar_type v10, scalar_type v11
		) noexcept
			REQUIRES_DIMENSIONS_EXACTLY(2, 2)
			: base{
				impl::columnwise_init_tag{},
				column_type{ v00, v10 },
				column_type{ v01, v11 }
			}
		{}

		LEGACY_REQUIRES_DIMENSIONS_EXACTLY(3, 3)
		MUU_NODISCARD_CTOR
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
				column_type{ v02, v12, v22 }
			}
		{}

		LEGACY_REQUIRES_DIMENSIONS_EXACTLY(3, 4)
		MUU_NODISCARD_CTOR
		constexpr matrix(
			scalar_type v00, scalar_type v01, scalar_type v02, scalar_type v03,
			scalar_type v10, scalar_type v11, scalar_type v12, scalar_type v13,
			scalar_type v20, scalar_type v21, scalar_type v22, scalar_type v23
		) noexcept
			REQUIRES_DIMENSIONS_EXACTLY(3, 4)
			: base{
				impl::columnwise_init_tag{},
				column_type{ v00, v10, v20 },
				column_type{ v01, v11, v21 },
				column_type{ v02, v12, v22 },
				column_type{ v03, v13, v23 },
			}
		{}

		LEGACY_REQUIRES_DIMENSIONS_EXACTLY(4, 4)
		MUU_NODISCARD_CTOR
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
				column_type{ v03, v13, v23, v33 }
			}
		{}

		#endif // ENABLE_PAIRED_FUNCS

	private:

		template <typename... T>
		MUU_NODISCARD_CTOR
		constexpr matrix(impl::columnwise_init_tag, T... cols) noexcept
			: base{ impl::columnwise_init_tag{}, cols... }
		{}

	public:


	#endif // constructors

	#if 1 // scalar component accessors -------------------------------------------------------------------------------

	private:

		template <size_t R, size_t C, typename T>
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(pure)
		MUU_ATTR(flatten)
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
		MUU_ATTR_NDEBUG(flatten)
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
		MUU_ATTR_NDEBUG(flatten)
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
		MUU_ATTR_NDEBUG(flatten)
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
		friend
		constexpr bool MUU_VECTORCALL operator == (matrix_param lhs, const matrix<T, rows, columns>& rhs) noexcept
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
		friend
		constexpr bool MUU_VECTORCALL operator != (matrix_param lhs, const matrix<T, rows, columns>& rhs) noexcept
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
		friend
		constexpr bool MUU_VECTORCALL operator == (matrix_param lhs, matrix<T, rows, columns> rhs) noexcept
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
		friend
		constexpr bool MUU_VECTORCALL operator != (matrix_param lhs, matrix<T, rows, columns> rhs) noexcept
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
		LEGACY_REQUIRES_FLOATING_POINT
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
		LEGACY_REQUIRES_FLOATING_POINT
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
		friend
		constexpr matrix MUU_VECTORCALL operator + (matrix_param lhs, matrix_param rhs) noexcept
		{
			matrix out{ lhs };
			MUU_PRAGMA_MSVC(omp simd)
			for (size_t i = 0; i < data_count; i++)
				out.m[i] += rhs.m[i];
			return out;
		}

		/// \brief Componentwise adds another matrix to this one.
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

	#if 1 // subtraction -------------------------------------------------------------------------------------------------

		/// \brief Returns the componentwise subtraction of two matrices.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend
		constexpr matrix MUU_VECTORCALL operator - (matrix_param lhs, matrix_param rhs) noexcept
		{
			matrix out{ lhs };
			MUU_PRAGMA_MSVC(omp simd)
			for (size_t i = 0; i < data_count; i++)
				out.m[i] -= rhs.m[i];
			return out;
		}

		/// \brief Componentwise subtracts another matrix from this one.
		constexpr matrix& MUU_VECTORCALL operator -= (matrix_param rhs) noexcept
		{
			MUU_PRAGMA_MSVC(omp simd)
			for (size_t i = 0; i < data_count; i++)
				base::m[i] -= rhs.m[i];
			return *this;
		}

		/// \brief Returns a componentwise negation of a matrix.
		LEGACY_REQUIRES_SIGNED
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr matrix operator - () const noexcept
			REQUIRES_SIGNED
		{
			matrix out{ *this };
			MUU_PRAGMA_MSVC(omp simd)
			for (size_t i = 0; i < data_count; i++)
				out.m[i] = -out.m[i];
			return out;
		}

	#endif // subtraction

	#if 1 // multiplication -------------------------------------------------------------------------------------------

		/// \brief Multiplies two matrices.
		///
		/// \tparam C The number of columns in the RHS matrix.
		/// \param lhs  The LHS matrix.
		/// \param rhs  The RHS matrix.
		///
		/// \return  The result of `LHS * RHS`.
		template <size_t C>
		[[nodiscard]]
		MUU_ATTR(pure)
		friend
		constexpr matrix<scalar_type, rows, C> MUU_VECTORCALL operator * (
			matrix_param lhs,
			const matrix<scalar_type, columns, C>& rhs
		) noexcept
		{
			using result_type = matrix<scalar_type, Rows, C>;
			using type = impl::highest_ranked<
				typename data_type::product_type,
				std::conditional_t<is_floating_point<scalar_type>, intermediate_float, scalar_type>
			>;

			#define MULT_DOT(row, col, idx)		\
				static_cast<type>(lhs.m[idx].template get<row>()) * static_cast<type>(rhs.m[col].template get<idx>())

			// common square cases are manually unrolled 
			if constexpr (Rows == 2 && Columns == 2 && C == 2)
			{
				MUU_FMA_BLOCK

				return result_type
				{
					impl::columnwise_init_tag{},
					column_type
					{
						static_cast<scalar_type>(MULT_DOT(0, 0, 0) + MULT_DOT(0, 0, 1)),
						static_cast<scalar_type>(MULT_DOT(1, 0, 0) + MULT_DOT(1, 0, 1))
					},
					column_type
					{
						static_cast<scalar_type>(MULT_DOT(0, 1, 0) + MULT_DOT(0, 1, 1)),
						static_cast<scalar_type>(MULT_DOT(1, 1, 0) + MULT_DOT(1, 1, 1))
					}
				};
			}
			else if constexpr (Rows == 3 && Columns == 3 && C == 3)
			{
				MUU_FMA_BLOCK

				return result_type
				{
					impl::columnwise_init_tag{},
					column_type
					{
						static_cast<scalar_type>(MULT_DOT(0, 0, 0) + MULT_DOT(0, 0, 1) + MULT_DOT(0, 0, 2)),
						static_cast<scalar_type>(MULT_DOT(1, 0, 0) + MULT_DOT(1, 0, 1) + MULT_DOT(1, 0, 2)),
						static_cast<scalar_type>(MULT_DOT(2, 0, 0) + MULT_DOT(2, 0, 1) + MULT_DOT(2, 0, 2))
					},
					column_type
					{
						static_cast<scalar_type>(MULT_DOT(0, 1, 0) + MULT_DOT(0, 1, 1) + MULT_DOT(0, 1, 2)),
						static_cast<scalar_type>(MULT_DOT(1, 1, 0) + MULT_DOT(1, 1, 1) + MULT_DOT(1, 1, 2)),
						static_cast<scalar_type>(MULT_DOT(2, 1, 0) + MULT_DOT(2, 1, 1) + MULT_DOT(2, 1, 2))
					},
					column_type
					{
						static_cast<scalar_type>(MULT_DOT(0, 2, 0) + MULT_DOT(0, 2, 1) + MULT_DOT(0, 2, 2)),
						static_cast<scalar_type>(MULT_DOT(1, 2, 0) + MULT_DOT(1, 2, 1) + MULT_DOT(1, 2, 2)),
						static_cast<scalar_type>(MULT_DOT(2, 2, 0) + MULT_DOT(2, 2, 1) + MULT_DOT(2, 2, 2))
					}

				};
			}
			else if constexpr (Rows == 4 && Columns == 4 && C == 4)
			{
				MUU_FMA_BLOCK

				return result_type
				{
					impl::columnwise_init_tag{},
					column_type{
						static_cast<scalar_type>(MULT_DOT(0, 0, 0) + MULT_DOT(0, 0, 1) + MULT_DOT(0, 0, 2) + MULT_DOT(0, 0, 3)),
						static_cast<scalar_type>(MULT_DOT(1, 0, 0) + MULT_DOT(1, 0, 1) + MULT_DOT(1, 0, 2) + MULT_DOT(1, 0, 3)),
						static_cast<scalar_type>(MULT_DOT(2, 0, 0) + MULT_DOT(2, 0, 1) + MULT_DOT(2, 0, 2) + MULT_DOT(2, 0, 3)),
						static_cast<scalar_type>(MULT_DOT(3, 0, 0) + MULT_DOT(3, 0, 1) + MULT_DOT(3, 0, 2) + MULT_DOT(3, 0, 3))
					},
					column_type{
						static_cast<scalar_type>(MULT_DOT(0, 1, 0) + MULT_DOT(0, 1, 1) + MULT_DOT(0, 1, 2) + MULT_DOT(0, 1, 3)),
						static_cast<scalar_type>(MULT_DOT(1, 1, 0) + MULT_DOT(1, 1, 1) + MULT_DOT(1, 1, 2) + MULT_DOT(1, 1, 3)),
						static_cast<scalar_type>(MULT_DOT(2, 1, 0) + MULT_DOT(2, 1, 1) + MULT_DOT(2, 1, 2) + MULT_DOT(2, 1, 3)),
						static_cast<scalar_type>(MULT_DOT(3, 1, 0) + MULT_DOT(3, 1, 1) + MULT_DOT(3, 1, 2) + MULT_DOT(3, 1, 3))
					},
					column_type{
						static_cast<scalar_type>(MULT_DOT(0, 2, 0) + MULT_DOT(0, 2, 1) + MULT_DOT(0, 2, 2) + MULT_DOT(0, 2, 3)),
						static_cast<scalar_type>(MULT_DOT(1, 2, 0) + MULT_DOT(1, 2, 1) + MULT_DOT(1, 2, 2) + MULT_DOT(1, 2, 3)),
						static_cast<scalar_type>(MULT_DOT(2, 2, 0) + MULT_DOT(2, 2, 1) + MULT_DOT(2, 2, 2) + MULT_DOT(2, 2, 3)),
						static_cast<scalar_type>(MULT_DOT(3, 2, 0) + MULT_DOT(3, 2, 1) + MULT_DOT(3, 2, 2) + MULT_DOT(3, 2, 3))
					},
					column_type{
						static_cast<scalar_type>(MULT_DOT(0, 3, 0) + MULT_DOT(0, 3, 1) + MULT_DOT(0, 3, 2) + MULT_DOT(0, 3, 3)),
						static_cast<scalar_type>(MULT_DOT(1, 3, 0) + MULT_DOT(1, 3, 1) + MULT_DOT(1, 3, 2) + MULT_DOT(1, 3, 3)),
						static_cast<scalar_type>(MULT_DOT(2, 3, 0) + MULT_DOT(2, 3, 1) + MULT_DOT(2, 3, 2) + MULT_DOT(2, 3, 3)),
						static_cast<scalar_type>(MULT_DOT(3, 3, 0) + MULT_DOT(3, 3, 1) + MULT_DOT(3, 3, 2) + MULT_DOT(3, 3, 3))
					}
				};
			}
			else
			{
				result_type out;
				for (size_t out_r = 0; out_r < Rows; out_r++)
				{
					for (size_t out_c = 0; out_c < C; out_c++)
					{
						MUU_FMA_BLOCK

						auto val = static_cast<type>(lhs(out_r, 0)) * static_cast<type>(rhs(0, out_c));

						MUU_PRAGMA_MSVC(omp simd)
						for (size_t r = 1; r < Columns; r++)
							val += static_cast<type>(lhs(out_r, r)) * static_cast<type>(rhs(r, out_c));

						out(out_r, out_c) = static_cast<scalar_type>(val);
					}
				}
				return out;
			}

			#undef MULT_DOT
		}

		/// \brief Multiplies this matrix with another and assigns the result.
		/// 
		/// \note This function is only available when the matrix is square.
		LEGACY_REQUIRES_SQUARE_AND(true)
		constexpr matrix& MUU_VECTORCALL operator *= (matrix_param rhs) noexcept
			REQUIRES_SQUARE_AND(true)
		{
			return *this = *this * rhs;
		}

		/// \brief Returns the componentwise multiplication of a matrix and a scalar.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend
		constexpr matrix MUU_VECTORCALL operator * (matrix_param lhs, scalar_type rhs) noexcept
		{
			matrix out{ lhs };
			out *= rhs;
			return out;
		}

		/// \brief Returns the componentwise multiplication of a matrix and a scalar.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend
		constexpr matrix MUU_VECTORCALL operator * (scalar_type lhs, matrix_param rhs) noexcept
		{
			return rhs * lhs;
		}

		/// \brief Componentwise multiplies this matrix by a scalar.
		constexpr matrix& MUU_VECTORCALL operator *= (scalar_type rhs) noexcept
		{
			using type = impl::highest_ranked<
				typename data_type::product_type,
				std::conditional_t<is_floating_point<scalar_type>, intermediate_float, scalar_type>
			>;

			if constexpr (std::is_same_v<type, scalar_type>)
			{
				for (auto& col : base::m)
					col.raw_multiply_assign_scalar(rhs);
			}
			else
			{
				const auto rhs_ = static_cast<type>(rhs);
				for (auto& col : base::m)
					col.raw_multiply_assign_scalar(rhs_);
			}

			return *this;
		}

	#endif // multiplication

	#if 1 // division -------------------------------------------------------------------------------------------------

		[[nodiscard]]
		MUU_ATTR(pure)
		friend
		constexpr matrix MUU_VECTORCALL operator / (matrix_param lhs, scalar_type rhs) noexcept
		{
			matrix out{ lhs };
			out /= rhs;
			return out;
		}

		/// \brief Componentwise multiplies this matrix by a scalar.
		constexpr matrix& MUU_VECTORCALL operator /= (scalar_type rhs) noexcept
		{
			using type = impl::highest_ranked<
				typename data_type::product_type,
				std::conditional_t<is_floating_point<scalar_type>, intermediate_float, scalar_type>
			>;

			if constexpr (is_floating_point<type>)
			{
				const auto rhs_ = type{ 1 } / static_cast<type>(rhs);
				for (auto& col : base::m)
					col.raw_multiply_assign_scalar(rhs_);
			}
			else if constexpr (std::is_same_v<type, scalar_type>)
			{
				for (auto& col : base::m)
					col.raw_divide_assign_scalar(rhs);
			}
			else
			{
				const auto rhs_ = static_cast<type>(rhs);
				for (auto& col : base::m)
					col.raw_divide_assign_scalar(rhs_);
			}
			return *this;
		}

	#endif // division

	#if 1 // transposition --------------------------------------------------------------------------------------------

		/// \brief	Returns a transposed copy of a matrix.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr matrix<scalar_type, columns, rows> MUU_VECTORCALL transpose(matrix_param m) noexcept
		{
			using result_type = matrix<scalar_type, columns, rows>;
			using result_column = vector<scalar_type, columns>;

			#define MAT_GET(r, c) m.m[c].template get<r>()

			// common square cases are manually unrolled 
			if constexpr (Rows == 2 && Columns == 2)
			{
				return result_type
				{
					impl::columnwise_init_tag{},
					result_column{ MAT_GET(0, 0), MAT_GET(0, 1) },
					result_column{ MAT_GET(1, 0), MAT_GET(1, 1) },
				};
			}
			else if constexpr (Rows == 3 && Columns == 3)
			{
				return result_type
				{
					impl::columnwise_init_tag{},
					result_column{ MAT_GET(0, 0), MAT_GET(0, 1), MAT_GET(0, 2) },
					result_column{ MAT_GET(1, 0), MAT_GET(1, 1), MAT_GET(1, 2) },
					result_column{ MAT_GET(2, 0), MAT_GET(2, 1), MAT_GET(2, 2) },
				};
			}
			else if constexpr (Rows == 4 && Columns == 4)
			{
				return result_type
				{
					impl::columnwise_init_tag{},
					result_column{ MAT_GET(0, 0), MAT_GET(0, 1), MAT_GET(0, 2), MAT_GET(0, 3) },
					result_column{ MAT_GET(1, 0), MAT_GET(1, 1), MAT_GET(1, 2), MAT_GET(1, 3) },
					result_column{ MAT_GET(2, 0), MAT_GET(2, 1), MAT_GET(2, 2), MAT_GET(2, 3) },
					result_column{ MAT_GET(3, 0), MAT_GET(3, 1), MAT_GET(3, 2), MAT_GET(3, 3) },
				};
			}
			else
			{
				result_type out;
				for (size_t c = 0; c < Columns; c++)
				{
					auto& col = m.m[c];
					MUU_PRAGMA_MSVC(omp simd)
					for (size_t r = 0; r < Rows; r++)
						out.m[r][c] = col[r];
				}
				return out;
			}

			#undef MAT_GET
		}

		/// \brief	Transposes the matrix (in-place).
		/// 
		/// \note This function is only available when the matrix is square.
		LEGACY_REQUIRES_SQUARE_AND(true)
		constexpr matrix& MUU_VECTORCALL transpose() noexcept
			REQUIRES_SQUARE_AND(true)
		{
			return *this = transpose(*this);
		}

	#endif // transposition

	#if 1 // inverse & determinant ------------------------------------------------------------------------------------

		/// \brief	Calculates the determinant of a matrix.
		/// 
		/// \note This function is only available when the matrix is square and has at most 4 rows and columns.
		LEGACY_REQUIRES_SQUARE_AND(Columns <= 4)
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr determinant_type MUU_VECTORCALL determinant(matrix_param m) noexcept
			REQUIRES_SQUARE_AND(Columns <= 4)
		{
			if constexpr (Columns == 1) return static_cast<determinant_type>(m.m[0].x);
			if constexpr (Columns == 2) return static_cast<determinant_type>(impl::raw_determinant_2x2(m));
			if constexpr (Columns == 3) return static_cast<determinant_type>(impl::raw_determinant_3x3(m));
			if constexpr (Columns == 4) return static_cast<determinant_type>(impl::raw_determinant_4x4(m));
		}

		/// \brief	Calculates the determinant of a matrix.
		/// 
		/// \note This function is only available when the matrix is square and has at most 4 rows and columns.
		LEGACY_REQUIRES_SQUARE_AND(Columns <= 4)
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr determinant_type MUU_VECTORCALL determinant() noexcept
			REQUIRES_SQUARE_AND(Columns <= 4)
		{
			return determinant(*this);
		}

		/// \brief	Returns the inverse of a matrix.
		/// 
		/// \note This function is only available when the matrix is square and has at most 4 rows and columns.
		LEGACY_REQUIRES_SQUARE_AND(Columns <= 4)
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr matrix<inverse_type, rows, columns> MUU_VECTORCALL invert(matrix_param m) noexcept
			REQUIRES_SQUARE_AND(Columns <= 4)
		{
			using result_type = matrix<inverse_type, rows, columns>;
			using result_column = vector<inverse_type, rows>;

			#define MAT_GET(r, c) static_cast<intermediate_float>(m.m[c].template get<r>())

			if constexpr (Columns == 1)
			{
				return result_type{
					impl::columnwise_init_tag{},
					result_column{
						static_cast<inverse_type>(intermediate_float{ 1 } / static_cast<intermediate_float>(m.m[0].x))
					}
				};
			}
			if constexpr (Columns == 2)
			{
				MUU_FMA_BLOCK

				const auto det = intermediate_float{ 1 } / static_cast<intermediate_float>(impl::raw_determinant_2x2(m));
				return result_type
				{
					impl::columnwise_init_tag{},
					result_column{
						static_cast<inverse_type>(det * MAT_GET(1, 1)),
						static_cast<inverse_type>(det * -MAT_GET(1, 0))
					},
					result_column{
						static_cast<inverse_type>(det * -MAT_GET(0, 1)),
						static_cast<inverse_type>(det * MAT_GET(0, 0))
					}
				};
			}
			if constexpr (Columns == 3)
			{
				MUU_FMA_BLOCK

				const auto det = intermediate_float{ 1 } / static_cast<intermediate_float>(impl::raw_determinant_3x3(m));
				return result_type
				{
					impl::columnwise_init_tag{},
					result_column{
						static_cast<inverse_type>(det *  (MAT_GET(1, 1) * MAT_GET(2, 2) - MAT_GET(1, 2) * MAT_GET(2, 1))),
						static_cast<inverse_type>(det * -(MAT_GET(1, 0) * MAT_GET(2, 2) - MAT_GET(1, 2) * MAT_GET(2, 0))),
						static_cast<inverse_type>(det *  (MAT_GET(1, 0) * MAT_GET(2, 1) - MAT_GET(1, 1) * MAT_GET(2, 0)))
					},
					result_column{
						static_cast<inverse_type>(det * -(MAT_GET(0, 1) * MAT_GET(2, 2) - MAT_GET(0, 2) * MAT_GET(2, 1))),
						static_cast<inverse_type>(det *  (MAT_GET(0, 0) * MAT_GET(2, 2) - MAT_GET(0, 2) * MAT_GET(2, 0))),
						static_cast<inverse_type>(det * -(MAT_GET(0, 0) * MAT_GET(2, 1) - MAT_GET(0, 1) * MAT_GET(2, 0)))
					},
					result_column{
						static_cast<inverse_type>(det *  (MAT_GET(0, 1) * MAT_GET(1, 2) - MAT_GET(0, 2) * MAT_GET(1, 1))),
						static_cast<inverse_type>(det * -(MAT_GET(0, 0) * MAT_GET(1, 2) - MAT_GET(0, 2) * MAT_GET(1, 0))),
						static_cast<inverse_type>(det *  (MAT_GET(0, 0) * MAT_GET(1, 1) - MAT_GET(0, 1) * MAT_GET(1, 0)))
					}
				};
			}
			if constexpr (Columns == 4)
			{
				// generated using https://github.com/willnode/N-Matrix-Programmer

				MUU_FMA_BLOCK

				const auto A2323 = MAT_GET(2, 2) * MAT_GET(3, 3) - MAT_GET(2, 3) * MAT_GET(3, 2);
				const auto A1323 = MAT_GET(2, 1) * MAT_GET(3, 3) - MAT_GET(2, 3) * MAT_GET(3, 1);
				const auto A1223 = MAT_GET(2, 1) * MAT_GET(3, 2) - MAT_GET(2, 2) * MAT_GET(3, 1);
				const auto A0323 = MAT_GET(2, 0) * MAT_GET(3, 3) - MAT_GET(2, 3) * MAT_GET(3, 0);
				const auto A0223 = MAT_GET(2, 0) * MAT_GET(3, 2) - MAT_GET(2, 2) * MAT_GET(3, 0);
				const auto A0123 = MAT_GET(2, 0) * MAT_GET(3, 1) - MAT_GET(2, 1) * MAT_GET(3, 0);
				const auto A2313 = MAT_GET(1, 2) * MAT_GET(3, 3) - MAT_GET(1, 3) * MAT_GET(3, 2);
				const auto A1313 = MAT_GET(1, 1) * MAT_GET(3, 3) - MAT_GET(1, 3) * MAT_GET(3, 1);
				const auto A1213 = MAT_GET(1, 1) * MAT_GET(3, 2) - MAT_GET(1, 2) * MAT_GET(3, 1);
				const auto A2312 = MAT_GET(1, 2) * MAT_GET(2, 3) - MAT_GET(1, 3) * MAT_GET(2, 2);
				const auto A1312 = MAT_GET(1, 1) * MAT_GET(2, 3) - MAT_GET(1, 3) * MAT_GET(2, 1);
				const auto A1212 = MAT_GET(1, 1) * MAT_GET(2, 2) - MAT_GET(1, 2) * MAT_GET(2, 1);
				const auto A0313 = MAT_GET(1, 0) * MAT_GET(3, 3) - MAT_GET(1, 3) * MAT_GET(3, 0);
				const auto A0213 = MAT_GET(1, 0) * MAT_GET(3, 2) - MAT_GET(1, 2) * MAT_GET(3, 0);
				const auto A0312 = MAT_GET(1, 0) * MAT_GET(2, 3) - MAT_GET(1, 3) * MAT_GET(2, 0);
				const auto A0212 = MAT_GET(1, 0) * MAT_GET(2, 2) - MAT_GET(1, 2) * MAT_GET(2, 0);
				const auto A0113 = MAT_GET(1, 0) * MAT_GET(3, 1) - MAT_GET(1, 1) * MAT_GET(3, 0);
				const auto A0112 = MAT_GET(1, 0) * MAT_GET(2, 1) - MAT_GET(1, 1) * MAT_GET(2, 0);

				const auto det = intermediate_float{ 1 } / (
					  MAT_GET(0, 0) * (MAT_GET(1, 1) * A2323 - MAT_GET(1, 2) * A1323 + MAT_GET(1, 3) * A1223)
					- MAT_GET(0, 1) * (MAT_GET(1, 0) * A2323 - MAT_GET(1, 2) * A0323 + MAT_GET(1, 3) * A0223)
					+ MAT_GET(0, 2) * (MAT_GET(1, 0) * A1323 - MAT_GET(1, 1) * A0323 + MAT_GET(1, 3) * A0123)
					- MAT_GET(0, 3) * (MAT_GET(1, 0) * A1223 - MAT_GET(1, 1) * A0223 + MAT_GET(1, 2) * A0123)
				);

				return result_type
				{
					impl::columnwise_init_tag{},
					result_column{
						static_cast<inverse_type>(det *  (MAT_GET(1, 1) * A2323 - MAT_GET(1, 2) * A1323 + MAT_GET(1, 3) * A1223)),
						static_cast<inverse_type>(det * -(MAT_GET(1, 0) * A2323 - MAT_GET(1, 2) * A0323 + MAT_GET(1, 3) * A0223)),
						static_cast<inverse_type>(det *  (MAT_GET(1, 0) * A1323 - MAT_GET(1, 1) * A0323 + MAT_GET(1, 3) * A0123)),
						static_cast<inverse_type>(det * -(MAT_GET(1, 0) * A1223 - MAT_GET(1, 1) * A0223 + MAT_GET(1, 2) * A0123))
					},
					result_column{
						static_cast<inverse_type>(det * -(MAT_GET(0, 1) * A2323 - MAT_GET(0, 2) * A1323 + MAT_GET(0, 3) * A1223)),
						static_cast<inverse_type>(det *  (MAT_GET(0, 0) * A2323 - MAT_GET(0, 2) * A0323 + MAT_GET(0, 3) * A0223)),
						static_cast<inverse_type>(det * -(MAT_GET(0, 0) * A1323 - MAT_GET(0, 1) * A0323 + MAT_GET(0, 3) * A0123)),
						static_cast<inverse_type>(det *  (MAT_GET(0, 0) * A1223 - MAT_GET(0, 1) * A0223 + MAT_GET(0, 2) * A0123))
					},
					result_column{
						static_cast<inverse_type>(det *  (MAT_GET(0, 1) * A2313 - MAT_GET(0, 2) * A1313 + MAT_GET(0, 3) * A1213)),
						static_cast<inverse_type>(det * -(MAT_GET(0, 0) * A2313 - MAT_GET(0, 2) * A0313 + MAT_GET(0, 3) * A0213)),
						static_cast<inverse_type>(det *  (MAT_GET(0, 0) * A1313 - MAT_GET(0, 1) * A0313 + MAT_GET(0, 3) * A0113)),
						static_cast<inverse_type>(det * -(MAT_GET(0, 0) * A1213 - MAT_GET(0, 1) * A0213 + MAT_GET(0, 2) * A0113))
					},
					result_column{
						static_cast<inverse_type>(det * -(MAT_GET(0, 1) * A2312 - MAT_GET(0, 2) * A1312 + MAT_GET(0, 3) * A1212)),
						static_cast<inverse_type>(det *  (MAT_GET(0, 0) * A2312 - MAT_GET(0, 2) * A0312 + MAT_GET(0, 3) * A0212)),
						static_cast<inverse_type>(det * -(MAT_GET(0, 0) * A1312 - MAT_GET(0, 1) * A0312 + MAT_GET(0, 3) * A0112)),
						static_cast<inverse_type>(det *  (MAT_GET(0, 0) * A1212 - MAT_GET(0, 1) * A0212 + MAT_GET(0, 2) * A0112))
					}
				};
			}
			
			#undef MAT_GET
		}

		/// \brief	Inverts the matrix (in-place).
		/// 
		/// \note This function is only available when the matrix is square, has at most 4 rows and columns,
		/// 	  and has a floating-point #scalar_type.
		LEGACY_REQUIRES_SQUARE_AND(Columns <= 4 && is_floating_point<Scalar>)
		constexpr matrix& MUU_VECTORCALL invert() noexcept
			REQUIRES_SQUARE_AND(Columns <= 4 && is_floating_point<Scalar>)
		{
			return *this = invert(*this);
		}

	#endif // inverse & determinant

	#if 1 // streams --------------------------------------------------------------------------------------------------

		/// \brief Writes a matrix out to a text stream.
		template <typename Char, typename Traits>
		friend
		std::basic_ostream<Char, Traits>& operator << (std::basic_ostream<Char, Traits>& os, const matrix& m)
		{
			impl::print_matrix_to_stream(os, &m.get<0, 0>(), Rows, Columns);
			return os;
		}

	#endif // streams

	#if 1 // misc -----------------------------------------------------------------------------------------------------
	#endif // misc
	};

	#ifndef DOXYGEN // deduction guides -------------------------------------------------------------------------------

	template <typename T MUU_ENABLE_IF(is_arithmetic<T>)>
	MUU_REQUIRES(is_arithmetic<T>)
	matrix(T) -> matrix<T, 1, 1>;

	template <
		typename T1, typename T2,
		typename T3, typename T4
		MUU_ENABLE_IF(all_arithmetic<T1, T2, T3, T4>)
	>
	MUU_REQUIRES(all_arithmetic<T1, T2, T3, T4>)
	matrix(T1, T2, T3, T4) -> matrix<impl::highest_ranked<T1, T2, T3, T4>, 2, 2>;

	template <
		typename T1, typename T2, typename T3,
		typename T4, typename T5, typename T6,
		typename T7, typename T8, typename T9
		MUU_ENABLE_IF(all_arithmetic<T1, T2, T3, T4, T5, T6, T7, T8, T9>)
	>
	MUU_REQUIRES(all_arithmetic<T1, T2, T3, T4, T5, T6, T7, T8, T9>)
	matrix(T1, T2, T3, T4, T5, T6, T7, T8, T9) -> matrix<impl::highest_ranked<T1, T2, T3, T4, T5, T6, T7, T8, T9>, 3, 3>;

	template <
		typename T1, typename T2, typename T3, typename T4,
		typename T5, typename T6, typename T7, typename T8,
		typename T9, typename T10, typename T11, typename T12
		MUU_ENABLE_IF(all_arithmetic<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12>)
	>
	MUU_REQUIRES(all_arithmetic<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12>)
	matrix(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12)
		-> matrix<impl::highest_ranked<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12>, 3, 4>;

	template <
		typename T1, typename T2, typename T3, typename T4,
		typename T5, typename T6, typename T7, typename T8,
		typename T9, typename T10, typename T11, typename T12,
		typename T13, typename T14, typename T15, typename T16
		MUU_ENABLE_IF(all_arithmetic<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16>)
	>
	MUU_REQUIRES(all_arithmetic<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16>)
	matrix(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16)
		-> matrix<impl::highest_ranked<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16>, 4, 4>;

	#endif // deduction guides
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

MUU_NAMESPACE_START
{
	/// \ingroup	infinity_or_nan
	/// \related	muu::matrix
	///
	/// \brief	Returns true if any of the scalar components of a matrix are infinity or NaN.
	template <typename S, size_t R, size_t C
		ENABLE_PAIRED_FUNC_BY_REF(S, R, C, true)
	>
	REQUIRES_PAIRED_FUNC_BY_REF(S, R, C, true)
	[[nodiscard]]
	MUU_ATTR(pure)
	MUU_ALWAYS_INLINE
	constexpr bool MUU_VECTORCALL infinity_or_nan(const matrix<S, R, C>& m) noexcept
	{
		if constexpr (is_floating_point<S>)
			return matrix<S, R, C>::infinity_or_nan(m);
		else
		{
			(void)m;
			return false;
		}
	}

	/// \ingroup	approx_equal
	/// \related	muu::matrix
	///
	/// \brief		Returns true if two matrices are approximately equal.
	///
	/// \note		This function is only available when at least one of `S` and `T` is a floating-point type.
	template <typename S, typename T, size_t R, size_t C,
		typename Epsilon = impl::highest_ranked<S, T>
		ENABLE_PAIRED_FUNC_BY_REF(S, R, C, any_floating_point<S, T>&& impl::pass_readonly_by_reference<matrix<T, R, C>>)
	>
	REQUIRES_PAIRED_FUNC_BY_REF(S, R, C, any_floating_point<S, T> && impl::pass_readonly_by_reference<matrix<T, R, C>>)
	[[nodiscard]]
	MUU_ATTR(pure)
	MUU_ALWAYS_INLINE
	constexpr bool MUU_VECTORCALL approx_equal(
		const matrix<S, R, C>& m1,
		const matrix<T, R, C>& m2,
		dont_deduce<Epsilon> epsilon = muu::constants<Epsilon>::approx_equal_epsilon
	) noexcept
	{
		static_assert(is_same_as_any<Epsilon, S, T>);

		return matrix<S, R, C>::approx_equal(m1, m2, epsilon);
	}

	/// \ingroup	approx_zero
	/// \related	muu::matrix
	///
	/// \brief		Returns true if all the scalar components of a matrix are approximately equal to zero.
	///
	/// \note		This function is only available when `S` is a floating-point type.
	template <typename S, size_t R, size_t C
		ENABLE_PAIRED_FUNC_BY_REF(S, R, C, is_floating_point<S>)
	>
	REQUIRES_PAIRED_FUNC_BY_REF(S, R, C, is_floating_point<S>)
	[[nodiscard]]
	MUU_ATTR(pure)
	MUU_ALWAYS_INLINE
	constexpr bool MUU_VECTORCALL approx_zero(
		const matrix<S, R, C>& m,
		S epsilon = muu::constants<S>::approx_equal_epsilon
	) noexcept
	{
		return matrix<S, R, C>::approx_zero(m, epsilon);
	}

	#if ENABLE_PAIRED_FUNCS

	template <typename S, size_t R, size_t C
		ENABLE_PAIRED_FUNC_BY_VAL(S, R, C, true)
	>
	REQUIRES_PAIRED_FUNC_BY_VAL(S, R, C, true)
	[[nodiscard]]
	MUU_ATTR(const)
	MUU_ALWAYS_INLINE
	constexpr bool MUU_VECTORCALL infinity_or_nan(matrix<S, R, C> m) noexcept
	{
		if constexpr (is_floating_point<S>)
			return matrix<S, R, C>::infinity_or_nan(m);
		else
		{
			(void)m;
			return false;
		}
	}

	template <typename S, typename T, size_t R, size_t C,
		typename Epsilon = impl::highest_ranked<S, T>
		ENABLE_PAIRED_FUNC_BY_VAL(S, R, C, any_floating_point<S, T>&& impl::pass_readonly_by_value<matrix<T, R, C>>)
	>
	REQUIRES_PAIRED_FUNC_BY_VAL(S, R, C, any_floating_point<S, T> && impl::pass_readonly_by_value<matrix<T, R, C>>)
	[[nodiscard]]
	MUU_ATTR(const)
	MUU_ALWAYS_INLINE
	constexpr bool approx_equal(
		matrix<S, R, C> m1,
		matrix<T, R, C> m2,
		dont_deduce<Epsilon> epsilon = muu::constants<Epsilon>::approx_equal_epsilon
	) noexcept
	{
		static_assert(is_same_as_any<Epsilon, S, T>);

		return matrix<S, R, C>::approx_equal(m1, m2, epsilon);
	}

	template <typename S, size_t R, size_t C
		ENABLE_PAIRED_FUNC_BY_VAL(S, R, C, is_floating_point<S>)
	>
	REQUIRES_PAIRED_FUNC_BY_VAL(S, R, C, is_floating_point<S>)
	[[nodiscard]]
	MUU_ATTR(const)
	MUU_ALWAYS_INLINE
	constexpr bool approx_zero(
		matrix<S, R, C> m,
		S epsilon = muu::constants<S>::approx_equal_epsilon
	) noexcept
	{
		return matrix<S, R, C>::approx_zero(m, epsilon);
	}

	#endif // ENABLE_PAIRED_FUNCS
}
MUU_NAMESPACE_END

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
#undef REQUIRES_SQUARE_AND
#undef REQUIRES_FLOATING_POINT
#undef REQUIRES_INTEGRAL
#undef REQUIRES_SIGNED
#undef ENABLE_IF_SIZE_AT_LEAST_AND
#undef LEGACY_REQUIRES_SIZE_AT_LEAST
#undef LEGACY_REQUIRES_SIZE_AT_LEAST_AND
#undef LEGACY_REQUIRES_SIZE_EXACTLY
#undef LEGACY_REQUIRES_ROWS_AT_LEAST
#undef LEGACY_REQUIRES_ROWS_EXACTLY
#undef LEGACY_REQUIRES_COLUMNS_AT_LEAST
#undef LEGACY_REQUIRES_COLUMNS_EXACTLY
#undef LEGACY_REQUIRES_DIMENSIONS_AT_LEAST
#undef LEGACY_REQUIRES_DIMENSIONS_EXACTLY
#undef LEGACY_REQUIRES_SQUARE_AND
#undef LEGACY_REQUIRES_FLOATING_POINT
#undef LEGACY_REQUIRES_INTEGRAL
#undef LEGACY_REQUIRES_SIGNED
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
