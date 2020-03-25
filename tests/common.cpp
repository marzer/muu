#include "tests.h"

// is_const - should return true when const or reference-to-const
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

// remove_const - should remove const from types or references
static_assert(std::is_same_v<remove_const<int>, int>);
static_assert(std::is_same_v<remove_const<int&>, int&>);
static_assert(std::is_same_v<remove_const<const int>, int>);
static_assert(std::is_same_v<remove_const<const int&>, int&>);
static_assert(std::is_same_v<remove_const<volatile int>, volatile int>);
static_assert(std::is_same_v<remove_const<volatile int&>, volatile int&>);
static_assert(std::is_same_v<remove_const<const volatile int>, volatile int>);
static_assert(std::is_same_v<remove_const<const volatile int&>, volatile int&>);

// conditionally_add_const - should add const from types or references per condition
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

// match_const - should set constness of types and references by matching that of another type
static_assert(std::is_same_v<match_const<int, float>, int>);
static_assert(std::is_same_v<match_const<int, const float>, const int>);
static_assert(std::is_same_v<match_const<int, volatile float>, int>);
static_assert(std::is_same_v<match_const<int, const volatile float>, const int>);
static_assert(std::is_same_v<match_const<const int, float>, int>);
static_assert(std::is_same_v<match_const<const int, const float>, const int>);
static_assert(std::is_same_v<match_const<const int, volatile float>, int>);
static_assert(std::is_same_v<match_const<const int, const volatile float>, const int>);
static_assert(std::is_same_v<match_const<const volatile int, float>, volatile int>);
static_assert(std::is_same_v<match_const<const volatile int, const float>, const volatile int>);
static_assert(std::is_same_v<match_const<const volatile int, volatile float>, volatile int>);
static_assert(std::is_same_v<match_const<const volatile int, const volatile float>, const volatile int>);
static_assert(std::is_same_v<match_const<int&, float>, int&>);
static_assert(std::is_same_v<match_const<int&, const float>, const int&>);
static_assert(std::is_same_v<match_const<int&, volatile float>, int&>);
static_assert(std::is_same_v<match_const<int&, const volatile float>, const int&>);
static_assert(std::is_same_v<match_const<const int&, float>, int&>);
static_assert(std::is_same_v<match_const<const int&, const float>, const int&>);
static_assert(std::is_same_v<match_const<const int&, volatile float>, int&>);
static_assert(std::is_same_v<match_const<const int&, const volatile float>, const int&>);
static_assert(std::is_same_v<match_const<const volatile int&, float>, volatile int&>);
static_assert(std::is_same_v<match_const<const volatile int&, const float>, const volatile int&>);
static_assert(std::is_same_v<match_const<const volatile int&, volatile float>, volatile int&>);
static_assert(std::is_same_v<match_const<const volatile int&, const volatile float>, const volatile int&>);
static_assert(std::is_same_v<match_const<int, float&>, int>);
static_assert(std::is_same_v<match_const<int, const float&>, const int>);
static_assert(std::is_same_v<match_const<int, volatile float&>, int>);
static_assert(std::is_same_v<match_const<int, const volatile float&>, const int>);
static_assert(std::is_same_v<match_const<const int, float&>, int>);
static_assert(std::is_same_v<match_const<const int, const float&>, const int>);
static_assert(std::is_same_v<match_const<const int, volatile float&>, int>);
static_assert(std::is_same_v<match_const<const int, const volatile float&>, const int>);
static_assert(std::is_same_v<match_const<const volatile int, float&>, volatile int>);
static_assert(std::is_same_v<match_const<const volatile int, const float&>, const volatile int>);
static_assert(std::is_same_v<match_const<const volatile int, volatile float&>, volatile int>);
static_assert(std::is_same_v<match_const<const volatile int, const volatile float&>, const volatile int>);

// is_const - should return true when volatile or reference-to-volatile
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

// remove_volatile - should remove volatility from types or references
static_assert(std::is_same_v<remove_volatile<int>, int>);
static_assert(std::is_same_v<remove_volatile<int&>, int&>);
static_assert(std::is_same_v<remove_volatile<const int>, const int>);
static_assert(std::is_same_v<remove_volatile<const int&>, const int&>);
static_assert(std::is_same_v<remove_volatile<volatile int>, int>);
static_assert(std::is_same_v<remove_volatile<volatile int&>, int&>);
static_assert(std::is_same_v<remove_volatile<const volatile int>, const int>);
static_assert(std::is_same_v<remove_volatile<const volatile int&>, const int&>);

// conditionally_add_volatile - should add volatile from types or references per condition
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

// match_volatile - should set volatility of types and references by matching that of another type
static_assert(std::is_same_v<match_volatile<int, float>, int>);
static_assert(std::is_same_v<match_volatile<int, const float>, int>);
static_assert(std::is_same_v<match_volatile<int, volatile float>, volatile int>);
static_assert(std::is_same_v<match_volatile<int, const volatile float>, volatile int>);
static_assert(std::is_same_v<match_volatile<const int, float>, const int>);
static_assert(std::is_same_v<match_volatile<const int, const float>, const int>);
static_assert(std::is_same_v<match_volatile<const int, volatile float>, const volatile int>);
static_assert(std::is_same_v<match_volatile<const int, const volatile float>, const volatile int>);
static_assert(std::is_same_v<match_volatile<const volatile int, float>, const int>);
static_assert(std::is_same_v<match_volatile<const volatile int, const float>, const int>);
static_assert(std::is_same_v<match_volatile<const volatile int, volatile float>, const volatile int>);
static_assert(std::is_same_v<match_volatile<const volatile int, const volatile float>, const volatile int>);
static_assert(std::is_same_v<match_volatile<int&, float>, int&>);
static_assert(std::is_same_v<match_volatile<int&, const float>, int&>);
static_assert(std::is_same_v<match_volatile<int&, volatile float>, volatile int&>);
static_assert(std::is_same_v<match_volatile<int&, const volatile float>, volatile int&>);
static_assert(std::is_same_v<match_volatile<const int&, float>, const int&>);
static_assert(std::is_same_v<match_volatile<const int&, const float>, const int&>);
static_assert(std::is_same_v<match_volatile<const int&, volatile float>, const volatile int&>);
static_assert(std::is_same_v<match_volatile<const int&, const volatile float>, const volatile int&>);
static_assert(std::is_same_v<match_volatile<const volatile int&, float>, const int&>);
static_assert(std::is_same_v<match_volatile<const volatile int&, const float>, const int&>);
static_assert(std::is_same_v<match_volatile<const volatile int&, volatile float>, const volatile int&>);
static_assert(std::is_same_v<match_volatile<const volatile int&, const volatile float>, const volatile int&>);
static_assert(std::is_same_v<match_volatile<int, float&>, int>);
static_assert(std::is_same_v<match_volatile<int, const float&>, int>);
static_assert(std::is_same_v<match_volatile<int, volatile float&>, volatile int>);
static_assert(std::is_same_v<match_volatile<int, const volatile float&>, volatile int>);
static_assert(std::is_same_v<match_volatile<const int, float&>, const int>);
static_assert(std::is_same_v<match_volatile<const int, const float&>, const int>);
static_assert(std::is_same_v<match_volatile<const int, volatile float&>, const volatile int>);
static_assert(std::is_same_v<match_volatile<const int, const volatile float&>, const volatile int>);
static_assert(std::is_same_v<match_volatile<const volatile int, float&>, const int>);
static_assert(std::is_same_v<match_volatile<const volatile int, const float&>, const int>);
static_assert(std::is_same_v<match_volatile<const volatile int, volatile float&>, const volatile int>);
static_assert(std::is_same_v<match_volatile<const volatile int, const volatile float&>, const volatile int>);

// remove_volatile - should remove constness and volatility from types or references
static_assert(std::is_same_v<remove_cv<int>, int>);
static_assert(std::is_same_v<remove_cv<int&>, int&>);
static_assert(std::is_same_v<remove_cv<const int>, int>);
static_assert(std::is_same_v<remove_cv<const int&>, int&>);
static_assert(std::is_same_v<remove_cv<volatile int>, int>);
static_assert(std::is_same_v<remove_cv<volatile int&>, int&>);
static_assert(std::is_same_v<remove_cv<const volatile int>, int>);
static_assert(std::is_same_v<remove_cv<const volatile int&>, int&>);

// match_cv - should set constness and volatility of types and references by matching that of another type
static_assert(std::is_same_v<match_cv<int, float>, int>);
static_assert(std::is_same_v<match_cv<int, const float>, const int>);
static_assert(std::is_same_v<match_cv<int, volatile float>, volatile int>);
static_assert(std::is_same_v<match_cv<int, const volatile float>, const volatile int>);
static_assert(std::is_same_v<match_cv<const int, float>, int>);
static_assert(std::is_same_v<match_cv<const int, const float>, const int>);
static_assert(std::is_same_v<match_cv<const int, volatile float>, volatile int>);
static_assert(std::is_same_v<match_cv<const int, const volatile float>, const volatile int>);
static_assert(std::is_same_v<match_cv<const volatile int, float>, int>);
static_assert(std::is_same_v<match_cv<const volatile int, const float>, const int>);
static_assert(std::is_same_v<match_cv<const volatile int, volatile float>, volatile int>);
static_assert(std::is_same_v<match_cv<const volatile int, const volatile float>, const volatile int>);
static_assert(std::is_same_v<match_cv<int&, float>, int&>);
static_assert(std::is_same_v<match_cv<int&, const float>, const int&>);
static_assert(std::is_same_v<match_cv<int&, volatile float>, volatile int&>);
static_assert(std::is_same_v<match_cv<int&, const volatile float>, const volatile int&>);
static_assert(std::is_same_v<match_cv<const int&, float>, int&>);
static_assert(std::is_same_v<match_cv<const int&, const float>, const int&>);
static_assert(std::is_same_v<match_cv<const int&, volatile float>, volatile int&>);
static_assert(std::is_same_v<match_cv<const int&, const volatile float>, const volatile int&>);
static_assert(std::is_same_v<match_cv<const volatile int&, float>, int&>);
static_assert(std::is_same_v<match_cv<const volatile int&, const float>, const int&>);
static_assert(std::is_same_v<match_cv<const volatile int&, volatile float>, volatile int&>);
static_assert(std::is_same_v<match_cv<const volatile int&, const volatile float>, const volatile int&>);
static_assert(std::is_same_v<match_cv<int, float&>, int>);
static_assert(std::is_same_v<match_cv<int, const float&>, const int>);
static_assert(std::is_same_v<match_cv<int, volatile float&>, volatile int>);
static_assert(std::is_same_v<match_cv<int, const volatile float&>, const volatile int>);
static_assert(std::is_same_v<match_cv<const int, float&>, int>);
static_assert(std::is_same_v<match_cv<const int, const float&>, const int>);
static_assert(std::is_same_v<match_cv<const int, volatile float&>, volatile int>);
static_assert(std::is_same_v<match_cv<const int, const volatile float&>, const volatile int>);
static_assert(std::is_same_v<match_cv<const volatile int, float&>, int>);
static_assert(std::is_same_v<match_cv<const volatile int, const float&>, const int>);
static_assert(std::is_same_v<match_cv<const volatile int, volatile float&>, volatile int>);
static_assert(std::is_same_v<match_cv<const volatile int, const volatile float&>, const volatile int>);

TEST_CASE("has_single_bit")
{
	static_assert(has_single_bit(1u));
	static_assert(has_single_bit(2u));
	static_assert(!has_single_bit(3u));
	static_assert(has_single_bit(4u));
	static_assert(!has_single_bit(5u));
	static_assert(!has_single_bit(6u));
	static_assert(!has_single_bit(7u));
	static_assert(has_single_bit(8u));
	static_assert(!has_single_bit(9u));
	static_assert(!has_single_bit(10u));

	CHECK(has_single_bit(1u));
	CHECK(has_single_bit(2u));
	CHECK(!has_single_bit(3u));
	CHECK(has_single_bit(4u));
	CHECK(!has_single_bit(5u));
	CHECK(!has_single_bit(6u));
	CHECK(!has_single_bit(7u));
	CHECK(has_single_bit(8u));
	CHECK(!has_single_bit(9u));
	CHECK(!has_single_bit(10u));
}

