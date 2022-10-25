// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/blob.h"
#include "../include/muu/bit.h"

TEST_CASE("blob")
{
	// default ctor
	blob blb;
	CHECK(blb.size() == 0);
	CHECK(blb.data() == nullptr);
	CHECK(blb.alignment() == impl::aligned_alloc_min_align);
	CHECK(has_single_bit(blb.alignment()));
	CHECK(!blb);

	// sized init
	blob blb2{ 1024 };
	CHECK(blb2);
	CHECK(blb2.size() == 1024);
	CHECK(blb2.data() != nullptr);
	CHECK(blb2.alignment() == impl::aligned_alloc_min_align);
	CHECK(has_single_bit(blb2.alignment()));

	// copy constructor (and move assignment)
	blb = blob{ blb2 };
	CHECK(blb);
	CHECK(blb.size() == 1024);
	CHECK(blb.data() != nullptr);
	CHECK(blb.alignment() == impl::aligned_alloc_min_align);
	CHECK(has_single_bit(blb.alignment()));
	CHECK(blb2);
	CHECK(blb2.size() == blb.size());
	CHECK(blb2.data() != nullptr);
	CHECK(blb2.data() != blb.data());
	CHECK(blb2.alignment() == blb.alignment());
	CHECK(blb2.alignment() == impl::aligned_alloc_min_align);
	CHECK(MUU_MEMCMP(blb.data(), blb2.data(), blb.size()) == 0);

	// move constructor
	blb = blob{ std::move(blb2) };
	CHECK(blb);
	CHECK(blb.size() == 1024);
	CHECK(blb.data() != nullptr);
	CHECK(blb.alignment() == impl::aligned_alloc_min_align);
	CHECK(has_single_bit(blb.alignment()));
	CHECK(!blb2);
	CHECK(blb2.size() == 0);
	CHECK(blb2.data() == nullptr);
	CHECK(blb2.alignment() == impl::aligned_alloc_min_align);

	// aligned data
	blb2 = blob{ 2048, nullptr, 128 };
	CHECK(blb2);
	CHECK(blb2.size() == 2048);
	CHECK(blb2.data() != nullptr);
	CHECK(blb2.alignment() == 128);

	// copy constructor w/ alignment change
	blb = blob{ blb2, 64 };
	CHECK(blb);
	CHECK(blb.size() == blb2.size());
	CHECK(blb.data() != nullptr);
	CHECK(blb.data() != blb2.data());
	CHECK(blb.alignment() == 64);
	CHECK(blb2);
	CHECK(MUU_MEMCMP(blb.data(), blb2.data(), blb.size()) == 0);
}
