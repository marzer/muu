// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "muu/generic_allocator.h"
#include "os.h"
#include "source_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;

namespace
{
	struct MUU_EMPTY_BASES default_allocator final : generic_allocator
	{
		void* allocate(size_t size, size_t alignment) override
		{
			MUU_ASSERT(size > 0u);
			MUU_ASSERT(alignment > 0u);
			MUU_ASSERT((alignment & (alignment - 1u)) == 0u);

			MUU_ASSUME(size > 0u);
			MUU_ASSUME(alignment > 0u);
			MUU_ASSUME((alignment & (alignment - 1u)) == 0u);

			if (alignment <= __STDCPP_DEFAULT_NEW_ALIGNMENT__)
			{
				return new std::byte[size];
			}
			else
			{
#if MUU_WINDOWS
				void* ptr = _aligned_malloc(size, alignment);
#else
				void* ptr = std::aligned_alloc(alignment, size);
#endif

#if MUU_HAS_EXCEPTIONS
				if (!ptr)
					throw std::bad_alloc{};
#endif

#ifndef NDEBUG
				if (ptr)
					MUU_ASSERT(reinterpret_cast<uintptr_t>(ptr) % alignment == 0u);
#endif

				return ptr;
			}
		}

		void deallocate(void* ptr, size_t size, size_t alignment) noexcept override
		{
			MUU_ASSERT(ptr);
			MUU_ASSERT(size > 0u);
			MUU_ASSERT(alignment > 0u);
			MUU_ASSERT((alignment & (alignment - 1u)) == 0u);

			MUU_ASSUME(ptr);
			MUU_ASSUME(size > 0u);
			MUU_ASSUME(alignment > 0u);
			MUU_ASSUME((alignment & (alignment - 1u)) == 0u);

			MUU_UNUSED(size);

			if (alignment <= __STDCPP_DEFAULT_NEW_ALIGNMENT__)
			{
				delete[] static_cast<std::byte*>(ptr);
			}
			else
			{
#if MUU_WINDOWS
				_aligned_free(ptr);
#else
				std::free(ptr);
#endif
			}
		}
	};
}

MUU_DISABLE_LIFETIME_WARNINGS;

generic_allocator& impl::get_default_allocator() noexcept
{
	static ::default_allocator allocator;
	return allocator;
}
