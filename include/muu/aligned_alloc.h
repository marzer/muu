// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the definition of #muu::aligned_alloc().

#include "assume_aligned.h"
#include "impl/std_cstdlib.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;


namespace muu
{
	//% aligned_alloc start

	namespace impl
	{
		inline constexpr size_t aligned_alloc_min_align =
			__STDCPP_DEFAULT_NEW_ALIGNMENT__ > 16u ? size_t{ __STDCPP_DEFAULT_NEW_ALIGNMENT__ } : size_t{ 16u };
	}

	/// \brief Allocates memory on a specific alignment boundary.
	/// \ingroup core
	MUU_NODISCARD
	MUU_DECLSPEC(noalias)
	MUU_DECLSPEC(restrict)
	MUU_ATTR(assume_aligned(impl::aligned_alloc_min_align))
	inline void* aligned_alloc(size_t size, size_t alignment) noexcept
	{
		alignment = alignment > impl::aligned_alloc_min_align ? alignment : impl::aligned_alloc_min_align;

#if MUU_WINDOWS
		return muu::assume_aligned<impl::aligned_alloc_min_align>(_aligned_malloc(size, alignment));
#else
		return muu::assume_aligned<impl::aligned_alloc_min_align>(std::aligned_alloc(alignment, size));
#endif
	}

	/// \brief Frees memory that was allocated using #muu::aligned_alloc().
	/// \ingroup core
	///
	/// \warning Do not use this to free memory that was not allocated using #muu::aligned_alloc()!
	MUU_ALWAYS_INLINE
	inline void aligned_free(void* ptr) noexcept
	{
#if MUU_WINDOWS
		_aligned_free(ptr);
#else
		std::free(ptr);
#endif
	}

	//% aligned_alloc end
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
