// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once
/// \cond

#include "../vector.h"
#include "std_tuple.h"
#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(float_control(except, off))

namespace muu::impl
{
	//--- base class----------------------------------------------------------------------------------------------------

	struct columnwise_init_tag
	{};
	struct columnwise_copy_tag
	{};
	struct row_major_tuple_tag
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct MUU_TRIVIAL_ABI matrix_
	{
		vector<Scalar, Rows> m[Columns];

		matrix_() noexcept = default;

#if MUU_MSVC // still problematic as of 19.33

	  private:
		// internal compiler error:
		// https://developercommunity2.visualstudio.com/t/C-Internal-Compiler-Error-in-constexpr/1264044

		template <size_t Index>
		MUU_CONST_INLINE_GETTER
		static constexpr vector<Scalar, Rows> fill_column_initializer_msvc(Scalar fill) noexcept
		{
			return vector<Scalar, Rows>{ fill };
		}

	  public:
		template <size_t... ColumnIndices>
		explicit constexpr matrix_(value_fill_tag, std::index_sequence<ColumnIndices...>, Scalar fill) noexcept
			: m{ fill_column_initializer_msvc<ColumnIndices>(fill)... }
		{
			static_assert(sizeof...(ColumnIndices) <= Columns);
		}

#else

		template <size_t... ColumnIndices>
		explicit constexpr matrix_(value_fill_tag, std::index_sequence<ColumnIndices...>, Scalar fill) noexcept
			: m{ (MUU_UNUSED(ColumnIndices), vector<Scalar, Rows>{ fill })... }
		{
			static_assert(sizeof...(ColumnIndices) <= Columns);
		}

#endif

		template <typename... T>
		explicit constexpr matrix_(columnwise_init_tag, T... cols) noexcept //
			: m{ cols... }
		{
			static_assert(sizeof...(T) <= Columns);
		}

		template <typename T, size_t... ColumnIndices>
		explicit constexpr matrix_(columnwise_copy_tag, std::index_sequence<ColumnIndices...>, const T& cols) noexcept
			: m{ vector<Scalar, Rows>{ cols[ColumnIndices] }... }
		{
			static_assert(sizeof...(ColumnIndices) <= Columns);
		}

	  private:
		template <size_t Index, typename T>
		MUU_PURE_INLINE_GETTER
		static constexpr decltype(auto) get_tuple_value_or_zero(const T& tpl) noexcept
		{
			if constexpr (Index < tuple_size<T>)
			{
				return get_from_tuple_like<Index>(tpl);
			}
			else
			{
				MUU_UNUSED(tpl);
				return Scalar{};
			}
		}

		template <size_t Column, typename T, size_t... RowIndices>
		MUU_PURE_GETTER
		static constexpr vector<Scalar, Rows> column_from_row_major_tuple(const T& tpl,
																		  std::index_sequence<RowIndices...>) noexcept
		{
			static_assert(sizeof...(RowIndices) == Rows);

			return vector<Scalar, Rows>{ static_cast<Scalar>(
				get_tuple_value_or_zero<Column + (Columns * RowIndices)>(tpl))... };
		}

	  public:
		template <typename T, size_t... ColumnIndices>
		explicit constexpr matrix_(const T& tpl, std::index_sequence<ColumnIndices...>) noexcept
			: m{ column_from_row_major_tuple<ColumnIndices>(tpl, std::make_index_sequence<Rows>{})... }
		{
			static_assert(tuple_size<T> <= Rows * Columns);
			static_assert(sizeof...(ColumnIndices) == Columns);
		}

		template <typename T>
		explicit constexpr matrix_(row_major_tuple_tag, const T& tpl) noexcept
			: matrix_{ tpl, std::make_index_sequence<Columns>{} }
		{
			static_assert(tuple_size<T> <= Rows * Columns);
		}

		// row-major scalar constructor optimizations for some common cases

		// 2x2
		MUU_HIDDEN_CONSTRAINT((R == 2 && C == 2), size_t R = Rows, size_t C = Columns)
		constexpr matrix_(Scalar v00, Scalar v01, Scalar v10 = Scalar{}, Scalar v11 = Scalar{}) noexcept
			: m{ { v00, v10 }, { v01, v11 } }
		{}

		// 2x3
		MUU_HIDDEN_CONSTRAINT((R == 2 && C == 3), size_t R = Rows, size_t C = Columns)
		constexpr matrix_(Scalar v00,
						  Scalar v01,
						  Scalar v02 = Scalar{},
						  Scalar v10 = Scalar{},
						  Scalar v11 = Scalar{},
						  Scalar v12 = Scalar{}) noexcept //
			: m{ { v00, v10 }, { v01, v11 }, { v02, v12 } }
		{}

		// 3x3
		MUU_HIDDEN_CONSTRAINT((R == 3 && C == 3), size_t R = Rows, size_t C = Columns)
		constexpr matrix_(Scalar v00,
						  Scalar v01,
						  Scalar v02 = Scalar{},
						  Scalar v10 = Scalar{},
						  Scalar v11 = Scalar{},
						  Scalar v12 = Scalar{},
						  Scalar v20 = Scalar{},
						  Scalar v21 = Scalar{},
						  Scalar v22 = Scalar{}) noexcept //
			: m{ { v00, v10, v20 }, { v01, v11, v21 }, { v02, v12, v22 } }
		{}

		// 3x4
		MUU_HIDDEN_CONSTRAINT((R == 3 && C == 4), size_t R = Rows, size_t C = Columns)
		constexpr matrix_(Scalar v00,
						  Scalar v01,
						  Scalar v02 = Scalar{},
						  Scalar v03 = Scalar{},
						  Scalar v10 = Scalar{},
						  Scalar v11 = Scalar{},
						  Scalar v12 = Scalar{},
						  Scalar v13 = Scalar{},
						  Scalar v20 = Scalar{},
						  Scalar v21 = Scalar{},
						  Scalar v22 = Scalar{},
						  Scalar v23 = Scalar{}) noexcept //
			: m{ { v00, v10, v20 }, { v01, v11, v21 }, { v02, v12, v22 }, { v03, v13, v23 } }
		{}

		// 4x4
		MUU_HIDDEN_CONSTRAINT((R == 4 && C == 4), size_t R = Rows, size_t C = Columns)
		constexpr matrix_(Scalar v00,
						  Scalar v01,
						  Scalar v02 = Scalar{},
						  Scalar v03 = Scalar{},
						  Scalar v10 = Scalar{},
						  Scalar v11 = Scalar{},
						  Scalar v12 = Scalar{},
						  Scalar v13 = Scalar{},
						  Scalar v20 = Scalar{},
						  Scalar v21 = Scalar{},
						  Scalar v22 = Scalar{},
						  Scalar v23 = Scalar{},
						  Scalar v30 = Scalar{},
						  Scalar v31 = Scalar{},
						  Scalar v32 = Scalar{},
						  Scalar v33 = Scalar{}) noexcept //
			: m{ { v00, v10, v20, v30 }, { v01, v11, v21, v31 }, { v02, v12, v22, v32 }, { v03, v13, v23, v33 } }
		{}
	};

