// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "core_meta.h"

MUU_DISABLE_WARNINGS;
#include <iterator>
MUU_ENABLE_WARNINGS;

#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;

/// \cond
namespace muu::impl
{
	template <typename T>
	MUU_NODISCARD
	MUU_ALWAYS_INLINE
	constexpr auto get_begin_iterator(T&& iterable) noexcept
	{
		if constexpr (has_iterator_member_funcs_<T&&>)
			return static_cast<T&&>(iterable).begin();
		else
		{
			using std::begin;
			return begin(static_cast<T&&>(iterable));
		}
	}

	template <typename T>
	MUU_NODISCARD
	MUU_ALWAYS_INLINE
	constexpr auto get_end_iterator(T&& iterable) noexcept
	{
		if constexpr (has_iterator_member_funcs_<T&&>)
			return static_cast<T&&>(iterable).end();
		else
		{
			using std::end;
			return end(static_cast<T&&>(iterable));
		}
	}

	template <typename B, typename E>
	MUU_NODISCARD
	MUU_ALWAYS_INLINE
	constexpr auto get_iterator_distance(B begin, E end) noexcept
	{
		if constexpr (is_detected_<has_subtraction_operator_, E, B>)
			return end - begin;
		else
			return std::distance(begin, end);
	}
}
// \endcond

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
