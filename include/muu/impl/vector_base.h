// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once
/// \cond

#include "../meta.h"
#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(float_control(except, off))

namespace muu::impl
{
	struct componentwise_tag
	{};
	struct broadcast_tag
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
	struct vector_base;

	template <typename Scalar>
	struct MUU_TRIVIAL_ABI vector_base<Scalar, 1> //
	{
		using scalar_type = Scalar;

		scalar_type x;

		MUU_NODISCARD_CTOR
		vector_base() noexcept = default;

		MUU_NODISCARD_CTOR
		constexpr vector_base(const vector_base&) noexcept = default;

		constexpr vector_base& operator=(const vector_base&) noexcept = default;

		MUU_NODISCARD_CTOR
		explicit constexpr vector_base(scalar_type x_) noexcept //
			: x{ x_ }
		{}

	  protected:
		MUU_NODISCARD_CTOR
		explicit constexpr vector_base(broadcast_tag, scalar_type broadcast) noexcept //
			: x{ broadcast }
		{}

		template <typename T, size_t... I>
		MUU_NODISCARD_CTOR
		MUU_ALWAYS_INLINE
		explicit constexpr vector_base(array_cast_tag, std::index_sequence<I...>, const T& arr) noexcept
			: x{ static_cast<scalar_type>(arr[I])... }
		{
			static_assert(sizeof...(I) == 1);
		}

		template <typename T, size_t... I>
		MUU_NODISCARD_CTOR
		MUU_ALWAYS_INLINE
		explicit constexpr vector_base(tuple_cast_tag, std::index_sequence<I...>, const T& tpl) noexcept
			: x{ static_cast<scalar_type>(get_from_tuple_like<I>(tpl))... }
		{
			static_assert(sizeof...(I) == 1);
		}
	};

	template <typename Scalar>
	struct MUU_TRIVIAL_ABI vector_base<Scalar, 2> //
	{
		using scalar_type = Scalar;

		scalar_type x;
		scalar_type y;

		MUU_NODISCARD_CTOR
		vector_base() noexcept = default;

		MUU_NODISCARD_CTOR
		constexpr vector_base(const vector_base&) noexcept = default;

		constexpr vector_base& operator=(const vector_base&) noexcept = default;

		MUU_NODISCARD_CTOR
		MUU_ALWAYS_INLINE
		/*implicit*/ constexpr vector_base(scalar_type x_, scalar_type y_) noexcept //
			: vector_base{ componentwise_tag{}, x_, y_ }
		{}

	  protected:
		MUU_NODISCARD_CTOR
		explicit constexpr vector_base(componentwise_tag,
									   scalar_type x_ = scalar_type{},
									   scalar_type y_ = scalar_type{}) noexcept //
			: x{ x_ },
			  y{ y_ }
		{}

		MUU_NODISCARD_CTOR
		explicit constexpr vector_base(broadcast_tag, scalar_type broadcast) noexcept //
			: x{ broadcast },
			  y{ broadcast }
		{}

		template <typename T, size_t... I>
		MUU_NODISCARD_CTOR
		MUU_ALWAYS_INLINE
		explicit constexpr vector_base(array_cast_tag, std::index_sequence<I...>, const T& arr) noexcept
			: vector_base{ componentwise_tag{}, static_cast<scalar_type>(arr[I])... }
		{
			static_assert(sizeof...(I) <= 2);
		}

		template <typename T, size_t... I>
		MUU_NODISCARD_CTOR
		MUU_ALWAYS_INLINE
		explicit constexpr vector_base(tuple_cast_tag, std::index_sequence<I...>, const T& tpl) noexcept
			: vector_base{ componentwise_tag{}, static_cast<scalar_type>(get_from_tuple_like<I>(tpl))... }
		{
			static_assert(sizeof...(I) <= 2);
		}

		template <typename T1, typename T2, size_t... I1, size_t... I2>
		MUU_NODISCARD_CTOR
		MUU_ALWAYS_INLINE
		explicit constexpr vector_base(tuple_concat_tag,
									   std::index_sequence<I1...>,
									   const T1& tpl1,
									   std::index_sequence<I2...>,
									   const T2& tpl2) noexcept
			: vector_base{ componentwise_tag{},
						   static_cast<scalar_type>(get_from_tuple_like<I1>(tpl1))...,
						   static_cast<scalar_type>(get_from_tuple_like<I2>(tpl2))... }
		{
			static_assert((sizeof...(I1) + sizeof...(I2)) <= 2);
		}

