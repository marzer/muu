// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

#include "impl/core_meta.h"
#include "impl/core_utils.h"
#include "iterators.h"
#include "impl/header_start.h"

namespace muu
{
	/// \brief A read-only LegacyForwardIterator used for iteration through #muu::integer_range.
	/// \ingroup core
	template <typename T>
	struct MUU_TRIVIAL_ABI integer_range_iterator
	{
		using value_type		= T;
		using size_type			= muu::largest<size_t, muu::make_unsigned<T>>;
		using difference_type	= muu::largest<ptrdiff_t, muu::make_signed<T>>;
		using iterator_category = std::input_iterator_tag;
		using reference			= value_type;
		// using pointer = ? ;

		value_type value;

		constexpr integer_range_iterator& operator++() noexcept // pre
		{
			value++;
			return *this;
		}

		constexpr integer_range_iterator operator++(int) noexcept // post
		{
			return integer_range_iterator{ value++ };
		}

		constexpr integer_range_iterator& operator--() noexcept // pre
		{
			value--;
			return *this;
		}

		constexpr integer_range_iterator operator--(int) noexcept // post
		{
			return integer_range_iterator{ value-- };
		}

		constexpr integer_range_iterator& operator+=(size_type offset) noexcept
		{
			value = static_cast<value_type>(static_cast<size_type>(value) + offset);
			return *this;
		}

		constexpr integer_range_iterator& operator-=(size_type offset) noexcept
		{
			value = static_cast<value_type>(static_cast<size_type>(value) - offset);
			return *this;
		}

		constexpr integer_range_iterator& operator+=(difference_type offset) noexcept
		{
			value = static_cast<value_type>(static_cast<difference_type>(value) + offset);
			return *this;
		}

		constexpr integer_range_iterator& operator-=(difference_type offset) noexcept
		{
			value = static_cast<value_type>(static_cast<difference_type>(value) - offset);
			return *this;
		}

		MUU_PURE_INLINE_GETTER
		constexpr value_type operator*() const noexcept
		{
			return value;
		}

		MUU_PURE_INLINE_GETTER
		friend constexpr bool operator==(const integer_range_iterator& lhs, const integer_range_iterator& rhs) noexcept
		{
			return lhs.value == rhs.value;
		}

		MUU_PURE_INLINE_GETTER
		friend constexpr bool operator!=(const integer_range_iterator& lhs, const integer_range_iterator& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		MUU_PURE_INLINE_GETTER
		friend constexpr difference_type operator-(const integer_range_iterator& lhs,
												   const integer_range_iterator& rhs) noexcept
		{
			return static_cast<difference_type>(lhs.value) - static_cast<difference_type>(rhs.value);
		}
	};

	/// \brief A half-open integer range.
	/// \ingroup core
	template <typename T>
	struct MUU_TRIVIAL_ABI integer_range
	{
		static_assert(!muu::is_cvref<T>, "Value type cannot be const, volatile or ref-qualified.");
		static_assert(muu::is_integer<T>, "Value type must be an integer.");

		/// \brief The range's value type.
		using value_type = T;

		/// \brief An unsigned integer type big enough to store the result of `e - s`.
		using size_type = muu::largest<size_t, muu::make_unsigned<T>>;

		/// \brief The start of the range (inclusive).
		value_type s;

		/// \brief The end of the range (exclusive).
		/// \warning This class does not support reverse ranges; #e should always be greater than or equal to #s.
		value_type e;

		/// \brief A read-only LegacyForwardIterator for iterating over the range.
		using iterator = integer_range_iterator<value_type>;

		/// \brief Default constructor. Does not initialize the range.
		MUU_NODISCARD_CTOR
		integer_range() noexcept = default;

		/// \brief Constructs a range from an end value.
		MUU_NODISCARD_CTOR
		constexpr integer_range(value_type end_) noexcept //
			: s{},
			  e{ end_ }
		{
			if constexpr (muu::is_signed<value_type>)
				MUU_CONSTEXPR_SAFE_ASSERT(e >= s);
		}

		/// \brief Constructs a range from start and end values.
		/// \warning This class does not support reverse ranges; `end_` should always be greater than or equal to `start_`.
		MUU_NODISCARD_CTOR
		constexpr integer_range(value_type start_, value_type end_) noexcept //
			: s{ start_ },
			  e{ end_ }
		{
			MUU_CONSTEXPR_SAFE_ASSERT(e >= s);
		}

		/// \brief Returns true if the range is not empty (i.e. `e > s`).
		MUU_PURE_INLINE_GETTER
		explicit constexpr operator bool() const noexcept
		{
			return e > s;
		}

		/// \brief Returns true if the range is empty (i.e. `e == s`).
		MUU_PURE_INLINE_GETTER
		constexpr bool empty() const noexcept
		{
			return e == s;
		}

