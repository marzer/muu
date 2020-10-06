// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::accumulator.

#pragma once
#include "../muu/core.h"
#include "../muu/compressed_pair.h"

MUU_PUSH_WARNINGS
MUU_DISABLE_SPAM_WARNINGS

MUU_NAMESPACE_START
{
	/// \brief	Determines min, max and sum of an interderminate number of values.
	/// \ingroup building_blocks
	///
	/// \detail For integral types the accumulator is a simple bookkeeping helper, but for floating-point
	/// 		types the default implementation uses Kahan summation to reduce numerical error.
	///
	/// \tparam	ValueType		Type being accumulated.
	/// \tparam	Impl			Implementation type.
	///
	/// \see [Kahan summation algorithm (wikipedia)](https://en.wikipedia.org/wiki/Kahan_summation_algorithm)
	template <typename ValueType, typename Impl
		#ifdef DOXYGEN
			= impl::default_accumulator<ValueType>::type
		#endif
	>
	class accumulator
	{
		static_assert(
			!is_cvref<ValueType>,
			"Accumulated type cannot be const, volatile, or a reference"
		);
		static_assert(
			!is_cvref<Impl>,
			"Implementation type cannot be const, volatile, or a reference"
		);
		static_assert(
			std::is_default_constructible_v<Impl>,
			"Implementation type must be default-constructible"
		);
		static_assert(
			!build::has_exceptions || std::is_trivially_destructible_v<Impl> || std::is_nothrow_destructible_v<Impl>,
			"Implementation type must be nothrow-destructible"
		);

		public:

			/// \brief	The type being accumulated.
			using value_type = ValueType;

			/// \brief	`value_type` or `const value_type&`, depending on size, triviality, etc.
			using value_param = impl::maybe_pass_readonly_by_value<ValueType>;

		private:
			compressed_pair<Impl, size_t> impl_and_count{};

		public:

			/// \brief	Constructs an empty accumulator.
			MUU_NODISCARD_CTOR
			constexpr accumulator()
				noexcept(std::is_nothrow_default_constructible_v<Impl>)
			{}

			/// \brief	Constructs an accumulator with one sample.
			MUU_NODISCARD_CTOR
			constexpr accumulator(const value_type& sample)
				noexcept(std::is_nothrow_default_constructible_v<Impl> && noexcept(std::declval<Impl>().add(sample)))
			{
				add(sample);
			}

			/// \brief	Constructs an accumulator with an initial range of samples.
			///
			/// \tparam	Iter	Iterator type.
			/// \param	begin	The beginning iterator.
			/// \param	end  	The end iterator.
			template <typename Iter>
			MUU_NODISCARD_CTOR
			constexpr accumulator(Iter begin, Iter end)
				noexcept(noexcept(std::declval<accumulator>().add(begin, end)))
			{
				add(begin, end);
			}

			/// \brief	Returns the number of samples added to the accumulator.
			[[nodiscard]]
			MUU_ALWAYS_INLINE
			constexpr size_t sample_count() const noexcept
			{
				return impl_and_count.second();
			}

			/// \brief	Returns true if no samples have been added to the accumulator.
			[[nodiscard]]
			MUU_ALWAYS_INLINE
			constexpr bool empty() const noexcept
			{
				return impl_and_count.second() == size_t{};
			}

			/// \brief	Returns the minimum value added to the accumulator.
			[[nodiscard]]
			MUU_ALWAYS_INLINE
			constexpr decltype(auto) (min)() const
				noexcept(noexcept(std::declval<Impl>().min()))
			{
				return impl_and_count.first().min();
			}

			/// \brief	Returns the maximum value added to the accumulator.
			[[nodiscard]]
			MUU_ALWAYS_INLINE
			constexpr decltype(auto) (max)() const
				noexcept(noexcept(std::declval<Impl>().max()))
			{
				return impl_and_count.first().max();
			}

			/// \brief	Returns the sum of all values added to the accumulator.
			[[nodiscard]]
			MUU_ALWAYS_INLINE
			constexpr decltype(auto) sum() const
				noexcept(noexcept(std::declval<Impl>().sum()))
			{
				return impl_and_count.first().sum();
			}

			/// \brief	Adds a new sample to the accumulator.
			///
			/// \param	sample	The sample to add.
			///
			/// \return	A reference to the accumulator.
			constexpr accumulator& MUU_VECTORCALL add(value_param sample)
				noexcept(noexcept(std::declval<Impl>().start(sample)) && noexcept(std::declval<Impl>().add(sample)))
			{
				if constexpr (is_floating_point<value_type>)
				{
					MUU_ASSERT(!infinity_or_nan(sample));
				}

				if MUU_UNLIKELY(!impl_and_count.second()++)
					impl_and_count.first().start(sample);
				else
					impl_and_count.first().add(sample);
				return *this;
			}

			/// \brief	Adds a range of values to the accumulator.
			///
			/// \tparam	Iter	Iterator type.
			/// \param	begin	The beginning iterator.
			/// \param	end  	The end iterator.
			///
			/// \return	A reference to the accumulator.
			template <typename Iter>
			constexpr accumulator& add(Iter begin, Iter end)
				noexcept(noexcept(std::declval<accumulator>().add(value_type{})))
			{
				while (begin != end)
					add(static_cast<value_type>(*(begin++)));
				return *this;
			}

			/// \brief	Adds the entire sample set of another Accumulator to this one.
			///
			/// \param	other	The other accumulator.
			///
			/// \return	A reference to this Accumulator.
			constexpr accumulator& add(const accumulator& other) noexcept
			{
				if (!other.empty())
				{
					if (empty())
					{
						impl_and_count.first().add(other.impl_and_count.first());
						impl_and_count.second() += other.impl_and_count.second();
					}
					else
					{
						impl_and_count.first() = other.impl_and_count.first();
						impl_and_count.second() = other.impl_and_count.second();
					}
				}
				return *this;
			}

			/// \brief	Adds a new sample to the accumulator.
			///
			/// \param	sample	The sample to add.
			///
			/// \return	A reference to the accumulator.
			MUU_ALWAYS_INLINE
			constexpr accumulator& MUU_VECTORCALL operator() (value_param sample)
				noexcept(noexcept(std::declval<accumulator>().add(sample)))
			{
				return add(sample);
			}

			/// \brief	Adds the entire sample set of another Accumulator to this one.
			///
			/// \param	other	The other accumulator.
			///
			/// \return	A reference to this accumulator.
			MUU_ALWAYS_INLINE
			constexpr accumulator& operator() (const accumulator& other)
				noexcept(noexcept(std::declval<accumulator>().add(other)))
			{
				return add(other);
			}
	};

	namespace impl
	{
		template <typename ValueType>
		struct basic_accumulator
		{
			using value_type = ValueType;
			using value_param = impl::maybe_pass_readonly_by_value<ValueType>;
			using sum_type = std::conditional_t<
				is_integral<ValueType>,
				std::conditional_t<
					is_signed<ValueType>,
					largest<ValueType, int32_t>,
					largest<ValueType, uint32_t>
				>,
				ValueType
			>;

			value_type min_ = {}, max_ = {};
			sum_type sum_ = {};

			constexpr void MUU_VECTORCALL start(value_param sample) noexcept
			{
				min_ = max_ = sample;
				sum_= static_cast<sum_type>(sample);
			}

			constexpr void MUU_VECTORCALL add(value_param sample) noexcept
			{
				using muu::min;
				using muu::max;
				min_ = (min)(sample, min_);
				max_ = (max)(sample, max_);
				sum_ += static_cast<sum_type>(sample);
			}

			constexpr void add(const basic_accumulator& other) noexcept
			{
				using muu::min;
				using muu::max;
				min_ = (min)(other.min_, min_);
				max_ = (max)(other.max_, max_);
				sum_ += other.sum_;
			}

			[[nodiscard]]
			MUU_ALWAYS_INLINE
			constexpr value_type(min)() const noexcept
			{
				return min_;
			}

			[[nodiscard]]
			MUU_ALWAYS_INLINE
			constexpr value_type(max)() const noexcept
			{
				return max_;
			}

			[[nodiscard]]
			MUU_ALWAYS_INLINE
			constexpr sum_type sum() const noexcept
			{
				return sum_;
			}
		};

		MUU_PRAGMA_MSVC(float_control(precise, on, push))
		MUU_PRAGMA_CLANG_GE(11, "float_control(precise, on, push)")
		MUU_PRAGMA_GCC("GCC push_options")
		MUU_PRAGMA_GCC("GCC optimize (\"-fno-fast-math\")")

		template <typename ValueType>
		struct kahan_accumulator // https://en.wikipedia.org/wiki/Kahan_summation_algorithm#Further_enhancements
		{
			using value_type = ValueType;
			using value_param = impl::maybe_pass_readonly_by_value<ValueType>;
			using sum_type = impl::highest_ranked<ValueType, float>;

			static_assert(
				is_floating_point<value_type>,
				"Kahan summation only makes sense with float types"
			);
			value_type min_ = {}, max_ = {};
			sum_type sum_ = {}, correction_ = {};

			constexpr void MUU_VECTORCALL start(value_param sample) noexcept
			{
				min_ = max_ = sample;
				sum_ = static_cast<sum_type>(sample);
			}

			MUU_PRAGMA_CLANG_LT(11, "clang optimize off")

			constexpr void MUU_VECTORCALL kahan_add(impl::maybe_pass_readonly_by_value<sum_type> sample) noexcept
			{
				MUU_PRAGMA_CLANG_GE(11, "clang fp reassociate(off)")
				MUU_PRAGMA_CLANG_LT(11, "clang fp contract(on)")

				const auto t = sum_ + sample;
				if (muu::abs(sum_) >= muu::abs(sample))
					correction_ += (sum_ - t) + sample;
				else
					correction_ += (sample - t) + sum_;
				sum_ = t;
			}

			MUU_PRAGMA_CLANG_LT(11, "clang optimize on")

			constexpr void MUU_VECTORCALL add(value_param sample) noexcept
			{
				min_ = (muu::min)(sample, min_);
				max_ = (muu::max)(sample, max_);
				kahan_add(static_cast<sum_type>(sample));
			}

			constexpr void add(const kahan_accumulator& other) noexcept
			{
				min_ = (muu::min)(other.min_, min_);
				max_ = (muu::max)(other.max_, max_);
				kahan_add(other.sum);
			}

			[[nodiscard]]
			MUU_ALWAYS_INLINE
			constexpr value_type(min)() const noexcept
			{
				return min_;
			}

			[[nodiscard]]
			MUU_ALWAYS_INLINE
			constexpr value_type(max)() const noexcept
			{
				return max_;
			}

			[[nodiscard]]
			MUU_ALWAYS_INLINE
			constexpr value_type sum() const noexcept
			{
				return static_cast<value_type>(sum_ + correction_);
			}
		};

		MUU_PRAGMA_GCC("GCC pop_options")
		MUU_PRAGMA_CLANG_GE(11, "float_control(pop)")
		MUU_PRAGMA_MSVC(float_control(pop))
	}
}
MUU_NAMESPACE_END

MUU_POP_WARNINGS // MUU_DISABLE_SPAM_WARNINGS
