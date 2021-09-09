// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/type_name.h"

#if !MUU_GCC || MUU_GCC > 7

struct test_struct_1
{};

namespace
{
	struct test_struct_2
	{};
}

namespace foo
{
	struct test_struct_3
	{};

	namespace
	{
		struct test_struct_4
		{};
	}
}

struct named
{};

namespace muu
{
	template <>
	inline constexpr auto type_name<named> = static_string{ "kek" };

	template <>
	inline constexpr auto type_name<named*> = static_string{ "kek_ptr" };
}

TEST_CASE("type_name")
{
	#define CHECK_TYPE_NAME(T) CHECK(type_name<T>.view() == MUU_MAKE_STRING_VIEW(T))

	#define CHECK_TYPE_NAME_PERMUTATION(T)                                                                             \
		CHECK_TYPE_NAME(T);                                                                                            \
		CHECK_TYPE_NAME(T*);                                                                                           \
		CHECK_TYPE_NAME(T* const);                                                                                     \
		CHECK_TYPE_NAME(T* volatile);                                                                                  \
		CHECK_TYPE_NAME(T* const volatile);                                                                            \
		CHECK_TYPE_NAME(T**);                                                                                          \
		CHECK_TYPE_NAME(T** const);                                                                                    \
		CHECK_TYPE_NAME(T** volatile);                                                                                 \
		CHECK_TYPE_NAME(T** const volatile);                                                                           \
		CHECK_TYPE_NAME(T* const* const);                                                                              \
		CHECK_TYPE_NAME(T* const* volatile);                                                                           \
		CHECK_TYPE_NAME(T* const* const volatile);                                                                     \
		CHECK_TYPE_NAME(T* volatile* const);                                                                           \
		CHECK_TYPE_NAME(T* volatile* volatile);                                                                        \
		CHECK_TYPE_NAME(T* volatile* const volatile);                                                                  \
		CHECK_TYPE_NAME(T* const volatile* const);                                                                     \
		CHECK_TYPE_NAME(T* const volatile* volatile);                                                                  \
		CHECK_TYPE_NAME(T* const volatile* const volatile);                                                            \
		CHECK_TYPE_NAME(T&);                                                                                           \
		CHECK_TYPE_NAME(T&&);                                                                                          \
		CHECK_TYPE_NAME(T*&);                                                                                          \
		CHECK_TYPE_NAME(T*&&);                                                                                         \
		CHECK_TYPE_NAME(T[]);                                                                                          \
		CHECK_TYPE_NAME(T[5]);                                                                                         \
		CHECK_TYPE_NAME(T(&)[]);                                                                                       \
		CHECK_TYPE_NAME(T(&)[5]);                                                                                      \
		CHECK_TYPE_NAME(T(&&)[]);                                                                                      \
		CHECK_TYPE_NAME(T(&&)[5])

	#define CHECK_ALL_TYPE_NAME_PERMUTATIONS(T)                                                                        \
		CHECK_TYPE_NAME(T);                                                                                            \
		CHECK_TYPE_NAME(const T);                                                                                      \
		CHECK_TYPE_NAME(volatile T);                                                                                   \
		CHECK_TYPE_NAME(const volatile T)

	CHECK_ALL_TYPE_NAME_PERMUTATIONS(int);
	CHECK_ALL_TYPE_NAME_PERMUTATIONS(test_struct_1);
	CHECK_ALL_TYPE_NAME_PERMUTATIONS(test_struct_2);
	CHECK_ALL_TYPE_NAME_PERMUTATIONS(foo::test_struct_3);
	// CHECK_ALL_TYPE_NAME_PERMUTATIONS(foo::test_struct_4);

	#define CHECK_CUSTOM_TYPE_NAME(T, str) CHECK(type_name<T>.view() == str ""sv)

	CHECK_CUSTOM_TYPE_NAME(named, "kek");
	CHECK_CUSTOM_TYPE_NAME(named*, "kek_ptr");
	CHECK_CUSTOM_TYPE_NAME(named* const, "kek* const");
	CHECK_CUSTOM_TYPE_NAME(named* volatile, "kek* volatile");
	CHECK_CUSTOM_TYPE_NAME(named* const volatile, "kek* const volatile");
	CHECK_CUSTOM_TYPE_NAME(named&, "kek&");
	CHECK_CUSTOM_TYPE_NAME(named&&, "kek&&");
	CHECK_CUSTOM_TYPE_NAME(named[], "kek[]");
	CHECK_CUSTOM_TYPE_NAME(named[5], "kek[5]");
	CHECK_CUSTOM_TYPE_NAME(named(&)[], "kek(&)[]");
	CHECK_CUSTOM_TYPE_NAME(named(&)[5], "kek(&)[5]");
	CHECK_CUSTOM_TYPE_NAME(named(&&)[], "kek(&&)[]");
	CHECK_CUSTOM_TYPE_NAME(named(&&)[5], "kek(&&)[5]");
}

#endif // !MUU_GCC || MUU_GCC > 7
