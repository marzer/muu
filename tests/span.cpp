// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/span.h"

static_assert(sizeof(span<int>) == sizeof(int*) + sizeof(size_t));
static_assert(std::is_trivially_copyable_v<span<int>>);
static_assert(std::is_trivially_destructible_v<span<int>>);

static_assert(sizeof(span<int, 10>) == sizeof(int*));
static_assert(std::is_trivially_copyable_v<span<int, 10>>);
static_assert(std::is_trivially_destructible_v<span<int, 10>>);

TEST_CASE("span")
{
	std::vector<int> vec{ 1, 2, 3, 4, 5 };
	int arr[]{ 1, 2, 3, 4, 5 };
	std::array std_arr{ 1, 2, 3, 4, 5 };
	const auto& const_std_arr = std_arr;

	SECTION("default - size zero") // (1)
	{
		span<std::byte, 0> s;
		static_assert(decltype(s)::extent == 0);
		static_assert(sizeof(s) == sizeof(void*));
		static_assert(std::is_same_v<decltype(s)::element_type, std::byte>);

		CHECK(s.size() == 0);
		CHECK(s.size_bytes() == 0);
		CHECK(s.data() == nullptr);
		CHECK(s.empty());
	}
	SECTION("default - dynamic extent") // (1)
	{
		span<std::byte> s;
		static_assert(decltype(s)::extent == dynamic_extent);
		static_assert(sizeof(s) == sizeof(void*) + sizeof(size_t));
		static_assert(std::is_same_v<decltype(s)::element_type, std::byte>);

		CHECK(s.size() == 0);
		CHECK(s.size_bytes() == 0);
		CHECK(s.data() == nullptr);
		CHECK(s.empty());
	}
	SECTION("iterator + size") // (2)
	{
		span s{ vec.begin(), 5 };
		static_assert(std::is_same_v<decltype(s), span<int, dynamic_extent>>);
		static_assert(sizeof(s) == sizeof(void*) + sizeof(size_t));
		static_assert(decltype(s)::extent == dynamic_extent);
		static_assert(std::is_same_v<decltype(s)::element_type, int>);

		CHECK(s.size() == 5);
		CHECK(s.size_bytes() == 5 * sizeof(int));
		CHECK(s.data() == vec.data());
		CHECK(!s.empty());
		for (size_t i = 0; i < 5; i++)
		{
			CHECK(s[i] == vec[i]);
			CHECK(&s[i] == &vec[i]);
		}
	}
	SECTION("begin + end iterators") // (3)
	{
		span s{ vec.begin(), vec.end() };
		static_assert(std::is_same_v<decltype(s), span<int, dynamic_extent>>);
		static_assert(sizeof(s) == sizeof(void*) + sizeof(size_t));
		static_assert(decltype(s)::extent == dynamic_extent);
		static_assert(std::is_same_v<decltype(s)::element_type, int>);

		CHECK(s.size() == 5);
		CHECK(s.size_bytes() == 5 * sizeof(int));
		CHECK(s.data() == vec.data());
		CHECK(!s.empty());
		for (size_t i = 0; i < 5; i++)
		{
			CHECK(s[i] == vec[i]);
			CHECK(&s[i] == &vec[i]);
		}
	}
	SECTION("begin + end pointers") // (3)
	{
		span s{ vec.data(), vec.data() + vec.size() };
		static_assert(std::is_same_v<decltype(s), span<int, dynamic_extent>>);
		static_assert(sizeof(s) == sizeof(void*) + sizeof(size_t));
		static_assert(decltype(s)::extent == dynamic_extent);
		static_assert(std::is_same_v<decltype(s)::element_type, int>);

		CHECK(s.size() == 5);
		CHECK(s.size_bytes() == 5 * sizeof(int));
		CHECK(s.data() == vec.data());
		CHECK(!s.empty());
		for (size_t i = 0; i < 5; i++)
		{
			CHECK(s[i] == vec[i]);
			CHECK(&s[i] == &vec[i]);
		}
	}
	SECTION("C array") // (4)
	{
		span s{ arr };
		static_assert(std::is_same_v<decltype(s), span<int, 5>>);
		static_assert(sizeof(s) == sizeof(void*));
		static_assert(decltype(s)::extent == 5);
		static_assert(std::is_same_v<decltype(s)::element_type, int>);

		CHECK(s.size() == 5);
		CHECK(s.size_bytes() == 5 * sizeof(int));
		CHECK(s.data() == arr);
		CHECK(!s.empty());
		for (size_t i = 0; i < 5; i++)
		{
			CHECK(s[i] == arr[i]);
			CHECK(&s[i] == &arr[i]);
		}
	}
	SECTION("std::array") // (5)
	{
		span s{ std_arr };
		static_assert(std::is_same_v<decltype(s), span<int, 5>>);
		static_assert(sizeof(s) == sizeof(void*));
		static_assert(decltype(s)::extent == 5);
		static_assert(std::is_same_v<decltype(s)::element_type, int>);

		CHECK(s.size() == 5);
		CHECK(s.size_bytes() == 5 * sizeof(int));
		CHECK(s.data() == std_arr.data());
		CHECK(!s.empty());
		for (size_t i = 0; i < 5; i++)
		{
			CHECK(s[i] == std_arr[i]);
			CHECK(&s[i] == &std_arr[i]);
		}
	}
	SECTION("const std::array") // (5)
	{
		span s{ const_std_arr };
		static_assert(std::is_same_v<decltype(s), span<const int, 5>>);
		static_assert(sizeof(s) == sizeof(void*));
		static_assert(decltype(s)::extent == 5);
		static_assert(std::is_same_v<decltype(s)::element_type, const int>);

		CHECK(s.size() == 5);
		CHECK(s.size_bytes() == 5 * sizeof(int));
		CHECK(s.data() == const_std_arr.data());
		CHECK(!s.empty());
		for (size_t i = 0; i < 5; i++)
		{
			CHECK(s[i] == const_std_arr[i]);
			CHECK(&s[i] == &const_std_arr[i]);
		}
	}
}
