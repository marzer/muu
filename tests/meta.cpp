// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/core.h"

MUU_DISABLE_SPAM_WARNINGS;

//======================================================================================================================
// these are all the public metafunctions and type traits in core_meta.h (ideally in the order they appear)
//======================================================================================================================

template <size_t align, typename T = char>
struct aligned
{
	alignas(align) T kek;
};
enum an_enum : int
{
	an_enum_one,
	an_enum_two,
	an_enum_three
};
enum class an_enum_class : unsigned
{
	one,
	two,
	three
};
using not_an_enum = double;

// remove_cvref
static_assert(std::is_same_v<remove_cvref<int>, int>);
static_assert(std::is_same_v<remove_cvref<int&>, int>);
static_assert(std::is_same_v<remove_cvref<int&&>, int>);
static_assert(std::is_same_v<remove_cvref<const int>, int>);
static_assert(std::is_same_v<remove_cvref<const int&>, int>);
static_assert(std::is_same_v<remove_cvref<const int&&>, int>);
static_assert(std::is_same_v<remove_cvref<volatile int>, int>);
static_assert(std::is_same_v<remove_cvref<volatile int&>, int>);
static_assert(std::is_same_v<remove_cvref<volatile int&&>, int>);
static_assert(std::is_same_v<remove_cvref<const volatile int>, int>);
static_assert(std::is_same_v<remove_cvref<const volatile int&>, int>);
static_assert(std::is_same_v<remove_cvref<const volatile int&&>, int>);

// remove_enum
static_assert(std::is_same_v<remove_enum<an_enum>, int>);
static_assert(std::is_same_v<remove_enum<an_enum&>, int&>);
static_assert(std::is_same_v<remove_enum<an_enum&&>, int&&>);
static_assert(std::is_same_v<remove_enum<const an_enum>, const int>);
static_assert(std::is_same_v<remove_enum<const an_enum&>, const int&>);
static_assert(std::is_same_v<remove_enum<const an_enum&&>, const int&&>);
static_assert(std::is_same_v<remove_enum<an_enum_class>, unsigned>);
static_assert(std::is_same_v<remove_enum<an_enum_class&>, unsigned&>);
static_assert(std::is_same_v<remove_enum<an_enum_class&&>, unsigned&&>);
static_assert(std::is_same_v<remove_enum<const an_enum_class>, const unsigned>);
static_assert(std::is_same_v<remove_enum<const an_enum_class&>, const unsigned&>);
static_assert(std::is_same_v<remove_enum<const an_enum_class&&>, const unsigned&&>);
static_assert(std::is_same_v<remove_enum<not_an_enum>, not_an_enum>);
static_assert(std::is_same_v<remove_enum<not_an_enum&>, not_an_enum&>);
static_assert(std::is_same_v<remove_enum<not_an_enum&&>, not_an_enum&&>);
static_assert(std::is_same_v<remove_enum<const not_an_enum>, const not_an_enum>);
static_assert(std::is_same_v<remove_enum<const not_an_enum&>, const not_an_enum&>);
static_assert(std::is_same_v<remove_enum<const not_an_enum&&>, const not_an_enum&&>);

// remove_lvalue_reference
static_assert(std::is_same_v<remove_lvalue_reference<int>, int>);
static_assert(std::is_same_v<remove_lvalue_reference<int&>, int>);
static_assert(std::is_same_v<remove_lvalue_reference<int&&>, int&&>);

// remove_rvalue_reference
static_assert(std::is_same_v<remove_rvalue_reference<int>, int>);
static_assert(std::is_same_v<remove_rvalue_reference<int&>, int&>);
static_assert(std::is_same_v<remove_rvalue_reference<int&&>, int>);

// largest
static_assert(std::is_same_v<largest<char, char[2], char[4], char[128]>, char[128]>);
static_assert(std::is_same_v<largest<char, char[2], char[4]>, char[4]>);
static_assert(std::is_same_v<largest<char, char[2]>, char[2]>);
static_assert(std::is_same_v<largest<char>, char>);

// smallest
static_assert(std::is_same_v<smallest<char, char[2], char[4], char[128]>, char>);
static_assert(std::is_same_v<smallest<char[2], char[4], char[128]>, char[2]>);
static_assert(std::is_same_v<smallest<char[4], char[128]>, char[4]>);
static_assert(std::is_same_v<smallest<char[128]>, char[128]>);

// total_size
static_assert(total_size<uint8_t> == 1);
static_assert(total_size<uint8_t, int8_t> == 2);
static_assert(total_size<uint8_t, int8_t, char> == 3);
static_assert(total_size<uint8_t, int8_t, char, std::byte> == 4);
static_assert(total_size<uint8_t, int8_t, char, std::byte, uint16_t> == 6);

// alignment_of
static_assert(alignment_of<aligned<1>> == 1);
static_assert(alignment_of<aligned<2>> == 2);
static_assert(alignment_of<aligned<4>> == 4);
static_assert(alignment_of<aligned<8>> == 8);
static_assert(alignment_of<aligned<128>> == 128);
static_assert(alignment_of<void> == 0);
static_assert(alignment_of<const void> == 0);
static_assert(alignment_of<const volatile void> == 0);
static_assert(alignment_of<int()> == 0);
static_assert(alignment_of<int() noexcept> == 0);

// most_aligned
static_assert(std::is_same_v<most_aligned<void, aligned<1>, aligned<2>, aligned<4>, aligned<128>>, aligned<128>>);
static_assert(std::is_same_v<most_aligned<void, aligned<1>, aligned<2>, aligned<4>>, aligned<4>>);
static_assert(std::is_same_v<most_aligned<void, aligned<1>, aligned<2>>, aligned<2>>);
static_assert(std::is_same_v<most_aligned<void, aligned<1>>, aligned<1>>);
static_assert(std::is_same_v<most_aligned<void>, void>);

// least_aligned
static_assert(std::is_same_v<least_aligned<void, aligned<1>, aligned<2>, aligned<4>, aligned<128>>, void>);
static_assert(std::is_same_v<least_aligned<void, aligned<2>, aligned<4>, aligned<128>>, void>);
static_assert(std::is_same_v<least_aligned<void, aligned<4>, aligned<128>>, void>);
static_assert(std::is_same_v<least_aligned<void, aligned<128>>, void>);
static_assert(std::is_same_v<least_aligned<void>, void>);
static_assert(std::is_same_v<least_aligned<aligned<1>, aligned<2>, aligned<4>, aligned<128>>, aligned<1>>);
static_assert(std::is_same_v<least_aligned<aligned<2>, aligned<4>, aligned<128>>, aligned<2>>);
static_assert(std::is_same_v<least_aligned<aligned<4>, aligned<128>>, aligned<4>>);
static_assert(std::is_same_v<least_aligned<aligned<128>>, aligned<128>>);

// any_same (variadic std::is_same_v || ...)
static_assert(!any_same<int>);
static_assert(any_same<int, int>);
static_assert(any_same<int, int, float>);
static_assert(any_same<int, int, float, double>);
static_assert(any_same<int, float, int, double>);
static_assert(!any_same<int&, int>);
static_assert(!any_same<int&, int, float>);
static_assert(!any_same<int&, int, float, double>);
static_assert(!any_same<int&, float, int, double>);

// all_same (variadic std::is_same_v && ...)
static_assert(all_same<int>);
static_assert(all_same<int, int>);
static_assert(all_same<int, int, int>);
static_assert(all_same<int, int, int, int>);
static_assert(all_same<int, int, int, int, int>);
static_assert(!all_same<int, int&>);
static_assert(!all_same<int, int, int&>);
static_assert(!all_same<int, int, int, int&>);
static_assert(!all_same<int, int, int, int, int&>);
static_assert(!all_same<int, int, float>);
static_assert(!all_same<int, int, float, double>);
static_assert(!all_same<int, float, int, double>);
static_assert(!all_same<int&, int>);
static_assert(!all_same<int&, int, float>);
static_assert(!all_same<int&, int, float, double>);
static_assert(!all_same<int&, float, int, double>);

// is_enum
static_assert(is_enum<an_enum>);
static_assert(is_enum<const an_enum>);
static_assert(is_enum<volatile an_enum>);
static_assert(is_enum<const volatile an_enum>);
static_assert(is_enum<an_enum&>);
static_assert(is_enum<an_enum&>);
static_assert(is_enum<const an_enum&>);
static_assert(is_enum<volatile an_enum&>);
static_assert(is_enum<const volatile an_enum&>);
static_assert(is_enum<an_enum&&>);
static_assert(is_enum<an_enum&&>);
static_assert(is_enum<const an_enum&&>);
static_assert(is_enum<volatile an_enum&&>);
static_assert(is_enum<const volatile an_enum&&>);
static_assert(is_enum<an_enum_class>);
static_assert(is_enum<const an_enum_class>);
static_assert(is_enum<volatile an_enum_class>);
static_assert(is_enum<const volatile an_enum_class>);
static_assert(is_enum<an_enum_class&>);
static_assert(is_enum<an_enum_class&>);
static_assert(is_enum<const an_enum_class&>);
static_assert(is_enum<volatile an_enum_class&>);
static_assert(is_enum<const volatile an_enum_class&>);
static_assert(is_enum<an_enum_class&&>);
static_assert(is_enum<an_enum_class&&>);
static_assert(is_enum<const an_enum_class&&>);
static_assert(is_enum<volatile an_enum_class&&>);
static_assert(is_enum<const volatile an_enum_class&&>);
static_assert(!is_enum<int>);
static_assert(!is_enum<not_an_enum>);
static_assert(!is_enum<const not_an_enum>);
static_assert(!is_enum<volatile not_an_enum>);
static_assert(!is_enum<const volatile not_an_enum>);
static_assert(!is_enum<int&>);
static_assert(!is_enum<not_an_enum&>);
static_assert(!is_enum<const not_an_enum&>);
static_assert(!is_enum<volatile not_an_enum&>);
static_assert(!is_enum<const volatile not_an_enum&>);
static_assert(!is_enum<int&&>);
static_assert(!is_enum<not_an_enum&&>);
static_assert(!is_enum<const not_an_enum&&>);
static_assert(!is_enum<volatile not_an_enum&&>);
static_assert(!is_enum<const volatile not_an_enum&&>);

// is_scoped_enum
static_assert(is_scoped_enum<an_enum_class>);
static_assert(is_scoped_enum<an_enum_class&>);
static_assert(is_scoped_enum<an_enum_class&&>);
static_assert(!is_scoped_enum<an_enum>);
static_assert(!is_scoped_enum<an_enum&>);
static_assert(!is_scoped_enum<an_enum&&>);
static_assert(!is_scoped_enum<not_an_enum>);
static_assert(!is_scoped_enum<not_an_enum&>);
static_assert(!is_scoped_enum<not_an_enum&&>);

