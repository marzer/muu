// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"
#include "../include/muu/matrix.h"

#define CHECK_MATRIX_APPROX_EQUAL(a, b)											\
	do																			\
	{																			\
		for (size_t r = 0; r < Rows; r++)										\
			for (size_t c = 0; c < Columns; c++)								\
				CHECK_APPROX_EQUAL(result(r, c), expected(r, c));				\
	} while (false)

#define CHECK_MATRIX_APPROX_EQUAL_EPS(a, b, eps)								\
	do																			\
	{																			\
		for (size_t r = 0; r < Rows; r++)										\
			for (size_t c = 0; c < Columns; c++)								\
				CHECK_APPROX_EQUAL_EPS(result(r, c), expected(r, c), eps);		\
	} while (false)

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

		const auto vals = random_array<T, NUM>(1, 5);

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
				smaller(r, c) = random<T>(1, 5);

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
				larger(r, c) = random<T>(1, 5);

		auto mat = matrix_t{ larger };
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				CHECK(mat(r, c) == larger(r, c));
	}
}

template <typename T, size_t Rows, size_t Columns>
struct blittable
{
	vector<T, Rows> m[Columns];
};

namespace muu
{
	template <typename T, size_t Rows, size_t Columns>
	inline constexpr bool can_blit<blittable<T, Rows, Columns>, matrix<T, Rows, Columns>> = true;
}

