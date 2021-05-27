// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "batching.h"

TEST_CASE("core - is_constant_evaluated")
{
	static_assert(is_constant_evaluated() == build::supports_is_constant_evaluated);
	volatile bool val = is_constant_evaluated();
	CHECK(val == false);
}

//TEST_CASE("core - launder")
//{
//	// todo?? kinda impossible to test, without being the compiler itself.
//}

TEST_CASE("core - unwrap")
{
	enum class scoped_enum : uint32_t
	{
		zero, one, two
	};
	const auto se_val = scoped_enum{};
	static_assert(std::is_same_v<uint32_t, decltype(unwrap(se_val))>);
	static_assert(std::is_same_v<uint32_t, decltype(unwrap(std::declval<scoped_enum&>()))>);
	static_assert(std::is_same_v<uint32_t, decltype(unwrap(std::declval<const scoped_enum&>()))>);
	static_assert(std::is_same_v<uint32_t, decltype(unwrap(std::declval<volatile scoped_enum&>()))>);
	static_assert(std::is_same_v<uint32_t, decltype(unwrap(std::declval<const volatile scoped_enum&>()))>);
	static_assert(std::is_same_v<uint32_t, decltype(unwrap(std::declval<scoped_enum&&>()))>);
	static_assert(std::is_same_v<uint32_t, decltype(unwrap(std::declval<const scoped_enum&&>()))>);
	static_assert(std::is_same_v<uint32_t, decltype(unwrap(std::declval<volatile scoped_enum&&>()))>);
	static_assert(std::is_same_v<uint32_t, decltype(unwrap(std::declval<const volatile scoped_enum&&>()))>);

	CHECK_AND_STATIC_ASSERT(unwrap(scoped_enum::zero) == 0u);
	CHECK_AND_STATIC_ASSERT(unwrap(scoped_enum::one) == 1u);
	CHECK_AND_STATIC_ASSERT(unwrap(scoped_enum::two) == 2u);

	enum unscoped_enum : uint32_t
	{
		ue_zero, ue_one, ue_two
	};
	const auto ue_val = unscoped_enum{};
	static_assert(std::is_same_v<uint32_t, decltype(unwrap(ue_val))>);
	static_assert(std::is_same_v<uint32_t, decltype(unwrap(std::declval<unscoped_enum&>()))>);
	static_assert(std::is_same_v<uint32_t, decltype(unwrap(std::declval<const unscoped_enum&>()))>);
	static_assert(std::is_same_v<uint32_t, decltype(unwrap(std::declval<volatile unscoped_enum&>()))>);
	static_assert(std::is_same_v<uint32_t, decltype(unwrap(std::declval<const volatile unscoped_enum&>()))>);
	static_assert(std::is_same_v<uint32_t, decltype(unwrap(std::declval<unscoped_enum&&>()))>);
	static_assert(std::is_same_v<uint32_t, decltype(unwrap(std::declval<const unscoped_enum&&>()))>);
	static_assert(std::is_same_v<uint32_t, decltype(unwrap(std::declval<volatile unscoped_enum&&>()))>);
	static_assert(std::is_same_v<uint32_t, decltype(unwrap(std::declval<const volatile unscoped_enum&&>()))>);

	CHECK_AND_STATIC_ASSERT(unwrap(unscoped_enum::ue_zero) == 0u);
	CHECK_AND_STATIC_ASSERT(unwrap(unscoped_enum::ue_one) == 1u);
	CHECK_AND_STATIC_ASSERT(unwrap(unscoped_enum::ue_two) == 2u);

	struct not_an_enum {};
	const auto ne_val = not_an_enum{};
	static_assert(std::is_same_v<const not_an_enum&, decltype(unwrap(ne_val))>);
	static_assert(std::is_same_v<not_an_enum&, decltype(unwrap(std::declval<not_an_enum&>()))>);
	static_assert(std::is_same_v<const not_an_enum&, decltype(unwrap(std::declval<const not_an_enum&>()))>);
	static_assert(std::is_same_v<volatile not_an_enum&, decltype(unwrap(std::declval<volatile not_an_enum&>()))>);
	static_assert(std::is_same_v<const volatile not_an_enum&, decltype(unwrap(std::declval<const volatile not_an_enum&>()))>);
	static_assert(std::is_same_v<not_an_enum&&, decltype(unwrap(std::declval<not_an_enum&&>()))>);
	static_assert(std::is_same_v<const not_an_enum&&, decltype(unwrap(std::declval<const not_an_enum&&>()))>);
	static_assert(std::is_same_v<volatile not_an_enum&&, decltype(unwrap(std::declval<volatile not_an_enum&&>()))>);
	static_assert(std::is_same_v<const volatile not_an_enum&&, decltype(unwrap(std::declval<const volatile not_an_enum&&>()))>);

}