		template <typename T, size_t... I, typename... V>
		MUU_NODISCARD_CTOR
		MUU_ALWAYS_INLINE
		explicit constexpr vector_base(tuple_concat_tag,
									   std::index_sequence<I...>,
									   const T& tpl,
									   const V&... vals) noexcept
			: vector_base{ componentwise_tag{},
						   static_cast<scalar_type>(get_from_tuple_like<I>(tpl))...,
						   static_cast<scalar_type>(vals)... }
		{
			static_assert((sizeof...(I) + sizeof...(V)) <= 2);
		}
	};

	template <typename Scalar>
	struct MUU_TRIVIAL_ABI vector_base<Scalar, 3> //
	{
		using scalar_type = Scalar;

		scalar_type x;
		scalar_type y;
		scalar_type z;

		MUU_NODISCARD_CTOR
		vector_base() noexcept = default;

		MUU_NODISCARD_CTOR
		constexpr vector_base(const vector_base&) noexcept = default;

		constexpr vector_base& operator=(const vector_base&) noexcept = default;

		MUU_NODISCARD_CTOR
		MUU_ALWAYS_INLINE
		/*implicit*/ constexpr vector_base(scalar_type x_, scalar_type y_, scalar_type z_ = scalar_type{}) noexcept //
			: vector_base{ componentwise_tag{}, x_, y_, z_ }
		{}

	  protected:
		MUU_NODISCARD_CTOR
		explicit constexpr vector_base(componentwise_tag,
									   scalar_type x_ = scalar_type{},
									   scalar_type y_ = scalar_type{},
									   scalar_type z_ = scalar_type{}) noexcept //
			: x{ x_ },
			  y{ y_ },
			  z{ z_ }
		{}

		MUU_NODISCARD_CTOR
		explicit constexpr vector_base(broadcast_tag, scalar_type broadcast) noexcept //
			: x{ broadcast },
			  y{ broadcast },
			  z{ broadcast }
		{}

		template <typename T, size_t... I>
		MUU_NODISCARD_CTOR
		MUU_ALWAYS_INLINE
		explicit constexpr vector_base(array_cast_tag, std::index_sequence<I...>, const T& arr) noexcept
			: vector_base{ componentwise_tag{}, static_cast<scalar_type>(arr[I])... }
		{
			static_assert(sizeof...(I) <= 3);
		}

		template <typename T, size_t... I>
		MUU_NODISCARD_CTOR
		MUU_ALWAYS_INLINE
		explicit constexpr vector_base(tuple_cast_tag, std::index_sequence<I...>, const T& tpl) noexcept
			: vector_base{ componentwise_tag{}, static_cast<scalar_type>(get_from_tuple_like<I>(tpl))... }
		{
			static_assert(sizeof...(I) <= 3);
		}

		template <typename T1, typename T2, size_t... I1, size_t... I2>
		MUU_NODISCARD_CTOR
		MUU_ALWAYS_INLINE
		explicit constexpr vector_base(tuple_concat_tag,
									   std::index_sequence<I1...>,
									   const T1& tpl1,
									   std::index_sequence<I2...>,
									   const T2& tpl2) noexcept
			: vector_base{ componentwise_tag{},
						   static_cast<scalar_type>(get_from_tuple_like<I1>(tpl1))...,
						   static_cast<scalar_type>(get_from_tuple_like<I2>(tpl2))... }
		{
			static_assert((sizeof...(I1) + sizeof...(I2)) <= 3);
		}

		template <typename T, size_t... I, typename... V>
		MUU_NODISCARD_CTOR
		MUU_ALWAYS_INLINE
		explicit constexpr vector_base(tuple_concat_tag,
									   std::index_sequence<I...>,
									   const T& tpl,
									   const V&... vals) noexcept
			: vector_base{ componentwise_tag{},
						   static_cast<scalar_type>(get_from_tuple_like<I>(tpl))...,
						   static_cast<scalar_type>(vals)... }
		{
			static_assert((sizeof...(I) + sizeof...(V)) <= 3);
		}
	};

	template <typename Scalar>
	struct MUU_TRIVIAL_ABI vector_base<Scalar, 4> //
	{
		using scalar_type = Scalar;