// is_legacy_enum
static_assert(!is_legacy_enum<an_enum_class>);
static_assert(!is_legacy_enum<an_enum_class&>);
static_assert(!is_legacy_enum<an_enum_class&&>);
static_assert(is_legacy_enum<an_enum>);
static_assert(is_legacy_enum<an_enum&>);
static_assert(is_legacy_enum<an_enum&&>);
static_assert(!is_legacy_enum<not_an_enum>);
static_assert(!is_legacy_enum<not_an_enum&>);
static_assert(!is_legacy_enum<not_an_enum&&>);

// is_unsigned
static_assert(is_unsigned<unsigned>);
static_assert(is_unsigned<an_enum_class>);
static_assert(is_unsigned<const unsigned>);
static_assert(is_unsigned<const an_enum_class>);
static_assert(is_unsigned<unsigned&>);
static_assert(is_unsigned<an_enum_class&>);
static_assert(is_unsigned<unsigned&&>);
static_assert(is_unsigned<an_enum_class&&>);
static_assert(!is_unsigned<int>);
static_assert(!is_unsigned<half>);
static_assert(!is_unsigned<float>);
static_assert(!is_unsigned<an_enum>);
static_assert(!is_unsigned<double>);

// any_unsigned
static_assert(any_unsigned<unsigned>);
static_assert(any_unsigned<an_enum_class>);
static_assert(any_unsigned<const unsigned>);
static_assert(any_unsigned<const an_enum_class>);
static_assert(any_unsigned<unsigned&>);
static_assert(any_unsigned<an_enum_class&>);
static_assert(any_unsigned<unsigned&&>);
static_assert(any_unsigned<an_enum_class&&>);
static_assert(!any_unsigned<int>);
static_assert(!any_unsigned<half>);
static_assert(!any_unsigned<float>);
static_assert(!any_unsigned<an_enum>);
static_assert(!any_unsigned<double>);
static_assert(any_unsigned<unsigned, int>);
static_assert(any_unsigned<an_enum_class, int>);
static_assert(any_unsigned<const unsigned, int>);
static_assert(any_unsigned<const an_enum_class, int>);
static_assert(any_unsigned<unsigned&, int>);
static_assert(any_unsigned<an_enum_class&, int>);
static_assert(any_unsigned<unsigned&&, int>);
static_assert(any_unsigned<an_enum_class&&, int>);
static_assert(!any_unsigned<int, int>);
static_assert(!any_unsigned<half, int>);
static_assert(!any_unsigned<float, int>);
static_assert(!any_unsigned<an_enum, int>);
static_assert(!any_unsigned<double, int>);

// all_unsigned
static_assert(all_unsigned<unsigned>);
static_assert(all_unsigned<an_enum_class>);
static_assert(all_unsigned<const unsigned>);
static_assert(all_unsigned<const an_enum_class>);
static_assert(all_unsigned<unsigned&>);
static_assert(all_unsigned<an_enum_class&>);
static_assert(all_unsigned<unsigned&&>);
static_assert(all_unsigned<an_enum_class&&>);
static_assert(!all_unsigned<int>);
static_assert(!all_unsigned<half>);
static_assert(!all_unsigned<float>);
static_assert(!all_unsigned<an_enum>);
static_assert(!all_unsigned<double>);
static_assert(all_unsigned<unsigned, unsigned>);
static_assert(all_unsigned<an_enum_class, unsigned>);
static_assert(all_unsigned<const unsigned, unsigned>);
static_assert(all_unsigned<const an_enum_class, unsigned>);
static_assert(all_unsigned<unsigned&, unsigned>);
static_assert(all_unsigned<an_enum_class&, unsigned>);
static_assert(all_unsigned<unsigned&&, unsigned>);
static_assert(all_unsigned<an_enum_class&&, unsigned>);
static_assert(!all_unsigned<unsigned, int>);
static_assert(!all_unsigned<an_enum_class, int>);
static_assert(!all_unsigned<const unsigned, int>);
static_assert(!all_unsigned<const an_enum_class, int>);
static_assert(!all_unsigned<unsigned&, int>);
static_assert(!all_unsigned<an_enum_class&, int>);
static_assert(!all_unsigned<unsigned&&, int>);
static_assert(!all_unsigned<an_enum_class&&, int>);

// is_signed
static_assert(is_signed<int>);
static_assert(is_signed<half>);
static_assert(is_signed<float>);
static_assert(is_signed<an_enum>);
static_assert(is_signed<double>);
static_assert(is_signed<const int>);
static_assert(is_signed<const half>);
static_assert(is_signed<const float>);
static_assert(is_signed<const an_enum>);
static_assert(is_signed<const double>);
static_assert(is_signed<int&>);
static_assert(is_signed<half&>);
static_assert(is_signed<float&>);
static_assert(is_signed<an_enum&>);
static_assert(is_signed<double&>);
static_assert(is_signed<int&&>);
static_assert(is_signed<half&&>);
static_assert(is_signed<float&&>);
static_assert(is_signed<an_enum&&>);
static_assert(is_signed<double&&>);
static_assert(!is_signed<unsigned>);
static_assert(!is_signed<an_enum_class>);
static_assert(!is_signed<const unsigned>);
static_assert(!is_signed<const an_enum_class>);
static_assert(!is_signed<unsigned&>);
static_assert(!is_signed<an_enum_class&>);

// is_integral
static_assert(is_integral<bool>);
static_assert(is_integral<short>);
static_assert(is_integral<int>);
static_assert(is_integral<long>);
static_assert(is_integral<long long>);
static_assert(is_integral<unsigned short>);
static_assert(is_integral<unsigned>);
static_assert(is_integral<unsigned long>);
static_assert(is_integral<unsigned long long>);
static_assert(is_integral<an_enum_class>);
static_assert(is_integral<an_enum>);
static_assert(!is_integral<half>);
static_assert(!is_integral<float>);
static_assert(!is_integral<double>);
static_assert(is_integral<short&>);
static_assert(is_integral<int&>);
static_assert(is_integral<long&>);
static_assert(is_integral<long long&>);
static_assert(is_integral<unsigned short&>);
static_assert(is_integral<unsigned&>);
static_assert(is_integral<unsigned long&>);
static_assert(is_integral<unsigned long long&>);
static_assert(is_integral<an_enum_class&>);
static_assert(is_integral<an_enum&>);
static_assert(!is_integral<half&>);
static_assert(!is_integral<float&>);
static_assert(!is_integral<double&>);

// is_integer
static_assert(!is_integer<bool>);
static_assert(is_integer<short>);
static_assert(is_integer<int>);
static_assert(is_integer<long>);
static_assert(is_integer<long long>);
static_assert(is_integer<unsigned short>);
static_assert(is_integer<unsigned>);
static_assert(is_integer<unsigned long>);
static_assert(is_integer<unsigned long long>);
static_assert(!is_integer<an_enum_class>);
static_assert(!is_integer<an_enum>);
static_assert(!is_integer<half>);
static_assert(!is_integer<float>);
static_assert(!is_integer<double>);
static_assert(is_integer<short&>);
static_assert(is_integer<int&>);
static_assert(is_integer<long&>);
static_assert(is_integer<long long&>);
static_assert(is_integer<unsigned short&>);
static_assert(is_integer<unsigned&>);
static_assert(is_integer<unsigned long&>);
static_assert(is_integer<unsigned long long&>);
static_assert(!is_integer<an_enum_class&>);
static_assert(!is_integer<an_enum&>);
static_assert(!is_integer<half&>);
static_assert(!is_integer<float&>);
static_assert(!is_integer<double&>);

// is_floating_point
static_assert(!is_floating_point<bool>);
static_assert(!is_floating_point<short>);
static_assert(!is_floating_point<int>);
static_assert(!is_floating_point<long>);
static_assert(!is_floating_point<long long>);
static_assert(!is_floating_point<unsigned short>);
static_assert(!is_floating_point<unsigned>);
static_assert(!is_floating_point<unsigned long>);
static_assert(!is_floating_point<unsigned long long>);
static_assert(!is_floating_point<an_enum_class>);
static_assert(!is_floating_point<an_enum>);
static_assert(is_floating_point<half>);
static_assert(is_floating_point<float>);
static_assert(is_floating_point<double>);
static_assert(!is_floating_point<short&>);
static_assert(!is_floating_point<int&>);
static_assert(!is_floating_point<long&>);
static_assert(!is_floating_point<long long&>);
static_assert(!is_floating_point<unsigned short&>);
static_assert(!is_floating_point<unsigned&>);
static_assert(!is_floating_point<unsigned long&>);
static_assert(!is_floating_point<unsigned long long&>);
static_assert(!is_floating_point<an_enum_class&>);
static_assert(!is_floating_point<an_enum&>);
static_assert(is_floating_point<half&>);
static_assert(is_floating_point<float&>);
static_assert(is_floating_point<double&>);

// is_arithmetic
static_assert(is_arithmetic<bool>);
static_assert(is_arithmetic<short>);
static_assert(is_arithmetic<int>);
static_assert(is_arithmetic<long>);
static_assert(is_arithmetic<long long>);
static_assert(is_arithmetic<unsigned short>);
static_assert(is_arithmetic<unsigned>);
static_assert(is_arithmetic<unsigned long>);
static_assert(is_arithmetic<unsigned long long>);
static_assert(!is_arithmetic<an_enum_class>);
static_assert(!is_arithmetic<an_enum>);
static_assert(is_arithmetic<half>);
static_assert(is_arithmetic<float>);
static_assert(is_arithmetic<double>);
static_assert(is_arithmetic<short&>);
static_assert(is_arithmetic<int&>);
static_assert(is_arithmetic<long&>);
static_assert(is_arithmetic<long long&>);
static_assert(is_arithmetic<unsigned short&>);
static_assert(is_arithmetic<unsigned&>);
static_assert(is_arithmetic<unsigned long&>);
static_assert(is_arithmetic<unsigned long long&>);
static_assert(!is_arithmetic<an_enum_class&>);
static_assert(!is_arithmetic<an_enum&>);
static_assert(is_arithmetic<half&>);
static_assert(is_arithmetic<float&>);
static_assert(is_arithmetic<double&>);

// is_const
static_assert(!is_const<int>);
static_assert(!is_const<int&>);
static_assert(!is_const<int&&>);
static_assert(is_const<const int>);
static_assert(is_const<const int&>);
static_assert(is_const<const int&&>);
static_assert(!is_const<volatile int>);
static_assert(!is_const<volatile int&>);
static_assert(!is_const<volatile int&&>);
static_assert(is_const<const volatile int>);
static_assert(is_const<const volatile int&>);
static_assert(is_const<const volatile int&&>);

// add_const
static_assert(std::is_same_v<add_const<void>, const void>);
static_assert(std::is_same_v<add_const<const void>, const void>);
static_assert(std::is_same_v<add_const<volatile void>, const volatile void>);
static_assert(std::is_same_v<add_const<const volatile void>, const volatile void>);
static_assert(std::is_same_v<add_const<int>, const int>);
static_assert(std::is_same_v<add_const<int&>, const int&>);
static_assert(std::is_same_v<add_const<int&&>, const int&&>);
static_assert(std::is_same_v<add_const<const int>, const int>);
static_assert(std::is_same_v<add_const<const int&>, const int&>);
static_assert(std::is_same_v<add_const<const int&&>, const int&&>);
static_assert(std::is_same_v<add_const<volatile int>, const volatile int>);
static_assert(std::is_same_v<add_const<volatile int&>, const volatile int&>);
static_assert(std::is_same_v<add_const<volatile int&&>, const volatile int&&>);
static_assert(std::is_same_v<add_const<const volatile int>, const volatile int>);
static_assert(std::is_same_v<add_const<const volatile int&>, const volatile int&>);
static_assert(std::is_same_v<add_const<const volatile int&&>, const volatile int&&>);

