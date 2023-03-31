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
	template <typename, typename, template <typename> typename...>
	struct strong_typedef;

	namespace impl
	{
		template <typename ValueType>
		struct stypedef_value_typedefs
		{
			using value_type		= ValueType;
			using lvalue_type		= std::add_lvalue_reference_t<std::remove_reference_t<ValueType>>;
			using const_lvalue_type = std::add_lvalue_reference_t<std::add_const_t<std::remove_reference_t<ValueType>>>;
		};

		template <typename ValueType, typename Tag, template <typename> typename... Traits>
		struct stypedef_value_typedefs<strong_typedef<ValueType, Tag, Traits...>> : stypedef_value_typedefs<ValueType>
		{};

		template <typename T>
		using stypedef_value = typename stypedef_value_typedefs<T>::value_type;

		template <typename T>
		using stypedef_lvalue = typename stypedef_value_typedefs<T>::lvalue_type;

		template <typename T>
		using stypedef_const_lvalue = typename stypedef_value_typedefs<T>::const_lvalue_type;

		template <typename T>
		inline constexpr bool stypedef_pure_ops = std::is_scalar_v<remove_cvref<stypedef_value<T>>>		 //
											   || std::is_fundamental_v<remove_cvref<stypedef_value<T>>> //
											   || is_arithmetic<remove_cvref<stypedef_value<T>>>		 //
											   || is_integral<remove_cvref<stypedef_value<T>>>;

		//--- equality -------------------

		template <typename T,
				  typename U = T,
				  bool		 = has_equality_operator<stypedef_const_lvalue<T>, stypedef_const_lvalue<U>>,
				  bool		 = (stypedef_pure_ops<T> && stypedef_pure_ops<U>)>
		struct stypedef_equality
		{};

		template <typename T, typename U>
		struct stypedef_equality<T, U, true, false> // impure ops
		{
			MUU_NODISCARD
			MUU_ALWAYS_INLINE
			friend constexpr decltype(auto) operator==(const T& lhs, const U& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() == std::declval<stypedef_const_lvalue<U>>()))
			{
				return lhs.value == rhs.value;
			}
		};

		template <typename T, typename U>
		struct stypedef_equality<T, U, true, true> // pure ops
		{
			MUU_PURE_INLINE_GETTER
			friend constexpr decltype(auto) operator==(const T& lhs, const U& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() == std::declval<stypedef_const_lvalue<U>>()))
			{
				return lhs.value == rhs.value;
			}
		};

		//--- inequality -------------------

		template <typename T,
				  typename U = T,
				  bool		 = has_inequality_operator<stypedef_const_lvalue<T>, stypedef_const_lvalue<U>>, //
				  bool		 = has_equality_operator<stypedef_const_lvalue<T>, stypedef_const_lvalue<U>>,
				  bool		 = (stypedef_pure_ops<T> && stypedef_pure_ops<U>)>
		struct stypedef_inequality
		{};

		template <typename T, typename U, bool Eq>
		struct stypedef_inequality<T, U, true, Eq, false> // has !=; impure
		{
			MUU_NODISCARD
			MUU_ALWAYS_INLINE
			friend constexpr decltype(auto) operator!=(const T& lhs, const U& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() != std::declval<stypedef_const_lvalue<U>>()))
			{
				return lhs.value != rhs.value;
			}
		};

		template <typename T, typename U, bool Eq>
		struct stypedef_inequality<T, U, true, Eq, true> // has !=; pure
		{
			MUU_PURE_INLINE_GETTER
			friend constexpr decltype(auto) operator!=(const T& lhs, const U& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() != std::declval<stypedef_const_lvalue<U>>()))
			{
				return lhs.value != rhs.value;
			}
		};

		template <typename T, typename U>
		struct stypedef_inequality<T, U, false, true, false> // doesn't have !=, but has ==; impure
		{
			MUU_NODISCARD
			MUU_ALWAYS_INLINE
			friend constexpr decltype(auto) operator!=(const T& lhs, const U& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() == std::declval<stypedef_const_lvalue<U>>()))
			{
				return !(lhs.value == rhs.value);
			}
		};

		template <typename T, typename U>
		struct stypedef_inequality<T, U, false, true, true> // doesn't have !=, but has ==; pure
		{
			MUU_PURE_INLINE_GETTER
			friend constexpr decltype(auto) operator!=(const T& lhs, const U& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() == std::declval<stypedef_const_lvalue<U>>()))
			{
				return !(lhs.value == rhs.value);
			}
		};

		//--- less-than -------------------

		template <typename T,
				  typename U = T,
				  bool		 = has_less_than_operator<stypedef_const_lvalue<T>, stypedef_const_lvalue<U>>,
				  bool		 = (stypedef_pure_ops<T> && stypedef_pure_ops<U>)>
		struct stypedef_less_than
		{};

		template <typename T, typename U>
		struct stypedef_less_than<T, U, true, false> // impure ops
		{
			MUU_NODISCARD
			MUU_ALWAYS_INLINE
			friend constexpr decltype(auto) operator<(const T& lhs, const U& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() < std::declval<stypedef_const_lvalue<U>>()))
			{
				return lhs.value < rhs.value;
			}
		};

		template <typename T, typename U>
		struct stypedef_less_than<T, U, true, true> // pure ops
		{
			MUU_PURE_INLINE_GETTER
			friend constexpr decltype(auto) operator<(const T& lhs, const U& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() < std::declval<stypedef_const_lvalue<U>>()))
			{
				return lhs.value < rhs.value;
			}
		};

		//--- less-than-or-equal -------------------

		template <typename T,
				  typename U = T,
				  bool		 = has_less_than_or_equal_operator<stypedef_const_lvalue<T>, stypedef_const_lvalue<U>>,
				  bool		 = (stypedef_pure_ops<T> && stypedef_pure_ops<U>)>
		struct stypedef_less_than_or_equal
		{};

		template <typename T, typename U>
		struct stypedef_less_than_or_equal<T, U, true, false> // impure
		{
			MUU_NODISCARD
			MUU_ALWAYS_INLINE
			friend constexpr decltype(auto) operator<=(const T& lhs, const U& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() <= std::declval<stypedef_const_lvalue<U>>()))
			{
				return lhs.value <= rhs.value;
			}
		};

		template <typename T, typename U>
		struct stypedef_less_than_or_equal<T, U, true, true> // pure
		{
			MUU_PURE_INLINE_GETTER
			friend constexpr decltype(auto) operator<=(const T& lhs, const U& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() <= std::declval<stypedef_const_lvalue<U>>()))
			{
				return lhs.value <= rhs.value;
			}
		};

		//--- greater-than -------------------

		template <typename T,
				  typename U = T,
				  bool		 = has_greater_than_operator<stypedef_const_lvalue<T>, stypedef_const_lvalue<U>>,
				  bool		 = (stypedef_pure_ops<T> && stypedef_pure_ops<U>)>
		struct stypedef_greater_than
		{};

		template <typename T, typename U>
		struct stypedef_greater_than<T, U, true, false> // impure
		{
			MUU_NODISCARD
			MUU_ALWAYS_INLINE
			friend constexpr decltype(auto) operator>(const T& lhs, const U& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() > std::declval<stypedef_const_lvalue<U>>()))
			{
				return lhs.value > rhs.value;
			}
		};

		template <typename T, typename U>
		struct stypedef_greater_than<T, U, true, true> // pure
		{
			MUU_PURE_INLINE_GETTER
			friend constexpr decltype(auto) operator>(const T& lhs, const U& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() > std::declval<stypedef_const_lvalue<U>>()))
			{
				return lhs.value > rhs.value;
			}
		};

		//--- greater-than-or-equal -------------------

		template <typename T,
				  typename U = T,
				  bool		 = has_greater_than_or_equal_operator<stypedef_const_lvalue<T>, stypedef_const_lvalue<U>>,
				  bool		 = (stypedef_pure_ops<T> && stypedef_pure_ops<U>)>
		struct stypedef_greater_than_or_equal
		{};

		template <typename T, typename U>
		struct stypedef_greater_than_or_equal<T, U, true, false> // impure
		{
			MUU_NODISCARD
			MUU_ALWAYS_INLINE
			friend constexpr decltype(auto) operator>=(const T& lhs, const U& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() >= std::declval<stypedef_const_lvalue<U>>()))
			{
				return lhs.value >= rhs.value;
			}
		};

		template <typename T, typename U>
		struct stypedef_greater_than_or_equal<T, U, true, true> // pure
		{
			MUU_PURE_INLINE_GETTER
			friend constexpr decltype(auto) operator>=(const T& lhs, const U& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() >= std::declval<stypedef_const_lvalue<U>>()))
			{
				return lhs.value >= rhs.value;
			}
		};

		//--- comparable -------------------

		template <typename T, typename U = T>
		struct MUU_EMPTY_BASES stypedef_comparable : stypedef_equality<T, U>,
													 stypedef_inequality<T, U>,
													 stypedef_less_than<T, U>,
													 stypedef_less_than_or_equal<T, U>,
													 stypedef_greater_than<T, U>,
													 stypedef_greater_than_or_equal<T, U>
		{};

		//--- boolean conversion -------------------

		template <typename T, bool = is_convertible<stypedef_const_lvalue<T>, bool>, bool = stypedef_pure_ops<T>>
		struct stypedef_boolean_conversion
		{};

		template <typename T>
		struct stypedef_boolean_conversion<T, true, false> // impure
		{
			MUU_NODISCARD
			MUU_ALWAYS_INLINE
			explicit constexpr operator bool() const
				noexcept(noexcept(static_cast<bool>(std::declval<stypedef_const_lvalue<T>>())))
			{
				return static_cast<bool>(static_cast<const T&>(*this).value);
			}
		};

		template <typename T>
		struct stypedef_boolean_conversion<T, true, true> // pure
		{
			MUU_PURE_INLINE_GETTER
			explicit constexpr operator bool() const
				noexcept(noexcept(static_cast<bool>(std::declval<stypedef_const_lvalue<T>>())))
			{
				return static_cast<bool>(static_cast<const T&>(*this).value);
			}
		};

		//--- logical not -------------------

		template <typename T,
				  bool = has_logical_not_operator<stypedef_const_lvalue<T>>,
				  bool = is_convertible<stypedef_const_lvalue<T>, bool>,
				  bool = stypedef_pure_ops<T>>
		struct stypedef_logical_not
		{};

		template <typename T, bool Conv>
		struct stypedef_logical_not<T, true, Conv, false> // has a ! operator; impure
		{
			MUU_NODISCARD
			MUU_ALWAYS_INLINE
			friend constexpr decltype(auto) operator!(const T& rhs) //
				noexcept(noexcept(!std::declval<stypedef_const_lvalue<T>>()))
			{
				return !rhs.value;
			}
		};

		template <typename T, bool Conv>
		struct stypedef_logical_not<T, true, Conv, true> // has a ! operator; pure
		{
			MUU_PURE_INLINE_GETTER
			friend constexpr decltype(auto) operator!(const T& rhs) //
				noexcept(noexcept(!std::declval<stypedef_const_lvalue<T>>()))
			{
				return !rhs.value;
			}
		};

		template <typename T>
		struct stypedef_logical_not<T, false, true, false> // doesn't have ! operator but is convertible to bool; impure
		{
			MUU_NODISCARD
			MUU_ALWAYS_INLINE
			friend constexpr bool operator!(const T& rhs) //
				noexcept(noexcept(static_cast<bool>(std::declval<stypedef_const_lvalue<T>>())))
			{
				return !static_cast<bool>(rhs.value);
			}
		};

		template <typename T>
		struct stypedef_logical_not<T, false, true, true> // doesn't have ! operator but is convertible to bool; impure
		{
			MUU_PURE_INLINE_GETTER
			friend constexpr bool operator!(const T& rhs) //
				noexcept(noexcept(static_cast<bool>(std::declval<stypedef_const_lvalue<T>>())))
			{
				return !static_cast<bool>(rhs.value);
			}
		};

		//--- boolean -------------------

		template <typename T>
		struct MUU_EMPTY_BASES stypedef_boolean : stypedef_boolean_conversion<T>, stypedef_logical_not<T>
		{};

		//--- ref conversions -------------------

		template <typename T, bool = std::is_same_v<stypedef_lvalue<T>, stypedef_const_lvalue<T>>>
		struct stypedef_ref_operators
		{
			MUU_PURE_INLINE_GETTER
			explicit constexpr operator stypedef_lvalue<T>() noexcept // value_type&
			{
				return static_cast<stypedef_lvalue<T>>(static_cast<T&>(*this).value);
			}

			MUU_PURE_INLINE_GETTER
			explicit constexpr operator stypedef_const_lvalue<T>() const noexcept // const value_type&
			{
				return static_cast<stypedef_const_lvalue<T>>(static_cast<const T&>(*this).value);
			}
		};

		template <typename T>
		struct stypedef_ref_operators<T, true>
		{
			MUU_PURE_INLINE_GETTER
			explicit constexpr operator stypedef_const_lvalue<T>() const noexcept // const value_type&
			{
				return static_cast<stypedef_const_lvalue<T>>(static_cast<const T&>(*this).value);
			}
		};

		//--- pointer arithmetic -------------------

		struct tag_type_just_for_pointer_tests
		{};

		template <typename T,
				  bool = has_addition_operator<tag_type_just_for_pointer_tests*, stypedef_const_lvalue<T>>,
				  bool = stypedef_pure_ops<T>>
		struct stypedef_pointer_arith_add_left
		{};

		template <typename T>
		struct stypedef_pointer_arith_add_left<T, true, false> // impure
		{
			template <typename U>
			MUU_NODISCARD
			MUU_ALWAYS_INLINE
			friend constexpr decltype(auto) operator+(U* lhs, const T& rhs) //
				noexcept(noexcept(static_cast<U*>(nullptr) + std::declval<stypedef_const_lvalue<T>>()))
			{
				return lhs + rhs.value;
			}
		};

		template <typename T>
		struct stypedef_pointer_arith_add_left<T, true, true> // pure
		{
			template <typename U>
			MUU_PURE_INLINE_GETTER
			friend constexpr decltype(auto) operator+(U* lhs, const T& rhs) //
				noexcept(noexcept(static_cast<U*>(nullptr) + std::declval<stypedef_const_lvalue<T>>()))
			{
				return lhs + rhs.value;
			}
		};

		template <typename T,
				  bool = has_addition_operator<stypedef_const_lvalue<T>, tag_type_just_for_pointer_tests*>,
				  bool = stypedef_pure_ops<T>>
		struct stypedef_pointer_arith_add_right
		{};

		template <typename T>
		struct stypedef_pointer_arith_add_right<T, true, false> // impure
		{
			template <typename U>
			MUU_NODISCARD
			MUU_ALWAYS_INLINE
			friend constexpr decltype(auto) operator+(const T& lhs, U* rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() + static_cast<U*>(nullptr)))
			{
				return lhs.value + rhs;
			}
		};

		template <typename T>
		struct stypedef_pointer_arith_add_right<T, true, true> // pure
		{
			template <typename U>
			MUU_PURE_INLINE_GETTER
			friend constexpr decltype(auto) operator+(const T& lhs, U* rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() + static_cast<U*>(nullptr)))
			{
				return lhs.value + rhs;
			}
		};

		template <typename T,
				  bool = has_subtraction_operator<tag_type_just_for_pointer_tests*, stypedef_const_lvalue<T>>,
				  bool = stypedef_pure_ops<T>>
		struct stypedef_pointer_arith_sub_left
		{};

		template <typename T>
		struct stypedef_pointer_arith_sub_left<T, true, false> // impure
		{
			template <typename U>
			MUU_NODISCARD
			MUU_ALWAYS_INLINE
			friend constexpr decltype(auto) operator-(U* lhs, const T& rhs) //
				noexcept(noexcept(static_cast<U*>(nullptr) - std::declval<stypedef_const_lvalue<T>>()))
			{
				return lhs - rhs.value;
			}
		};

		template <typename T>
		struct stypedef_pointer_arith_sub_left<T, true, true> // pure
		{
			template <typename U>
			MUU_PURE_INLINE_GETTER
			friend constexpr decltype(auto) operator-(U* lhs, const T& rhs) //
				noexcept(noexcept(static_cast<U*>(nullptr) - std::declval<stypedef_const_lvalue<T>>()))
			{
				return lhs - rhs.value;
			}
		};

		template <typename T,
				  bool = has_subtraction_operator<stypedef_const_lvalue<T>, tag_type_just_for_pointer_tests*>,
				  bool = stypedef_pure_ops<T>>
		struct stypedef_pointer_arith_sub_right
		{};

		template <typename T>
		struct stypedef_pointer_arith_sub_right<T, true, false> // impure
		{
			template <typename U>
			MUU_NODISCARD
			MUU_ALWAYS_INLINE
			friend constexpr decltype(auto) operator-(const T& lhs, U* rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() - static_cast<U*>(nullptr)))
			{
				return lhs.value - rhs;
			}
		};

		template <typename T>
		struct stypedef_pointer_arith_sub_right<T, true, true> // pure
		{
			template <typename U>
			MUU_PURE_INLINE_GETTER
			friend constexpr decltype(auto) operator-(const T& lhs, U* rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() - static_cast<U*>(nullptr)))
			{
				return lhs.value - rhs;
			}
		};

		template <typename T>
		struct MUU_EMPTY_BASES stypedef_pointer_arithmetic : stypedef_pointer_arith_add_left<T>,
															 stypedef_pointer_arith_add_right<T>,
															 stypedef_pointer_arith_sub_left<T>,
															 stypedef_pointer_arith_sub_right<T>
		{};

		//--- addable -------------------

		template <typename T,
				  bool = has_addition_operator<stypedef_const_lvalue<T>, stypedef_const_lvalue<T>>,
				  bool = stypedef_pure_ops<T>>
		struct stypedef_binary_plus
		{};

		template <typename T>
		struct stypedef_binary_plus<T, true, false> // impure
		{
			MUU_NODISCARD
			MUU_ALWAYS_INLINE
			friend constexpr T operator+(const T& lhs, const T& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() + std::declval<stypedef_const_lvalue<T>>()))
			{
				return T{ lhs.value + rhs.value };
			}
		};

		template <typename T>
		struct stypedef_binary_plus<T, true, true> // pure
		{
			MUU_PURE_INLINE_GETTER
			friend constexpr T operator+(const T& lhs, const T& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() + std::declval<stypedef_const_lvalue<T>>()))
			{
				return T{ lhs.value + rhs.value };
			}
		};

		template <typename T, bool = has_unary_plus_operator<stypedef_const_lvalue<T>>, bool = stypedef_pure_ops<T>>
		struct stypedef_unary_plus
		{};

		template <typename T>
		struct stypedef_unary_plus<T, true, false> // impure
		{
			MUU_NODISCARD
			MUU_ALWAYS_INLINE
			friend constexpr T operator+(const T& lhs) //
				noexcept(noexcept(+std::declval<stypedef_const_lvalue<T>>()))
			{
				return T{ +lhs.value };
			}
		};

		template <typename T>
		struct stypedef_unary_plus<T, true, true> // pure
		{
			MUU_PURE_INLINE_GETTER
			friend constexpr T operator+(const T& lhs) //
				noexcept(noexcept(+std::declval<stypedef_const_lvalue<T>>()))
			{
				return T{ +lhs.value };
			}
		};

		template <typename T>
		struct MUU_EMPTY_BASES stypedef_addable : stypedef_binary_plus<T>, stypedef_unary_plus<T>
		{};

		//--- subtractable -------------------

		template <typename T,
				  bool = has_subtraction_operator<stypedef_const_lvalue<T>, stypedef_const_lvalue<T>>,
				  bool = stypedef_pure_ops<T>>
		struct stypedef_binary_minus
		{};

		template <typename T>
		struct stypedef_binary_minus<T, true, false> // impure
		{
			MUU_NODISCARD
			MUU_ALWAYS_INLINE
			friend constexpr T operator-(const T& lhs, const T& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() - std::declval<stypedef_const_lvalue<T>>()))
			{
				return T{ lhs.value - rhs.value };
			}
		};

		template <typename T>
		struct stypedef_binary_minus<T, true, true> // pure
		{
			MUU_PURE_INLINE_GETTER
			friend constexpr T operator-(const T& lhs, const T& rhs) //
				noexcept(noexcept(std::declval<stypedef_const_lvalue<T>>() - std::declval<stypedef_const_lvalue<T>>()))
			{
				return T{ lhs.value - rhs.value };
			}
		};

		template <typename T, bool = has_unary_minus_operator<stypedef_const_lvalue<T>>, bool = stypedef_pure_ops<T>>
		struct stypedef_unary_minus
		{};

		template <typename T>
		struct stypedef_unary_minus<T, true, false> // pure
		{
			MUU_NODISCARD
			MUU_ALWAYS_INLINE
			friend constexpr T operator-(const T& lhs) //
				noexcept(noexcept(-std::declval<stypedef_const_lvalue<T>>()))
			{
				return T{ -lhs.value };
			}
		};

		template <typename T>
		struct stypedef_unary_minus<T, true, true> // pure
		{
			MUU_PURE_INLINE_GETTER
			friend constexpr T operator-(const T& lhs) //
				noexcept(noexcept(-std::declval<stypedef_const_lvalue<T>>()))
			{
				return T{ -lhs.value };
			}
		};

		template <typename T>
		struct MUU_EMPTY_BASES stypedef_subtractable : stypedef_binary_minus<T>, stypedef_unary_minus<T>
		{};

		//--- incrementable -------------------

		template <typename T, bool = has_pre_increment_operator<stypedef_lvalue<T>>>
		struct stypedef_pre_increment
		{};

		template <typename T>
		struct stypedef_pre_increment<T, true>
		{
			MUU_ALWAYS_INLINE
			friend constexpr T& operator++(T& lhs) //
				noexcept(noexcept(++std::declval<stypedef_lvalue<T>>()))
			{
				++lhs.value;
				return lhs;
			}
		};

		template <typename T, bool = has_post_increment_operator<stypedef_lvalue<T>>>
		struct stypedef_post_increment
		{};

		template <typename T>
		struct stypedef_post_increment<T, true>
		{
			MUU_ALWAYS_INLINE
			friend constexpr T operator++(T& lhs, int) //
				noexcept(noexcept(std::declval<stypedef_lvalue<T>>()++))
			{
				return T{ lhs.value++ };
			}
		};

		template <typename T>
		struct MUU_EMPTY_BASES stypedef_incrementable : stypedef_pre_increment<T>, stypedef_post_increment<T>
		{};

		//--- decrementable -------------------

		template <typename T, bool = has_pre_decrement_operator<stypedef_lvalue<T>>>
		struct stypedef_pre_decrement
		{};

		template <typename T>
		struct stypedef_pre_decrement<T, true>
		{
			MUU_ALWAYS_INLINE
			friend constexpr T& operator--(T& lhs) //
				noexcept(noexcept(--std::declval<stypedef_lvalue<T>>()))
			{
				--lhs.value;
				return lhs;
			}
		};

		template <typename T, bool = has_post_decrement_operator<stypedef_lvalue<T>>>
		struct stypedef_post_decrement
		{};

		template <typename T>
		struct stypedef_post_decrement<T, true>
		{
			MUU_ALWAYS_INLINE
			friend constexpr T operator--(T& lhs, int) //
				noexcept(noexcept(std::declval<stypedef_lvalue<T>>()--))
			{
				return T{ lhs.value-- };
			}
		};

		template <typename T>
		struct MUU_EMPTY_BASES stypedef_decrementable : stypedef_pre_decrement<T>, stypedef_post_decrement<T>
		{};

		//--- callable -------------------

		template <typename T>
		struct stypedef_callable
		{
			MUU_CONSTRAINED_TEMPLATE((std::is_invocable_v<stypedef_lvalue<T>, Args&&...>), typename... Args)
			MUU_ALWAYS_INLINE
			constexpr decltype(auto) operator()(Args&&... args) //
				noexcept(std::is_nothrow_invocable_v<stypedef_lvalue<T>, Args&&...>)
			{
				return static_cast<T&>(*this).value(static_cast<Args&&>(args)...);
			}

			MUU_CONSTRAINED_TEMPLATE((std::is_invocable_v<stypedef_const_lvalue<T>, Args&&...>), typename... Args)
			MUU_ALWAYS_INLINE
			constexpr decltype(auto) operator()(Args&&... args) const //
				noexcept(std::is_nothrow_invocable_v<stypedef_const_lvalue<T>, Args&&...>)
			{
				return static_cast<const T&>(*this).value(static_cast<Args&&>(args)...);
			}
		};

		//--- should the constructor take the init argument by value? -------------------

		// note that "by value" here just means "don't add any ref qualifiers".
		// strong typedefs with ref-qualified value_types are considered "by value" for these purposes since the
		// bare value_type will already have the correct ref qualifier.

		template <typename ValueType>
		inline constexpr bool stypedef_init_with_value_semantics = std::is_reference_v<ValueType>	//
																|| std::is_scalar_v<ValueType>		//
																|| std::is_fundamental_v<ValueType> //
																|| is_arithmetic<ValueType>			//
																|| is_integral<ValueType>;
	}
	/// \endcond

	/// \brief CRTP-based 'mixin' traits for strong typedefs.
	/// \see #muu::strong_typedef
	namespace strong_typedef_traits
	{
		/// \brief Imbues a #muu::strong_typedef with pointer arithmetic operators.
		///
		/// \availability Arithmetic with arbitrary pointers must be supported by the underlying value type.
		MUU_HIDDEN(template <typename T>)
		using pointer_arithmetic = POXY_IMPLEMENTATION_DETAIL(muu::impl::stypedef_pointer_arithmetic<T>);

		/// \brief Imbues a #muu::strong_typedef with addition operators (`x + x`, `+x`)
		///
		/// \availability Addition must be supported by the underlying value type.
		MUU_HIDDEN(template <typename T>)
		using addable = POXY_IMPLEMENTATION_DETAIL(muu::impl::stypedef_addable<T>);

		/// \brief Imbues a #muu::strong_typedef with subtraction operators  (`x - x`, `-x`)
		///
		/// \availability Subtraction must be supported by the underlying value type.
		MUU_HIDDEN(template <typename T>)
		using subtractable = POXY_IMPLEMENTATION_DETAIL(muu::impl::stypedef_subtractable<T>);

		/// \brief Imbues a #muu::strong_typedef with pre- and post-increment operators (`++`).
		///
		/// \availability Incrementing must be supported by the underlying value type.
		MUU_HIDDEN(template <typename T>)
		using incrementable = POXY_IMPLEMENTATION_DETAIL(muu::impl::stypedef_incrementable<T>);

		/// \brief Imbues a #muu::strong_typedef with pre- and post-decrement operators (`--`).
		///
		/// \availability Decrementing must be supported by the underlying value type.
		MUU_HIDDEN(template <typename T>)
		using decrementable = POXY_IMPLEMENTATION_DETAIL(muu::impl::stypedef_decrementable<T>);

		/// \brief Imbues a #muu::strong_typedef with comparison operators (`==`, `!=`, `<`, `<=`, `>`, `>=`).
		///
		/// \availability The operators must be supported by the underlying value type. Any not supported will be omitted.
		MUU_HIDDEN(template <typename T>)
		using comparable = POXY_IMPLEMENTATION_DETAIL(muu::impl::stypedef_comparable<T>);

		/// \brief Imbues a #muu::strong_typedef with explicit boolean conversion and unary logical not operator.
		///
		/// \availability The underlying value type must be convertible to bool.
		MUU_HIDDEN(template <typename T>)
		using boolean = POXY_IMPLEMENTATION_DETAIL(muu::impl::stypedef_boolean<T>);

		/// \brief Imbues a #muu::strong_typedef with function-call operator overloads (`x()`).
		///
		/// \availability The underlying value type must be a callable type (function pointer, lambda, etc).
		MUU_HIDDEN(template <typename T>)
		using callable = POXY_IMPLEMENTATION_DETAIL(muu::impl::stypedef_callable<T>);
	}

	/// \brief A utility class for creating 'strong typedefs'.
	/// \ingroup core
	///
	/// \tparam ValueType	The underlying value type held by the typedef object. May be a reference.
	///						May not be cv-qualified unless it is also a reference.
	/// \tparam	Tag			The tag type necessary to uniquely identify this typedef. May be an incomplete type.
	/// \tparam Traits		Additional CRTP-based 'traits' for adding custom functionality.
	///
	/// \detail		Strong typedefs use the C++ type system to create compile-time guarantees for values that
	///				otherwise might only be represented by regular typedefs, and thus lose their semantic meaning when
	///				passed to the compiler. Doing so prevents an entire class of bugs where incompatible values can be
	///				accidentally used interchangeably because they were of same/convertible type:
	///
	///				\cpp
	///
	///				#ifdef DO_BAD_STUFF
	///				using thing_id    = int;
	///				using thing_index = int;
	///				#else
	///				using thing_id    = muu::strong_typedef<int, struct thing_id_tag>;
	///				using thing_index = muu::strong_typedef<int, struct thing_index_tag>;
	///				#endif
	///
	///				void do_important_stuff(thing_id id)
	///				{
	///					//
	///				}
	///
	///				void this_function_does_bad_stuff(thing_index index)
	///				{
	///					do_important_stuff(index); // won't compile with strong typedefs
	///				}
	///
	///				\ecpp
	///
	///				By default the only operations provided are:
	///					- explicit construction with an initializer
	///					- default construction (if supported by the value type)
	///					- copy construction + assignment (if supported by the value type)
	///					- move construction + assignment (if supported by the value type)
	///					- explicit casts to references of the underlying value type
	///
	///				\cpp
	///
	///				using strong_string = muu::strong_typedef<std::string, struct strong_string_tag>;
	///
	///				strong_string s{ "my string could beat up your string"s };
	///				auto& ref = static_cast<std::string&>(s);
	///
	///				\ecpp
	///
	///				Additional operations can be added using CRTP-based 'traits':
	///
	///				\cpp
	///
	///				using thing_index = muu::strong_typedef<int, struct thing_index_tag,
	///					muu::strong_typedef_traits::incrementable, // adds ++
	///					muu::strong_typedef_traits::decrementable, // adds --
	///					muu::strong_typedef_traits::comparable     // adds ==, !=, <, <=, >, >=
	///				>;
	///
	///				\ecpp
	///
	///
	/// \see
	///		- [Strong typedefs](https://www.jambit.com/en/latest-info/toilet-papers/strong-typedefs/)
	///		- #muu::strong_typedef_traits
	template <typename ValueType, typename Tag, template <typename> typename... Traits>
	struct MUU_EMPTY_BASES strong_typedef //
