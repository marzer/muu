//# This file is a part of muu and is subject to the the terms of the MIT license.
//# Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
//# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.

/// \file
/// \brief Contains aligned_alloc, aligned_realloc and aligned_free.
#pragma once
#include "../muu/common.h"

namespace muu
{
	[[nodiscard]]
	MUU_API
	MUU_UNALIASED_ALLOC
	void* aligned_alloc(size_t alignment, size_t size) noexcept;

	[[nodiscard]]
	MUU_API
	MUU_UNALIASED_ALLOC
	void* aligned_realloc(void* ptr, size_t new_size) noexcept;

	MUU_API
	void aligned_free(void* ptr) noexcept;
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
		if (!alignment || !size || !has_single_bit(alignment) || alignment > 8192_sz)
			return nullptr;

		impl::aligned_alloc_data data{
			alignment,
			max(alignment, impl::aligned_alloc_data_footprint),
			size
		};
		data.actual_size = data.actual_alignment
			+ ((size + data.actual_alignment - 1_sz) & ~(data.actual_alignment - 1_sz));

		auto priv_alloc = reinterpret_cast<byte*>(
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
		return reinterpret_cast<void*>(priv_alloc + data.actual_alignment);
	}

	MUU_POP_WARNINGS

	MUU_FUNC_EXTERNAL_LINKAGE
	MUU_API
	MUU_UNALIASED_ALLOC
	void* aligned_realloc(void* ptr, size_t new_size) noexcept
	{
		if (!new_size)
			return nullptr;
		if (!ptr)
			return aligned_alloc(__STDCPP_DEFAULT_NEW_ALIGNMENT__, new_size);

		auto data = *reinterpret_cast<impl::aligned_alloc_data*>(
			reinterpret_cast<byte*>(ptr) - impl::aligned_alloc_data_footprint
		);
		const auto new_actual_size = data.actual_alignment
			+ ((new_size + data.actual_alignment - 1_sz) & ~(data.actual_alignment - 1_sz));
		if (is_between(new_actual_size, data.actual_size / 2_sz, data.actual_size))
			return ptr;
		
		#ifdef _MSC_VER
		{
			ptr = _aligned_realloc(reinterpret_cast<byte*>(ptr) - data.actual_alignment,
				new_actual_size,
				data.actual_alignment
			);
			if (ptr)
			{
				data.actual_size = new_actual_size;
				memcpy(
					reinterpret_cast<byte*>(ptr) + data.actual_alignment - impl::aligned_alloc_data_footprint,
					&data,
					sizeof(impl::aligned_alloc_data)
				);
				return reinterpret_cast<byte*>(ptr) + data.actual_alignment;
			}
		}
		#else
		{
			if (auto new_ptr = aligned_alloc(data.requested_alignment, new_size))
			{
				memcpy(new_ptr, ptr, min(new_size, data.requested_size));
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
		if (!ptr)
			return;
		const auto& data = *reinterpret_cast<impl::aligned_alloc_data*>(
			reinterpret_cast<byte*>(ptr) - impl::aligned_alloc_data_footprint
		);

		#ifdef _MSC_VER
			_aligned_free(reinterpret_cast<byte*>(ptr) - data.actual_alignment);
		#else
			std::free(reinterpret_cast<byte*>(ptr) - data.actual_alignment);
		#endif
	}
}

#endif //MUU_IMPLEMENTATION
