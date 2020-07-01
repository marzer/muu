// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "../../muu/preprocessor.h"
#if !MUU_IMPLEMENTATION
	#error This is an implementation-only header.
#endif

#include "../../muu/blob.h"
#include "../../muu/aligned_alloc.h"

namespace MUU_INTERNAL_NAMESPACE
{
	[[nodiscard]]
	MUU_INTERNAL_LINKAGE
	size_t blob_check_alignment(size_t align) noexcept
	{
		using namespace ::muu;

		if (!align)
			return blob::default_alignment;

		MUU_ASSERT(
			align <= impl::aligned_alloc_max_alignment
			&& "Alignment must not be greater than impl::aligned_alloc_max_alignment (it will be clamped)."
		);
		MUU_ASSERT(
			has_single_bit(align)
			&& "Alignment must be a power of two (it will be rounded up)."
		);
		return bit_ceil(min(align, impl::aligned_alloc_max_alignment));
	}

	[[nodiscard]]
	MUU_INTERNAL_LINKAGE
	MUU_UNALIASED_ALLOC
	void* blob_allocate(size_t align, size_t size) noexcept
	{
		using namespace ::muu;

		MUU_ASSERT(align);
		MUU_ASSERT(has_single_bit(align));
		MUU_ASSERT(align <= impl::aligned_alloc_max_alignment);

		return size
			? muu::aligned_alloc(align, size)
			: nullptr;
	}
}

namespace muu
{
	MUU_EXTERNAL_LINKAGE
	blob::blob() noexcept
		: alignment_{ default_alignment }
	{}

	MUU_EXTERNAL_LINKAGE
	blob::blob(size_t sz, const void* src, size_t align) noexcept
		: alignment_{ MUU_INTERNAL_NAMESPACE::blob_check_alignment(align) },
		size_{ sz },
		data_{ MUU_INTERNAL_NAMESPACE::blob_allocate(alignment_, size_) }
	{
		if (data_ && src)
			memcpy(data_, src, size_);
	}

	MUU_EXTERNAL_LINKAGE
	blob::blob(const blob& other, size_t align) noexcept
		: blob{ other.size_, other.data_, align ? align : other.alignment_ }
	{ }

	MUU_EXTERNAL_LINKAGE
	blob::blob(const blob& other) noexcept
		: blob{ other.size_, other.data_, other.alignment_ }
	{ }

	MUU_EXTERNAL_LINKAGE
	blob::blob(blob&& other) noexcept
		: alignment_{ other.alignment_ },
		size_{ other.size_ },
		data_{ other.data_ }
	{
		other.alignment_ = default_alignment;
		other.size_ = 0;
		other.data_ = nullptr;
	}

	MUU_EXTERNAL_LINKAGE
	blob::~blob() noexcept
	{
		if (data_)
			muu::aligned_free(data_);
	}

	MUU_EXTERNAL_LINKAGE
	blob& blob::operator=(blob&& rhs) noexcept
	{
		if (&rhs != this)
		{
			if (data_)
				muu::aligned_free(data_);

			data_ = rhs.data_;
			size_ = rhs.size_;
			alignment_ = rhs.alignment_;

			rhs.alignment_ = default_alignment;
			rhs.size_ = 0;
			rhs.data_ = nullptr;
		}
		return *this;
	}

	MUU_EXTERNAL_LINKAGE
	blob& blob::assign(size_t sz, const void* src, size_t align) noexcept
	{
		MUU_USING_INTERNAL_NAMESPACE;

		align = blob_check_alignment(align);

		//check if this is effectively a resize with a copy or move
		if (alignment_ == align)
		{
			size(sz); //no-op if the same as current
			MUU_ASSERT(size_ == sz);
			if (src && data_ && data_ != src)
				memcpy(data_, src, size_);
			return *this;
		}

		//changing alignment, must deallocate and reallocate
		if (data_)
			muu::aligned_free(data_);
		alignment_ = align;
		size_ = sz;
		data_ = blob_allocate(alignment_, size_);
		if (src && data_)
			memcpy(data_, src, size_);
		return *this;
	}

	MUU_EXTERNAL_LINKAGE
	blob& blob::size(size_t sz) noexcept
	{
		MUU_USING_INTERNAL_NAMESPACE;

		if (size_ == sz)
			return *this;

		size_ = sz;

		//something -> nothing
		if (!size_)
		{
			MUU_ASSERT(data_);
			muu::aligned_free(data_);
			data_ = nullptr;
			return *this;
		}

		//something -> something
		if (data_)
		{
			data_ = muu::aligned_realloc(data_, size_);
			MUU_ASSERT(data_);

			return *this;
		}

		//nothing -> something
		MUU_ASSERT(!data_);
		data_ = blob_allocate(alignment_, size_);
		return *this;
	}
}