TEST_CASE("core - pack")
{
	CHECK_AND_STATIC_ASSERT(pack(0xFEDCBA98_u32, 0x76543210_u32) == 0xFEDCBA9876543210_u64);
	CHECK_AND_STATIC_ASSERT(pack(0xFEDC_u16, 0xBA98_u16, 0x76543210_u32) == 0xFEDCBA9876543210_u64);
	CHECK_AND_STATIC_ASSERT(pack(0xFEDC_u16, 0xBA98_u16, 0x7654_u16, 0x3210_u16) == 0xFEDCBA9876543210_u64);
	CHECK_AND_STATIC_ASSERT(pack(0xFEDC_u16, 0xBA_u8, 0x98_u8, 0x7654_u16, 0x32_u8, 0x10_u8) == 0xFEDCBA9876543210_u64);
}

//TEST_CASE("core - bit_cast")
//{
//	// todo
//}

namespace
{
	static int test_val = 0;
	MUU_NEVER_INLINE int func1() { return test_val = 1; }
	MUU_NEVER_INLINE int func2() noexcept { return test_val = 2; }

	struct base { virtual ~base() noexcept = default; };
	struct derived1 : base {};
	struct derived2 : base {};
}

TEST_CASE("core - pointer_cast")
{
	using int_ptr = int*;

	// same input and output types (no-op)
	CHECK(pointer_cast<int*>(&test_val) == &test_val);

	// nullptr -> *
	CHECK(pointer_cast<int*>(nullptr) == int_ptr{});

	// pointer -> integer
	// integer -> pointer
	CHECK(pointer_cast<intptr_t>(&test_val) == reinterpret_cast<intptr_t>(&test_val));
	CHECK(pointer_cast<uintptr_t>(&test_val) == reinterpret_cast<uintptr_t>(&test_val));

	// function pointers
	if constexpr (sizeof(void*) == sizeof(void(*)()))
	{
		// function -> void
		CHECK(pointer_cast<void*>(func1) == reinterpret_cast<void*>(func1));
		CHECK(pointer_cast<void*>(func2) == reinterpret_cast<void*>(func2));

		// void -> function
		// function -> function (noexcept)
		// function (noexcept) -> function
		{
			void* ptr1 = pointer_cast<void*>(func1);
			CHECK(pointer_cast<int(*)()>(ptr1) == reinterpret_cast<int(*)()>(ptr1));
			CHECK(pointer_cast<int(*)()>(ptr1) == func1);
			pointer_cast<int(*)()>(ptr1)();
			CHECK(test_val == 1);

			void* ptr2 = pointer_cast<void*>(func2);
			CHECK(pointer_cast<int(*)()noexcept>(ptr2) == reinterpret_cast<int(*)()noexcept>(ptr2));
			CHECK(pointer_cast<int(*)()noexcept>(ptr2) == func2);
			pointer_cast<int(*)()noexcept>(ptr2)();
			CHECK(test_val == 2);

			test_val = 0;

			pointer_cast<int(*)()noexcept>(func1)();
			CHECK(test_val == 1);
			pointer_cast<int(*)()>(func2)();
			CHECK(test_val == 2);

			test_val = 0;
		}
	}

	// void -> void (different cv)
	CHECK(pointer_cast<void*>(static_cast<const void*>(&test_val)) == &test_val);
	CHECK(pointer_cast<const void*>(static_cast<volatile void*>(&test_val)) == &test_val);
	CHECK(pointer_cast<volatile void*>(static_cast<const void*>(&test_val)) == &test_val);
	CHECK(pointer_cast<const volatile void*>(static_cast<volatile void*>(&test_val)) == &test_val);

	// * -> void
	CHECK(pointer_cast<void*>(&test_val) == &test_val);
	CHECK(pointer_cast<const void*>(&test_val) == &test_val);
	CHECK(pointer_cast<volatile void*>(&test_val) == &test_val);
	CHECK(pointer_cast<const volatile void*>(&test_val) == &test_val);

	// void -> *
	CHECK(pointer_cast<int*>(static_cast<void*>(&test_val)) == &test_val);
	CHECK(pointer_cast<const int*>(static_cast<void*>(&test_val)) == &test_val);
	CHECK(pointer_cast<volatile int*>(static_cast<void*>(&test_val)) == &test_val);
	CHECK(pointer_cast<const volatile int*>(static_cast<void*>(&test_val)) == &test_val);

	//derived -> base
	{
		derived1 d{};
		CHECK(pointer_cast<base*>(&d) == &d);
		CHECK(pointer_cast<const base*>(&d) == &d);
		CHECK(pointer_cast<volatile base*>(&d) == &d);
		CHECK(pointer_cast<const volatile base*>(&d) == &d);
	}

	// base -> derived
	{
		std::unique_ptr<base> b{ new derived1 };
		CHECK(pointer_cast<derived1*>(b.get()) == b.get());
		CHECK(pointer_cast<const derived1*>(b.get()) == b.get());
		CHECK(pointer_cast<volatile derived1*>(b.get()) == b.get());
		CHECK(pointer_cast<const volatile derived1*>(b.get()) == b.get());

		CHECK(pointer_cast<derived2*>(b.get()) == nullptr);
		CHECK(pointer_cast<const derived2*>(b.get()) == nullptr);
		CHECK(pointer_cast<volatile derived2*>(b.get()) == nullptr);
		CHECK(pointer_cast<const volatile derived2*>(b.get()) == nullptr);
	}

	// rank changes
	{
		void* void_ptr = &test_val;
		int* val_ptr = &test_val;
		int** val_ptr_ptr = &val_ptr;

		CHECK(void_ptr == pointer_cast<int***>(val_ptr));
		CHECK(pointer_cast<void*>(&val_ptr) == pointer_cast<const void*>(val_ptr_ptr));
	}
}

