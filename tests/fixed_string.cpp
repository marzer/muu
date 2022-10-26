// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/fixed_string.h"

static_assert(!fixed_string{ "" });

static_assert(fixed_string{ "" } == fixed_string{ "" });
static_assert(fixed_string{ "te" } == fixed_string{ "te" });
static_assert(fixed_string{ "test" } == fixed_string{ "test" });

static_assert(fixed_string{ "" } != fixed_string{ "test" });
static_assert(fixed_string{ "te" } != fixed_string{ "test" });
static_assert(fixed_string{ "test" } != fixed_string{ "" });
static_assert(fixed_string{ "test" } != fixed_string{ "te" });

static_assert(fixed_string{ "" } < fixed_string{ "test" });
static_assert(fixed_string{ "te" } < fixed_string{ "test" });
static_assert(fixed_string{ "" } <= fixed_string{ "test" });
static_assert(fixed_string{ "te" } <= fixed_string{ "test" });
static_assert(fixed_string{ "test" } > fixed_string{ "" });
static_assert(fixed_string{ "test" } > fixed_string{ "te" });
static_assert(fixed_string{ "test" } >= fixed_string{ "" });
static_assert(fixed_string{ "test" } >= fixed_string{ "te" });

static_assert(fixed_string{ "the quick brown fox" }.substr<4, 0>() == fixed_string{ "" });
static_assert(fixed_string{ "the quick brown fox" }.substr<4, 5>() == fixed_string{ "quick" });
static_assert(fixed_string{ "the quick brown fox" }.substr<9999, 5>() == fixed_string{ "" });
static_assert(fixed_string{ "the quick brown fox" }.substr<-3>() == fixed_string{ "fox" });
static_assert(fixed_string{ "the quick brown fox" }.substr<-3, 2>() == fixed_string{ "fo" });

static_assert(fixed_string{ "the quick brown fox" }.slice<4, 3>() == fixed_string{ "" });
static_assert(fixed_string{ "the quick brown fox" }.slice<4, 4>() == fixed_string{ "" });
static_assert(fixed_string{ "the quick brown fox" }.slice<4, 5>() == fixed_string{ "q" });
static_assert(fixed_string{ "the quick brown fox" }.slice<4, 9>() == fixed_string{ "quick" });
static_assert(fixed_string{ "the quick brown fox" }.slice<4, -4>() == fixed_string{ "quick brown" });
static_assert(fixed_string{ "the quick brown fox" }.slice<-3>() == fixed_string{ "fox" });
static_assert(fixed_string{ "the quick brown fox" }.slice<-3, -1>() == fixed_string{ "fo" });

// static_assert(fixed_string{ "the quick brown fox" }.find_first_of<'q'>() == 4_sz);
// static_assert(!fixed_string{ "the quick brown fox" }.find_first_of<'q', 7>());
// static_assert(fixed_string{ "the quick brown fox" }.find_last_of<'f'>() == 16_sz);
// static_assert(fixed_string{ "the quick brown fox" }.find_last_of<'f', 16>() == 16_sz);
// static_assert(!fixed_string{ "the quick brown fox" }.find_last_of<'f', 17>());

static_assert(fixed_string{ "abc" } + fixed_string{ "def" } == fixed_string{ "abcdef" });
static_assert(fixed_string{ "abc" } + fixed_string{ "" } == fixed_string{ "abc" });
static_assert(fixed_string{ "" } + fixed_string{ "def" } == fixed_string{ "def" });
static_assert(fixed_string{ "" } + fixed_string{ "" } == fixed_string{ "" });
static_assert(fixed_string{ "abc" } + 'd' == fixed_string{ "abcd" });
static_assert('a' + fixed_string{ "bcd" } == fixed_string{ "abcd" });
static_assert('a' + fixed_string{ "bcd" } == fixed_string{ "abcd" });
#if MUU_HAS_FIXED_STRING_LITERALS
static_assert("abc"_fs == fixed_string{ "abc" });
#endif