TEST_CASE("countl_zero")
{
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000000000000000000000000_u64) == 64);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000000000000000000000001_u64) == 63);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000000000000000000000011_u64) == 62);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000000000000000000000111_u64) == 61);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000000000000000000001111_u64) == 60);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000000000000000000011111_u64) == 59);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000000000000000000111111_u64) == 58);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000000000000000001111111_u64) == 57);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000000000000000011111111_u64) == 56);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000000000000000111111111_u64) == 55);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000000000000001111111111_u64) == 54);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000000000000011111111111_u64) == 53);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000000000000111111111111_u64) == 52);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000000000001111111111111_u64) == 51);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000000000011111111111111_u64) == 50);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000000000111111111111111_u64) == 49);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000000001111111111111111_u64) == 48);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000000011111111111111111_u64) == 47);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000000111111111111111111_u64) == 46);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000001111111111111111111_u64) == 45);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000011111111111111111111_u64) == 44);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000000111111111111111111111_u64) == 43);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000001111111111111111111111_u64) == 42);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000011111111111111111111111_u64) == 41);
	static_assert(countl_zero(0b0000000000000000000000000000000000000000111111111111111111111111_u64) == 40);
	static_assert(countl_zero(0b0000000000000000000000000000000000000001111111111111111111111111_u64) == 39);
	static_assert(countl_zero(0b0000000000000000000000000000000000000011111111111111111111111111_u64) == 38);
	static_assert(countl_zero(0b0000000000000000000000000000000000000111111111111111111111111111_u64) == 37);
	static_assert(countl_zero(0b0000000000000000000000000000000000001111111111111111111111111111_u64) == 36);
	static_assert(countl_zero(0b0000000000000000000000000000000000011111111111111111111111111111_u64) == 35);
	static_assert(countl_zero(0b0000000000000000000000000000000000111111111111111111111111111111_u64) == 34);
	static_assert(countl_zero(0b0000000000000000000000000000000001111111111111111111111111111111_u64) == 33);
	static_assert(countl_zero(0b0000000000000000000000000000000011111111111111111111111111111111_u64) == 32);
	static_assert(countl_zero(0b0000000000000000000000000000000111111111111111111111111111111111_u64) == 31);
	static_assert(countl_zero(0b0000000000000000000000000000001111111111111111111111111111111111_u64) == 30);
	static_assert(countl_zero(0b0000000000000000000000000000011111111111111111111111111111111111_u64) == 29);
	static_assert(countl_zero(0b0000000000000000000000000000111111111111111111111111111111111111_u64) == 28);
	static_assert(countl_zero(0b0000000000000000000000000001111111111111111111111111111111111111_u64) == 27);
	static_assert(countl_zero(0b0000000000000000000000000011111111111111111111111111111111111111_u64) == 26);
	static_assert(countl_zero(0b0000000000000000000000000111111111111111111111111111111111111111_u64) == 25);
	static_assert(countl_zero(0b0000000000000000000000001111111111111111111111111111111111111111_u64) == 24);
	static_assert(countl_zero(0b0000000000000000000000011111111111111111111111111111111111111111_u64) == 23);
	static_assert(countl_zero(0b0000000000000000000000111111111111111111111111111111111111111111_u64) == 22);
	static_assert(countl_zero(0b0000000000000000000001111111111111111111111111111111111111111111_u64) == 21);
	static_assert(countl_zero(0b0000000000000000000011111111111111111111111111111111111111111111_u64) == 20);
	static_assert(countl_zero(0b0000000000000000000111111111111111111111111111111111111111111111_u64) == 19);
	static_assert(countl_zero(0b0000000000000000001111111111111111111111111111111111111111111111_u64) == 18);
	static_assert(countl_zero(0b0000000000000000011111111111111111111111111111111111111111111111_u64) == 17);
	static_assert(countl_zero(0b0000000000000000111111111111111111111111111111111111111111111111_u64) == 16);
	static_assert(countl_zero(0b0000000000000001111111111111111111111111111111111111111111111111_u64) == 15);
	static_assert(countl_zero(0b0000000000000011111111111111111111111111111111111111111111111111_u64) == 14);
	static_assert(countl_zero(0b0000000000000111111111111111111111111111111111111111111111111111_u64) == 13);
	static_assert(countl_zero(0b0000000000001111111111111111111111111111111111111111111111111111_u64) == 12);
	static_assert(countl_zero(0b0000000000011111111111111111111111111111111111111111111111111111_u64) == 11);
	static_assert(countl_zero(0b0000000000111111111111111111111111111111111111111111111111111111_u64) == 10);
	static_assert(countl_zero(0b0000000001111111111111111111111111111111111111111111111111111111_u64) == 9);
	static_assert(countl_zero(0b0000000011111111111111111111111111111111111111111111111111111111_u64) == 8);
	static_assert(countl_zero(0b0000000111111111111111111111111111111111111111111111111111111111_u64) == 7);
	static_assert(countl_zero(0b0000001111111111111111111111111111111111111111111111111111111111_u64) == 6);
	static_assert(countl_zero(0b0000011111111111111111111111111111111111111111111111111111111111_u64) == 5);
	static_assert(countl_zero(0b0000111111111111111111111111111111111111111111111111111111111111_u64) == 4);
	static_assert(countl_zero(0b0001111111111111111111111111111111111111111111111111111111111111_u64) == 3);
	static_assert(countl_zero(0b0011111111111111111111111111111111111111111111111111111111111111_u64) == 2);
	static_assert(countl_zero(0b0111111111111111111111111111111111111111111111111111111111111111_u64) == 1);
	static_assert(countl_zero(0b1111111111111111111111111111111111111111111111111111111111111111_u64) == 0);

	CHECK(countl_zero(0b0000000000000000000000000000000000000000000000000000000000000000_u64) == 64);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000000000000000000000001_u64) == 63);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000000000000000000000011_u64) == 62);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000000000000000000000111_u64) == 61);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000000000000000000001111_u64) == 60);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000000000000000000011111_u64) == 59);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000000000000000000111111_u64) == 58);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000000000000000001111111_u64) == 57);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000000000000000011111111_u64) == 56);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000000000000000111111111_u64) == 55);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000000000000001111111111_u64) == 54);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000000000000011111111111_u64) == 53);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000000000000111111111111_u64) == 52);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000000000001111111111111_u64) == 51);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000000000011111111111111_u64) == 50);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000000000111111111111111_u64) == 49);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000000001111111111111111_u64) == 48);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000000011111111111111111_u64) == 47);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000000111111111111111111_u64) == 46);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000001111111111111111111_u64) == 45);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000011111111111111111111_u64) == 44);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000000111111111111111111111_u64) == 43);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000001111111111111111111111_u64) == 42);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000011111111111111111111111_u64) == 41);
	CHECK(countl_zero(0b0000000000000000000000000000000000000000111111111111111111111111_u64) == 40);
	CHECK(countl_zero(0b0000000000000000000000000000000000000001111111111111111111111111_u64) == 39);
	CHECK(countl_zero(0b0000000000000000000000000000000000000011111111111111111111111111_u64) == 38);
	CHECK(countl_zero(0b0000000000000000000000000000000000000111111111111111111111111111_u64) == 37);
	CHECK(countl_zero(0b0000000000000000000000000000000000001111111111111111111111111111_u64) == 36);
	CHECK(countl_zero(0b0000000000000000000000000000000000011111111111111111111111111111_u64) == 35);
	CHECK(countl_zero(0b0000000000000000000000000000000000111111111111111111111111111111_u64) == 34);
	CHECK(countl_zero(0b0000000000000000000000000000000001111111111111111111111111111111_u64) == 33);
	CHECK(countl_zero(0b0000000000000000000000000000000011111111111111111111111111111111_u64) == 32);
	CHECK(countl_zero(0b0000000000000000000000000000000111111111111111111111111111111111_u64) == 31);
	CHECK(countl_zero(0b0000000000000000000000000000001111111111111111111111111111111111_u64) == 30);
	CHECK(countl_zero(0b0000000000000000000000000000011111111111111111111111111111111111_u64) == 29);
	CHECK(countl_zero(0b0000000000000000000000000000111111111111111111111111111111111111_u64) == 28);
	CHECK(countl_zero(0b0000000000000000000000000001111111111111111111111111111111111111_u64) == 27);
	CHECK(countl_zero(0b0000000000000000000000000011111111111111111111111111111111111111_u64) == 26);
	CHECK(countl_zero(0b0000000000000000000000000111111111111111111111111111111111111111_u64) == 25);
	CHECK(countl_zero(0b0000000000000000000000001111111111111111111111111111111111111111_u64) == 24);
	CHECK(countl_zero(0b0000000000000000000000011111111111111111111111111111111111111111_u64) == 23);
	CHECK(countl_zero(0b0000000000000000000000111111111111111111111111111111111111111111_u64) == 22);
	CHECK(countl_zero(0b0000000000000000000001111111111111111111111111111111111111111111_u64) == 21);
	CHECK(countl_zero(0b0000000000000000000011111111111111111111111111111111111111111111_u64) == 20);
	CHECK(countl_zero(0b0000000000000000000111111111111111111111111111111111111111111111_u64) == 19);
	CHECK(countl_zero(0b0000000000000000001111111111111111111111111111111111111111111111_u64) == 18);
	CHECK(countl_zero(0b0000000000000000011111111111111111111111111111111111111111111111_u64) == 17);
	CHECK(countl_zero(0b0000000000000000111111111111111111111111111111111111111111111111_u64) == 16);
	CHECK(countl_zero(0b0000000000000001111111111111111111111111111111111111111111111111_u64) == 15);
	CHECK(countl_zero(0b0000000000000011111111111111111111111111111111111111111111111111_u64) == 14);
	CHECK(countl_zero(0b0000000000000111111111111111111111111111111111111111111111111111_u64) == 13);
	CHECK(countl_zero(0b0000000000001111111111111111111111111111111111111111111111111111_u64) == 12);
	CHECK(countl_zero(0b0000000000011111111111111111111111111111111111111111111111111111_u64) == 11);
	CHECK(countl_zero(0b0000000000111111111111111111111111111111111111111111111111111111_u64) == 10);
	CHECK(countl_zero(0b0000000001111111111111111111111111111111111111111111111111111111_u64) == 9);
	CHECK(countl_zero(0b0000000011111111111111111111111111111111111111111111111111111111_u64) == 8);
	CHECK(countl_zero(0b0000000111111111111111111111111111111111111111111111111111111111_u64) == 7);
	CHECK(countl_zero(0b0000001111111111111111111111111111111111111111111111111111111111_u64) == 6);
	CHECK(countl_zero(0b0000011111111111111111111111111111111111111111111111111111111111_u64) == 5);
	CHECK(countl_zero(0b0000111111111111111111111111111111111111111111111111111111111111_u64) == 4);
	CHECK(countl_zero(0b0001111111111111111111111111111111111111111111111111111111111111_u64) == 3);
	CHECK(countl_zero(0b0011111111111111111111111111111111111111111111111111111111111111_u64) == 2);
	CHECK(countl_zero(0b0111111111111111111111111111111111111111111111111111111111111111_u64) == 1);
	CHECK(countl_zero(0b1111111111111111111111111111111111111111111111111111111111111111_u64) == 0);

	static_assert(countl_zero(0b00000000000000000000000000000000_u32) == 32);
	static_assert(countl_zero(0b00000000000000000000000000000001_u32) == 31);
	static_assert(countl_zero(0b00000000000000000000000000000011_u32) == 30);
	static_assert(countl_zero(0b00000000000000000000000000000111_u32) == 29);
	static_assert(countl_zero(0b00000000000000000000000000001111_u32) == 28);
	static_assert(countl_zero(0b00000000000000000000000000011111_u32) == 27);
	static_assert(countl_zero(0b00000000000000000000000000111111_u32) == 26);
	static_assert(countl_zero(0b00000000000000000000000001111111_u32) == 25);
	static_assert(countl_zero(0b00000000000000000000000011111111_u32) == 24);
	static_assert(countl_zero(0b00000000000000000000000111111111_u32) == 23);
	static_assert(countl_zero(0b00000000000000000000001111111111_u32) == 22);
	static_assert(countl_zero(0b00000000000000000000011111111111_u32) == 21);
	static_assert(countl_zero(0b00000000000000000000111111111111_u32) == 20);
	static_assert(countl_zero(0b00000000000000000001111111111111_u32) == 19);
	static_assert(countl_zero(0b00000000000000000011111111111111_u32) == 18);
	static_assert(countl_zero(0b00000000000000000111111111111111_u32) == 17);
	static_assert(countl_zero(0b00000000000000001111111111111111_u32) == 16);
	static_assert(countl_zero(0b00000000000000011111111111111111_u32) == 15);
	static_assert(countl_zero(0b00000000000000111111111111111111_u32) == 14);
	static_assert(countl_zero(0b00000000000001111111111111111111_u32) == 13);
	static_assert(countl_zero(0b00000000000011111111111111111111_u32) == 12);
	static_assert(countl_zero(0b00000000000111111111111111111111_u32) == 11);
	static_assert(countl_zero(0b00000000001111111111111111111111_u32) == 10);
	static_assert(countl_zero(0b00000000011111111111111111111111_u32) == 9);
	static_assert(countl_zero(0b00000000111111111111111111111111_u32) == 8);
	static_assert(countl_zero(0b00000001111111111111111111111111_u32) == 7);
	static_assert(countl_zero(0b00000011111111111111111111111111_u32) == 6);
	static_assert(countl_zero(0b00000111111111111111111111111111_u32) == 5);
	static_assert(countl_zero(0b00001111111111111111111111111111_u32) == 4);
	static_assert(countl_zero(0b00011111111111111111111111111111_u32) == 3);
	static_assert(countl_zero(0b00111111111111111111111111111111_u32) == 2);
	static_assert(countl_zero(0b01111111111111111111111111111111_u32) == 1);
	static_assert(countl_zero(0b11111111111111111111111111111111_u32) == 0);

	CHECK(countl_zero(0b00000000000000000000000000000000_u32) == 32);
	CHECK(countl_zero(0b00000000000000000000000000000001_u32) == 31);
	CHECK(countl_zero(0b00000000000000000000000000000011_u32) == 30);
	CHECK(countl_zero(0b00000000000000000000000000000111_u32) == 29);
	CHECK(countl_zero(0b00000000000000000000000000001111_u32) == 28);
	CHECK(countl_zero(0b00000000000000000000000000011111_u32) == 27);
	CHECK(countl_zero(0b00000000000000000000000000111111_u32) == 26);
	CHECK(countl_zero(0b00000000000000000000000001111111_u32) == 25);
	CHECK(countl_zero(0b00000000000000000000000011111111_u32) == 24);
	CHECK(countl_zero(0b00000000000000000000000111111111_u32) == 23);
	CHECK(countl_zero(0b00000000000000000000001111111111_u32) == 22);
	CHECK(countl_zero(0b00000000000000000000011111111111_u32) == 21);
	CHECK(countl_zero(0b00000000000000000000111111111111_u32) == 20);
	CHECK(countl_zero(0b00000000000000000001111111111111_u32) == 19);
	CHECK(countl_zero(0b00000000000000000011111111111111_u32) == 18);
	CHECK(countl_zero(0b00000000000000000111111111111111_u32) == 17);
	CHECK(countl_zero(0b00000000000000001111111111111111_u32) == 16);
	CHECK(countl_zero(0b00000000000000011111111111111111_u32) == 15);
	CHECK(countl_zero(0b00000000000000111111111111111111_u32) == 14);
	CHECK(countl_zero(0b00000000000001111111111111111111_u32) == 13);
	CHECK(countl_zero(0b00000000000011111111111111111111_u32) == 12);
	CHECK(countl_zero(0b00000000000111111111111111111111_u32) == 11);
	CHECK(countl_zero(0b00000000001111111111111111111111_u32) == 10);
	CHECK(countl_zero(0b00000000011111111111111111111111_u32) == 9);
	CHECK(countl_zero(0b00000000111111111111111111111111_u32) == 8);
	CHECK(countl_zero(0b00000001111111111111111111111111_u32) == 7);
	CHECK(countl_zero(0b00000011111111111111111111111111_u32) == 6);
	CHECK(countl_zero(0b00000111111111111111111111111111_u32) == 5);
	CHECK(countl_zero(0b00001111111111111111111111111111_u32) == 4);
	CHECK(countl_zero(0b00011111111111111111111111111111_u32) == 3);
	CHECK(countl_zero(0b00111111111111111111111111111111_u32) == 2);
	CHECK(countl_zero(0b01111111111111111111111111111111_u32) == 1);
	CHECK(countl_zero(0b11111111111111111111111111111111_u32) == 0);

	static_assert(countl_zero(0b0000000000000000_u16) == 16);
	static_assert(countl_zero(0b0000000000000001_u16) == 15);
	static_assert(countl_zero(0b0000000000000011_u16) == 14);
	static_assert(countl_zero(0b0000000000000111_u16) == 13);
	static_assert(countl_zero(0b0000000000001111_u16) == 12);
	static_assert(countl_zero(0b0000000000011111_u16) == 11);
	static_assert(countl_zero(0b0000000000111111_u16) == 10);
	static_assert(countl_zero(0b0000000001111111_u16) == 9);
	static_assert(countl_zero(0b0000000011111111_u16) == 8);
	static_assert(countl_zero(0b0000000111111111_u16) == 7);
	static_assert(countl_zero(0b0000001111111111_u16) == 6);
	static_assert(countl_zero(0b0000011111111111_u16) == 5);
	static_assert(countl_zero(0b0000111111111111_u16) == 4);
	static_assert(countl_zero(0b0001111111111111_u16) == 3);
	static_assert(countl_zero(0b0011111111111111_u16) == 2);
	static_assert(countl_zero(0b0111111111111111_u16) == 1);
	static_assert(countl_zero(0b1111111111111111_u16) == 0);

	CHECK(countl_zero(0b0000000000000000_u16) == 16);
	CHECK(countl_zero(0b0000000000000001_u16) == 15);
	CHECK(countl_zero(0b0000000000000011_u16) == 14);
	CHECK(countl_zero(0b0000000000000111_u16) == 13);
	CHECK(countl_zero(0b0000000000001111_u16) == 12);
	CHECK(countl_zero(0b0000000000011111_u16) == 11);
	CHECK(countl_zero(0b0000000000111111_u16) == 10);
	CHECK(countl_zero(0b0000000001111111_u16) == 9);
	CHECK(countl_zero(0b0000000011111111_u16) == 8);
	CHECK(countl_zero(0b0000000111111111_u16) == 7);
	CHECK(countl_zero(0b0000001111111111_u16) == 6);
	CHECK(countl_zero(0b0000011111111111_u16) == 5);
	CHECK(countl_zero(0b0000111111111111_u16) == 4);
	CHECK(countl_zero(0b0001111111111111_u16) == 3);
	CHECK(countl_zero(0b0011111111111111_u16) == 2);
	CHECK(countl_zero(0b0111111111111111_u16) == 1);
	CHECK(countl_zero(0b1111111111111111_u16) == 0);

	static_assert(countl_zero(0b00000000_u8) == 8);
	static_assert(countl_zero(0b00000001_u8) == 7);
	static_assert(countl_zero(0b00000011_u8) == 6);
	static_assert(countl_zero(0b00000111_u8) == 5);
	static_assert(countl_zero(0b00001111_u8) == 4);
	static_assert(countl_zero(0b00011111_u8) == 3);
	static_assert(countl_zero(0b00111111_u8) == 2);
	static_assert(countl_zero(0b01111111_u8) == 1);
	static_assert(countl_zero(0b11111111_u8) == 0);

	CHECK(countl_zero(0b00000000_u8) == 8);
	CHECK(countl_zero(0b00000001_u8) == 7);
	CHECK(countl_zero(0b00000011_u8) == 6);
	CHECK(countl_zero(0b00000111_u8) == 5);
	CHECK(countl_zero(0b00001111_u8) == 4);
	CHECK(countl_zero(0b00011111_u8) == 3);
	CHECK(countl_zero(0b00111111_u8) == 2);
	CHECK(countl_zero(0b01111111_u8) == 1);
	CHECK(countl_zero(0b11111111_u8) == 0);
}

