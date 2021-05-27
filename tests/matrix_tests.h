// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"
#include "batching.h"
#include "../include/muu/matrix.h"

#define CHECK_MATRIX_APPROX_EQUAL(a, b)                                                                                \
	do                                                                                                                 \
	{                                                                                                                  \
		for (size_t r = 0; r < decltype(a)::rows; r++)                                                                 \
			for (size_t c = 0; c < decltype(a)::columns; c++)                                                          \
				CHECK_APPROX_EQUAL(a(r, c), b(r, c));                                                                  \
	}                                                                                                                  \
	while (false)

#define CHECK_MATRIX_APPROX_EQUAL_EPS(a, b, eps)                                                                       \
	do                                                                                                                 \
	{                                                                                                                  \
		for (size_t r = 0; r < decltype(a)::rows; r++)                                                                 \
			for (size_t c = 0; c < decltype(a)::columns; c++)                                                          \
				CHECK_APPROX_EQUAL_EPS(a(r, c), b(r, c), eps);                                                         \
	}                                                                                                                  \
	while (false)

namespace
{
	template <typename T, size_t Rows, size_t Columns, size_t NUM>
	static void matrix_construction_test_from_scalars() noexcept
	{
		TEST_INFO("constructing from "sv << NUM << " scalars"sv);
		using matrix_t = matrix<T, Rows, Columns>;

		const auto vals = random_array<T, NUM>(1, 5);

		// std::apply is super taxing for the compiler to instantiate on some implementations
		// I'm simulating it for <= 16 values
		matrix_t mat;
		if constexpr (NUM == 1)
			mat = matrix_t{ vals[0] };
		if constexpr (NUM == 2)
			mat = matrix_t{ vals[0], vals[1] };
		if constexpr (NUM == 3)
			mat = matrix_t{ vals[0], vals[1], vals[2] };
		if constexpr (NUM == 4)
			mat = matrix_t{ vals[0], vals[1], vals[2], vals[3] };
		if constexpr (NUM == 5)
			mat = matrix_t{ vals[0], vals[1], vals[2], vals[3], vals[4] };
		if constexpr (NUM == 6)
			mat = matrix_t{ vals[0], vals[1], vals[2], vals[3], vals[4], vals[5] };
		if constexpr (NUM == 7)
			mat = matrix_t{ vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6] };
		if constexpr (NUM == 8)
			mat = matrix_t{ vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6], vals[7] };
		if constexpr (NUM == 9)
			mat = matrix_t{ vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6], vals[7], vals[8] };
		if constexpr (NUM == 10)
			mat = matrix_t{ vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6], vals[7], vals[8], vals[9] };
		if constexpr (NUM == 11)
			mat = matrix_t{ vals[0], vals[1], vals[2], vals[3], vals[4], vals[5],
							vals[6], vals[7], vals[8], vals[9], vals[10] };
		if constexpr (NUM == 12)
			mat = matrix_t{ vals[0], vals[1], vals[2], vals[3], vals[4],  vals[5],
							vals[6], vals[7], vals[8], vals[9], vals[10], vals[11] };
		if constexpr (NUM == 13)
			mat = matrix_t{ vals[0], vals[1], vals[2], vals[3],	 vals[4],  vals[5], vals[6],
							vals[7], vals[8], vals[9], vals[10], vals[11], vals[12] };
		if constexpr (NUM == 14)
			mat = matrix_t{ vals[0], vals[1], vals[2], vals[3],	 vals[4],  vals[5],	 vals[6],
							vals[7], vals[8], vals[9], vals[10], vals[11], vals[12], vals[13] };
		if constexpr (NUM == 15)
			mat = matrix_t{ vals[0], vals[1], vals[2],	vals[3],  vals[4],	vals[5],  vals[6], vals[7],
							vals[8], vals[9], vals[10], vals[11], vals[12], vals[13], vals[14] };
		if constexpr (NUM == 16)
			mat = matrix_t{
				vals[0], vals[1], vals[2],	vals[3],  vals[4],	vals[5],  vals[6],	vals[7],
				vals[8], vals[9], vals[10], vals[11], vals[12], vals[13], vals[14], vals[15],
			};
		if constexpr (NUM > 16)
			mat = std::apply([](auto&&... s) noexcept { return matrix_t{ s... }; }, vals);

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

	template <typename T, size_t Rows, size_t Columns, size_t R, size_t C>
	static void matrix_construction_test_from_smaller_matrix() noexcept
	{
		if constexpr (Rows >= R && Columns >= C && (Rows > R || Columns > C))
		{
			TEST_INFO("constructing from a smaller matrix with "sv << R << " x "sv << C << " elements"sv);
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
	static void matrix_construction_test_from_larger_matrix() noexcept
	{
		if constexpr (Rows <= R && Columns <= C && (Rows < R || Columns < C))
		{
			TEST_INFO("constructing from a larger matrix with "sv << R << " x "sv << C << " elements"sv);
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

	// clang-format off

	template <typename... T>
	using all_matrices = type_list<
		matrix<T, 1, 1>..., matrix<T, 1, 2>..., matrix<T, 1, 3>..., matrix<T, 1, 4>..., matrix<T, 1, 5>...,
		matrix<T, 2, 1>..., matrix<T, 2, 2>..., matrix<T, 2, 3>..., matrix<T, 2, 4>..., matrix<T, 2, 5>...,
		matrix<T, 3, 1>..., matrix<T, 3, 2>..., matrix<T, 3, 3>..., matrix<T, 3, 4>..., matrix<T, 3, 5>...,
		matrix<T, 4, 1>..., matrix<T, 4, 2>..., matrix<T, 4, 3>..., matrix<T, 4, 4>..., matrix<T, 4, 5>...,
		matrix<T, 5, 1>..., matrix<T, 5, 2>..., matrix<T, 5, 3>..., matrix<T, 5, 4>..., matrix<T, 5, 5>...
	>;

	// clang-format on

	template <typename... T>
	using common_matrices = type_list<matrix<T, 2, 2>..., matrix<T, 3, 3>..., matrix<T, 3, 4>..., matrix<T, 4, 4>...>;

	template <typename... T>
	using square_matrices = type_list<matrix<T, 2, 2>..., matrix<T, 3, 3>..., matrix<T, 4, 4>...>;

	template <typename... T>
	using rotation_matrices = type_list<matrix<T, 3, 3>..., matrix<T, 3, 4>..., matrix<T, 4, 4>...>;
}

namespace muu
{
	template <typename T, size_t Rows, size_t Columns>
	inline constexpr bool allow_implicit_bit_cast<blittable<T, Rows, Columns>, matrix<T, Rows, Columns>> = true;
}

BATCHED_TEST_CASE("matrix constructors", common_matrices<ALL_ARITHMETIC>)
{
	using matrix_t					   = TestType;
	using T							   = typename matrix_t::scalar_type;
	static constexpr auto scalar_count = matrix_t::rows * matrix_t::columns;
	TEST_INFO("matrix<"sv << nameof<T> << ", "sv << matrix_t::rows << ", "sv << matrix_t::columns << ">"sv);

	static_assert(sizeof(matrix_t) == sizeof(T) * matrix_t::rows * matrix_t::columns);
	static_assert(sizeof(matrix_t[5]) == sizeof(T) * matrix_t::rows * matrix_t::columns * 5);
	static_assert(std::is_standard_layout_v<matrix_t>);
	static_assert(std::is_trivially_constructible_v<matrix_t>);
	static_assert(std::is_trivially_copy_constructible_v<matrix_t>);
	static_assert(std::is_trivially_copy_assignable_v<matrix_t>);
	static_assert(std::is_trivially_move_constructible_v<matrix_t>);
	static_assert(std::is_trivially_move_assignable_v<matrix_t>);
	static_assert(std::is_trivially_destructible_v<matrix_t>);
	static_assert(std::is_nothrow_constructible_v<matrix_t, T>);

#if MUU_HAS_VECTORCALL
	static_assert(
		impl::is_hva<
			matrix_t> == ((matrix_t::rows * matrix_t::columns) <= 4 && is_same_as_any<T, float, double, long double>));
#endif

	BATCHED_SECTION("zero-initialization")
	{
		const auto m = matrix_t{};
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				CHECK(m(r, c) == T{});
	}

	BATCHED_SECTION("fill constructor")
	{
		const auto val = random<T>(1, 5);
		matrix_t m{ val };
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				CHECK(m(r, c) == val);
	}

	BATCHED_SECTION("copy constructor")
	{
		matrix_t m1;
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				m1(r, c) = random<T>(1, 5);

		matrix_t m2{ m1 };
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				CHECK(m1(r, c) == m2(r, c));
	}

	BATCHED_SECTION("blitting constructor")
	{
		blittable<T, matrix_t::rows, matrix_t::columns> m1;
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				m1.m[c][r] = random<T>(1, 5);

		matrix_t m2{ m1 };
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				CHECK(m1.m[c][r] == m2(r, c));
	}

	BATCHED_SECTION("scalar constructor")
	{
		if constexpr (scalar_count > 3_sz)
			matrix_construction_test_from_scalars<T, matrix_t::rows, matrix_t::columns, 3>();
		matrix_construction_test_from_scalars<T, matrix_t::rows, matrix_t::columns, scalar_count>();
	}

	BATCHED_SECTION("coercing constructor")
	{
		using other_type = std::
			conditional_t<is_same_as_any<T, signed int, unsigned int>, float, set_signed<signed int, is_signed<T>>>;

		matrix<other_type, matrix_t::rows, matrix_t::columns> other;
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				other(r, c) = random<other_type>(0.5, 5);

		matrix_t coerced{ other };
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				CHECK(coerced(r, c) == static_cast<T>(other(r, c)));
	}

	BATCHED_SECTION("enlarging constructor")
	{
		muu::for_product<3_sz, 3_sz>(
			[](auto r, auto c) noexcept
			{
				static constexpr auto rows = decltype(r)::value + 1_sz;
				static constexpr auto cols = decltype(c)::value + 1_sz;
				if constexpr (matrix_t::rows >= rows && matrix_t::columns >= cols
							  && (matrix_t::rows > rows || matrix_t::columns > cols))
				{
					matrix_construction_test_from_smaller_matrix<T, matrix_t::rows, matrix_t::columns, rows, cols>();
				}
			});
	}

	BATCHED_SECTION("truncating constructor")
	{
		muu::for_product<3_sz, 3_sz>(
			[](auto r, auto c) noexcept
			{
				static constexpr auto rows = (decltype(r)::value + 1_sz) * 2_sz;
				static constexpr auto cols = (decltype(c)::value + 1_sz) * 2_sz;
				if constexpr (matrix_t::rows <= rows && matrix_t::columns <= cols
							  && (matrix_t::rows < rows || matrix_t::columns < cols))
				{
					matrix_construction_test_from_larger_matrix<T, matrix_t::rows, matrix_t::columns, rows, cols>();
				}
			});
	}
}

BATCHED_TEST_CASE("matrix accessors", common_matrices<ALL_ARITHMETIC>)
{
	using matrix_t = TestType;
	using T		   = typename matrix_t::scalar_type;
	TEST_INFO("matrix<"sv << nameof<T> << ", "sv << matrix_t::rows << ", "sv << matrix_t::columns << ">"sv);

	const auto vals = random_array<T, matrix_t::rows * matrix_t::columns>(1, 5);
	std::array<const T*, matrix_t::rows> rows;
	for (size_t i = 0; i < matrix_t::rows; i++)
		rows[i] = vals.data() + i * matrix_t::columns;
	matrix_t mat;
	for (size_t r = 0; r < matrix_t::rows; r++)
		for (size_t c = 0; c < matrix_t::columns; c++)
			mat.m[c][r] = rows[r][c];
	const matrix_t& mat_const = mat;

	BATCHED_SECTION("operator()")
	{
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				CHECK(rows[r][c] == mat(r, c));
	}

	BATCHED_SECTION("get()")
	{
		if constexpr (matrix_t::rows >= 1 && matrix_t::columns >= 1)
			CHECK(rows[0][0] == (mat.template get<0, 0>()));
		if constexpr (matrix_t::rows >= 1 && matrix_t::columns >= 2)
			CHECK(rows[0][1] == (mat.template get<0, 1>()));
		if constexpr (matrix_t::rows >= 1 && matrix_t::columns >= 3)
			CHECK(rows[0][2] == (mat.template get<0, 2>()));
		if constexpr (matrix_t::rows >= 2 && matrix_t::columns >= 1)
			CHECK(rows[1][0] == (mat.template get<1, 0>()));
		if constexpr (matrix_t::rows >= 2 && matrix_t::columns >= 2)
			CHECK(rows[1][1] == (mat.template get<1, 1>()));
		if constexpr (matrix_t::rows >= 2 && matrix_t::columns >= 3)
			CHECK(rows[1][2] == (mat.template get<1, 2>()));
		if constexpr (matrix_t::rows >= 3 && matrix_t::columns >= 1)
			CHECK(rows[2][0] == (mat.template get<2, 0>()));
		if constexpr (matrix_t::rows >= 3 && matrix_t::columns >= 2)
			CHECK(rows[2][1] == (mat.template get<2, 1>()));
		if constexpr (matrix_t::rows >= 3 && matrix_t::columns >= 3)
			CHECK(rows[2][2] == (mat.template get<2, 2>()));
	}

	BATCHED_SECTION("operator() (const)")
	{
		for (size_t r = 0; r < matrix_t::rows; r++)
		{
			for (size_t c = 0; c < matrix_t::columns; c++)
			{
				CHECK(rows[r][c] == mat_const(r, c));
				CHECK(&mat(r, c) == &mat_const(r, c));
			}
		}
	}

	BATCHED_SECTION("get() (const)")
	{
		if constexpr (matrix_t::rows >= 1 && matrix_t::columns >= 1)
			CHECK(rows[0][0] == (mat_const.template get<0, 0>()));
		if constexpr (matrix_t::rows >= 1 && matrix_t::columns >= 2)
			CHECK(rows[0][1] == (mat_const.template get<0, 1>()));
		if constexpr (matrix_t::rows >= 1 && matrix_t::columns >= 3)
			CHECK(rows[0][2] == (mat_const.template get<0, 2>()));
		if constexpr (matrix_t::rows >= 2 && matrix_t::columns >= 1)
			CHECK(rows[1][0] == (mat_const.template get<1, 0>()));
		if constexpr (matrix_t::rows >= 2 && matrix_t::columns >= 2)
			CHECK(rows[1][1] == (mat_const.template get<1, 1>()));
		if constexpr (matrix_t::rows >= 2 && matrix_t::columns >= 3)
			CHECK(rows[1][2] == (mat_const.template get<1, 2>()));
		if constexpr (matrix_t::rows >= 3 && matrix_t::columns >= 1)
			CHECK(rows[2][0] == (mat_const.template get<2, 0>()));
		if constexpr (matrix_t::rows >= 3 && matrix_t::columns >= 2)
			CHECK(rows[2][1] == (mat_const.template get<2, 1>()));
		if constexpr (matrix_t::rows >= 3 && matrix_t::columns >= 3)
			CHECK(rows[2][2] == (mat_const.template get<2, 2>()));
	}
}

BATCHED_TEST_CASE("matrix equality", common_matrices<ALL_ARITHMETIC>)
{
	using matrix_t = TestType;
	using T		   = typename matrix_t::scalar_type;
	TEST_INFO("matrix<"sv << nameof<T> << ", "sv << matrix_t::rows << ", "sv << matrix_t::columns << ">"sv);

	matrix_t mat;
	for (size_t r = 0; r < matrix_t::rows; r++)
		for (size_t c = 0; c < matrix_t::columns; c++)
			mat(r, c) = random<T>(1, 5);

	BATCHED_SECTION("same type")
	{
		matrix_t same{ mat };
		CHECK_SYMMETRIC_EQUAL(mat, same);
		if constexpr (is_floating_point<T>)
		{
			CHECK(matrix_t::approx_equal(mat, same));
			CHECK(mat.approx_equal(same));
			CHECK(muu::approx_equal(mat, same));
		}

		matrix_t different{ mat };
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
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
		BATCHED_SECTION("different type")
		{
			using other_t =
				matrix<std::conditional_t<std::is_same_v<T, long>, int, long>, matrix_t::rows, matrix_t::columns>;

			other_t same{ mat };
			CHECK_SYMMETRIC_EQUAL(mat, same);

			other_t different{ mat };
			for (size_t r = 0; r < matrix_t::rows; r++)
				for (size_t c = 0; c < matrix_t::columns; c++)
					different(r, c)++;
			CHECK_SYMMETRIC_INEQUAL(mat, different);
		}
	}
}

BATCHED_TEST_CASE("matrix zero", common_matrices<ALL_ARITHMETIC>)
{
	using matrix_t = TestType;
	using T		   = typename matrix_t::scalar_type;
	TEST_INFO("matrix<"sv << nameof<T> << ", "sv << matrix_t::rows << ", "sv << matrix_t::columns << ">"sv);

	BATCHED_SECTION("all zeroes")
	{
		matrix_t mat{ T{} };

		CHECK(mat.zero());
		if constexpr (is_floating_point<T>)
		{
			CHECK(matrix_t::approx_zero(mat));
			CHECK(mat.approx_zero());
			CHECK(muu::approx_zero(mat));
		}
	}

	BATCHED_SECTION("no zeroes")
	{
		matrix_t mat;
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				mat(r, c) = random<T>(1, 10);

		CHECK_FALSE(mat.zero());
		if constexpr (is_floating_point<T>)
		{
			CHECK_FALSE(matrix_t::approx_zero(mat));
			CHECK_FALSE(mat.approx_zero());
			CHECK_FALSE(muu::approx_zero(mat));
		}
	}

	if constexpr (matrix_t::rows * matrix_t::columns > 1)
	{
		BATCHED_SECTION("some zeroes")
		{
			matrix_t mat{ T{ 1 } };
			for (size_t r = 0, i = 0; r < matrix_t::rows; r++)
				for (size_t c = 0; c < matrix_t::columns; c++, i++)
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
	}

	BATCHED_SECTION("one zero")
	{
		for (size_t r = 0; r < matrix_t::rows; r++)
		{
			for (size_t c = 0; c < matrix_t::columns; c++)
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

BATCHED_TEST_CASE("matrix infinity_or_nan", common_matrices<ALL_ARITHMETIC>)
{
	using matrix_t = TestType;
	using T		   = typename matrix_t::scalar_type;
	TEST_INFO("matrix<"sv << nameof<T> << ", "sv << matrix_t::rows << ", "sv << matrix_t::columns << ">"sv);

	matrix_t mat;
	for (size_t r = 0; r < matrix_t::rows; r++)
		for (size_t c = 0; c < matrix_t::columns; c++)
			mat(r, c) = random<T>(1, 5);

	BATCHED_SECTION("all finite")
	{
		CHECK_FALSE(mat.infinity_or_nan());
		CHECK_FALSE(matrix_t::infinity_or_nan(mat));
		CHECK_FALSE(muu::infinity_or_nan(mat));
	}

	if constexpr (is_floating_point<T>)
	{
		BATCHED_SECTION("contains one NaN")
		{
			for (size_t r = 0; r < matrix_t::rows; r++)
			{
				for (size_t c = 0; c < matrix_t::columns; c++)
				{
					matrix_t mat2{ mat };
					mat2(r, c) = make_nan<T>();
					CHECK(mat2.infinity_or_nan());
					CHECK(matrix_t::infinity_or_nan(mat2));
					CHECK(muu::infinity_or_nan(mat2));
				}
			}
		}

		BATCHED_SECTION("contains one infinity")
		{
			for (size_t r = 0; r < matrix_t::rows; r++)
			{
				for (size_t c = 0; c < matrix_t::columns; c++)
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

BATCHED_TEST_CASE("matrix addition", common_matrices<ALL_ARITHMETIC>)
{
	using matrix_t = TestType;
	using T		   = typename matrix_t::scalar_type;
	TEST_INFO("matrix<"sv << nameof<T> << ", "sv << matrix_t::rows << ", "sv << matrix_t::columns << ">"sv);

	matrix_t mat1;
	matrix_t mat2;
	for (size_t r = 0; r < matrix_t::rows; r++)
	{
		for (size_t c = 0; c < matrix_t::columns; c++)
		{
			mat1(r, c) = random<T>(0, 5);
			mat2(r, c) = random<T>(1, 5);
		}
	}

	BATCHED_SECTION("matrix + matrix")
	{
		matrix_t result = mat1 + mat2;
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				CHECK_APPROX_EQUAL(static_cast<T>(mat1(r, c) + mat2(r, c)), result(r, c));
	}

	BATCHED_SECTION("matrix += matrix")
	{
		matrix_t result{ mat1 };
		result += mat2;
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				CHECK_APPROX_EQUAL(static_cast<T>(mat1(r, c) + mat2(r, c)), result(r, c));
	}
}

BATCHED_TEST_CASE("matrix subtraction", common_matrices<ALL_ARITHMETIC>)
{
	using matrix_t = TestType;
	using T		   = typename matrix_t::scalar_type;
	TEST_INFO("matrix<"sv << nameof<T> << ", "sv << matrix_t::rows << ", "sv << matrix_t::columns << ">"sv);

	matrix_t mat1;
	matrix_t mat2;
	for (size_t r = 0; r < matrix_t::rows; r++)
	{
		for (size_t c = 0; c < matrix_t::columns; c++)
		{
			if constexpr (is_signed<T>)
				mat1(r, c) = random<T>(0, 10);
			else
				mat1(r, c) = random<T>(11, 20);
			mat2(r, c) = random<T>(0, 10);
		}
	}

	BATCHED_SECTION("matrix - matrix")
	{
		matrix_t result = mat1 - mat2;
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				CHECK_APPROX_EQUAL(static_cast<T>(mat1(r, c) - mat2(r, c)), result(r, c));
	}

	BATCHED_SECTION("matrix -= matrix")
	{
		matrix_t result{ mat1 };
		result -= mat2;
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				CHECK_APPROX_EQUAL(static_cast<T>(mat1(r, c) - mat2(r, c)), result(r, c));
	}
}

BATCHED_TEST_CASE("matrix multiplication", common_matrices<ALL_ARITHMETIC>)
{
	using matrix_t = TestType;
	using T		   = typename matrix_t::scalar_type;
	TEST_INFO("matrix<"sv << nameof<T> << ", "sv << matrix_t::rows << ", "sv << matrix_t::columns << ">"sv);

	const auto min_val = T{ 1 }; // static_cast<T>(0.5);
	const auto max_val = T{ 5 }; // is_floating_point<T> ? T{ 1 } : T{ 5 };

	matrix_t mat1;
	for (size_t r = 0; r < matrix_t::rows; r++)
		for (size_t c = 0; c < matrix_t::columns; c++)
			mat1(r, c) = random<T>(min_val, max_val);

	BATCHED_SECTION("matrix * scalar")
	{
		const auto val	  = random<T>(min_val, max_val);
		const auto result = mat1 * val;
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				CHECK_APPROX_EQUAL(static_cast<T>(mat1(r, c) * val), result(r, c));
	}

	BATCHED_SECTION("scalar * matrix")
	{
		const auto val	  = random<T>(min_val, max_val);
		const auto result = val * mat1;
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				CHECK_APPROX_EQUAL(static_cast<T>(mat1(r, c) * val), result(r, c));
	}

	BATCHED_SECTION("matrix *= scalar")
	{
		const auto val = random<T>(min_val, max_val);
		auto result	   = mat1;
		result *= val;
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				CHECK_APPROX_EQUAL(static_cast<T>(mat1(r, c) * val), result(r, c));
	}

	BATCHED_SECTION("matrix * column vector")
	{
		const auto col_vec = vector<T, matrix_t::columns>{ random_array<T, matrix_t::columns>(min_val, max_val) };
		const auto result  = mat1 * col_vec;
		for (size_t r = 0; r < matrix_t::rows; r++)
		{
			typename matrix_t::row_type lhs_row;
			for (size_t i = 0; i < matrix_t::columns; i++)
				lhs_row[i] = mat1(r, i);

			if constexpr (is_floating_point<T> && sizeof(T) >= sizeof(double))
			{
				static constexpr T eps = constants<T>::default_epsilon * T{ 10 };
				CHECK_APPROX_EQUAL_EPS(static_cast<T>(lhs_row.dot(col_vec)), result[r], eps);
			}
			else
				CHECK_APPROX_EQUAL(static_cast<T>(lhs_row.dot(col_vec)), result[r]);
		}
	}

	BATCHED_SECTION("row vector * matrix")
	{
		const auto row_vec = vector<T, matrix_t::rows>{ random_array<T, matrix_t::rows>(min_val, max_val) };
		const auto result  = row_vec * mat1;
		for (size_t c = 0; c < matrix_t::columns; c++)
		{
			if constexpr (is_floating_point<T> && sizeof(T) >= sizeof(double))
			{
				static constexpr T eps = constants<T>::default_epsilon * T{ 10 };
				CHECK_APPROX_EQUAL_EPS(static_cast<T>(mat1.m[c].dot(row_vec)), result[c], eps);
			}
			else
				CHECK_APPROX_EQUAL(static_cast<T>(mat1.m[c].dot(row_vec)), result[c]);
		}
	}

	BATCHED_SECTION("matrix * matrix")
	{
		matrix<T, matrix_t::columns, matrix_t::rows> mat2;
		for (size_t r = 0; r < matrix_t::columns; r++)
			for (size_t c = 0; c < matrix_t::rows; c++)
				mat2(r, c) = random<T>(min_val, max_val);

		auto result = mat1 * mat2;
		for (size_t r = 0; r < matrix_t::rows; r++)
		{
			for (size_t c = 0; c < matrix_t::rows; c++) // rhs COLS, but here rhs COLS == lhs ROWS
			{
				typename matrix_t::row_type lhs_row;
				for (size_t i = 0; i < matrix_t::columns; i++)
					lhs_row[i] = mat1(r, i);
				if constexpr (is_floating_point<T> && sizeof(T) >= sizeof(double))
				{
					static constexpr T eps = constants<T>::default_epsilon * T{ 10 };
					CHECK_APPROX_EQUAL_EPS(static_cast<T>(lhs_row.dot(mat2.m[c])), result(r, c), eps);
				}
				else
					CHECK_APPROX_EQUAL(static_cast<T>(lhs_row.dot(mat2.m[c])), result(r, c));
			}
		}
	}
}

BATCHED_TEST_CASE("matrix multiplication special cases", all_matrices<ALL_ARITHMETIC>)
{
	using matrix_t = TestType;
	using T		   = typename matrix_t::scalar_type;
	TEST_INFO("matrix<"sv << nameof<T> << ", "sv << matrix_t::rows << ", "sv << matrix_t::columns << ">"sv);

	if constexpr (matrix_t::rows == 2 && matrix_t::columns == 3 && (sizeof(T) > 1 || is_unsigned<T>))
	{
		BATCHED_SECTION("matrix * matrix - case #1")
		{
			const auto lhs = matrix<T, 2, 3>{ T{ 1 }, T{ 2 }, T{ 3 }, T{ 4 }, T{ 5 }, T{ 6 } };

			const auto rhs = matrix<T, 3, 2>{
				T{ 7 }, T{ 8 }, T{ 9 }, T{ 10 }, T{ 11 }, T{ 12 },
			};

			const auto expected = matrix<T, 2, 2>{ T{ 58 }, T{ 64 }, T{ 139 }, T{ 154 } };

			CHECK_APPROX_EQUAL(lhs * rhs, expected);
		}
	}

	if constexpr (matrix_t::rows == 1 && matrix_t::columns == 3)
	{
		BATCHED_SECTION("matrix * matrix - case #2")
		{
			const auto lhs = matrix<T, 1, 3>{ T{ 3 }, T{ 4 }, T{ 2 } };

			const auto rhs = matrix<T, 3, 4>{
				T{ 13 }, T{ 9 }, T{ 7 }, T{ 15 }, T{ 8 }, T{ 7 }, T{ 4 }, T{ 6 }, T{ 6 }, T{ 4 }, T{ 0 }, T{ 3 },
			};

			const auto expected = matrix<T, 1, 4>{ T{ 83 }, T{ 63 }, T{ 37 }, T{ 75 } };

			CHECK_APPROX_EQUAL(lhs * rhs, expected);
		}
	}

	if constexpr (matrix_t::rows == 1 && matrix_t::columns == 3)
	{
		BATCHED_SECTION("matrix * matrix - case #3")
		{
			const auto lhs = matrix<T, 1, 3>{ T{ 1 }, T{ 2 }, T{ 3 } };

			const auto rhs = matrix<T, 3, 1>{ T{ 4 }, T{ 5 }, T{ 6 } };

			const auto expected = matrix<T, 1, 1>{ T{ 32 } };

			CHECK_APPROX_EQUAL(lhs * rhs, expected);
		}
	}

	if constexpr (matrix_t::rows == 3 && matrix_t::columns == 1)
	{
		BATCHED_SECTION("matrix * matrix - case #4")
		{
			const auto lhs = matrix<T, 3, 1>{ T{ 4 }, T{ 5 }, T{ 6 } };

			const auto rhs = matrix<T, 1, 3>{ T{ 1 }, T{ 2 }, T{ 3 } };

			const auto expected =
				matrix<T, 3, 3>{ T{ 4 }, T{ 8 }, T{ 12 }, T{ 5 }, T{ 10 }, T{ 15 }, T{ 6 }, T{ 12 }, T{ 18 } };

			CHECK_APPROX_EQUAL(lhs * rhs, expected);
		}
	}

	if constexpr (matrix_t::rows == 2 && matrix_t::columns == 2)
	{
		BATCHED_SECTION("matrix * matrix - case #5")
		{
			const auto lhs = matrix<T, 2, 2>{ T{ 1 }, T{ 2 }, T{ 3 }, T{ 4 } };

			const auto rhs = matrix<T, 2, 2>{ T{ 2 }, T{ 0 }, T{ 1 }, T{ 2 } };

			const auto expected = matrix<T, 2, 2>{ T{ 4 }, T{ 4 }, T{ 10 }, T{ 8 } };

			CHECK_APPROX_EQUAL(lhs * rhs, expected);
		}
	}

	if constexpr (matrix_t::rows == 2 && matrix_t::columns == 2)
	{
		BATCHED_SECTION("matrix * matrix - case #6")
		{
			const auto lhs = matrix<T, 2, 2>{ T{ 2 }, T{ 0 }, T{ 1 }, T{ 2 } };

			const auto rhs = matrix<T, 2, 2>{ T{ 1 }, T{ 2 }, T{ 3 }, T{ 4 } };

			const auto expected = matrix<T, 2, 2>{ T{ 2 }, T{ 4 }, T{ 7 }, T{ 10 } };

			CHECK_APPROX_EQUAL(lhs * rhs, expected);
		}
	}

	if constexpr (matrix_t::rows == 3 && matrix_t::columns == 3 && (sizeof(T) > 1 || is_unsigned<T>))
	{
		BATCHED_SECTION("matrix * matrix - case #7")
		{
			const auto lhs =
				matrix<T, 3, 3>{ T{ 10 }, T{ 20 }, T{ 10 }, T{ 4 }, T{ 5 }, T{ 6 }, T{ 2 }, T{ 3 }, T{ 5 } };

			const auto rhs = matrix<T, 3, 3>{ T{ 3 }, T{ 2 }, T{ 4 }, T{ 3 }, T{ 3 }, T{ 9 }, T{ 4 }, T{ 4 }, T{ 2 } };

			const auto expected =
				matrix<T, 3, 3>{ T{ 130 }, T{ 120 }, T{ 240 }, T{ 51 }, T{ 47 }, T{ 73 }, T{ 35 }, T{ 33 }, T{ 45 } };

			CHECK_APPROX_EQUAL(lhs * rhs, expected);
		}
	}

	if constexpr (matrix_t::rows == 4 && matrix_t::columns == 4 && sizeof(T) > 1)
	{
		BATCHED_SECTION("matrix * matrix - case #8")
		{
			const auto lhs = matrix<T, 4, 4>{ T{ 5 }, T{ 7 },  T{ 9 }, T{ 10 }, T{ 2 }, T{ 3 }, T{ 3 }, T{ 8 },
											  T{ 8 }, T{ 10 }, T{ 2 }, T{ 3 },	T{ 3 }, T{ 3 }, T{ 4 }, T{ 8 } };

			const auto rhs = matrix<T, 4, 4>{ T{ 3 }, T{ 10 }, T{ 12 }, T{ 18 }, T{ 12 }, T{ 1 },  T{ 4 }, T{ 9 },
											  T{ 9 }, T{ 10 }, T{ 12 }, T{ 2 },	 T{ 3 },  T{ 12 }, T{ 4 }, T{ 10 } };

			const auto expected =
				matrix<T, 4, 4>{ T{ 210 }, T{ 267 }, T{ 236 }, T{ 271 }, T{ 93 },  T{ 149 }, T{ 104 }, T{ 149 },
								 T{ 171 }, T{ 146 }, T{ 172 }, T{ 268 }, T{ 105 }, T{ 169 }, T{ 128 }, T{ 169 } };

			CHECK_APPROX_EQUAL(lhs * rhs, expected);
		}
	}
}

BATCHED_TEST_CASE("matrix division", common_matrices<ALL_ARITHMETIC>)
{
	using matrix_t = TestType;
	using T		   = typename matrix_t::scalar_type;
	TEST_INFO("matrix<"sv << nameof<T> << ", "sv << matrix_t::rows << ", "sv << matrix_t::columns << ">"sv);

	const auto scalar = static_cast<T>(2.4);
	matrix_t mat;
	for (size_t r = 0; r < matrix_t::rows; r++)
		for (size_t c = 0; c < matrix_t::columns; c++)
			mat(r, c) = random<T>(2, 5);

	using promoted = impl::promote_if_small_float<T>;

	BATCHED_SECTION("matrix / scalar")
	{
		const auto result = mat / scalar;
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				CHECK_APPROX_EQUAL(static_cast<T>(static_cast<promoted>(mat(r, c)) / static_cast<promoted>(scalar)),
								   result(r, c));
	}

	BATCHED_SECTION("matrix /= scalar")
	{
		auto result = mat;
		result /= scalar;
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				CHECK_APPROX_EQUAL(static_cast<T>(static_cast<promoted>(mat(r, c)) / static_cast<promoted>(scalar)),
								   result(r, c));
	}
}

BATCHED_TEST_CASE("matrix transpose", common_matrices<ALL_ARITHMETIC>)
{
	using matrix_t = TestType;
	using T		   = typename matrix_t::scalar_type;
	TEST_INFO("matrix<"sv << nameof<T> << ", "sv << matrix_t::rows << ", "sv << matrix_t::columns << ">"sv);

	matrix_t mat;
	for (size_t r = 0; r < matrix_t::rows; r++)
		for (size_t c = 0; c < matrix_t::columns; c++)
			mat(r, c) = random<T>(0, 10);

	BATCHED_SECTION("transpose(matrix)")
	{
		matrix<T, matrix_t::columns, matrix_t::rows> transposed = matrix_t::transpose(mat);
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				CHECK(transposed(c, r) == mat(r, c));
	}

	BATCHED_SECTION("muu::transpose(matrix)")
	{
		matrix<T, matrix_t::columns, matrix_t::rows> transposed = muu::transpose(mat);
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				CHECK(transposed(c, r) == mat(r, c));
	}

	if constexpr (matrix_t::rows == matrix_t::columns)
	{
		BATCHED_SECTION("matrix.transpose()")
		{
			matrix_t transposed{ mat };
			transposed.transpose();
			for (size_t r = 0; r < matrix_t::rows; r++)
				for (size_t c = 0; c < matrix_t::columns; c++)
					CHECK(transposed(c, r) == mat(r, c));
		}
	}
}

BATCHED_TEST_CASE("matrix determinant", square_matrices<ALL_ARITHMETIC>)
{
	using matrix_t = TestType;
	using T		   = typename matrix_t::scalar_type;
	TEST_INFO("matrix<"sv << nameof<T> << ", "sv << matrix_t::rows << ", "sv << matrix_t::columns << ">"sv);

	using determinant_t = typename matrix_t::determinant_type;

#define CHECK_DETERMINANT(expected)                                                                                    \
	CHECK_APPROX_EQUAL(matrix_t::determinant(mat), determinant_t{ expected });                                         \
	CHECK_APPROX_EQUAL(mat.determinant(), determinant_t{ expected });                                                  \
	CHECK_APPROX_EQUAL(muu::determinant(mat), determinant_t{ expected })

	if constexpr (matrix_t::rows == 2)
	{
		matrix_t mat{ T{ 3 }, T{ 8 }, T{ 4 }, T{ 6 } };
		CHECK_DETERMINANT(-14);

		mat = matrix_t{ T{ 2 }, T{ 5 }, T{ 3 }, T{ 8 } };
		CHECK_DETERMINANT(1);

		if constexpr (is_signed<T>)
		{
			mat = matrix_t{ T{ -4 }, T{ 7 }, T{ -2 }, T{ 9 } };
			CHECK_DETERMINANT(-22);
		}
	}

	if constexpr (matrix_t::rows == 3)
	{
		matrix_t mat{ T{ 1 }, T{ 2 }, T{ 3 }, T{ 4 }, T{ 5 }, T{ 6 }, T{ 7 }, T{ 8 }, T{ 9 } };
		CHECK_DETERMINANT(0);

		mat = matrix_t{ T{ 3 }, T{ 4 }, T{ 4 }, T{ 5 }, T{ 9 }, T{ 3 }, T{ 2 }, T{ 1 }, T{ 5 } };
		CHECK_DETERMINANT(-2);

		if constexpr (is_signed<T>)
		{
			mat = matrix_t{ T{ -2 }, T{ 3 }, T{ -1 }, T{ 5 }, T{ -1 }, T{ 4 }, T{ 4 }, T{ -8 }, T{ 2 } };
			CHECK_DETERMINANT(-6);

			mat = matrix_t{ T{ 2 }, T{ -3 }, T{ 1 }, T{ 2 }, T{ 0 }, T{ -1 }, T{ 1 }, T{ 4 }, T{ 5 } };
			CHECK_DETERMINANT(49);
		}
	}

	if constexpr (matrix_t::rows == 4)
	{
		matrix_t mat{ T{ 4 }, T{ 7 }, T{ 2 }, T{ 3 }, T{ 1 }, T{ 3 }, T{ 1 }, T{ 2 },
					  T{ 2 }, T{ 5 }, T{ 3 }, T{ 4 }, T{ 1 }, T{ 4 }, T{ 2 }, T{ 3 } };
		CHECK_DETERMINANT(-3);

		mat = matrix_t{ T{ 2 }, T{ 1 }, T{ 3 }, T{ 4 }, T{ 1 }, T{ 3 }, T{ 4 }, T{ 2 },
						T{ 3 }, T{ 4 }, T{ 2 }, T{ 1 }, T{ 4 }, T{ 2 }, T{ 1 }, T{ 3 } };
		CHECK_DETERMINANT(0);

		if constexpr (is_signed<T>)
		{
			mat = matrix_t{ T{ 2 },	 T{ 5 }, T{ 3 }, T{ 5 },  T{ 4 }, T{ 6 },  T{ 6 }, T{ 3 },
							T{ 11 }, T{ 3 }, T{ 2 }, T{ -2 }, T{ 4 }, T{ -7 }, T{ 9 }, T{ 3 } };
			CHECK_DETERMINANT(2960);
		}
	}
}

BATCHED_TEST_CASE("matrix inverse", square_matrices<ALL_FLOATS>)
{
	using matrix_t = TestType;
	using T		   = typename matrix_t::scalar_type;
	TEST_INFO("matrix<"sv << nameof<T> << ", "sv << matrix_t::rows << ", "sv << matrix_t::columns << ">"sv);

	using inverse_t		 = typename matrix_t::inverse_type;
	using inverse_scalar = typename inverse_t::scalar_type;

	static constexpr auto i = [](auto scalar) noexcept { return static_cast<inverse_scalar>(scalar); };

	using eps_common_type = impl::highest_ranked<inverse_scalar, double>;

	[[maybe_unused]] const inverse_scalar eps =
		static_cast<inverse_scalar>(muu::max(static_cast<eps_common_type>(constants<inverse_scalar>::default_epsilon),
											 static_cast<eps_common_type>(constants<double>::default_epsilon)));

#define CHECK_INVERSE(mat, expected)                                                                                   \
	do                                                                                                                 \
	{                                                                                                                  \
		auto result = matrix_t::invert(mat);                                                                           \
		CHECK_MATRIX_APPROX_EQUAL_EPS(result, expected, eps);                                                          \
                                                                                                                       \
		result = muu::invert(mat);                                                                                     \
		CHECK_MATRIX_APPROX_EQUAL_EPS(result, expected, eps);                                                          \
	}                                                                                                                  \
	while (false)

	if constexpr (matrix_t::rows == 2)
	{
		const auto mat = matrix_t{ T{ 4 }, T{ 7 }, T{ 2 }, T{ 6 } };

		const auto expected = inverse_t{ i(0.6), i(-0.7), i(-0.2), i(0.4) };

		CHECK_INVERSE(mat, expected);
	}

	if constexpr (matrix_t::rows == 3 && is_signed<T>)
	{
		const auto mat = matrix_t{ T{ 3 }, T{ 0 }, T{ 2 }, T{ 2 }, T{ 0 }, T{ -2 }, T{ 0 }, T{ 1 }, T{ 1 } };

		const auto expected = inverse_t{ i(0.2), i(0.2), i(0), i(-0.2), i(0.3), i(1), i(0.2), i(-0.3), i(0) };

		CHECK_INVERSE(mat, expected);
	}

	if constexpr (matrix_t::rows == 4)
	{
		const auto mat = matrix_t{ T{ 3 }, T{ 4 }, T{ 3 }, T{ 1 }, T{ 1 }, T{ 3 }, T{ 5 }, T{ 4 },
								   T{ 1 }, T{ 1 }, T{ 2 }, T{ 4 }, T{ 1 }, T{ 1 }, T{ 1 }, T{ 1 } };

		const auto expected = inverse_t{ i(-1),	  i(0),	  i(-1),   i(5),   i(1.8), i(-0.4), i(1.6), i(-6.6),
										 i(-1.2), i(0.6), i(-1.4), i(4.4), i(0.4), i(-0.2), i(0.8), i(-1.8) };

		CHECK_INVERSE(mat, expected);
	}
}

BATCHED_TEST_CASE("matrix orthonormalize", rotation_matrices<ALL_FLOATS>)
{
	using matrix_t = TestType;
	using T		   = typename matrix_t::scalar_type;
	TEST_INFO("matrix<"sv << nameof<T> << ", "sv << matrix_t::rows << ", "sv << matrix_t::columns << ">"sv);

	using vec3 = vector<T, 3>;
	using mat3 = matrix<T, 3, 3>;

	matrix_t raw;
	if constexpr (matrix_t::rows > 3 || matrix_t::columns > 3)
	{
		for (size_t r = 0; r < matrix_t::rows; r++)
			for (size_t c = 0; c < matrix_t::columns; c++)
				raw(r, c) = random<T>(1, 5);
	}
	mat3 raw_axes;
	raw_axes.m[0] = vec3::constants::x_axis * random<T>(2, 5);
	raw_axes.m[1] = vec3::constants::y_axis * random<T>(2, 5);
	raw_axes.m[2] = vec3::constants::z_axis * random<T>(2, 5);
	for (size_t c = 0; c < 3; c++)
		for (size_t r = 0; r < 3; r++)
			raw(r, c) = raw_axes(r, c);

	matrix_t ortho{ raw };
	ortho.orthonormalize();

	mat3 ortho_axes;
	for (size_t r = 0; r < 3; r++)
		for (size_t c = 0; c < 3; c++)
			ortho_axes(r, c) = ortho(r, c);

	// each should be unit-length
	CHECK(ortho_axes.m[0].normalized());
	CHECK(ortho_axes.m[1].normalized());
	CHECK(ortho_axes.m[2].normalized());

	// each should be orthogonal to the other
	CHECK_APPROX_EQUAL(vec3::dot(ortho_axes.m[0], ortho_axes.m[1]), T{});
	CHECK_APPROX_EQUAL(vec3::dot(ortho_axes.m[0], ortho_axes.m[2]), T{});
	CHECK_APPROX_EQUAL(vec3::dot(ortho_axes.m[1], ortho_axes.m[2]), T{});

	// any cells outside the inner 3x3 must remain unchanged
	if constexpr (matrix_t::rows > 3 || matrix_t::columns > 3)
	{
		for (size_t r = 0; r < matrix_t::rows; r++)
		{
			for (size_t c = 0; c < matrix_t::columns; c++)
			{
				if (r >= 3u || c >= 3u)
					CHECK(raw(r, c) == ortho(r, c));
			}
		}
	}
}
