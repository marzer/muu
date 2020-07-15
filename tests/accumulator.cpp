#include "tests.h"
#include "float_tests.h"
#include "../include/muu/accumulator.h"

MUU_PUSH_WARNINGS
MUU_DISABLE_ARITHMETIC_WARNINGS

template <typename T>
static void test_float_accumulator() noexcept
{
	using data = float_test_data<T>;

	//test calling add() for each value
	{
		T raw_sum = {};
		accumulator<T> accum;
		CHECK(accum.sample_count() == 0_sz);
		CHECK(accum.value() == T{});
		for (auto val : data::values)
		{
			raw_sum += static_cast<T>(val);
			accum.add(static_cast<T>(val));
		}
		CHECK(accum.sample_count() == std::size(data::values));
		CHECK(raw_sum != data::values_sum);
		CHECK((std::abs(data::values_sum - accum.value()))
		   <= (std::abs(data::values_sum - raw_sum)));
		CHECK(accum.value() >= data::values_sum_low);
		CHECK(accum.value() <= data::values_sum_high);
	}

	//test calling add(begin, end)
	if constexpr (!std::is_same_v<T, float16>)
	{
		accumulator<T> accum;
		accum.add(std::begin(data::values), std::end(data::values));
		CHECK(accum.sample_count() == std::size(data::values));
		CHECK(accum.value() >= data::values_sum_low);
		CHECK(accum.value() <= data::values_sum_high);
	}
}

TEST_CASE("accumulator - float16")
{
	test_float_accumulator<float16>();
}

TEST_CASE("accumulator - float")
{
	test_float_accumulator<float>();
}

TEST_CASE("accumulator - double")
{
	test_float_accumulator<double>();
}

TEST_CASE("accumulator - long double")
{
	test_float_accumulator<long double>();
}

TEST_CASE("accumulator - integers")
{
	int raw_sum = {};
	accumulator<int> accum;
	CHECK(accum.sample_count() == 0_sz);
	CHECK(accum.value() == 0);
	for (size_t i = 0; i < 1000; i++)
	{
		const auto val = rand();
		raw_sum += val;
		accum.add(val);
	}
	CHECK(accum.sample_count() == 1000_sz);
	CHECK(accum.value() == raw_sum);
}

MUU_POP_WARNINGS // MUU_DISABLE_ARITHMETIC_WARNINGS
