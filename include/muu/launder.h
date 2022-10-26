// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the definition of #muu::launder().

#include "preprocessor.h"

#if MUU_CLANG >= 8 || MUU_GCC >= 7 || MUU_ICC >= 1910 || MUU_MSVC >= 1914 || MUU_HAS_BUILTIN(launder)
	#define MUU_LAUNDER(...) __builtin_launder(__VA_ARGS__)
#endif

MUU_DISABLE_WARNINGS;
#ifndef MUU_LAUNDER
	#include "std_new.h"
	#if defined(__cpp_lib_launder) && __cpp_lib_launder >= 201606
		#define MUU_LAUNDER(...) std::launder(__VA_ARGS__)
	#endif
#endif
MUU_ENABLE_WARNINGS;

#ifndef MUU_LAUNDER
	#define MUU_LAUNDER(...) __VA_ARGS__
#endif

#include "impl/std_type_traits.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS; // these should be considered "intrinsics"

namespace muu
{
	/// \brief	Equivalent to C++17's std::launder
	/// \ingroup core
	///
	/// \note	Older implementations don't provide this as an intrinsic or have a placeholder
	/// 		 for it in their standard library. Using this version allows you to get around that
	/// 		 by writing code 'as if' it were there and have it compile just the same.
	template <class T>
	MUU_CONST_INLINE_GETTER
	constexpr T* launder(T* ptr) noexcept
	{
		static_assert(!std::is_function_v<T> && !std::is_void_v<T>,
					  "launder() may not be used on pointers to functions or void.");

		return MUU_LAUNDER(ptr);
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