template <typename T, size_t Rows, size_t Columns>
inline void matrix_construction_tests(std::string_view scalar_typename) noexcept
{
	INFO("matrix<"sv << scalar_typename << ", "sv << Rows << ", "sv << Columns << ">"sv)
	using matrix_t = matrix<T, Rows, Columns>;

	{
		INFO("fill constructor")

		const auto val = random<T>(1, 5);
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
				m1(r, c) = random<T>(1, 5);

		matrix_t m2{ m1 };
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				CHECK(m1(r, c) == m2(r, c));
	}

	{
		INFO("blitting constructor"sv)
		blittable<T, Rows, Columns> m1;
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				m1.m[c][r] = random<T>(1, 5);

		matrix_t m2{ m1 };
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				CHECK(m1.m[c][r] == m2(r, c));
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
				other(r, c) = random<other_type>(0.5, 5);

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

	const auto vals = random_array<T, Rows * Columns>(1, 5);
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
			mat(r, c) = random<T>(1, 5);
		
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

template <typename T, size_t Rows, size_t Columns>
inline void matrix_zero_tests(std::string_view scalar_typename) noexcept
{
	INFO("matrix<"sv << scalar_typename << ", "sv << Rows << ", "sv << Columns << ">"sv)
	using matrix_t = matrix<T, Rows, Columns>;

	{
		INFO("all zeroes"sv)

		matrix_t mat{ T{} };

		CHECK(mat.zero());
		if constexpr (is_floating_point<T>)
		{
			CHECK(matrix_t::approx_zero(mat));
			CHECK(mat.approx_zero());
			CHECK(muu::approx_zero(mat));
		}
	}

	{
		INFO("no zeroes"sv)

		matrix_t mat;
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				mat(r, c) = random<T>(1, 10);

		CHECK_FALSE(mat.zero());
		if constexpr (is_floating_point<T>)
		{
			CHECK_FALSE(matrix_t::approx_zero(mat));
			CHECK_FALSE(mat.approx_zero());
			CHECK_FALSE(muu::approx_zero(mat));
		}
	}

	if constexpr (Rows * Columns > 1)
	{
		INFO("some zeroes"sv)

		matrix_t mat { T{1} };
		for (size_t r = 0, i = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++, i++)
				if ((i % 2_sz))
					mat(r, c) = T{};

		CHECK_FALSE(mat.zero());
		if constexpr (is_floating_point<T>)
		{
			CHECK_FALSE(matrix_t::approx_zero(mat));
			CHECK_FALSE(mat.approx_zero());
			CHECK_FALSE(muu::approx_zero(mat));
		}
	}

	{
		INFO("one zero"sv)
		for (size_t r = 0; r < Rows; r++)
		{
			for (size_t c = 0; c < Columns; c++)
			{
				matrix_t mat{ T{} };
				mat(r, c) = random<T>(1, 10);

				CHECK_FALSE(mat.zero());
				if constexpr (is_floating_point<T>)
				{
					CHECK_FALSE(matrix_t::approx_zero(mat));
					CHECK_FALSE(mat.approx_zero());
					CHECK_FALSE(muu::approx_zero(mat));
				}
			}
			}
	}
}

template <typename T, size_t Rows, size_t Columns>
inline void matrix_infinity_or_nan_tests(std::string_view scalar_typename) noexcept
{
	INFO("matrix<"sv << scalar_typename << ", "sv << Rows << ", "sv << Columns << ">"sv)
	using matrix_t = matrix<T, Rows, Columns>;

	matrix_t mat;
	for (size_t r = 0; r < Rows; r++)
		for (size_t c = 0; c < Columns; c++)
			mat(r, c) = random<T>(1, 5);

	{
		INFO("all finite"sv)
		CHECK_FALSE(mat.infinity_or_nan());
		CHECK_FALSE(matrix_t::infinity_or_nan(mat));
		CHECK_FALSE(muu::infinity_or_nan(mat));
	}


	if constexpr (is_floating_point<T>)
	{
		{
			INFO("contains one NaN"sv)
			for (size_t r = 0; r < Rows; r++)
			{
				for (size_t c = 0; c < Columns; c++)
				{
					matrix_t mat2{ mat };
					mat2(r, c) = make_nan<T>();
					CHECK(mat2.infinity_or_nan());
					CHECK(matrix_t::infinity_or_nan(mat2));
					CHECK(muu::infinity_or_nan(mat2));
				}
			}
		}

		{
			INFO("contains one infinity"sv)
			for (size_t r = 0; r < Rows; r++)
			{
				for (size_t c = 0; c < Columns; c++)
				{
					matrix_t mat2{ mat };
					mat2(r, c) = make_infinity<T>();
					CHECK(mat2.infinity_or_nan());
					CHECK(matrix_t::infinity_or_nan(mat2));
					CHECK(muu::infinity_or_nan(mat2));
				}
			}
		}
	}
}

template <typename T, size_t Rows, size_t Columns>
inline void matrix_addition_tests(std::string_view scalar_typename) noexcept
{
	INFO("matrix<"sv << scalar_typename << ", "sv << Rows << ", "sv << Columns << ">"sv)
	using matrix_t = matrix<T, Rows, Columns>;

	matrix_t mat1;
	matrix_t mat2;
	for (size_t r = 0; r < Rows; r++)
	{
		for (size_t c = 0; c < Columns; c++)
		{
			mat1(r, c) = random<T>(0, 5);
			mat2(r, c) = random<T>(1, 5);
		}
	}

	{
		INFO("matrix + matrix"sv)

		matrix_t result = mat1 + mat2;
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				CHECK_APPROX_EQUAL(static_cast<T>(mat1(r, c) + mat2(r, c)), result(r, c));
	}

	{
		INFO("matrix += matrix"sv)

		matrix_t result { mat1 };
		result += mat2;
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				CHECK_APPROX_EQUAL(static_cast<T>(mat1(r, c) + mat2(r, c)), result(r, c));
	}
}

template <typename T, size_t Rows, size_t Columns>
inline void matrix_subtraction_tests(std::string_view scalar_typename) noexcept
{
	INFO("matrix<"sv << scalar_typename << ", "sv << Rows << ", "sv << Columns << ">"sv)
	using matrix_t = matrix<T, Rows, Columns>;

	matrix_t mat1;
	matrix_t mat2;
	for (size_t r = 0; r < Rows; r++)
	{
		for (size_t c = 0; c < Columns; c++)
		{
			if constexpr (is_signed<T>)
				mat1(r, c) = random<T>(0, 10);
			else
				mat1(r, c) = random<T>(11, 20);
			mat2(r, c) = random<T>(0, 10);
		}
	}

	{
		INFO("matrix - matrix"sv)

		matrix_t result = mat1 - mat2;
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				CHECK_APPROX_EQUAL(static_cast<T>(mat1(r, c) - mat2(r, c)), result(r, c));
	}

	{
		INFO("matrix -= matrix"sv)

		matrix_t result { mat1 };
		result -= mat2;
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				CHECK_APPROX_EQUAL(static_cast<T>(mat1(r, c) - mat2(r, c)), result(r, c));
	}
}

template <typename T, size_t Rows, size_t Columns>
inline void matrix_multiplication_tests(std::string_view scalar_typename) noexcept
{
	INFO("matrix<"sv << scalar_typename << ", "sv << Rows << ", "sv << Columns << ">"sv)
	using matrix_t = matrix<T, Rows, Columns>;

	const auto min_val = T{ 1 }; //static_cast<T>(0.5);
	const auto max_val = T{ 5 };//is_floating_point<T> ? T{ 1 } : T{ 5 };

	matrix_t mat1;
	for (size_t r = 0; r < Rows; r++)
		for (size_t c = 0; c < Columns; c++)
			mat1(r, c) = random<T>(min_val, max_val);

	{
		INFO("matrix * scalar"sv)

		const auto val = random<T>(min_val, max_val);
		const auto result = mat1 * val;
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				CHECK_APPROX_EQUAL(static_cast<T>(mat1(r, c) * val), result(r, c));
	}

	{
		INFO("scalar * matrix"sv)

		const auto val = random<T>(min_val, max_val);
		const auto result = val * mat1;
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				CHECK_APPROX_EQUAL(static_cast<T>(mat1(r, c) * val), result(r, c));
	}

	{
		INFO("matrix *= scalar"sv)

		const auto val = random<T>(min_val, max_val);
		auto result = mat1;
		result *= val;
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				CHECK_APPROX_EQUAL(static_cast<T>(mat1(r, c) * val), result(r, c));
	}

	{
		INFO("matrix * column vector"sv)

		const auto col_vec = vector<T, Columns>{ random_array<T, Columns>(min_val, max_val) };
		const auto result = mat1 * col_vec;
		for (size_t r = 0; r < Rows; r++)
		{
			typename matrix_t::row_type lhs_row;
			for (size_t i = 0; i < Columns; i++)
				lhs_row[i] = mat1(r, i);
			CHECK_APPROX_EQUAL(static_cast<T>(lhs_row.dot(col_vec)), result[r]);
		}
	}

	{
		INFO("row vector * matrix"sv)
		const auto row_vec = vector<T, Rows>{ random_array<T, Rows>(min_val, max_val) };
		const auto result = row_vec * mat1;
		for (size_t c = 0; c < Columns; c++)
			CHECK_APPROX_EQUAL(static_cast<T>(mat1.m[c].dot(row_vec)), result[c]);
	}

	{
		INFO("matrix * matrix"sv)

		matrix<T, Columns, Rows> mat2;
		for (size_t r = 0; r < Columns; r++)
			for (size_t c = 0; c < Rows; c++)
				mat2(r, c) = random<T>(min_val, max_val);

		auto result = mat1 * mat2;
		for (size_t r = 0; r < Rows; r++)
		{
			for (size_t c = 0; c < Rows; c++) //rhs COLS, but here rhs COLS == lhs ROWS
			{
				typename matrix_t::row_type lhs_row;
				for (size_t i = 0; i < Columns; i++)
					lhs_row[i] = mat1(r, i);
				CHECK_APPROX_EQUAL(static_cast<T>(lhs_row.dot(mat2.m[c])), result(r, c));
			}
		}
	}

	if constexpr (Rows == 2 && Columns == 3 && (sizeof(T) > 1 || is_unsigned<T>))
	{
		INFO("matrix * matrix - case #1"sv)

		const auto lhs = matrix<T, 2, 3>{
			T{1},	T{2},	T{3},
			T{4},	T{5},	T{6}
		};

		const auto rhs = matrix<T, 3, 2>{
			T{7},	T{8},
			T{9},	T{10},
			T{11},	T{12},
		};

		const auto expected = matrix<T, 2, 2>{
			T{58},	T{64},
			T{139},	T{154}
		};

		CHECK_APPROX_EQUAL(lhs * rhs, expected);
	}

	if constexpr (Rows == 1 && Columns == 3)
	{
		INFO("matrix * matrix - case #2"sv)

		const auto lhs = matrix<T, 1, 3>{
			T{3},	T{4},	T{2}
		};

		const auto rhs = matrix<T, 3, 4>{
			T{13},	T{9},	T{7},	T{15},
			T{8},	T{7},	T{4},	T{6},
			T{6},	T{4},	T{0},	T{3},
		};

		const auto expected = matrix<T, 1, 4>{
			T{83},	T{63}, T{37},	T{75}
		};

		CHECK_APPROX_EQUAL(lhs * rhs, expected);
	}

	if constexpr (Rows == 1 && Columns == 3)
	{
		INFO("matrix * matrix - case #3"sv)

		const auto lhs = matrix<T, 1, 3>{
			T{1},	T{2},	T{3}
		};

		const auto rhs = matrix<T, 3, 1>{
			T{4},
			T{5},
			T{6}
		};

		const auto expected = matrix<T, 1, 1>{
			T{32}
		};

		CHECK_APPROX_EQUAL(lhs * rhs, expected);
	}

	if constexpr (Rows == 3 && Columns == 1)
	{
		INFO("matrix * matrix - case #4"sv)

		const auto lhs = matrix<T, 3, 1>{
			T{4},
			T{5},
			T{6}
		};

		const auto rhs = matrix<T, 1, 3>{
			T{1},	T{2},	T{3}
		};

		const auto expected = matrix<T, 3, 3>{
			T{4},	T{8},	T{12},
			T{5},	T{10},	T{15},
			T{6},	T{12},	T{18}
		};

		CHECK_APPROX_EQUAL(lhs * rhs, expected);
	}

	if constexpr (Rows == 2 && Columns == 2)
	{
		INFO("matrix * matrix - case #5"sv)

		const auto lhs = matrix<T, 2, 2>{
			T{1},	T{2},
			T{3},	T{4}
		};

		const auto rhs = matrix<T, 2, 2>{
			T{2},	T{0},
			T{1},	T{2}
		};

		const auto expected = matrix<T, 2, 2>{
			T{4},	T{4},
			T{10},	T{8}
		};

		CHECK_APPROX_EQUAL(lhs * rhs, expected);
	}

	if constexpr (Rows == 2 && Columns == 2)
	{
		INFO("matrix * matrix - case #6"sv)

		const auto lhs = matrix<T, 2, 2>{
			T{2},	T{0},
			T{1},	T{2}
		};

		const auto rhs = matrix<T, 2, 2>{
			T{1},	T{2},
			T{3},	T{4}
		};

		const auto expected = matrix<T, 2, 2>{
			T{2},	T{4},
			T{7},	T{10}
		};

		CHECK_APPROX_EQUAL(lhs * rhs, expected);
	}

	if constexpr (Rows == 3 && Columns == 3 && (sizeof(T) > 1 || is_unsigned<T>))
	{
		INFO("matrix * matrix - case #7"sv)

		const auto lhs = matrix<T, 3, 3>{
			T{10},	T{20},	T{10},
			T{4},	T{5},	T{6},
			T{2},	T{3},	T{5}
		};

		const auto rhs = matrix<T, 3, 3>{
			T{3},	T{2},	T{4},
			T{3},	T{3},	T{9},
			T{4},	T{4},	T{2}
		};

		const auto expected = matrix<T, 3, 3>{
			T{130},	T{120},	T{240},
			T{51},	T{47},	T{73},
			T{35},	T{33},	T{45}
		};

		CHECK_APPROX_EQUAL(lhs* rhs, expected);
	}

	if constexpr (Rows == 4 && Columns == 4 && sizeof(T) > 1)
	{
		INFO("matrix * matrix - case #8"sv)

		const auto lhs = matrix<T, 4, 4>{
			T{5},	T{7},	T{9},	T{10},
			T{2},	T{3},	T{3},	T{8},
			T{8},	T{10},	T{2},	T{3},
			T{3},	T{3},	T{4},	T{8}
		};

		const auto rhs = matrix<T, 4, 4>{
			T{3},	T{10},	T{12},	T{18},
			T{12},	T{1},	T{4},	T{9},
			T{9},	T{10},	T{12},	T{2},
			T{3},	T{12},	T{4},	T{10}
		};

		const auto expected = matrix<T, 4, 4>{
			T{210},	T{267},	T{236},	T{271},
			T{93},	T{149},	T{104},	T{149},
			T{171},	T{146},	T{172},	T{268},
			T{105},	T{169},	T{128},	T{169}
		};

		CHECK_APPROX_EQUAL(lhs * rhs, expected);
	}
}

template <typename T, size_t Rows, size_t Columns>
inline void matrix_division_tests(std::string_view scalar_typename) noexcept
{
	INFO("matrix<"sv << scalar_typename << ", "sv << Rows << ", "sv << Columns << ">"sv)
	using matrix_t = matrix<T, Rows, Columns>;

	const auto scalar = static_cast<T>(2.4);
	matrix_t mat;
	for (size_t r = 0; r < Rows; r++)
		for (size_t c = 0; c < Columns; c++)
			mat(r, c) = random<T>(2, 5);

	using promoted = impl::promote_if_small_float<T>;

	{
		INFO("matrix / scalar"sv)

		const auto result = mat / scalar;
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				CHECK_APPROX_EQUAL(static_cast<T>(static_cast<promoted>(mat(r, c)) / static_cast<promoted>(scalar)), result(r, c));
	}

	{
		INFO("matrix /= scalar"sv)

		auto result = mat;
		result /= scalar;
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				CHECK_APPROX_EQUAL(static_cast<T>(static_cast<promoted>(mat(r, c)) / static_cast<promoted>(scalar)), result(r, c));
	}
}

template <typename T, size_t Rows, size_t Columns>
inline void matrix_transpose_tests(std::string_view scalar_typename) noexcept
{
	INFO("matrix<"sv << scalar_typename << ", "sv << Rows << ", "sv << Columns << ">"sv)
	using matrix_t = matrix<T, Rows, Columns>;

	matrix_t mat;
	for (size_t r = 0; r < Rows; r++)
		for (size_t c = 0; c < Columns; c++)
			mat(r, c) = random<T>(0, 10);

	{
		INFO("transpose(matrix)"sv)

		matrix<T, Columns, Rows> transposed = matrix_t::transpose(mat);
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				CHECK(transposed(c, r) == mat(r, c));
	}

	{
		INFO("muu::transpose(matrix)"sv)

		matrix<T, Columns, Rows> transposed = muu::transpose(mat);
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				CHECK(transposed(c, r) == mat(r, c));
	}

	if constexpr (Rows == Columns)
	{
		INFO("matrix.transpose()"sv)
		matrix_t transposed{ mat };
		transposed.transpose();
		for (size_t r = 0; r < Rows; r++)
			for (size_t c = 0; c < Columns; c++)
				CHECK(transposed(c, r) == mat(r, c));
	}
}

template <typename T, size_t Rows, size_t Columns>
inline void matrix_determinant_tests(std::string_view scalar_typename) noexcept
{
	INFO("matrix<"sv << scalar_typename << ", "sv << Rows << ", "sv << Columns << ">"sv)
	using matrix_t = matrix<T, Rows, Columns>;
	using determinant_t = typename matrix_t::determinant_type;

	#define CHECK_DETERMINANT(expected)												\
		CHECK_APPROX_EQUAL(matrix_t::determinant(mat), determinant_t{ expected });	\
		CHECK_APPROX_EQUAL(mat.determinant(), determinant_t{ expected });			\
		CHECK_APPROX_EQUAL(muu::determinant(mat), determinant_t{ expected })

	if constexpr (Rows == 2 && Columns == 2)
	{
		matrix_t mat{
			T{3},	T{8},
			T{4},	T{6}
		};
		CHECK_DETERMINANT(-14);

		mat = matrix_t{
			T{2},	T{5},
			T{3},	T{8}
		};
		CHECK_DETERMINANT(1);

		if constexpr (is_signed<T>)
		{
			mat = matrix_t{
				T{-4},	T{7},
				T{-2},	T{9}
			};
			CHECK_DETERMINANT(-22);
		}
	}

	if constexpr (Rows == 3 && Columns == 3)
	{
		matrix_t mat{
			T{1},	T{2},	T{3},
			T{4},	T{5},	T{6},
			T{7},	T{8},	T{9}
		};
		CHECK_DETERMINANT(0);

		mat = matrix_t{
			T{3},	T{4},	T{4},
			T{5},	T{9},	T{3},
			T{2},	T{1},	T{5}
		};
		CHECK_DETERMINANT(-2);

		if constexpr (is_signed<T>)
		{
			mat = matrix_t{
				T{-2},	T{3},	T{-1},
				T{5},	T{-1},	T{4},
				T{4},	T{-8},	T{2}
			};
			CHECK_DETERMINANT(-6);

			mat = matrix_t{
				T{2},	T{-3},	T{1},
				T{2},	T{0},	T{-1},
				T{1},	T{4},	T{5}
			};
			CHECK_DETERMINANT(49);
		}
	}

	if constexpr (Rows == 4 && Columns == 4)
	{
		matrix_t mat{
			T{4},	T{7},	T{2},	T{3},
			T{1},	T{3},	T{1},	T{2},
			T{2},	T{5},	T{3},	T{4},
			T{1},	T{4},	T{2},	T{3}
		};
		CHECK_DETERMINANT(-3);

		mat = matrix_t{
			T{2},	T{1},	T{3},	T{4},
			T{1},	T{3},	T{4},	T{2},
			T{3},	T{4},	T{2},	T{1},
			T{4},	T{2},	T{1},	T{3}
		};
		CHECK_DETERMINANT(0);

		if constexpr (is_signed<T>)
		{
			mat = matrix_t{
				T{2},	T{5},	T{3},	T{5},
				T{4},	T{6},	T{6},	T{3},
				T{11},	T{3},	T{2},	T{-2},
				T{4},	T{-7},	T{9},	T{3}
			};
			CHECK_DETERMINANT(2960);
		}
	}
}

template <typename T, size_t Rows, size_t Columns>
inline void matrix_invert_tests(std::string_view scalar_typename) noexcept
{
	INFO("matrix<"sv << scalar_typename << ", "sv << Rows << ", "sv << Columns << ">"sv)
	using matrix_t = matrix<T, Rows, Columns>;
	using inverse_t = typename matrix_t::inverse_type;
	using inverse_scalar = typename inverse_t::scalar_type;

	[[maybe_unused]]
	static constexpr auto i = [](auto scalar) noexcept
	{
		return static_cast<inverse_scalar>(scalar);
	};

	using eps_common_type = impl::highest_ranked<inverse_scalar, double>;

	[[maybe_unused]]
	const inverse_scalar eps = static_cast<inverse_scalar>(muu::max(
		static_cast<eps_common_type>(constants<inverse_scalar>::approx_equal_epsilon),
		static_cast<eps_common_type>(constants<double>::approx_equal_epsilon)
	));

	#define CHECK_INVERSE(mat, expected)						\
		do														\
		{														\
			auto result = matrix_t::invert(mat);				\
			CHECK_MATRIX_APPROX_EQUAL_EPS(mat, expected, eps);	\
																\
			result = muu::invert(mat);							\
			CHECK_MATRIX_APPROX_EQUAL_EPS(mat, expected, eps);	\
		} while (false)

	if constexpr (Rows == 2 && Columns == Rows)
	{
		const auto mat = matrix_t{
			T{4},	T{7},
			T{2},	T{6}
		};

		const auto expected = inverse_t{
			i(0.6),		i(-0.7),
			i(-0.2),	i(0.4)
		};

		CHECK_INVERSE(mat, expected);
	}

	if constexpr (Rows == 3 && Columns == Rows && is_signed<T>)
	{
		const auto mat = matrix_t{
			T{3},	T{0},	T{2},
			T{2},	T{0},	T{-2},
			T{0},	T{1},	T{1}
		};

		const auto expected = inverse_t{
			i(0.2),		i(0.2),		i(0),
			i(-0.2),	i(0.3),		i(1),
			i(0.2),		i(-0.3),	i(0)
		};

		CHECK_INVERSE(mat, expected);
	}

	if constexpr (Rows == 4 && Columns == Rows)
	{
		const auto mat = matrix_t{
			T{3},	T{4},	T{3},	T{1},
			T{1},	T{3},	T{5},	T{4},
			T{1},	T{1},	T{2},	T{4},
			T{1},	T{1},	T{1},	T{1}
		};

		const auto expected = inverse_t{
			i(-1),		i(0),		i(-1),		i(5),
			i(1.8),		i(-0.4),	i(1.6),		i(-6.6),
			i(-1.2),	i(0.6),		i(-1.4),	i(4.4),
			i(0.4),		i(-0.2),	i(0.8),		i(-1.8)
		};

		CHECK_INVERSE(mat, expected);
	}
}

template <typename T, size_t Rows, size_t Columns>
inline void matrix_orthonormalize_tests(std::string_view scalar_typename) noexcept
{
	INFO("matrix<"sv << scalar_typename << ", "sv << Rows << ", "sv << Columns << ">"sv)
	using matrix_t = matrix<T, Rows, Columns>;
	using vec3 = vector<T, 3>;
	using column_t = vector<T, Rows>;

	#define CHECK_ORTHONORMALIZE(mat, expected)				\
		do													\
		{													\
			auto result = matrix_t::orthonormalize(mat);	\
			CHECK_MATRIX_APPROX_EQUAL(mat, expected);		\
															\
			result = muu::orthonormalize(mat);				\
			CHECK_MATRIX_APPROX_EQUAL(mat, expected);		\
		} while (false)

	if constexpr (Rows >= 3 && Rows <= 4
		&& Columns >= 3 && Columns <= 4
		&& is_floating_point<T>)
	{
		matrix_t mat{ T{} };
		mat.m[0] = column_t{ vec3::constants::x_axis * random<T>(2, 5) };
		mat.m[1] = column_t{ vec3::constants::y_axis * random<T>(2, 5) };
		mat.m[2] = column_t{ vec3::constants::z_axis * random<T>(2, 5) };

		matrix_t expected{ T{} };
		expected.m[0] = column_t{ vec3::constants::x_axis };
		expected.m[1] = column_t{ vec3::constants::y_axis };
		expected.m[2] = column_t{ vec3::constants::z_axis };

		CHECK_ORTHONORMALIZE(mat, expected);
	}
}

