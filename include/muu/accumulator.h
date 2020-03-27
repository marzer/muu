// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::accumulator.
#pragma once
#include "../muu/common.h"

namespace muu::impl
{
	template <typename T>
	struct accumulator;
}

namespace muu
{
	/// \brief	Statefully accumulates (adds) an indeterminate range of values.
	/// 
	/// \detail For integral types the accumulator is a simple bookkeeping helper, but for floating-point
	/// 		types the default implementation uses Kahan summation to reduce numerical error.
	///
	/// \tparam	T		Type being accumulated.
	/// \tparam	IMPL	Implementation type.
	///
	/// \see [Kahan summation algorithm (wikipedia)](https://en.wikipedia.org/wiki/Kahan_summation_algorithm)
	template <typename T, typename IMPL = impl::accumulator<T>>
	class accumulator final
	{
		static_assert(
			!is_cvref<T>,
			"Accumulated type cannot be const, volatile, or a reference"
		);
		static_assert(
			!is_cvref<IMPL>,
			"Implementation type cannot be const, volatile, or a reference"
		);
		static_assert(
			std::is_default_constructible_v<IMPL>,
			"Implementation type must be default-constructible"
		);
		static_assert(
			!build::has_exceptions || std::is_trivially_destructible_v<IMPL> || std::is_nothrow_destructible_v<IMPL>,
			"Implementation type must be nothrow-destructible"
		);

		public:

			/// \brief	The type being accumulated.
			using value_type = T;

		private:
			size_t count{};
			MUU_NO_UNIQUE_ADDRESS IMPL impl{};

		public:

			/// \brief	Adds a new sample to the accumulator.
			///
			/// \param	sample	The sample to add.
			///
			/// \return	A reference to the accumulator.
			constexpr accumulator& MUU_VECTORCALL add(value_type MUU_VECTORCALL_ARG sample)
				noexcept(noexcept(impl.start(sample)) && noexcept(impl.add(sample)))
			{
				if constexpr (is_floating_point<value_type>)
					MUU_ASSERT(!is_infinity_or_nan(sample));

				if (!count++)
					impl.start(sample);
				else
					impl.add(sample);
				return *this;
			}

			/// \brief	Adds a range of values to the accumulator.
			///
			/// \tparam	IT		Iterator type.
			/// \param	begin	The beginning iterator.
			/// \param	end  	The end iterator.
			///
			/// \return	A reference to the accumulator.
			template <typename IT>
			constexpr accumulator& add(IT begin, IT end)
				noexcept(noexcept(std::declval<accumulator>().add(value_type{})))
			{
				while (begin != end)
					add(*(begin++));
				return *this;
			}

			/// \brief	Constructs an empty accumulator.
			MUU_NODISCARD_CTOR
			constexpr accumulator()
				noexcept(std::is_nothrow_default_constructible_v<IMPL>)
			{}

			/// \brief	Constructs an accumulator with one sample.
			MUU_NODISCARD_CTOR
			constexpr accumulator(const value_type& sample)
				noexcept(std::is_nothrow_default_constructible_v<IMPL> && noexcept(impl.add(sample)))
			{
				add(sample);
			}

			/// \brief	Constructs an accumulator with an initial range of samples.
			///
			/// \tparam	IT		Iterator type.
			/// \param	begin	The beginning iterator.
			/// \param	end  	The end iterator.
			template <typename IT>
			MUU_NODISCARD_CTOR
			constexpr accumulator(IT begin, IT end)
				noexcept(noexcept(std::declval<accumulator>().add(begin, end)))
			{
				add(begin, end);
			}

			/// \brief	Returns the number of samples added to the accumulator.
			[[nodiscard]] MUU_ALWAYS_INLINE
			constexpr size_t sample_count() const noexcept
			{
				return count;
			}

			/// \brief	Returns the sum of all values added to the accumulator.
			[[nodiscard]] MUU_ALWAYS_INLINE
			constexpr decltype(auto) value() const
				noexcept(noexcept(impl.value()))
			{
				return impl.value();
			}

			/// \brief	Returns the sum of all values added to the accumulator.
			[[nodiscard]] MUU_ALWAYS_INLINE
			explicit operator value_type() const
				noexcept(noexcept(impl.value()))
			{
				return impl.value();
			}
	};
}

namespace muu::impl
{
	template <typename value_type>
	struct accumulator
	{
		value_type sum = {};

		MUU_ALWAYS_INLINE
		constexpr void MUU_VECTORCALL start(value_type MUU_VECTORCALL_ARG sample)
			noexcept(noexcept(sum = sample))
		{
			sum = sample;
		}

		MUU_ALWAYS_INLINE
		constexpr void MUU_VECTORCALL add(value_type MUU_VECTORCALL_ARG sample)
			noexcept(noexcept(sum += sample))
		{
			sum += sample;
		}

		[[nodiscard]] MUU_ALWAYS_INLINE
		constexpr value_type value() const noexcept
		{
			return sum;
		}
	};

	MUU_PRAGMA_MSVC(float_control(precise, on, push))

	template <typename value_type, typename sum_type = value_type>
	struct kahan_accumulator // https://en.wikipedia.org/wiki/Kahan_summation_algorithm#Further_enhancements
	{
		static_assert(
			is_floating_point<value_type> && is_floating_point<sum_type>,
			"Kahan summation only makes sense with float types"
		);
		sum_type sum = {}, correction = {};

		MUU_ALWAYS_INLINE
		constexpr void MUU_VECTORCALL start(value_type sample) noexcept
		{
			sum = static_cast<sum_type>(sample);
		}

		constexpr void MUU_VECTORCALL kahan_add(sum_type sample) noexcept
		{
			const auto t = sum + sample;
			if (abs(sum) >= abs(sample))
				correction += (sum - t) + sample;
			else
				correction += (sample - t) + sum;
			sum = t;
		}

		MUU_ALWAYS_INLINE
		constexpr void MUU_VECTORCALL add(value_type sample) noexcept
		{
			kahan_add(static_cast<sum_type>(sample));
		}

		[[nodiscard]] MUU_ALWAYS_INLINE
		constexpr value_type value() const noexcept
		{
			return static_cast<value_type>(sum + correction);
		}
	};

	template <> struct MUU_EMPTY_BASES accumulator<float16>		: kahan_accumulator<float16, float> {};
	template <> struct MUU_EMPTY_BASES accumulator<float>		: kahan_accumulator<float> {};
	template <> struct MUU_EMPTY_BASES accumulator<double>		: kahan_accumulator<double> {};
	template <> struct MUU_EMPTY_BASES accumulator<long double>	: kahan_accumulator<long double> {};

	MUU_PRAGMA_MSVC(float_control(pop))

}