// remove_const
static_assert(std::is_same_v<remove_const<void>, void>);
static_assert(std::is_same_v<remove_const<const void>, void>);
static_assert(std::is_same_v<remove_const<volatile void>, volatile void>);
static_assert(std::is_same_v<remove_const<const volatile void>, volatile void>);
static_assert(std::is_same_v<remove_const<int>, int>);
static_assert(std::is_same_v<remove_const<int&>, int&>);
static_assert(std::is_same_v<remove_const<int&&>, int&&>);
static_assert(std::is_same_v<remove_const<const int>, int>);
static_assert(std::is_same_v<remove_const<const int&>, int&>);
static_assert(std::is_same_v<remove_const<const int&&>, int&&>);
static_assert(std::is_same_v<remove_const<volatile int>, volatile int>);
static_assert(std::is_same_v<remove_const<volatile int&>, volatile int&>);
static_assert(std::is_same_v<remove_const<volatile int&&>, volatile int&&>);
static_assert(std::is_same_v<remove_const<const volatile int>, volatile int>);
static_assert(std::is_same_v<remove_const<const volatile int&>, volatile int&>);
static_assert(std::is_same_v<remove_const<const volatile int&&>, volatile int&&>);

// set_const
static_assert(std::is_same_v<set_const<void, true>, const void>);
static_assert(std::is_same_v<set_const<const void, true>, const void>);
static_assert(std::is_same_v<set_const<volatile void, true>, const volatile void>);
static_assert(std::is_same_v<set_const<const volatile void, true>, const volatile void>);
static_assert(std::is_same_v<set_const<int, true>, const int>);
static_assert(std::is_same_v<set_const<int&, true>, const int&>);
static_assert(std::is_same_v<set_const<int&&, true>, const int&&>);
static_assert(std::is_same_v<set_const<const int, true>, const int>);
static_assert(std::is_same_v<set_const<const int&, true>, const int&>);
static_assert(std::is_same_v<set_const<const int&&, true>, const int&&>);
static_assert(std::is_same_v<set_const<volatile int, true>, const volatile int>);
static_assert(std::is_same_v<set_const<volatile int&, true>, const volatile int&>);
static_assert(std::is_same_v<set_const<volatile int&&, true>, const volatile int&&>);
static_assert(std::is_same_v<set_const<const volatile int, true>, const volatile int>);
static_assert(std::is_same_v<set_const<const volatile int&, true>, const volatile int&>);
static_assert(std::is_same_v<set_const<const volatile int&&, true>, const volatile int&&>);
static_assert(std::is_same_v<set_const<void, false>, void>);
static_assert(std::is_same_v<set_const<const void, false>, void>);
static_assert(std::is_same_v<set_const<volatile void, false>, volatile void>);
static_assert(std::is_same_v<set_const<const volatile void, false>, volatile void>);
static_assert(std::is_same_v<set_const<int, false>, int>);
static_assert(std::is_same_v<set_const<int&, false>, int&>);
static_assert(std::is_same_v<set_const<int&&, false>, int&&>);
static_assert(std::is_same_v<set_const<const int, false>, int>);
static_assert(std::is_same_v<set_const<const int&, false>, int&>);
static_assert(std::is_same_v<set_const<const int&&, false>, int&&>);
static_assert(std::is_same_v<set_const<volatile int, false>, volatile int>);
static_assert(std::is_same_v<set_const<volatile int&, false>, volatile int&>);
static_assert(std::is_same_v<set_const<volatile int&&, false>, volatile int&&>);
static_assert(std::is_same_v<set_const<const volatile int, false>, volatile int>);
static_assert(std::is_same_v<set_const<const volatile int&, false>, volatile int&>);
static_assert(std::is_same_v<set_const<const volatile int&&, false>, volatile int&&>);

// conditionally_add_const
static_assert(std::is_same_v<conditionally_add_const<void, true>, const void>);
static_assert(std::is_same_v<conditionally_add_const<const void, true>, const void>);
static_assert(std::is_same_v<conditionally_add_const<volatile void, true>, const volatile void>);
static_assert(std::is_same_v<conditionally_add_const<const volatile void, true>, const volatile void>);
static_assert(std::is_same_v<conditionally_add_const<void, false>, void>);
static_assert(std::is_same_v<conditionally_add_const<const void, false>, const void>);
static_assert(std::is_same_v<conditionally_add_const<volatile void, false>, volatile void>);
static_assert(std::is_same_v<conditionally_add_const<const volatile void, false>, const volatile void>);
static_assert(std::is_same_v<conditionally_add_const<int, true>, const int>);
static_assert(std::is_same_v<conditionally_add_const<int, false>, int>);
static_assert(std::is_same_v<conditionally_add_const<const int, true>, const int>);
static_assert(std::is_same_v<conditionally_add_const<const int, false>, const int>);
static_assert(std::is_same_v<conditionally_add_const<volatile int, true>, const volatile int>);
static_assert(std::is_same_v<conditionally_add_const<volatile int, false>, volatile int>);
static_assert(std::is_same_v<conditionally_add_const<const volatile int, true>, const volatile int>);
static_assert(std::is_same_v<conditionally_add_const<const volatile int, false>, const volatile int>);
static_assert(std::is_same_v<conditionally_add_const<int&, true>, const int&>);
static_assert(std::is_same_v<conditionally_add_const<int&, false>, int&>);
static_assert(std::is_same_v<conditionally_add_const<const int&, true>, const int&>);
static_assert(std::is_same_v<conditionally_add_const<const int&, false>, const int&>);
static_assert(std::is_same_v<conditionally_add_const<volatile int&, true>, const volatile int&>);
static_assert(std::is_same_v<conditionally_add_const<volatile int&, false>, volatile int&>);
static_assert(std::is_same_v<conditionally_add_const<const volatile int&, true>, const volatile int&>);
static_assert(std::is_same_v<conditionally_add_const<const volatile int&, false>, const volatile int&>);

// copy_const
static_assert(std::is_same_v<copy_const<int, float>, int>);
static_assert(std::is_same_v<copy_const<int, const float>, const int>);
static_assert(std::is_same_v<copy_const<int, volatile float>, int>);
static_assert(std::is_same_v<copy_const<int, const volatile float>, const int>);
static_assert(std::is_same_v<copy_const<const int, float>, int>);
static_assert(std::is_same_v<copy_const<const int, const float>, const int>);
static_assert(std::is_same_v<copy_const<const int, volatile float>, int>);
static_assert(std::is_same_v<copy_const<const int, const volatile float>, const int>);
static_assert(std::is_same_v<copy_const<const volatile int, float>, volatile int>);
static_assert(std::is_same_v<copy_const<const volatile int, const float>, const volatile int>);
static_assert(std::is_same_v<copy_const<const volatile int, volatile float>, volatile int>);
static_assert(std::is_same_v<copy_const<const volatile int, const volatile float>, const volatile int>);
static_assert(std::is_same_v<copy_const<int&, float>, int&>);
static_assert(std::is_same_v<copy_const<int&, const float>, const int&>);
static_assert(std::is_same_v<copy_const<int&, volatile float>, int&>);
static_assert(std::is_same_v<copy_const<int&, const volatile float>, const int&>);
static_assert(std::is_same_v<copy_const<const int&, float>, int&>);
static_assert(std::is_same_v<copy_const<const int&, const float>, const int&>);
static_assert(std::is_same_v<copy_const<const int&, volatile float>, int&>);
static_assert(std::is_same_v<copy_const<const int&, const volatile float>, const int&>);
static_assert(std::is_same_v<copy_const<const volatile int&, float>, volatile int&>);
static_assert(std::is_same_v<copy_const<const volatile int&, const float>, const volatile int&>);
static_assert(std::is_same_v<copy_const<const volatile int&, volatile float>, volatile int&>);
static_assert(std::is_same_v<copy_const<const volatile int&, const volatile float>, const volatile int&>);
static_assert(std::is_same_v<copy_const<int, float&>, int>);
static_assert(std::is_same_v<copy_const<int, const float&>, const int>);
static_assert(std::is_same_v<copy_const<int, volatile float&>, int>);
static_assert(std::is_same_v<copy_const<int, const volatile float&>, const int>);
static_assert(std::is_same_v<copy_const<const int, float&>, int>);
static_assert(std::is_same_v<copy_const<const int, const float&>, const int>);
static_assert(std::is_same_v<copy_const<const int, volatile float&>, int>);
static_assert(std::is_same_v<copy_const<const int, const volatile float&>, const int>);
static_assert(std::is_same_v<copy_const<const volatile int, float&>, volatile int>);
static_assert(std::is_same_v<copy_const<const volatile int, const float&>, const volatile int>);
static_assert(std::is_same_v<copy_const<const volatile int, volatile float&>, volatile int>);
static_assert(std::is_same_v<copy_const<const volatile int, const volatile float&>, const volatile int>);

// is_volatile
static_assert(!is_volatile<int>);
static_assert(!is_volatile<int&>);
static_assert(!is_volatile<int&&>);
static_assert(!is_volatile<const int>);
static_assert(!is_volatile<const int&>);
static_assert(!is_volatile<const int&&>);
static_assert(is_volatile<volatile int>);
static_assert(is_volatile<volatile int&>);
static_assert(is_volatile<volatile int&&>);
static_assert(is_volatile<const volatile int>);
static_assert(is_volatile<const volatile int&>);
static_assert(is_volatile<const volatile int&&>);

// add_volatile
static_assert(std::is_same_v<add_volatile<int>, volatile int>);
static_assert(std::is_same_v<add_volatile<int&>, volatile int&>);
static_assert(std::is_same_v<add_volatile<int&&>, volatile int&&>);
static_assert(std::is_same_v<add_volatile<const int>, volatile const int>);
static_assert(std::is_same_v<add_volatile<const int&>, volatile const int&>);
static_assert(std::is_same_v<add_volatile<const int&&>, volatile const int&&>);
static_assert(std::is_same_v<add_volatile<volatile int>, volatile int>);
static_assert(std::is_same_v<add_volatile<volatile int&>, volatile int&>);
static_assert(std::is_same_v<add_volatile<volatile int&&>, volatile int&&>);
static_assert(std::is_same_v<add_volatile<const volatile int>, volatile const int>);
static_assert(std::is_same_v<add_volatile<const volatile int&>, volatile const int&>);
static_assert(std::is_same_v<add_volatile<const volatile int&&>, volatile const int&&>);

