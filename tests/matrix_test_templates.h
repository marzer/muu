// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"
#include "../include/muu/matrix.h"

template <typename T>
inline constexpr bool matrix_invoke_trait_tests = false;

template <typename T, size_t Rows, size_t Columns>
inline constexpr void matrix_trait_tests(std::string_view /*scalar_typename*/) noexcept
{
	using matrix_t = matrix<T, Rows, Columns>;
	static_assert(sizeof(matrix_t) == sizeof(T) * Rows * Columns);
	static_assert(sizeof(matrix_t[5]) == sizeof(T) * Rows * Columns * 5);
	static_assert(std::is_standard_layout_v<matrix_t>);
	static_assert(std::is_trivially_constructible_v<matrix_t>);
	static_assert(std::is_trivially_copy_constructible_v<matrix_t>);
	static_assert(std::is_trivially_copy_assignable_v<matrix_t>);
	static_assert(std::is_trivially_move_constructible_v<matrix_t>);
	static_assert(std::is_trivially_move_assignable_v<matrix_t>);
	static_assert(std::is_trivially_destructible_v<matrix_t>);
	static_assert(std::is_nothrow_constructible_v<matrix_t, T>);

	#if MUU_HAS_VECTORCALL
	static_assert(impl::is_hva<matrix_t> == (
		(Rows * Columns) <= 4
		&& is_same_as_any<T, float, double, long double>
	));
	#endif
};

template <typename T, size_t Rows, size_t Columns, size_t NUM>
inline void matrix_construction_test_from_scalars() noexcept
{
	static_assert(NUM != 1 || (Rows * Columns) == 1);

	if constexpr (Rows * Columns >= NUM)
	{
		INFO("constructing from "sv << NUM << " scalars"sv)
		using matrix_t = matrix<T, Rows, Columns>;

		const auto vals = random_array<T, NUM>();

		// std::apply is super taxing for the compiler to instantiate on some implementations
		// I'm simulating it for small value counts
		matrix_t mat;
		if constexpr (NUM == 1) mat = matrix_t{ vals[0] };
		if constexpr (NUM == 2) mat = matrix_t{ vals[0], vals[1] };
		if constexpr (NUM == 3) mat = matrix_t{ vals[0], vals[1], vals[2] };
		if constexpr (NUM == 4) mat = matrix_t{ vals[0], vals[1], vals[2], vals[3] };
		if constexpr (NUM == 5) mat = matrix_t{ vals[0], vals[1], vals[2], vals[3], vals[4] };
		if constexpr (NUM == 6) mat = matrix_t{ vals[0], vals[1], vals[2], vals[3], vals[4], vals[5] };
		if constexpr (NUM == 7) mat = matrix_t{ vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6] };
		if constexpr (NUM == 8) mat = matrix_t{ vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6], vals[7] };
		if constexpr (NUM == 9) mat = matrix_t{ vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6], vals[7], vals[8] };
		if constexpr (NUM > 9)  mat = std::apply([](auto&& ... s) noexcept { return matrix_t{ s... }; }, vals);

		// scalar constructor is row-major, matrix value storage is column-major!
		for (size_t r = 0, i = 0; r < Rows; r++)
		{
			for (size_t c = 0; c < Columns; c++, i++)
			{
				if (i < NUM)
					CHECK(mat(r, c) == vals[i]);
				else
					CHECK(mat(r, c) == T{});
			}
		}
	}
}

template <typename T, size_t Rows, size_t Columns, size_t R, size_t C>
inline void matrix_construction_test_from_smaller_matrix() noexcept
{
	if constexpr (Rows >= R && Columns >= C && (Rows > R || Columns > C))
	{
		INFO("constructing from a smaller matrix with "sv << R << " x "sv << C << " elements"sv)
		using matrix_t = matrix<T, Rows, Columns>;

		matrix<T, R, C> smaller;
		for (size_t r = 0; r < R; r++)
			for (size_t c = 0; c < C; c++)
				smaller(r, c) = random<T>();

		auto mat = matrix_t{ smaller };
		for (size_t r = 0; r < Rows; r++)
		{
			for (size_t c = 0; c < Columns; c++)
			{
				if (r < R && c < C)
					CHECK(mat(r, c) == smaller(r, c));
				else
					CHECK(mat(r, c) == T{});
			}
		}
	}
}

template <typename T, size_t Rows, size_t Columns, size_t R, size_t C>
inline void matrix_construction_test_from_larger_matrix() noexcept
{
	if constexpr (Rows <= R && Columns <= C && (Rows < R || Columns < C))
	{
		INFO("constructing from a larger matrix with "sv << R << " x "sv << C << " elements"sv)
		using matrix_t = matrix<T, Rows, Columns>;

		matrix<T, R, C> larger;
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				larger(r, c) = random<T>();

		auto mat = matrix_t{ larger };
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				CHECK(mat(r, c) == larger(r, c));
	}
}

