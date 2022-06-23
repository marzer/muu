// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/vector.h"

using namespace muu;

template <typename S, size_t D>
static constexpr bool readonly_param_by_value = !std::is_reference_v<muu::readonly_param<vector<S, D>>>;
template <typename S, size_t D>
static constexpr bool vector_param_by_value = !std::is_reference_v<muu::vector_param<vector<S, D>>>;

static_assert(readonly_param_by_value<uintptr_t, 1>);
static_assert(readonly_param_by_value<uintptr_t, 2>);
static_assert(!readonly_param_by_value<uintptr_t, 3>);
static_assert(!readonly_param_by_value<uintptr_t, 4>);
static_assert(!readonly_param_by_value<uintptr_t, 5>);

static_assert(vector_param_by_value<uintptr_t, 1>);
static_assert(vector_param_by_value<uintptr_t, 2>);
static_assert(!vector_param_by_value<uintptr_t, 3>);
static_assert(!vector_param_by_value<uintptr_t, 4>);
static_assert(!vector_param_by_value<uintptr_t, 5>);

using pfloat = std::conditional_t<sizeof(double) == sizeof(void*), double, float>;
static_assert(sizeof(pfloat) == sizeof(void*));

static_assert(readonly_param_by_value<pfloat, 1>);
static_assert(readonly_param_by_value<pfloat, 2>);
static_assert(!readonly_param_by_value<pfloat, 3>);
static_assert(!readonly_param_by_value<pfloat, 4>);
static_assert(!readonly_param_by_value<pfloat, 5>);

#if MUU_HAS_VECTORCALL

static_assert(vector_param_by_value<pfloat, 1>);
static_assert(vector_param_by_value<pfloat, 2>);
static_assert(vector_param_by_value<pfloat, 3>);
static_assert(vector_param_by_value<pfloat, 4>);
static_assert(!vector_param_by_value<pfloat, 5>);

#else

static_assert(vector_param_by_value<pfloat, 1>);
static_assert(vector_param_by_value<pfloat, 2>);
static_assert(!vector_param_by_value<pfloat, 3>);
static_assert(!vector_param_by_value<pfloat, 4>);
static_assert(!vector_param_by_value<pfloat, 5>);

#endif