TEST_CASE("countr_zero")
{
	static_assert(countr_zero(0b0000000000000000000000000000000000000000000000000000000000000000_u64) == 64);
	static_assert(countr_zero(0b1000000000000000000000000000000000000000000000000000000000000000_u64) == 63);
	static_assert(countr_zero(0b1100000000000000000000000000000000000000000000000000000000000000_u64) == 62);
	static_assert(countr_zero(0b1110000000000000000000000000000000000000000000000000000000000000_u64) == 61);
	static_assert(countr_zero(0b1111000000000000000000000000000000000000000000000000000000000000_u64) == 60);
	static_assert(countr_zero(0b1111100000000000000000000000000000000000000000000000000000000000_u64) == 59);
	static_assert(countr_zero(0b1111110000000000000000000000000000000000000000000000000000000000_u64) == 58);
	static_assert(countr_zero(0b1111111000000000000000000000000000000000000000000000000000000000_u64) == 57);
	static_assert(countr_zero(0b1111111100000000000000000000000000000000000000000000000000000000_u64) == 56);
	static_assert(countr_zero(0b1111111110000000000000000000000000000000000000000000000000000000_u64) == 55);
	static_assert(countr_zero(0b1111111111000000000000000000000000000000000000000000000000000000_u64) == 54);
	static_assert(countr_zero(0b1111111111100000000000000000000000000000000000000000000000000000_u64) == 53);
	static_assert(countr_zero(0b1111111111110000000000000000000000000000000000000000000000000000_u64) == 52);
	static_assert(countr_zero(0b1111111111111000000000000000000000000000000000000000000000000000_u64) == 51);
	static_assert(countr_zero(0b1111111111111100000000000000000000000000000000000000000000000000_u64) == 50);
	static_assert(countr_zero(0b1111111111111110000000000000000000000000000000000000000000000000_u64) == 49);
	static_assert(countr_zero(0b1111111111111111000000000000000000000000000000000000000000000000_u64) == 48);
	static_assert(countr_zero(0b1111111111111111100000000000000000000000000000000000000000000000_u64) == 47);
	static_assert(countr_zero(0b1111111111111111110000000000000000000000000000000000000000000000_u64) == 46);
	static_assert(countr_zero(0b1111111111111111111000000000000000000000000000000000000000000000_u64) == 45);
	static_assert(countr_zero(0b1111111111111111111100000000000000000000000000000000000000000000_u64) == 44);
	static_assert(countr_zero(0b1111111111111111111110000000000000000000000000000000000000000000_u64) == 43);
	static_assert(countr_zero(0b1111111111111111111111000000000000000000000000000000000000000000_u64) == 42);
	static_assert(countr_zero(0b1111111111111111111111100000000000000000000000000000000000000000_u64) == 41);
	static_assert(countr_zero(0b1111111111111111111111110000000000000000000000000000000000000000_u64) == 40);
	static_assert(countr_zero(0b1111111111111111111111111000000000000000000000000000000000000000_u64) == 39);
	static_assert(countr_zero(0b1111111111111111111111111100000000000000000000000000000000000000_u64) == 38);
	static_assert(countr_zero(0b1111111111111111111111111110000000000000000000000000000000000000_u64) == 37);
	static_assert(countr_zero(0b1111111111111111111111111111000000000000000000000000000000000000_u64) == 36);
	static_assert(countr_zero(0b1111111111111111111111111111100000000000000000000000000000000000_u64) == 35);
	static_assert(countr_zero(0b1111111111111111111111111111110000000000000000000000000000000000_u64) == 34);
	static_assert(countr_zero(0b1111111111111111111111111111111000000000000000000000000000000000_u64) == 33);
	static_assert(countr_zero(0b1111111111111111111111111111111100000000000000000000000000000000_u64) == 32);
	static_assert(countr_zero(0b1111111111111111111111111111111110000000000000000000000000000000_u64) == 31);
	static_assert(countr_zero(0b1111111111111111111111111111111111000000000000000000000000000000_u64) == 30);
	static_assert(countr_zero(0b1111111111111111111111111111111111100000000000000000000000000000_u64) == 29);
	static_assert(countr_zero(0b1111111111111111111111111111111111110000000000000000000000000000_u64) == 28);
	static_assert(countr_zero(0b1111111111111111111111111111111111111000000000000000000000000000_u64) == 27);
	static_assert(countr_zero(0b1111111111111111111111111111111111111100000000000000000000000000_u64) == 26);
	static_assert(countr_zero(0b1111111111111111111111111111111111111110000000000000000000000000_u64) == 25);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111000000000000000000000000_u64) == 24);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111100000000000000000000000_u64) == 23);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111110000000000000000000000_u64) == 22);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111000000000000000000000_u64) == 21);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111100000000000000000000_u64) == 20);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111110000000000000000000_u64) == 19);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111111000000000000000000_u64) == 18);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111111100000000000000000_u64) == 17);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111111110000000000000000_u64) == 16);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111111111000000000000000_u64) == 15);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111111111100000000000000_u64) == 14);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111111111110000000000000_u64) == 13);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111111111111000000000000_u64) == 12);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111111111111100000000000_u64) == 11);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111111111111110000000000_u64) == 10);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111111111111111000000000_u64) == 9);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111111111111111100000000_u64) == 8);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111111111111111110000000_u64) == 7);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111111111111111111000000_u64) == 6);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111111111111111111100000_u64) == 5);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111111111111111111110000_u64) == 4);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111111111111111111111000_u64) == 3);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111111111111111111111100_u64) == 2);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111111111111111111111110_u64) == 1);
	static_assert(countr_zero(0b1111111111111111111111111111111111111111111111111111111111111111_u64) == 0);

	CHECK(countr_zero(0b0000000000000000000000000000000000000000000000000000000000000000_u64) == 64);
	CHECK(countr_zero(0b1000000000000000000000000000000000000000000000000000000000000000_u64) == 63);
	CHECK(countr_zero(0b1100000000000000000000000000000000000000000000000000000000000000_u64) == 62);
	CHECK(countr_zero(0b1110000000000000000000000000000000000000000000000000000000000000_u64) == 61);
	CHECK(countr_zero(0b1111000000000000000000000000000000000000000000000000000000000000_u64) == 60);
	CHECK(countr_zero(0b1111100000000000000000000000000000000000000000000000000000000000_u64) == 59);
	CHECK(countr_zero(0b1111110000000000000000000000000000000000000000000000000000000000_u64) == 58);
	CHECK(countr_zero(0b1111111000000000000000000000000000000000000000000000000000000000_u64) == 57);
	CHECK(countr_zero(0b1111111100000000000000000000000000000000000000000000000000000000_u64) == 56);
	CHECK(countr_zero(0b1111111110000000000000000000000000000000000000000000000000000000_u64) == 55);
	CHECK(countr_zero(0b1111111111000000000000000000000000000000000000000000000000000000_u64) == 54);
	CHECK(countr_zero(0b1111111111100000000000000000000000000000000000000000000000000000_u64) == 53);
	CHECK(countr_zero(0b1111111111110000000000000000000000000000000000000000000000000000_u64) == 52);
	CHECK(countr_zero(0b1111111111111000000000000000000000000000000000000000000000000000_u64) == 51);
	CHECK(countr_zero(0b1111111111111100000000000000000000000000000000000000000000000000_u64) == 50);
	CHECK(countr_zero(0b1111111111111110000000000000000000000000000000000000000000000000_u64) == 49);
	CHECK(countr_zero(0b1111111111111111000000000000000000000000000000000000000000000000_u64) == 48);
	CHECK(countr_zero(0b1111111111111111100000000000000000000000000000000000000000000000_u64) == 47);
	CHECK(countr_zero(0b1111111111111111110000000000000000000000000000000000000000000000_u64) == 46);
	CHECK(countr_zero(0b1111111111111111111000000000000000000000000000000000000000000000_u64) == 45);
	CHECK(countr_zero(0b1111111111111111111100000000000000000000000000000000000000000000_u64) == 44);
	CHECK(countr_zero(0b1111111111111111111110000000000000000000000000000000000000000000_u64) == 43);
	CHECK(countr_zero(0b1111111111111111111111000000000000000000000000000000000000000000_u64) == 42);
	CHECK(countr_zero(0b1111111111111111111111100000000000000000000000000000000000000000_u64) == 41);
	CHECK(countr_zero(0b1111111111111111111111110000000000000000000000000000000000000000_u64) == 40);
	CHECK(countr_zero(0b1111111111111111111111111000000000000000000000000000000000000000_u64) == 39);
	CHECK(countr_zero(0b1111111111111111111111111100000000000000000000000000000000000000_u64) == 38);
	CHECK(countr_zero(0b1111111111111111111111111110000000000000000000000000000000000000_u64) == 37);
	CHECK(countr_zero(0b1111111111111111111111111111000000000000000000000000000000000000_u64) == 36);
	CHECK(countr_zero(0b1111111111111111111111111111100000000000000000000000000000000000_u64) == 35);
	CHECK(countr_zero(0b1111111111111111111111111111110000000000000000000000000000000000_u64) == 34);
	CHECK(countr_zero(0b1111111111111111111111111111111000000000000000000000000000000000_u64) == 33);
	CHECK(countr_zero(0b1111111111111111111111111111111100000000000000000000000000000000_u64) == 32);
	CHECK(countr_zero(0b1111111111111111111111111111111110000000000000000000000000000000_u64) == 31);
	CHECK(countr_zero(0b1111111111111111111111111111111111000000000000000000000000000000_u64) == 30);
	CHECK(countr_zero(0b1111111111111111111111111111111111100000000000000000000000000000_u64) == 29);
	CHECK(countr_zero(0b1111111111111111111111111111111111110000000000000000000000000000_u64) == 28);
	CHECK(countr_zero(0b1111111111111111111111111111111111111000000000000000000000000000_u64) == 27);
	CHECK(countr_zero(0b1111111111111111111111111111111111111100000000000000000000000000_u64) == 26);
	CHECK(countr_zero(0b1111111111111111111111111111111111111110000000000000000000000000_u64) == 25);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111000000000000000000000000_u64) == 24);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111100000000000000000000000_u64) == 23);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111110000000000000000000000_u64) == 22);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111000000000000000000000_u64) == 21);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111100000000000000000000_u64) == 20);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111110000000000000000000_u64) == 19);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111111000000000000000000_u64) == 18);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111111100000000000000000_u64) == 17);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111111110000000000000000_u64) == 16);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111111111000000000000000_u64) == 15);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111111111100000000000000_u64) == 14);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111111111110000000000000_u64) == 13);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111111111111000000000000_u64) == 12);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111111111111100000000000_u64) == 11);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111111111111110000000000_u64) == 10);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111111111111111000000000_u64) == 9);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111111111111111100000000_u64) == 8);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111111111111111110000000_u64) == 7);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111111111111111111000000_u64) == 6);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111111111111111111100000_u64) == 5);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111111111111111111110000_u64) == 4);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111111111111111111111000_u64) == 3);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111111111111111111111100_u64) == 2);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111111111111111111111110_u64) == 1);
	CHECK(countr_zero(0b1111111111111111111111111111111111111111111111111111111111111111_u64) == 0);

	static_assert(countr_zero(0b00000000000000000000000000000000_u32) == 32);
	static_assert(countr_zero(0b10000000000000000000000000000000_u32) == 31);
	static_assert(countr_zero(0b11000000000000000000000000000000_u32) == 30);
	static_assert(countr_zero(0b11100000000000000000000000000000_u32) == 29);
	static_assert(countr_zero(0b11110000000000000000000000000000_u32) == 28);
	static_assert(countr_zero(0b11111000000000000000000000000000_u32) == 27);
	static_assert(countr_zero(0b11111100000000000000000000000000_u32) == 26);
	static_assert(countr_zero(0b11111110000000000000000000000000_u32) == 25);
	static_assert(countr_zero(0b11111111000000000000000000000000_u32) == 24);
	static_assert(countr_zero(0b11111111100000000000000000000000_u32) == 23);
	static_assert(countr_zero(0b11111111110000000000000000000000_u32) == 22);
	static_assert(countr_zero(0b11111111111000000000000000000000_u32) == 21);
	static_assert(countr_zero(0b11111111111100000000000000000000_u32) == 20);
	static_assert(countr_zero(0b11111111111110000000000000000000_u32) == 19);
	static_assert(countr_zero(0b11111111111111000000000000000000_u32) == 18);
	static_assert(countr_zero(0b11111111111111100000000000000000_u32) == 17);
	static_assert(countr_zero(0b11111111111111110000000000000000_u32) == 16);
	static_assert(countr_zero(0b11111111111111111000000000000000_u32) == 15);
	static_assert(countr_zero(0b11111111111111111100000000000000_u32) == 14);
	static_assert(countr_zero(0b11111111111111111110000000000000_u32) == 13);
	static_assert(countr_zero(0b11111111111111111111000000000000_u32) == 12);
	static_assert(countr_zero(0b11111111111111111111100000000000_u32) == 11);
	static_assert(countr_zero(0b11111111111111111111110000000000_u32) == 10);
	static_assert(countr_zero(0b11111111111111111111111000000000_u32) == 9);
	static_assert(countr_zero(0b11111111111111111111111100000000_u32) == 8);
	static_assert(countr_zero(0b11111111111111111111111110000000_u32) == 7);
	static_assert(countr_zero(0b11111111111111111111111111000000_u32) == 6);
	static_assert(countr_zero(0b11111111111111111111111111100000_u32) == 5);
	static_assert(countr_zero(0b11111111111111111111111111110000_u32) == 4);
	static_assert(countr_zero(0b11111111111111111111111111111000_u32) == 3);
	static_assert(countr_zero(0b11111111111111111111111111111100_u32) == 2);
	static_assert(countr_zero(0b11111111111111111111111111111110_u32) == 1);
	static_assert(countr_zero(0b11111111111111111111111111111111_u32) == 0);

	CHECK(countr_zero(0b00000000000000000000000000000000_u32) == 32);
	CHECK(countr_zero(0b10000000000000000000000000000000_u32) == 31);
	CHECK(countr_zero(0b11000000000000000000000000000000_u32) == 30);
	CHECK(countr_zero(0b11100000000000000000000000000000_u32) == 29);
	CHECK(countr_zero(0b11110000000000000000000000000000_u32) == 28);
	CHECK(countr_zero(0b11111000000000000000000000000000_u32) == 27);
	CHECK(countr_zero(0b11111100000000000000000000000000_u32) == 26);
	CHECK(countr_zero(0b11111110000000000000000000000000_u32) == 25);
	CHECK(countr_zero(0b11111111000000000000000000000000_u32) == 24);
	CHECK(countr_zero(0b11111111100000000000000000000000_u32) == 23);
	CHECK(countr_zero(0b11111111110000000000000000000000_u32) == 22);
	CHECK(countr_zero(0b11111111111000000000000000000000_u32) == 21);
	CHECK(countr_zero(0b11111111111100000000000000000000_u32) == 20);
	CHECK(countr_zero(0b11111111111110000000000000000000_u32) == 19);
	CHECK(countr_zero(0b11111111111111000000000000000000_u32) == 18);
	CHECK(countr_zero(0b11111111111111100000000000000000_u32) == 17);
	CHECK(countr_zero(0b11111111111111110000000000000000_u32) == 16);
	CHECK(countr_zero(0b11111111111111111000000000000000_u32) == 15);
	CHECK(countr_zero(0b11111111111111111100000000000000_u32) == 14);
	CHECK(countr_zero(0b11111111111111111110000000000000_u32) == 13);
	CHECK(countr_zero(0b11111111111111111111000000000000_u32) == 12);
	CHECK(countr_zero(0b11111111111111111111100000000000_u32) == 11);
	CHECK(countr_zero(0b11111111111111111111110000000000_u32) == 10);
	CHECK(countr_zero(0b11111111111111111111111000000000_u32) == 9);
	CHECK(countr_zero(0b11111111111111111111111100000000_u32) == 8);
	CHECK(countr_zero(0b11111111111111111111111110000000_u32) == 7);
	CHECK(countr_zero(0b11111111111111111111111111000000_u32) == 6);
	CHECK(countr_zero(0b11111111111111111111111111100000_u32) == 5);
	CHECK(countr_zero(0b11111111111111111111111111110000_u32) == 4);
	CHECK(countr_zero(0b11111111111111111111111111111000_u32) == 3);
	CHECK(countr_zero(0b11111111111111111111111111111100_u32) == 2);
	CHECK(countr_zero(0b11111111111111111111111111111110_u32) == 1);
	CHECK(countr_zero(0b11111111111111111111111111111111_u32) == 0);

	static_assert(countr_zero(0b0000000000000000_u16) == 16);
	static_assert(countr_zero(0b1000000000000000_u16) == 15);
	static_assert(countr_zero(0b1100000000000000_u16) == 14);
	static_assert(countr_zero(0b1110000000000000_u16) == 13);
	static_assert(countr_zero(0b1111000000000000_u16) == 12);
	static_assert(countr_zero(0b1111100000000000_u16) == 11);
	static_assert(countr_zero(0b1111110000000000_u16) == 10);
	static_assert(countr_zero(0b1111111000000000_u16) == 9);
	static_assert(countr_zero(0b1111111100000000_u16) == 8);
	static_assert(countr_zero(0b1111111110000000_u16) == 7);
	static_assert(countr_zero(0b1111111111000000_u16) == 6);
	static_assert(countr_zero(0b1111111111100000_u16) == 5);
	static_assert(countr_zero(0b1111111111110000_u16) == 4);
	static_assert(countr_zero(0b1111111111111000_u16) == 3);
	static_assert(countr_zero(0b1111111111111100_u16) == 2);
	static_assert(countr_zero(0b1111111111111110_u16) == 1);
	static_assert(countr_zero(0b1111111111111111_u16) == 0);

	CHECK(countr_zero(0b0000000000000000_u16) == 16);
	CHECK(countr_zero(0b1000000000000000_u16) == 15);
	CHECK(countr_zero(0b1100000000000000_u16) == 14);
	CHECK(countr_zero(0b1110000000000000_u16) == 13);
	CHECK(countr_zero(0b1111000000000000_u16) == 12);
	CHECK(countr_zero(0b1111100000000000_u16) == 11);
	CHECK(countr_zero(0b1111110000000000_u16) == 10);
	CHECK(countr_zero(0b1111111000000000_u16) == 9);
	CHECK(countr_zero(0b1111111100000000_u16) == 8);
	CHECK(countr_zero(0b1111111110000000_u16) == 7);
	CHECK(countr_zero(0b1111111111000000_u16) == 6);
	CHECK(countr_zero(0b1111111111100000_u16) == 5);
	CHECK(countr_zero(0b1111111111110000_u16) == 4);
	CHECK(countr_zero(0b1111111111111000_u16) == 3);
	CHECK(countr_zero(0b1111111111111100_u16) == 2);
	CHECK(countr_zero(0b1111111111111110_u16) == 1);
	CHECK(countr_zero(0b1111111111111111_u16) == 0);

	static_assert(countr_zero(0b00000000_u8) == 8);
	static_assert(countr_zero(0b10000000_u8) == 7);
	static_assert(countr_zero(0b11000000_u8) == 6);
	static_assert(countr_zero(0b11100000_u8) == 5);
	static_assert(countr_zero(0b11110000_u8) == 4);
	static_assert(countr_zero(0b11111000_u8) == 3);
	static_assert(countr_zero(0b11111100_u8) == 2);
	static_assert(countr_zero(0b11111110_u8) == 1);
	static_assert(countr_zero(0b11111111_u8) == 0);

	CHECK(countr_zero(0b00000000_u8) == 8);
	CHECK(countr_zero(0b10000000_u8) == 7);
	CHECK(countr_zero(0b11000000_u8) == 6);
	CHECK(countr_zero(0b11100000_u8) == 5);
	CHECK(countr_zero(0b11110000_u8) == 4);
	CHECK(countr_zero(0b11111000_u8) == 3);
	CHECK(countr_zero(0b11111100_u8) == 2);
	CHECK(countr_zero(0b11111110_u8) == 1);
	CHECK(countr_zero(0b11111111_u8) == 0);
}

