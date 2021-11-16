// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

#include "core_meta.h"
#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_DISABLE_SHADOW_WARNINGS;
MUU_DISABLE_SUGGEST_WARNINGS;
MUU_DISABLE_ARITHMETIC_WARNINGS;
MUU_PRAGMA_MSVC(float_control(except, off))
MUU_PRAGMA_MSVC(float_control(precise, off))
/// \cond

namespace muu::impl
{
	struct value_fill_tag
	{};
	struct zero_fill_tag
	{};
	struct array_cast_tag
	{};
	struct tuple_cast_tag
	{};
	struct componentwise_func_tag
	{};
	struct tuple_concat_tag
	{};

	template <typename Scalar, size_t Dimensions>
	struct MUU_TRIVIAL_ABI vector_ //
	{
		static_assert(Dimensions > 4);

		Scalar values[Dimensions];

		vector_() noexcept = default;

		explicit constexpr vector_(zero_fill_tag) noexcept //
			: values{}
		{}

		template <size_t... Indices>
		explicit constexpr vector_(Scalar fill, std::index_sequence<Indices...>) noexcept
			: values{ (MUU_UNUSED(Indices), fill)... }
		{
			static_assert(sizeof...(Indices) <= Dimensions);
		}

		explicit constexpr vector_(value_fill_tag, Scalar fill) noexcept
			: vector_{ fill, std::make_index_sequence<Dimensions>{} }
		{}

		explicit constexpr vector_(Scalar x_, Scalar y_ = Scalar{}, Scalar z_ = Scalar{}, Scalar w_ = Scalar{}) noexcept
			: values{ x_, y_, z_, w_ }
		{}