		/// \brief Returns the size of the range (i.e. `e - s`).
		MUU_PURE_GETTER
		constexpr size_type size() const noexcept
		{
			if constexpr (muu::is_signed<value_type>)
			{
				using difference_type = largest<ptrdiff_t, make_signed<T>>;

				return static_cast<size_type>(static_cast<difference_type>(e) - static_cast<difference_type>(s));
			}
			else
				return static_cast<size_type>(e - s);
		}

		/// \brief Returns the first value in the range (i.e. `s`).
		MUU_PURE_INLINE_GETTER
		constexpr value_type first() const noexcept
		{
			return s;
		}

		/// \brief Returns the last value in the range (i.e. `e - 1`).
		MUU_PURE_GETTER
		constexpr value_type last() const noexcept
		{
			MUU_CONSTEXPR_SAFE_ASSERT(e > s);

			return static_cast<value_type>(e - value_type{ 1 });
		}

		/// \brief Returns true if the range contains a specific value.
		MUU_PURE_GETTER
		constexpr bool contains(value_type val) const noexcept
		{
			return val >= s && val < e;
		}

		/// \brief Returns true if the range contains all the values in another range.
		MUU_PURE_GETTER
		constexpr bool contains(const integer_range& other) const noexcept
		{
			return other.s >= s && other.e <= e;
		}

		/// \brief Returns true if the range is adjacent to (but not overlapping) another range.
		MUU_PURE_GETTER
		constexpr bool adjacent(const integer_range& other) const noexcept
		{
			return s == other.e || other.s == e;
		}

		/// \brief Returns true if the range intersects another range.
		MUU_PURE_GETTER
		constexpr bool intersects(const integer_range& other) const noexcept
		{
			return !(s >= other.e || other.s >= e);
		}

		/// \brief Returns true if the range intersects or is adjacent to another range.
		MUU_PURE_GETTER
		constexpr bool intersects_or_adjacent(const integer_range& other) const noexcept
		{
			return !(s > other.e || other.s > e);
		}

		/// \brief Returns the union of this range and another range.
		MUU_PURE_GETTER
		constexpr integer_range union_with(const integer_range& other) const noexcept
		{
			return integer_range{ muu::min(s, other.s), muu::max(e, other.e) };
		}

		/// \name Equality
		/// @{

		/// \brief Returns true if two ranges are equal.
		MUU_PURE_GETTER
		friend constexpr bool operator==(const integer_range& lhs, const integer_range& rhs) noexcept
		{
			return lhs.s == rhs.s && lhs.e == rhs.e;
		}

		/// \brief Returns true if two ranges are not equal.
		MUU_PURE_GETTER
		friend constexpr bool operator!=(const integer_range& lhs, const integer_range& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		/// @}

		/// \name Iterators
		/// @{

		/// \brief Returns an iterator to the start of the range.
		MUU_PURE_INLINE_GETTER
		constexpr iterator begin() const noexcept
		{
			return iterator{ s };
		}

		/// \brief Returns an iterator to the end of the range.
		MUU_PURE_INLINE_GETTER
		constexpr iterator end() const noexcept
		{
			return iterator{ e };
		}

		/// @}

		/// \name Iterators (ADL)
		/// @{

		/// \brief Returns an iterator to the start of the range.
		MUU_PURE_INLINE_GETTER
		friend constexpr iterator begin(const integer_range& range) noexcept
		{
			return iterator{ range.s };
		}

		/// \brief Returns an iterator to the end of the range.
		MUU_PURE_INLINE_GETTER
		friend constexpr iterator end(const integer_range& range) noexcept
		{
			return iterator{ range.e };
		}

		/// @}
	};

	/// \cond

	MUU_CONSTRAINED_TEMPLATE(is_integral<T>, typename T)
	integer_range(T)->integer_range<muu::remove_enum<muu::remove_cvref<T>>>;

	MUU_CONSTRAINED_TEMPLATE((all_integral<T, U>), typename T, typename U)
	integer_range(T, U)
		->integer_range<
			impl::highest_ranked<muu::remove_enum<muu::remove_cvref<T>>, muu::remove_enum<muu::remove_cvref<U>>>>;

	/// \endcond

	/// \brief	Convenience alias for `integer_range<size_t>`.
	/// \ingroup	core
	using index_range = integer_range<size_t>;

	/// \brief	Convenience alias for `integer_range<uintptr_t>`.
	/// \ingroup	core
	using uintptr_range = integer_range<uintptr_t>;

	/// \brief	Convenience alias for `integer_range<intptr_t>`.
	/// \ingroup	core
	using intptr_range = integer_range<intptr_t>;
}

#include "impl/header_end.h"