template <typename T, size_t Rows, size_t Columns>
inline void matrix_construction_tests(std::string_view scalar_typename) noexcept
{
	INFO("matrix<"sv << scalar_typename << ", "sv << Rows << ", "sv << Columns << ">"sv)
	using matrix_t = matrix<T, Rows, Columns>;

	{
		INFO("fill constructor")

		const auto val = random<T>();
		matrix_t m{ val };
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				CHECK(m(r, c) == val);
	}

	{
		INFO("copy constructor"sv)
		matrix_t m1;
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				m1(r, c) = random<T>();

		matrix_t m2{ m1 };
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				CHECK(m1(r, c) == m2(r, c));
	}

	// scalar constructors
	//construction_test_from_scalars<T, Rows, Columns, 2>();
	matrix_construction_test_from_scalars<T, Rows, Columns, 3>();
	matrix_construction_test_from_scalars<T, Rows, Columns, 4>();
	matrix_construction_test_from_scalars<T, Rows, Columns, 5>();
	matrix_construction_test_from_scalars<T, Rows, Columns, 7>();
	matrix_construction_test_from_scalars<T, Rows, Columns, 10>();
	matrix_construction_test_from_scalars<T, Rows, Columns, 12>();
	matrix_construction_test_from_scalars<T, Rows, Columns, 16>();
	matrix_construction_test_from_scalars<T, Rows, Columns, 20>();
	// no single-scalar test; it's the fill constructor.
	
	// coercing constructor
	{
		using other_type = std::conditional_t<
			is_same_as_any<T, signed int, unsigned int>,
			float,
			set_signed<signed int, is_signed<T>>
		>;

		matrix<other_type, Rows, Columns> other;
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				other(r, c) = random<other_type>();

		matrix_t coerced{ other };
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				CHECK(coerced(r, c) == static_cast<T>(other(r, c)));
	}

	// enlarging constructor
	matrix_construction_test_from_smaller_matrix<T, Rows, Columns, 1, 1>();
	matrix_construction_test_from_smaller_matrix<T, Rows, Columns, 1, 2>();
	matrix_construction_test_from_smaller_matrix<T, Rows, Columns, 1, 3>();
	matrix_construction_test_from_smaller_matrix<T, Rows, Columns, 2, 1>();
	matrix_construction_test_from_smaller_matrix<T, Rows, Columns, 2, 2>();
	matrix_construction_test_from_smaller_matrix<T, Rows, Columns, 2, 3>();
	matrix_construction_test_from_smaller_matrix<T, Rows, Columns, 3, 1>();
	matrix_construction_test_from_smaller_matrix<T, Rows, Columns, 3, 2>();
	matrix_construction_test_from_smaller_matrix<T, Rows, Columns, 3, 3>();

	// truncating constructor
	matrix_construction_test_from_larger_matrix<T, Rows, Columns, 3, 3>();
	matrix_construction_test_from_larger_matrix<T, Rows, Columns, 3, 6>();
	matrix_construction_test_from_larger_matrix<T, Rows, Columns, 3, 12>();
	matrix_construction_test_from_larger_matrix<T, Rows, Columns, 6, 3>();
	matrix_construction_test_from_larger_matrix<T, Rows, Columns, 6, 6>();
	matrix_construction_test_from_larger_matrix<T, Rows, Columns, 6, 12>();
	matrix_construction_test_from_larger_matrix<T, Rows, Columns, 12, 3>();
	matrix_construction_test_from_larger_matrix<T, Rows, Columns, 12, 6>();
	matrix_construction_test_from_larger_matrix<T, Rows, Columns, 12, 12>();

}

