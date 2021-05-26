// This file is a part of toml++ and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/tomlplusplus/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "tests.h"

template <typename T>
using code_unit_func = bool(T)noexcept;

template <typename T, int ID>
struct code_unit_func_group;

template <typename T>
struct code_unit_range
{
	T first;
	T last;

	template <typename U, typename V>
	MUU_NODISCARD_CTOR
	constexpr code_unit_range(U first_, V last_) noexcept
		: first{ static_cast<T>(first_) },
		last{ static_cast<T>(last_) }
	{
		if (last < first)
			std::swap(first, last);
	}

	template <typename U>
	MUU_NODISCARD_CTOR
	constexpr code_unit_range(U first_) noexcept
		: first{ static_cast<T>(first_) },
		last{ first }
	{}
};

template <typename T, typename U>
MUU_NODISCARD
inline bool in(code_unit_func<T>* func, const code_unit_range<U>& range) noexcept
{
	for (auto cu = static_cast<muu::make_unsigned<U>>(range.first), e = static_cast<muu::make_unsigned<U>>(range.last); cu <= e; cu++)
		if (!func(static_cast<T>(cu)))
			return false;
	return true;
}

template <typename T, typename U>
MUU_NODISCARD
inline bool not_in(code_unit_func<T>* func, const code_unit_range<U>& range) noexcept
{
	for (auto cu = static_cast<muu::make_unsigned<U>>(range.first), e = static_cast<muu::make_unsigned<U>>(range.last); cu <= e; cu++)
		if (func(static_cast<T>(cu)))
			return false;
	return true;
}

template <int GroupID, typename T>
MUU_NODISCARD
inline bool in_only(code_unit_func<T>* func, T cu) noexcept
{
	if (!func(cu))
		return false;
	for (auto fn : code_unit_func_group<T, GroupID>::functions)
	{
		if (fn == func)
			continue;
		if (fn(cu))
			return false;
	}
	return true;
}

template <int GroupID, typename T, typename U>
MUU_NODISCARD
inline bool in_only(code_unit_func<T>* func, const code_unit_range<U>& range) noexcept
{
	for (auto cu = static_cast<muu::make_unsigned<U>>(range.first), e = static_cast<muu::make_unsigned<U>>(range.last); cu <= e; cu++)
		if (!in_only<GroupID>(func, static_cast<T>(cu)))
			return false;
	return true;
}