		scalar_type x;
		scalar_type y;
		scalar_type z;
		scalar_type w;

		MUU_NODISCARD_CTOR
		vector_base() noexcept = default;

		MUU_NODISCARD_CTOR
		constexpr vector_base(const vector_base&) noexcept = default;

		constexpr vector_base& operator=(const vector_base&) noexcept = default;

		MUU_NODISCARD_CTOR
		MUU_ALWAYS_INLINE
		/*implicit*/ constexpr vector_base(scalar_type x_,
										   scalar_type y_,
										   scalar_type z_ = scalar_type{},
										   scalar_type w_ = scalar_type{}) noexcept //
			: vector_base{ componentwise_tag{}, x_, y_, z_, w_ }
		{}

	  protected:
		MUU_NODISCARD_CTOR
		explicit constexpr vector_base(componentwise_tag,
									   scalar_type x_ = scalar_type{},
									   scalar_type y_ = scalar_type{},
									   scalar_type z_ = scalar_type{},
									   scalar_type w_ = scalar_type{}) noexcept //
			: x{ x_ },
			  y{ y_ },
			  z{ z_ },
			  w{ w_ }
		{}

		MUU_NODISCARD_CTOR
		explicit constexpr vector_base(broadcast_tag, scalar_type broadcast) noexcept //
			: x{ broadcast },
			  y{ broadcast },
			  z{ broadcast },
			  w{ broadcast }
		{}

		template <typename T, size_t... I>
		MUU_NODISCARD_CTOR
		MUU_ALWAYS_INLINE
		explicit constexpr vector_base(array_cast_tag, std::index_sequence<I...>, const T& arr) noexcept
			: vector_base{ componentwise_tag{}, static_cast<scalar_type>(arr[I])... }
		{
			static_assert(sizeof...(I) <= 4);
		}

		template <typename T, size_t... I>
		MUU_NODISCARD_CTOR
		MUU_ALWAYS_INLINE
		explicit constexpr vector_base(tuple_cast_tag, std::index_sequence<I...>, const T& tpl) noexcept
			: vector_base{ componentwise_tag{}, static_cast<scalar_type>(get_from_tuple_like<I>(tpl))... }
		{
			static_assert(sizeof...(I) <= 4);
		}

		template <typename T1, typename T2, size_t... I1, size_t... I2>
		MUU_NODISCARD_CTOR
		MUU_ALWAYS_INLINE
		explicit constexpr vector_base(tuple_concat_tag,
									   std::index_sequence<I1...>,
									   const T1& tpl1,
									   std::index_sequence<I2...>,
									   const T2& tpl2) noexcept
			: vector_base{ componentwise_tag{},
						   static_cast<scalar_type>(get_from_tuple_like<I1>(tpl1))...,
						   static_cast<scalar_type>(get_from_tuple_like<I2>(tpl2))... }
		{
			static_assert((sizeof...(I1) + sizeof...(I2)) <= 4);
		}

		template <typename T, size_t... I, typename... V>
		MUU_NODISCARD_CTOR
		MUU_ALWAYS_INLINE
		explicit constexpr vector_base(tuple_concat_tag,
									   std::index_sequence<I...>,
									   const T& tpl,
									   const V&... vals) noexcept
			: vector_base{ componentwise_tag{},
						   static_cast<scalar_type>(get_from_tuple_like<I>(tpl))...,
						   static_cast<scalar_type>(vals)... }
		{
			static_assert((sizeof...(I) + sizeof...(V)) <= 4);
		}
	};

	template <typename Scalar, size_t Dimensions>
	struct MUU_TRIVIAL_ABI vector_base //
	{
		static_assert(Dimensions > 4);
		using scalar_type = Scalar;

		scalar_type values[Dimensions];

		MUU_NODISCARD_CTOR
		vector_base() noexcept = default;

		MUU_NODISCARD_CTOR
		constexpr vector_base(const vector_base&) noexcept = default;

		constexpr vector_base& operator=(const vector_base&) noexcept = default;

		MUU_NODISCARD_CTOR
		/*implicit*/ constexpr vector_base(scalar_type x_,
										   scalar_type y_,
										   scalar_type z_ = scalar_type{},
										   scalar_type w_ = scalar_type{}) noexcept //
			: values{ x_, y_, z_, w_ }
		{}

