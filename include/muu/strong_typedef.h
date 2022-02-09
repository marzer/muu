// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::strong_typedef.

#include "meta.h"
#include "impl/header_start.h"

namespace muu
{
	/// \cond
	template <typename, typename, typename...>
	struct strong_typedef;

	namespace impl
	{
		template <typename>
		struct stypedef_traits
		{
			using value_type = void;
		};

		template <typename ValueType, typename Tag, typename... Mixins>
		struct stypedef_traits<strong_typedef<ValueType, Tag, Mixins...>>
		{
			using value_type		= ValueType;
			using lvalue_type		= std::add_lvalue_reference_t<std::remove_reference_t<ValueType>>;
			using const_lvalue_type = std::add_lvalue_reference_t<std::add_const_t<std::remove_reference_t<ValueType>>>;
			using rvalue_type		= std::add_rvalue_reference_t<std::remove_reference_t<ValueType>>;
			using const_rvalue_type = std::add_rvalue_reference_t<std::add_const_t<std::remove_reference_t<ValueType>>>;

			static constexpr bool is_reference = std::is_reference_v<value_type>;
			static constexpr bool is_const	   = muu::is_const<value_type>;
			static constexpr bool is_volatile  = muu::is_volatile<value_type>;

			static_assert(!(is_const || is_volatile) || is_reference); // plain values may not be cv-qualified
		};

		template <typename T>
		using stypedef_value = typename stypedef_traits<T>::value_type;

		template <typename T>
		using stypedef_lvalue = typename stypedef_traits<T>::lvalue_type;

		template <typename T>
		using stypedef_const_lvalue = typename stypedef_traits<T>::const_lvalue_type;

		template <typename T>
		using stypedef_rvalue = typename stypedef_traits<T>::rvalue_type;

		template <typename T>
		using stypedef_const_rvalue = typename stypedef_traits<T>::const_rvalue_type;

		//--- equality -------------------

		template <typename T, bool = has_equality_operator<stypedef_const_lvalue<T>>>
		struct stypedef_equality
		{};

		template <typename T>
		struct stypedef_equality<T, true>
		{
			MUU_NODISCARD
			friend constexpr decltype(auto) operator==(const T& lhs, const T& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() == std::declval<stypedef_const_lvalue<T>>()))
			{
				return lhs.value == rhs.value;
			}
		};

		//--- inequality -------------------

		template <typename T,
				  bool = has_inequality_operator<stypedef_const_lvalue<T>>, //
				  bool = has_equality_operator<stypedef_const_lvalue<T>>>
		struct stypedef_inequality
		{};

