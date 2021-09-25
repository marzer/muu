// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the definition of muu::blob.

#include "impl/std_memcpy.h"
#include "impl/std_utility.h"
#include "impl/core_bit.h"
#include "generic_allocator.h"
#include "impl/header_start.h"

namespace muu
{
	/// \brief Interface for managing chunks of memory.
	/// \ingroup memory
	///
	/// \details A blob is an RAII wrapper around a memory allocation intended to be used anywhere
	/// 		 you might previously have used something like std::vector<std::byte> or std::unique_ptr<std::byte[]>.
	///
	class blob
	{
	  private:
		generic_allocator* allocator_;
		size_t alignment_;
		size_t size_ = 0;
		void* data_	 = nullptr;

	  public:
		/// \brief Creates an empty blob.
		///
		/// \param	allocator 	The #muu::generic_allocator used for allocations.
		///						Leave as `nullptr` to use the default global allocator.
		MUU_NODISCARD_CTOR
		explicit blob(generic_allocator* allocator = nullptr) noexcept
			: allocator_{ allocator },
			  alignment_{ impl::aligned_alloc_min_align }
		{}

		/// \brief Creates a blob of a fixed size and alignment.
		///
		/// \param	size		The size of the blob's data, in bytes.
		/// \param	src	 		The source data to copy, if any.
		/// \param	align		The alignment of the blob's data.
		///						Leave as `0` to use `__STDCPP_DEFAULT_NEW_ALIGNMENT__`.
		/// \param	allocator 	The #muu::generic_allocator used for allocations.
		///						Leave as `nullptr` to use the default global allocator.
		MUU_NODISCARD_CTOR
		explicit blob(size_t size, const void* src = nullptr, size_t align = {}, generic_allocator* allocator = nullptr)
			: allocator_{ allocator },
			  alignment_{ impl::aligned_alloc_actual_align(size, align) },
			  size_{ size },
			  data_{ size_ ? impl::generic_alloc(allocator_, size_, alignment_) : nullptr }
		{
			if (data_ && src)
				MUU_MEMCPY(data_, src, size_);
		}

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		blob(const blob& other) //
			: blob{ other.size_, other.data_, other.alignment_, other.allocator_ }
		{}

		/// \brief Copy constructor.
		///
		/// \param	other		The blob to copy.
		/// \param	align		The alignment of the blob's data.
		///						Leave as `0` to copy the source alignment.
		/// \param	allocator 	The #muu::generic_allocator used for allocations.
		///						Leave as `nullptr` to copy the source allocator.
		MUU_NODISCARD_CTOR
		blob(const blob& other, size_t align, generic_allocator* allocator = nullptr) //
			: blob{ other.size_,
					other.data_,
					align ? align : other.alignment_,
					allocator ? allocator : other.allocator_ }
		{}

		/// \brief Move constructor.
		MUU_NODISCARD_CTOR
		blob(blob&& other) noexcept
			: allocator_{ other.allocator_ },
			  alignment_{ std::exchange(other.alignment_, impl::aligned_alloc_min_align) },
			  size_{ std::exchange(other.size_, 0_sz) },
			  data_{ std::exchange(other.data_, nullptr) }
		{}

		/// \brief Destructor.
		~blob() noexcept
		{
			if (auto data = std::exchange(data_, nullptr))
				impl::generic_free(allocator_, data);
		}

		/// Replaces the contents of the blob with the given data.
		///
		/// \param	src	 		The data to copy.
		/// \param	sz 			The size of the data.
		/// \param	align		The alignment to use.
		///						Leave as `0` to use `__STDCPP_DEFAULT_NEW_ALIGNMENT__`.
		/// \param	allocator 	The #muu::generic_allocator used for allocations.
		///						Leave as `nullptr` to keep using the current allocator.
		///
		/// \return	A reference to the input blob.
		blob& assign(size_t sz, const void* src, size_t align = {}, generic_allocator* allocator = nullptr)
		{
			align = impl::aligned_alloc_actual_align(sz, align);
			if (!allocator)
				allocator = allocator_;

			// check if this is effectively a resize with a copy or move
			if (align == alignment_ && allocator == allocator_)
			{
				resize(sz); // no-op if the same as current
				MUU_ASSERT(size_ == sz);

				if (src && data_ && src != data_)
					MUU_MEMCPY(data_, src, size_);

				return *this;
			}

			// changing alignment or allocator; must deallocate and reallocate
			if (data_)
				impl::generic_free(allocator_, data_);
			allocator_ = allocator;
			alignment_ = align;
			size_	   = sz;
			data_	   = size_ ? impl::generic_alloc(allocator_, size_, alignment_) : nullptr;
			if (src && data_)
				MUU_MEMCPY(data_, src, size_);
			return *this;
		}

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
		blob& operator=(blob&& rhs) noexcept
		{
			if (&rhs != this)
			{
				if (data_)
					impl::generic_free(allocator_, data_);

				allocator_ = rhs.allocator_;
				alignment_ = std::exchange(rhs.alignment_, impl::aligned_alloc_min_align);
				size_	   = std::exchange(rhs.size_, 0_sz);
				data_	   = std::exchange(rhs.data_, nullptr);
			}
			return *this;
		}

		/// \brief Returns the size of the blob's data, in bytes.
		MUU_PURE_INLINE_GETTER
		size_t size() const noexcept
		{
			return size_;
		}

		/// \brief Resizes the blob, keeping the region of `min(newSize, oldSize)` intact.
		///
		/// \param	sz New size.
		///
		/// \return	A reference to the blob.
		blob& resize(size_t sz)
		{
			if (size_ == sz)
				return *this;

			// something -> nothing
			if (!size_)
			{
				MUU_ASSERT(data_);
				impl::generic_free(allocator_, data_);
				data_ = nullptr;
			}

			// something -> something
			else if (data_)
			{
				auto new_data = impl::generic_alloc(allocator_, sz, alignment_);
				MUU_MEMCPY(new_data, data_, min(sz, size_));
				impl::generic_free(allocator_, data_);
				data_ = new_data;
			}

			// nothing -> something
			else
				data_ = impl::generic_alloc(allocator_, sz, alignment_);

			size_ = sz;
			return *this;
		}

		/// \brief Returns the alignment of the blob's data, in bytes.
		MUU_PURE_INLINE_GETTER
		size_t alignment() const noexcept
		{
			return alignment_;
		}

		/// \brief Returns a pointer to the blob's data.
		MUU_PURE_INLINE_GETTER
		MUU_ATTR(assume_aligned(impl::aligned_alloc_min_align))
		std::byte* data() noexcept
		{
			return muu::assume_aligned<impl::aligned_alloc_min_align>(static_cast<std::byte*>(data_));
		}

		/// \brief Returns a pointer to the blob's data (const overload).
		MUU_PURE_INLINE_GETTER
		MUU_ATTR(assume_aligned(impl::aligned_alloc_min_align))
		const std::byte* data() const noexcept
		{
			return muu::assume_aligned<impl::aligned_alloc_min_align>(static_cast<const std::byte*>(data_));
		}

		/// \brief Returns true if the blob contains data.
		MUU_PURE_INLINE_GETTER
		explicit operator bool() const noexcept
		{
			return data_ != nullptr;
		}
	};
}

#include "impl/header_end.h"
