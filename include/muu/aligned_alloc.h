// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "../muu/common.h"


namespace muu
{
	/// \addtogroup		aligned_alloc		Aligned allocation
	/// \brief Functions for performing (re)allocations on specific alignment boundaries.
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
	/// \warning Do not use this to reallocate memory allocated from any source,
	/// 		 not even your implementation's `std::aligned_alloc`!
	/// 
	/// \param 	ptr			An allocation previously returned from muu::aligned_alloc or muu::aligned_realloc.
	/// \param 	new_size	The new desired size of the allocation.
	///
	/// \returns	A (possibly-relocated) pointer to the beginning of the reallocated memory, or `nullptr` if
	/// 			`new_size` was `0` or the system could not provide the requested allocation.
	/// 			Null return values leave the original input allocation unchanged.
	/// 
	/// \remark Allocations returned by muu::aligned_realloc will have the same alignment
	/// 		 as was initially requested by muu::aligned_alloc.
	///	
	/// \remark Calling muu::aligned_realloc with `nullptr` for `ptr`
	/// 		is equivalent to calling muu::aligned_alloc with an alignment of `__STDCPP_DEFAULT_NEW_ALIGNMENT__`.
	/// 
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
}

namespace muu::impl
{
	inline constexpr size_t aligned_alloc_max_alignment = 32768;
}

#if MUU_IMPLEMENTATION

#ifndef _MSC_VER
	#include <cstdlib>
#endif

namespace muu::impl
{
	struct aligned_alloc_data final
	{
		size_t requested_alignment;
		size_t actual_alignment;
		size_t requested_size;
		size_t actual_size; //as passed to malloc etc.
	};
	inline constexpr size_t aligned_alloc_data_footprint = max(
		bit_ceil(sizeof(aligned_alloc_data)),
		alignof(aligned_alloc_data)
	);
	static_assert(has_single_bit(aligned_alloc_data_footprint));
}

namespace muu
{
	MUU_PUSH_WARNINGS
	MUU_DISABLE_INIT_WARNINGS

	MUU_FUNC_EXTERNAL_LINKAGE
	MUU_API
	MUU_UNALIASED_ALLOC
	void* aligned_alloc(size_t alignment, size_t size) noexcept
	{
		using std::byte;

		if (!alignment || !size || !has_single_bit(alignment) || alignment > impl::aligned_alloc_max_alignment)
			return nullptr;

		impl::aligned_alloc_data data{
			alignment,
			max(alignment, impl::aligned_alloc_data_footprint),
			size
		};
		data.actual_size = data.actual_alignment
			+ ((size + data.actual_alignment - 1_sz) & ~(data.actual_alignment - 1_sz));

		auto priv_alloc = pointer_cast<byte*>(
			#ifdef _MSC_VER
				_aligned_malloc(data.actual_size, data.actual_alignment)
			#else
				std::aligned_alloc(data.actual_alignment, data.actual_size)
			#endif
		);

		memcpy(
			priv_alloc + data.actual_alignment - impl::aligned_alloc_data_footprint,
			&data,
			sizeof(impl::aligned_alloc_data)
		);
		return pointer_cast<void*>(priv_alloc + data.actual_alignment);
	}

	MUU_POP_WARNINGS

	MUU_FUNC_EXTERNAL_LINKAGE
	MUU_API
	MUU_UNALIASED_ALLOC
	void* aligned_realloc(void* ptr, size_t new_size) noexcept
	{
		using std::byte;

		if (!new_size)
			return nullptr;
		if (!ptr)
			return aligned_alloc(__STDCPP_DEFAULT_NEW_ALIGNMENT__, new_size);

		auto data = *pointer_cast<impl::aligned_alloc_data*>(
			pointer_cast<byte*>(ptr) - impl::aligned_alloc_data_footprint
		);
		const auto new_actual_size = data.actual_alignment
			+ ((new_size + data.actual_alignment - 1_sz) & ~(data.actual_alignment - 1_sz));
		if (is_between(new_actual_size, data.actual_size / 2_sz, data.actual_size))
			return ptr;
		
		#ifdef _MSC_VER
		{
			ptr = _aligned_realloc(pointer_cast<byte*>(ptr) - data.actual_alignment,
				new_actual_size,
				data.actual_alignment
			);
			if (ptr)
			{
				data.actual_size = new_actual_size;
				memcpy(
					pointer_cast<byte*>(ptr) + data.actual_alignment - impl::aligned_alloc_data_footprint,
					&data,
					sizeof(impl::aligned_alloc_data)
				);
				return pointer_cast<byte*>(ptr) + data.actual_alignment;
			}
		}
		#else
		{
			if (auto new_ptr = aligned_alloc(data.requested_alignment, new_size))
			{
				memcpy(new_ptr, ptr, min(new_size, data.requested_size));
				aligned_free(ptr);
				return new_ptr;
			}
		}
		#endif

		return nullptr;
	}

	MUU_FUNC_EXTERNAL_LINKAGE
	MUU_API
	void aligned_free(void* ptr) noexcept
	{
		using std::byte;

		if (!ptr)
			return;
		const auto& data = *pointer_cast<impl::aligned_alloc_data*>(
			pointer_cast<byte*>(ptr) - impl::aligned_alloc_data_footprint
		);

		#ifdef _MSC_VER
			_aligned_free(pointer_cast<byte*>(ptr) - data.actual_alignment);
		#else
			std::free(pointer_cast<byte*>(ptr) - data.actual_alignment);
		#endif
	}
}

#endif //MUU_IMPLEMENTATION
