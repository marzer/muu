// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief Typedefs and intrinsics used by all other muu headers.
#pragma once
#include "preprocessor.h"

//=====================================================================================================================
// INCLUDES
//=====================================================================================================================

MUU_PUSH_WARNINGS
MUU_DISABLE_ALL_WARNINGS

// common.h include file rationale:
// If it's small and simple it can go in (c headers are generally OK)
// If it drags in half the standard library or is itself a behemoth it stays out (<algorithm>...)
// (more info: https://www.reddit.com/r/cpp/comments/eumou7/stl_header_token_parsing_benchmarks_for_vs2017/)

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <climits>
#include <cmath>
#include <type_traits>
#include <utility>


MUU_POP_WARNINGS

//=====================================================================================================================
// TYPEDES AND FORWARD DECLARATIONS
//=====================================================================================================================

/// \brief	The root namespace for all muu functions and types.
namespace muu
{
	using size_t = std::size_t;
	using intptr_t = std::intptr_t;
	using uintptr_t = std::uintptr_t;
	using ptrdiff_t = std::ptrdiff_t;
	using nullptr_t = std::nullptr_t;

	#if !MUU_DOXYGEN

	// foward declarations are hidden from doxygen
	// because they fuck it up =/

	struct uuid;
	struct semver;
	struct float16;

	class blob;
	class string_param;
	class bitset;
	class thread_pool;

	template <typename>				class scope_guard;
	template <typename, size_t>		class tagged_ptr;

	#endif
}

//=====================================================================================================================
// TYPE TRAITS AND METAFUNCTIONS
//=====================================================================================================================

namespace muu::impl
{
	template <typename T, typename... U>
	struct is_one_of : std::integral_constant<bool,
		(false || ... || std::is_same_v<T, U>)
	> {};

	template <typename T> using add_lref = std::add_lvalue_reference_t<T>;
	template <typename T> using add_rref = std::add_rvalue_reference_t<T>;
	template <typename T> using remove_ref = std::remove_reference_t<T>;
	template <typename T, typename U> struct rebase_ref_ { using type = U; };
	template <typename T, typename U> struct rebase_ref_<T&, U> { using type = add_lref<U>; };
	template <typename T, typename U> struct rebase_ref_<T&&, U> { using type = add_rref<U>; };
	template <typename T, typename U> using rebase_ref = typename rebase_ref_<T, U>::type;

	template <typename T> using add_const = rebase_ref<T, std::add_const_t<remove_ref<T>>>;
	template <typename T> using add_volatile = rebase_ref<T, std::add_volatile_t<remove_ref<T>>>;
	template <typename T> using add_cv = rebase_ref<T, std::add_volatile_t<std::add_const_t<remove_ref<T>>>>;

	template <typename T> using remove_const = rebase_ref<T, std::remove_const_t<remove_ref<T>>>;
	template <typename T> using remove_volatile = rebase_ref<T, std::remove_volatile_t<remove_ref<T>>>;
	template <typename T> using remove_cv = rebase_ref<T, std::remove_volatile_t<std::remove_const_t<remove_ref<T>>>>;
	template <typename T> using remove_cvref = std::remove_cv_t<remove_ref<T>>;

	template <typename T, bool Cond> using conditionally_add_cv = std::conditional_t<Cond, add_cv<T>, T>;