		template <typename T, bool Eq>
		struct stypedef_inequality<T, true, Eq> // has !=
		{
			MUU_NODISCARD
			friend constexpr decltype(auto) operator!=(const T& lhs, const T& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() != std::declval<stypedef_const_lvalue<T>>()))
			{
				return lhs.value != rhs.value;
			}
		};

		template <typename T>
		struct stypedef_inequality<T, false, true> // doesn't have !=, but has ==
		{
			MUU_NODISCARD
			friend constexpr decltype(auto) operator!=(const T& lhs, const T& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() == std::declval<stypedef_const_lvalue<T>>()))
			{
				return !(lhs.value == rhs.value);
			}
		};

		//--- less-than -------------------

		template <typename T, bool = has_less_than_operator<stypedef_const_lvalue<T>>>
		struct stypedef_less_than
		{};

		template <typename T>
		struct stypedef_less_than<T, true>
		{
			MUU_NODISCARD
			friend constexpr decltype(auto) operator<(const T& lhs, const T& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() < std::declval<stypedef_const_lvalue<T>>()))
			{
				return lhs.value < rhs.value;
			}
		};

		//--- less-than-or-equal -------------------

		template <typename T, bool = has_less_than_or_equal_operator<stypedef_const_lvalue<T>>>
		struct stypedef_less_than_or_equal
		{};

		template <typename T>
		struct stypedef_less_than_or_equal<T, true>
		{
			MUU_NODISCARD
			friend constexpr decltype(auto) operator<=(const T& lhs, const T& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() <= std::declval<stypedef_const_lvalue<T>>()))
			{
				return lhs.value <= rhs.value;
			}
		};

		//--- greater-than -------------------

		template <typename T, bool = has_greater_than_operator<stypedef_const_lvalue<T>>>
		struct stypedef_greater_than
		{};

		template <typename T>
		struct stypedef_greater_than<T, true>
		{
			MUU_NODISCARD
			friend constexpr decltype(auto) operator>(const T& lhs, const T& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() > std::declval<stypedef_const_lvalue<T>>()))
			{
				return lhs.value > rhs.value;
			}
		};

		//--- greater-than-or-equal -------------------

		template <typename T, bool = has_greater_than_or_equal_operator<stypedef_const_lvalue<T>>>
		struct stypedef_greater_than_or_equal
		{};

		template <typename T>
		struct stypedef_greater_than_or_equal<T, true>
		{
			MUU_NODISCARD
			friend constexpr decltype(auto) operator>=(const T& lhs, const T& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() >= std::declval<stypedef_const_lvalue<T>>()))
			{
				return lhs.value >= rhs.value;
			}
		};

		//--- logical not -------------------

		template <typename T, bool = has_logical_not_operator<stypedef_const_lvalue<T>>>
		struct stypedef_logical_not
		{};

		template <typename T>
		struct stypedef_logical_not<T, true>
		{
			MUU_NODISCARD
			friend constexpr decltype(auto) operator!(const T& rhs) //
				noexcept(noexcept(!std::declval<stypedef_const_lvalue<T>>()))
			{
				return !rhs.value;
			}
		};

		//--- explicit value conversion  -------------------

		template <typename T, bool = std::is_copy_constructible_v<remove_cvref<stypedef_value<T>>>>
		struct stypedef_value_operator
		{};

		template <typename T>
		struct stypedef_value_operator<T, true>
		{
			// todo
			//
			// MUU_NODISCARD
			// explicit constexpr operator remove_cvref<stypedef_value<T>>() const // value_type
			//	noexcept(std::is_nothrow_copy_constructible_v<remove_cvref<stypedef_value<T>>>)
			//{
			//	using raw_type = remove_cvref<stypedef_value<T>>;
			//	if constexpr (std::is_aggregate_v<raw_type>)
			//		return raw_type{ static_cast<const T&>(*this).value };
			//	else
			//		return raw_type(static_cast<const T&>(*this).value);
			//}
		};

		//--- explicit lvalue & rvalue ref conversions -------------------

		template <typename T, bool = (!stypedef_traits<T>::is_reference || !stypedef_traits<T>::is_const)>
		struct stypedef_ref_operators
		{
			MUU_PURE_INLINE_GETTER
			explicit constexpr operator stypedef_lvalue<T>() noexcept // value_type&
			{
				return static_cast<T&>(*this).value;
			}

			MUU_PURE_INLINE_GETTER
			explicit constexpr operator stypedef_rvalue<T>() noexcept // value_type&&
			{
				return static_cast<stypedef_rvalue<T>>(static_cast<T&&>(*this).value);
			}

			MUU_PURE_INLINE_GETTER
			explicit constexpr operator stypedef_const_lvalue<T>() const noexcept // const value_type&
			{
				return static_cast<const T&>(*this).value;
			}

			MUU_PURE_INLINE_GETTER
			explicit constexpr operator stypedef_const_rvalue<T>() const noexcept // const value_type&&
			{
				return static_cast<stypedef_const_rvalue<T>>(static_cast<const T&&>(*this).value);
			}
		};

		template <typename T>
		struct stypedef_ref_operators<T, false>
		{
			MUU_PURE_INLINE_GETTER
			explicit constexpr operator stypedef_const_lvalue<T>() const noexcept // const value_type&
			{
				return static_cast<const T&>(*this).value;
			}

			MUU_PURE_INLINE_GETTER
			explicit constexpr operator stypedef_const_rvalue<T>() const noexcept // const value_type&&
			{
				return static_cast<stypedef_const_rvalue<T>>(static_cast<const T&&>(*this).value);
			}
		};

		template <typename ValueType>
		inline constexpr bool stypedef_init_with_value_semantics = std::is_reference_v<ValueType>	//
																|| std::is_scalar_v<ValueType>		//
																|| std::is_fundamental_v<ValueType> //
																|| is_arithmetic<ValueType>			//
																|| is_integral<ValueType>;
	}
	/// \endcond

	/// \brief A utility class for creating 'strong typedefs'.
	/// \ingroup core
	///
	/// \tparam ValueType	The underlying value type held by the typedef object. May be a reference.
	///						May not be cv-qualified unless it is also a reference.
	/// \tparam	Tag			The tag type necessary to uniquely identify this typedef.
	/// \tparam Mixins		Additional CTRP-based mixins for adding custom functionality.
	///
	/// \detail		Strong typedefs use the C++ type system to create compile-time guarantees for values that
	///				otherwise might only be represented by regular typedefs, and thus lose their semantic meaning when
	///				passed to the compiler. Doing so prevents an entire class of bugs where incompatible values can be
	///				accidentally used interchangeably because they were of same/convertible type, e.g. a non-sequential
	///				ID `thing_id` being mistakenly used instead of `thing_index` when indexing into an array,
	///				because they both happened to be aliases of `int`:
	///
	///				\cpp
	///
	///				using thing_id = muu::strong_typedef<int, struct thing_id_tag>;
	///				using thing_index = muu::strong_typedef<int, struct thing_index_tag>;
	///
	///				void do_important_stuff(thing_id id)
	///				{
	///					//
	///				}
	///
	///				void this_function_is_broken(thing_index index)
	///				{
	///					do_important_stuff(index); // an obvious error that won't compile with strong typedefs
	///				}
	///
	///				\ecpp
	///
	///				This implementation will automatically provide a number of operators if they are supported by
	///				the wrapped type: `==`, `!=`, `<`, `<=`, `>`, `>=` and `!` (unary logical not). Additional
	///				functionality can be added by adding CTRP-based 'mixins'.
	///
	///
	/// \see [Strong typedefs](https://www.jambit.com/en/latest-info/toilet-papers/strong-typedefs/)
	template <typename ValueType, typename Tag, typename... Mixins>
	struct MUU_EMPTY_BASES strong_typedef //
		: public Mixins...
