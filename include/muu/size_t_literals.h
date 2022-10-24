// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definitions of a number of literal operators for std::size_t.

#include "fwd.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS; // these should be considered "intrinsics"

namespace muu
{
	inline namespace literals
	{
		/// \brief	Creates a size_t.
		/// \detail \cpp
		/// const size_t val = 42_sz;
		/// \ecpp
		MUU_CONST_INLINE_GETTER
		MUU_CONSTEVAL
		size_t operator"" _sz(unsigned long long n) noexcept
		{
			return static_cast<size_t>(n);
		}

		/// \brief	Creates a size_t for a given number of bytes.
		/// \detail \cpp
		/// const size_t val = 42_b; // 42
		/// \ecpp
		MUU_CONST_INLINE_GETTER
		MUU_CONSTEVAL
		size_t operator"" _b(unsigned long long b) noexcept
		{
			return static_cast<size_t>(b);
		}

		/// \brief	Creates a size_t for a given number of kilobytes.
		/// \detail \cpp
		/// const size_t val = 42_kb; // 42 * 1024
		/// \ecpp
		MUU_CONST_INLINE_GETTER
		MUU_CONSTEVAL
		size_t operator"" _kb(unsigned long long kb) noexcept
		{
			return static_cast<size_t>(kb * 1024ull);
		}

		/// \brief	Creates a size_t for a given number of megabytes.
		/// \detail \cpp
		/// const size_t val = 42_mb; // 42 * 1024 * 1024
		/// \ecpp
		MUU_CONST_INLINE_GETTER
		MUU_CONSTEVAL
		size_t operator"" _mb(unsigned long long mb) noexcept
		{
			return static_cast<size_t>(mb * 1024ull * 1024ull);
		}

		/// \brief	Creates a size_t for a given number of gigabytes.
		/// \detail \cpp
		/// const size_t val = 42_gb; // 42 * 1024 * 1024 * 1024
		/// \ecpp
		MUU_CONST_INLINE_GETTER
		MUU_CONSTEVAL
		size_t operator"" _gb(unsigned long long gb) noexcept
		{
			return static_cast<size_t>(gb * 1024ull * 1024ull * 1024ull);
		}
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
