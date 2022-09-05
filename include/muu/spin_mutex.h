// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::spin_mutex.

#include "fwd.h"
#include "muu/impl/pause.h"
MUU_DISABLE_WARNINGS;
#include <atomic>
MUU_ENABLE_WARNINGS;
MUU_FORCE_NDEBUG_OPTIMIZATIONS;

namespace muu
{
	/// \brief A mutex that acquires locks by-spin waiting.
	class MUU_NODISCARD_CLASS spin_mutex
	{
	  private:
		std::atomic_bool held_;

	  public:
		/// \brief Default constructor.
		MUU_NODISCARD_CTOR
		spin_mutex() noexcept = default;

		/// \brief Destructor
		~spin_mutex() noexcept = default;

		MUU_DELETE_COPY(spin_mutex);
		MUU_DELETE_MOVE(spin_mutex);

		/// \brief Acquires a lock on the mutex.
		void lock() noexcept
		{
			int mask		  = 1;
			constexpr int max = 64;
			while (held_.exchange(true, std::memory_order_acquire))
			{
				while (held_.load(std::memory_order_relaxed))
				{
					for (int i = mask; i; --i)
						MUU_PAUSE();
					mask = mask < max ? mask << 1 : max;
				}
			}
		}

		/// \brief Tries to acquire a lock on the mutex.
		/// \returns `true` if the lock was acquired.
		MUU_NODISCARD
		MUU_ALWAYS_INLINE
		bool try_lock() noexcept
		{
			return !held_.load(std::memory_order_relaxed) //
				&& !held_.exchange(true, std::memory_order_acquire);
		}

		/// \brief Releases the held lock on the mutex.
		MUU_ALWAYS_INLINE
		void unlock() noexcept
		{
			held_.store(false, std::memory_order_release);
		}
	};
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