		MUU_CONSTRAINED_TEMPLATE((Dims >= (4 + sizeof...(T)) //
								  && all_convertible_to<scalar_type, const T&...>),
								 typename... T,
								 size_t Dims = Dimensions)
		MUU_NODISCARD_CTOR
		/*implicit*/ constexpr vector_base(scalar_type x_,
										   scalar_type y_,
										   scalar_type z_,
										   scalar_type w_,
										   const T&... vals) noexcept
			: values{ x_, y_, z_, w_, static_cast<scalar_type>(vals)... }
		{
			static_assert(sizeof...(T) <= Dimensions - 4);
		}

	  protected:
		template <size_t... I>
		MUU_NODISCARD_CTOR
		explicit constexpr vector_base(scalar_type broadcast, std::index_sequence<I...>) noexcept
			: values{ (static_cast<void>(I), broadcast)... }
		{
			static_assert(sizeof...(I) <= Dimensions);
		}

		MUU_NODISCARD_CTOR
		explicit constexpr vector_base(broadcast_tag, scalar_type broadcast) noexcept //
			: vector_base{ broadcast, std::make_index_sequence<Dimensions>{} }
		{}

		template <typename T, size_t... I>
		MUU_NODISCARD_CTOR
		explicit constexpr vector_base(array_cast_tag, std::index_sequence<I...>, const T& arr) noexcept
			: values{ static_cast<scalar_type>(arr[I])... }
		{
			static_assert(sizeof...(I) <= Dimensions);
		}

		template <typename T, size_t... I>
		MUU_NODISCARD_CTOR
		explicit constexpr vector_base(tuple_cast_tag, std::index_sequence<I...>, const T& tpl) noexcept
			: values{ static_cast<scalar_type>(get_from_tuple_like<I>(tpl))... }
		{
			static_assert(sizeof...(I) <= Dimensions);
		}

		template <typename Func, size_t... I>
		MUU_NODISCARD_CTOR
		explicit constexpr vector_base(componentwise_func_tag, std::index_sequence<I...>, Func&& func) noexcept
			: values{ static_cast<Func&&>(func)(std::integral_constant<size_t, I>{})... }
		{
			static_assert(sizeof...(I) <= Dimensions);
		}

		template <typename T1, typename T2, size_t... I1, size_t... I2>
		MUU_NODISCARD_CTOR
		explicit constexpr vector_base(tuple_concat_tag,
									   std::index_sequence<I1...>,
									   const T1& tpl1,
									   std::index_sequence<I2...>,
									   const T2& tpl2) noexcept
			: values{ static_cast<scalar_type>(get_from_tuple_like<I1>(tpl1))...,
					  static_cast<scalar_type>(get_from_tuple_like<I2>(tpl2))... }
		{
			static_assert((sizeof...(I1) + sizeof...(I2)) <= Dimensions);
		}

		template <typename T, size_t... I, typename... V>
		MUU_NODISCARD_CTOR
		explicit constexpr vector_base(tuple_concat_tag,
									   std::index_sequence<I...>,
									   const T& tpl,
									   const V&... vals) noexcept
			: values{ static_cast<scalar_type>(get_from_tuple_like<I>(tpl))..., static_cast<scalar_type>(vals)... }
		{
			static_assert((sizeof...(I) + sizeof...(V)) <= Dimensions);
		}
	};

	template <typename Scalar, size_t Dimensions>
	inline constexpr bool is_hva<vector_base<Scalar, Dimensions>> =
		can_be_hva_of<Scalar, vector_base<Scalar, Dimensions>>;

	template <typename Scalar, size_t Dimensions>
	inline constexpr bool is_hva<vector<Scalar, Dimensions>> = is_hva<vector_base<Scalar, Dimensions>>;

	template <typename Scalar, size_t Dimensions>
	struct vector_param_<vector<Scalar, Dimensions>>
	{
		using type = copy_cvref<vector<Scalar, Dimensions>, vector_param<vector_base<Scalar, Dimensions>>>;
	};
}

namespace muu
{
	template <typename From, typename Scalar, size_t Dimensions>
	inline constexpr bool allow_implicit_bit_cast<From, impl::vector_base<Scalar, Dimensions>> =
		allow_implicit_bit_cast<From, vector<Scalar, Dimensions>>;
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
/// \endcond