//TEST_CASE("core - apply_offset")
//{
//	// todo
//}

//TEST_CASE("core - min")
//{
//	// todo
//}

//TEST_CASE("core - max")
//{
//	// todo
//}

//TEST_CASE("core - abs")
//{
//	// todo
//}

TEST_CASE("core - clamp")
{
	CHECK_AND_STATIC_ASSERT(clamp(1, 2, 4) == 2);
	CHECK_AND_STATIC_ASSERT(clamp(2, 2, 4) == 2);
	CHECK_AND_STATIC_ASSERT(clamp(3, 2, 4) == 3);
	CHECK_AND_STATIC_ASSERT(clamp(4, 2, 4) == 4);
	CHECK_AND_STATIC_ASSERT(clamp(5, 2, 4) == 4);
}

namespace
{
	template <typename T>
	struct lerp_test_data
	{
		struct test_case
		{
			T start;
			T finish;
			T alpha;
			T expected;
		};

		using c = muu::constants<T>;

		static constexpr test_case cases[] =
		{
			/*  0 */ { -c::one,			c::one,			c::two,				c::three },
			/*  1 */ { c::zero,			c::one,			c::two,				c::two },
			/*  2 */ { -c::one,			c::zero,		c::two,				c::one },
			/*  3 */ { c::one,			-c::one,		c::two,				-c::three },
			/*  4 */ { c::zero,			-c::one,		c::two,				-c::two },
			/*  5 */ { c::one,			c::zero,		c::two,				-c::one },
			/*  6 */ { c::one,			c::two,			c::one,				c::two },
			/*  7 */ { c::one,			c::two,			c::two,				c::three },
			/*  8 */ { c::one,			c::two,			c::one_over_two,	c::three_over_two },
			/*  9 */ { c::one,			c::two,			c::zero,			c::one },
			/* 10 */ { c::one,			c::one,			c::two,				c::one },
			/* 11 */ { c::zero,			c::zero,		c::one_over_two,	c::zero },
			/* 12 */ { -c::five,		c::five,		c::one_over_two,	c::zero },
			/* 13 */ { c::negative_zero,	c::negative_zero,	c::one_over_two,	c::negative_zero }
		};
	};
}

