// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"
#include "../include/muu/matrix.h"

template <typename T>
inline constexpr bool invoke_trait_tests = false;

template <typename T, size_t Rows, size_t Columns>
inline constexpr void trait_tests(std::string_view /*scalar_typename*/) noexcept
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
inline void construction_test_from_scalars() noexcept
{
	if constexpr (Rows * Columns >= NUM)
	{
		INFO("constructing from "sv << NUM << " scalars"sv)
		using matrix_t = matrix<T, Rows, Columns>;

		const auto vals = random_array<T, NUM>();
		matrix_t mat = std::apply([](auto&& ... v) noexcept { return matrix_t{ std::forward<decltype(v)>(v)... }; }, vals);

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

template <typename T, size_t Rows, size_t Columns>
inline void construction_tests(std::string_view scalar_typename) noexcept
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
	construction_test_from_scalars<T, Rows, Columns, 3>();
	construction_test_from_scalars<T, Rows, Columns, 4>();
	construction_test_from_scalars<T, Rows, Columns, 5>();
	construction_test_from_scalars<T, Rows, Columns, 7>();
	construction_test_from_scalars<T, Rows, Columns, 10>();
	construction_test_from_scalars<T, Rows, Columns, 12>();
	construction_test_from_scalars<T, Rows, Columns, 16>();
	construction_test_from_scalars<T, Rows, Columns, 20>();
	// no single-scalar test; it's the fill constructor.

}

template <typename T, size_t Rows, size_t Columns>
inline void accessor_tests(std::string_view scalar_typename) noexcept
{
	INFO("matrix<"sv << scalar_typename << ", "sv << Rows << ", "sv << Columns << ">"sv)
	using matrix_t = matrix<T, Rows, Columns>;

	const auto vals = random_array<T, Rows * Columns>();
	std::array<const T*, Rows> rows;
	for (size_t i = 0; i < Rows; i++)
		rows[i] = vals.data() + i * Columns;
	matrix_t mat = std::apply([](auto&& ... v) noexcept { return matrix_t{ v... }; }, vals);
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