	template <typename T>
	struct remove_noexcept { using type = T; };
	template <typename T>
	struct remove_noexcept<const T> { using type = const typename remove_noexcept<T>::type; };
	template <typename T>
	struct remove_noexcept<volatile T> { using type = volatile typename remove_noexcept<T>::type; };
	template <typename T>
	struct remove_noexcept<const volatile T> { using type = const volatile typename remove_noexcept<T>::type; };
	template <typename T>
	struct remove_noexcept<T&> { using type = typename remove_noexcept<T>::type&; };
	template <typename T>
	struct remove_noexcept<T&&> { using type = typename remove_noexcept<T>::type&&; };
	template <typename R, typename ...P>
	struct remove_noexcept<R(P...) noexcept> { using type = R(P...); };
	template <typename R, typename ...P>
	struct remove_noexcept<R(*)(P...) noexcept> { using type = R(*)(P...); };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) noexcept> { using type = R(C::*)(P...); };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) & noexcept> { using type = R(C::*)(P...)&; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) && noexcept> { using type = R(C::*)(P...)&&; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) const noexcept> { using type = R(C::*)(P...) const; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) const& noexcept> { using type = R(C::*)(P...) const&; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) const&& noexcept> { using type = R(C::*)(P...) const&&; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) volatile noexcept> { using type = R(C::*)(P...) volatile; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) volatile& noexcept> { using type = R(C::*)(P...) volatile&; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) volatile&& noexcept> { using type = R(C::*)(P...) volatile&&; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) const volatile noexcept> { using type = R(C::*)(P...) const volatile; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) const volatile& noexcept> { using type = R(C::*)(P...) const volatile&; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) const volatile&& noexcept> { using type = R(C::*)(P...) const volatile&&; };

	template <typename T>
	struct constify_pointer
	{
		static_assert(std::is_pointer_v<T>);
		using type = std::add_pointer_t<std::add_const_t<std::remove_pointer_t<T>>>;
	};
	template <typename T> struct constify_pointer<T&> { using type = typename constify_pointer<T>::type&; };
	template <typename T> struct constify_pointer<T&&> { using type = typename constify_pointer<T>::type&&; };
	template <> struct constify_pointer<const volatile void*> { using type = const volatile void*; };
	template <> struct constify_pointer<volatile void*> { using type = const volatile void*; };
	template <> struct constify_pointer<const void*> { using type = const void*; };
	template <> struct constify_pointer<void*> { using type = const void*; };

	template <typename T, typename U> struct rebase_pointer
	{
		static_assert(std::is_pointer_v<T>);
		using type = U*;
	};
	template <typename T, typename U> struct rebase_pointer<const volatile T*, U> { using type = std::add_const_t<std::add_volatile_t<U>>*; };
	template <typename T, typename U> struct rebase_pointer<volatile T*, U> { using type = std::add_volatile_t<U>*; };
	template <typename T, typename U> struct rebase_pointer<const T*, U> { using type = std::add_const_t<U>*; };
	template <typename T, typename U> struct rebase_pointer<T&, U> { using type = typename rebase_pointer<T, U>::type&; };
	template <typename T, typename U> struct rebase_pointer<T&&, U> { using type = typename rebase_pointer<T, U>::type&&; };

	template <typename T>
	struct alignment_of
	{
		static constexpr size_t value = alignof(T);
	};
	template <> struct alignment_of<void> { static constexpr size_t value = 1; };
	template <typename R, typename ...P> struct alignment_of<R(P...)> { static constexpr size_t value = 1; };
	template <typename R, typename ...P> struct alignment_of<R(P...)noexcept> { static constexpr size_t value = 1; };

	template <typename...> struct largest;
	template <typename T> struct largest<T> { using type = T; };
	template <typename T, typename U>
	struct largest<T, U>
	{
		using type = std::conditional_t<(sizeof(U) < sizeof(T)), T, U>;
	};
	template <typename T, typename U, typename... V>
	struct largest<T, U, V...>
	{
		using type = typename largest<T, typename largest<U, V...>::type>::type;
	};

	template <typename...>	struct smallest;
	template <typename T>	struct smallest<T> { using type = T; };
	template <typename T, typename U>
	struct smallest<T, U>
	{
		using type = std::conditional_t<(sizeof(T) < sizeof(U)), T, U>;
	};
	template <typename T, typename U, typename... V>
	struct smallest<T, U, V...>
	{
		using type = typename smallest<T, typename smallest<U, V...>::type>::type;
	};

	template <typename...>	struct most_aligned;
	template <typename T>	struct most_aligned<T> { using type = T; };
	template <typename T, typename U>
	struct most_aligned<T, U>
	{
		using type = std::conditional_t<(alignment_of<U>::value < alignment_of<T>::value), T, U>;
	};
	template <typename T, typename U, typename... V>
	struct most_aligned<T, U, V...>
	{
		using type = typename most_aligned<T, typename most_aligned<U, V...>::type>::type;
	};

	template <typename...>	struct least_aligned;
	template <typename T>	struct least_aligned<T> { using type = T; };
	template <typename T, typename U>
	struct least_aligned<T, U>
	{
		using type = std::conditional_t<(alignment_of<T>::value < alignment_of<U>::value), T, U>;
	};
	template <typename T, typename U, typename... V>
	struct least_aligned<T, U, V...>
	{
		using type = typename least_aligned<T, typename least_aligned<U, V...>::type>::type;
	};

	template <typename T> struct make_signed;
	template <typename T> struct make_signed<const volatile T> { using type = const volatile typename make_signed<T>::type; };
	template <typename T> struct make_signed<volatile T> { using type = volatile typename make_signed<T>::type; };
	template <typename T> struct make_signed<const T> { using type = const typename make_signed<T>::type; };
	template <typename T> struct make_signed<T&> { using type = typename make_signed<T>::type&; };
	template <typename T> struct make_signed<T&&> { using type = typename make_signed<T>::type&&; };
	template <> struct make_signed<char> { using type = signed char; };
	template <> struct make_signed<signed char> { using type = signed char; };
	template <> struct make_signed<unsigned char> { using type = signed char; };
	template <> struct make_signed<short> { using type = short; };
	template <> struct make_signed<unsigned short> { using type = short; };
	template <> struct make_signed<int> { using type = int; };
	template <> struct make_signed<unsigned int> { using type = int; };
	template <> struct make_signed<long> { using type = long; };
	template <> struct make_signed<unsigned long> { using type = long; };
	template <> struct make_signed<long long> { using type = long long; };
	template <> struct make_signed<unsigned long long> { using type = long long; };
	template <> struct make_signed<float16> { using type = float16; };
	template <> struct make_signed<float> { using type = float; };
	template <> struct make_signed<double> { using type = double; };
	template <> struct make_signed<long double> { using type = long double; };

	template <typename T> struct make_unsigned;
	template <typename T> struct make_unsigned<const volatile T> { using type = const volatile typename make_unsigned<T>::type; };
	template <typename T> struct make_unsigned<volatile T> { using type = volatile typename make_unsigned<T>::type; };
	template <typename T> struct make_unsigned<const T> { using type = const typename make_unsigned<T>::type; };
	template <typename T> struct make_unsigned<T&> { using type = typename make_unsigned<T>::type&; };
	template <typename T> struct make_unsigned<T&&> { using type = typename make_unsigned<T>::type&&; };
	template <> struct make_unsigned<char> { using type = unsigned char; };
	template <> struct make_unsigned<signed char> { using type = unsigned char; };
	template <> struct make_unsigned<unsigned char> { using type = unsigned char; };
	template <> struct make_unsigned<short> { using type = unsigned short; };
	template <> struct make_unsigned<unsigned short> { using type = unsigned short; };
	template <> struct make_unsigned<int> { using type = unsigned int; };
	template <> struct make_unsigned<unsigned int> { using type = unsigned int; };
	template <> struct make_unsigned<long> { using type = unsigned long; };
	template <> struct make_unsigned<unsigned long> { using type = unsigned long; };
	template <> struct make_unsigned<long long> { using type = unsigned long long; };
	template <> struct make_unsigned<unsigned long long> { using type = unsigned long long; };

	template <typename T, bool = std::is_enum_v<remove_cvref<T>>>
	struct remove_enum
	{
		using type = std::underlying_type_t<T>;
	};
	template <typename T> struct remove_enum<T, false> { using type = T; };
	template <typename T> struct remove_enum<const volatile T, true> { using type = const volatile typename remove_enum<T>::type; };
	template <typename T> struct remove_enum<volatile T, true> { using type = volatile typename remove_enum<T>::type; };
	template <typename T> struct remove_enum<const T, true> { using type = const typename remove_enum<T>::type; };
	template <typename T> struct remove_enum<T&, true> { using type = typename remove_enum<T>::type&; };
	template <typename T> struct remove_enum<T&&, true> { using type = typename remove_enum<T>::type&&; };

	//template <typename T, bool = std::is_enum_v<T>>
	//struct underlying_type : std::underlying_type<T> {};
	//template <typename T>
	//struct underlying_type<T, false> { using type = T; };
}

namespace muu
{
	/// \addtogroup		meta		Metafunctions and type traits.
	/// \brief Many of these are mirrors of (or supplementary to) traits found in the standard library's
	/// 	   `<type_traits>`, but with simpler/saner default behaviour (e.g. most metafunctions treat references to
	/// 	   types as if they were the actual type, because that's typically how you actually want them to work).
	/// @{

	/// \brief	Removes the topmost const, volatile and reference qualifiers from a type.
	/// \remarks This is equivalent to C++20's std::remove_cvref_t.
	template <typename T>
	using remove_cvref = impl::remove_cvref<T>;

	/// \brief	Removes the outer enum wrapper from a type, converting it to the underlying integer equivalent.
	/// \detail This is similar to std::underlying_type_t but preserves cv qualifiers and ref categories, as well as
	/// 		being safe to use in SFINAE contexts.
	template <typename T>
	using remove_enum = typename impl::remove_enum<T>::type;

	///// \brief	Removes the outer enum wrapper from a type, converting it to the underlying integer equivalent.
	///// \remarks This _exactly equivalent_ to std::underlying_type_t while also being safe to use in SFINAE contexts.
	///// 		 If you want it to preserve ref categories and cv-qualifiers use muu::remove_enum.
	//template <typename T>
	//using underlying_type = typename impl::underlying_type<T>::type;

