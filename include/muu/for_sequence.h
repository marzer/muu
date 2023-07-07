// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the definitions of #muu::for_sequence() and #muu::for_product().

#include "impl/std_utility.h"
#include "impl/std_type_traits.h"
#include "impl/header_start.h"

namespace muu
{
	//% for_sequence start
	/// \cond
	namespace impl
	{
		template <typename Func, typename T, T... N>
		inline constexpr bool for_sequence_impl_noexcept_ =
			sizeof...(N) == 0 || ((noexcept(std::declval<Func>()(std::integral_constant<T, N>{})) && ...));

		template <auto Num, bool AlwaysInline = (Num <= 4)> // force inline for a small number of invocations
		struct for_sequence_impl
		{
			template <typename Func, typename T, T... N>
			MUU_ALWAYS_INLINE
			static constexpr void invoke(Func&& func, std::integer_sequence<T, N...>) noexcept(
				for_sequence_impl_noexcept_<Func&&, T, N...>)
			{
				(static_cast<void>(static_cast<Func&&>(func)(std::integral_constant<T, N>{})), ...);
			}
		};

		template <auto Num>
		struct for_sequence_impl<Num, false>
		{
			template <typename Func, typename T, T... N>
			static constexpr void invoke(Func&& func, std::integer_sequence<T, N...>) noexcept(
				for_sequence_impl_noexcept_<Func&&, T, N...>)
			{
				(static_cast<void>(static_cast<Func&&>(func)(std::integral_constant<T, N>{})), ...);
			}
		};

		template <auto N, typename Func>
		inline constexpr bool for_sequence_noexcept_ =
			noexcept(for_sequence_impl<N>::invoke(std::declval<Func>(), std::make_integer_sequence<decltype(N), N>{}));
	}
	/// \endcond

	/// \brief	Generates a series of sequential function calls by pack expansion.
	/// \ingroup core
	///
	/// \details Generates a std::integral_constant sequence, [0...N1), which is passed into the callable: \cpp
	///
	/// auto vals = std::tuple{ 0, "1"sv, 2.3f };
	/// for_sequence<3>([&](auto i)
	/// {
	///		std::cout << std::get<decltype(i)::value>(vals) << "\n";
	/// });
	/// \ecpp
	///
	/// \out
	/// 0
	/// 1
	/// 2.3
	/// \eout
	///
	/// \tparam N		The length of the sequence (the number of calls). Cannot be negative.
	/// \tparam Func	A callable type with the signature `void(auto)`.
	template <auto N, typename Func>
	MUU_ALWAYS_INLINE
	constexpr void for_sequence(Func&& func) noexcept(impl::for_sequence_noexcept_<N, Func>)
	{
		using n_type = decltype(N);
		static_assert(N >= n_type{}, "N cannot be negative.");

		if constexpr (N > n_type{})
			impl::for_sequence_impl<N>::invoke(static_cast<Func&&>(func), std::make_integer_sequence<n_type, N>{});
		else
			static_cast<void>(func);
	}
	//% for_sequence end

	/// \cond
	namespace impl
	{
		template <typename Func, typename T, T... N1, typename U, U... N2>
		MUU_NODISCARD
		MUU_CONSTEVAL
		bool for_product_impl_noexcept(std::integer_sequence<T, N1...>, std::integer_sequence<U, N2...>) noexcept
		{
			if constexpr (sizeof...(N1) == 0 || sizeof...(N2) == 0)
				return true;
			else
			{
				bool value = true;

				const auto inner = [&](auto i, auto j) noexcept
				{
					value = noexcept(std::declval<Func>()(std::integral_constant<T, decltype(i)::value>{},
														  std::integral_constant<U, decltype(j)::value>{}))
						 && value;
				};

				const auto outer = [&](auto i) noexcept
				{ (static_cast<void>(inner(i, std::integral_constant<U, N2>{})), ...); };

				(static_cast<void>(outer(std::integral_constant<T, N1>{})), ...);

				return value;
			}
		}

		template <typename Func, typename T, T... N1, typename U, U... N2>
		constexpr void for_product_impl(Func&& func,
										std::integer_sequence<T, N1...>,
										std::integer_sequence<U,
															  N2...>) //
			noexcept(for_product_impl_noexcept<Func&&>(std::integer_sequence<T, N1...>{},
													   std::integer_sequence<U, N2...>{}))
		{
			const auto inner = [&](auto i, auto j)
			{
				static_cast<Func&&>(func)(std::integral_constant<T, decltype(i)::value>{},
										  std::integral_constant<U, decltype(j)::value>{});
			};

			const auto outer = [&](auto i) { (static_cast<void>(inner(i, std::integral_constant<U, N2>{})), ...); };

			(static_cast<void>(outer(std::integral_constant<T, N1>{})), ...);
		}

		template <auto N1, auto N2, typename Func>
		inline constexpr bool for_product_noexcept_ =
			noexcept(for_product_impl(std::declval<Func>(),
									  std::make_integer_sequence<decltype(N1), N1>{},
									  std::make_integer_sequence<decltype(N2), N2>{}));
	}
	/// \endcond

	/// \brief	Generates a series of sequential function calls by pack expansion.
	/// \ingroup core
	///
	/// \details	Generates a std::integral_constant cartesian product sequence, [0...N1) * [0...N2),
	///				which is passed into the callable: \cpp
	///
	/// auto vals_1 = std::pair{ 1, 2 };
	/// auto vals_2 = std::pair{ 3, 4 };
	/// for_product<2, 2>([&](auto i, auto j)
	/// {
	///		const auto lhs = std::get<decltype(i)::value>(vals_1);
	///		const auto rhs = std::get<decltype(j)::value>(vals_2);
	///		std::cout << lhs
	///		          << " * "
	///		          << rhs
	///		          << " = "
	///		          << lhs * rhs;
	/// });
	/// \ecpp
	///
	/// \out
	/// 1 * 3 = 3
	/// 1 * 4 = 4
	/// 2 * 3 = 6
	/// 2 * 4 = 8
	/// \eout
	///
	/// \tparam N1		The length of the LHS sequence. Cannot be negative.
	/// \tparam N2		The length of the RHS sequence. Cannot be negative.
	/// \tparam Func	A callable type with the signature `void(auto, auto)`.
	template <auto N1, auto N2, typename Func>
	constexpr void for_product(Func&& func) noexcept(impl::for_product_noexcept_<N1, N2, Func>)
	{
		using n1_type = decltype(N1);
		static_assert(N1 >= n1_type{}, "N1 cannot be negative.");

		using n2_type = decltype(N2);
		static_assert(N2 >= n2_type{}, "N2 cannot be negative.");

		if constexpr (N1 > n1_type{} && N2 > n2_type{})
			impl::for_product_impl(static_cast<Func&&>(func),
								   std::make_integer_sequence<n1_type, N1>{},
								   std::make_integer_sequence<n2_type, N2>{});
		else
			MUU_UNUSED(func);
	}
}

#include "impl/header_end.h"
