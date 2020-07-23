// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "../../muu/preprocessor.h"
#if !MUU_IMPLEMENTATION
	#error This is an implementation-only header.
#endif

#include "../../muu/core.h"
#include "../../muu/aligned_alloc.h"
#if !MUU_MSVC
	#include <cstdlib>
#endif

MUU_ANON_NAMESPACE_START
{
	struct aligned_alloc_data final
	{
		size_t requested_alignment;
		size_t actual_alignment;
		size_t requested_size;
		size_t actual_size; //as passed to malloc etc.
	};
	inline constexpr size_t aligned_alloc_data_footprint = (muu::max)(
		muu::bit_ceil(sizeof(aligned_alloc_data)),
		alignof(aligned_alloc_data)
	);
	static_assert(muu::has_single_bit(aligned_alloc_data_footprint));
}
MUU_ANON_NAMESPACE_END

MUU_NAMESPACE_START
{
	MUU_PUSH_WARNINGS
	MUU_DISABLE_INIT_WARNINGS

	MUU_EXTERNAL_LINKAGE
	MUU_API
	MUU_UNALIASED_ALLOC
	void* aligned_alloc(size_t alignment, size_t size) noexcept
	{
		MUU_USING_ANON_NAMESPACE;
		using std::byte;

		if (!alignment || !size || !has_single_bit(alignment) || alignment > impl::aligned_alloc_max_alignment)
			return nullptr;

		aligned_alloc_data data{
			alignment,
			(max)(alignment, aligned_alloc_data_footprint),
			size
		};
		data.actual_size = data.actual_alignment
			+ ((size + data.actual_alignment - 1_sz) & ~(data.actual_alignment - 1_sz));

		auto priv_alloc = pointer_cast<byte*>(
			#if MUU_MSVC
				_aligned_malloc(data.actual_size, data.actual_alignment)
			#else
				std::aligned_alloc(data.actual_alignment, data.actual_size)
			#endif
		);

		memcpy(
			priv_alloc + data.actual_alignment - aligned_alloc_data_footprint,
			&data,
			sizeof(aligned_alloc_data)
		);
		return pointer_cast<void*>(priv_alloc + data.actual_alignment);
	}

	MUU_POP_WARNINGS

	MUU_EXTERNAL_LINKAGE
	MUU_API
	MUU_UNALIASED_ALLOC
	void* aligned_realloc(void* ptr, size_t new_size) noexcept
	{
		MUU_USING_ANON_NAMESPACE;
		using std::byte;

		if (!new_size)
			return nullptr;
		if (!ptr)
			return aligned_alloc(__STDCPP_DEFAULT_NEW_ALIGNMENT__, new_size);

		auto data = *pointer_cast<aligned_alloc_data*>(
			pointer_cast<byte*>(ptr) - aligned_alloc_data_footprint
		);
		const auto new_actual_size = data.actual_alignment
			+ ((new_size + data.actual_alignment - 1_sz) & ~(data.actual_alignment - 1_sz));
		if (between(new_actual_size, data.actual_size / 2_sz, data.actual_size))
			return ptr;
		
		#if MUU_MSVC
		{
			ptr = _aligned_realloc(pointer_cast<byte*>(ptr) - data.actual_alignment,
				new_actual_size,
				data.actual_alignment
			);
			if (ptr)
			{
				data.actual_size = new_actual_size;
				memcpy(
					pointer_cast<byte*>(ptr) + data.actual_alignment - aligned_alloc_data_footprint,
					&data,
					sizeof(aligned_alloc_data)
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

	MUU_EXTERNAL_LINKAGE
	MUU_API
	void aligned_free(void* ptr) noexcept
	{
		MUU_USING_ANON_NAMESPACE;
		using std::byte;

		if (!ptr)
			return;
		const auto& data = *pointer_cast<aligned_alloc_data*>(
			pointer_cast<byte*>(ptr) - aligned_alloc_data_footprint
		);

		#if MUU_MSVC
			_aligned_free(pointer_cast<byte*>(ptr) - data.actual_alignment);
		#else
			std::free(pointer_cast<byte*>(ptr) - data.actual_alignment);
		#endif
	}
}
MUU_NAMESPACE_END