#ifndef DOXYGEN
		,
		  public impl::stypedef_equality<strong_typedef<ValueType, Tag, Mixins...>>,
		  public impl::stypedef_inequality<strong_typedef<ValueType, Tag, Mixins...>>,
		  public impl::stypedef_less_than<strong_typedef<ValueType, Tag, Mixins...>>,
		  public impl::stypedef_less_than_or_equal<strong_typedef<ValueType, Tag, Mixins...>>,
		  public impl::stypedef_greater_than<strong_typedef<ValueType, Tag, Mixins...>>,
		  public impl::stypedef_greater_than_or_equal<strong_typedef<ValueType, Tag, Mixins...>>,
		  public impl::stypedef_logical_not<strong_typedef<ValueType, Tag, Mixins...>>,
		  public impl::stypedef_value_operator<strong_typedef<ValueType, Tag, Mixins...>>,
		  public impl::stypedef_ref_operators<strong_typedef<ValueType, Tag, Mixins...>>
#endif
	{
		static_assert(!is_cv<ValueType> || std::is_reference_v<ValueType>,
					  "strong_typedef::value_type may not be cv-qualified unless it is also a reference");

		/// \brief The typedef's underlying value type.
		using value_type = ValueType;

		/// The underlying value stored in the typedef.
		value_type value;

		/// \brief Default constructor.
		strong_typedef() = default;

		/// \brief Copy constructor.
		constexpr strong_typedef(const strong_typedef&) = default;

		/// \brief Move constructor.
		constexpr strong_typedef(strong_typedef&&) = default;

		/// \brief Copy-assignment operator.
		strong_typedef& operator=(const strong_typedef&) = default;

		/// \brief Move-assignment operator.
		strong_typedef& operator=(strong_typedef&&) = default;

		/// \brief Value copy constructor.
		/// \availability This constructor is only available when `value_type` is not a reference and is copy-constructible.
		MUU_LEGACY_REQUIRES(!impl::stypedef_init_with_value_semantics<T> && std::is_copy_constructible_v<T>,
							typename T = value_type)
		explicit constexpr strong_typedef(const value_type& val) //
			noexcept(std::is_nothrow_copy_constructible_v<value_type>)
			: value{ val }
		{}

		/// \brief Value move constructor.
		/// \availability This constructor is only available when `value_type` is not a reference and is move-constructible.
		MUU_LEGACY_REQUIRES(!impl::stypedef_init_with_value_semantics<T> && std::is_move_constructible_v<T>,
							typename T = value_type)
		explicit constexpr strong_typedef(value_type&& val) //
			noexcept(std::is_nothrow_move_constructible_v<value_type>)
			: value{ static_cast<value_type&&>(val) }
		{}

		/// \brief Value constructor.
		/// \availability This constructor is only available when `value_type` is a reference.
		MUU_LEGACY_REQUIRES(impl::stypedef_init_with_value_semantics<T>, typename T = value_type)
		explicit constexpr strong_typedef(value_type val) noexcept
			: value{ static_cast<value_type>(val) } // in case value_type is an rvalue reference
		{}
	};
}

#include "impl/header_end.h"