TEST_CASE("bit_ceil")
{
	static_assert(bit_ceil(1u) == 1u);
	static_assert(bit_ceil(2u) == 2u);
	static_assert(bit_ceil(3u) == 4u);
	static_assert(bit_ceil(4u) == 4u);
	static_assert(bit_ceil(5u) == 8u);
	static_assert(bit_ceil(6u) == 8u);
	static_assert(bit_ceil(7u) == 8u);
	static_assert(bit_ceil(8u) == 8u);
	static_assert(bit_ceil(9u) == 16u);
	static_assert(bit_ceil(10u) == 16u);

	CHECK(bit_ceil(1u) == 1u);
	CHECK(bit_ceil(2u) == 2u);
	CHECK(bit_ceil(3u) == 4u);
	CHECK(bit_ceil(4u) == 4u);
	CHECK(bit_ceil(5u) == 8u);
	CHECK(bit_ceil(6u) == 8u);
	CHECK(bit_ceil(7u) == 8u);
	CHECK(bit_ceil(8u) == 8u);
	CHECK(bit_ceil(9u) == 16u);
	CHECK(bit_ceil(10u) == 16u);
}

TEST_CASE("bit_floor")
{
	static_assert(bit_floor(1u) == 1u);
	static_assert(bit_floor(2u) == 2u);
	static_assert(bit_floor(3u) == 2u);
	static_assert(bit_floor(4u) == 4u);
	static_assert(bit_floor(5u) == 4u);
	static_assert(bit_floor(6u) == 4u);
	static_assert(bit_floor(7u) == 4u);
	static_assert(bit_floor(8u) == 8u);
	static_assert(bit_floor(9u) == 8u);
	static_assert(bit_floor(10u) == 8u);

	CHECK(bit_floor(1u) == 1u);
	CHECK(bit_floor(2u) == 2u);
	CHECK(bit_floor(3u) == 2u);
	CHECK(bit_floor(4u) == 4u);
	CHECK(bit_floor(5u) == 4u);
	CHECK(bit_floor(6u) == 4u);
	CHECK(bit_floor(7u) == 4u);
	CHECK(bit_floor(8u) == 8u);
	CHECK(bit_floor(9u) == 8u);
	CHECK(bit_floor(10u) == 8u);
}

