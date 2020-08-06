// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief		Convenience header for including _all_ other muu headers.
/// \warning	This header is a convenience facility meant to simplify builds with precompiled headers or
/// 			where MUU_HEADER_ONLY is disabled. Using it outside of these contexts may cause compile-times to
/// 			increase significantly. Buyer beware.

#pragma once

#include "../muu/accumulator.h"
#include "../muu/aligned_alloc.h"
#include "../muu/bitset.h"
#include "../muu/blob.h"
#include "../muu/concatenate.h"
#include "../muu/half.h"
#include "../muu/hashing.h"
#include "../muu/numeric_range.h"
#include "../muu/rolling_average.h"
#include "../muu/semver.h"
#include "../muu/span.h"
#include "../muu/string_param.h"
#include "../muu/tagged_ptr.h"
#include "../muu/thread_pool.h"
#include "../muu/uuid.h"
