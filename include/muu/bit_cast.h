// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \ingroup cpp20
/// \brief Contains the definition of #muu::bit_cast().

#include "meta.h"
#include "impl/std_memcpy.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS; // these should be considered "intrinsics"
MUU_DISABLE_LIFETIME_WARNINGS;
#if 1

namespace muu
{
	//% bit_cast start
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
	MUU_CONSTRAINED_TEMPLATE((std::is_trivially_copyable_v<From>  //
							  && std::is_trivially_copyable_v<To> //
							  && sizeof(From) == sizeof(To)),
							 typename To,
							 typename From)
	MUU_PURE_INLINE_GETTER
	constexpr To bit_cast(const From& from) noexcept
	{
		static_assert(!std::is_reference_v<To> && !std::is_reference_v<From>);

	#if MUU_CLANG >= 11 || MUU_GCC >= 11 || MUU_MSVC >= 1926                                                           \
		|| (!MUU_CLANG && !MUU_GCC && MUU_HAS_BUILTIN(__builtin_bit_cast))

		#define MUU_HAS_INTRINSIC_BIT_CAST 1
		return __builtin_bit_cast(To, from);

	#else

		#define MUU_HAS_INTRINSIC_BIT_CAST 0

		if constexpr (std::is_same_v<std::remove_cv_t<From>, std::remove_cv_t<To>>)
		{
			return from;
		}
		//# {{
		else if constexpr (is_integral<From> && is_integral<To>)
		{
			return static_cast<To>(
				static_cast<remove_enum<std::remove_cv_t<To>>>(static_cast<remove_enum<From>>(from)));
		}
		//# }}
		else if constexpr (!std::is_nothrow_default_constructible_v<std::remove_cv_t<To>>)
		{
			union proxy_t
			{
				alignas(To) unsigned char dummy[sizeof(To)];
				std::remove_cv_t<To> to;

				proxy_t() noexcept
				{}
			};

			proxy_t proxy;
			MUU_MEMCPY(&proxy.to, &from, sizeof(To));
			return proxy.to;
		}
		else
		{
			static_assert(std::is_nothrow_default_constructible_v<std::remove_cv_t<To>>,
						  "Bit-cast fallback requires the To type be nothrow default-constructible");

			std::remove_cv_t<To> to;
			MUU_MEMCPY(&to, &from, sizeof(To));
			return to;
		}
	#endif
	}
	//% bit_cast end

	namespace build
	{
		/// \brief	True if using bit_cast() in constexpr contexts is supported on this compiler.
		inline constexpr bool supports_constexpr_bit_cast = !!MUU_HAS_INTRINSIC_BIT_CAST;
	}
}

#endif
MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
