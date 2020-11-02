// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::matrix.

#pragma once
#include "../muu/quaternion.h"

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
	template <typename Scalar, size_t Rows, size_t Columns>
	struct MUU_TRIVIAL_ABI matrix_base
	{
		vector<Scalar, Rows> c[Columns];

		matrix_base() noexcept = default;

		template <size_t... ColumnIndices>
		explicit constexpr matrix_base(Scalar fill, std::index_sequence<ColumnIndices...>) noexcept
			: c{ ((void)ColumnIndices, vector<Scalar, Rows>{ fill })... }
		{
			static_assert(sizeof...(ColumnIndices) <= Columns);
		}

		explicit constexpr matrix_base(value_fill_tag, Scalar fill) noexcept
			: matrix_base{ fill, std::make_index_sequence<Columns>{} }
		{}
	};

	#if MUU_HAS_VECTORCALL

	template <typename Scalar, size_t Rows, size_t Columns>
	inline constexpr bool is_hva<matrix_base<Scalar, Rows, Columns>> = can_be_hva_of<matrix_base<Scalar, Rows, Columns>, Scalar>;

	template <typename Scalar, size_t Rows, size_t Columns>
	inline constexpr bool is_hva<matrix<Scalar, Rows, Columns>> = is_hva<matrix_base<Scalar, Rows, Columns>>;

	#endif // MUU_HAS_VECTORCALL

	template <typename Scalar, size_t Rows, size_t Columns>
	inline constexpr bool pass_matrix_by_reference = pass_readonly_by_reference<matrix<Scalar, Rows, Columns>>;

	template <typename Scalar, size_t Rows, size_t Columns>
	inline constexpr bool pass_matrix_by_value = !pass_matrix_by_reference<Scalar, Rows, Columns>;
}
MUU_IMPL_NAMESPACE_END

#endif // !DOXYGEN

#endif // =============================================================================================================

//=====================================================================================================================
// MATRIX CLASS
#if 1

MUU_NAMESPACE_START
{
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

		/// \brief Compile-time constants for this matrix.
		using constants = muu::constants<matrix>;

	private:

		using base = impl::matrix_base<Scalar, Rows, Columns>;
		static_assert(
			sizeof(base) == (sizeof(scalar_type) * Rows * Columns),
			"Matrices should not have padding"
		);
		using intermediate_type = impl::promote_if_small_float<scalar_type>;

	public:

		#ifdef DOXYGEN
		/// \brief The values in the matrix (stored column-major).
		vector<Scalar, rows> c[columns];
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
		/// \param	fill	The value used to initialize each of the matrix's scalar components.
		MUU_NODISCARD_CTOR
		explicit constexpr matrix(scalar_type fill) noexcept
			: base{ impl::value_fill_tag{}, fill }
		{}

	#endif // constructors

	#if 1 // ___________ ---------------------------------------------------------------------------------------------
	#endif // ___________

	#if 1 // ___________ ---------------------------------------------------------------------------------------------
	#endif // ___________

	#if 1 // ___________ ---------------------------------------------------------------------------------------------
	#endif // ___________

	#if 1 // ___________ ---------------------------------------------------------------------------------------------
	#endif // ___________

	#if 1 // ___________ ---------------------------------------------------------------------------------------------
	#endif // ___________

	#if 1 // ___________ ---------------------------------------------------------------------------------------------
	#endif // ___________

	#if 1 // ___________ ---------------------------------------------------------------------------------------------
	#endif // ___________

	#if 1 // ___________ ---------------------------------------------------------------------------------------------
	#endif // ___________

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

MUU_PRAGMA_MSVC(pop_macro("min"))
MUU_PRAGMA_MSVC(pop_macro("max"))
MUU_PRAGMA_MSVC(float_control(pop))
MUU_PRAGMA_MSVC(inline_recursion(off))

MUU_POP_WARNINGS	// MUU_DISABLE_SHADOW_WARNINGS
