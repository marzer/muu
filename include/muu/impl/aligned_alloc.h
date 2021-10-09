// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

#include "../bit.h"
#include "core_utils.h"
#if !MUU_WINDOWS
	#include "std_cstdlib.h"
#endif
#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;

/// \cond
namespace muu::impl
{
	inline constexpr size_t aligned_alloc_min_align		= muu::max<size_t>(__STDCPP_DEFAULT_NEW_ALIGNMENT__, 16);
	inline constexpr size_t aligned_alloc_big_threshold = 2048;
	inline constexpr size_t aligned_alloc_big_align		= muu::max<size_t>(aligned_alloc_min_align, 64);

	MUU_NODISCARD
	MUU_ATTR(const)
	constexpr size_t aligned_alloc_actual_align(size_t size, size_t alignment) noexcept
	{
		if (!alignment)
			return aligned_alloc_min_align;
		return muu::max(muu::bit_ceil(alignment),
						size >= aligned_alloc_big_threshold ? aligned_alloc_big_align : aligned_alloc_min_align);
	}

	MUU_NODISCARD
	MUU_UNALIASED_ALLOC
	MUU_ATTR(assume_aligned(aligned_alloc_min_align))
	inline void* aligned_alloc(size_t size, size_t alignment) noexcept
	{
		MUU_ASSERT(size > 0u);

		alignment = aligned_alloc_actual_align(size, alignment);

#if MUU_WINDOWS

		const size_t offset = (alignment - 1u) + sizeof(void*);
		void* p1			= std::malloc(size + offset);
		if (!p1)
			return nullptr;

		void** p2 = reinterpret_cast<void**>((reinterpret_cast<uintptr_t>(p1) + offset) & ~(alignment - 1u));
		p2[-1]	  = p1;
		return muu::assume_aligned<aligned_alloc_min_align>(p2);

#else
		return muu::assume_aligned<aligned_alloc_min_align>(std::aligned_alloc(alignment, size));
#endif
	}

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
/// \endcond

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
