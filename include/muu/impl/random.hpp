// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "../../muu/preprocessor.h"
#if !MUU_IMPLEMENTATION
	#error This is an implementation-only header.
#endif

MUU_PUSH_WARNINGS
MUU_DISABLE_ALL_WARNINGS
#include <random>
MUU_POP_WARNINGS

MUU_PUSH_WARNINGS
MUU_DISABLE_LIFETIME_WARNINGS
MUU_PRAGMA_CLANG("clang diagnostic ignored \"-Wmissing-prototypes\"")

MUU_IMPL_NAMESPACE_START
{
	// todo: not everyone will be ok with thread_local.
	// I should implement a way of controlling this via a library config option or two.

	[[nodiscard]]
	std::random_device& random_device() noexcept
	{
		thread_local std::random_device rdev;
		return rdev;
	}

	[[nodiscard]]
	std::mt19937& mersenne_twister() noexcept
	{
		thread_local std::mt19937 engine{ random_device()() };
		return engine;
	}
}
MUU_IMPL_NAMESPACE_END

MUU_POP_WARNINGS // MUU_DISABLE_LIFETIME_WARNINGS