TEST_CASE("bit_width")
{
	static_assert(bit_width(0b0000000000000000000000000000000000000000000000000000000000000000_u64) == 0);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000000000000000000000001_u64) == 1);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000000000000000000000011_u64) == 2);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000000000000000000000111_u64) == 3);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000000000000000000001111_u64) == 4);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000000000000000000011111_u64) == 5);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000000000000000000111111_u64) == 6);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000000000000000001111111_u64) == 7);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000000000000000011111111_u64) == 8);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000000000000000111111111_u64) == 9);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000000000000001111110111_u64) == 10);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000000000000011111111111_u64) == 11);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000000000000111111111111_u64) == 12);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000000000001111111111111_u64) == 13);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000000000011111111101111_u64) == 14);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000000000111111111111111_u64) == 15);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000000001111111111111111_u64) == 16);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000000011111110111110111_u64) == 17);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000000111111111111111111_u64) == 18);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000001111101111111110111_u64) == 19);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000011111111111101110111_u64) == 20);
	static_assert(bit_width(0b0000000000000000000000000000000000000000000111111111111111111111_u64) == 21);
	static_assert(bit_width(0b0000000000000000000000000000000000000000001111101111111111111111_u64) == 22);
	static_assert(bit_width(0b0000000000000000000000000000000000000000011111111110111011111111_u64) == 23);
	static_assert(bit_width(0b0000000000000000000000000000000000000000111111111111111111111111_u64) == 24);
	static_assert(bit_width(0b0000000000000000000000000000000000000001111111111111111111011111_u64) == 25);
	static_assert(bit_width(0b0000000000000000000000000000000000000011111111111111111111111111_u64) == 26);
	static_assert(bit_width(0b0000000000000000000000000000000000000111101111110111111111111111_u64) == 27);
	static_assert(bit_width(0b0000000000000000000000000000000000001111111111111111111101111111_u64) == 28);
	static_assert(bit_width(0b0000000000000000000000000000000000011111111111111110001110111111_u64) == 29);
	static_assert(bit_width(0b0000000000000000000000000000000000111111111111011110001111011111_u64) == 30);
	static_assert(bit_width(0b0000000000000000000000000000000001111111111111111111111111111111_u64) == 31);
	static_assert(bit_width(0b0000000000000000000000000000000011111000111111111110111111111111_u64) == 32);
	static_assert(bit_width(0b0000000000000000000000000000000111111111101111111111111111111111_u64) == 33);
	static_assert(bit_width(0b0000000000000000000000000000001111111111111111111011111111111111_u64) == 34);
	static_assert(bit_width(0b0000000000000000000000000000011111111111111111111110001101011111_u64) == 35);
	static_assert(bit_width(0b0000000000000000000000000000111111111000011111111111111111111111_u64) == 36);
	static_assert(bit_width(0b0000000000000000000000000001111111111111111110111111111111111111_u64) == 37);
	static_assert(bit_width(0b0000000000000000000000000011111111111111111111111111011111111111_u64) == 38);
	static_assert(bit_width(0b0000000000000000000000000111111110001111100111111111111101111111_u64) == 39);
	static_assert(bit_width(0b0000000000000000000000001111111111111111111111111111111111111111_u64) == 40);
	static_assert(bit_width(0b0000000000000000000000011111111111111111111110111111111111111111_u64) == 41);
	static_assert(bit_width(0b0000000000000000000000111111111111101111111111111011111111111111_u64) == 42);
	static_assert(bit_width(0b0000000000000000000001111111111111110111111111111111100011111111_u64) == 43);
	static_assert(bit_width(0b0000000000000000000011111111100010111111111111111111111111111111_u64) == 44);
	static_assert(bit_width(0b0000000000000000000111111111111111111000111111111100111111111111_u64) == 45);
	static_assert(bit_width(0b0000000000000000001111111111111111110111111111111111111011111111_u64) == 46);
	static_assert(bit_width(0b0000000000000000011111100001111111111111111111111101111111111111_u64) == 47);
	static_assert(bit_width(0b0000000000000000111111111111111111011111111111111111111111111111_u64) == 48);
	static_assert(bit_width(0b0000000000000001111111111111111111111111111111110011000111111111_u64) == 49);
	static_assert(bit_width(0b0000000000000011111110001111110111111111111111111111111111111111_u64) == 50);
	static_assert(bit_width(0b0000000000000111111111111111101111111111110111111111111111111111_u64) == 51);
	static_assert(bit_width(0b0000000000001111111110111111111111111111111001111111111111111111_u64) == 52);
	static_assert(bit_width(0b0000000000011111111111111111111100111111111111111111111111111111_u64) == 53);
	static_assert(bit_width(0b0000000000111111111110001111111111111111111111111111111111111111_u64) == 54);
	static_assert(bit_width(0b0000000001111111011111101111111111111111111111111111110001111111_u64) == 55);
	static_assert(bit_width(0b0000000011111111111111111100111111111111111101111111111111111111_u64) == 56);
	static_assert(bit_width(0b0000000111111111111111111111111111111101111111111111111111111111_u64) == 57);
	static_assert(bit_width(0b0000001111111000110001111111111111100011111111111111111111111111_u64) == 58);
	static_assert(bit_width(0b0000011111111111111111111111111011111111011111111111111111111111_u64) == 59);
	static_assert(bit_width(0b0000111111111000111111111111111111111111111111111111111111111111_u64) == 60);
	static_assert(bit_width(0b0001111111111111111111111111111100111111000111100011111111111111_u64) == 61);
	static_assert(bit_width(0b0011111111111111110011111111111111111111111111111111111111111111_u64) == 62);
	static_assert(bit_width(0b0111111111111111111111111111111111111111111111111111111111111111_u64) == 63);
	static_assert(bit_width(0b1111111000111111111111111111111111111111000111111111111111111111_u64) == 64);

	CHECK(bit_width(0b0000000000000000000000000000000000000000000000000000000000000000_u64) == 0);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000000000000000000000001_u64) == 1);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000000000000000000000011_u64) == 2);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000000000000000000000111_u64) == 3);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000000000000000000001111_u64) == 4);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000000000000000000011111_u64) == 5);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000000000000000000111011_u64) == 6);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000000000000000001111111_u64) == 7);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000000000000000011110111_u64) == 8);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000000000000000111111111_u64) == 9);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000000000000001110101111_u64) == 10);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000000000000011111111111_u64) == 11);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000000000000111011111111_u64) == 12);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000000000001111111111111_u64) == 13);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000000000011111110011111_u64) == 14);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000000000111111011111111_u64) == 15);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000000001111101111111111_u64) == 16);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000000011111111001111111_u64) == 17);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000000111101110111101111_u64) == 18);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000001111111111111111111_u64) == 19);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000011111111111010001111_u64) == 20);
	CHECK(bit_width(0b0000000000000000000000000000000000000000000111111111111101111111_u64) == 21);
	CHECK(bit_width(0b0000000000000000000000000000000000000000001111111101111110001111_u64) == 22);
	CHECK(bit_width(0b0000000000000000000000000000000000000000011111111111111001111111_u64) == 23);
	CHECK(bit_width(0b0000000000000000000000000000000000000000111111111111111111111111_u64) == 24);
	CHECK(bit_width(0b0000000000000000000000000000000000000001111111011111111111111111_u64) == 25);
	CHECK(bit_width(0b0000000000000000000000000000000000000011111111110111111111111111_u64) == 26);
	CHECK(bit_width(0b0000000000000000000000000000000000000111111101111111111111111111_u64) == 27);
	CHECK(bit_width(0b0000000000000000000000000000000000001111111111110001111101111111_u64) == 28);
	CHECK(bit_width(0b0000000000000000000000000000000000011111111111111111111111111111_u64) == 29);
	CHECK(bit_width(0b0000000000000000000000000000000000111111101111111011111111111111_u64) == 30);
	CHECK(bit_width(0b0000000000000000000000000000000001111111110111111111111110111111_u64) == 31);
	CHECK(bit_width(0b0000000000000000000000000000000011111111111111111100111111111111_u64) == 32);
	CHECK(bit_width(0b0000000000000000000000000000000111111111111111111000111111111111_u64) == 33);
	CHECK(bit_width(0b0000000000000000000000000000001111111101111101111111111111111111_u64) == 34);
	CHECK(bit_width(0b0000000000000000000000000000011111111111111111110001001011111111_u64) == 35);
	CHECK(bit_width(0b0000000000000000000000000000111111111111111110111111001111111111_u64) == 36);
	CHECK(bit_width(0b0000000000000000000000000001111111011111110111111111111111111111_u64) == 37);
	CHECK(bit_width(0b0000000000000000000000000011111111111110111001111111111111110111_u64) == 38);
	CHECK(bit_width(0b0000000000000000000000000111110111111111111111101111111111111111_u64) == 39);
	CHECK(bit_width(0b0000000000000000000000001111111111111111111111111111111111111111_u64) == 40);
	CHECK(bit_width(0b0000000000000000000000011111111111011111111111111111111010011111_u64) == 41);
	CHECK(bit_width(0b0000000000000000000000111111111111111111111101111111111111111111_u64) == 42);
	CHECK(bit_width(0b0000000000000000000001111111111111111111111111111111100011111111_u64) == 43);
	CHECK(bit_width(0b0000000000000000000011111111111111110111111110011111111110111111_u64) == 44);
	CHECK(bit_width(0b0000000000000000000111111111111111111111100111111111111111111111_u64) == 45);
	CHECK(bit_width(0b0000000000000000001111111111111111111111111111111111111111111111_u64) == 46);
	CHECK(bit_width(0b0000000000000000011111111101100111111111111111111111111111111111_u64) == 47);
	CHECK(bit_width(0b0000000000000000111111111111111111111111110111111111111111111111_u64) == 48);
	CHECK(bit_width(0b0000000000000001111111111111111101111111111111111111111111111111_u64) == 49);
	CHECK(bit_width(0b0000000000000011111111111111111111111111111101111111001111110111_u64) == 50);
	CHECK(bit_width(0b0000000000000111111101111110111111111111111111111111111111111111_u64) == 51);
	CHECK(bit_width(0b0000000000001111111111111000111111111001111101111111111111111111_u64) == 52);
	CHECK(bit_width(0b0000000000011111111111111111110011111111111111111111111111111111_u64) == 53);
	CHECK(bit_width(0b0000000000111111011111111111111111111111110111111111111111111111_u64) == 54);
	CHECK(bit_width(0b0000000001111111111111111111111111110111111111111111111111111111_u64) == 55);
	CHECK(bit_width(0b0000000011111111111111111011111111111111110001111011111111111111_u64) == 56);
	CHECK(bit_width(0b0000000111111111111111111111111111111111111111011111111111111111_u64) == 57);
	CHECK(bit_width(0b0000001111111111100011111111111111111111111111111111110011111111_u64) == 58);
	CHECK(bit_width(0b0000011111111111111111111111111110111111111111111111111111111111_u64) == 59);
	CHECK(bit_width(0b0000111111111111111111111111111111111111111111111111111011111111_u64) == 60);
	CHECK(bit_width(0b0001111111011111011111111111111111111111100011110011111111111111_u64) == 61);
	CHECK(bit_width(0b0011111111111000111100111111111111011111111111111101111111111111_u64) == 62);
	CHECK(bit_width(0b0111111111111111111111111101000111111111111111111111111111111111_u64) == 63);
	CHECK(bit_width(0b1111111111111111110001111111111111111111111111100011111111111111_u64) == 64);

	static_assert(bit_width(0b00000000000000000000000000000000_u32) == 0);
	static_assert(bit_width(0b00000000000000000000000000000001_u32) == 1);
	static_assert(bit_width(0b00000000000000000000000000000011_u32) == 2);
	static_assert(bit_width(0b00000000000000000000000000000111_u32) == 3);
	static_assert(bit_width(0b00000000000000000000000000001111_u32) == 4);
	static_assert(bit_width(0b00000000000000000000000000011111_u32) == 5);
	static_assert(bit_width(0b00000000000000000000000000111111_u32) == 6);
	static_assert(bit_width(0b00000000000000000000000001111111_u32) == 7);
	static_assert(bit_width(0b00000000000000000000000011111111_u32) == 8);
	static_assert(bit_width(0b00000000000000000000000111001101_u32) == 9);
	static_assert(bit_width(0b00000000000000000000001111111111_u32) == 10);
	static_assert(bit_width(0b00000000000000000000011111111011_u32) == 11);
	static_assert(bit_width(0b00000000000000000000111111001111_u32) == 12);
	static_assert(bit_width(0b00000000000000000001111111111111_u32) == 13);
	static_assert(bit_width(0b00000000000000000011111111111111_u32) == 14);
	static_assert(bit_width(0b00000000000000000111110111011111_u32) == 15);
	static_assert(bit_width(0b00000000000000001111111111111111_u32) == 16);
	static_assert(bit_width(0b00000000000000011111111000111111_u32) == 17);
	static_assert(bit_width(0b00000000000000111111111111111111_u32) == 18);
	static_assert(bit_width(0b00000000000001110111111111011101_u32) == 19);
	static_assert(bit_width(0b00000000000011111111111000111111_u32) == 20);
	static_assert(bit_width(0b00000000000111111111111111111111_u32) == 21);
	static_assert(bit_width(0b00000000001111111011111111111111_u32) == 22);
	static_assert(bit_width(0b00000000011111111111111111011111_u32) == 23);
	static_assert(bit_width(0b00000000111101111111111100111111_u32) == 24);
	static_assert(bit_width(0b00000001111111111111111111111111_u32) == 25);
	static_assert(bit_width(0b00000011101110001111111001111111_u32) == 26);
	static_assert(bit_width(0b00000111111111111111111111011111_u32) == 27);
	static_assert(bit_width(0b00001111111111111111111111111111_u32) == 28);
	static_assert(bit_width(0b00011111111111101111110001111111_u32) == 29);
	static_assert(bit_width(0b00111100011111111111111111111111_u32) == 30);
	static_assert(bit_width(0b01111111110001111111111111111111_u32) == 31);
	static_assert(bit_width(0b11111111111111111110011111111111_u32) == 32);

	CHECK(bit_width(0b00000000000000000000000000000000_u32) == 0);
	CHECK(bit_width(0b00000000000000000000000000000001_u32) == 1);
	CHECK(bit_width(0b00000000000000000000000000000011_u32) == 2);
	CHECK(bit_width(0b00000000000000000000000000000111_u32) == 3);
	CHECK(bit_width(0b00000000000000000000000000001101_u32) == 4);
	CHECK(bit_width(0b00000000000000000000000000011101_u32) == 5);
	CHECK(bit_width(0b00000000000000000000000000111111_u32) == 6);
	CHECK(bit_width(0b00000000000000000000000001111111_u32) == 7);
	CHECK(bit_width(0b00000000000000000000000011111011_u32) == 8);
	CHECK(bit_width(0b00000000000000000000000111110011_u32) == 9);
	CHECK(bit_width(0b00000000000000000000001111111111_u32) == 10);
	CHECK(bit_width(0b00000000000000000000011111111111_u32) == 11);
	CHECK(bit_width(0b00000000000000000000111110110111_u32) == 12);
	CHECK(bit_width(0b00000000000000000001111011111111_u32) == 13);
	CHECK(bit_width(0b00000000000000000011111100111111_u32) == 14);
	CHECK(bit_width(0b00000000000000000111111111111111_u32) == 15);
	CHECK(bit_width(0b00000000000000001111110111111111_u32) == 16);
	CHECK(bit_width(0b00000000000000011111111111100111_u32) == 17);
	CHECK(bit_width(0b00000000000000111110111111111111_u32) == 18);
	CHECK(bit_width(0b00000000000001111111111101111111_u32) == 19);
	CHECK(bit_width(0b00000000000011111111111111111111_u32) == 20);
	CHECK(bit_width(0b00000000000111110110001111111111_u32) == 21);
	CHECK(bit_width(0b00000000001111111111101111011111_u32) == 22);
	CHECK(bit_width(0b00000000011111111111111111100111_u32) == 23);
	CHECK(bit_width(0b00000000111111111111111111111111_u32) == 24);
	CHECK(bit_width(0b00000001111101111111101111111111_u32) == 25);
	CHECK(bit_width(0b00000011111110111111111110111111_u32) == 26);
	CHECK(bit_width(0b00000111111111100111011111111111_u32) == 27);
	CHECK(bit_width(0b00001111111001111111111111111111_u32) == 28);
	CHECK(bit_width(0b00011111111111111111111111100111_u32) == 29);
	CHECK(bit_width(0b00111111111110011111011111111111_u32) == 30);
	CHECK(bit_width(0b01111111111101111111111111111111_u32) == 31);
	CHECK(bit_width(0b11111111111001111111111111111111_u32) == 32);

	static_assert(bit_width(0b0000000000000000_u16) == 0);
	static_assert(bit_width(0b0000000000000001_u16) == 1);
	static_assert(bit_width(0b0000000000000011_u16) == 2);
	static_assert(bit_width(0b0000000000000111_u16) == 3);
	static_assert(bit_width(0b0000000000001111_u16) == 4);
	static_assert(bit_width(0b0000000000011111_u16) == 5);
	static_assert(bit_width(0b0000000000111011_u16) == 6);
	static_assert(bit_width(0b0000000001111111_u16) == 7);
	static_assert(bit_width(0b0000000011100111_u16) == 8);
	static_assert(bit_width(0b0000000111111111_u16) == 9);
	static_assert(bit_width(0b0000001111111111_u16) == 10);
	static_assert(bit_width(0b0000011111011111_u16) == 11);
	static_assert(bit_width(0b0000111111111111_u16) == 12);
	static_assert(bit_width(0b0001111011111001_u16) == 13);
	static_assert(bit_width(0b0011111110111111_u16) == 14);
	static_assert(bit_width(0b0111100111111111_u16) == 15);
	static_assert(bit_width(0b1111111110011111_u16) == 16);

	CHECK(bit_width(0b0000000000000000_u16) == 0);
	CHECK(bit_width(0b0000000000000001_u16) == 1);
	CHECK(bit_width(0b0000000000000011_u16) == 2);
	CHECK(bit_width(0b0000000000000111_u16) == 3);
	CHECK(bit_width(0b0000000000001111_u16) == 4);
	CHECK(bit_width(0b0000000000011111_u16) == 5);
	CHECK(bit_width(0b0000000000111111_u16) == 6);
	CHECK(bit_width(0b0000000001111011_u16) == 7);
	CHECK(bit_width(0b0000000011111111_u16) == 8);
	CHECK(bit_width(0b0000000111111011_u16) == 9);
	CHECK(bit_width(0b0000001111111111_u16) == 10);
	CHECK(bit_width(0b0000011111111111_u16) == 11);
	CHECK(bit_width(0b0000111011111111_u16) == 12);
	CHECK(bit_width(0b0001111111111111_u16) == 13);
	CHECK(bit_width(0b0011111111111111_u16) == 14);
	CHECK(bit_width(0b0111111111011111_u16) == 15);
	CHECK(bit_width(0b1111011101111111_u16) == 16);

	static_assert(bit_width(0b00000000_u8) == 0);
	static_assert(bit_width(0b00000001_u8) == 1);
	static_assert(bit_width(0b00000011_u8) == 2);
	static_assert(bit_width(0b00000111_u8) == 3);
	static_assert(bit_width(0b00001101_u8) == 4);
	static_assert(bit_width(0b00011101_u8) == 5);
	static_assert(bit_width(0b00111111_u8) == 6);
	static_assert(bit_width(0b01110111_u8) == 7);
	static_assert(bit_width(0b11111101_u8) == 8);

	CHECK(bit_width(0b00000000_u8) == 0);
	CHECK(bit_width(0b00000001_u8) == 1);
	CHECK(bit_width(0b00000011_u8) == 2);
	CHECK(bit_width(0b00000110_u8) == 3);
	CHECK(bit_width(0b00001101_u8) == 4);
	CHECK(bit_width(0b00011111_u8) == 5);
	CHECK(bit_width(0b00111011_u8) == 6);
	CHECK(bit_width(0b01111111_u8) == 7);
	CHECK(bit_width(0b11110111_u8) == 8);
}

