// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \ingroup cpp20
/// \brief Contains the definition of #muu::source_location.

#include "fwd.h"
#include "impl/header_start.h"

namespace muu
{
	/// \brief	A backport of C++20's std::source_location.
	/// \ingroup cpp20
	///
	/// \availability This requires support for the `consteval` keyword, as well as some compiler intrinsics.
	class MUU_TRIVIAL_ABI source_location
	{
	  private:
		uint_least32_t line_{};
		uint_least32_t column_{};
		const char* file_	  = "";
		const char* function_ = "";

	  public:
		MUU_NODISCARD
		MUU_CONSTEVAL
		static source_location current(const uint_least32_t line   = __builtin_LINE(),
									   const uint_least32_t column = __builtin_COLUMN(),
									   const char* const file	   = __builtin_FILE(),
									   const char* const function  = __builtin_FUNCTION()) noexcept
		{
			source_location loc{};
			loc.line_	  = line;
			loc.column_	  = column;
			loc.file_	  = file;
			loc.function_ = function;
			return loc;
		}

		MUU_NODISCARD_CTOR
		constexpr source_location() noexcept = default;

		/// \brief The calling line.
		MUU_NODISCARD
		constexpr uint_least32_t line() const noexcept
		{
			return line_;
		}

		/// \brief The calling column.
		MUU_NODISCARD
		constexpr uint_least32_t column() const noexcept
		{
			return column_;
		}

		/// \brief The calling file name.
		MUU_NODISCARD
		constexpr const char* file_name() const noexcept
		{
			return file_;
		}

		/// \brief The calling function name.
		MUU_NODISCARD
		constexpr const char* function_name() const noexcept
		{
			return function_;
		}
	};
}

#include "impl/header_end.h"
