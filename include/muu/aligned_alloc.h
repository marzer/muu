// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the implementation of #muu::aligned_alloc().

#include "bit_ceil.h"
#include "assume_aligned.h"
#include "impl/core_utils.h" // max
#include "impl/std_cstdlib.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;

namespace muu::impl
{
	inline constexpr size_t aligned_alloc_min_align =
		__STDCPP_DEFAULT_NEW_ALIGNMENT__ > 16u ? size_t{ __STDCPP_DEFAULT_NEW_ALIGNMENT__ } : size_t{ 16u };

	inline constexpr size_t aligned_alloc_big_threshold = 2048u;

	inline constexpr size_t aligned_alloc_big_align =
		aligned_alloc_min_align > 64u ? aligned_alloc_min_align : size_t{ 64 };

	MUU_CONST_GETTER
	constexpr size_t aligned_alloc_actual_align(size_t size, size_t alignment) noexcept
	{
		if (!alignment)
			return aligned_alloc_min_align;
		return muu::max(muu::bit_ceil(alignment),
						size >= aligned_alloc_big_threshold ? aligned_alloc_big_align : aligned_alloc_min_align);
	}
}

namespace muu
{
	/// \brief Allocates memory on a specific alignment boundary.
	/// \ingroup core
	MUU_NODISCARD
	MUU_MALLOC
	MUU_ATTR(assume_aligned(impl::aligned_alloc_min_align))
	inline void* aligned_alloc(size_t size, size_t alignment) noexcept
	{
		MUU_ASSERT(size > 0u);

		alignment = impl::aligned_alloc_actual_align(size, alignment);

#if MUU_WINDOWS

		const size_t offset = (alignment - 1u) + sizeof(void*);
		void* p1			= std::malloc(size + offset);
		if (!p1)
			return nullptr;

		void** p2 = reinterpret_cast<void**>((reinterpret_cast<uintptr_t>(p1) + offset) & ~(alignment - 1u));
		p2[-1]	  = p1;
		return muu::assume_aligned<impl::aligned_alloc_min_align>(p2);

#else
		return muu::assume_aligned<impl::aligned_alloc_min_align>(std::aligned_alloc(alignment, size));
#endif
	}

	/// \brief Frees memory that was allocated using #muu::aligned_alloc().
	/// \ingroup core
	///
	/// \warning Do not use this to free memory that was not allocated using #muu::aligned_alloc()!
	inline void aligned_free(void* ptr) noexcept
	{
		MUU_ASSERT(ptr != nullptr);

#if MUU_WINDOWS
		std::free((static_cast<void**>(ptr))[-1]);
#else
		std::free(ptr);
#endif
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
