// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"

// many of the muu math functions are just shims around the standard library ones at runtime,
// and only behave differently at compile-time, where they use some integration to arrive at a result
// (e.g. taylor series).
//
// this means that tests just boil down to muu::func == std::func at runtime
// and muu::func == lookup table value at compile time.
// 
// a complicating factor is the presense of libquadmath; when it's available the muu math functions will use quad-precision
// functions internally at runtime for float128_t, otherwise they downcast to long double and upcast the result.
// that's what all the QUADMATH nonsense is about down below. still, it boils down to making sure muu returns what the
// local implementation does at runtime.

namespace
{
	#if MUU_HAS_QUADMATH
	float128_t absq(float128_t x) noexcept { return ::fabsq(x); }
	#endif

	template <typename T, typename Return = T>
	using muu_func_ptr = Return(MUU_VECTORCALL*)(T) noexcept;

	template <typename T, typename Return = T>
	using std_func_ptr = Return(*)(T) noexcept;

	template <typename T, typename Return = T>
	void math_test_runtime(muu_func_ptr<T, Return> muu_func, std_func_ptr<T, Return> std_func, T start, T end, size_t subdivs) noexcept
	{
		using val_type = impl::highest_ranked<long double, T>;
		for (size_t i = 0; i <= subdivs; i++) // +1 for inclusive end
		{
			const auto val = muu::lerp(
				static_cast<val_type>(start),
				static_cast<val_type>(end),
				(static_cast<val_type>(i) / static_cast<val_type>(subdivs))
			);
			const auto input = static_cast<T>(val);
			const auto muu_output = muu_func(input);
			const auto std_output = std_func(input);
			if (muu_output != std_output)
			{
				INFO("     input: " << print_aligned{ input })
				INFO("muu output: " << print_aligned{ muu_output })
				INFO("std output: " << print_aligned{ std_output })
				FAIL_CHECK("output mismatch!");
				break;
			}
			else
				SUCCEED(); // increment assertion counter
		}
	}

	template <typename T, typename Return = T>
	struct table_value
	{
		T input;
		Return output;
	};

	template <typename T, typename Return, muu_func_ptr<T, Return> Func, typename Start, typename End, size_t Subdivs>
	[[maybe_unused]]
	inline constexpr std::array<table_value<T, Return>, Subdivs + 1u> lookup_table
		= [](muu_func_ptr<T, Return> fn) constexpr noexcept
		{
			std::array<table_value<T, Return>, Subdivs + 1u> table{};  // +1 for inclusive end

			using val_type = impl::highest_ranked<long double, T>;
			for (size_t i = 0; i <= Subdivs; i++)
			{
				const auto val = muu::lerp(
					static_cast<val_type>(Start::value),
					static_cast<val_type>(End::value),
					(static_cast<val_type>(i) / static_cast<val_type>(Subdivs))
				);
				table[i] = { static_cast<T>(val), fn(static_cast<T>(val)) };
			}
			return table;
		}(Func);

	template <typename T, typename Return, muu_func_ptr<T, Return> Func, typename Start, typename End, size_t Subdivs>
	void math_test_constexpr() noexcept
	{
		static constexpr auto& table = lookup_table<T, Return, Func, Start, End, Subdivs>;

		const auto epsilon = static_cast<T>(
			muu::constants<std::conditional_t<MUU_HAS_QUADMATH, T, impl::demote_if_large_float<T>>>::default_epsilon
			* 10ull
		);

		for (size_t i = 0; i <= Subdivs; i++) // +1 for inclusive end
		{
			const auto output = Func(table[i].input);
			if constexpr (is_floating_point<Return>)
			{
				auto eps = epsilon;
				{
					const auto abs_output = muu::abs(output);
					int eps_mult = 1;
					if (abs_output >= T{ 10 })
						eps_mult *= 30;
					if (abs_output >= T{ 100 })
						eps_mult *= 2;
					if (impl::is_small_float_<T> && eps_mult == 1)
						eps_mult *= 2;
					eps *= static_cast<T>(eps_mult);
				}
				if (!approx_equal(output, table[i].output, eps))
				{
					INFO("           input: " << print_aligned{ table[i].input })
					INFO("constexpr output: " << print_aligned{ table[i].output })
					INFO("  runtime output: " << print_aligned{ output })
					INFO("         epsilon: " << print_aligned{ eps })
					FAIL_CHECK("output mismatch!");
					break;
				}
				else
					SUCCEED(); // increment assertion counter
			}
			else
			{
				if (output != table[i].output)
				{
					INFO("           input: " << print_aligned{ table[i].input })
					INFO("constexpr output: " << print_aligned{ table[i].output })
					INFO("  runtime output: " << print_aligned{ output })
					FAIL_CHECK("output mismatch!");
					break;
				}
				else
					SUCCEED(); // increment assertion counter
			}
		}
	}
}

#if MUU_HAS_QUADMATH
	#define FLOAT128_T					float128_t
	#define QUADMATH_CALL(name, val)	MUU_CONCAT(name, q)(val)
#else
	#define FLOAT128_T					void // doesn't matter what this is
	#define QUADMATH_CALL(name, val)	decltype(val){}
#endif

#define MATH_CHECK_STRUCT(name)		MUU_CONCAT(MUU_CONCAT(math_check_,name), MUU_CONCAT(_,__LINE__))

#define MATH_CHECK_STRUCT_INSTANTIATION(type, name, val)\
	template <> struct MATH_CHECK_STRUCT(name)<type> { using T = type; static constexpr T value = T{ val }; };

