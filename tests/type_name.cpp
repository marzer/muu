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

template <auto... N>
struct nttp_list;

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

	#define CHECK_TYPE_NAME_EXPLICIT(str, ...) CHECK(str ""sv == type_name<__VA_ARGS__>.view())

	CHECK_TYPE_NAME_EXPLICIT("kek", named);
	CHECK_TYPE_NAME_EXPLICIT("kek_ptr", named*);
	CHECK_TYPE_NAME_EXPLICIT("kek* const", named* const);
	CHECK_TYPE_NAME_EXPLICIT("kek* volatile", named* volatile);
	CHECK_TYPE_NAME_EXPLICIT("kek* const volatile", named* const volatile);
	CHECK_TYPE_NAME_EXPLICIT("kek&", named&);
	CHECK_TYPE_NAME_EXPLICIT("kek&&", named &&);
	CHECK_TYPE_NAME_EXPLICIT("kek[]", named[]);
	CHECK_TYPE_NAME_EXPLICIT("kek[5]", named[5]);
	CHECK_TYPE_NAME_EXPLICIT("kek(&)[]", named(&)[]);
	CHECK_TYPE_NAME_EXPLICIT("kek(&)[5]", named(&)[5]);
	CHECK_TYPE_NAME_EXPLICIT("kek(&&)[]", named(&&)[]);
	CHECK_TYPE_NAME_EXPLICIT("kek(&&)[5]", named(&&)[5]);

	CHECK_TYPE_NAME_EXPLICIT("muu::type_list<>", type_list<>);
	CHECK_TYPE_NAME_EXPLICIT("muu::type_list<int>", type_list<int>);
	CHECK_TYPE_NAME_EXPLICIT("muu::type_list<int, float>", type_list<int, float>);
	CHECK_TYPE_NAME_EXPLICIT("muu::type_list<int, float, double>", type_list<int, float, double>);
	CHECK_TYPE_NAME_EXPLICIT("muu::type_list<int, float, kek>", type_list<int, float, named>);

	CHECK_TYPE_NAME_EXPLICIT("nttp_list<>", nttp_list<>);
	CHECK_TYPE_NAME_EXPLICIT("nttp_list<1>", nttp_list<1>);
	CHECK_TYPE_NAME_EXPLICIT("nttp_list<1, 2>", nttp_list<1, 2u>);
	CHECK_TYPE_NAME_EXPLICIT("nttp_list<1, 2, 3>", nttp_list<1, 2u, 3_sz>);

	CHECK_TYPE_NAME_EXPLICIT("std::vector<int>", std::vector<int>);
	CHECK_TYPE_NAME_EXPLICIT("std::vector<kek>", std::vector<named>);

	#if !MUU_MSVC
	// msvc is bad at this: https://developercommunity.visualstudio.com/t/C-template-partial-specialization-fail/1533888
	CHECK_TYPE_NAME_EXPLICIT("muu::vector<float, 3>", muu::vector<float, 3>);
	CHECK_TYPE_NAME_EXPLICIT("muu::matrix<float, 3, 3>", muu::matrix<float, 3, 3>);
	#endif
}

#endif // !MUU_GCC || MUU_GCC > 7
