#include "tests.h"
#include "../include/muu/float16.h"

MUU_PUSH_WARNINGS
MUU_DISABLE_FLOAT_WARNINGS

///////////////////////////////////////////////////////////////////////////////////
// Some runtime tests in this file adapted from
// 1) https://github.com/acgessler/half_float/blob/master/HalfPrecisionFloatTest.cpp
// 2) https://github.com/openexr/openexr/blob/develop/IlmBase/HalfTest/
///////////////////////////////////////////////////////////////////////////////////

static_assert(std::is_standard_layout_v<float16>);
static_assert(std::is_trivially_constructible_v<float16>);
static_assert(std::is_trivially_copy_constructible_v<float16>);
static_assert(std::is_trivially_copy_assignable_v<float16>);
static_assert(std::is_trivially_move_constructible_v<float16>);
static_assert(std::is_trivially_move_assignable_v<float16>);
static_assert(std::is_trivially_destructible_v<float16>);
static_assert(sizeof(float16) == 2_sz);

TEST_CASE("float16 - negation")
{
	for (int i = -100; i <= 100; i++)
	{
		const auto negated1 = -float16{ i };
		const auto negated2 = float16{ -i };
		if (i == 0)
			CHECK((negated1.bits & 0b0111111111111111_u16) == (negated2.bits & 0b0111111111111111_u16));
		else
		{
			CHECK(negated1.bits == negated2.bits);
			CHECK(!!(negated2.bits & 0b1000000000000000_u16) == (i >= 0));
		}
		CHECK(static_cast<float>(negated1) == -static_cast<float>(i));
		CHECK(static_cast<double>(negated1) == -static_cast<double>(i));
	}
}

TEST_CASE("float16 - conversions")
{
	static constexpr auto convert_from_int = [](auto v) noexcept
	{
		const auto val = float16{ v };
		CHECK(static_cast<float>(val) == static_cast<float>(v));
		CHECK(static_cast<double>(val) == static_cast<double>(v));
	};

	for (int i = -10; i < 0; i++)
	{
		convert_from_int(static_cast<int8_t>(i));
		convert_from_int(static_cast<int16_t>(i));
		convert_from_int(static_cast<int32_t>(i));
		convert_from_int(static_cast<int64_t>(i));
	}
	for (int i = 0; i < 10; i++)
	{
		convert_from_int(static_cast<int8_t>(i));
		convert_from_int(static_cast<int16_t>(i));
		convert_from_int(static_cast<int32_t>(i));
		convert_from_int(static_cast<int64_t>(i));
		convert_from_int(static_cast<uint8_t>(i));
		convert_from_int(static_cast<uint16_t>(i));
		convert_from_int(static_cast<uint32_t>(i));
		convert_from_int(static_cast<uint64_t>(i));
	}
}

TEST_CASE("float16 - basic arithmetic")
{
	//adapted from acgessler:
	{
		float16 h{ 1 }, h2{ 2 };
		--h2;
		++h2;
		--h;
		++h;
		h2 -= float16{ 1 };
		float f = static_cast<float>(h2), f2 = static_cast<float>(h);
		CHECK(f == 1.0f);
		CHECK(f == f2);

		h = h2;
		h2 = float16{ 15.5f };

		f = static_cast<float>(h2);
		f2 = static_cast<float>(h);
		CHECK(15.5f == f);
		CHECK(1.f == f2);

		h2 *= h;
		f = static_cast<float>(h2);
		f2 = static_cast<float>(h);
		CHECK(15.5f == f);
		CHECK(1.f == f2);

		h2 /= h;
		f = static_cast<float>(h2);
		f2 = static_cast<float>(h);
		CHECK(15.5f == f);
		CHECK(1.f == f2);

		h2 += h;
		f = static_cast<float>(h2);
		f2 = static_cast<float>(h);
		CHECK(16.5f == f);
		CHECK(1.f == f2);

		h++; h++; h++;
		h2 = -h2;
		h2 += float16{ 17.5f };
		h2 *= h;
		f = static_cast<float>(h2);
		f2 = static_cast<float>(h);
		CHECK(4.f == f);
		CHECK(4.f == f2);
		CHECK(h == h2);
		CHECK(h <= h2);

		--h;
		CHECK(h <= h2);

		h -= float16{ 250.f };
		CHECK(h < h2);

		h += float16{ 500.f };
		CHECK(h > h2);
		CHECK(h >= h2);

		f = static_cast<float>(h2);
		f2 = static_cast<float>(h);
		CHECK(h * h2 == float16{ f * f2 });
	}

	//adapted from openexr:
	{
		float f1 = 1.0f;
		float f2 = 2.0f;
		float16  h1 = float16{ 3 };
		float16  h2 = float16{ 4 };

		h1 = float16{ f1 + f2 };
		CHECK(h1 == float16{ 3 });

		h2 += float16{ f1 };
		CHECK(h2 == float16{ 5 });

		h2 = h1 + h2;
		CHECK(h2 == float16{ 8 });

		h2 += h1;
		CHECK(h2 == float16{ 11 });

		h1 = h2;
		CHECK(h1 == float16{ 11 });

		h2 = -h1;
		CHECK(h2 == float16{ -11 });
	}

}

TEST_CASE("float16 - addition")
{
	// identical exponents
	for (float f = 0.f; f < 1000.f; ++f)
	{
		float16 one = float16{ f };
		float16 two = float16{ f };
		float16 three = one + two;
		auto f2 = static_cast<float>(three);
		CHECK(f * 2.f == f2);
	}

	// different exponents
	for (float f = 0.f, fp = 1000.f; f < 500.f; ++f, --fp)
	{
		float16 one = float16{ f };
		float16 two = float16{ fp };
		float16 three = one + two;
		auto f2 = static_cast<float>(three);
		CHECK(f + fp == f2);
	}

	// very small numbers - this is already beyond the accuracy of 16 bit floats.
	for (float f = 0.003f; f < 100.f; f += 0.0005f)
	{
		float16 one = float16{ f };
		float16 two = float16{ f };
		float16 three = one + two;
		auto f2 = static_cast<float>(three);
		float m = f * 2.f;
		CHECK(f2 > (m - 0.05 * m));
		CHECK(f2 < (m + 0.05 * m));
	}
}

TEST_CASE("float16 - subtraction")
{
	// identical exponents
	for (float f = 0.f; f < 1000.f; ++f)
	{
		float16 one = float16{ f };
		float16 two = float16{ f };
		float16 three = one - two;
		auto f2 = static_cast<float>(three);
		CHECK(0.f == f2);
	}

	// different exponents
	for (float f = 0.f, fp = 1000.f; f < 500.f; ++f, --fp)
	{
		float16 one = float16{ f };
		float16 two = float16{ fp };
		float16 three = one - two;
		auto f2 = static_cast<float>(three);
		CHECK(f - fp == f2);
	}
}

MUU_POP_WARNINGS // MUU_DISABLE_FLOAT_WARNINGS