TEST_CASE("bit_fill_right")
{
	static_assert(bit_fill_right<uint8_t>(0)  == 0b00000000_u8);
	static_assert(bit_fill_right<uint8_t>(1)  == 0b00000001_u8);
	static_assert(bit_fill_right<uint8_t>(2)  == 0b00000011_u8);
	static_assert(bit_fill_right<uint8_t>(3)  == 0b00000111_u8);
	static_assert(bit_fill_right<uint8_t>(4)  == 0b00001111_u8);
	static_assert(bit_fill_right<uint8_t>(5)  == 0b00011111_u8);
	static_assert(bit_fill_right<uint8_t>(6)  == 0b00111111_u8);
	static_assert(bit_fill_right<uint8_t>(7)  == 0b01111111_u8);
	static_assert(bit_fill_right<uint8_t>(8)  == 0b11111111_u8);
	static_assert(bit_fill_right<uint8_t>(99) == 0b11111111_u8);

	CHECK(bit_fill_right<uint8_t>(0)  == 0b00000000_u8);
	CHECK(bit_fill_right<uint8_t>(1)  == 0b00000001_u8);
	CHECK(bit_fill_right<uint8_t>(2)  == 0b00000011_u8);
	CHECK(bit_fill_right<uint8_t>(3)  == 0b00000111_u8);
	CHECK(bit_fill_right<uint8_t>(4)  == 0b00001111_u8);
	CHECK(bit_fill_right<uint8_t>(5)  == 0b00011111_u8);
	CHECK(bit_fill_right<uint8_t>(6)  == 0b00111111_u8);
	CHECK(bit_fill_right<uint8_t>(7)  == 0b01111111_u8);
	CHECK(bit_fill_right<uint8_t>(8)  == 0b11111111_u8);
	CHECK(bit_fill_right<uint8_t>(99) == 0b11111111_u8);

	static_assert(bit_fill_right<uint16_t>(0)  == 0b0000000000000000_u16);
	static_assert(bit_fill_right<uint16_t>(1)  == 0b0000000000000001_u16);
	static_assert(bit_fill_right<uint16_t>(2)  == 0b0000000000000011_u16);
	static_assert(bit_fill_right<uint16_t>(3)  == 0b0000000000000111_u16);
	static_assert(bit_fill_right<uint16_t>(4)  == 0b0000000000001111_u16);
	static_assert(bit_fill_right<uint16_t>(5)  == 0b0000000000011111_u16);
	static_assert(bit_fill_right<uint16_t>(6)  == 0b0000000000111111_u16);
	static_assert(bit_fill_right<uint16_t>(7)  == 0b0000000001111111_u16);
	static_assert(bit_fill_right<uint16_t>(8)  == 0b0000000011111111_u16);
	static_assert(bit_fill_right<uint16_t>(9)  == 0b0000000111111111_u16);
	static_assert(bit_fill_right<uint16_t>(10) == 0b0000001111111111_u16);
	static_assert(bit_fill_right<uint16_t>(11) == 0b0000011111111111_u16);
	static_assert(bit_fill_right<uint16_t>(12) == 0b0000111111111111_u16);
	static_assert(bit_fill_right<uint16_t>(13) == 0b0001111111111111_u16);
	static_assert(bit_fill_right<uint16_t>(14) == 0b0011111111111111_u16);
	static_assert(bit_fill_right<uint16_t>(15) == 0b0111111111111111_u16);
	static_assert(bit_fill_right<uint16_t>(16) == 0b1111111111111111_u16);
	static_assert(bit_fill_right<uint16_t>(99) == 0b1111111111111111_u16);

	CHECK(bit_fill_right<uint16_t>(0)  == 0b0000000000000000_u16);
	CHECK(bit_fill_right<uint16_t>(1)  == 0b0000000000000001_u16);
	CHECK(bit_fill_right<uint16_t>(2)  == 0b0000000000000011_u16);
	CHECK(bit_fill_right<uint16_t>(3)  == 0b0000000000000111_u16);
	CHECK(bit_fill_right<uint16_t>(4)  == 0b0000000000001111_u16);
	CHECK(bit_fill_right<uint16_t>(5)  == 0b0000000000011111_u16);
	CHECK(bit_fill_right<uint16_t>(6)  == 0b0000000000111111_u16);
	CHECK(bit_fill_right<uint16_t>(7)  == 0b0000000001111111_u16);
	CHECK(bit_fill_right<uint16_t>(8)  == 0b0000000011111111_u16);
	CHECK(bit_fill_right<uint16_t>(9)  == 0b0000000111111111_u16);
	CHECK(bit_fill_right<uint16_t>(10) == 0b0000001111111111_u16);
	CHECK(bit_fill_right<uint16_t>(11) == 0b0000011111111111_u16);
	CHECK(bit_fill_right<uint16_t>(12) == 0b0000111111111111_u16);
	CHECK(bit_fill_right<uint16_t>(13) == 0b0001111111111111_u16);
	CHECK(bit_fill_right<uint16_t>(14) == 0b0011111111111111_u16);
	CHECK(bit_fill_right<uint16_t>(15) == 0b0111111111111111_u16);
	CHECK(bit_fill_right<uint16_t>(16) == 0b1111111111111111_u16);
	CHECK(bit_fill_right<uint16_t>(99) == 0b1111111111111111_u16);

	static_assert(bit_fill_right<uint32_t>(0)  == 0b00000000000000000000000000000000_u32);
	static_assert(bit_fill_right<uint32_t>(1)  == 0b00000000000000000000000000000001_u32);
	static_assert(bit_fill_right<uint32_t>(2)  == 0b00000000000000000000000000000011_u32);
	static_assert(bit_fill_right<uint32_t>(3)  == 0b00000000000000000000000000000111_u32);
	static_assert(bit_fill_right<uint32_t>(4)  == 0b00000000000000000000000000001111_u32);
	static_assert(bit_fill_right<uint32_t>(5)  == 0b00000000000000000000000000011111_u32);
	static_assert(bit_fill_right<uint32_t>(6)  == 0b00000000000000000000000000111111_u32);
	static_assert(bit_fill_right<uint32_t>(7)  == 0b00000000000000000000000001111111_u32);
	static_assert(bit_fill_right<uint32_t>(8)  == 0b00000000000000000000000011111111_u32);
	static_assert(bit_fill_right<uint32_t>(9)  == 0b00000000000000000000000111111111_u32);
	static_assert(bit_fill_right<uint32_t>(10) == 0b00000000000000000000001111111111_u32);
	static_assert(bit_fill_right<uint32_t>(11) == 0b00000000000000000000011111111111_u32);
	static_assert(bit_fill_right<uint32_t>(12) == 0b00000000000000000000111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(13) == 0b00000000000000000001111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(14) == 0b00000000000000000011111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(15) == 0b00000000000000000111111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(16) == 0b00000000000000001111111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(17) == 0b00000000000000011111111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(18) == 0b00000000000000111111111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(19) == 0b00000000000001111111111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(20) == 0b00000000000011111111111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(21) == 0b00000000000111111111111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(22) == 0b00000000001111111111111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(23) == 0b00000000011111111111111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(24) == 0b00000000111111111111111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(25) == 0b00000001111111111111111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(26) == 0b00000011111111111111111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(27) == 0b00000111111111111111111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(28) == 0b00001111111111111111111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(29) == 0b00011111111111111111111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(30) == 0b00111111111111111111111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(31) == 0b01111111111111111111111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(32) == 0b11111111111111111111111111111111_u32);
	static_assert(bit_fill_right<uint32_t>(99) == 0b11111111111111111111111111111111_u32);

	CHECK(bit_fill_right<uint32_t>(0)  == 0b00000000000000000000000000000000_u32);
	CHECK(bit_fill_right<uint32_t>(1)  == 0b00000000000000000000000000000001_u32);
	CHECK(bit_fill_right<uint32_t>(2)  == 0b00000000000000000000000000000011_u32);
	CHECK(bit_fill_right<uint32_t>(3)  == 0b00000000000000000000000000000111_u32);
	CHECK(bit_fill_right<uint32_t>(4)  == 0b00000000000000000000000000001111_u32);
	CHECK(bit_fill_right<uint32_t>(5)  == 0b00000000000000000000000000011111_u32);
	CHECK(bit_fill_right<uint32_t>(6)  == 0b00000000000000000000000000111111_u32);
	CHECK(bit_fill_right<uint32_t>(7)  == 0b00000000000000000000000001111111_u32);
	CHECK(bit_fill_right<uint32_t>(8)  == 0b00000000000000000000000011111111_u32);
	CHECK(bit_fill_right<uint32_t>(9)  == 0b00000000000000000000000111111111_u32);
	CHECK(bit_fill_right<uint32_t>(10) == 0b00000000000000000000001111111111_u32);
	CHECK(bit_fill_right<uint32_t>(11) == 0b00000000000000000000011111111111_u32);
	CHECK(bit_fill_right<uint32_t>(12) == 0b00000000000000000000111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(13) == 0b00000000000000000001111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(14) == 0b00000000000000000011111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(15) == 0b00000000000000000111111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(16) == 0b00000000000000001111111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(17) == 0b00000000000000011111111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(18) == 0b00000000000000111111111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(19) == 0b00000000000001111111111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(20) == 0b00000000000011111111111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(21) == 0b00000000000111111111111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(22) == 0b00000000001111111111111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(23) == 0b00000000011111111111111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(24) == 0b00000000111111111111111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(25) == 0b00000001111111111111111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(26) == 0b00000011111111111111111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(27) == 0b00000111111111111111111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(28) == 0b00001111111111111111111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(29) == 0b00011111111111111111111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(30) == 0b00111111111111111111111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(31) == 0b01111111111111111111111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(32) == 0b11111111111111111111111111111111_u32);
	CHECK(bit_fill_right<uint32_t>(99) == 0b11111111111111111111111111111111_u32);
}