	/// \brief The largest type from a set of types.
	template <typename... T>
	using largest = typename impl::largest<T...>::type;

	/// \brief The smallest type from a set of types.
	template <typename... T>
	using smallest = typename impl::smallest<T...>::type;

	/// \brief The default alignment of a type.
	/// \remarks Returns an alignment of `1` for `void` and functions.
	template <typename T>
	inline constexpr size_t alignment_of = impl::alignment_of<remove_cvref<T>>::value;

	/// \brief The type with the largest alignment (i.e. having the largest value for `alignment_of<T>`) from a set of types.
	/// \remarks `void` and functions are considered as having an alignment of `1`.
	template <typename... T>
	using most_aligned = typename impl::most_aligned<T...>::type;

	/// \brief The type with the smallest alignment (i.e. having the smallest value for `alignment_of<T>`) from a set of types.
	/// \remarks `void` and functions are considered as having an alignment of `1`.
	template <typename... T>
	using least_aligned = typename impl::least_aligned<T...>::type;

	/// \brief	True if T is exactly the same as one or more of the types named by U.
	/// \remarks This is a variadic version of std::is_same_v.
	template <typename T, typename... U>
	inline constexpr bool is_one_of = impl::is_one_of<T, U...>::value;

	/// \brief Is a type an enum or reference-to-enum?
	template <typename T>
	inline constexpr bool is_enum = std::is_enum_v<std::remove_reference_t<T>>;

	/// \brief Is a type a C++11 scoped enum class, or reference to one?
	template <typename T>
	inline constexpr bool is_scoped_enum = is_enum<T>
		&& !std::is_convertible_v<remove_cvref<T>, remove_enum<remove_cvref<T>>>;

	/// \brief Is a type a pre-C++11 unscoped enum, or reference to one?
	template <typename T>
	inline constexpr bool is_legacy_enum = is_enum<T>
		&& std::is_convertible_v<remove_cvref<T>, remove_enum<remove_cvref<T>>>;

	/// \brief Is a type unsigned or reference-to-unsigned?
	/// \remarks Returns true for enums backed by unsigned integers.
	template <typename T>
	inline constexpr bool is_unsigned = std::is_unsigned_v<remove_enum<remove_cvref<T>>>;

	/// \brief Is a type signed or reference-to-signed?
	/// \remarks Returns true for enums backed by signed integers.
	/// \remarks Returns true for muu::float16.
	template <typename T>
	inline constexpr bool is_signed = std::is_signed_v<remove_enum<remove_cvref<T>>>
		|| std::is_same_v<remove_cvref<T>, float16>;

	/// \brief Is a type an integral type or a reference to an integral type?
	/// \remarks Returns true for enums.
	template <typename T>
	inline constexpr bool is_integral = std::is_integral_v<remove_enum<remove_cvref<T>>>;

	/// \brief Is a type a floating-point or reference-to-floating-point?
	/// \remarks Returns true for muu::float16.
	template <typename T>
	inline constexpr bool is_floating_point = std::is_floating_point_v<std::remove_reference_t<T>>
		|| std::is_same_v<remove_cvref<T>, float16>;

	/// \brief Is a type arithmetic or reference-to-arithmetic?
	/// \remarks Returns true for muu::float16.
	template <typename T>
	inline constexpr bool is_arithmetic = std::is_arithmetic_v<std::remove_reference_t<T>>
		|| std::is_same_v<remove_cvref<T>, float16>;

	/// \brief Is a type const or reference-to-const?
	template <typename T>
	inline constexpr bool is_const = std::is_const_v<std::remove_reference_t<T>>;

	/// \brief Adds a const qualifier to a type or reference.
	template <typename T>
	using add_const = impl::add_const<T>;

	/// \brief Removes the topmost const qualifier from a type or reference.
	template <typename T>
	using remove_const = impl::remove_const<T>;

	/// \brief Sets the constness of a type or reference according to a boolean.
	template <typename T, bool Const>
	using set_const = std::conditional_t<Const, add_const<T>, remove_const<T>>;

	/// \brief Shorthand for conditionally adding const to a type or reference.
	template <typename T, bool AddConst>
	using conditionally_add_const = std::conditional_t<AddConst, add_const<T>, T>;

	/// \brief Shorthand for marking a type or reference with the same constness as another type or reference.
	template <typename T, typename MatchWith>
	using match_const = set_const<T, is_const<MatchWith>>;

	/// \brief Is a type volatile or reference-to-volatile?
	template <typename T>
	inline constexpr bool is_volatile = std::is_volatile_v<std::remove_reference_t<T>>;

	/// \brief Adds a volatile qualifier to a type or reference.
	template <typename T>
	using add_volatile = impl::add_volatile<T>;

	/// \brief Removes the topmost volatile qualifier from a type or reference.
	template <typename T>
	using remove_volatile = impl::remove_volatile<T>;

	/// \brief Sets the volatility of a type or reference according to a boolean.
	template <typename T, bool Volatile>
	using set_volatile = std::conditional_t<Volatile, add_volatile<T>, remove_volatile<T>>;

	/// \brief Shorthand for conditionally adding volatile to a type or reference.
	template <typename T, bool AddVolatile>
	using conditionally_add_volatile = std::conditional_t<AddVolatile, add_volatile<T>, T>;

	/// \brief Shorthand for marking a type or reference with the same volatility as another type or reference.
	template <typename T, typename MatchWith>
	using match_volatile = set_volatile<T, is_volatile<MatchWith>>;

	/// \brief Adds const and volatile qualifiers to a type or reference.
	template <typename T>
	using add_cv = add_const<add_volatile<T>>;

	/// \brief Removes the topmost const and volatile qualifiers from a type or reference.
	template <typename T>
	using remove_cv = remove_const<remove_volatile<T>>;

	/// \brief Sets the constness and volatility of a type or reference according to a boolean.
	template <typename T, bool ConstVolatile>
	using set_cv = std::conditional_t<ConstVolatile, add_cv<T>, remove_cv<T>>;

	/// \brief Shorthand for conditionally adding const and volatile qualifiers to a type or reference.
	template <typename T, bool AddConstVolatile>
	using conditionally_add_cv = std::conditional_t<AddConstVolatile, add_cv<T>, T>;

	/// \brief Shorthand for marking a type or reference with the same constness and volatility
	/// 	   as another type or reference.
	template <typename T, typename MatchWith>
	using match_cv = match_const<match_volatile<T, MatchWith>, MatchWith>;

	/// \brief Is a type const, volatile, or a reference?
	template <typename T>
	inline constexpr bool is_cvref = std::is_const_v<T> || std::is_volatile_v<T> || std::is_reference_v<T>;

