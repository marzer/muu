// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "settings.h"

#ifdef __clang__
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Weverything"
#elif defined (__GNUC__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wall"
	#pragma GCC diagnostic ignored "-Wextra"
	#pragma GCC diagnostic ignored "-Wpadded"
	#pragma GCC diagnostic ignored "-Wfloat-equal"
#elif defined(_MSC_VER) || defined(__INTEL_COMPILER)
	#pragma warning(push, 0)
	#ifdef __INTEL_COMPILER // icc is such fucking garbage honestly
		#pragma warning(disable: 82)
		#pragma warning(disable: 111)
		#pragma warning(disable: 177)
		#pragma warning(disable: 869)
		#pragma warning(disable: 1011)
		#pragma warning(disable: 2261)
		#pragma warning(disable: 2557)
		#pragma warning(disable: 3280)
	#endif
#endif

#ifdef __has_include
	#if __has_include(<Catch2/single_include/catch2/catch.hpp>)
		#include <Catch2/single_include/catch2/catch.hpp>
	#else
		#error Catch2 is missing! You probably need to fetch submodules ("git submodule update --init extern/Catch2")
	#endif
#else
	#include <Catch2/single_include/catch2/catch.hpp>
#endif

#ifdef __clang__
	#pragma clang diagnostic pop
#elif defined (__GNUC__)
	#pragma GCC diagnostic pop
#elif defined(_MSC_VER) || defined(__INTEL_COMPILER)
	#pragma warning(pop)
#endif