template <typename T, size_t Rows, size_t Columns>
inline void matrix_accessor_tests(std::string_view scalar_typename) noexcept
{
	INFO("matrix<"sv << scalar_typename << ", "sv << Rows << ", "sv << Columns << ">"sv)
	using matrix_t = matrix<T, Rows, Columns>;

	const auto vals = random_array<T, Rows * Columns>();
	std::array<const T*, Rows> rows;
	for (size_t i = 0; i < Rows; i++)
		rows[i] = vals.data() + i * Columns;
	matrix_t mat;
	for (size_t r = 0; r < Rows; r++)
		for (size_t c = 0; c < Columns; c++)
			mat.m[c][r] = rows[r][c];
	const matrix_t& mat_const = mat;

	{
		INFO("operator()")
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				CHECK(rows[r][c] == mat(r, c));
	}

	{
		INFO("get()"sv)
		if constexpr (Rows >= 1 && Columns >= 1) CHECK(rows[0][0] == (mat.template get<0, 0>()));
		if constexpr (Rows >= 1 && Columns >= 2) CHECK(rows[0][1] == (mat.template get<0, 1>()));
		if constexpr (Rows >= 1 && Columns >= 3) CHECK(rows[0][2] == (mat.template get<0, 2>()));
		if constexpr (Rows >= 2 && Columns >= 1) CHECK(rows[1][0] == (mat.template get<1, 0>()));
		if constexpr (Rows >= 2 && Columns >= 2) CHECK(rows[1][1] == (mat.template get<1, 1>()));
		if constexpr (Rows >= 2 && Columns >= 3) CHECK(rows[1][2] == (mat.template get<1, 2>()));
		if constexpr (Rows >= 3 && Columns >= 1) CHECK(rows[2][0] == (mat.template get<2, 0>()));
		if constexpr (Rows >= 3 && Columns >= 2) CHECK(rows[2][1] == (mat.template get<2, 1>()));
		if constexpr (Rows >= 3 && Columns >= 3) CHECK(rows[2][2] == (mat.template get<2, 2>()));
	}

	{
		INFO("operator() (const)")
		for (size_t r = 0; r < Rows; r++)
		{
			for (size_t c = 0; c < Columns; c++)
			{
				CHECK(rows[r][c] == mat_const(r, c));
				CHECK(&mat(r, c) == &mat_const(r, c));
			}
		}
	}

	{
		INFO("get() (const)"sv)
		if constexpr (Rows >= 1 && Columns >= 1) CHECK(rows[0][0] == (mat_const.template get<0, 0>()));
		if constexpr (Rows >= 1 && Columns >= 2) CHECK(rows[0][1] == (mat_const.template get<0, 1>()));
		if constexpr (Rows >= 1 && Columns >= 3) CHECK(rows[0][2] == (mat_const.template get<0, 2>()));
		if constexpr (Rows >= 2 && Columns >= 1) CHECK(rows[1][0] == (mat_const.template get<1, 0>()));
		if constexpr (Rows >= 2 && Columns >= 2) CHECK(rows[1][1] == (mat_const.template get<1, 1>()));
		if constexpr (Rows >= 2 && Columns >= 3) CHECK(rows[1][2] == (mat_const.template get<1, 2>()));
		if constexpr (Rows >= 3 && Columns >= 1) CHECK(rows[2][0] == (mat_const.template get<2, 0>()));
		if constexpr (Rows >= 3 && Columns >= 2) CHECK(rows[2][1] == (mat_const.template get<2, 1>()));
		if constexpr (Rows >= 3 && Columns >= 3) CHECK(rows[2][2] == (mat_const.template get<2, 2>()));
	}
}

template <typename T, size_t Rows, size_t Columns>
inline void matrix_equality_tests(std::string_view scalar_typename) noexcept
{
	INFO("matrix<"sv << scalar_typename << ", "sv << Rows << ", "sv << Columns << ">"sv)
	using matrix_t = matrix<T, Rows, Columns>;

	matrix_t mat;
	for (size_t r = 0; r < Rows; r++)
		for (size_t c = 0; c < Columns; c++)
			mat(r, c) = random<T>();
		
	{
		INFO("same type"sv)
		
		matrix_t same{ mat };
		CHECK_SYMMETRIC_EQUAL(mat, same);
		if constexpr (is_floating_point<T>)
		{
			CHECK(matrix_t::approx_equal(mat, same));
			CHECK(mat.approx_equal(same));
			CHECK(muu::approx_equal(mat, same));
		}

		matrix_t different{ mat };
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				different(r, c)++;
		CHECK_SYMMETRIC_INEQUAL(mat, different);
		if constexpr (is_floating_point<T>)
		{
			CHECK_FALSE(matrix_t::approx_equal(mat, different));
			CHECK_FALSE(mat.approx_equal(different));
			CHECK_FALSE(muu::approx_equal(mat, different));
		}
	}

	if constexpr (!is_floating_point<T>)
	{
		INFO("different type"sv)

		using other_t = matrix<
			std::conditional_t<std::is_same_v<T, long>, int, long>,
			Rows,
			Columns
		>;

		other_t same{ mat };
		CHECK_SYMMETRIC_EQUAL(mat, same);

		other_t different{ mat };
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				different(r, c)++;
		CHECK_SYMMETRIC_INEQUAL(mat, different);
	}
}
