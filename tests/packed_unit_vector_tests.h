// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"
#include "batching.h"
#include "../include/muu/packed_unit_vector.h"

namespace
{
	template <typename Integer, size_t Dimensions, typename Float>
	struct puv_test_tuple
	{
		using vector_type = packed_unit_vector<Integer, Dimensions>;
		using float_type  = Float;
	};

	template <typename Integer, typename Float>
	using make_puvs_both_dimensions = type_list<puv_test_tuple<Integer, 2, Float>, puv_test_tuple<Integer, 3, Float>>;

	template <typename, typename>
	struct puv_test_tuple_product;

	template <typename... Integers, typename... Floats>
	struct puv_test_tuple_product<type_list<Integers...>, type_list<Floats...>>
	{
		using integers = type_list<Integers...>;
		using floats   = type_list<Floats...>;

		template <size_t Index>
		using puvs_at_index = make_puvs_both_dimensions<typename integers::template select<Index / floats::length>,
														typename floats::template select<Index % floats::length>>;

		template <typename>
		struct flattener;

		template <size_t... Indices>
		struct flattener<std::index_sequence<Indices...>>
		{
			using types = typename type_list<puvs_at_index<Indices>...>::flatten;
		};

		using types = typename flattener<std::make_index_sequence<sizeof...(Integers) * sizeof...(Floats)>>::types;
	};

	using all_packed_unit_vectors =
		typename puv_test_tuple_product<type_list<ALL_UNSIGNED_INTS>, type_list<ALL_FLOATS>>::types;

	template <typename>
	struct packed_vector_bit_printer;

	template <typename Integer, size_t Dimensions>
	struct packed_vector_bit_printer<packed_unit_vector<Integer, Dimensions>>
	{
		using vector_type = packed_unit_vector<Integer, Dimensions>;

		using traits	 = impl::packed_unit_vector_traits<Integer, Dimensions>;
		using arith_type = typename traits::arith_type;

		template <size_t Length, size_t Skip = 0>
		using print = print_bits<Skip, Length, arith_type>;

		const vector_type& value;

		template <typename Char>
		friend std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& lhs, const packed_vector_bit_printer& rhs)
		{
			const auto bits = static_cast<arith_type>(rhs.value.bits);

			lhs << print<traits::sign_bits, traits::x_bits + traits::y_bits>{ bits };
			lhs.put(static_cast<Char>(32));
			lhs << print<traits::x_bits, traits::y_bits>{ bits };
			if constexpr (Dimensions >= 3)
			{
				lhs.put(static_cast<Char>(32));
				lhs << print<traits::y_bits>{ bits };
			}

			return lhs;
		}
	};

	template <typename T>
	packed_vector_bit_printer(T) -> packed_vector_bit_printer<T>;
}

BATCHED_TEST_CASE("packed_unit_vector", all_packed_unit_vectors)
{
	using pvec						   = typename TestType::vector_type;
	using I							   = typename pvec::integer_type;
	using F							   = typename TestType::float_type;
	using D							   = impl::highest_ranked<double, F>;
	using vec						   = vector<F, pvec::dimensions>;
	using dvec						   = vector<D, pvec::dimensions>;
	static constexpr size_t dimensions = pvec::dimensions;
	static constexpr size_t int_bits   = build::bits_per_byte * sizeof(I);
	static constexpr size_t float_bits = constants<F>::significand_digits;

	if constexpr (int_bits > 8 && float_bits >= 24 && dimensions == 3) // exclude ints <= uint8_t and floats < f32
	{
		static_assert(!impl::is_small_float_<F>);

		TEST_INFO("packed_unit_vector<"sv << nameof<I> << ", "sv << dimensions << "> <=> "sv << nameof<F>);

		TEST_INFO("integer bits:   "sv << int_bits);
		TEST_INFO("float sig bits: "sv << float_bits);

		static_assert(sizeof(pvec) == sizeof(I));
		static_assert(alignof(pvec) == alignof(I));
		static_assert(std::is_standard_layout_v<pvec>);
		static_assert(std::is_trivially_constructible_v<pvec>);
		static_assert(std::is_trivially_copy_constructible_v<pvec>);
		static_assert(std::is_trivially_copy_assignable_v<pvec>);
		static_assert(std::is_trivially_move_constructible_v<pvec>);
		static_assert(std::is_trivially_move_assignable_v<pvec>);
		static_assert(std::is_trivially_destructible_v<pvec>);
		static_assert(std::is_nothrow_constructible_v<pvec, vec>);
		static_assert(muu::is_nothrow_convertible<pvec, vec>);

		static constexpr auto random_unit_vector = []() noexcept -> vec
		{
			while (true)
			{
				using rand_type = promote_if_small_float<F>;
				vector<rand_type, dimensions> v;
				v.x = random<rand_type>(-1, 1);
				v.y = random<rand_type>(-1, 1);
				if constexpr (vec::dimensions > 2)
					v.z = random<rand_type>(-1, 1);
				v.normalize();

				if (infinity_or_nan(v))
					continue;

				if constexpr (!std::is_same_v<rand_type, F>)
				{
					auto out = vec{ v };
					out.normalize();
					if (infinity_or_nan(out))
						continue;
					return out;
				}
				else
					return v;
			}
			MUU_UNREACHABLE;
		};

		// note: i realize this is pretty convoluted but there's a lot of interplay between the backing integer type,
		// the vector float type, and the number of dimensions in the packed vector, so this sort of nonsense
		// can't really be avoided
		auto eps_precision = 1000000000ull;
		if constexpr (int_bits <= 8u)
		{
			eps_precision = 100ull;
			if constexpr (dimensions >= 3u)
				eps_precision /= 15ull;
		}
		else if constexpr (int_bits > 8u && int_bits <= 64u)
		{
			eps_precision /= 1000ull;
			if constexpr (dimensions >= 3u)
				eps_precision /= 1000ull;
		}
		else if constexpr (int_bits > 64u && int_bits <= 128u)
		{
			eps_precision /= 1000ull;
		}
		const D epsilon = D{ 1 } / static_cast<D>(eps_precision);
		TEST_INFO("eps:    "sv << std::fixed << std::setprecision(constants<D>::decimal_digits + 2) << epsilon);

		for (size_t i = 0; i < 1000_sz; i++)
		{
			const vec in = random_unit_vector();
			const pvec packed{ in };
			const vec out{ packed };
			const auto dot = dvec::dot(dvec{ in }, dvec{ out });

			TEST_INFO("dot:    "sv << std::fixed << std::setprecision(constants<D>::decimal_digits + 2) << dot);
			TEST_INFO("in:     "sv << in);
			TEST_INFO("out:    "sv << out);
			TEST_INFO("packed: "sv << packed_vector_bit_printer{ packed });

			if (in.x != F{})
				REQUIRE(packed.x_negative() == (in.x < F{}));
			if (in.y != F{})
				REQUIRE(packed.y_negative() == (in.y < F{}));
			if constexpr (dimensions >= 3u)
			{
				if (in.z != F{})
					REQUIRE(packed.z_negative() == (in.z < F{}));
			}

			if (out.x != F{})
				REQUIRE(packed.x_negative() == (out.x < F{}));
			if (out.y != F{})
				REQUIRE(packed.y_negative() == (out.y < F{}));
			if constexpr (dimensions >= 3u)
			{
				if (out.z != F{})
					REQUIRE(packed.z_negative() == (out.z < F{}));
			}

			REQUIRE(muu::approx_equal(dot, D{ 1 }, epsilon));
		}
	}
}
