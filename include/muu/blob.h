// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::blob.
#pragma once
#include "../muu/common.h"

namespace muu
{
	/// \brief Interface for managing chunks of memory.
	/// 
	/// \detail A blob is effectively an RAII wrapper around muu::aligned_alloc, muu::aligned_realloc and muu::aligned_free,
	/// 		intended to be used anywhere you might previously have used something like std::vector<std::byte>.
	/// 		
	class MUU_API blob
	{
		private:
			size_t alignment_, size_ = 0;
			void* data_ = nullptr;

		public:

			/// \brief	The default alignment of a blob's allocated memory if no value is provided.
			static constexpr size_t default_alignment = __STDCPP_DEFAULT_NEW_ALIGNMENT__;

			/// \brief Creates an empty blob.
			MUU_NODISCARD_CTOR
			blob() noexcept;

			/// \brief Creates a blob of a fixed size and alignment.
			/// 
			/// \param	size	The size of the blob's data, in bytes.
			/// \param	src	 	The source data to copy, if any.
			/// \param	align	The alignment of the blob's data.
			MUU_NODISCARD_CTOR
			blob(size_t size, const void* src = nullptr, size_t align = {}) noexcept;

			/// \brief Copy constructor.
			MUU_NODISCARD_CTOR
			blob(const blob& other) noexcept;

			/// \brief Copy constructor.
			/// 
			/// \param	other		The blob to copy.
			/// \param	align		The alignment of the blob's data. Set to `0` to copy the source alignment.
			MUU_NODISCARD_CTOR
			blob(const blob& other, size_t align) noexcept;

			/// \brief Move constructor.
			MUU_NODISCARD_CTOR
			blob(blob&& other) noexcept;

			/// \brief Destructor.
			~blob() noexcept;

			/// Replaces the contents of the blob with the given data.
			/// 
			/// \param	src	 	The data to copy.
			/// \param	size 	The size of the data.
			/// \param	align	The alignment to use.
			/// 
			/// \return	A reference to the input blob.
			blob& assign(size_t size, const void* src, size_t align = {}) noexcept;

			/// \brief Replaces the contents of the blob by copying from another.
			/// 
			/// \remarks Alignment will be made to match the source blob; use assign() if you wish to be explicit
			/// about alignment when copying another blob.
			/// 
			/// \param	rhs	The blob to copy.
			/// 
			/// \return	A reference to the LHS blob.
			blob& operator=(const blob& rhs) noexcept
			{
				return assign(rhs.size_, rhs.data_, rhs.alignment_);
			}

			/// \brief Move-assignment operator.
			blob& operator=(blob&&) noexcept;

			/// \brief Returns the size of the blob's data, in bytes.
			[[nodiscard]]
			size_t size() const noexcept
			{
				return size_;
			}

			/// \brief Resizes the blob, keeping the span of `min(newSize, oldSize)` intact. 
			/// 
			/// \param	size New size.
			/// 
			/// \return	A reference to the blob.
			blob& size(size_t size) noexcept;

			/// \brief Returns the alignment of the blob's data, in bytes.
			[[nodiscard]]
			size_t alignment() const noexcept
			{
				return alignment_;
			}

			/// \brief Returns a pointer to the blob's data.
			[[nodiscard]]
			std::byte* data() noexcept
			{
				return pointer_cast<std::byte*>(data_);
			}

			/// \brief Returns a pointer to the blob's data (const overload).
			[[nodiscard]]
			const std::byte* data() const noexcept
			{
				return pointer_cast<const std::byte*>(data_);
			}

			/// \brief Returns true if the blob contains data.
			[[nodiscard]]
			explicit operator bool () const noexcept
			{
				return data_ != nullptr;
			}
	};
}

#if MUU_IMPLEMENTATION

#include "../muu/aligned_alloc.h"

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
		align = MUU_INTERNAL_NAMESPACE::blob_check_alignment(align);

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
		data_ = MUU_INTERNAL_NAMESPACE::blob_allocate(alignment_, size_);
		if (src && data_)
			memcpy(data_, src, size_);
		return *this;
	}

	MUU_EXTERNAL_LINKAGE
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
		data_ = MUU_INTERNAL_NAMESPACE::blob_allocate(alignment_, size_);
		return *this;
	}
}

#endif //MUU_IMPLEMENTATION
