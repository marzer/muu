// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#ifndef MUU_STD_TYPE_TRAITS_H
#define MUU_STD_TYPE_TRAITS_H

#include "../preprocessor.h"

MUU_DISABLE_WARNINGS;
#include <type_traits>
MUU_ENABLE_WARNINGS;

// replace the muu homebrew remove_reference_ and enable_if_ with those from <type_traits>
// because major compilers memoize things in <type_traits> for improved throughput
#undef MUU_MOVE
#define MUU_MOVE(...) static_cast<std::remove_reference_t<decltype(__VA_ARGS__)>&&>(__VA_ARGS__)

#undef MUU_ENABLE_IF
#define MUU_ENABLE_IF(...) , std::enable_if_t<!!(__VA_ARGS__), int> = 0

#if defined(__cpp_lib_type_identity) && __cpp_lib_type_identity >= 201806
	#undef MUU_TYPE_IDENTITY
	#define MUU_TYPE_IDENTITY(...) std::type_identity_t<__VA_ARGS__>
#endif

#endif // MUU_STD_TYPE_TRAITS_H
