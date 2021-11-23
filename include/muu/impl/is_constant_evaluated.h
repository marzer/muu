// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the definition of muu::is_constant_evaluated.

#include "std_type_traits.h"
#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS; // these should be considered "intrinsics"

namespace muu
{
	/// \brief	Equivalent to C++20's std::is_constant_evaluated.
	/// \ingroup core
	///
	/// \remark Compilers typically implement std::is_constant_evaluated as an intrinsic which is
	/// 		 available regardless of the C++ mode. Using this function on these compilers allows
	/// 		 you to get the same behaviour even when you aren't targeting C++20.
	///
	/// \availability On older compilers lacking support for std::is_constant_evaluated this will always return `false`.
	/// 		   Check for support by examining build::supports_is_constant_evaluated.
	MUU_CONST_INLINE_GETTER
	MUU_ATTR(flatten)
	constexpr bool is_constant_evaluated() noexcept
	{
#if MUU_CLANG >= 9 || MUU_GCC >= 9 || MUU_MSVC >= 1925 || MUU_HAS_BUILTIN(is_constant_evaluated)
		return __builtin_is_constant_evaluated();
#elif defined(__cpp_lib_is_constant_evaluated)
		return std::is_constant_evaluated();
#else
		return false;
#endif
	}

	namespace build
	{
		/// \brief	True if is_constant_evaluated() is properly supported on this compiler.
		inline constexpr bool supports_is_constant_evaluated = is_constant_evaluated();
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