	//--- hva/vectorcall -----------------------------------------------------------------------------------------------

	template <typename Scalar, size_t Rows, size_t Columns>
	inline constexpr bool is_hva<matrix_<Scalar, Rows, Columns>> =
		can_be_hva_of<Scalar, matrix_<Scalar, Rows, Columns>>;

	template <typename Scalar, size_t Rows, size_t Columns>
	inline constexpr bool is_hva<matrix<Scalar, Rows, Columns>> = is_hva<matrix_<Scalar, Rows, Columns>>;

	template <typename Scalar, size_t Rows, size_t Columns>
	struct vectorcall_param_<matrix<Scalar, Rows, Columns>>
	{
		using type = std::conditional_t<pass_vectorcall_by_value<matrix_<Scalar, Rows, Columns>>,
										matrix<Scalar, Rows, Columns>,
										const matrix<Scalar, Rows, Columns>&>;
	};

	//--- matrix classification  ---------------------------------------------------------------------------------------

	template <typename T>
	inline constexpr bool is_floating_point_matrix_ = false;
	template <template <typename, size_t, size_t> typename Matrix, typename T, size_t R, size_t C>
	inline constexpr bool is_floating_point_matrix_<Matrix<T, R, C>> = is_matrix_<Matrix<T, R, C>> //
		&& is_floating_point<T>;

	// "common matrix" == a matrix which is of a common size (has special constructor overloads)
	template <typename T>
	inline constexpr bool is_common_matrix_ = false;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_common_matrix_<Matrix<T, 2, 2>> = is_matrix_<Matrix<T, 2, 2>>;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_common_matrix_<Matrix<T, 2, 3>> = is_matrix_<Matrix<T, 2, 3>>;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_common_matrix_<Matrix<T, 3, 3>> = is_matrix_<Matrix<T, 3, 3>>;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_common_matrix_<Matrix<T, 3, 4>> = is_matrix_<Matrix<T, 3, 4>>;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_common_matrix_<Matrix<T, 4, 4>> = is_matrix_<Matrix<T, 4, 4>>;

	// "2d scale matrix" == any 2d transform matrix (all can encode scale)
	template <typename T>
	inline constexpr bool is_2d_scale_matrix_ = false;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_2d_scale_matrix_<Matrix<T, 2, 2>> = is_matrix_<Matrix<T, 2, 2>>;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_2d_scale_matrix_<Matrix<T, 2, 3>> = is_matrix_<Matrix<T, 2, 3>>;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_2d_scale_matrix_<Matrix<T, 3, 3>> = is_matrix_<Matrix<T, 3, 3>>;