	/// \brief Removes any `noexcept` modifier from a functional type.
	template <typename T>
	using remove_noexcept = typename impl::remove_noexcept<T>::type;

	/// \brief Does Child inherit from Parent?
	/// \remarks This does not return true when the objects are the same type, unlike std::is_base_of.
	template <typename Parent, typename Child>
	inline constexpr bool inherits_from
		= std::is_base_of_v<remove_cvref<Parent>, remove_cvref<Child>>
		&& !std::is_same_v<remove_cvref<Parent>, remove_cvref<Child>>;

	/// \brief	Rebases a pointer, preserving the const and volatile qualification of the pointed type.
	template <typename Ptr, typename Base>
	using rebase_pointer = typename impl::rebase_pointer<Ptr, Base>::type;

	/// \brief	Adds const to the pointed-to type of a pointer, correctly handling pointers-to-void.
	template <typename T>
	using constify_pointer = typename impl::constify_pointer<T>::type;

	/// \brief	Converts a numeric type to the signed equivalent with the same rank.
	/// \remarks CV qualifiers and reference categories are preserved.
	template <typename T>
	using make_signed = typename impl::make_signed<T>::type;

	/// \brief	Converts a numeric type to the unsigned equivalent with the same rank.
	/// \remarks CV qualifiers and reference categories are preserved.
	template <typename T>
	using make_unsigned = typename impl::make_unsigned<T>::type;

	/// @}
}

//=====================================================================================================================
// LITERALS, BUILD CONSTANTS AND 'INTRINSICS'
//=====================================================================================================================

MUU_PRAGMA_MSVC(inline_recursion(on))

namespace muu
{
	/// \brief Literal operators.
	inline namespace literals
	{
		/// \brief	Creates a size_t.
		[[nodiscard]] MUU_ALWAYS_INLINE
		MUU_CONSTEVAL size_t operator"" _sz(unsigned long long n) noexcept
		{
			return static_cast<size_t>(n);
		}

		/// \brief	Creates a uint8_t.
		[[nodiscard]] MUU_ALWAYS_INLINE
		MUU_CONSTEVAL uint8_t operator"" _u8(unsigned long long n) noexcept
		{
			return static_cast<uint8_t>(n);
		}

		/// \brief	Creates a uint16_t.
		[[nodiscard]] MUU_ALWAYS_INLINE
		MUU_CONSTEVAL uint16_t operator"" _u16(unsigned long long n) noexcept
		{
			return static_cast<uint16_t>(n);
		}

		/// \brief	Creates a uint32_t.
		[[nodiscard]] MUU_ALWAYS_INLINE
		MUU_CONSTEVAL uint32_t operator"" _u32(unsigned long long n) noexcept
		{
			return static_cast<uint32_t>(n);
		}

		/// \brief	Creates a uint64_t.
		[[nodiscard]] MUU_ALWAYS_INLINE
		MUU_CONSTEVAL uint64_t operator"" _u64(unsigned long long n) noexcept
		{
			return static_cast<uint64_t>(n);
		}

		/// \brief	Creates a int8_t.
		[[nodiscard]] MUU_ALWAYS_INLINE
		MUU_CONSTEVAL int8_t operator"" _i8(unsigned long long n) noexcept
		{
			return static_cast<int8_t>(n);
		}

		/// \brief	Creates a int16_t.
		[[nodiscard]] MUU_ALWAYS_INLINE
		MUU_CONSTEVAL int16_t operator"" _i16(unsigned long long n) noexcept
		{
			return static_cast<int16_t>(n);
		}

		/// \brief	Creates a int32_t.
		[[nodiscard]] MUU_ALWAYS_INLINE
		MUU_CONSTEVAL int32_t operator"" _i32(unsigned long long n) noexcept
		{
			return static_cast<int32_t>(n);
		}

		/// \brief	Creates a int64_t.
		[[nodiscard]] MUU_ALWAYS_INLINE
		MUU_CONSTEVAL int64_t operator"" _i64(unsigned long long n) noexcept
		{
			return static_cast<int64_t>(n);
		}
	}

	namespace impl
	{
		inline constexpr auto date_str = __DATE__;
		inline constexpr auto date_month_hash = date_str[0] + date_str[1] + date_str[2];
		inline constexpr auto time_str = __TIME__;
	}

	/// \addtogroup		constants		Compile-time constants
	/// @{

	/// \brief Compile-time build constants.
	namespace build
	{
		/// \brief The current C++ language version (17, 20...)
		inline constexpr uint32_t cpp_version = MUU_CPP;
		static_assert(
			cpp_version == 17
			|| cpp_version == 20
			|| cpp_version == 23 //??
			|| cpp_version == 26 //??
		);

		/// \brief The current year.
		inline constexpr uint32_t year =
			(impl::date_str[7] - '0') * 1000
			+ (impl::date_str[8] - '0') * 100
			+ (impl::date_str[9] - '0') * 10
			+ (impl::date_str[10] - '0');
		static_assert(year >= 2020u);

		/// \brief The current month of the year (1-12).
		inline constexpr uint32_t month =
			impl::date_month_hash == 281 ? 1 : (
			impl::date_month_hash == 269 ? 2 : (
			impl::date_month_hash == 288 ? 3 : (
			impl::date_month_hash == 291 ? 4 : (
			impl::date_month_hash == 295 ? 5 : (
			impl::date_month_hash == 301 ? 6 : (
			impl::date_month_hash == 299 ? 7 : (
			impl::date_month_hash == 285 ? 8 : (
			impl::date_month_hash == 296 ? 9 : (
			impl::date_month_hash == 294 ? 10 : (
			impl::date_month_hash == 307 ? 11 : (
			impl::date_month_hash == 268 ? 12 : 0
		)))))))))));
		static_assert(month >= 1 && month <= 12);

		/// \brief The current day of the month (1-31).
		inline constexpr uint32_t day =
			(impl::date_str[4] == ' ' ? 0 : impl::date_str[4] - '0') * 10
			+ (impl::date_str[5] - '0');
		static_assert(day >= 1 && day <= 31);

		/// \brief The current hour of the day (0-23).
		inline constexpr uint32_t hour =
			(impl::time_str[0] == ' ' ? 0 : impl::time_str[0] - '0') * 10
			+ (impl::time_str[1] - '0');
		static_assert(hour >= 0 && hour <= 23);

		/// \brief The current minute (0-59).
		inline constexpr uint32_t minute =
			(impl::time_str[3] == ' ' ? 0 : impl::time_str[3] - '0') * 10
			+ (impl::time_str[4] - '0');
		static_assert(minute >= 0 && minute <= 59);

		/// \brief The current second (0-59).
		inline constexpr uint32_t second =
			(impl::time_str[6] == ' ' ? 0 : impl::time_str[6] - '0') * 10
			+ (impl::time_str[7] - '0');
		static_assert(second >= 0 && second <= 60); // 60 b/c leap seconds

