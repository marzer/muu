// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/static_string.h"

static_assert(!static_string{ "" });

static_assert(static_string{ "" } == static_string{ "" });
static_assert(static_string{ "te" } == static_string{ "te" });
static_assert(static_string{ "test" } == static_string{ "test" });

static_assert(static_string{ "" } != static_string{ "test" });
static_assert(static_string{ "te" } != static_string{ "test" });
static_assert(static_string{ "test" } != static_string{ "" });
static_assert(static_string{ "test" } != static_string{ "te" });

static_assert(static_string{ "" } < static_string{ "test" });
static_assert(static_string{ "te" } < static_string{ "test" });
static_assert(static_string{ "" } <= static_string{ "test" });
static_assert(static_string{ "te" } <= static_string{ "test" });
static_assert(static_string{ "test" } > static_string{ "" });
static_assert(static_string{ "test" } > static_string{ "te" });
static_assert(static_string{ "test" } >= static_string{ "" });
static_assert(static_string{ "test" } >= static_string{ "te" });

static_assert(static_string{ "the quick brown fox" }.substr<4, 0>() == static_string{ "" });
static_assert(static_string{ "the quick brown fox" }.substr<4, 5>() == static_string{ "quick" });
static_assert(static_string{ "the quick brown fox" }.substr<9999, 5>() == static_string{ "" });
static_assert(static_string{ "the quick brown fox" }.substr<-3>() == static_string{ "fox" });
static_assert(static_string{ "the quick brown fox" }.substr<-3, 2>() == static_string{ "fo" });

static_assert(static_string{ "the quick brown fox" }.slice<4, 3>() == static_string{ "" });
static_assert(static_string{ "the quick brown fox" }.slice<4, 4>() == static_string{ "" });
static_assert(static_string{ "the quick brown fox" }.slice<4, 5>() == static_string{ "q" });
static_assert(static_string{ "the quick brown fox" }.slice<4, 9>() == static_string{ "quick" });
static_assert(static_string{ "the quick brown fox" }.slice<4, -4>() == static_string{ "quick brown" });
static_assert(static_string{ "the quick brown fox" }.slice<-3>() == static_string{ "fox" });
static_assert(static_string{ "the quick brown fox" }.slice<-3, -1>() == static_string{ "fo" });

// static_assert(static_string{ "the quick brown fox" }.find_first_of<'q'>() == 4_sz);
// static_assert(!static_string{ "the quick brown fox" }.find_first_of<'q', 7>());
// static_assert(static_string{ "the quick brown fox" }.find_last_of<'f'>() == 16_sz);
// static_assert(static_string{ "the quick brown fox" }.find_last_of<'f', 16>() == 16_sz);
// static_assert(!static_string{ "the quick brown fox" }.find_last_of<'f', 17>());

static_assert(static_string{ "abc" } + static_string{ "def" } == static_string{ "abcdef" });
static_assert(static_string{ "abc" } + static_string{ "" } == static_string{ "abc" });
static_assert(static_string{ "" } + static_string{ "def" } == static_string{ "def" });
static_assert(static_string{ "" } + static_string{ "" } == static_string{ "" });
static_assert(static_string{ "abc" } + 'd' == static_string{ "abcd" });
static_assert('a' + static_string{ "bcd" } == static_string{ "abcd" });
static_assert('a' + static_string{ "bcd" } == static_string{ "abcd" });
#if MUU_HAS_STATIC_STRING_LITERALS
static_assert("abc"_ss == static_string{ "abc" });
#endif
