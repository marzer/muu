// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::integral_range.

#include "iterators.h"
#include "meta.h"
#include "impl/core_utils.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;

namespace muu
{
	/// \cond
	namespace impl
	{
		template <typename T, bool = std::is_pointer_v<T>>
		struct integral_range_size_type_
		{
			using type = size_t;
		};
		template <typename T>
		struct integral_range_size_type_<T, false /* not a pointer */>
		{
			using type = muu::largest<size_t, muu::make_unsigned<muu::remove_enum<T>>>;
		};
		template <typename T>
		using integral_range_size_type = typename integral_range_size_type_<T>::type;

		template <typename T, bool = std::is_pointer_v<T>>
		struct integral_range_difference_type_
		{
			using type = ptrdiff_t;
		};
		template <typename T>
		struct integral_range_difference_type_<T, false /* not a pointer */>
		{
			using type = muu::largest<ptrdiff_t, muu::make_signed<muu::remove_enum<T>>>;
		};
		template <typename T>
		using integral_range_difference_type = typename integral_range_difference_type_<T>::type;

		template <typename T>
		struct MUU_TRIVIAL_ABI integral_range_iterator
		{
		  public:
			using value_type		= T;
			using size_type			= integral_range_size_type<T>;
			using difference_type	= integral_range_difference_type<T>;
			using iterator_category = std::input_iterator_tag;
			using reference			= value_type;
			// using pointer = ? ;

			value_type val_;

			constexpr integral_range_iterator& operator++() noexcept // pre
			{
				val_++;
				return *this;
			}

			constexpr integral_range_iterator operator++(int) noexcept // post
			{
				return integral_range_iterator{ val_++ };
			}

			constexpr integral_range_iterator& operator--() noexcept // pre
			{
				val_--;
				return *this;
			}

			constexpr integral_range_iterator operator--(int) noexcept // post
			{
				return integral_range_iterator{ val_-- };
			}

			constexpr integral_range_iterator& operator+=(size_type offset) noexcept
			{
				if constexpr (std::is_pointer_v<T>)
					val_ = static_cast<value_type>(val_ + offset);
				else
					val_ = static_cast<value_type>(static_cast<size_type>(val_) + offset);

				return *this;
			}

			constexpr integral_range_iterator& operator-=(size_type offset) noexcept
			{
				if constexpr (std::is_pointer_v<T>)
					val_ = static_cast<value_type>(val_ - offset);
				else
					val_ = static_cast<value_type>(static_cast<size_type>(val_) - offset);

				return *this;
			}

			constexpr integral_range_iterator& operator+=(difference_type offset) noexcept
			{
				if constexpr (std::is_pointer_v<T>)
					val_ = static_cast<value_type>(val_ + offset);
				else
					val_ = static_cast<value_type>(static_cast<difference_type>(val_) + offset);

				return *this;
			}

			constexpr integral_range_iterator& operator-=(difference_type offset) noexcept
			{
				if constexpr (std::is_pointer_v<T>)
					val_ = static_cast<value_type>(val_ - offset);
				else
					val_ = static_cast<value_type>(static_cast<difference_type>(val_) - offset);

				return *this;
			}

			MUU_PURE_INLINE_GETTER
			constexpr value_type operator*() const noexcept
			{
				return val_;
			}

			MUU_PURE_INLINE_GETTER
			friend constexpr bool operator==(const integral_range_iterator& lhs,
											 const integral_range_iterator& rhs) noexcept
			{
				return lhs.val_ == rhs.val_;
			}

			MUU_PURE_INLINE_GETTER
			friend constexpr bool operator!=(const integral_range_iterator& lhs,
											 const integral_range_iterator& rhs) noexcept
			{
				return !(lhs == rhs);
			}

			MUU_PURE_INLINE_GETTER
			friend constexpr difference_type operator-(const integral_range_iterator& lhs,
													   const integral_range_iterator& rhs) noexcept
			{
				if constexpr (muu::is_unsigned<T>)
				{
					if (rhs.val_ > lhs.val_)
						return -static_cast<difference_type>(static_cast<size_type>(rhs.val_)
															 - static_cast<size_type>(lhs.val_));
				}

				return static_cast<difference_type>(lhs.val_) - static_cast<difference_type>(rhs.val_);
			}
		};
	}
	/// \endcond

	/// \brief A half-open range of integral values (ints, enums, pointers).
	/// \ingroup core
	template <typename T>
	struct MUU_TRIVIAL_ABI integral_range
	{
		static_assert(!muu::is_cvref<T>, "Value type may not be const, volatile or ref-qualified.");
		static_assert(muu::is_integer<T> || std::is_pointer_v<T>, "Value type must be an integer or object pointer.");
		static_assert(!std::is_function_v<std::remove_pointer_t<T>>, "Value type may not be a function pointer.");

		/// \brief The range's value type.
		using value_type = T;

		/// \brief An unsigned integer type big enough to store the result of `e - s`.
		using size_type = POXY_IMPLEMENTATION_DETAIL(impl::integral_range_size_type<T>);

