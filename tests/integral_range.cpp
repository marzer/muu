// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/integral_range.h"

TEST_CASE("integral_range")
{
	const auto range_empty = integral_range<int>{};
	CHECK(!range_empty);
	CHECK(range_empty.empty());
	CHECK(range_empty.size() == 0u);
	CHECK(range_empty.s == 0);
	CHECK(range_empty.e == 0);

	const auto range_5678 = integral_range{ 5, 9 };
	CHECK(range_5678);
	CHECK(!range_5678.empty());
	CHECK(range_5678.size() == 4u);
	CHECK(range_5678.s == 5);
	CHECK(range_5678.e == 9);
	CHECK(range_5678.first() == 5);
	CHECK(range_5678.last() == 8);
	CHECK(!range_5678.contains(4));
	CHECK(range_5678.contains(5));
	CHECK(range_5678.contains(8));
	CHECK(!range_5678.contains(9));

	const auto range_34 = integral_range{ 3, 5 };
	CHECK(range_34);
	CHECK(!range_34.empty());
	CHECK(range_34.size() == 2u);
	CHECK(range_34.s == 3);
	CHECK(range_34.e == 5);
	CHECK(range_34.first() == 3);
	CHECK(range_34.last() == 4);
	CHECK(!range_34.contains(2));
	CHECK(range_34.contains(3));
	CHECK(range_34.contains(4));
	CHECK(!range_34.contains(5));

	const auto range_7 = integral_range{ 7, 8 };
	CHECK(range_7);
	CHECK(!range_7.empty());
	CHECK(range_7.size() == 1u);
	CHECK(range_7.s == 7);
	CHECK(range_7.e == 8);
	CHECK(range_7.first() == 7);
	CHECK(range_7.last() == 7);
	CHECK(!range_7.contains(6));
	CHECK(range_7.contains(7));
	CHECK(!range_7.contains(8));

	CHECK_SYMMETRIC_EQUAL(range_5678, range_5678);
	CHECK_SYMMETRIC_INEQUAL(range_5678, range_7);
	CHECK_SYMMETRIC_INEQUAL(range_5678, range_34);
	CHECK_SYMMETRIC_INEQUAL(range_7, range_34);

	CHECK(range_5678.contains(range_5678));
	CHECK(!range_5678.contains(range_34));
	CHECK(range_5678.contains(range_7));
	CHECK(!range_34.contains(range_5678));
	CHECK(range_34.contains(range_34));
	CHECK(!range_34.contains(range_7));
	CHECK(!range_7.contains(range_5678));
	CHECK(!range_7.contains(range_34));
	CHECK(range_7.contains(range_7));

	CHECK(range_5678.intersects(range_5678));
	CHECK(!range_5678.intersects(range_34));
	CHECK(range_5678.intersects(range_7));
	CHECK(!range_34.intersects(range_5678));
	CHECK(range_34.intersects(range_34));
	CHECK(!range_34.intersects(range_7));
	CHECK(range_7.intersects(range_5678));
	CHECK(!range_7.intersects(range_34));
	CHECK(range_7.intersects(range_7));

	CHECK(!range_5678.adjacent(range_5678));
	CHECK(range_5678.adjacent(range_34));
	CHECK(!range_5678.adjacent(range_7));
	CHECK(range_34.adjacent(range_5678));
	CHECK(!range_34.adjacent(range_34));
	CHECK(!range_34.adjacent(range_7));
	CHECK(!range_7.adjacent(range_5678));
	CHECK(!range_7.adjacent(range_34));
	CHECK(!range_7.adjacent(range_7));

	CHECK(range_5678.intersects_or_adjacent(range_5678));
	CHECK(range_5678.intersects_or_adjacent(range_34));
	CHECK(range_5678.intersects_or_adjacent(range_7));
	CHECK(range_34.intersects_or_adjacent(range_5678));
	CHECK(range_34.intersects_or_adjacent(range_34));
	CHECK(!range_34.intersects_or_adjacent(range_7));
	CHECK(range_7.intersects_or_adjacent(range_5678));
	CHECK(!range_7.intersects_or_adjacent(range_34));
	CHECK(range_7.intersects_or_adjacent(range_7));

	const auto range_345678 = integral_range{ 3, 9 };
	CHECK_SYMMETRIC_EQUAL(range_5678.union_with(range_34), range_345678);
	CHECK_SYMMETRIC_EQUAL(range_34.union_with(range_5678), range_345678);
	CHECK_SYMMETRIC_EQUAL(range_5678.union_with(range_7), range_5678);
	CHECK_SYMMETRIC_EQUAL(range_7.union_with(range_5678), range_5678);

	std::array<int, 4> vals;
	const std::array<int, 4> expected{ { 5, 6, 7, 8 } };
	auto v = vals.data();
	for (auto i : range_5678)
		*v++ = i;
	CHECK(v == vals.data() + vals.size());
	CHECK(vals == expected);
}