#ifndef DOXYGEN
		: impl::stypedef_ref_operators<strong_typedef<ValueType, Tag, Traits...>>,
		  Traits<strong_typedef<ValueType, Tag, Traits...>>...
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
		MUU_HIDDEN_CONSTRAINT(!impl::stypedef_init_with_value_semantics<T> && std::is_copy_constructible_v<T>,
							  typename T = value_type)
		explicit constexpr strong_typedef(const value_type& val) //
			noexcept(std::is_nothrow_copy_constructible_v<value_type>)
			: value{ val }
		{}

		/// \brief Value move constructor.
		MUU_HIDDEN_CONSTRAINT(!impl::stypedef_init_with_value_semantics<T> && std::is_move_constructible_v<T>,
							  typename T = value_type)
		explicit constexpr strong_typedef(value_type&& val) //
			noexcept(std::is_nothrow_move_constructible_v<value_type>)
			: value{ static_cast<value_type&&>(val) }
		{}

		/// \brief Value constructor.
		MUU_HIDDEN_CONSTRAINT(impl::stypedef_init_with_value_semantics<T>, typename T = value_type)
		explicit constexpr strong_typedef(value_type val) noexcept
			: value{ static_cast<value_type>(val) } // in case value_type is an rvalue reference
		{}
	};
}

