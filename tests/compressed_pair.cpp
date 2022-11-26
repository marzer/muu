// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/compressed_pair.h"

struct empty
{};
struct empty2
{};

// implementation sanity-checks
static_assert(std::is_empty_v<empty>);
static_assert(std::is_empty_v<empty2>);
static_assert(!std::is_empty_v<int>);

template <typename First, typename Second, template <class> typename Trait>
inline constexpr bool compressed_pair_matches_members =
	Trait<compressed_pair<First, Second>>::value == (Trait<First>::value && Trait<Second>::value);

template <typename First, typename Second>
MUU_CONSTEVAL
bool compressed_pair_static_checks() noexcept
{
	using pair = compressed_pair<First, Second>;

	if constexpr (std::is_empty_v<First> && std::is_empty_v<Second>)
		static_assert(sizeof(pair) == 1);
	else if constexpr (std::is_empty_v<First>)
		static_assert(sizeof(pair) == sizeof(Second));
	else if constexpr (std::is_empty_v<Second>)
		static_assert(sizeof(pair) == sizeof(First));
	else
		static_assert(sizeof(pair) == sizeof(First) + sizeof(Second));

	static_assert(is_tuple_like<pair>);

	static_assert(
		compressed_pair_matches_members<First, Second, std::is_empty> //
		|| (MUU_MSVC && MUU_MSVC < 1929)
		// compiler bug:
		// https://developercommunity.visualstudio.com/content/problem/1142409/c-stdis-empty-is-incorrect-for-a-template-speciali.html
	);

	static_assert(compressed_pair_matches_members<First, Second, std::is_standard_layout>);
	static_assert(compressed_pair_matches_members<First, Second, std::is_trivially_default_constructible>);
	static_assert(compressed_pair_matches_members<First, Second, std::is_trivially_copy_constructible>);
	static_assert(compressed_pair_matches_members<First, Second, std::is_trivially_copy_assignable>);
	static_assert(compressed_pair_matches_members<First, Second, std::is_trivially_move_constructible>);
	static_assert(compressed_pair_matches_members<First, Second, std::is_trivially_move_assignable>);
	static_assert(compressed_pair_matches_members<First, Second, std::is_trivially_destructible>);
	static_assert(compressed_pair_matches_members<First, Second, std::is_nothrow_default_constructible>);
	static_assert(compressed_pair_matches_members<First, Second, std::is_nothrow_copy_constructible>);
	static_assert(compressed_pair_matches_members<First, Second, std::is_nothrow_copy_assignable>);
	static_assert(compressed_pair_matches_members<First, Second, std::is_nothrow_move_constructible>);
	static_assert(compressed_pair_matches_members<First, Second, std::is_nothrow_move_assignable>);
	static_assert(compressed_pair_matches_members<First, Second, std::is_nothrow_destructible>);

	static_assert(std::is_same_v<decltype(std::declval<pair&>().first()), First&>);
	static_assert(std::is_same_v<decltype(std::declval<pair&&>().first()), First&&>);
	static_assert(std::is_same_v<decltype(std::declval<const pair&>().first()), const First&>);
	static_assert(std::is_same_v<decltype(std::declval<const pair&&>().first()), const First&&>);

	static_assert(std::is_same_v<decltype(std::declval<pair&>().second()), Second&>);
	static_assert(std::is_same_v<decltype(std::declval<pair&&>().second()), Second&&>);
	static_assert(std::is_same_v<decltype(std::declval<const pair&>().second()), const Second&>);
	static_assert(std::is_same_v<decltype(std::declval<const pair&&>().second()), const Second&&>);

	static_assert(std::is_same_v<decltype(std::declval<pair&>().template get<0>()), First&>);
	static_assert(std::is_same_v<decltype(std::declval<pair&&>().template get<0>()), First&&>);
	static_assert(std::is_same_v<decltype(std::declval<const pair&>().template get<0>()), const First&>);
	static_assert(std::is_same_v<decltype(std::declval<const pair&&>().template get<0>()), const First&&>);

	static_assert(std::is_same_v<decltype(std::declval<pair&>().template get<1>()), Second&>);
	static_assert(std::is_same_v<decltype(std::declval<pair&&>().template get<1>()), Second&&>);
	static_assert(std::is_same_v<decltype(std::declval<const pair&>().template get<1>()), const Second&>);
	static_assert(std::is_same_v<decltype(std::declval<const pair&&>().template get<1>()), const Second&&>);

	return true;
}

static_assert(compressed_pair_static_checks<int, int>());
static_assert(compressed_pair_static_checks<empty, int>());
static_assert(compressed_pair_static_checks<int, empty>());
static_assert(compressed_pair_static_checks<empty, empty2>());