TEST_CASE("integral_range - byte*")
{
	const auto range_empty = byteptr_range{};
	CHECK(!range_empty);
	CHECK(range_empty.empty());
	CHECK(range_empty.size() == 0u);
	CHECK(range_empty.s == nullptr);
	CHECK(range_empty.e == nullptr);

	std::byte bytes[10]{};

	const auto range_5678 = integral_range{ bytes + 5, bytes + 9 };
	CHECK(range_5678);
	CHECK(!range_5678.empty());
	CHECK(range_5678.size() == 4u);
	CHECK(range_5678.s == bytes + 5);
	CHECK(range_5678.e == bytes + 9);
	CHECK(range_5678.first() == bytes + 5);
	CHECK(range_5678.last() == bytes + 8);
	CHECK(!range_5678.contains(bytes + 4));
	CHECK(range_5678.contains(bytes + 5));
	CHECK(range_5678.contains(bytes + 8));
	CHECK(!range_5678.contains(bytes + 9));

	const auto range_34 = integral_range{ bytes + 3, bytes + 5 };
	CHECK(range_34);
	CHECK(!range_34.empty());
	CHECK(range_34.size() == 2u);
	CHECK(range_34.s == bytes + 3);
	CHECK(range_34.e == bytes + 5);
	CHECK(range_34.first() == bytes + 3);
	CHECK(range_34.last() == bytes + 4);
	CHECK(!range_34.contains(bytes + 2));
	CHECK(range_34.contains(bytes + 3));
	CHECK(range_34.contains(bytes + 4));
	CHECK(!range_34.contains(bytes + 5));

	const auto range_7 = integral_range{ bytes + 7, bytes + 8 };
	CHECK(range_7);
	CHECK(!range_7.empty());
	CHECK(range_7.size() == 1u);
	CHECK(range_7.s == bytes + 7);
	CHECK(range_7.e == bytes + 8);
	CHECK(range_7.first() == bytes + 7);
	CHECK(range_7.last() == bytes + 7);
	CHECK(!range_7.contains(bytes + 6));
	CHECK(range_7.contains(bytes + 7));
	CHECK(!range_7.contains(bytes + 8));

	CHECK_SYMMETRIC_EQUAL(range_5678, range_5678);
	CHECK_SYMMETRIC_INEQUAL(range_5678, range_7);
	CHECK_SYMMETRIC_INEQUAL(range_5678, range_34);
	CHECK_SYMMETRIC_INEQUAL(range_7, range_34);

	CHECK(range_5678.contains(range_5678));
	CHECK(!range_5678.contains(range_34));
	CHECK(range_5678.contains(range_7));
	CHECK(!range_34.contains(range_5678));
	CHECK(range_34.contains(range_34));
	CHECK(!range_34.contains(range_7));
	CHECK(!range_7.contains(range_5678));
	CHECK(!range_7.contains(range_34));
	CHECK(range_7.contains(range_7));

	CHECK(range_5678.intersects(range_5678));
	CHECK(!range_5678.intersects(range_34));
	CHECK(range_5678.intersects(range_7));
	CHECK(!range_34.intersects(range_5678));
	CHECK(range_34.intersects(range_34));
	CHECK(!range_34.intersects(range_7));
	CHECK(range_7.intersects(range_5678));
	CHECK(!range_7.intersects(range_34));
	CHECK(range_7.intersects(range_7));

	CHECK(!range_5678.adjacent(range_5678));
	CHECK(range_5678.adjacent(range_34));
	CHECK(!range_5678.adjacent(range_7));
	CHECK(range_34.adjacent(range_5678));
	CHECK(!range_34.adjacent(range_34));
	CHECK(!range_34.adjacent(range_7));
	CHECK(!range_7.adjacent(range_5678));
	CHECK(!range_7.adjacent(range_34));
	CHECK(!range_7.adjacent(range_7));

	CHECK(range_5678.intersects_or_adjacent(range_5678));
	CHECK(range_5678.intersects_or_adjacent(range_34));
	CHECK(range_5678.intersects_or_adjacent(range_7));
	CHECK(range_34.intersects_or_adjacent(range_5678));
	CHECK(range_34.intersects_or_adjacent(range_34));
	CHECK(!range_34.intersects_or_adjacent(range_7));
	CHECK(range_7.intersects_or_adjacent(range_5678));
	CHECK(!range_7.intersects_or_adjacent(range_34));
	CHECK(range_7.intersects_or_adjacent(range_7));

	const auto range_345678 = integral_range{ bytes + 3, bytes + 9 };
	CHECK_SYMMETRIC_EQUAL(range_5678.union_with(range_34), range_345678);
	CHECK_SYMMETRIC_EQUAL(range_34.union_with(range_5678), range_345678);
	CHECK_SYMMETRIC_EQUAL(range_5678.union_with(range_7), range_5678);
	CHECK_SYMMETRIC_EQUAL(range_7.union_with(range_5678), range_5678);

	std::array<std::byte*, 4> vals;
	const std::array<std::byte*, 4> expected{ { bytes + 5, bytes + 6, bytes + 7, bytes + 8 } };
	auto v = vals.data();
	for (auto i : range_5678)
		*v++ = i;
	CHECK(v == vals.data() + vals.size());
	CHECK(vals == expected);
}
