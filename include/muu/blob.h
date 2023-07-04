// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the definition of #muu::blob.

#include "aligned_alloc.h"
#include "impl/std_memcpy.h"
#include "impl/std_utility.h"
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
		/// \cond

		size_t alignment_;
		size_t size_ = 0;
		void* data_	 = nullptr;

		static constexpr size_t big_align_ = impl::aligned_alloc_min_align > 64u ? impl::aligned_alloc_min_align : 64u;

		MUU_CONST_INLINE_GETTER
		static constexpr size_t actual_alignment(size_t size, size_t align) noexcept
		{
			if (size < 2048u)
				return align > impl::aligned_alloc_min_align ? align : impl::aligned_alloc_min_align;
			return align > big_align_ ? align : big_align_;
		}
		/// \endcond

	  public:
		/// \brief Creates an empty blob.
		MUU_NODISCARD_CTOR
		explicit blob() noexcept //
			: alignment_{ impl::aligned_alloc_min_align }
		{}

		/// \brief Creates a blob of a fixed size and alignment.
		///
		/// \param	size		The size of the blob's data, in bytes.
		/// \param	src	 		The source data to copy, if any.
		/// \param	align		The alignment of the blob's data.
		///						Leave as `0` to use `__STDCPP_DEFAULT_NEW_ALIGNMENT__`.
		MUU_NODISCARD_CTOR
		explicit blob(size_t size, const void* src = nullptr, size_t align = {})
			: alignment_{ actual_alignment(size, align) },
			  size_{ size },
			  data_{ size_ ? aligned_alloc(size_, alignment_) : nullptr }
		{
			if (data_ && src)
				MUU_MEMCPY(data_, src, size_);
		}

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		blob(const blob& other) //
			: blob{ other.size_, other.data_, other.alignment_ }
		{}

		/// \brief Copy constructor.
		///
		/// \param	other		The blob to copy.
		/// \param	align		The alignment of the blob's data.
		///						Leave as `0` to copy the source alignment.
		MUU_NODISCARD_CTOR
		blob(const blob& other, size_t align) //
			: blob{ other.size_, other.data_, align ? align : other.alignment_ }
		{}

		/// \brief Move constructor.
		MUU_NODISCARD_CTOR
		blob(blob&& other) noexcept //
			: alignment_{ std::exchange(other.alignment_, impl::aligned_alloc_min_align) },
			  size_{ std::exchange(other.size_, size_t{}) },
			  data_{ std::exchange(other.data_, nullptr) }
		{}

		/// \brief Destructor.
		~blob() noexcept
		{
			if (auto data = std::exchange(data_, nullptr))
				aligned_free(data);
		}

		/// Replaces the contents of the blob with the given data.
		///
		/// \param	src	 		The data to copy.
		/// \param	sz 			The size of the data.
		/// \param	new_align	The new alignment to use.
		///						Leave as `0` to use `__STDCPP_DEFAULT_NEW_ALIGNMENT__`.
		///
		/// \return	A reference to the input blob.
		blob& assign(size_t sz, const void* src, size_t new_align = {})
		{
			new_align = actual_alignment(sz, new_align);

			// check if this is effectively a resize with a copy or move
			if (new_align <= alignment_)
			{
				new_align = alignment_;

				resize(sz); // no-op if the same size as current
				MUU_ASSERT(size_ == sz);

				if (src && data_ && src != data_)
					MUU_MEMCPY(data_, src, size_);

				return *this;
			}

			// changing alignment; must deallocate and reallocate
			if (data_)
				aligned_free(data_);
			alignment_ = new_align;
			size_	   = sz;
			data_	   = size_ ? aligned_alloc(size_, alignment_) : nullptr;
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
					aligned_free(data_);

				alignment_ = std::exchange(rhs.alignment_, impl::aligned_alloc_min_align);
				size_	   = std::exchange(rhs.size_, size_t{});
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
		blob& resize(size_t sz) noexcept
		{
			if (size_ == sz)
				return *this;

			// something -> *
			if (size_)
			{
				MUU_ASSERT(data_);

				// something -> nothing
				if (!sz)
				{
					aligned_free(data_);
					data_ = nullptr;
				}

				// something -> something
				else
				{
					auto new_data = aligned_alloc(sz, alignment_);
					MUU_ASSERT(new_data);

					MUU_MEMCPY(new_data, data_, min(sz, size_));
					aligned_free(data_);
					data_ = new_data;
				}
			}

			// nothing -> something
			else
			{
				data_ = aligned_alloc(sz, alignment_);
				MUU_ASSERT(data_);
			}

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
