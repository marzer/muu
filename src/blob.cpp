// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "muu/core.h"
#include "muu/blob.h"

MUU_DISABLE_WARNINGS;
#if !MUU_HAS_EXCEPTIONS
	#include <exception> // std::terminate()
#endif
MUU_ENABLE_WARNINGS;

#include "source_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;

namespace
{
	static constexpr size_t default_blob_alignment = size_t{ __STDCPP_DEFAULT_NEW_ALIGNMENT__ };

	MUU_NODISCARD
	MUU_ATTR(pure)
	constexpr size_t blob_check_alignment(size_t align) noexcept
	{
		if (!align)
			return default_blob_alignment;
		return bit_ceil(align);
	}

	MUU_NODISCARD
	MUU_UNALIASED_ALLOC
	static void* blob_allocate(generic_allocator& alloc, size_t size, size_t align) noexcept(!build::has_exceptions)
	{
		MUU_ASSERT(align);
		MUU_ASSERT(has_single_bit(align));

		if (size)
		{
			auto ptr = alloc.allocate(size, max(size_t{ __STDCPP_DEFAULT_NEW_ALIGNMENT__ }, align));
			MUU_ASSERT(ptr && "allocate() failed!");
#if !MUU_HAS_EXCEPTIONS
			{
				if (!ptr)
					std::terminate();
			}
#endif
			return ptr;
		}
		else
			return nullptr;
	}
}

blob::blob(generic_allocator* alloc) noexcept
	: allocator_{ alloc ? alloc : ::muu_impl_get_default_allocator() },
	  alignment_{ default_blob_alignment }
{}

blob::blob(size_t sz, const void* src, size_t align, generic_allocator* alloc)
	: allocator_{ alloc ? alloc : ::muu_impl_get_default_allocator() },
	  alignment_{ blob_check_alignment(align) },
	  size_{ sz },
	  data_{ blob_allocate(*allocator_, size_, alignment_) }
{
	if (data_ && src)
		std::memcpy(data_, src, size_);
}

blob::blob(const blob& other, size_t align, generic_allocator* alloc)
	: blob{ other.size_, other.data_, align ? align : other.alignment_, alloc ? alloc : other.allocator_ }
{}

blob::blob(const blob& other) : blob{ other.size_, other.data_, other.alignment_, other.allocator_ }
{}

blob::blob(blob&& other) noexcept
	: allocator_{ other.allocator_ },
	  alignment_{ other.alignment_ },
	  size_{ other.size_ },
	  data_{ other.data_ }
{
	other.alignment_ = default_blob_alignment;
	other.size_		 = 0;
	other.data_		 = nullptr;
}

blob::~blob() noexcept
{
	if (auto data = std::exchange(data_, nullptr))
		allocator_->deallocate(data);
}

blob& blob::operator=(blob&& rhs) noexcept
{
	if (&rhs != this)
	{
		if (data_)
			allocator_->deallocate(data_);

		allocator_ = rhs.allocator_;
		data_	   = rhs.data_;
		size_	   = rhs.size_;
		alignment_ = rhs.alignment_;

		rhs.alignment_ = default_blob_alignment;
		rhs.size_	   = 0;
		rhs.data_	   = nullptr;
	}
	return *this;
}

blob& blob::assign(size_t sz, const void* src, size_t align, generic_allocator* alloc)
{
	align = blob_check_alignment(align);
	if (!alloc)
		alloc = allocator_;

	// check if this is effectively a resize with a copy or move
	if (align == alignment_ && alloc == allocator_)
	{
		size(sz); // no-op if the same as current
		MUU_ASSERT(size_ == sz);
		if (src && data_ && data_ != src)
			std::memcpy(data_, src, size_);
		return *this;
	}

	// changing alignment or allocator; must deallocate and reallocate
	if (data_)
		allocator_->deallocate(data_);
	allocator_ = alloc;
	alignment_ = align;
	size_	   = sz;
	data_	   = blob_allocate(*allocator_, size_, alignment_);
	if (src && data_)
		std::memcpy(data_, src, size_);
	return *this;
}

blob& blob::size(size_t sz)
{
	if (size_ == sz)
		return *this;

	// something -> nothing
	if (!size_)
	{
		MUU_ASSERT(data_);
		allocator_->deallocate(data_);
		data_ = nullptr;
	}

	// something -> something
	else if (data_)
	{
		auto new_data = blob_allocate(*allocator_, sz, alignment_);
		std::memcpy(new_data, data_, min(sz, size_));
		allocator_->deallocate(data_);
		data_ = new_data;
	}

	// nothing -> something
	else
		data_ = blob_allocate(*allocator_, sz, alignment_);

	size_ = sz;
	return *this;
}