		/// \brief The start of the range (inclusive).
		value_type s;

		/// \brief The end of the range (exclusive).
		/// \warning This class does not support reverse ranges; #e should always be greater than or equal to #s.
		value_type e;

		/// \brief A read-only LegacyForwardIterator for iterating over the range.
		using iterator = POXY_IMPLEMENTATION_DETAIL(impl::integral_range_iterator<T>);

		/// \brief Default constructor. Does not initialize the range.
		MUU_NODISCARD_CTOR
		integral_range() noexcept = default;

		/// \brief Constructs a range over `[0, end)`.
		/// \warning This class does not support reverse ranges; `end_` should always be greater than or equal to `0`.
		MUU_NODISCARD_CTOR
		constexpr integral_range(value_type end_) noexcept //
			: s{},
			  e{ end_ }
		{
			if constexpr (muu::is_signed<value_type> || std::is_pointer_v<value_type>)
				MUU_CONSTEXPR_SAFE_ASSERT(e >= s);
		}

		/// \brief Constructs a range over `[start, end)`.
		/// \warning This class does not support reverse ranges; `end_` should always be greater than or equal to `start_`.
		MUU_NODISCARD_CTOR
		constexpr integral_range(value_type start_, value_type end_) noexcept //
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
			if constexpr (std::is_pointer_v<value_type>)
				return static_cast<size_type>(e - s);
			else if constexpr (muu::is_signed<value_type>)
			{
				using difference_type = impl::integral_range_difference_type<T>;

				return static_cast<size_type>(static_cast<difference_type>(e) - static_cast<difference_type>(s));
			}
			else
				return static_cast<size_type>(e) - static_cast<size_type>(s);
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

			if constexpr (std::is_pointer_v<value_type>)
				return e - 1;
			else
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
		constexpr bool contains(const integral_range& other) const noexcept
		{
			return other.s >= s && other.e <= e;
		}

		/// \brief Returns true if the range is adjacent to (but not overlapping) another range.
		MUU_PURE_GETTER
		constexpr bool adjacent(const integral_range& other) const noexcept
		{
			return s == other.e || other.s == e;
		}

		/// \brief Returns true if the range intersects another range.
		MUU_PURE_GETTER
		constexpr bool intersects(const integral_range& other) const noexcept
		{
			return e > other.s && other.e > s;
		}

		/// \brief Returns true if the range intersects or is adjacent to another range.
		MUU_PURE_GETTER
		constexpr bool intersects_or_adjacent(const integral_range& other) const noexcept
		{
			return e >= other.s && other.e >= s;
		}

		/// \brief Returns the union of this range and another range.
		MUU_PURE_GETTER
		constexpr integral_range union_with(const integral_range& other) const noexcept
		{
			return integral_range{ muu::min(s, other.s), muu::max(e, other.e) };
		}

		/// \name Equality
		/// @{

		/// \brief Returns true if two ranges are equal.
		MUU_PURE_GETTER
		friend constexpr bool operator==(const integral_range& lhs, const integral_range& rhs) noexcept
		{
			return lhs.s == rhs.s && lhs.e == rhs.e;
		}

		/// \brief Returns true if two ranges are not equal.
		MUU_PURE_GETTER
		friend constexpr bool operator!=(const integral_range& lhs, const integral_range& rhs) noexcept
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
		friend constexpr iterator begin(const integral_range& range) noexcept
		{
			return iterator{ range.s };
		}

		/// \brief Returns an iterator to the end of the range.
		MUU_PURE_INLINE_GETTER
		friend constexpr iterator end(const integral_range& range) noexcept
		{
			return iterator{ range.e };
		}

		/// @}
	};

	/// \cond

	MUU_CONSTRAINED_TEMPLATE(is_integral<T> || std::is_pointer_v<T>, typename T)
	integral_range(T)->integral_range<muu::remove_enum<muu::remove_cvref<T>>>;

	MUU_CONSTRAINED_TEMPLATE(std::is_pointer_v<T>, typename T)
	integral_range(T, T)->integral_range<muu::remove_cvref<T>>;

	MUU_CONSTRAINED_TEMPLATE((all_integral<T, U>), typename T, typename U)
	integral_range(T, U)
		->integral_range<
			impl::highest_ranked<muu::remove_enum<muu::remove_cvref<T>>, muu::remove_enum<muu::remove_cvref<U>>>>;

	/// \endcond

	/// \brief	Convenience alias for `integral_range<size_t>`.
	/// \ingroup	core
	using index_range = integral_range<size_t>;

	/// \brief	Convenience alias for `integral_range<uintptr_t>`.
	/// \ingroup	core
	using uintptr_range = integral_range<uintptr_t>;

	/// \brief	Convenience alias for `integral_range<intptr_t>`.
	/// \ingroup	core
	using intptr_range = integral_range<intptr_t>;

	/// \brief	Convenience alias for `integral_range<std::byte*>`.
	/// \ingroup	core
	using byteptr_range = integral_range<std::byte*>;
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