		template <typename... T>
		explicit constexpr vector_(Scalar x_, Scalar y_, Scalar z_, Scalar w_, const T&... vals) noexcept
			: values{ x_, y_, z_, w_, static_cast<Scalar>(vals)... }
		{
			static_assert(sizeof...(T) <= Dimensions - 4);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_(array_cast_tag, std::index_sequence<Indices...>, const T& arr) noexcept
			: values{ static_cast<Scalar>(arr[Indices])... }
		{
			static_assert(sizeof...(Indices) <= Dimensions);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_(tuple_cast_tag, std::index_sequence<Indices...>, const T& tpl) noexcept
			: values{ static_cast<Scalar>(get_from_tuple_like<Indices>(tpl))... }
		{
			static_assert(sizeof...(Indices) <= Dimensions);
		}

		template <typename Func, size_t... Indices>
		explicit constexpr vector_(componentwise_func_tag, std::index_sequence<Indices...>, Func&& func) noexcept
			: values{ func(std::integral_constant<size_t, Indices>{})... }
		{
			static_assert(sizeof...(Indices) <= Dimensions);
		}

		template <typename T1, typename T2, size_t... Indices1, size_t... Indices2>
		explicit constexpr vector_(tuple_concat_tag,
								   std::index_sequence<Indices1...>,
								   const T1& tpl1,
								   std::index_sequence<Indices2...>,
								   const T2& tpl2) noexcept
			: values{ static_cast<Scalar>(get_from_tuple_like<Indices1>(tpl1))...,
					  static_cast<Scalar>(get_from_tuple_like<Indices2>(tpl2))... }
		{
			static_assert((sizeof...(Indices1) + sizeof...(Indices2)) <= Dimensions);
		}

		template <typename T, size_t... Indices, typename... V>
		explicit constexpr vector_(tuple_concat_tag,
								   std::index_sequence<Indices...>,
								   const T& tpl,
								   const V&... vals) noexcept
			: values{ static_cast<Scalar>(get_from_tuple_like<Indices>(tpl))..., static_cast<Scalar>(vals)... }
		{
			static_assert((sizeof...(Indices) + sizeof...(V)) <= Dimensions);
		}
	};

	template <typename Scalar>
	struct MUU_TRIVIAL_ABI vector_<Scalar, 1> //
	{
		Scalar x;

		vector_() noexcept = default;

		explicit constexpr vector_(zero_fill_tag) noexcept //
			: x{}
		{}

		explicit constexpr vector_(value_fill_tag, Scalar x_) noexcept //
			: x{ x_ }
		{}

		explicit constexpr vector_(Scalar x_) noexcept //
			: x{ x_ }
		{}

		template <typename T, size_t... Indices>
		explicit constexpr vector_(array_cast_tag, std::index_sequence<Indices...>, const T& arr) noexcept
			: vector_{ static_cast<Scalar>(arr[Indices])... }
		{
			static_assert(sizeof...(Indices) == 1);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_(tuple_cast_tag, std::index_sequence<Indices...>, const T& tpl) noexcept
			: vector_{ static_cast<Scalar>(get_from_tuple_like<Indices>(tpl))... }
		{
			static_assert(sizeof...(Indices) == 1);
		}
	};

	template <typename Scalar>
	struct MUU_TRIVIAL_ABI vector_<Scalar, 2> //
	{
		Scalar x;
		Scalar y;

		vector_() noexcept = default;

		explicit constexpr vector_(zero_fill_tag) noexcept //
			: x{},
			  y{}
		{}

		explicit constexpr vector_(value_fill_tag, Scalar fill) noexcept //
			: x{ fill },
			  y{ fill }
		{}

		explicit constexpr vector_(Scalar x_, Scalar y_ = Scalar{}) noexcept //
			: x{ x_ },
			  y{ y_ }
		{}

		template <typename T, size_t... Indices>
		explicit constexpr vector_(array_cast_tag, std::index_sequence<Indices...>, const T& arr) noexcept
			: vector_{ static_cast<Scalar>(arr[Indices])... }
		{
			static_assert(sizeof...(Indices) <= 2);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_(tuple_cast_tag, std::index_sequence<Indices...>, const T& tpl) noexcept
			: vector_{ static_cast<Scalar>(get_from_tuple_like<Indices>(tpl))... }
		{
			static_assert(sizeof...(Indices) <= 2);
		}

		template <typename T1, typename T2, size_t... Indices1, size_t... Indices2>
		explicit constexpr vector_(tuple_concat_tag,
								   std::index_sequence<Indices1...>,
								   const T1& tpl1,
								   std::index_sequence<Indices2...>,
								   const T2& tpl2) noexcept
			: vector_{ static_cast<Scalar>(get_from_tuple_like<Indices1>(tpl1))...,
					   static_cast<Scalar>(get_from_tuple_like<Indices2>(tpl2))... }
		{
			static_assert((sizeof...(Indices1) + sizeof...(Indices2)) <= 2);
		}

		template <typename T, size_t... Indices, typename... V>
		explicit constexpr vector_(tuple_concat_tag,
								   std::index_sequence<Indices...>,
								   const T& tpl,
								   const V&... vals) noexcept
			: vector_{ static_cast<Scalar>(get_from_tuple_like<Indices>(tpl))..., static_cast<Scalar>(vals)... }
		{
			static_assert((sizeof...(Indices) + sizeof...(V)) <= 2);
		}
	};

	template <typename Scalar>
	struct MUU_TRIVIAL_ABI vector_<Scalar, 3> //
	{
		Scalar x;
		Scalar y;
		Scalar z;

		vector_() noexcept = default;

		explicit constexpr vector_(zero_fill_tag) noexcept //
			: x{},
			  y{},
			  z{}
		{}

		explicit constexpr vector_(value_fill_tag, Scalar fill) noexcept //
			: x{ fill },
			  y{ fill },
			  z{ fill }
		{}

		explicit constexpr vector_(Scalar x_, Scalar y_ = Scalar{}, Scalar z_ = Scalar{}) noexcept
			: x{ x_ },
			  y{ y_ },
			  z{ z_ }
		{}

		template <typename T, size_t... Indices>
		explicit constexpr vector_(array_cast_tag, std::index_sequence<Indices...>, const T& arr) noexcept
			: vector_{ static_cast<Scalar>(arr[Indices])... }
		{
			static_assert(sizeof...(Indices) <= 3);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_(tuple_cast_tag, std::index_sequence<Indices...>, const T& tpl) noexcept
			: vector_{ static_cast<Scalar>(get_from_tuple_like<Indices>(tpl))... }
		{
			static_assert(sizeof...(Indices) <= 3);
		}

		template <typename T1, typename T2, size_t... Indices1, size_t... Indices2>
		explicit constexpr vector_(tuple_concat_tag,
								   std::index_sequence<Indices1...>,
								   const T1& tpl1,
								   std::index_sequence<Indices2...>,
								   const T2& tpl2) noexcept
			: vector_{ static_cast<Scalar>(get_from_tuple_like<Indices1>(tpl1))...,
					   static_cast<Scalar>(get_from_tuple_like<Indices2>(tpl2))... }
		{
			static_assert((sizeof...(Indices1) + sizeof...(Indices2)) <= 3);
		}

		template <typename T, size_t... Indices, typename... V>
		explicit constexpr vector_(tuple_concat_tag,
								   std::index_sequence<Indices...>,
								   const T& tpl,
								   const V&... vals) noexcept
			: vector_{ static_cast<Scalar>(get_from_tuple_like<Indices>(tpl))..., static_cast<Scalar>(vals)... }
		{
			static_assert((sizeof...(Indices) + sizeof...(V)) <= 3);
		}
	};

	template <typename Scalar>
	struct MUU_TRIVIAL_ABI vector_<Scalar, 4> //
	{
		Scalar x;
		Scalar y;
		Scalar z;
		Scalar w;

		vector_() noexcept = default;

		explicit constexpr vector_(zero_fill_tag) noexcept //
			: x{},
			  y{},
			  z{},
			  w{}
		{}

		explicit constexpr vector_(value_fill_tag, Scalar fill) noexcept //
			: x{ fill },
			  y{ fill },
			  z{ fill },
			  w{ fill }
		{}

		explicit constexpr vector_(Scalar x_, Scalar y_ = Scalar{}, Scalar z_ = Scalar{}, Scalar w_ = Scalar{}) noexcept
			: x{ x_ },
			  y{ y_ },
			  z{ z_ },
			  w{ w_ }
		{}

		template <typename T, size_t... Indices>
		explicit constexpr vector_(array_cast_tag, std::index_sequence<Indices...>, const T& arr) noexcept
			: vector_{ static_cast<Scalar>(arr[Indices])... }
		{
			static_assert(sizeof...(Indices) <= 4);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_(tuple_cast_tag, std::index_sequence<Indices...>, const T& tpl) noexcept
			: vector_{ static_cast<Scalar>(get_from_tuple_like<Indices>(tpl))... }
		{
			static_assert(sizeof...(Indices) <= 4);
		}

		template <typename T1, typename T2, size_t... Indices1, size_t... Indices2>
		explicit constexpr vector_(tuple_concat_tag,
								   std::index_sequence<Indices1...>,
								   const T1& tpl1,
								   std::index_sequence<Indices2...>,
								   const T2& tpl2) noexcept
			: vector_{ static_cast<Scalar>(get_from_tuple_like<Indices1>(tpl1))...,
					   static_cast<Scalar>(get_from_tuple_like<Indices2>(tpl2))... }
		{
			static_assert((sizeof...(Indices1) + sizeof...(Indices2)) <= 4);
		}

		template <typename T, size_t... Indices, typename... V>
		explicit constexpr vector_(tuple_concat_tag,
								   std::index_sequence<Indices...>,
								   const T& tpl,
								   const V&... vals) noexcept
			: vector_{ static_cast<Scalar>(get_from_tuple_like<Indices>(tpl))..., static_cast<Scalar>(vals)... }
		{
			static_assert((sizeof...(Indices) + sizeof...(V)) <= 4);
		}
	};

	template <typename Scalar, size_t Dimensions>
	inline constexpr bool is_hva<vector_<Scalar, Dimensions>> = can_be_hva_of<Scalar, vector_<Scalar, Dimensions>>;

	template <typename Scalar, size_t Dimensions>
	inline constexpr bool is_hva<vector<Scalar, Dimensions>> = is_hva<vector_<Scalar, Dimensions>>;

	template <typename Scalar, size_t Dimensions>
	struct vectorcall_param_<vector<Scalar, Dimensions>>
	{
		using type = std::conditional_t<pass_vectorcall_by_value<vector_<Scalar, Dimensions>>,
										vector<Scalar, Dimensions>,
										const vector<Scalar, Dimensions>&>;
	};
}

namespace muu
{
	template <typename From, typename Scalar, size_t Dimensions>
	inline constexpr bool allow_implicit_bit_cast<From, impl::vector_<Scalar, Dimensions>> =
		allow_implicit_bit_cast<From, vector<Scalar, Dimensions>>;
}

/// \endcond
MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
