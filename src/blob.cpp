// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "muu/core.h"
#include "muu/blob.h"
#include "muu/aligned_alloc.h"

using namespace muu;

namespace
{
	[[nodiscard]]
	MUU_ATTR(pure)
	constexpr size_t blob_check_alignment(size_t align) noexcept
	{
		if (!align)
			return blob::default_alignment;
		return bit_ceil(muu::min(align, impl::aligned_alloc_max_alignment));
	}

	[[nodiscard]]
	MUU_UNALIASED_ALLOC
	static void* blob_allocate(size_t align, size_t size) noexcept
	{
		MUU_ASSERT(align);
		MUU_ASSERT(has_single_bit(align));
		MUU_ASSERT(align <= impl::aligned_alloc_max_alignment);

		return size
			? muu::aligned_alloc(align, size)
			: nullptr;
	}
}

blob::blob() noexcept
	: alignment_{ default_alignment }
{}

blob::blob(size_t sz, const void* src, size_t align) noexcept
	: alignment_{ blob_check_alignment(align) },
	size_{ sz },
	data_{ blob_allocate(alignment_, size_) }
{
	if (data_ && src)
		memcpy(data_, src, size_);
}

blob::blob(const blob& other, size_t align) noexcept
	: blob{ other.size_, other.data_, align ? align : other.alignment_ }
{ }

blob::blob(const blob& other) noexcept
	: blob{ other.size_, other.data_, other.alignment_ }
{ }

blob::blob(blob&& other) noexcept
	: alignment_{ other.alignment_ },
	size_{ other.size_ },
	data_{ other.data_ }
{
	other.alignment_ = default_alignment;
	other.size_ = 0;
	other.data_ = nullptr;
}

blob::~blob() noexcept
{
	if (data_)
		muu::aligned_free(data_);
}

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

blob& blob::assign(size_t sz, const void* src, size_t align) noexcept
{
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

blob& blob::size(size_t sz) noexcept
{
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
