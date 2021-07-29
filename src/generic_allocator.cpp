// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "muu/generic_allocator.h"
#include "os.h"
#include "source_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;

using namespace muu;

namespace
{
	struct MUU_EMPTY_BASES default_allocator final : muu::generic_allocator
	{
		void* allocate(size_t size, size_t alignment) override
		{
			alignment = muu::max(alignment, size_t{ __STDCPP_DEFAULT_NEW_ALIGNMENT__ });

			MUU_ASSERT(size > 0u);
			MUU_ASSERT(alignment > 0u);
			MUU_ASSERT((alignment & (alignment - 1u)) == 0u);

			MUU_ASSUME(size > 0u);
			MUU_ASSUME(alignment > 0u);
			MUU_ASSUME((alignment & (alignment - 1u)) == 0u);

#if MUU_WINDOWS

			const size_t offset = (alignment - 1u) + sizeof(void*);
			void* p1			= std::malloc(size + offset);
			if (!p1)
				return nullptr;

			void** p2 = reinterpret_cast<void**>((reinterpret_cast<uintptr_t>(p1) + offset) & ~(alignment - 1u));
			p2[-1]	  = p1;
			return p2;

#else
			return std::aligned_alloc(alignment, size);
#endif
		}

		void deallocate(void* ptr) noexcept override
		{
			MUU_ASSERT(ptr);
			MUU_ASSUME(ptr);

#if MUU_WINDOWS
			std::free((static_cast<void**>(ptr))[-1]);
#else
			std::free(ptr);
#endif
		}
	};
}

MUU_DISABLE_LIFETIME_WARNINGS;

namespace muu::impl
{
	generic_allocator& get_default_allocator() noexcept
	{
		static ::default_allocator allocator;
		return allocator;
	}
}