		/// \brief	The bitness of the current architecture.
		inline constexpr size_t bitness = MUU_ARCH_BITNESS;

		/// \brief The number of bits in a byte (aka CHAR_BIT).
		inline constexpr size_t bits_per_byte = CHAR_BIT;

		/// \brief The number of bytes required to store a pointer.
		inline constexpr size_t pointer_size = sizeof(void*);

		/// \brief The number of bits required to store a pointer.
		inline constexpr size_t pointer_bits = pointer_size * bits_per_byte;

		/// \brief True if exceptions are enabled.
		inline constexpr bool has_exceptions = !!MUU_EXCEPTIONS;

		/// \brief True if run-time type identification (RTTI) is enabled.
		inline constexpr bool has_rtti = !!MUU_RTTI;

		/// \brief True if the target environment is little-endian.
		inline constexpr bool is_little_endian = !!MUU_LITTLE_ENDIAN;

		/// \brief True if the target environment is big-endian.
		inline constexpr bool is_big_endian = !!MUU_BIG_ENDIAN;
	
		static_assert(is_little_endian != is_big_endian);
	
	} //::build

	/// @}

	/// \addtogroup		intrinsics		Intrinsics
	/// \brief Small functions available from any muu header, many of which map to compiler intrinsics.
	/// @{

	/// \brief	Equivalent to C++20's std::is_constant_evaluated.
	/// 
	/// \remarks Compilers typically implement std::is_constant_evaluated as an intrinsic which is
	/// 		 available regardless of the C++ mode. Using this function on these compilers allows
	/// 		 you to get the same behaviour even when you aren't targeting C++20.
	/// 
	/// \attention On older compilers lacking support for std::is_constant_evaluated this will always return `false`.
	/// 		   You can check for support by examining build::supports_is_constant_evaluated.
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr bool is_constant_evaluated() noexcept
	{
		#if defined(__clang__)
			#if __has_builtin(__builtin_is_constant_evaluated)
				return __builtin_is_constant_evaluated();
			#else
				return false;
			#endif
		#elif defined(__GNUC__) && __GNUC__ >= 9
			return __builtin_is_constant_evaluated();
		#elif defined(_MSC_VER) && _MSC_VER >= 1925 // Visual Studio 2019 version 16.5
			return __builtin_is_constant_evaluated();
		#elif defined(__cpp_lib_is_constant_evaluated)
			return std::is_constant_evaluated();
		#else
			return false;
		#endif
	}

	namespace build
	{
		/// \brief	True if is_constant_evaluated() is properly supported on this compiler.
		inline constexpr bool supports_is_constant_evaluated = is_constant_evaluated();
	}

	/// \brief	Equivalent to C++17's std::launder.
	/// 		
	/// \remarks Older implementations don't provide this as an intinsic or have a placeholder
	/// 		 for it in their standard library. Using this version allows you to get around that 
	/// 		 by writing code 'as if' it were there and have it compile just the same.
	template <class T>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr T* launder(T* p) noexcept
	{
		#if defined(__clang__)
			#if __has_builtin(__builtin_launder)
				return __builtin_launder(p);
			#else
				return p;
			#endif
		#elif defined(__GNUC__)
			return __builtin_launder(p);
		#elif defined(_MSC_VER) && _MSC_VER >= 1914 // Visual Studio 2017 version 15.7
			return __builtin_launder(p);
		#elif defined(__cpp_lib_launder)
			return std::launder(p);
		#else
			return p;
		#endif
	}

	/// \brief	Unwraps an enum to it's raw integer equivalent.
	///
	/// \tparam	T		An enum type.
	/// \param 	val		The value to unwrap.
	///
	/// \returns	<strong><em>Enum inputs: </em></strong>`static_cast<std::underlying_type_t<T>>(val)`<br>
	/// 			<strong><em>Everything else: A straight pass-through of the input (a no-op).<br>
	template <typename T>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr decltype(auto) unwrap(T&& val) noexcept
	{
		if constexpr (is_enum<T>)
			return static_cast<std::underlying_type_t<std::remove_reference_t<T>>>(val);
		else
			return std::forward<T>(val);
	}

	/// \brief	Checks if an integral value has only a single bit set.
	///
	/// \tparam	T		An unsigned integral or enum type.
	/// \param 	val		The value to test.
	///
	/// \returns	True if the input value had only a single bit set (and thus was a power of two).
	/// 
	/// \remarks This is equivalent to C++20's std::has_single_bit, with the addition of also being
	/// 		 extended to work with enum types.
	template <typename T, typename = std::enable_if_t<is_unsigned<T>>>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr bool has_single_bit(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return has_single_bit(unwrap(val));
		else
			return val != T{} && (val & (val - T{ 1 })) == T{};
	}

	/// \brief	Counts the number of consecutive 0 bits, starting from the most significant bit.
	///
	/// \tparam	T		An unsigned integral or enum type.
	/// \param 	val		The value to test.
	///
	/// \returns	The number of consecutive zeros from the most-significant end of an integer's bits.
	/// 
	/// \remarks This is equivalent to C++20's std::countl_zero, with the addition of also being
	/// 		 extended to work with enum types.
	template <typename T, typename = std::enable_if_t<is_unsigned<T>>>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr int countl_zero(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return static_cast<T>(countl_zero(unwrap(val)));
		else
		{
			if (!val)
				return static_cast<int>(sizeof(T) * CHAR_BIT);

			#if defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)
			{
				if constexpr (std::is_same_v<T, unsigned long long> || sizeof(T) == sizeof(unsigned long long))
					return __builtin_clzll(val);
				else if constexpr (std::is_same_v<T, unsigned int> || sizeof(T) == sizeof(unsigned int))
					return __builtin_clz(val);
				else if constexpr (std::is_same_v<T, unsigned long> || sizeof(T) == sizeof(unsigned long))
					return __builtin_clzl(val);
				else if constexpr (sizeof(T) < sizeof(unsigned int))
					return __builtin_clz(val) - static_cast<int>((sizeof(unsigned int) - sizeof(T)) * CHAR_BIT);
			}
			#else
				// ...
			#endif
		}
	}