// remove_volatile
static_assert(std::is_same_v<remove_volatile<int>, int>);
static_assert(std::is_same_v<remove_volatile<int&>, int&>);
static_assert(std::is_same_v<remove_volatile<int&&>, int&&>);
static_assert(std::is_same_v<remove_volatile<const int>, const int>);
static_assert(std::is_same_v<remove_volatile<const int&>, const int&>);
static_assert(std::is_same_v<remove_volatile<const int&&>, const int&&>);
static_assert(std::is_same_v<remove_volatile<volatile int>, int>);
static_assert(std::is_same_v<remove_volatile<volatile int&>, int&>);
static_assert(std::is_same_v<remove_volatile<volatile int&&>, int&&>);
static_assert(std::is_same_v<remove_volatile<const volatile int>, const int>);
static_assert(std::is_same_v<remove_volatile<const volatile int&>, const int&>);
static_assert(std::is_same_v<remove_volatile<const volatile int&&>, const int&&>);

// set_volatile
static_assert(std::is_same_v<set_volatile<int, true>, volatile int>);
static_assert(std::is_same_v<set_volatile<int&, true>, volatile int&>);
static_assert(std::is_same_v<set_volatile<int&&, true>, volatile int&&>);
static_assert(std::is_same_v<set_volatile<const int, true>, volatile const int>);
static_assert(std::is_same_v<set_volatile<const int&, true>, volatile const int&>);
static_assert(std::is_same_v<set_volatile<const int&&, true>, volatile const int&&>);
static_assert(std::is_same_v<set_volatile<volatile int, true>, volatile int>);
static_assert(std::is_same_v<set_volatile<volatile int&, true>, volatile int&>);
static_assert(std::is_same_v<set_volatile<volatile int&&, true>, volatile int&&>);
static_assert(std::is_same_v<set_volatile<const volatile int, true>, volatile const int>);
static_assert(std::is_same_v<set_volatile<const volatile int&, true>, volatile const int&>);
static_assert(std::is_same_v<set_volatile<const volatile int&&, true>, volatile const int&&>);
static_assert(std::is_same_v<set_volatile<int, false>, int>);
static_assert(std::is_same_v<set_volatile<int&, false>, int&>);
static_assert(std::is_same_v<set_volatile<int&&, false>, int&&>);
static_assert(std::is_same_v<set_volatile<const int, false>, const int>);
static_assert(std::is_same_v<set_volatile<const int&, false>, const int&>);
static_assert(std::is_same_v<set_volatile<const int&&, false>, const int&&>);
static_assert(std::is_same_v<set_volatile<volatile int, false>, int>);
static_assert(std::is_same_v<set_volatile<volatile int&, false>, int&>);
static_assert(std::is_same_v<set_volatile<volatile int&&, false>, int&&>);
static_assert(std::is_same_v<set_volatile<const volatile int, false>, const int>);
static_assert(std::is_same_v<set_volatile<const volatile int&, false>, const int&>);
static_assert(std::is_same_v<set_volatile<const volatile int&&, false>, const int&&>);

// conditionally_add_volatile
static_assert(std::is_same_v<conditionally_add_volatile<int, true>, volatile int>);
static_assert(std::is_same_v<conditionally_add_volatile<int, false>, int>);
static_assert(std::is_same_v<conditionally_add_volatile<const int, true>, const volatile int>);
static_assert(std::is_same_v<conditionally_add_volatile<const int, false>, const int>);
static_assert(std::is_same_v<conditionally_add_volatile<volatile int, true>, volatile int>);
static_assert(std::is_same_v<conditionally_add_volatile<volatile int, false>, volatile int>);
static_assert(std::is_same_v<conditionally_add_volatile<const volatile int, true>, const volatile int>);
static_assert(std::is_same_v<conditionally_add_volatile<const volatile int, false>, const volatile int>);
static_assert(std::is_same_v<conditionally_add_volatile<int&, true>, volatile int&>);
static_assert(std::is_same_v<conditionally_add_volatile<int&, false>, int&>);
static_assert(std::is_same_v<conditionally_add_volatile<const int&, true>, const volatile int&>);
static_assert(std::is_same_v<conditionally_add_volatile<const int&, false>, const int&>);
static_assert(std::is_same_v<conditionally_add_volatile<volatile int&, true>, volatile int&>);
static_assert(std::is_same_v<conditionally_add_volatile<volatile int&, false>, volatile int&>);
static_assert(std::is_same_v<conditionally_add_volatile<const volatile int&, true>, const volatile int&>);
static_assert(std::is_same_v<conditionally_add_volatile<const volatile int&, false>, const volatile int&>);

// copy_volatile
static_assert(std::is_same_v<copy_volatile<int, float>, int>);
static_assert(std::is_same_v<copy_volatile<int, const float>, int>);
static_assert(std::is_same_v<copy_volatile<int, volatile float>, volatile int>);
static_assert(std::is_same_v<copy_volatile<int, const volatile float>, volatile int>);
static_assert(std::is_same_v<copy_volatile<const int, float>, const int>);
static_assert(std::is_same_v<copy_volatile<const int, const float>, const int>);
static_assert(std::is_same_v<copy_volatile<const int, volatile float>, const volatile int>);
static_assert(std::is_same_v<copy_volatile<const int, const volatile float>, const volatile int>);
static_assert(std::is_same_v<copy_volatile<const volatile int, float>, const int>);
static_assert(std::is_same_v<copy_volatile<const volatile int, const float>, const int>);
static_assert(std::is_same_v<copy_volatile<const volatile int, volatile float>, const volatile int>);
static_assert(std::is_same_v<copy_volatile<const volatile int, const volatile float>, const volatile int>);
static_assert(std::is_same_v<copy_volatile<int&, float>, int&>);
static_assert(std::is_same_v<copy_volatile<int&, const float>, int&>);
static_assert(std::is_same_v<copy_volatile<int&, volatile float>, volatile int&>);
static_assert(std::is_same_v<copy_volatile<int&, const volatile float>, volatile int&>);
static_assert(std::is_same_v<copy_volatile<const int&, float>, const int&>);
static_assert(std::is_same_v<copy_volatile<const int&, const float>, const int&>);
static_assert(std::is_same_v<copy_volatile<const int&, volatile float>, const volatile int&>);
static_assert(std::is_same_v<copy_volatile<const int&, const volatile float>, const volatile int&>);
static_assert(std::is_same_v<copy_volatile<const volatile int&, float>, const int&>);
static_assert(std::is_same_v<copy_volatile<const volatile int&, const float>, const int&>);
static_assert(std::is_same_v<copy_volatile<const volatile int&, volatile float>, const volatile int&>);
static_assert(std::is_same_v<copy_volatile<const volatile int&, const volatile float>, const volatile int&>);
static_assert(std::is_same_v<copy_volatile<int, float&>, int>);
static_assert(std::is_same_v<copy_volatile<int, const float&>, int>);
static_assert(std::is_same_v<copy_volatile<int, volatile float&>, volatile int>);
static_assert(std::is_same_v<copy_volatile<int, const volatile float&>, volatile int>);
static_assert(std::is_same_v<copy_volatile<const int, float&>, const int>);
static_assert(std::is_same_v<copy_volatile<const int, const float&>, const int>);
static_assert(std::is_same_v<copy_volatile<const int, volatile float&>, const volatile int>);
static_assert(std::is_same_v<copy_volatile<const int, const volatile float&>, const volatile int>);
static_assert(std::is_same_v<copy_volatile<const volatile int, float&>, const int>);
static_assert(std::is_same_v<copy_volatile<const volatile int, const float&>, const int>);
static_assert(std::is_same_v<copy_volatile<const volatile int, volatile float&>, const volatile int>);
static_assert(std::is_same_v<copy_volatile<const volatile int, const volatile float&>, const volatile int>);

// add_cv
static_assert(std::is_same_v<add_cv<int>, const volatile int>);
static_assert(std::is_same_v<add_cv<int&>, const volatile int&>);
static_assert(std::is_same_v<add_cv<int&&>, const volatile int&&>);
static_assert(std::is_same_v<add_cv<const int>, const volatile int>);
static_assert(std::is_same_v<add_cv<const int&>, const volatile int&>);
static_assert(std::is_same_v<add_cv<const int&&>, const volatile int&&>);
static_assert(std::is_same_v<add_cv<volatile int>, const volatile int>);
static_assert(std::is_same_v<add_cv<volatile int&>, const volatile int&>);
static_assert(std::is_same_v<add_cv<volatile int&&>, const volatile int&&>);
static_assert(std::is_same_v<add_cv<const volatile int>, const volatile int>);
static_assert(std::is_same_v<add_cv<const volatile int&>, const volatile int&>);
static_assert(std::is_same_v<add_cv<const volatile int&&>, const volatile int&&>);

// remove_cv
static_assert(std::is_same_v<remove_cv<int>, int>);
static_assert(std::is_same_v<remove_cv<int&>, int&>);
static_assert(std::is_same_v<remove_cv<int&&>, int&&>);
static_assert(std::is_same_v<remove_cv<const int>, int>);
static_assert(std::is_same_v<remove_cv<const int&>, int&>);
static_assert(std::is_same_v<remove_cv<const int&&>, int&&>);
static_assert(std::is_same_v<remove_cv<volatile int>, int>);
static_assert(std::is_same_v<remove_cv<volatile int&>, int&>);
static_assert(std::is_same_v<remove_cv<volatile int&&>, int&&>);
static_assert(std::is_same_v<remove_cv<const volatile int>, int>);
static_assert(std::is_same_v<remove_cv<const volatile int&>, int&>);
static_assert(std::is_same_v<remove_cv<const volatile int&&>, int&&>);

// set_cv
static_assert(std::is_same_v<set_cv<int, true>, const volatile int>);
static_assert(std::is_same_v<set_cv<int&, true>, const volatile int&>);
static_assert(std::is_same_v<set_cv<int&&, true>, const volatile int&&>);
static_assert(std::is_same_v<set_cv<const int, true>, const volatile int>);
static_assert(std::is_same_v<set_cv<const int&, true>, const volatile int&>);
static_assert(std::is_same_v<set_cv<const int&&, true>, const volatile int&&>);
static_assert(std::is_same_v<set_cv<volatile int, true>, const volatile int>);
static_assert(std::is_same_v<set_cv<volatile int&, true>, const volatile int&>);
static_assert(std::is_same_v<set_cv<volatile int&&, true>, const volatile int&&>);
static_assert(std::is_same_v<set_cv<const volatile int, true>, const volatile int>);
static_assert(std::is_same_v<set_cv<const volatile int&, true>, const volatile int&>);
static_assert(std::is_same_v<set_cv<const volatile int&&, true>, const volatile int&&>);
static_assert(std::is_same_v<set_cv<int, false>, int>);
static_assert(std::is_same_v<set_cv<int&, false>, int&>);
static_assert(std::is_same_v<set_cv<int&&, false>, int&&>);
static_assert(std::is_same_v<set_cv<const int, false>, int>);
static_assert(std::is_same_v<set_cv<const int&, false>, int&>);
static_assert(std::is_same_v<set_cv<const int&&, false>, int&&>);
static_assert(std::is_same_v<set_cv<volatile int, false>, int>);
static_assert(std::is_same_v<set_cv<volatile int&, false>, int&>);
static_assert(std::is_same_v<set_cv<volatile int&&, false>, int&&>);
static_assert(std::is_same_v<set_cv<const volatile int, false>, int>);
static_assert(std::is_same_v<set_cv<const volatile int&, false>, int&>);
static_assert(std::is_same_v<set_cv<const volatile int&&, false>, int&&>);