	// "3d scale matrix" == any 3d transform matrix (all can encode scale)
	template <typename T>
	inline constexpr bool is_3d_scale_matrix_ = false;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_3d_scale_matrix_<Matrix<T, 3, 3>> = is_matrix_<Matrix<T, 3, 3>>;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_3d_scale_matrix_<Matrix<T, 3, 4>> = is_matrix_<Matrix<T, 3, 4>>;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_3d_scale_matrix_<Matrix<T, 4, 4>> = is_matrix_<Matrix<T, 4, 4>>;

	// "scale matrix" == a matrix which satisfies is_2d_scale_matrix_ or is_3d_scale_matrix_
	template <typename T>
	inline constexpr bool is_scale_matrix_ = is_2d_scale_matrix_<T> //
										  || is_3d_scale_matrix_<T>;

	// "2d rotation matrix" == any 2d transform matrix with floating-point scalars
	template <typename T>
	inline constexpr bool is_2d_rotation_matrix_ = false;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_2d_rotation_matrix_<Matrix<T, 2, 2>> = is_matrix_<Matrix<T, 2, 2>>&& is_floating_point<T>;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_2d_rotation_matrix_<Matrix<T, 2, 3>> = is_matrix_<Matrix<T, 2, 3>>&& is_floating_point<T>;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_2d_rotation_matrix_<Matrix<T, 3, 3>> = is_matrix_<Matrix<T, 3, 3>>&& is_floating_point<T>;

	// "3d rotation matrix" == any 3d transform matrix with floating-point scalars
	template <typename T>
	inline constexpr bool is_3d_rotation_matrix_ = false;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_3d_rotation_matrix_<Matrix<T, 3, 3>> = is_matrix_<Matrix<T, 3, 3>>&& is_floating_point<T>;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_3d_rotation_matrix_<Matrix<T, 3, 4>> = is_matrix_<Matrix<T, 3, 4>>&& is_floating_point<T>;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_3d_rotation_matrix_<Matrix<T, 4, 4>> = is_matrix_<Matrix<T, 4, 4>>&& is_floating_point<T>;

	// "rotation matrix" == a matrix which satisfies is_2d_rotation_matrix_ or is_3d_rotation_matrix_
	template <typename T>
	inline constexpr bool is_rotation_matrix_ = is_2d_rotation_matrix_<T> //
											 || is_3d_rotation_matrix_<T>;

	// "2d translation matrix" == a 2d transform matrix which contains a translation column
	template <typename T>
	inline constexpr bool is_2d_translation_matrix_ = false;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_2d_translation_matrix_<Matrix<T, 2, 3>> = is_matrix_<Matrix<T, 2, 3>>;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_2d_translation_matrix_<Matrix<T, 3, 3>> = is_matrix_<Matrix<T, 3, 3>>;

	// "3d translation matrix" == a 2d transform matrix which contains a translation column
	template <typename T>
	inline constexpr bool is_3d_translation_matrix_ = false;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_3d_translation_matrix_<Matrix<T, 3, 4>> = is_matrix_<Matrix<T, 3, 4>>;
	template <template <typename, size_t, size_t> typename Matrix, typename T>
	inline constexpr bool is_3d_translation_matrix_<Matrix<T, 4, 4>> = is_matrix_<Matrix<T, 4, 4>>;

	// "translation matrix" == a matrix which satisfies is_2d_translation_matrix_ or is_3d_translation_matrix_
	template <typename T>
	inline constexpr bool is_translation_matrix_ = is_2d_translation_matrix_<T> //
												|| is_3d_translation_matrix_<T>;

	// "2d transform matrix" == a matrix which satisfies any of the is_2d_ constraints and has floating-point scalars
	template <typename T>
	inline constexpr bool is_2d_transform_matrix_ = is_floating_point_matrix_<T>	 //
												 && (is_2d_scale_matrix_<T>			 //
													 || is_2d_translation_matrix_<T> //
													 || is_2d_rotation_matrix_<T>);

	// "3d transform matrix" == a matrix which satisfies any of the is_3d_ constraints and has floating-point scalars
	template <typename T>
	inline constexpr bool is_3d_transform_matrix_ = is_floating_point_matrix_<T>	 //
												 && (is_3d_scale_matrix_<T>			 //
													 || is_3d_translation_matrix_<T> //
													 || is_3d_rotation_matrix_<T>);

	// "transform matrix" == a matrix which satisfies is_3d_transform_matrix_ or is_3d_transform_matrix_
	template <typename T>
	inline constexpr bool is_transform_matrix_ = is_2d_transform_matrix_<T> //
											  || is_3d_transform_matrix_<T>;
}

namespace muu
{
	template <typename From, typename Scalar, size_t Rows, size_t Columns>
	inline constexpr bool allow_implicit_bit_cast<From, impl::matrix_<Scalar, Rows, Columns>> =
		allow_implicit_bit_cast<From, matrix<Scalar, Rows, Columns>>;
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
/// \endcond
