// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Utilities for working with iterators.

#include "meta.h"
MUU_DISABLE_WARNINGS;
#include <iterator>
MUU_ENABLE_WARNINGS;
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;

namespace muu
{
	/// \brief	Returns the 'begin' iterator of an iterable object.
	/// \ingroup iterators
	///
	/// \detail This function tries to follow the same rules as the compiler[strong][entity 2A][/strong] for
	/// determining the 'begin' iterator in a [range-based for loop](https://en.cppreference.com/w/cpp/language/range-for):
	/// 1. Is the object a bounded array? If so, return a pointer to the first element.
	/// 2. Does the object have a member function `obj.begin()`? If so, call it and return the result.
	/// 3. Otherwise return the result of calling ADL-resolved free-function `begin(obj)`.
	///
	/// \parblock [set_parent_class m-note m-info] [strong][entity 2A] Some caveats apply:[/strong]
	/// - The compiler ignores access level of member `obj.begin()`; `begin_iterator()` can only call visible members
	/// - The compiler ignores the return type of member `obj.begin()`; `begin_iterator()` checks for basic
	///   [LegacyIterator](https://en.cppreference.com/w/cpp/named_req/Iterator) compatibility
	/// - The compiler does not examine the chosen `begin()` and `end()` functions as a pair;
	///   `begin_iterator()` checks that both exist and return compatible types.
	/// \endparblock
	template <typename T>
	MUU_NODISCARD
	MUU_ALWAYS_INLINE
	constexpr auto begin_iterator(T&& iterable) noexcept
	{
		if constexpr (is_bounded_array<T>)
		{
			using element_type = std::remove_extent_t<std::remove_reference_t<T>>;
			return static_cast<std::add_pointer_t<element_type>>(iterable);
		}
		else if constexpr (impl::has_iterator_member_funcs_<T&&>)
			return static_cast<T&&>(iterable).begin();
		else if constexpr (is_iterable<T&&>)
		{
			using std::begin;
			return begin(static_cast<T&&>(iterable));
		}
		else
			static_assert(always_false<T>, "was not an iterable type");
	}

	/// \brief	Returns the 'end' iterator of an iterable object.
	/// \ingroup iterators
	///
	/// \detail This function tries to follow the same rules as the compiler[strong][entity 2A][/strong] for
	/// determining the 'end' iterator in a [range-based for loop](https://en.cppreference.com/w/cpp/language/range-for):
	/// 1. Is the object a bounded array? If so, return a pointer to one-past-the-last element.
	/// 2. Does the object have a member function `obj.end()`? If so, call it and return the result.
	/// 3. Otherwise return the result of calling ADL-resolved free-function `end(obj)`.
	///
	/// \parblock [set_parent_class m-note m-info] [strong][entity 2A] Some caveats apply:[/strong]
	/// - The compiler ignores access level of member `obj.end()`; `end_iterator()` can only call visible members
	/// - The compiler ignores the return type of member `obj.end()`; `end_iterator()` checks for basic
	///   [LegacyIterator](https://en.cppreference.com/w/cpp/named_req/Iterator) compatibility
	/// - The compiler does not examine the chosen `begin()` and `end()` functions as a pair;
	///   `end_iterator()` checks that both exist and return compatible types.
	/// \endparblock
	template <typename T>
	MUU_NODISCARD
	MUU_ALWAYS_INLINE
	constexpr auto end_iterator(T&& iterable) noexcept
	{
		if constexpr (is_bounded_array<T>)
		{
			using element_type = std::remove_extent_t<std::remove_reference_t<T>>;
			return static_cast<std::add_pointer_t<element_type>>(iterable) + (sizeof(T) / sizeof(element_type));
		}
		else if constexpr (impl::has_iterator_member_funcs_<T&&>)
			return static_cast<T&&>(iterable).end();
		else if constexpr (is_iterable<T&&>)
		{
			using std::end;
			return end(static_cast<T&&>(iterable));
		}
		else
			static_assert(always_false<T>, "was not an iterable type");
	}

	/// \brief	Returns the distance between two iterators.
	/// \ingroup iterators
	///
	/// \remark This function works similarly to #std::distance(), which has an optimized implementation for Iterators
	/// satisfying LegacyRandomAccessIterator, but additionally optimizes for iterators that are _not_ strictly
	/// random-access but still provide an integral difference subtraction operation (`end - begin`).
	template <typename B, typename E>
	MUU_NODISCARD
	MUU_ALWAYS_INLINE
	constexpr auto iterator_distance(B begin, E end) noexcept
	{
		if constexpr (has_subtraction_operator<E, B>)
		{
			using result_type = decltype(std::declval<E>() - std::declval<B>());
			if constexpr (is_integral<result_type>)
				return unwrap(end - begin);
			else
				return std::distance(begin, end);
		}
		else
			return std::distance(begin, end);
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