#define MATH_CHECK_TYPE(type, func, subdivs)												\
	{																						\
		INFO(MUU_MAKE_STRING(type))															\
		using T = type;																		\
		using Return = decltype(muu::func(std::declval<T>()));								\
		using start = MATH_CHECK_STRUCT(start)<T>;											\
		using end = MATH_CHECK_STRUCT(end)<T>;												\
		math_test_runtime<T, Return>(muu::func, MUU_CONCAT(std_, func)<T>, start::value, end::value, subdivs);	\
		if constexpr (build::supports_constexpr_math										\
			&& (!std::is_same_v<muu::half, T> || build::supports_constexpr_half))			\
		{																					\
			math_test_constexpr<T, Return, muu::func, start, end, subdivs>();				\
		}																					\
	}

#define MATH_STD_FUNC_SHIM(func)															\
	template <typename T>																	\
	static auto MUU_CONCAT(std_, func)(T val) noexcept										\
		-> decltype(muu::func(std::declval<T>()))											\
	{																						\
		using Return = decltype(muu::func(std::declval<T>()));								\
		if constexpr (std::is_same_v<T, FLOAT128_T> && MUU_HAS_QUADMATH)					\
			return QUADMATH_CALL(func, val);												\
		else																				\
			return static_cast<Return>(														\
				::std::func(static_cast<impl::clamp_to_standard_float<T>>(val))				\
			);																				\
	}

#define MATH_CHECKS(func, start_val, end_val, subdivs)							\
	namespace																	\
	{																			\
		MATH_STD_FUNC_SHIM(func)												\
		template <typename T> struct MATH_CHECK_STRUCT(start);					\
		FOREACH_FLOAT_VARARGS(MATH_CHECK_STRUCT_INSTANTIATION, start, start_val)\
		template <typename T> struct MATH_CHECK_STRUCT(end);					\
		FOREACH_FLOAT_VARARGS(MATH_CHECK_STRUCT_INSTANTIATION, end, end_val)	\
	}																			\
	TEST_CASE("math - " MUU_MAKE_STRING(func))									\
	{																			\
		FOREACH_FLOAT_VARARGS(MATH_CHECK_TYPE, func, subdivs)					\
	}																			\
	static_assert(true)

// these are all named because muu::half literals arent constexpr-friendly on old compilers

MATH_CHECKS(abs,		-constants<T>::one_hundred, constants<T>::one_hundred,	500);
MATH_CHECKS(floor,		-constants<T>::one_hundred, constants<T>::one_hundred,	500);
MATH_CHECKS(ceil,		-constants<T>::one_hundred, constants<T>::one_hundred,	500);
MATH_CHECKS(sqrt,		constants<T>::zero,			constants<T>::one_hundred,	500);
MATH_CHECKS(cos,		-constants<T>::two_pi,		constants<T>::two_pi,		500);
MATH_CHECKS(sin,		-constants<T>::two_pi,		constants<T>::two_pi,		500);
MATH_CHECKS(tan,		-constants<T>::two_pi,		constants<T>::two_pi,		500);
MATH_CHECKS(acos,		-constants<T>::one,			constants<T>::one,			500);
MATH_CHECKS(asin,		-constants<T>::one,			constants<T>::one,			500);
MATH_CHECKS(atan,		-constants<T>::one_hundred,	constants<T>::one_hundred,	500);

TEST_CASE("math - normalize_angle")
{
	#define CHECK_NORMALIZE_ANGLE(value, expected) \
		CHECK_AND_STATIC_ASSERT(muu::approx_equal(normalize_angle(value), expected))

	// "Normalizes angles, wrapping their values to the range [0.0, 2 * pi)."

	CHECK_NORMALIZE_ANGLE(constants<double>::pi * -4.0, 0.0);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * -3.5, constants<double>::pi * 0.5);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * -3.0, constants<double>::pi);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * -2.5, constants<double>::pi * 1.5);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * -2.0, 0.0);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * -1.5, constants<double>::pi * 0.5);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * -1.0, constants<double>::pi);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * -0.5, constants<double>::pi * 1.5);
	CHECK_NORMALIZE_ANGLE(0.0, 0.0);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * 0.5, constants<double>::pi * 0.5);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * 1.0, constants<double>::pi);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * 1.5, constants<double>::pi * 1.5);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * 2.0, 0.0);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * 2.5, constants<double>::pi * 0.5);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * 3.0, constants<double>::pi);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * 3.5, constants<double>::pi * 1.5);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * 4.0, 0.0);
}

TEST_CASE("math - normalize_angle_signed")
{
	#undef CHECK_NORMALIZE_ANGLE
	#define CHECK_NORMALIZE_ANGLE(value, expected) \
			CHECK_AND_STATIC_ASSERT(muu::approx_equal(normalize_angle_signed(value), expected))

	// "Normalizes angles, wrapping their values to the range [-pi, pi)."

	CHECK_NORMALIZE_ANGLE(constants<double>::pi * -4.0, 0.0);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * -3.5, constants<double>::pi * 0.5);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * -3.0, -constants<double>::pi);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * -2.5, -constants<double>::pi * 0.5);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * -2.0, 0.0);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * -1.5, constants<double>::pi * 0.5);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * -1.0, -constants<double>::pi);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * -0.5, -constants<double>::pi * 0.5);
	CHECK_NORMALIZE_ANGLE(0.0, 0.0);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * 0.5, constants<double>::pi * 0.5);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * 1.0, -constants<double>::pi);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * 1.5, -constants<double>::pi * 0.5);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * 2.0, 0.0);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * 2.5, constants<double>::pi * 0.5);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * 3.0, -constants<double>::pi);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * 3.5, -constants<double>::pi * 0.5);
	CHECK_NORMALIZE_ANGLE(constants<double>::pi * 4.0, 0.0);
}