// conditionally_add_cv
static_assert(std::is_same_v<conditionally_add_cv<int, true>, const volatile int>);
static_assert(std::is_same_v<conditionally_add_cv<int, false>, int>);
static_assert(std::is_same_v<conditionally_add_cv<const int, true>, const volatile int>);
static_assert(std::is_same_v<conditionally_add_cv<const int, false>, const int>);
static_assert(std::is_same_v<conditionally_add_cv<volatile int, true>, const volatile int>);
static_assert(std::is_same_v<conditionally_add_cv<volatile int, false>, volatile int>);
static_assert(std::is_same_v<conditionally_add_cv<const volatile int, true>, const volatile int>);
static_assert(std::is_same_v<conditionally_add_cv<const volatile int, false>, const volatile int>);
static_assert(std::is_same_v<conditionally_add_cv<int&, true>, const volatile int&>);
static_assert(std::is_same_v<conditionally_add_cv<int&, false>, int&>);
static_assert(std::is_same_v<conditionally_add_cv<const int&, true>, const volatile int&>);
static_assert(std::is_same_v<conditionally_add_cv<const int&, false>, const int&>);
static_assert(std::is_same_v<conditionally_add_cv<volatile int&, true>, const volatile int&>);
static_assert(std::is_same_v<conditionally_add_cv<volatile int&, false>, volatile int&>);
static_assert(std::is_same_v<conditionally_add_cv<const volatile int&, true>, const volatile int&>);
static_assert(std::is_same_v<conditionally_add_cv<const volatile int&, false>, const volatile int&>);

// copy_cv
static_assert(std::is_same_v<copy_cv<int, float>, int>);
static_assert(std::is_same_v<copy_cv<int, const float>, const int>);
static_assert(std::is_same_v<copy_cv<int, volatile float>, volatile int>);
static_assert(std::is_same_v<copy_cv<int, const volatile float>, const volatile int>);
static_assert(std::is_same_v<copy_cv<const int, float>, int>);
static_assert(std::is_same_v<copy_cv<const int, const float>, const int>);
static_assert(std::is_same_v<copy_cv<const int, volatile float>, volatile int>);
static_assert(std::is_same_v<copy_cv<const int, const volatile float>, const volatile int>);
static_assert(std::is_same_v<copy_cv<const volatile int, float>, int>);
static_assert(std::is_same_v<copy_cv<const volatile int, const float>, const int>);
static_assert(std::is_same_v<copy_cv<const volatile int, volatile float>, volatile int>);
static_assert(std::is_same_v<copy_cv<const volatile int, const volatile float>, const volatile int>);
static_assert(std::is_same_v<copy_cv<int&, float>, int&>);
static_assert(std::is_same_v<copy_cv<int&, const float>, const int&>);
static_assert(std::is_same_v<copy_cv<int&, volatile float>, volatile int&>);
static_assert(std::is_same_v<copy_cv<int&, const volatile float>, const volatile int&>);
static_assert(std::is_same_v<copy_cv<const int&, float>, int&>);
static_assert(std::is_same_v<copy_cv<const int&, const float>, const int&>);
static_assert(std::is_same_v<copy_cv<const int&, volatile float>, volatile int&>);
static_assert(std::is_same_v<copy_cv<const int&, const volatile float>, const volatile int&>);
static_assert(std::is_same_v<copy_cv<const volatile int&, float>, int&>);
static_assert(std::is_same_v<copy_cv<const volatile int&, const float>, const int&>);
static_assert(std::is_same_v<copy_cv<const volatile int&, volatile float>, volatile int&>);
static_assert(std::is_same_v<copy_cv<const volatile int&, const volatile float>, const volatile int&>);
static_assert(std::is_same_v<copy_cv<int, float&>, int>);
static_assert(std::is_same_v<copy_cv<int, const float&>, const int>);
static_assert(std::is_same_v<copy_cv<int, volatile float&>, volatile int>);
static_assert(std::is_same_v<copy_cv<int, const volatile float&>, const volatile int>);
static_assert(std::is_same_v<copy_cv<const int, float&>, int>);
static_assert(std::is_same_v<copy_cv<const int, const float&>, const int>);
static_assert(std::is_same_v<copy_cv<const int, volatile float&>, volatile int>);
static_assert(std::is_same_v<copy_cv<const int, const volatile float&>, const volatile int>);
static_assert(std::is_same_v<copy_cv<const volatile int, float&>, int>);
static_assert(std::is_same_v<copy_cv<const volatile int, const float&>, const int>);
static_assert(std::is_same_v<copy_cv<const volatile int, volatile float&>, volatile int>);
static_assert(std::is_same_v<copy_cv<const volatile int, const volatile float&>, const volatile int>);

// is_cvref
static_assert(!is_cvref<int>);
static_assert(is_cvref<int&>);
static_assert(is_cvref<int&&>);
static_assert(is_cvref<const int>);
static_assert(is_cvref<const int&>);
static_assert(is_cvref<const int&&>);
static_assert(is_cvref<volatile int>);
static_assert(is_cvref<volatile int&>);
static_assert(is_cvref<volatile int&&>);
static_assert(is_cvref<const volatile int>);
static_assert(is_cvref<const volatile int&>);
static_assert(is_cvref<const volatile int&&>);

// copy_ref
static_assert(std::is_same_v<copy_ref<int, float>, int>);
static_assert(std::is_same_v<copy_ref<int, float&>, int&>);
static_assert(std::is_same_v<copy_ref<int, float&&>, int&&>);
static_assert(std::is_same_v<copy_ref<int&, float>, int>);
static_assert(std::is_same_v<copy_ref<int&, float&>, int&>);
static_assert(std::is_same_v<copy_ref<int&, float&&>, int&&>);
static_assert(std::is_same_v<copy_ref<int&&, float>, int>);
static_assert(std::is_same_v<copy_ref<int&&, float&>, int&>);
static_assert(std::is_same_v<copy_ref<int&&, float&&>, int&&>);

// copy_cvref
static_assert(std::is_same_v<copy_cvref<int, float>, int>);
static_assert(std::is_same_v<copy_cvref<int, const float&>, const int&>);
static_assert(std::is_same_v<copy_cvref<int, volatile float&&>, volatile int&&>);
static_assert(std::is_same_v<copy_cvref<int&, float>, int>);
static_assert(std::is_same_v<copy_cvref<int&, const float&>, const int&>);
static_assert(std::is_same_v<copy_cvref<int&, volatile float&&>, volatile int&&>);
static_assert(std::is_same_v<copy_cvref<int&&, float>, int>);
static_assert(std::is_same_v<copy_cvref<int&&, const float&>, const int&>);
static_assert(std::is_same_v<copy_cvref<int&&, volatile float&&>, volatile int&&>);
static_assert(std::is_same_v<copy_cvref<const int, float>, int>);
static_assert(std::is_same_v<copy_cvref<const int, const float&>, const int&>);
static_assert(std::is_same_v<copy_cvref<const int, volatile float&&>, volatile int&&>);
static_assert(std::is_same_v<copy_cvref<const int&, float>, int>);
static_assert(std::is_same_v<copy_cvref<const int&, const float&>, const int&>);
static_assert(std::is_same_v<copy_cvref<const int&, volatile float&&>, volatile int&&>);
static_assert(std::is_same_v<copy_cvref<const int&&, float>, int>);
static_assert(std::is_same_v<copy_cvref<const int&&, const float&>, const int&>);
static_assert(std::is_same_v<copy_cvref<const int&&, volatile float&&>, volatile int&&>);
static_assert(std::is_same_v<copy_cvref<volatile int, float>, int>);
static_assert(std::is_same_v<copy_cvref<volatile int, const float&>, const int&>);
static_assert(std::is_same_v<copy_cvref<volatile int, volatile float&&>, volatile int&&>);
static_assert(std::is_same_v<copy_cvref<volatile int&, float>, int>);
static_assert(std::is_same_v<copy_cvref<volatile int&, const float&>, const int&>);
static_assert(std::is_same_v<copy_cvref<volatile int&, volatile float&&>, volatile int&&>);
static_assert(std::is_same_v<copy_cvref<volatile int&&, float>, int>);
static_assert(std::is_same_v<copy_cvref<volatile int&&, const float&>, const int&>);
static_assert(std::is_same_v<copy_cvref<volatile int&&, volatile float&&>, volatile int&&>);
static_assert(std::is_same_v<copy_cvref<const volatile int, float>, int>);
static_assert(std::is_same_v<copy_cvref<const volatile int, const float&>, const int&>);
static_assert(std::is_same_v<copy_cvref<const volatile int, volatile float&&>, volatile int&&>);
static_assert(std::is_same_v<copy_cvref<const volatile int&, float>, int>);
static_assert(std::is_same_v<copy_cvref<const volatile int&, const float&>, const int&>);
static_assert(std::is_same_v<copy_cvref<const volatile int&, volatile float&&>, volatile int&&>);
static_assert(std::is_same_v<copy_cvref<const volatile int&&, float>, int>);
static_assert(std::is_same_v<copy_cvref<const volatile int&&, const float&>, const int&>);
static_assert(std::is_same_v<copy_cvref<const volatile int&&, volatile float&&>, volatile int&&>);

// remove_noexcept
static_assert(std::is_same_v<remove_noexcept<int>, int>); // non-functions should be unchanged
static_assert(std::is_same_v<remove_noexcept<const int>, const int>);
static_assert(std::is_same_v<remove_noexcept<int()>, int()>);
static_assert(std::is_same_v<remove_noexcept<int() noexcept>, int()>);
static_assert(std::is_same_v<remove_noexcept<int (*)()>, int (*)()>);
static_assert(std::is_same_v<remove_noexcept<int (*)() noexcept>, int (*)()>);
static_assert(std::is_same_v<remove_noexcept<int (&)()>, int (&)()>);
static_assert(std::is_same_v<remove_noexcept<int (&)() noexcept>, int (&)()>);
#if MUU_WINDOWS
static_assert(std::is_same_v<remove_noexcept<int __cdecl()>, int __cdecl()>);
static_assert(std::is_same_v<remove_noexcept<int __cdecl() noexcept>, int __cdecl()>);
static_assert(std::is_same_v<remove_noexcept<int(__cdecl*)()>, int(__cdecl*)()>);
static_assert(std::is_same_v<remove_noexcept<int(__cdecl*)() noexcept>, int(__cdecl*)()>);
static_assert(std::is_same_v<remove_noexcept<int(__cdecl&)()>, int(__cdecl&)()>);
static_assert(std::is_same_v<remove_noexcept<int(__cdecl&)() noexcept>, int(__cdecl&)()>);
static_assert(std::is_same_v<remove_noexcept<int __vectorcall()>, int __vectorcall()>);
static_assert(std::is_same_v<remove_noexcept<int __vectorcall() noexcept>, int __vectorcall()>);
static_assert(std::is_same_v<remove_noexcept<int(__vectorcall*)()>, int(__vectorcall*)()>);
static_assert(std::is_same_v<remove_noexcept<int(__vectorcall*)() noexcept>, int(__vectorcall*)()>);
static_assert(std::is_same_v<remove_noexcept<int(__vectorcall&)()>, int(__vectorcall&)()>);
static_assert(std::is_same_v<remove_noexcept<int(__vectorcall&)() noexcept>, int(__vectorcall&)()>);
#endif

