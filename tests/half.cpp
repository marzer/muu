#include "tests.h"
#include "../include/muu/half.h"

MUU_DISABLE_ARITHMETIC_WARNINGS

///////////////////////////////////////////////////////////////////////////////////
// Some runtime tests in this file adapted from
// 1) https://github.com/acgessler/half_float/blob/master/HalfPrecisionFloatTest.cpp
// 2) https://github.com/openexr/openexr/blob/develop/IlmBase/HalfTest/
///////////////////////////////////////////////////////////////////////////////////

static_assert(sizeof(half) == 2_sz);
static_assert(std::is_standard_layout_v<half>);
static_assert(std::is_trivially_constructible_v<half>);
static_assert(std::is_trivially_copy_constructible_v<half>);
static_assert(std::is_trivially_copy_assignable_v<half>);
static_assert(std::is_trivially_move_constructible_v<half>);
static_assert(std::is_trivially_move_assignable_v<half>);
static_assert(std::is_trivially_destructible_v<half>);

TEST_CASE("half - construction")
{
	constexpr auto zero_init_aggr_constexpr = half{};
	CHECK(zero_init_aggr_constexpr == constants<half>::zero);

	auto zero_init_aggr = half{};
	CHECK(zero_init_aggr == constants<half>::zero);

	// sanity check the constant
	uint16_t val;
	memcpy(&val, &zero_init_aggr, 2u);
	CHECK(val == 0_u16);
}

TEST_CASE("half - negation")
{
	for (int i = -100; i <= 100; i++)
	{
		const auto negated1 = -half{ i };
		const auto negated2 = half{ -i };
		if (i != 0)
			CHECK(negated1 == negated2);
		CHECK(static_cast<float>(negated1) == -static_cast<float>(i));
		CHECK(static_cast<double>(negated1) == -static_cast<double>(i));
	}
}

TEST_CASE("half - conversions")
{
	static constexpr auto convert_from_int = [](auto v) noexcept
	{
		const auto val = half{ v };
		CHECK(static_cast<float>(val) == static_cast<float>(v));
		CHECK(static_cast<double>(val) == static_cast<double>(v));
	};

	for (int i = -10; i < 0; i++)
	{
		convert_from_int(static_cast<int8_t>(i));
		convert_from_int(static_cast<int16_t>(i));
		convert_from_int(static_cast<int32_t>(i));
		convert_from_int(static_cast<int64_t>(i));
		#if MUU_HAS_INT128
		convert_from_int(static_cast<int128_t>(i));
		#endif
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
		#if MUU_HAS_INT128
		convert_from_int(static_cast<int128_t>(i));
		convert_from_int(static_cast<uint128_t>(i));
		#endif
	}
}

TEST_CASE("half - basic arithmetic")
{
	//adapted from acgessler:
	{
		half h{ 1 }, h2{ 2 };
		--h2;
		++h2;
		--h;
		++h;
		h2 -= half{ 1 };
		float f = static_cast<float>(h2), f2 = static_cast<float>(h);
		CHECK(f == 1.0f);
		CHECK(f == f2);

		h = h2;
		h2 = half{ 15.5f };

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
		h2 += half{ 17.5f };
		h2 *= h;
		f = static_cast<float>(h2);
		f2 = static_cast<float>(h);
		CHECK(4.f == f);
		CHECK(4.f == f2);
		CHECK(h == h2);
		CHECK(h <= h2);

		--h;
		CHECK(h <= h2);

		h -= half{ 250.f };
		CHECK(h < h2);

		h += half{ 500.f };
		CHECK(h > h2);
		CHECK(h >= h2);

		f = static_cast<float>(h2);
		f2 = static_cast<float>(h);
		CHECK(h * h2 == half{ f * f2 });
	}

	//adapted from openexr:
	{
		float f1 = 1.0f;
		float f2 = 2.0f;
		half  h1 = half{ 3 };
		half  h2 = half{ 4 };

		h1 = half{ f1 + f2 };
		CHECK(h1 == half{ 3 });

		h2 += half{ f1 };
		CHECK(h2 == half{ 5 });

		h2 = h1 + h2;
		CHECK(h2 == half{ 8 });

		h2 += h1;
		CHECK(h2 == half{ 11 });

		h1 = h2;
		CHECK(h1 == half{ 11 });

		h2 = -h1;
		CHECK(h2 == half{ -11 });
	}

}

TEST_CASE("half - addition")
{
	// identical exponents
	for (float f = 0.f; f < 1000.f; ++f)
	{
		half one = half{ f };
		half two = half{ f };
		half three = one + two;
		auto f2 = static_cast<float>(three);
		CHECK(f * 2.f == f2);
	}

	// different exponents
	for (float f = 0.f, fp = 1000.f; f < 500.f; ++f, --fp)
	{
		half one = half{ f };
		half two = half{ fp };
		half three = one + two;
		auto f2 = static_cast<float>(three);
		CHECK(f + fp == f2);
	}

	// very small numbers - this is already beyond the accuracy of 16 bit floats.
	for (float f = 0.003f; f < 100.f; f += 0.0005f)
	{
		half one = half{ f };
		half two = half{ f };
		half three = one + two;
		auto f2 = static_cast<float>(three);
		float m = f * 2.f;
		CHECK(f2 > (m - 0.05 * m));
		CHECK(f2 < (m + 0.05 * m));
	}
}

TEST_CASE("half - subtraction")
{
	// identical exponents
	for (float f = 0.f; f < 1000.f; ++f)
	{
		half one = half{ f };
		half two = half{ f };
		half three = one - two;
		auto f2 = static_cast<float>(three);
		CHECK(0.f == f2);
	}

	// different exponents
	for (float f = 0.f, fp = 1000.f; f < 500.f; ++f, --fp)
	{
		half one = half{ f };
		half two = half{ fp };
		half three = one - two;
		auto f2 = static_cast<float>(three);
		CHECK(f - fp == f2);
	}
}

