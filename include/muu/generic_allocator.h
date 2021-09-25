// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the definition of muu::generic_allocator.

#include "impl/aligned_alloc.h"
#include "impl/header_start.h"

namespace muu
{
	/// \brief An interface for encapsulating generic allocators.
	/// \ingroup memory
	struct generic_allocator
	{
		/// \brief Allocation callback.
		///
		/// \param 	data	 	The data member of the generic_allocator being used.
		/// \param 	size	 	The size of the requested allocation.
		/// \param 	alignment	The required alignment. Must be a power of two.
		///
		/// \returns A pointer to the new allocation, or `nullptr`.
		using allocate_func = void*(MUU_CALLCONV*)(void* data, size_t size, size_t alignment) noexcept;

		/// \brief Deallocation callback.
		///
		/// \param 	data	 	The data member of the generic_allocator being used.
		/// \param 	ptr	 		The memory being deallocated.
		using deallocate_func = void(MUU_CALLCONV*)(void* data, void* ptr) noexcept;

		/// \brief Generic data tag to pass to allocation and deallocation callbacks.
		void* data;

		/// \brief	Pointer to a function which allocates memory.
		allocate_func allocate;

		/// \brief	Pointer to a function which deallocates memory.
		deallocate_func deallocate;
	};

	/// \cond
	namespace impl
	{
		MUU_NODISCARD
		MUU_UNALIASED_ALLOC
		MUU_ATTR(assume_aligned(aligned_alloc_min_align))
		inline void* generic_alloc(generic_allocator* alloc, size_t size, size_t alignment) noexcept
		{
			if (!alloc)
				return muu::impl::aligned_alloc(size, alignment);
			return alloc->allocate(alloc->data, size, aligned_alloc_actual_align(size, alignment));
		}

		inline void generic_free(generic_allocator* alloc, void* ptr) noexcept
		{
			if (!alloc)
				muu::impl::aligned_free(ptr);
			else
				alloc->deallocate(alloc->data, ptr);
		}
	}
	/// \endcond
}

#include "impl/header_end.h"