// remove_callconv
static_assert(std::is_same_v<remove_callconv<int>, int>); // non-functions should be unchanged
static_assert(std::is_same_v<remove_callconv<const int>, const int>);
static_assert(std::is_same_v<remove_callconv<int()>, int()>);
static_assert(std::is_same_v<remove_callconv<int() noexcept>, int() noexcept>);
static_assert(std::is_same_v<remove_callconv<int (*)()>, int (*)()>);
static_assert(std::is_same_v<remove_callconv<int (*)() noexcept>, int (*)() noexcept>);
static_assert(std::is_same_v<remove_callconv<int (&)()>, int (&)()>);
static_assert(std::is_same_v<remove_callconv<int (&)() noexcept>, int (&)() noexcept>);
#if MUU_WINDOWS
static_assert(std::is_same_v<remove_callconv<int __cdecl()>, int()>);
static_assert(std::is_same_v<remove_callconv<int __cdecl() noexcept>, int() noexcept>);
static_assert(std::is_same_v<remove_callconv<int(__cdecl*)()>, int (*)()>);
static_assert(std::is_same_v<remove_callconv<int(__cdecl*)() noexcept>, int (*)() noexcept>);
static_assert(std::is_same_v<remove_callconv<int(__cdecl&)()>, int (&)()>);
static_assert(std::is_same_v<remove_callconv<int(__cdecl&)() noexcept>, int (&)() noexcept>);
static_assert(std::is_same_v<remove_callconv<int __vectorcall()>, int()>);
static_assert(std::is_same_v<remove_callconv<int __vectorcall() noexcept>, int() noexcept>);
static_assert(std::is_same_v<remove_callconv<int(__vectorcall*)()>, int (*)()>);
static_assert(std::is_same_v<remove_callconv<int(__vectorcall*)() noexcept>, int (*)() noexcept>);
static_assert(std::is_same_v<remove_callconv<int(__vectorcall&)()>, int (&)()>);
static_assert(std::is_same_v<remove_callconv<int(__vectorcall&)() noexcept>, int (&)() noexcept>);
#endif

// inherits_from
struct base_type
{};
struct derived_type : base_type
{};
struct more_derived_type : derived_type
{};
static_assert(inherits_from<derived_type, base_type>);
static_assert(inherits_from<more_derived_type, derived_type>);
static_assert(inherits_from<more_derived_type, base_type>);
static_assert(!inherits_from<base_type, base_type>);
static_assert(!inherits_from<derived_type, derived_type>);
static_assert(!inherits_from<more_derived_type, more_derived_type>);
static_assert(!inherits_from<base_type, derived_type>);
static_assert(!inherits_from<base_type, more_derived_type>);
static_assert(!inherits_from<derived_type, more_derived_type>);

// inherits_from_any/all
struct base_type_2
{};
struct base_type_3
{};
struct derived_from_1_2 : base_type, base_type_2
{};
static_assert(inherits_from_any<derived_from_1_2, base_type, base_type_2, base_type_3>);
static_assert(!inherits_from_all<derived_from_1_2, base_type, base_type_2, base_type_3>);
static_assert(inherits_from_all<derived_from_1_2, base_type>);
static_assert(inherits_from_all<derived_from_1_2, base_type, base_type_2>);

// rebase_pointer
static_assert(std::is_same_v<rebase_pointer<int*, an_enum>, an_enum*>);
static_assert(std::is_same_v<rebase_pointer<const int*, an_enum>, const an_enum*>);
static_assert(std::is_same_v<rebase_pointer<volatile int*, an_enum>, volatile an_enum*>);
static_assert(std::is_same_v<rebase_pointer<const volatile int*, an_enum>, const volatile an_enum*>);
static_assert(std::is_same_v<rebase_pointer<void*, an_enum>, an_enum*>);
static_assert(std::is_same_v<rebase_pointer<const void*, an_enum>, const an_enum*>);
static_assert(std::is_same_v<rebase_pointer<volatile void*, an_enum>, volatile an_enum*>);
static_assert(std::is_same_v<rebase_pointer<const volatile void*, an_enum>, const volatile an_enum*>);
static_assert(std::is_same_v<rebase_pointer<an_enum*, int>, int*>);
static_assert(std::is_same_v<rebase_pointer<const an_enum*, int>, const int*>);
static_assert(std::is_same_v<rebase_pointer<volatile an_enum*, int>, volatile int*>);
static_assert(std::is_same_v<rebase_pointer<const volatile an_enum*, int>, const volatile int*>);
static_assert(std::is_same_v<rebase_pointer<an_enum*, void>, void*>);
static_assert(std::is_same_v<rebase_pointer<const an_enum*, void>, const void*>);
static_assert(std::is_same_v<rebase_pointer<volatile an_enum*, void>, volatile void*>);
static_assert(std::is_same_v<rebase_pointer<const volatile an_enum*, void>, const volatile void*>);
static_assert(std::is_same_v<rebase_pointer<void*, volatile an_enum>, volatile an_enum*>);
static_assert(std::is_same_v<rebase_pointer<const void*, volatile an_enum>, const volatile an_enum*>);
static_assert(std::is_same_v<rebase_pointer<volatile void*, volatile an_enum>, volatile an_enum*>);
static_assert(std::is_same_v<rebase_pointer<const volatile void*, volatile an_enum>, const volatile an_enum*>);

// make_signed
static_assert(std::is_same_v<make_signed<char>, signed char>);
static_assert(std::is_same_v<make_signed<const char>, const signed char>);
static_assert(std::is_same_v<make_signed<volatile char>, volatile signed char>);
static_assert(std::is_same_v<make_signed<const volatile char>, const volatile signed char>);
static_assert(std::is_same_v<make_signed<char&>, signed char&>);
static_assert(std::is_same_v<make_signed<const char&>, const signed char&>);
static_assert(std::is_same_v<make_signed<volatile char&>, volatile signed char&>);
static_assert(std::is_same_v<make_signed<const volatile char&>, const volatile signed char&>);
static_assert(std::is_same_v<make_signed<char&&>, signed char&&>);
static_assert(std::is_same_v<make_signed<const char&&>, const signed char&&>);
static_assert(std::is_same_v<make_signed<volatile char&&>, volatile signed char&&>);
static_assert(std::is_same_v<make_signed<const volatile char&&>, const volatile signed char&&>);
static_assert(std::is_same_v<make_signed<short>, short>);
static_assert(std::is_same_v<make_signed<unsigned short>, short>);
static_assert(std::is_same_v<make_signed<int>, int>);
static_assert(std::is_same_v<make_signed<unsigned>, int>);
static_assert(std::is_same_v<make_signed<long>, long>);
static_assert(std::is_same_v<make_signed<unsigned long>, long>);
static_assert(std::is_same_v<make_signed<long long>, long long>);
static_assert(std::is_same_v<make_signed<unsigned long long>, long long>);
static_assert(std::is_same_v<make_signed<half>, half>);
static_assert(std::is_same_v<make_signed<float>, float>);
static_assert(std::is_same_v<make_signed<double>, double>);
static_assert(std::is_same_v<make_signed<long double>, long double>);
static_assert(std::is_same_v<make_signed<half&>, half&>);
static_assert(std::is_same_v<make_signed<float&>, float&>);
static_assert(std::is_same_v<make_signed<double&>, double&>);
static_assert(std::is_same_v<make_signed<long double&>, long double&>);

// make_unsigned
static_assert(std::is_same_v<make_unsigned<char>, unsigned char>);
static_assert(std::is_same_v<make_unsigned<const char>, const unsigned char>);
static_assert(std::is_same_v<make_unsigned<volatile char>, volatile unsigned char>);
static_assert(std::is_same_v<make_unsigned<const volatile char>, const volatile unsigned char>);
static_assert(std::is_same_v<make_unsigned<char&>, unsigned char&>);
static_assert(std::is_same_v<make_unsigned<const char&>, const unsigned char&>);
static_assert(std::is_same_v<make_unsigned<volatile char&>, volatile unsigned char&>);
static_assert(std::is_same_v<make_unsigned<const volatile char&>, const volatile unsigned char&>);
static_assert(std::is_same_v<make_unsigned<char&&>, unsigned char&&>);
static_assert(std::is_same_v<make_unsigned<const char&&>, const unsigned char&&>);
static_assert(std::is_same_v<make_unsigned<volatile char&&>, volatile unsigned char&&>);
static_assert(std::is_same_v<make_unsigned<const volatile char&&>, const volatile unsigned char&&>);
static_assert(std::is_same_v<make_unsigned<short>, unsigned short>);
static_assert(std::is_same_v<make_unsigned<unsigned short>, unsigned short>);
static_assert(std::is_same_v<make_unsigned<int>, unsigned>);
static_assert(std::is_same_v<make_unsigned<unsigned>, unsigned>);
static_assert(std::is_same_v<make_unsigned<long>, unsigned long>);
static_assert(std::is_same_v<make_unsigned<unsigned long>, unsigned long>);
static_assert(std::is_same_v<make_unsigned<long long>, unsigned long long>);
static_assert(std::is_same_v<make_unsigned<unsigned long long>, unsigned long long>);
static_assert(std::is_same_v<make_unsigned<half>, void>);
static_assert(std::is_same_v<make_unsigned<float>, void>);
static_assert(std::is_same_v<make_unsigned<double>, void>);
static_assert(std::is_same_v<make_unsigned<long double>, void>);
static_assert(std::is_same_v<make_unsigned<half&>, void>);
static_assert(std::is_same_v<make_unsigned<float&>, void>);
static_assert(std::is_same_v<make_unsigned<double&>, void>);
static_assert(std::is_same_v<make_unsigned<long double&>, void>);

