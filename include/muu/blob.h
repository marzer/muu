// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief Contains the definition of muu::blob.

#pragma once
#include "../muu/fwd.h"

MUU_NAMESPACE_START
{
	/// \brief Interface for managing chunks of memory.
	/// \ingroup mem
	/// \ingroup blocks
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
MUU_NAMESPACE_END

#if MUU_IMPLEMENTATION
	#include "../muu/impl/blob.hpp"
#endif