BATCHED_TEST_CASE("core - lerp", type_list<ALL_FLOATS>)
{
	using T = TestType;
	TEST_INFO(nameof<T>);

	size_t i{};
	for (const auto& case_ : lerp_test_data<T>::cases)
	{
		TEST_INFO("test case " << i);
		CHECK(muu::lerp(case_.start, case_.finish, case_.alpha) == case_.expected);
		i++;
	}
}

TEST_CASE("core - between")
{
	// signed, signed
	CHECK_AND_STATIC_ASSERT(!between(   -1,     2,     4));
	CHECK_AND_STATIC_ASSERT(!between(    0,     2,     4));
	CHECK_AND_STATIC_ASSERT(!between(    1,     2,     4));
	CHECK_AND_STATIC_ASSERT( between(    2,     2,     4));
	CHECK_AND_STATIC_ASSERT( between(    3,     2,     4));
	CHECK_AND_STATIC_ASSERT( between(    4,     2,     4));
	CHECK_AND_STATIC_ASSERT(!between(    5,     2,     4));

	// signed, unsigned
	CHECK_AND_STATIC_ASSERT(!between(   -1,    2u,    4u));
	CHECK_AND_STATIC_ASSERT(!between(    0,    2u,    4u));
	CHECK_AND_STATIC_ASSERT(!between(    1,    2u,    4u));
	CHECK_AND_STATIC_ASSERT( between(    2,    2u,    4u));
	CHECK_AND_STATIC_ASSERT( between(    3,    2u,    4u));
	CHECK_AND_STATIC_ASSERT( between(    4,    2u,    4u));
	CHECK_AND_STATIC_ASSERT(!between(    5,    2u,    4u));

	// float, signed
	CHECK_AND_STATIC_ASSERT(!between(-1.0f,     2,     4));
	CHECK_AND_STATIC_ASSERT(!between( 0.0f,     2,     4));
	CHECK_AND_STATIC_ASSERT(!between( 1.0f,     2,     4));
	CHECK_AND_STATIC_ASSERT( between( 2.0f,     2,     4));
	CHECK_AND_STATIC_ASSERT( between( 3.0f,     2,     4));
	CHECK_AND_STATIC_ASSERT( between( 4.0f,     2,     4));
	CHECK_AND_STATIC_ASSERT(!between( 5.0f,     2,     4));

	// float, unsigned
	CHECK_AND_STATIC_ASSERT(!between(-1.0f,    2u,    4u));
	CHECK_AND_STATIC_ASSERT(!between( 0.0f,    2u,    4u));
	CHECK_AND_STATIC_ASSERT(!between( 1.0f,    2u,    4u));
	CHECK_AND_STATIC_ASSERT( between( 2.0f,    2u,    4u));
	CHECK_AND_STATIC_ASSERT( between( 3.0f,    2u,    4u));
	CHECK_AND_STATIC_ASSERT( between( 4.0f,    2u,    4u));
	CHECK_AND_STATIC_ASSERT(!between( 5.0f,    2u,    4u));

	// signed, float
	CHECK_AND_STATIC_ASSERT(!between(   -1,  2.0f,  4.0f));
	CHECK_AND_STATIC_ASSERT(!between(    0,  2.0f,  4.0f));
	CHECK_AND_STATIC_ASSERT(!between(    1,  2.0f,  4.0f));
	CHECK_AND_STATIC_ASSERT( between(    2,  2.0f,  4.0f));
	CHECK_AND_STATIC_ASSERT( between(    3,  2.0f,  4.0f));
	CHECK_AND_STATIC_ASSERT( between(    4,  2.0f,  4.0f));
	CHECK_AND_STATIC_ASSERT(!between(    5,  2.0f,  4.0f));

	// check for integer overflow nonsense
	{
		constexpr uint8_t minval = 5;
		constexpr uint8_t maxval = 100;
		for (int32_t i = -128; i <= 4; i++)
			CHECK(!between(i, minval, maxval));
		for (int32_t i = 5; i <= 100; i++)
			CHECK(between(i, minval, maxval));
		for (int32_t i = 101; i <= 255; i++)
			CHECK(!between(i, minval, maxval));
	}
}