// set_signed
static_assert(std::is_same_v<set_signed<char, true>, signed char>);
static_assert(std::is_same_v<set_signed<const char, true>, const signed char>);
static_assert(std::is_same_v<set_signed<volatile char, true>, volatile signed char>);
static_assert(std::is_same_v<set_signed<const volatile char, true>, const volatile signed char>);
static_assert(std::is_same_v<set_signed<char&, true>, signed char&>);
static_assert(std::is_same_v<set_signed<const char&, true>, const signed char&>);
static_assert(std::is_same_v<set_signed<volatile char&, true>, volatile signed char&>);
static_assert(std::is_same_v<set_signed<const volatile char&, true>, const volatile signed char&>);
static_assert(std::is_same_v<set_signed<char&&, true>, signed char&&>);
static_assert(std::is_same_v<set_signed<const char&&, true>, const signed char&&>);
static_assert(std::is_same_v<set_signed<volatile char&&, true>, volatile signed char&&>);
static_assert(std::is_same_v<set_signed<const volatile char&&, true>, const volatile signed char&&>);
static_assert(std::is_same_v<set_signed<short, true>, short>);
static_assert(std::is_same_v<set_signed<unsigned short, true>, short>);
static_assert(std::is_same_v<set_signed<int, true>, int>);
static_assert(std::is_same_v<set_signed<unsigned, true>, int>);
static_assert(std::is_same_v<set_signed<long, true>, long>);
static_assert(std::is_same_v<set_signed<unsigned long, true>, long>);
static_assert(std::is_same_v<set_signed<long long, true>, long long>);
static_assert(std::is_same_v<set_signed<unsigned long long, true>, long long>);
static_assert(std::is_same_v<set_signed<half, true>, half>);
static_assert(std::is_same_v<set_signed<float, true>, float>);
static_assert(std::is_same_v<set_signed<double, true>, double>);
static_assert(std::is_same_v<set_signed<long double, true>, long double>);
static_assert(std::is_same_v<set_signed<half&, true>, half&>);
static_assert(std::is_same_v<set_signed<float&, true>, float&>);
static_assert(std::is_same_v<set_signed<double&, true>, double&>);
static_assert(std::is_same_v<set_signed<long double&, true>, long double&>);
static_assert(std::is_same_v<set_signed<char, false>, unsigned char>);
static_assert(std::is_same_v<set_signed<const char, false>, const unsigned char>);
static_assert(std::is_same_v<set_signed<volatile char, false>, volatile unsigned char>);
static_assert(std::is_same_v<set_signed<const volatile char, false>, const volatile unsigned char>);
static_assert(std::is_same_v<set_signed<char&, false>, unsigned char&>);
static_assert(std::is_same_v<set_signed<const char&, false>, const unsigned char&>);
static_assert(std::is_same_v<set_signed<volatile char&, false>, volatile unsigned char&>);
static_assert(std::is_same_v<set_signed<const volatile char&, false>, const volatile unsigned char&>);
static_assert(std::is_same_v<set_signed<char&&, false>, unsigned char&&>);
static_assert(std::is_same_v<set_signed<const char&&, false>, const unsigned char&&>);
static_assert(std::is_same_v<set_signed<volatile char&&, false>, volatile unsigned char&&>);
static_assert(std::is_same_v<set_signed<const volatile char&&, false>, const volatile unsigned char&&>);
static_assert(std::is_same_v<set_signed<short, false>, unsigned short>);
static_assert(std::is_same_v<set_signed<unsigned short, false>, unsigned short>);
static_assert(std::is_same_v<set_signed<int, false>, unsigned int>);
static_assert(std::is_same_v<set_signed<unsigned, false>, unsigned int>);
static_assert(std::is_same_v<set_signed<long, false>, unsigned long>);
static_assert(std::is_same_v<set_signed<unsigned long, false>, unsigned long>);
static_assert(std::is_same_v<set_signed<long long, false>, unsigned long long>);
static_assert(std::is_same_v<set_signed<unsigned long long, false>, unsigned long long>);
static_assert(std::is_same_v<set_signed<half, false>, void>);
static_assert(std::is_same_v<set_signed<float, false>, void>);
static_assert(std::is_same_v<set_signed<double, false>, void>);
static_assert(std::is_same_v<set_signed<long double, false>, void>);
static_assert(std::is_same_v<set_signed<half&, false>, void>);
static_assert(std::is_same_v<set_signed<float&, false>, void>);
static_assert(std::is_same_v<set_signed<double&, false>, void>);
static_assert(std::is_same_v<set_signed<long double&, false>, void>);

// set_unsigned
static_assert(std::is_same_v<set_unsigned<char, true>, unsigned char>);
static_assert(std::is_same_v<set_unsigned<const char, true>, const unsigned char>);
static_assert(std::is_same_v<set_unsigned<volatile char, true>, volatile unsigned char>);
static_assert(std::is_same_v<set_unsigned<const volatile char, true>, const volatile unsigned char>);
static_assert(std::is_same_v<set_unsigned<char&, true>, unsigned char&>);
static_assert(std::is_same_v<set_unsigned<const char&, true>, const unsigned char&>);
static_assert(std::is_same_v<set_unsigned<volatile char&, true>, volatile unsigned char&>);
static_assert(std::is_same_v<set_unsigned<const volatile char&, true>, const volatile unsigned char&>);
static_assert(std::is_same_v<set_unsigned<char&&, true>, unsigned char&&>);
static_assert(std::is_same_v<set_unsigned<const char&&, true>, const unsigned char&&>);
static_assert(std::is_same_v<set_unsigned<volatile char&&, true>, volatile unsigned char&&>);
static_assert(std::is_same_v<set_unsigned<const volatile char&&, true>, const volatile unsigned char&&>);
static_assert(std::is_same_v<set_unsigned<short, true>, unsigned short>);
static_assert(std::is_same_v<set_unsigned<unsigned short, true>, unsigned short>);
static_assert(std::is_same_v<set_unsigned<int, true>, unsigned>);
static_assert(std::is_same_v<set_unsigned<unsigned, true>, unsigned>);
static_assert(std::is_same_v<set_unsigned<long, true>, unsigned long>);
static_assert(std::is_same_v<set_unsigned<unsigned long, true>, unsigned long>);
static_assert(std::is_same_v<set_unsigned<long long, true>, unsigned long long>);
static_assert(std::is_same_v<set_unsigned<unsigned long long, true>, unsigned long long>);
static_assert(std::is_same_v<set_unsigned<half, true>, void>);
static_assert(std::is_same_v<set_unsigned<float, true>, void>);
static_assert(std::is_same_v<set_unsigned<double, true>, void>);
static_assert(std::is_same_v<set_unsigned<long double, true>, void>);
static_assert(std::is_same_v<set_unsigned<half&, true>, void>);
static_assert(std::is_same_v<set_unsigned<float&, true>, void>);
static_assert(std::is_same_v<set_unsigned<double&, true>, void>);
static_assert(std::is_same_v<set_unsigned<long double&, true>, void>);
static_assert(std::is_same_v<set_unsigned<char, false>, signed char>);
static_assert(std::is_same_v<set_unsigned<const char, false>, const signed char>);
static_assert(std::is_same_v<set_unsigned<volatile char, false>, volatile signed char>);
static_assert(std::is_same_v<set_unsigned<const volatile char, false>, const volatile signed char>);
static_assert(std::is_same_v<set_unsigned<char&, false>, signed char&>);
static_assert(std::is_same_v<set_unsigned<const char&, false>, const signed char&>);
static_assert(std::is_same_v<set_unsigned<volatile char&, false>, volatile signed char&>);
static_assert(std::is_same_v<set_unsigned<const volatile char&, false>, const volatile signed char&>);
static_assert(std::is_same_v<set_unsigned<char&&, false>, signed char&&>);
static_assert(std::is_same_v<set_unsigned<const char&&, false>, const signed char&&>);
static_assert(std::is_same_v<set_unsigned<volatile char&&, false>, volatile signed char&&>);
static_assert(std::is_same_v<set_unsigned<const volatile char&&, false>, const volatile signed char&&>);
static_assert(std::is_same_v<set_unsigned<short, false>, signed short>);
static_assert(std::is_same_v<set_unsigned<unsigned short, false>, signed short>);
static_assert(std::is_same_v<set_unsigned<int, false>, signed>);
static_assert(std::is_same_v<set_unsigned<unsigned, false>, signed>);
static_assert(std::is_same_v<set_unsigned<long, false>, signed long>);
static_assert(std::is_same_v<set_unsigned<unsigned long, false>, signed long>);
static_assert(std::is_same_v<set_unsigned<long long, false>, signed long long>);
static_assert(std::is_same_v<set_unsigned<unsigned long long, false>, signed long long>);
static_assert(std::is_same_v<set_unsigned<half, false>, half>);
static_assert(std::is_same_v<set_unsigned<float, false>, float>);
static_assert(std::is_same_v<set_unsigned<double, false>, double>);
static_assert(std::is_same_v<set_unsigned<long double, false>, long double>);
static_assert(std::is_same_v<set_unsigned<half&, false>, half&>);
static_assert(std::is_same_v<set_unsigned<float&, false>, float&>);
static_assert(std::is_same_v<set_unsigned<double&, false>, double&>);
static_assert(std::is_same_v<set_unsigned<long double&, false>, long double&>);

// pointer_rank
static_assert(pointer_rank<void> == 0);
static_assert(pointer_rank<void*> == 1);
static_assert(pointer_rank<void**> == 2);
static_assert(pointer_rank<void***> == 3);
static_assert(pointer_rank<void****> == 4);
static_assert(pointer_rank<void*****> == 5);
static_assert(pointer_rank<const void*> == 1);
static_assert(pointer_rank<const void**> == 2);
static_assert(pointer_rank<const void***> == 3);
static_assert(pointer_rank<const void****> == 4);
static_assert(pointer_rank<const void*****> == 5);
static_assert(pointer_rank<const void*> == 1);
static_assert(pointer_rank<const void* volatile*> == 2);
static_assert(pointer_rank<const void***> == 3);
static_assert(pointer_rank<const void*** const* const> == 4);
static_assert(pointer_rank<const void* volatile* const volatile* const volatile* const* volatile> == 5); // lmao c++ is
																										 // a nightmare

// has_arrow_operator
// has_unary_plus_operator
struct Bar
{
	int value;

	Bar operator+() const noexcept
	{
		return Bar{ value };
	}
};
struct Foo
{
	Bar value;

	Bar* operator->() noexcept
	{
		return &value;
	}
};
static_assert(!has_arrow_operator<void>);
static_assert(!has_arrow_operator<void*>);
static_assert(has_arrow_operator<Foo>);
static_assert(!has_arrow_operator<const Foo>);
static_assert(!has_arrow_operator<Bar>);
static_assert(!has_arrow_operator<const Bar>);
static_assert(has_arrow_operator<Foo*>);
static_assert(has_arrow_operator<const Foo*>);

static_assert(!has_unary_plus_operator<void>);
static_assert(has_unary_plus_operator<void*>); // built-in operator
static_assert(!has_unary_plus_operator<Foo>);
static_assert(!has_unary_plus_operator<const Foo>);
static_assert(has_unary_plus_operator<Bar>);
static_assert(has_unary_plus_operator<const Bar>);

// is_tuple_like
static_assert(!is_tuple_like<void>);
static_assert(!is_tuple_like<int>);
static_assert(!is_tuple_like<Foo>);
static_assert(is_tuple_like<std::tuple<int, int>>);
static_assert(is_tuple_like<std::pair<int, int>>);

#if MUU_HAS_VECTORCALL

// is_hva
struct hva1
{
	float a;
};
struct hva2
{
	float a, b;
};
struct hva3
{
	float a, b, c;
};
struct hva4
{
	float a, b, c, d;
};
struct hva5
{
	__m64 a, b, c;
};
struct hva6
{
	float abcd[4];
};
struct hva7
{
	float ab[2];
	float c, d;
};