	/// \brief	Counts the number of consecutive 0 bits, starting from the least significant bit.
	///
	/// \tparam	T		An unsigned integral or enum type.
	/// \param 	val		The input value.
	///
	/// \returns	The number of consecutive zeros from the least-significant end of an integer's bits.
	/// 
	/// \remarks This is equivalent to C++20's std::countr_zero, with the addition of also being
	/// 		 extended to work with enum types.
	template <typename T, typename = std::enable_if_t<is_unsigned<T>>>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr int countr_zero(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return static_cast<T>(countr_zero(unwrap(val)));
		else
		{
			if (!val)
				return static_cast<int>(sizeof(T) * CHAR_BIT);

			#if defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)
			{
				if constexpr (std::is_same_v<T, unsigned long long> || sizeof(T) == sizeof(unsigned long long))
					return __builtin_ctzll(val);
				else if constexpr (std::is_same_v<T, unsigned int> || sizeof(T) <= sizeof(unsigned int))
					return __builtin_ctz(val);
				else if constexpr (std::is_same_v<T, unsigned long> || sizeof(T) == sizeof(unsigned long))
					return __builtin_ctzl(val);
			}
			#else 
				// ...
			#endif
		}
	}

	/// \brief	Finds the smallest integral power of two not less than the given value.
	///
	/// \tparam	T		An unsigned integral or enum type.
	/// \param 	val		The input value.
	///
	/// \returns	The smallest integral power of two that is not smaller than `val`.
	/// 
	/// \remarks This is equivalent to C++20's std::bit_ceil, with the addition of also being
	/// 		 extended to work with enum types.
	template <typename T, typename = std::enable_if_t<is_unsigned<T>>>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr T bit_ceil(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return static_cast<T>(bit_ceil(unwrap(val)));
		else
		{
			if (!val)
				return T{ 1 };
			return T{ 1 } << (sizeof(T) * CHAR_BIT - static_cast<size_t>(countl_zero(static_cast<T>(val - T{ 1 }))));
		}
	}

	/// \brief	Finds the largest integral power of two not greater than the given value.
	///
	/// \tparam	T		An unsigned integral or enum type.
	/// \param 	val		The input value.
	///
	/// \returns	Zero if `val` is zero; otherwise, the largest integral power of two that is not greater than `val`.
	/// 
	/// \remarks This is equivalent to C++20's std::bit_floor, with the addition of also being
	/// 		 extended to work with enum types.
	template <typename T, typename = std::enable_if_t<is_unsigned<T>>>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr T bit_floor(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return static_cast<T>(bit_floor(unwrap(val)));
		else
		{
			if (!val)
				return T{ 0 };
			return T{ 1 } << (sizeof(T) * CHAR_BIT - 1_sz - static_cast<size_t>(countl_zero(val)));
		}
	}

	/// \brief	Finds the smallest number of bits needed to represent the given value.
	///
	/// \tparam	T		An unsigned integral or enum type.
	/// \param 	val		The input value.
	///
	/// \returns	If `val` is not zero, calculates the number of bits needed to store `val` (i.e. `1 + log2(x)`).
	/// 			Returns `0` if `val` is zero.
	/// 
	/// \remarks This is equivalent to C++20's std::bit_width, with the addition of also being
	/// 		 extended to work with enum types.
	template <typename T, typename = std::enable_if_t<is_unsigned<T>>>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr T bit_width(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return static_cast<T>(bit_width(unwrap(val)));
		else
			return static_cast<T>(sizeof(T) * CHAR_BIT - static_cast<size_t>(countl_zero(val)));
	}

	/// \brief	Returns an unsigned integer filled from the least-significant end
	/// 		with the desired number of consecutive ones.
	/// \detail \cpp
	/// const auto val1 = bit_fill_right<uint32_t>(5);
	/// const auto val2 = 0b00000000000000000000000000011111u;
	/// assert(val1 == val2);
	///  \ecpp
	///  
	/// \tparam	T		An unsigned integral type.
	/// \param 	count	Number of consecutive ones.
	///
	/// \returns	An instance of T right-filled with the desired number of ones.
	template <typename T, typename = std::enable_if_t<is_unsigned<T>>>
	[[nodiscard]]
	constexpr T bit_fill_right(size_t count) noexcept
	{
		if constexpr (is_enum<T>)
			return T{ bit_fill_right<remove_enum<remove_cvref<T>>>(count) };
		else
		{
			if (!count)
				return T{};
			if (count >= build::bits_per_byte * sizeof(T))
				return static_cast<T>(~T{});
			return static_cast<T>((T{ 1 } << count) - T{ 1 });
		}
	}

	/// \brief	Returns an unsigned integer filled from the most-significant end
	/// 		with the desired number of consecutive ones.
	/// \detail \cpp
	/// const auto val1 = bit_fill_left<uint32_t>(5);
	/// const auto val2 = 0b11111000000000000000000000000000u;
	/// assert(val1 == val2);
	///  \ecpp
	///  
	/// \tparam	T		An unsigned integral type.
	/// \param 	count	Number of consecutive ones.
	///
	/// \returns	An instance of T left-filled with the desired number of ones.
	template <typename T, typename = std::enable_if_t<is_unsigned<T>>>
	[[nodiscard]]
	constexpr T bit_fill_left(size_t count) noexcept
	{
		if constexpr (is_enum<T>)
			return T{ bit_fill_left<remove_enum<remove_cvref<T>>>(count) };
		else
		{
			if (!count)
				return T{};
			if (count >= build::bits_per_byte * sizeof(T))
				return static_cast<T>(~T{});
			return static_cast<T>(bit_fill_right<T>(count) << (build::bits_per_byte * sizeof(T) - count));
		}
	}

	#define MUU_HAS_CONSTEXPR_BIT_CAST 1

	/// \brief	Equivalent to C++20's std::bit_cast.
	/// 
	/// \remarks Compilers implement this as an intrinsic which is typically
	/// 		 available regardless of the C++ mode. Using this function
	/// 		 on these compilers allows you to get the same behaviour
	/// 		 even when you aren't targeting C++20.
	/// 
	/// \attention On older compilers lacking support for std::bit_cast you won't be able to call this function
	/// 		   in constexpr contexts (since it falls back to a memcpy-based implementation).
	/// 		   You can check for constexpr support by examining build::supports_constexpr_bit_cast.
	template <typename To, typename From>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr To bit_cast(const From& from) noexcept
	{
		static_assert(
			!std::is_reference_v<To> && !std::is_reference_v<From>,
			"FROM and TO types cannot be references"
		);
		static_assert(
			std::is_trivially_copyable_v<From> && std::is_trivially_copyable_v<To>,
			"FROM and TO types must be trivially-copyable"
		);
		static_assert(
			sizeof(To) == sizeof(From),
			"FROM and TO types must be the same size"
		);

		#define MUU_FALLBACK_IMPL				\
			To dst;								\
			memcpy(&dst, &from, sizeof(To));	\
			return dst

		#if defined(__clang__)
			#if __has_builtin(__builtin_bit_cast)
				return __builtin_bit_cast(To, from);
			#else
				MUU_FALLBACK_IMPL;
				#undef MUU_HAS_CONSTEXPR_BIT_CAST
				#define MUU_HAS_CONSTEXPR_BIT_CAST 0
			#endif
		#elif defined(__GNUC__) && __GNUC__ >= 10 // ??
			return __builtin_bit_cast(To, from);
		#elif defined(_MSC_VER) && _MSC_VER >= 1926 // Visual Studio 2019 version 16.6
			return __builtin_bit_cast(To, from);
		#else
			MUU_FALLBACK_IMPL;
			#undef MUU_HAS_CONSTEXPR_BIT_CAST
			#define MUU_HAS_CONSTEXPR_BIT_CAST 0
		#endif

		#undef MUU_FALLBACK_IMPL
	}