TEST_CASE("core - byte_select")
{
	#define CHECK_BYTE_SELECT(index, expected, ...)										\
		CHECK_AND_STATIC_ASSERT(byte_select<index>(__VA_ARGS__) == 0x##expected##_u8);	\
		CHECK_AND_STATIC_ASSERT(byte_select(__VA_ARGS__, index) == 0x##expected##_u8)


	#if MUU_HAS_INT128
	CHECK_BYTE_SELECT(15, FE, pack(0xFEDCBA9876543210_u64, 0xAABBCCDDABCDEF01_u64));
	CHECK_BYTE_SELECT(14, DC, pack(0xFEDCBA9876543210_u64, 0xAABBCCDDABCDEF01_u64));
	CHECK_BYTE_SELECT(13, BA, pack(0xFEDCBA9876543210_u64, 0xAABBCCDDABCDEF01_u64));
	CHECK_BYTE_SELECT(12, 98, pack(0xFEDCBA9876543210_u64, 0xAABBCCDDABCDEF01_u64));
	CHECK_BYTE_SELECT(11, 76, pack(0xFEDCBA9876543210_u64, 0xAABBCCDDABCDEF01_u64));
	CHECK_BYTE_SELECT(10, 54, pack(0xFEDCBA9876543210_u64, 0xAABBCCDDABCDEF01_u64));
	CHECK_BYTE_SELECT( 9, 32, pack(0xFEDCBA9876543210_u64, 0xAABBCCDDABCDEF01_u64));
	CHECK_BYTE_SELECT( 8, 10, pack(0xFEDCBA9876543210_u64, 0xAABBCCDDABCDEF01_u64));
	CHECK_BYTE_SELECT( 7, AA, pack(0xFEDCBA9876543210_u64, 0xAABBCCDDABCDEF01_u64));
	CHECK_BYTE_SELECT( 6, BB, pack(0xFEDCBA9876543210_u64, 0xAABBCCDDABCDEF01_u64));
	CHECK_BYTE_SELECT( 5, CC, pack(0xFEDCBA9876543210_u64, 0xAABBCCDDABCDEF01_u64));
	CHECK_BYTE_SELECT( 4, DD, pack(0xFEDCBA9876543210_u64, 0xAABBCCDDABCDEF01_u64));
	CHECK_BYTE_SELECT( 3, AB, pack(0xFEDCBA9876543210_u64, 0xAABBCCDDABCDEF01_u64));
	CHECK_BYTE_SELECT( 2, CD, pack(0xFEDCBA9876543210_u64, 0xAABBCCDDABCDEF01_u64));
	CHECK_BYTE_SELECT( 1, EF, pack(0xFEDCBA9876543210_u64, 0xAABBCCDDABCDEF01_u64));
	CHECK_BYTE_SELECT( 0, 01, pack(0xFEDCBA9876543210_u64, 0xAABBCCDDABCDEF01_u64));
	#endif

	CHECK_BYTE_SELECT(7, AA, 0xAABBCCDDABCDEF01_u64);
	CHECK_BYTE_SELECT(6, BB, 0xAABBCCDDABCDEF01_u64);
	CHECK_BYTE_SELECT(5, CC, 0xAABBCCDDABCDEF01_u64);
	CHECK_BYTE_SELECT(4, DD, 0xAABBCCDDABCDEF01_u64);
	CHECK_BYTE_SELECT(3, AB, 0xAABBCCDDABCDEF01_u64);
	CHECK_BYTE_SELECT(2, CD, 0xAABBCCDDABCDEF01_u64);
	CHECK_BYTE_SELECT(1, EF, 0xAABBCCDDABCDEF01_u64);
	CHECK_BYTE_SELECT(0, 01, 0xAABBCCDDABCDEF01_u64);

	CHECK_BYTE_SELECT(3, AB, 0xABCDEF01_u32);
	CHECK_BYTE_SELECT(2, CD, 0xABCDEF01_u32);
	CHECK_BYTE_SELECT(1, EF, 0xABCDEF01_u32);
	CHECK_BYTE_SELECT(0, 01, 0xABCDEF01_u32);

	CHECK_BYTE_SELECT(1, EF, 0xEF01_u16);
	CHECK_BYTE_SELECT(0, 01, 0xEF01_u16);

	CHECK_BYTE_SELECT(0, 01, 0x01_u8);
}

TEST_CASE("core - byte_reverse")
{
	#if MUU_HAS_INT128

	CHECK_AND_STATIC_ASSERT(byte_reverse(pack(0xFEDCBA9876543210_u64, 0xAABBCCDDABCDEF01_u64)) == pack(0x01EFCDABDDCCBBAA_u64, 0x1032547698BADCFE_u64));

	#endif

	CHECK_AND_STATIC_ASSERT(byte_reverse(0xAABBCCDDABCDEF01_u64) == 0x01EFCDABDDCCBBAA_u64);
	CHECK_AND_STATIC_ASSERT(byte_reverse(0xABCDEF01_u32) == 0x01EFCDAB_u32);
	CHECK_AND_STATIC_ASSERT(byte_reverse(0xABCD_u16) == 0xCDAB_u16);
}

TEST_CASE("core - swizzle")
{
	#define CHECK_SWIZZLE(input, expected, ...)	\
		CHECK_AND_STATIC_ASSERT(swizzle<__VA_ARGS__>(input) == expected)

	CHECK_SWIZZLE(0xAABBCCDD_u32, 0xDD_u8, 0);
	CHECK_SWIZZLE(0xAABBCCDD_u32, 0xCCDD_u16, 1, 0);
	CHECK_SWIZZLE(0xAABBCCDD_u32, 0xBBCCDD_u32, 2, 1, 0);
	CHECK_SWIZZLE(0xAABBCCDD_u32, 0xAABBCCDD_u32, 3, 2, 1, 0);
	CHECK_SWIZZLE(0xAABBCCDD_u32, 0xAA_u8, 3);
	CHECK_SWIZZLE(0xAABBCCDD_u32, 0xCCCC_u16, 1, 1);
	CHECK_SWIZZLE(0xAABBCCDD_u32, 0xBBDDBB_u32, 2, 0, 2);
	CHECK_SWIZZLE(0xAABBCCDD_u32, 0xAAAABBBB_u32, 3, 3, 2, 2);
	CHECK_SWIZZLE(0xAABBCCDD_u32, 0xAACCBBCCDD_u64, 3, 1, 2, 1, 0);

	CHECK_SWIZZLE(0xAABBCCDD_i32, 0xDD_i8, 0);
	CHECK_SWIZZLE(0xAABBCCDD_i32, 0xCCDD_i16, 1, 0);
	CHECK_SWIZZLE(0xAABBCCDD_i32, 0xBBCCDD_i32, 2, 1, 0);
	CHECK_SWIZZLE(0xAABBCCDD_i32, 0xAABBCCDD_i32, 3, 2, 1, 0);
	CHECK_SWIZZLE(0xAABBCCDD_i32, 0xAA_i8, 3);
	CHECK_SWIZZLE(0xAABBCCDD_i32, 0xCCCC_i16, 1, 1);
	CHECK_SWIZZLE(0xAABBCCDD_i32, 0xBBDDBB_i32, 2, 0, 2);
	CHECK_SWIZZLE(0xAABBCCDD_i32, 0xAAAABBBB_i32, 3, 3, 2, 2);
	CHECK_SWIZZLE(0xAABBCCDD_i32, 0xAACCBBCCDD_i64, 3, 1, 2, 1, 0);
}

TEST_CASE("core - for_sequence")
{
	SECTION("rvalue func")
	{
		int nums[10];
		for_sequence<10>([&](auto i) { nums[i] = i; });
		for (int i = 0; i < 10; i++)
			CHECK(nums[i] == i);
	}

	SECTION("lvalue func")
	{
		int nums[10];
		const auto func = [&](auto i) { nums[i] = i; };
		for_sequence<10>(func);
		for (int i = 0; i < 10; i++)
			CHECK(nums[i] == i);
	}

	SECTION("rvalue nothrow func")
	{
		int nums[10];
		for_sequence<10>([&](auto i) noexcept { nums[i] = i; });
		for (int i = 0; i < 10; i++)
			CHECK(nums[i] == i);
	}

	SECTION("lvalue nothrow func")
	{
		int nums[10];
		const auto func = [&](auto i) noexcept { nums[i] = i; };
		for_sequence<10>(func);
		for (int i = 0; i < 10; i++)
			CHECK(nums[i] == i);

	}
}
