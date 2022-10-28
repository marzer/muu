// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \ingroup cpp20
/// \brief Contains the definition of #muu::source_location.

#ifndef MUU_SOURCE_LOCATION_COLUMN
	#if MUU_GCC
		#define MUU_SOURCE_LOCATION_COLUMN() 0
	#else
		#define MUU_SOURCE_LOCATION_COLUMN() __builtin_COLUMN()
	#endif
#endif

#include "fwd.h"
#include "impl/header_start.h"

/// \cond
namespace muu::impl
{
	MUU_PURE_GETTER
	MUU_CONSTEVAL
	const char* sloc_file_name_tail(const char* const file_path) noexcept
	{
		const char* end = file_path;
		while (*end)
			end++;
		const auto len = static_cast<size_t>(end - file_path);

		const char* slash = {};
		for (auto i = len; i-- > 0u && !slash;)
			if (file_path[i] == '\\' || file_path[i] == '/')
				slash = file_path + i;

		return slash ? slash + 1u : file_path;
	}
}
/// \endcond

namespace muu
{
	/// \brief	A backport of C++20's std::source_location.
	/// \ingroup cpp20
	///
	/// \availability This requires support some compiler intrinsics.
	class MUU_TRIVIAL_ABI source_location
	{
	  private:
		uint_least32_t line_{};
		uint_least32_t column_{};
		const char* file_	   = "";
		const char* function_  = "";
		const char* file_tail_ = "";

	  public:
		MUU_NODISCARD
		MUU_CONSTEVAL
		static source_location current(const uint_least32_t line   = __builtin_LINE(),
									   const uint_least32_t column = MUU_SOURCE_LOCATION_COLUMN(),
									   const char* const file	   = __builtin_FILE(),
									   const char* const function  = __builtin_FUNCTION()) noexcept
		{
			source_location loc{};
			loc.line_	   = line;
			loc.column_	   = column;
			loc.file_	   = file;
			loc.function_  = function;
			loc.file_tail_ = impl::sloc_file_name_tail(file);
			return loc;
		}

		/// \brief Default destructor.
		MUU_NODISCARD_CTOR
		constexpr source_location() noexcept = default;

		/// \brief The calling line.
		MUU_NODISCARD
		constexpr uint_least32_t line() const noexcept
		{
			return line_;
		}

		/// \brief The calling column.
		/// \availability This will always be `0` on some compilers.
		MUU_NODISCARD
		constexpr uint_least32_t column() const noexcept
		{
			return column_;
		}

		/// \brief The calling file path.
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

		/// \brief The filename-only 'tail' of the calling file path.
		///
		/// \attention	This method is a non-standard extension to std::source_location.
		MUU_NODISCARD
		constexpr const char* file_name_tail() const noexcept
		{
			return file_tail_;
		}
	};
}

#include "impl/header_end.h"