	namespace build
	{
		/// \brief	True if using bit_cast() in constexpr contexts is supported on this compiler.
		inline constexpr bool supports_constexpr_bit_cast = !!MUU_HAS_CONSTEXPR_BIT_CAST;
	}

	#undef MUU_HAS_CONSTEXPR_BIT_CAST

	/// \brief	Returns the minimum of two values.
	/// 
	/// \remarks This is equivalent to std::min without requiring you to drag in the enormity of `<algorithm>`.
	template <typename T>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr const T& min(const T& val1, const T& val2) noexcept
	{
		return val1 < val2 ? val1 : val2;
	}

	/// \brief	Returns the maximum of two values.
	/// 
	/// \remarks This is equivalent to std::max without requiring you to drag in the enormity of `<algorithm>`.
	template <typename T>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr const T& max(const T& val1, const T& val2) noexcept
	{
		return val1 < val2 ? val2 : val1;
	}

	/// \brief	Returns a value clamped between two bounds (inclusive).
	///
	/// \remarks This is equivalent to std::clamp without requiring you to drag in the enormity of `<algorithm>`.
	template <typename T>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr const T& clamp(const T& val, const T& min, const T& max) noexcept
	{
		return muu::max(muu::min(val, max), min);
	}

	/// \brief	Returns true if a value is between two bounds (inclusive).
	template <typename T>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr bool MUU_VECTORCALL is_between(T val, T min, T max) noexcept
	{
		return min <= val && val <= max;
	}

	/// \brief	Returns the absolute value of an arithmetic value.
	/// 
	/// \remarks This is similar to std::abs but is `constexpr` and doesn't coerce or promote the input types.
	template <typename T, typename = std::enable_if_t<is_arithmetic<T>>>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr T MUU_VECTORCALL abs(T val) noexcept
	{
		if constexpr (is_unsigned<T>)
			return val;
		else
			return val < T{} ? -val : val;
	}

	/// \brief	Casts between pointers, choosing the most appropriate conversion path.
	/// 
	/// \detail Doing low-level work with pointers often requires a lot of tedious boilerplate,
	/// 		particularly when moving to/from raw byte representations or dealing with `const`.
	/// 		By using `pointer_cast` instead you can eliminate a lot of that boilerplate,
	/// 		since it will do 'the right thing' via some combination of:
	/// 		- adding `const` and/or `volatile` => `static_cast`  
	/// 		- removing `const` and/or `volatile` => `const_cast`  
	/// 		- casting to/from `void` => `static_cast`  
	/// 		- casting from derived to base => `static_cast`  
	/// 		- casting from base to derived => `dynamic_cast`  
	/// 		- converting between pointers and integers => `reinterpret_cast`  
	/// 		- converting between pointers to unrelated types => `reinterpret_cast`  
	/// 		- converting between function pointers and `void` => `reinterpret_cast` (where available)  
	/// \cpp
	/// 
	/// \ecpp
	/// 
	/// \warning There are lots of static checks to make sure you don't do something completely insane,
	/// 		 but ultimately the fallback behaviour for casting between unrelated types is to use a
	/// 		 `reinterpret_cast`, and there's nothing stopping you from using multiple `pointer_casts`
	/// 		 through `void*` to make a conversion 'work'. Footguns aplenty!
	/// 
	/// 
	/// \tparam To	A pointer or integral type large enough to store a pointer
	/// \tparam From A pointer, array, nullptr_t, or an integral type large enough to store a pointer.
	/// \param from The value being cast.
	/// 
	/// \return The input casted to the desired type.
	/// 
	template <typename To, typename From>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr To pointer_cast(From from) noexcept
	{
		static_assert(
			!std::is_reference_v<To>
			&& !std::is_reference_v<From>, // will never be deduced as a reference but it might be specified explicitly
			"From and To types cannot be references"
		);
		static_assert(
			!(is_integral<From> && is_integral<To>),
			"From and To types cannot both be integral types (did you mean static_cast?)"
		);
		static_assert(
			std::is_pointer_v<To>
			|| (is_integral<To> && sizeof(To) >= sizeof(void*)),
			"To type must be a pointer or an integral type large enough to store a pointer"
		);
		static_assert(
			!(!std::is_same_v<From, remove_cv<nullptr_t>> && std::is_same_v<remove_cv<nullptr_t>, To>),
			"To type cannot be nullptr_t (such a conversion is nonsensical)"
		);
		static_assert(
			std::is_pointer_v<From>
			|| std::is_same_v<From, remove_cv<nullptr_t>>
			|| (is_integral<From> && sizeof(From) >= sizeof(void*)),
			"From type must be a pointer, array, nullptr_t, or an integral type large enough to store a pointer"
		);
		static_assert(
			!std::is_member_pointer_v<To> && !std::is_member_pointer_v<From>,
			"From and To types cannot be pointers to members"
		);

		using from_base = std::remove_pointer_t<From>;
		using to_base = std::remove_pointer_t<To>;

		static_assert(
			(!std::is_function_v<from_base> || sizeof(From) == sizeof(void*))
			&& (!std::is_function_v<to_base> || sizeof(To) == sizeof(void*)),
			"Cannot pointer_cast with function pointers on this platform"
		);

		// same input and output types (no-op)
		if constexpr (std::is_same_v<From, To>)
			return from;

		// nullptr -> *
		else if constexpr (std::is_same_v<From, nullptr_t>)
		{
			(void)from;
			return {};
		}

		// pointer -> integral
		else if constexpr (is_integral<To>)
		{
			// widening conversion and enums
			if constexpr (is_enum<To> || sizeof(To) > sizeof(From))
			{
				using cast_t = std::conditional_t<is_signed<To>, intptr_t, uintptr_t>;
				return static_cast<To>(reinterpret_cast<cast_t>(from));
			}
			// integers of the same size
			else
			{
				static_assert(sizeof(To) == sizeof(From));
				return reinterpret_cast<To>(from);
			}
		}

		// * -> pointer
		else
		{
			// integral -> pointer
			if constexpr (is_integral<From>)
			{
				// enum -> pointer
				if constexpr (is_enum<From>)
					return pointer_cast<To>(unwrap(from));

				// (uintptr_t, intptr_t) -> pointer
				else if constexpr (is_one_of<From, uintptr_t, intptr_t>)
					return reinterpret_cast<To>(from);

				// other integers -> pointer
				else
				{
					using cast_t = std::conditional_t<is_signed<From>, intptr_t, uintptr_t>;
					return reinterpret_cast<To>(static_cast<cast_t>(from));
				}
			}

			// pointer -> pointer
			else
			{
				// function -> *
				if constexpr (std::is_function_v<from_base> && !std::is_function_v<to_base>)
				{
					static_assert(
						std::is_void_v<to_base>,
						"Cannot cast from a function pointer to a type other than void"
					);

					// function -> void
					return static_cast<To>(reinterpret_cast<void*>(from));
				}

				// * -> function
				else if constexpr (!std::is_function_v<from_base> && std::is_function_v<to_base>)
				{
					static_assert(
						std::is_void_v<from_base>,
						"Cannot cast to a function pointer from a type other than void"
					);

					// void -> function
					return reinterpret_cast<To>(pointer_cast<void*>(from));
				}

				// function -> function
				else if constexpr (std::is_function_v<from_base> && std::is_function_v<to_base>)
				{
					static_assert(
						std::is_same_v<remove_noexcept<from_base>, remove_noexcept<to_base>>,
						"Cannot cast between pointers to two different function types"
					);

					// function -> function (noexcept)
					if constexpr (std::is_same_v<from_base, remove_noexcept<from_base>>)
						return reinterpret_cast<To>(from);

					// function (noexcept) -> function
					else
						return static_cast<To>(from);
				}

				// void -> void (different cv)
				else if constexpr (std::is_void_v<from_base> && std::is_void_v<to_base>)
				{
					// remove const/volatile
					if constexpr (is_one_of<From, const void*, const volatile void*, volatile void*>)
						return static_cast<To>(const_cast<void*>(from));

					// add const/volatile
					else
					{
						static_assert(std::is_same_v<From, void*>);
						static_assert(is_one_of<To, const void*, const volatile void*, volatile void*>);
						return static_cast<To>(from);
					}
				}

				// void -> *
				// * -> void
				// derived -> base
				else if constexpr (std::is_void_v<from_base> || std::is_void_v<to_base> || inherits_from<to_base, from_base>)
					return pointer_cast<To>(static_cast<rebase_pointer<From, remove_cv<to_base>>>(from));

				// base -> derived
				else if constexpr (inherits_from<from_base, to_base>)
					return pointer_cast<To>(dynamic_cast<rebase_pointer<From, remove_cv<to_base>>>(from));

				// A -> B (unrelated types)
				else if constexpr (!std::is_same_v<remove_cv<from_base>, remove_cv<to_base>>)
					return pointer_cast<To>(reinterpret_cast<rebase_pointer<From, remove_cv<to_base>>>(from));

				// A -> A (different cv)
				else
				{
					static_assert(!std::is_same_v<from_base, to_base>);
					static_assert(std::is_same_v<remove_cv<from_base>, remove_cv<to_base>>);

					// remove const/volatile
					if constexpr (is_const<from_base> || is_volatile<from_base>)
						return static_cast<To>(const_cast<remove_cv<from_base>*>(from));

					// add const/volatile
					else
					{
						static_assert(std::is_same_v<From, remove_cv<from_base>*>);
						static_assert(is_one_of<To, const to_base*, const volatile to_base*, volatile to_base*>);
						return static_cast<To>(from);
					}
				}
			}
		}
	}

