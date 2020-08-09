// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"

struct empty {};
struct empty2 {};
static_assert(std::is_empty_v<empty>);	// implementation sanity-checks
static_assert(std::is_empty_v<empty2>);	// 

// variant 0: no empty members

static_assert(sizeof(compressed_pair<int, int>) == sizeof(int) * 2);
static_assert(std::is_same_v<compressed_pair<int, int>, compressed_pair<int, int, 0u>>);
static_assert(!std::is_empty_v<compressed_pair<int, int>>);
static_assert(std::is_standard_layout_v<compressed_pair<int, int>>);
static_assert(std::is_trivially_constructible_v<compressed_pair<int, int>>);
static_assert(std::is_trivially_copy_constructible_v<compressed_pair<int, int>>);
static_assert(std::is_trivially_copy_assignable_v<compressed_pair<int, int>>);
static_assert(std::is_trivially_move_constructible_v<compressed_pair<int, int>>);
static_assert(std::is_trivially_move_assignable_v<compressed_pair<int, int>>);
static_assert(std::is_trivially_destructible_v<compressed_pair<int, int>>);

// variant 1: empty member is first

static_assert(sizeof(compressed_pair<empty, int>) == sizeof(int));
static_assert(std::is_same_v<compressed_pair<empty, int>, compressed_pair<empty, int, 1u>>);
static_assert(!std::is_empty_v<compressed_pair<empty, int>>);
static_assert(std::is_standard_layout_v<compressed_pair<empty, int>>);
static_assert(std::is_trivially_constructible_v<compressed_pair<empty, int>>);
static_assert(std::is_trivially_copy_constructible_v<compressed_pair<empty, int>>);
static_assert(std::is_trivially_copy_assignable_v<compressed_pair<empty, int>>);
static_assert(std::is_trivially_move_constructible_v<compressed_pair<empty, int>>);
static_assert(std::is_trivially_move_assignable_v<compressed_pair<empty, int>>);
static_assert(std::is_trivially_destructible_v<compressed_pair<empty, int>>);

// variant 2: empty member is second

static_assert(sizeof(compressed_pair<int, empty>) == sizeof(int));
static_assert(std::is_same_v<compressed_pair<int, empty>, compressed_pair<int, empty, 2u>>);
static_assert(!std::is_empty_v<compressed_pair<int, empty>>);
static_assert(std::is_standard_layout_v<compressed_pair<int, empty>>);
static_assert(std::is_trivially_constructible_v<compressed_pair<int, empty>>);
static_assert(std::is_trivially_copy_constructible_v<compressed_pair<int, empty>>);
static_assert(std::is_trivially_copy_assignable_v<compressed_pair<int, empty>>);
static_assert(std::is_trivially_move_constructible_v<compressed_pair<int, empty>>);
static_assert(std::is_trivially_move_assignable_v<compressed_pair<int, empty>>);
static_assert(std::is_trivially_destructible_v<compressed_pair<int, empty>>);

// variant 3: both members are empty

static_assert(sizeof(compressed_pair<empty, empty2>) == 1);
static_assert(std::is_same_v<compressed_pair<empty, empty2>, compressed_pair<empty, empty2, 3u>>);
#if !MUU_MSVC && !MUU_ICC_CL // https://developercommunity.visualstudio.com/content/problem/1142409/c-stdis-empty-is-incorrect-for-a-template-speciali.html
static_assert(std::is_empty_v<compressed_pair<empty, empty2>>);
#endif
static_assert(std::is_standard_layout_v<compressed_pair<empty, empty2>>);
static_assert(std::is_trivially_constructible_v<compressed_pair<empty, empty2>>);
static_assert(std::is_trivially_copy_constructible_v<compressed_pair<empty, empty2>>);
static_assert(std::is_trivially_copy_assignable_v<compressed_pair<empty, empty2>>);
static_assert(std::is_trivially_move_constructible_v<compressed_pair<empty, empty2>>);
static_assert(std::is_trivially_move_assignable_v<compressed_pair<empty, empty2>>);
static_assert(std::is_trivially_destructible_v<compressed_pair<empty, empty2>>);
