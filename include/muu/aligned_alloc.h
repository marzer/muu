// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief Contains the definitions of functions for performing (re)allocations on specific alignment boundaries.

#pragma once
#include "../muu/fwd.h"

MUU_NAMESPACE_START
{
	/// \addtogroup		mem
	/// @{

	/// \brief	Allocates memory with a specific alignment boundary.
	///
	/// \param 	alignment	The desired alignment. Must be a power of 2.
	/// \param 	size	 	The desired allocation size.
	///
	/// \returns	A pointer to the beginning of the allocated memory, or `nullptr` if:
	/// 			- `alignment` or `size` were `0`  
	/// 			- `alignment` was not a power of 2  
	/// 			- `alignment` was too high  
	/// 			- The system could not provide the requested allocation
	[[nodiscard]]
	MUU_API
	MUU_UNALIASED_ALLOC
	void* aligned_alloc(size_t alignment, size_t size) noexcept;

	/// \brief	Resizes memory previously allocated with a specific alignment boundary.
	/// 
	/// \detail Allocations returned by muu::aligned_realloc will have the same alignment
	/// 		 as was initially requested by muu::aligned_alloc.
	///	
	///			Calling muu::aligned_realloc with `nullptr` for `ptr`
	/// 		is equivalent to calling muu::aligned_alloc with an alignment of `__STDCPP_DEFAULT_NEW_ALIGNMENT__`.
	///
	/// \warning Do not use this to reallocate memory allocated from any source,
	/// 		 not even your implementation's `std::aligned_alloc`!
	/// 
	/// \param 	ptr			An allocation previously returned from muu::aligned_alloc or muu::aligned_realloc.
	/// \param 	new_size	The new desired size of the allocation.
	///
	/// \returns	A (possibly-relocated) pointer to the beginning of the reallocated memory, or `nullptr` if
	/// 			`new_size` was `0` or the system could not provide the requested allocation.
	/// 			Null return values leave the original input allocation unchanged.
	[[nodiscard]]
	MUU_API
	MUU_UNALIASED_ALLOC
	void* aligned_realloc(void* ptr, size_t new_size) noexcept;


	/// \brief	Frees memory previously allocated with muu::aligned_alloc.
	///
	/// \warning Do not use this to free memory allocated from any other source,
	/// 		 not even your implementation's `std::aligned_alloc`!
	/// 
	/// \param 	ptr	The pointer to the allocation being freed.
	MUU_API
	void aligned_free(void* ptr) noexcept;

	/// @}

	namespace impl
	{
		inline constexpr size_t aligned_alloc_max_alignment = 32768;
	}
}
MUU_NAMESPACE_END

#if MUU_IMPLEMENTATION
	#include "../muu/impl/aligned_alloc.hpp"
#endif