namespace std
{
	/// \brief Specialization of std::tuple_size for muu::strong_typedef.
	template <typename ValueType, typename Tag, template <typename> typename... Traits>
	struct tuple_size<muu::strong_typedef<ValueType, Tag, Traits...>>
	{
		static constexpr size_t value = 1;
	};

	/// \brief Specialization of std::tuple_element for muu::strong_typedef.
	template <size_t Index, typename ValueType, typename Tag, template <typename> typename... Traits>
	struct tuple_element<Index, muu::strong_typedef<ValueType, Tag, Traits...>>
	{
		static_assert(Index == 0);

		using type = ValueType;
	};

	/// \brief Specialization of std::hash for muu::strong_typedef.
	template <typename ValueType, typename Tag, template <typename> typename... Traits>
	struct hash<muu::strong_typedef<ValueType, Tag, Traits...>>
	{
		using value_type = muu::remove_cvref<ValueType>;
		using hash_type	 = std::hash<value_type>;

		MUU_ALWAYS_INLINE
		constexpr decltype(auto) operator()(const muu::strong_typedef<ValueType, Tag, Traits...>& val) const
			noexcept(std::is_nothrow_invocable_v<hash_type, const value_type&>)
		{
			return hash_type{}(val.value);
		}
	};
}

#include "impl/header_end.h"
