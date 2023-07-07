// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the definition of #muu::apply_alignment().

#include "meta.h"
#include "assume_aligned.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS; // these should be considered "intrinsics"

namespace muu
{
	//% apply_alignment start
	/// \brief	Rounds an unsigned value up to the next multiple of the given alignment.
	/// \ingroup core
	///
	/// \tparam 	Alignment	The alignment to round up to. Must be a power of two.
	/// \tparam	T	An unsigned integer or enum type.
	/// \param 	val	The unsigned value being aligned.
	MUU_CONSTRAINED_TEMPLATE(is_unsigned<T>, size_t Alignment, typename T)
	MUU_CONST_INLINE_GETTER
	constexpr T apply_alignment(T val) noexcept
	{
		static_assert(Alignment, "alignment cannot be zero");
		static_assert((Alignment & (Alignment - 1u)) == 0u, "alignment must be a power of two");

		if constexpr (std::is_enum_v<T>)
		{
			return static_cast<T>(apply_alignment(static_cast<std::underlying_type_t<T>>(val)));
		}
		else
		{
			using uint = decltype(T{} * size_t{});
			return static_cast<T>((val + Alignment - uint{ 1 }) & ~(Alignment - uint{ 1 }));
		}
	}


	/// \brief	Rounds a pointer up to the byte offset that is the next multiple of the given alignment.
	/// \ingroup core
	///
	/// \tparam 		Alignment	The alignment to round up to. Must be a power of two.
	/// \tparam	T		An object type (or void).
	/// \param 	ptr		The pointer being aligned.
	template <size_t Alignment, typename T>
	MUU_CONST_INLINE_GETTER
	MUU_ATTR(nonnull)
	MUU_ATTR(returns_nonnull)
	MUU_ATTR(assume_aligned(Alignment))
	constexpr T* apply_alignment(T* ptr) noexcept
	{
		static_assert(!std::is_function_v<T>, "apply_alignment() may not be used on pointers to functions.");
		static_assert(Alignment, "alignment cannot be zero");
		static_assert((Alignment & (Alignment - 1u)) == 0u, "alignment must be a power of two");
		if constexpr (!std::is_void_v<T>)
		{
			static_assert(Alignment >= alignof(T), "cannot under-align types.");
		}

		return muu::assume_aligned<Alignment>(
			reinterpret_cast<T*>(apply_alignment<Alignment>(reinterpret_cast<uintptr_t>(ptr))));
	}
	//% apply_alignment end

	/// \brief	Rounds an unsigned value up to the next multiple of the given alignment.
	/// \ingroup core
	///
	/// \tparam	T			An unsigned integer or enum type.
	/// \param 	val			The unsigned value being aligned.
	/// \param 	alignment	The alignment to round up to. Must be a power of two.
	MUU_CONSTRAINED_TEMPLATE(is_unsigned<T>, typename T)
	MUU_CONST_GETTER
	constexpr T apply_alignment(T val, size_t alignment) noexcept
	{
		MUU_CONSTEXPR_SAFE_ASSERT(alignment && "alignment cannot be zero");
		MUU_CONSTEXPR_SAFE_ASSERT(((alignment & (alignment - 1u)) == 0u) && "alignment must be a power of two");
		MUU_ASSUME(alignment > 0u);
		MUU_ASSUME((alignment & (alignment - 1u)) == 0u);

		if constexpr (std::is_enum_v<T>)
		{
			return static_cast<T>(apply_alignment(static_cast<std::underlying_type_t<T>>(val), alignment));
		}
		else
		{
			using uint = decltype(T{} * size_t{});
			return static_cast<T>((val + alignment - uint{ 1 }) & ~(alignment - uint{ 1 }));
		}
	}

	/// \brief	Rounds a pointer up to the byte offset that is the next multiple of the given alignment.
	/// \ingroup core
	///
	/// \tparam	T			An object type (or void).
	/// \param 	ptr			The pointer being aligned.
	/// \param 	alignment	The alignment to round up to. Must be a power of two.
	template <typename T>
	MUU_CONST_GETTER
	MUU_ATTR(nonnull)
	MUU_ATTR(returns_nonnull)
	constexpr T* apply_alignment(T* ptr, size_t alignment) noexcept
	{
		static_assert(!std::is_function_v<T>, "apply_alignment() may not be used on pointers to functions.");
		MUU_CONSTEXPR_SAFE_ASSERT(alignment && "alignment cannot be zero");
		MUU_CONSTEXPR_SAFE_ASSERT(((alignment & (alignment - 1u)) == 0u) && "alignment must be a power of two");
		if constexpr (!std::is_void_v<T>)
		{
			MUU_CONSTEXPR_SAFE_ASSERT(alignment >= alignof(T) && "cannot under-align types.");
		}

		return reinterpret_cast<T*>(apply_alignment(reinterpret_cast<uintptr_t>(ptr), alignment));
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
