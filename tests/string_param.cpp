// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/string_param.h"

template <typename T, size_t N>
static void string_param_test(const T(&str_)[N]) noexcept
{
	// make sure it's a null-terminated string literal
	static_assert(N >= 1u);
	CHECK(str_[N - 1u] == T{});

	using string = std::basic_string<T>;
	using string_view = std::basic_string_view<T>;
	using char_traits = std::char_traits<T>;

	constexpr auto actual_length = N - 1u;
	constexpr auto actual_empty = !actual_length;
	const auto strlen_length = char_traits::length(str_); // different if there's a null in the middle of the string
	CHECK(strlen_length <= actual_length);

	INFO(nameof<T>)

	{
		INFO("string_view")

		auto source = string_view{ str_, actual_length };
		string_param s{ source };
		CHECK(s.empty() == actual_empty);
		CHECK(s.owning() == false);
		CHECK(string_view{ s } == source);
	}

	{
		INFO("const string&")

		const auto source = string( str_, actual_length);
		string_param s{ source };
		CHECK(s.empty() == actual_empty);
		CHECK(s.owning() == false);
		CHECK(string_view{ s } == source);
	}

	{
		INFO("const Char*, size_t")

		const T* source = str_;
		string_param s{ source, actual_length };
		CHECK(s.empty() == actual_empty);
		CHECK(s.owning() == false);
		CHECK(string_view{ s } == string_view{ source, actual_length });
	}

	{
		INFO("const Char*")

		const T* source = str_;
		string_param s{ source };
		CHECK(s.empty() == !strlen_length);
		CHECK(s.owning() == false);
		CHECK(string_view{ s } == string_view{ source });
	}

	{
		INFO("string&&")

		auto source = string(str_, actual_length);
		string_param s{ std::move(source) };
		CHECK(s.empty() == actual_empty);
		CHECK(s.owning() == !actual_empty);
	}
}

TEST_CASE("string_param")
{
	{
		INFO("empty string")
		string_param_test("");
		string_param_test(L"");
		string_param_test(u8"");
		string_param_test(u"");
		string_param_test(U"");
	}

	{
		INFO("\"hello\"")
		string_param_test("hello");
		string_param_test(L"hello");
		string_param_test(u8"hello");
		string_param_test(u"hello");
		string_param_test(U"hello");
	}

	{
		INFO("\"hello world\"")
		string_param_test("hello world");
		string_param_test(L"hello world");
		string_param_test(u8"hello world");
		string_param_test(u"hello world");
		string_param_test(U"hello world");
	}

	{
		INFO("\"hello\\0world\"")
		string_param_test("hello\0world");
		string_param_test(L"hello\0world");
		string_param_test(u8"hello\0world");
		string_param_test(u"hello\0world");
		string_param_test(U"hello\0world");
	}
}


inline constexpr std::string_view kek = "hello\0world"sv;
