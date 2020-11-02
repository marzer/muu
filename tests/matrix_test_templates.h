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

template <typename T, size_t Rows, size_t Columns>
inline void construction_tests(std::string_view scalar_typename) noexcept
{
	INFO("matrix<"sv << scalar_typename << ", "sv << Rows << ", "sv << Columns << ">"sv)
	using matrix_t = matrix<T, Rows, Columns>;

	{
		INFO("fill constructor")

		const auto val = random<T>();
		matrix_t m{ val };
		for (size_t c = 0; c < Columns; c++)
			for (size_t r = 0; r < Rows; r++)
				CHECK(val == m.c[c][r]);
	}

	{
		INFO("copy constructor"sv)
		matrix_t m1;
		for (size_t c = 0; c < Columns; c++)
			for (size_t r = 0; r < Rows; r++)
				m1.c[c][r] = random<T>();
		matrix_t m2{ m1 };
		for (size_t c = 0; c < Columns; c++)
			for (size_t r = 0; r < Rows; r++)
				CHECK(m1.c[c][r] == m2.c[c][r]);
	}
}