TEST_CASE("bit_fill_left")
{
	static_assert(bit_fill_left<uint8_t>(0)  == 0b00000000_u8);
	static_assert(bit_fill_left<uint8_t>(1)  == 0b10000000_u8);
	static_assert(bit_fill_left<uint8_t>(2)  == 0b11000000_u8);
	static_assert(bit_fill_left<uint8_t>(3)  == 0b11100000_u8);
	static_assert(bit_fill_left<uint8_t>(4)  == 0b11110000_u8);
	static_assert(bit_fill_left<uint8_t>(5)  == 0b11111000_u8);
	static_assert(bit_fill_left<uint8_t>(6)  == 0b11111100_u8);
	static_assert(bit_fill_left<uint8_t>(7)  == 0b11111110_u8);
	static_assert(bit_fill_left<uint8_t>(8)  == 0b11111111_u8);
	static_assert(bit_fill_left<uint8_t>(99) == 0b11111111_u8);

	CHECK(bit_fill_left<uint8_t>(0)  == 0b00000000_u8);
	CHECK(bit_fill_left<uint8_t>(1)  == 0b10000000_u8);
	CHECK(bit_fill_left<uint8_t>(2)  == 0b11000000_u8);
	CHECK(bit_fill_left<uint8_t>(3)  == 0b11100000_u8);
	CHECK(bit_fill_left<uint8_t>(4)  == 0b11110000_u8);
	CHECK(bit_fill_left<uint8_t>(5)  == 0b11111000_u8);
	CHECK(bit_fill_left<uint8_t>(6)  == 0b11111100_u8);
	CHECK(bit_fill_left<uint8_t>(7)  == 0b11111110_u8);
	CHECK(bit_fill_left<uint8_t>(8)  == 0b11111111_u8);
	CHECK(bit_fill_left<uint8_t>(99) == 0b11111111_u8);

	static_assert(bit_fill_left<uint16_t>(0)  == 0b0000000000000000_u16);
	static_assert(bit_fill_left<uint16_t>(1)  == 0b1000000000000000_u16);
	static_assert(bit_fill_left<uint16_t>(2)  == 0b1100000000000000_u16);
	static_assert(bit_fill_left<uint16_t>(3)  == 0b1110000000000000_u16);
	static_assert(bit_fill_left<uint16_t>(4)  == 0b1111000000000000_u16);
	static_assert(bit_fill_left<uint16_t>(5)  == 0b1111100000000000_u16);
	static_assert(bit_fill_left<uint16_t>(6)  == 0b1111110000000000_u16);
	static_assert(bit_fill_left<uint16_t>(7)  == 0b1111111000000000_u16);
	static_assert(bit_fill_left<uint16_t>(8)  == 0b1111111100000000_u16);
	static_assert(bit_fill_left<uint16_t>(9)  == 0b1111111110000000_u16);
	static_assert(bit_fill_left<uint16_t>(10) == 0b1111111111000000_u16);
	static_assert(bit_fill_left<uint16_t>(11) == 0b1111111111100000_u16);
	static_assert(bit_fill_left<uint16_t>(12) == 0b1111111111110000_u16);
	static_assert(bit_fill_left<uint16_t>(13) == 0b1111111111111000_u16);
	static_assert(bit_fill_left<uint16_t>(14) == 0b1111111111111100_u16);
	static_assert(bit_fill_left<uint16_t>(15) == 0b1111111111111110_u16);
	static_assert(bit_fill_left<uint16_t>(16) == 0b1111111111111111_u16);
	static_assert(bit_fill_left<uint16_t>(99) == 0b1111111111111111_u16);

	CHECK(bit_fill_left<uint16_t>(0)  == 0b0000000000000000_u16);
	CHECK(bit_fill_left<uint16_t>(1)  == 0b1000000000000000_u16);
	CHECK(bit_fill_left<uint16_t>(2)  == 0b1100000000000000_u16);
	CHECK(bit_fill_left<uint16_t>(3)  == 0b1110000000000000_u16);
	CHECK(bit_fill_left<uint16_t>(4)  == 0b1111000000000000_u16);
	CHECK(bit_fill_left<uint16_t>(5)  == 0b1111100000000000_u16);
	CHECK(bit_fill_left<uint16_t>(6)  == 0b1111110000000000_u16);
	CHECK(bit_fill_left<uint16_t>(7)  == 0b1111111000000000_u16);
	CHECK(bit_fill_left<uint16_t>(8)  == 0b1111111100000000_u16);
	CHECK(bit_fill_left<uint16_t>(9)  == 0b1111111110000000_u16);
	CHECK(bit_fill_left<uint16_t>(10) == 0b1111111111000000_u16);
	CHECK(bit_fill_left<uint16_t>(11) == 0b1111111111100000_u16);
	CHECK(bit_fill_left<uint16_t>(12) == 0b1111111111110000_u16);
	CHECK(bit_fill_left<uint16_t>(13) == 0b1111111111111000_u16);
	CHECK(bit_fill_left<uint16_t>(14) == 0b1111111111111100_u16);
	CHECK(bit_fill_left<uint16_t>(15) == 0b1111111111111110_u16);
	CHECK(bit_fill_left<uint16_t>(16) == 0b1111111111111111_u16);
	CHECK(bit_fill_left<uint16_t>(99) == 0b1111111111111111_u16);

	static_assert(bit_fill_left<uint32_t>(0)  == 0b00000000000000000000000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(1)  == 0b10000000000000000000000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(2)  == 0b11000000000000000000000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(3)  == 0b11100000000000000000000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(4)  == 0b11110000000000000000000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(5)  == 0b11111000000000000000000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(6)  == 0b11111100000000000000000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(7)  == 0b11111110000000000000000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(8)  == 0b11111111000000000000000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(9)  == 0b11111111100000000000000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(10) == 0b11111111110000000000000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(11) == 0b11111111111000000000000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(12) == 0b11111111111100000000000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(13) == 0b11111111111110000000000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(14) == 0b11111111111111000000000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(15) == 0b11111111111111100000000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(16) == 0b11111111111111110000000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(17) == 0b11111111111111111000000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(18) == 0b11111111111111111100000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(19) == 0b11111111111111111110000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(20) == 0b11111111111111111111000000000000_u32);
	static_assert(bit_fill_left<uint32_t>(21) == 0b11111111111111111111100000000000_u32);
	static_assert(bit_fill_left<uint32_t>(22) == 0b11111111111111111111110000000000_u32);
	static_assert(bit_fill_left<uint32_t>(23) == 0b11111111111111111111111000000000_u32);
	static_assert(bit_fill_left<uint32_t>(24) == 0b11111111111111111111111100000000_u32);
	static_assert(bit_fill_left<uint32_t>(25) == 0b11111111111111111111111110000000_u32);
	static_assert(bit_fill_left<uint32_t>(26) == 0b11111111111111111111111111000000_u32);
	static_assert(bit_fill_left<uint32_t>(27) == 0b11111111111111111111111111100000_u32);
	static_assert(bit_fill_left<uint32_t>(28) == 0b11111111111111111111111111110000_u32);
	static_assert(bit_fill_left<uint32_t>(29) == 0b11111111111111111111111111111000_u32);
	static_assert(bit_fill_left<uint32_t>(30) == 0b11111111111111111111111111111100_u32);
	static_assert(bit_fill_left<uint32_t>(31) == 0b11111111111111111111111111111110_u32);
	static_assert(bit_fill_left<uint32_t>(32) == 0b11111111111111111111111111111111_u32);
	static_assert(bit_fill_left<uint32_t>(99) == 0b11111111111111111111111111111111_u32);

	CHECK(bit_fill_left<uint32_t>(0)  == 0b00000000000000000000000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(1)  == 0b10000000000000000000000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(2)  == 0b11000000000000000000000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(3)  == 0b11100000000000000000000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(4)  == 0b11110000000000000000000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(5)  == 0b11111000000000000000000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(6)  == 0b11111100000000000000000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(7)  == 0b11111110000000000000000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(8)  == 0b11111111000000000000000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(9)  == 0b11111111100000000000000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(10) == 0b11111111110000000000000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(11) == 0b11111111111000000000000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(12) == 0b11111111111100000000000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(13) == 0b11111111111110000000000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(14) == 0b11111111111111000000000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(15) == 0b11111111111111100000000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(16) == 0b11111111111111110000000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(17) == 0b11111111111111111000000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(18) == 0b11111111111111111100000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(19) == 0b11111111111111111110000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(20) == 0b11111111111111111111000000000000_u32);
	CHECK(bit_fill_left<uint32_t>(21) == 0b11111111111111111111100000000000_u32);
	CHECK(bit_fill_left<uint32_t>(22) == 0b11111111111111111111110000000000_u32);
	CHECK(bit_fill_left<uint32_t>(23) == 0b11111111111111111111111000000000_u32);
	CHECK(bit_fill_left<uint32_t>(24) == 0b11111111111111111111111100000000_u32);
	CHECK(bit_fill_left<uint32_t>(25) == 0b11111111111111111111111110000000_u32);
	CHECK(bit_fill_left<uint32_t>(26) == 0b11111111111111111111111111000000_u32);
	CHECK(bit_fill_left<uint32_t>(27) == 0b11111111111111111111111111100000_u32);
	CHECK(bit_fill_left<uint32_t>(28) == 0b11111111111111111111111111110000_u32);
	CHECK(bit_fill_left<uint32_t>(29) == 0b11111111111111111111111111111000_u32);
	CHECK(bit_fill_left<uint32_t>(30) == 0b11111111111111111111111111111100_u32);
	CHECK(bit_fill_left<uint32_t>(31) == 0b11111111111111111111111111111110_u32);
	CHECK(bit_fill_left<uint32_t>(32) == 0b11111111111111111111111111111111_u32);
	CHECK(bit_fill_left<uint32_t>(99) == 0b11111111111111111111111111111111_u32);
}

