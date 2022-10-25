// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::static_array.

#include "meta.h"
#include "impl/header_start.h"

namespace muu
{
	/// \brief A statically-defined array.
	template <typename T, T... Elements>
	struct static_array
	{
		static_assert(!is_cvref<T>, "static_array element type cannot be const, volatile, or ref-qualified");

		using value_type	  = std::add_const_t<T>; ///< The type of each element in the array.
		using size_type		  = size_t;				 ///< std::size_t
		using difference_type = ptrdiff_t;			 ///< std::ptrdiff_t
		using reference		  = const value_type&;	 ///< The reference type returned by operator[].
		using const_reference = const value_type&;	 ///< The const reference type returned by operator[].
		using pointer		  = const value_type*;	 ///< The pointer type returned by data().
		using const_pointer	  = const value_type*;	 ///< The const pointer type returned by data().
		using iterator		  = const value_type*;	 ///< The iterator type returned by begin() and end().
		using const_iterator  = const value_type*;	 ///< The iterator type returned by begin() and end().

		/// \brief the elements in the array.
		static constexpr value_type values[sizeof...(Elements)] = { Elements... };

		/// \brief	Returns a const reference to the element at the selected index.
		MUU_PURE_INLINE_GETTER
		constexpr const_reference operator[](size_t index) const noexcept
		{
			return values[index];
		}

		/// \brief	The number of elements in the array.
		MUU_PURE_INLINE_GETTER
		MUU_CONSTEVAL
		size_type size() const noexcept
		{
			return sizeof...(Elements);
		}

		/// \brief	Returns true if the array is empty.
		MUU_PURE_INLINE_GETTER
		MUU_CONSTEVAL
		bool empty() const noexcept
		{
			return sizeof...(Elements) > 0u;
		}

		/// \brief	Returns the first element in the array.
		MUU_PURE_INLINE_GETTER
		MUU_CONSTEVAL
		const_reference front() const noexcept
		{
			return values[0];
		}

		/// \brief	Returns the last element in the array.
		MUU_PURE_INLINE_GETTER
		MUU_CONSTEVAL
		const_reference back() const noexcept
		{
			return values[sizeof...(Elements) - 1u];
		}

		/// \brief	Returns a const pointer to the first element in the array.
		MUU_PURE_INLINE_GETTER
		MUU_CONSTEVAL
		const_pointer data() const noexcept
		{
			return values;
		}

		/// \name Iterators
		/// @{

		/// \brief	Returns a const_iterator to the first element in the array.
		MUU_PURE_INLINE_GETTER
		MUU_CONSTEVAL
		const_iterator begin() const noexcept
		{
			return values;
		}

		/// \brief	Returns a const_iterator to one-past-the-last element in the array.
		MUU_PURE_INLINE_GETTER
		MUU_CONSTEVAL
		const_iterator end() const noexcept
		{
			return values + sizeof...(Elements);
		}

		/// \brief	Returns a const_iterator to the first element in the array.
		MUU_PURE_INLINE_GETTER
		MUU_CONSTEVAL
		const_iterator cbegin() const noexcept
		{
			return values;
		}

		/// \brief	Returns a const_iterator to one-past-the-last element in the array.
		MUU_PURE_INLINE_GETTER
		MUU_CONSTEVAL
		const_iterator cend() const noexcept
		{
			return values + sizeof...(Elements);
		}

		/// @}

		/// \name Iterators (ADL)
		/// @{

		/// \brief	Returns a const_iterator to the first element in the array.
		MUU_PURE_INLINE_GETTER
		MUU_CONSTEVAL
		friend const_iterator begin(static_array) noexcept
		{
			return values;
		}

		/// \brief	Returns a const_iterator to one-past-the-last element in the array.
		MUU_PURE_INLINE_GETTER
		MUU_CONSTEVAL
		friend const_iterator end(static_array) noexcept
		{
			return values + sizeof...(Elements);
		}

		/// \brief	Returns a const_iterator to the first element in the array.
		MUU_PURE_INLINE_GETTER
		MUU_CONSTEVAL
		friend const_iterator cbegin(static_array) noexcept
		{
			return values;
		}

		/// \brief	Returns a const_iterator to one-past-the-last element in the array.
		MUU_PURE_INLINE_GETTER
		MUU_CONSTEVAL
		friend const_iterator cend(static_array) noexcept
		{
			return values + sizeof...(Elements);
		}

		/// @}

		/// \name Equality and Comparison
		/// @{

	  private:
		///  \cond

		template <T... E>
		static constexpr bool is_equal = std::is_same_v<static_array, static_array<T, E...>>;

		template <T... E>
		static constexpr bool is_less = []() noexcept
		{
			if constexpr (is_equal<T, E...>)
				return false;
			else
			{
				// https://en.cppreference.com/w/cpp/algorithm/lexicographical_compare

				auto* first1	 = values;
				const auto last1 = first1 + sizeof...(Elements);
				auto* first2	 = static_array<T, E...>::values;
				const auto last2 = last1 + sizeof...(E);

				for (; (first1 != last1) && (first2 != last2); ++first1, ++first2)
				{
					if (*first1 < *first2)
						return true;
					if (*first2 < *first1)
						return false;
				}

				return (first1 == last1) && (first2 != last2);
			}
		}();

		///  \endcond

	  public:
		/// \brief Equality operator.
		template <T... E>
		MUU_CONST_INLINE_GETTER
		MUU_CONSTEVAL
		friend bool operator==(static_array, static_array<T, E...>) noexcept
		{
			return is_equal<E...>;
		}

		/// \brief Inequality operator.
		template <T... E>
		MUU_CONST_INLINE_GETTER
		MUU_CONSTEVAL
		friend bool operator!=(static_array, static_array<T, E...>) noexcept
		{
			return !is_equal<E...>;
		}

		/// \brief Less-than operator.
		template <T... E>
		MUU_CONST_INLINE_GETTER
		friend constexpr bool operator<(static_array, static_array<T, E...>) noexcept
		{
			return is_less<E...>;
		}

		/// \brief Less-than-equal-to operator.
		template <T... E>
		MUU_CONST_INLINE_GETTER
		friend constexpr bool operator<=(static_array, static_array<T, E...>) noexcept
		{
			return is_less<E...> || is_equal<E...>;
		}

		/// \brief Greater-than operator.
		template <T... E>
		MUU_CONST_INLINE_GETTER
		friend constexpr bool operator>(static_array, static_array<T, E...>) noexcept
		{
			return !is_less<E...> && !is_equal<E...>;
		}

		/// \brief Greater-than-equal-to operator.
		template <T... E>
		MUU_CONST_INLINE_GETTER
		friend constexpr bool operator>=(static_array, static_array<T, E...>) noexcept
		{
			return !is_less<E...>;
		}

		/// @}
	};
}

#include "impl/header_end.h"
