// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \ingroup cpp20
/// \brief Contains the definition of #muu::assume_aligned().

#include "impl/std_type_traits.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS; // these should be considered "intrinsics"

namespace muu
{
	//% assume_aligned start
	/// \brief	Equivalent to C++20's std::assume_aligned.
	/// \ingroup core
	///
	/// \remark Compilers typically implement std::assume_aligned as an intrinsic which is
	/// 		 available regardless of the C++ mode. Using this function on these compilers allows
	/// 		 you to get the same behaviour even when you aren't targeting C++20.
	///
	/// \see [P1007R1: std::assume_aligned](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1007r1.pdf)
	template <size_t N, typename T>
	MUU_CONST_INLINE_GETTER
	MUU_ATTR(assume_aligned(N))
	constexpr T* assume_aligned(T* ptr) noexcept
	{
		static_assert(N > 0 && (N & (N - 1u)) == 0u, "assume_aligned() requires a power-of-two alignment value.");
		static_assert(!std::is_function_v<T>, "assume_aligned may not be used on functions.");

		MUU_ASSUME((reinterpret_cast<uintptr_t>(ptr) & (N - uintptr_t{ 1 })) == 0);

		if constexpr (std::is_volatile_v<T>)
		{
			return static_cast<T*>(muu::assume_aligned<N>(const_cast<std::remove_volatile_t<T>*>(ptr)));
		}
		else
		{
#if MUU_CLANG || MUU_GCC || MUU_HAS_BUILTIN(assume_aligned)

			return static_cast<T*>(__builtin_assume_aligned(ptr, N));

#elif MUU_MSVC

			if constexpr (N < 16384)
				return static_cast<T*>(__builtin_assume_aligned(ptr, N));
			else
				return ptr;

#elif MUU_ICC

			__assume_aligned(ptr, N);
			return ptr;

#elif defined(__cpp_lib_assume_aligned)

			return std::assume_aligned<N>(ptr);

#else

			return ptr;

#endif
		}
	}
	//% assume_aligned end
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
