// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::spin_mutex.

#include "fwd.h"
#include "pause.h"
MUU_DISABLE_WARNINGS;
#include <atomic>
MUU_ENABLE_WARNINGS;
MUU_FORCE_NDEBUG_OPTIMIZATIONS;

namespace muu
{
	//% spin_mutex start

	/// \brief A mutex that acquires locks by spin-waiting.
	class MUU_NODISCARD_CLASS spin_mutex
	{
		// implementation is based on this article:
		// https://rigtorp.se/spinlock/
		//
		// increasing spin-wait backoff based on "Intel 64 and IA-32 Architectures Optimization Reference Manual":
		// https://software.intel.com/sites/default/files/managed/9e/bc/64-ia-32-architectures-optimization-manual.pdf

	  private:
		std::atomic_bool held_;

	  public:
		/// \brief Default constructor.
		MUU_NODISCARD_CTOR
		spin_mutex() noexcept //
			: held_{ false }
		{}

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
		bool try_lock() noexcept
		{
			return !held_.load(std::memory_order_relaxed) //
				&& !held_.exchange(true, std::memory_order_acquire);
		}

		/// \brief Releases the held lock on the mutex.
		void unlock() noexcept
		{
			held_.store(false, std::memory_order_release);
		}
	};

	//% spin_mutex end
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
