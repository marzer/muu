// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "muu/core.h"
#include "muu/aligned_alloc.h"
#if !(MUU_MSVC || MUU_ICC_CL)
	#include <cstdlib>
#endif

MUU_DISABLE_SPAM_WARNINGS
using namespace muu;

namespace
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

MUU_NAMESPACE_START
{
	MUU_UNALIASED_ALLOC
	void* aligned_alloc(size_t alignment, size_t size) noexcept
	{
		if (!alignment || !size || !has_single_bit(alignment) || alignment > impl::aligned_alloc_max_alignment)
			return nullptr;

		const auto actual_alignment = (max)(alignment, aligned_alloc_data_footprint);
		aligned_alloc_data data{
			alignment,
			actual_alignment,
			size,
			actual_alignment + ((size + actual_alignment - 1u) & ~(actual_alignment - 1u))
		};
		auto priv_alloc = pointer_cast<std::byte*>(
			#if MUU_MSVC || MUU_ICC_CL
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

	MUU_UNALIASED_ALLOC
	void* aligned_realloc(void* ptr, size_t new_size) noexcept
	{
		if (!new_size)
			return nullptr;
		if (!ptr)
			return muu::aligned_alloc(__STDCPP_DEFAULT_NEW_ALIGNMENT__, new_size);

		auto data = *pointer_cast<aligned_alloc_data*>(
			pointer_cast<std::byte*>(ptr) - aligned_alloc_data_footprint
		);
		const auto new_actual_size = data.actual_alignment
			+ ((new_size + data.actual_alignment - 1u) & ~(data.actual_alignment - 1u));
		if (is_between(new_actual_size, data.actual_size / 2u, data.actual_size))
			return ptr;

		#if MUU_MSVC || MUU_ICC_CL
		{
			ptr = _aligned_realloc(pointer_cast<std::byte*>(ptr) - data.actual_alignment,
				new_actual_size,
				data.actual_alignment
			);
			if (ptr)
			{
				data.actual_size = new_actual_size;
				memcpy(
					pointer_cast<std::byte*>(ptr) + data.actual_alignment - aligned_alloc_data_footprint,
					&data,
					sizeof(aligned_alloc_data)
				);
				return pointer_cast<std::byte*>(ptr) + data.actual_alignment;
			}
		}
		#else
		{
			if (auto new_ptr = aligned_alloc(data.requested_alignment, new_size))
			{
				memcpy(new_ptr, ptr, min(new_size, data.requested_size));
				muu::aligned_free(ptr);
				return new_ptr;
			}
		}
		#endif

		return nullptr;
	}

	void aligned_free(void* ptr) noexcept
	{
		if (!ptr)
			return;
		const auto& data = *pointer_cast<aligned_alloc_data*>(
			pointer_cast<std::byte*>(ptr) - aligned_alloc_data_footprint
		);

		#if MUU_MSVC || MUU_ICC_CL
			_aligned_free(pointer_cast<std::byte*>(ptr) - data.actual_alignment);
		#else
			std::free(pointer_cast<std::byte*>(ptr) - data.actual_alignment);
		#endif
	}
}
MUU_NAMESPACE_END