TEST_CASE("clamp")
{
	static_assert(clamp(1, 2, 4) == 2);
	static_assert(clamp(2, 2, 4) == 2);
	static_assert(clamp(3, 2, 4) == 3);
	static_assert(clamp(4, 2, 4) == 4);
	static_assert(clamp(5, 2, 4) == 4);

	CHECK(clamp(1, 2, 4) == 2);
	CHECK(clamp(2, 2, 4) == 2);
	CHECK(clamp(3, 2, 4) == 3);
	CHECK(clamp(4, 2, 4) == 4);
	CHECK(clamp(5, 2, 4) == 4);
}

TEST_CASE("is_between")
{
	static_assert(!is_between(1, 2, 4));
	static_assert(is_between(2, 2, 4));
	static_assert(is_between(3, 2, 4));
	static_assert(is_between(4, 2, 4));
	static_assert(!is_between(5, 2, 4));

	CHECK(!is_between(1, 2, 4));
	CHECK(is_between(2, 2, 4));
	CHECK(is_between(3, 2, 4));
	CHECK(is_between(4, 2, 4));
	CHECK(!is_between(5, 2, 4));
}

namespace
{
	static int test_val = 0;
	MUU_NEVER_INLINE int func1() { return test_val = 1; }
	MUU_NEVER_INLINE int func2() noexcept { return test_val = 2; }

	struct base { virtual ~base() noexcept = default; };
	struct derived1 : base {};
	struct derived2 : base {};
}

TEST_CASE("pointer_cast")
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
}