	template <typename To, typename From, size_t N>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr To pointer_cast(From(&arr)[N]) noexcept
	{
		return pointer_cast<To>(&arr[0]);
	}

	template <typename To, typename From, size_t N>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr To pointer_cast(From(&&arr)[N]) noexcept
	{
		return pointer_cast<To>(&arr[0]);
	}

	/// \brief	Applies a byte offset to a pointer.
	///
	/// \tparam	T	The type being pointed to.
	/// \param	ptr	The pointer to offset.
	/// \param	offset	The number of bytes to add to the pointer's address.
	///
	/// \return	The equivalent of `(T*)((std::byte*)ptr + offset)`.
	/// 
	/// \warning This function is a simple pointer arithmetic helper; absolutely no consideration
	/// 		 is given to the alignment of the pointed type. If you need to dereference pointers
	/// 		 returned by apply_offset the onus is on you to ensure that the offset made sense
	/// 		 before doing so!
	template <typename T>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr T* apply_offset(T* ptr, ptrdiff_t offset) noexcept
	{
		return pointer_cast<T*>(pointer_cast<rebase_pointer<T*, std::byte>>(ptr) + offset);
	}

	/// \brief	Checks if an arithmetic value is infinity or NaN.
	///
	/// \tparam	T	The value type.
	/// \param 	val	The value to examine.
	///
	/// \returns	True if the value is floating-point infinity or NaN.
	/// 			Always returns false if the value was not a floating-point type.
	template <typename T, typename = std::enable_if_t<is_arithmetic<T>>>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr bool MUU_VECTORCALL is_infinity_or_nan(T val) noexcept
	{
		if constexpr (is_floating_point<T>)
		{
			auto classification = std::fpclassify(val);
			return classification == FP_INFINITE || classification == FP_NAN;
		}
		else
		{
			(void)val;
			return false;
		}
	}

	/// \brief	Gets a specific byte from an integer.
	///
	/// \tparam	Index	Index of the byte to retrieve.
	/// \tparam	T		An integer or enum type.
	/// \param 	val		An integer or enum value.
	///
	/// \remarks The indexation order of bytes is the _memory_ order, not their
	/// 		 numeric significance (i.e. byte 0 is always the first byte in the integer's
	/// 		 memory allocation, regardless of the endianness of the platform).
	/// 
	/// \returns	The value of the selected byte.
	template <size_t Index, typename T, typename = std::enable_if_t<is_integral<T>>>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr uint8_t MUU_VECTORCALL select_byte(T val) noexcept
	{
		static_assert(
			Index < sizeof(T),
			"The byte index is out-of-range; it must be less than the size of the input integer"
		);

		if constexpr (is_enum<T>)
			return select_byte<Index>(unwrap(val));
		else if constexpr (is_signed<T>)
			return select_byte<Index>(static_cast<make_unsigned<T>>(val));
		else if constexpr (sizeof(T) == 1_sz)
			return static_cast<uint8_t>(val);
		else
		{
			constexpr auto mask = static_cast<T>(0xFFu) << (Index * CHAR_BIT);
			if constexpr (Index == 0_sz)
				return static_cast<uint8_t>(mask & static_cast<T>(val));
			else
				return static_cast<uint8_t>((mask & static_cast<T>(val)) >> (Index * CHAR_BIT));
		}
	}


	/// @}
}

MUU_PRAGMA_MSVC(inline_recursion(off))

// clang-format on
