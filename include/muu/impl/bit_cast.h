// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the definition of muu::bit_cast.

#include "../meta.h"
#include "std_memcpy.h"
#if MUU_CLANG >= 12 && defined(__cpp_lib_bit_cast) && __cpp_lib_bit_cast >= 201806
MUU_DISABLE_WARNINGS;
	#include <bit>
MUU_ENABLE_WARNINGS;
#endif
#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS; // these should be considered "intrinsics"
MUU_DISABLE_LIFETIME_WARNINGS;

#define MUU_HAS_INTRINSIC_BIT_CAST 1

namespace muu
{
	/// \brief	Equivalent to C++20's std::bit_cast.
	/// \ingroup core
	///
	/// \remark Compilers implement this as an intrinsic which is typically
	/// 		 available regardless of the C++ mode. Using this function
	/// 		 on these compilers allows you to get the same behaviour
	/// 		 even when you aren't targeting C++20.
	///
	/// \availability On older compilers lacking support for std::bit_cast you won't be able to call this function
	/// 		   in constexpr contexts (since it falls back to a memcpy-based implementation).
	/// 		   Check for constexpr support by examining build::supports_constexpr_bit_cast.
	template <typename To, typename From>
	MUU_PURE_INLINE_GETTER
	MUU_ATTR(flatten)
	constexpr To bit_cast(const From& from) noexcept
	{
		static_assert(!std::is_reference_v<From> && !std::is_reference_v<To>, "From and To types cannot be references");
		static_assert(std::is_trivially_copyable_v<From> && std::is_trivially_copyable_v<To>,
					  "From and To types must be trivially-copyable");
		static_assert(sizeof(From) == sizeof(To), "From and To types must be the same size");

#if MUU_CLANG == 11 || MUU_GCC >= 11 || MUU_MSVC >= 1926

		return __builtin_bit_cast(To, from);

#elif MUU_CLANG >= 12 && defined(__cpp_lib_bit_cast) && __cpp_lib_bit_cast >= 201806

		return std::bit_cast<To>(from);

#else

	#undef MUU_HAS_INTRINSIC_BIT_CAST
	#define MUU_HAS_INTRINSIC_BIT_CAST 0

		if constexpr (std::is_same_v<remove_cv<From>, remove_cv<To>>)
		{
			return from;
		}
		else if constexpr (all_integral<From, To>)
		{
			return static_cast<To>(
				static_cast<std::underlying_type_t<remove_cv<To>>>(static_cast<std::underlying_type_t<From>>(from)));
		}
		else
		{
			static_assert(std::is_nothrow_default_constructible_v<remove_cv<To>>,
						  "Bit-cast fallback requires the To type be nothrow default-constructible");

			remove_cv<To> dst;
			MUU_MEMCPY(&dst, &from, sizeof(To));
			return dst;
		}

#endif
	}

	namespace build
	{
		/// \brief	True if using bit_cast() in constexpr contexts is supported on this compiler.
		inline constexpr bool supports_constexpr_bit_cast = !!MUU_HAS_INTRINSIC_BIT_CAST;
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