static_assert(impl::is_hva<hva1>);
static_assert(impl::is_hva<hva2>);
static_assert(impl::is_hva<hva3>);
static_assert(impl::is_hva<hva4>);
static_assert(impl::is_hva<hva5>);
static_assert(impl::is_hva<hva6>);
static_assert(impl::is_hva<hva7>);

struct non_hva1
{
	float a, b, c, d, e;
};
struct non_hva2
{};
struct non_hva3
{
	int a, b;
};
struct non_hva4
{
	float a, b;
	int c;
};
struct alignas(64) non_hva5
{
	float a, b, c, d;
};
struct non_hva6
{
	float abcde[5];
};
struct non_hva7
{
	float ab[2];
	int c, d;
};

static_assert(!impl::is_hva<non_hva1>);
static_assert(!impl::is_hva<non_hva2>);
static_assert(!impl::is_hva<non_hva3>);
static_assert(!impl::is_hva<non_hva4>);
static_assert(!impl::is_hva<non_hva5>);
static_assert(!impl::is_hva<non_hva6>);
static_assert(!impl::is_hva<non_hva7>);

#endif // MUU_HAS_VECTORCALL

// promote_if_small_float
static_assert(std::is_same_v<promote_if_small_float<char>, char>);
static_assert(std::is_same_v<promote_if_small_float<int>, int>);
static_assert(std::is_same_v<promote_if_small_float<half>, float>);
static_assert(std::is_same_v<promote_if_small_float<float>, float>);
static_assert(std::is_same_v<promote_if_small_float<double>, double>);
static_assert(std::is_same_v<promote_if_small_float<long double>, long double>);
static_assert(std::is_same_v<promote_if_small_float<char&>, char&>);
static_assert(std::is_same_v<promote_if_small_float<int&>, int&>);
static_assert(std::is_same_v<promote_if_small_float<half&>, float&>);
static_assert(std::is_same_v<promote_if_small_float<float&>, float&>);
static_assert(std::is_same_v<promote_if_small_float<double&>, double&>);
static_assert(std::is_same_v<promote_if_small_float<long double&>, long double&>);
static_assert(std::is_same_v<promote_if_small_float<const char&>, const char&>);
static_assert(std::is_same_v<promote_if_small_float<const int&>, const int&>);
static_assert(std::is_same_v<promote_if_small_float<const half&>, const float&>);
static_assert(std::is_same_v<promote_if_small_float<const float&>, const float&>);
static_assert(std::is_same_v<promote_if_small_float<const double&>, const double&>);
static_assert(std::is_same_v<promote_if_small_float<const long double&>, const long double&>);
#if MUU_HAS_FLOAT128
static_assert(std::is_same_v<promote_if_small_float<float128_t>, float128_t>);
static_assert(std::is_same_v<promote_if_small_float<float128_t&>, float128_t&>);
static_assert(std::is_same_v<promote_if_small_float<const float128_t&>, const float128_t&>);
#endif

// demote_if_large_float
static_assert(std::is_same_v<demote_if_large_float<char>, char>);
static_assert(std::is_same_v<demote_if_large_float<int>, int>);
static_assert(std::is_same_v<demote_if_large_float<half>, half>);
static_assert(std::is_same_v<demote_if_large_float<float>, float>);
static_assert(std::is_same_v<demote_if_large_float<double>, double>);
static_assert(std::is_same_v<demote_if_large_float<long double>, long double>);
static_assert(std::is_same_v<demote_if_large_float<char&>, char&>);
static_assert(std::is_same_v<demote_if_large_float<int&>, int&>);
static_assert(std::is_same_v<demote_if_large_float<half&>, half&>);
static_assert(std::is_same_v<demote_if_large_float<float&>, float&>);
static_assert(std::is_same_v<demote_if_large_float<double&>, double&>);
static_assert(std::is_same_v<demote_if_large_float<long double&>, long double&>);
static_assert(std::is_same_v<demote_if_large_float<const char&>, const char&>);
static_assert(std::is_same_v<demote_if_large_float<const int&>, const int&>);
static_assert(std::is_same_v<demote_if_large_float<const half&>, const half&>);
static_assert(std::is_same_v<demote_if_large_float<const float&>, const float&>);
static_assert(std::is_same_v<demote_if_large_float<const double&>, const double&>);
static_assert(std::is_same_v<demote_if_large_float<const long double&>, const long double&>);
#if MUU_HAS_FLOAT128
static_assert(std::is_same_v<demote_if_large_float<float128_t>, long double>);
static_assert(std::is_same_v<demote_if_large_float<float128_t&>, long double&>);
static_assert(std::is_same_v<demote_if_large_float<const float128_t&>, const long double&>);
#endif

// clamp_to_standard_float
static_assert(std::is_same_v<clamp_to_standard_float<char>, char>);
static_assert(std::is_same_v<clamp_to_standard_float<int>, int>);
static_assert(std::is_same_v<clamp_to_standard_float<half>, float>);
static_assert(std::is_same_v<clamp_to_standard_float<float>, float>);
static_assert(std::is_same_v<clamp_to_standard_float<double>, double>);
static_assert(std::is_same_v<clamp_to_standard_float<long double>, long double>);
static_assert(std::is_same_v<clamp_to_standard_float<char&>, char&>);
static_assert(std::is_same_v<clamp_to_standard_float<int&>, int&>);
static_assert(std::is_same_v<clamp_to_standard_float<half&>, float&>);
static_assert(std::is_same_v<clamp_to_standard_float<float&>, float&>);
static_assert(std::is_same_v<clamp_to_standard_float<double&>, double&>);
static_assert(std::is_same_v<clamp_to_standard_float<long double&>, long double&>);
static_assert(std::is_same_v<clamp_to_standard_float<const char&>, const char&>);
static_assert(std::is_same_v<clamp_to_standard_float<const int&>, const int&>);
static_assert(std::is_same_v<clamp_to_standard_float<const half&>, const float&>);
static_assert(std::is_same_v<clamp_to_standard_float<const float&>, const float&>);
static_assert(std::is_same_v<clamp_to_standard_float<const double&>, const double&>);
static_assert(std::is_same_v<clamp_to_standard_float<const long double&>, const long double&>);
#if MUU_HAS_FLOAT128
static_assert(std::is_same_v<clamp_to_standard_float<float128_t>, long double>);
static_assert(std::is_same_v<clamp_to_standard_float<float128_t&>, long double&>);
static_assert(std::is_same_v<clamp_to_standard_float<const float128_t&>, const long double&>);
#endif

// is_function
static void test_func1() noexcept
{}
static void test_func2()
{}
static constexpr auto test_lambda1 = []() noexcept {};
static constexpr auto test_lambda2 = []() {};
static_assert(is_function<decltype(test_func1)>);
static_assert(is_function<decltype(test_func2)>);
static_assert(is_function<decltype(test_func1)&>);
static_assert(is_function<decltype(test_func2)&>);
static_assert(!is_function<decltype(test_func1)*>);
static_assert(!is_function<decltype(test_func2)*>);
static_assert(!is_function<decltype(test_func1)*&>);
static_assert(!is_function<decltype(test_func2)*&>);
static_assert(!is_function<decltype(test_lambda1)>);
static_assert(!is_function<decltype(test_lambda2)>);
static_assert(!is_function<decltype(test_lambda1)*>);
static_assert(!is_function<decltype(test_lambda2)*>);
static_assert(!is_function<decltype(test_lambda1)&>);
static_assert(!is_function<decltype(test_lambda2)&>);
static_assert(!is_function<decltype(+test_lambda1)>);
static_assert(!is_function<decltype(+test_lambda2)>);
static_assert(is_function<std::remove_pointer_t<decltype(+test_lambda1)>>);
static_assert(is_function<std::remove_pointer_t<decltype(+test_lambda2)>>);

// is_function_pointer
static_assert(!is_function_pointer<decltype(test_func1)>);
static_assert(!is_function_pointer<decltype(test_func2)>);
static_assert(!is_function_pointer<decltype(test_func1)&>);
static_assert(!is_function_pointer<decltype(test_func2)&>);
static_assert(is_function_pointer<decltype(test_func1)*>);
static_assert(is_function_pointer<decltype(test_func2)*>);
static_assert(is_function_pointer<decltype(test_func1)*&>);
static_assert(is_function_pointer<decltype(test_func2)*&>);
static_assert(!is_function_pointer<decltype(test_lambda1)>);
static_assert(!is_function_pointer<decltype(test_lambda2)>);
static_assert(!is_function_pointer<decltype(test_lambda1)*>);
static_assert(!is_function_pointer<decltype(test_lambda2)*>);
static_assert(!is_function_pointer<decltype(test_lambda1)&>);
static_assert(!is_function_pointer<decltype(test_lambda2)&>);
static_assert(is_function_pointer<decltype(+test_lambda1)>);
static_assert(is_function_pointer<decltype(+test_lambda2)>);
static_assert(!is_function_pointer<std::remove_pointer_t<decltype(+test_lambda1)>>);
static_assert(!is_function_pointer<std::remove_pointer_t<decltype(+test_lambda2)>>);

// is_stateless_lambda
static_assert(!is_stateless_lambda<decltype(test_func1)>);
static_assert(!is_stateless_lambda<decltype(test_func2)>);
static_assert(!is_stateless_lambda<decltype(test_func1)&>);
static_assert(!is_stateless_lambda<decltype(test_func2)&>);
static_assert(!is_stateless_lambda<decltype(test_func1)*>);
static_assert(!is_stateless_lambda<decltype(test_func2)*>);
static_assert(!is_stateless_lambda<decltype(test_func1)*&>);
static_assert(!is_stateless_lambda<decltype(test_func2)*&>);
static_assert(is_stateless_lambda<decltype(test_lambda1)>);
static_assert(is_stateless_lambda<decltype(test_lambda2)>);
static_assert(!is_stateless_lambda<decltype(test_lambda1)*>);
static_assert(!is_stateless_lambda<decltype(test_lambda2)*>);
static_assert(is_stateless_lambda<decltype(test_lambda1)&>);
static_assert(is_stateless_lambda<decltype(test_lambda2)&>);
static_assert(!is_stateless_lambda<decltype(+test_lambda1)>);
static_assert(!is_stateless_lambda<decltype(+test_lambda2)>);
static_assert(!is_stateless_lambda<std::remove_pointer_t<decltype(+test_lambda1)>>);
static_assert(!is_stateless_lambda<std::remove_pointer_t<decltype(+test_lambda2)>>);

[[maybe_unused]] static void stateless_lambda_test_container()
{
	[[maybe_unused]] int foo = std::rand();
	const auto test_lambda3	 = [&]() noexcept -> int { return foo ^ std::rand(); };

	static_assert(!is_stateless_lambda<decltype(test_lambda3)>);
	static_assert(!is_stateless_lambda<decltype(test_lambda3)*>);
	static_assert(!is_stateless_lambda<decltype(test_lambda3)&>);
}
