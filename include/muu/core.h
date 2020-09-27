// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief Typedefs, intrinsics and core components.

#pragma once
#include "../muu/fwd.h"

#if MUU_GCC && MUU_HAS_FLOAT128 && MUU_EXTENDED_LITERALS
	#pragma GCC system_header
#endif

//=====================================================================================================================
// INCLUDES
//=====================================================================================================================

MUU_DISABLE_WARNINGS
// core.h include file rationale:
// - If it's small and simple it can go in (c headers are generally OK)
// - If it drags in half the standard library or is itself a behemoth it stays out (<algorithm>...)
// 
// Or, put differently: If the 'impact' is anything above green-yellow according to
// https://artificial-mind.net/projects/compile-health/, or measuring off-the-charts expensive according to
// https://www.reddit.com/r/cpp/comments/eumou7/stl_header_token_parsing_benchmarks_for_vs2017/, it's a no from me, dawg.
// 
// Mercifully, most things that you might feel compelled to stick here can be worked around by forward-declarations.
#include <cstring>
#include <climits>
#include <cmath>
#include <cfloat>
#include <type_traits>
#include <utility>
#include <limits>
#if MUU_MSVC
	#include <intrin0.h>
#endif
MUU_ENABLE_WARNINGS

MUU_PUSH_WARNINGS
MUU_DISABLE_SPAM_WARNINGS
MUU_PRAGMA_MSVC(inline_recursion(on))

//=====================================================================================================================
// ENVIRONMENT GROUND-TRUTHS
//=====================================================================================================================

#ifndef DOXYGEN
#ifndef MUU_DISABLE_ENVIRONMENT_CHECKS
#define MUU_ENV_MESSAGE																								\
	"If you're seeing this error it's because you're building muu for an environment that doesn't conform to "		\
	"one of the 'ground truths' assumed by the library. Essentially this just means that I don't have the "			\
	"resources to test on more esoteric platforms, but I wish I did! You can try disabling the checks by defining "	\
	"MUU_DISABLE_ENVIRONMENT_CHECKS, but your mileage may vary. Please consider filing an issue at "				\
	"https://github.com/marzer/muu/issues to help me improve support for your target environment. Thanks!"

static_assert(CHAR_BIT == 8, MUU_ENV_MESSAGE);
static_assert(FLT_RADIX == 2, MUU_ENV_MESSAGE);
static_assert('A' == 65, MUU_ENV_MESSAGE);
static_assert(std::numeric_limits<float>::is_iec559, MUU_ENV_MESSAGE);
static_assert(std::numeric_limits<double>::is_iec559, MUU_ENV_MESSAGE);

#undef MUU_ENV_MESSAGE
#endif // !MUU_DISABLE_ENVIRONMENT_CHECKS
#endif // !DOXYGEN

//=====================================================================================================================
// TYPE TRAITS AND METAFUNCTIONS
//=====================================================================================================================

MUU_IMPL_NAMESPACE_START
{
	// note that all the structs with nested types end in underscores;
	// this is a disambiguation mechanism for code in the impl namespace.

	template <typename T, typename U> struct rebase_ref_ { using type = U; };
	template <typename T, typename U> struct rebase_ref_<T&, U> { using type = std::add_lvalue_reference_t<U>; };
	template <typename T, typename U> struct rebase_ref_<T&&, U> { using type = std::add_rvalue_reference_t<U>; };

	template <typename T, typename U> struct rebase_pointer_
	{
		static_assert(std::is_pointer_v<T>);
		using type = U*;
	};
	template <typename T, typename U> struct rebase_pointer_<const volatile T*, U> { using type = std::add_const_t<std::add_volatile_t<U>>*; };
	template <typename T, typename U> struct rebase_pointer_<volatile T*, U> { using type = std::add_volatile_t<U>*; };
	template <typename T, typename U> struct rebase_pointer_<const T*, U> { using type = std::add_const_t<U>*; };
	template <typename T, typename U> struct rebase_pointer_<T&, U> { using type = typename rebase_pointer_<T, U>::type&; };
	template <typename T, typename U> struct rebase_pointer_<T&&, U> { using type = typename rebase_pointer_<T, U>::type&&; };

	template <typename T, bool = std::is_enum_v<std::remove_reference_t<T>>>
	struct remove_enum_
	{
		using type = std::underlying_type_t<T>;
	};
	template <typename T> struct remove_enum_<T, false> { using type = T; };
	template <typename T> struct remove_enum_<const volatile T, true> { using type = const volatile typename remove_enum_<T>::type; };
	template <typename T> struct remove_enum_<volatile T, true> { using type = volatile typename remove_enum_<T>::type; };
	template <typename T> struct remove_enum_<const T, true> { using type = const typename remove_enum_<T>::type; };
	template <typename T> struct remove_enum_<T&, true> { using type = typename remove_enum_<T>::type&; };
	template <typename T> struct remove_enum_<T&&, true> { using type = typename remove_enum_<T>::type&&; };

	template <typename T>
	struct remove_noexcept_ { using type = T; };
	template <typename T>
	struct remove_noexcept_<const T> { using type = const typename remove_noexcept_<T>::type; };
	template <typename T>
	struct remove_noexcept_<volatile T> { using type = volatile typename remove_noexcept_<T>::type; };
	template <typename T>
	struct remove_noexcept_<const volatile T> { using type = const volatile typename remove_noexcept_<T>::type; };
	template <typename T>
	struct remove_noexcept_<T&> { using type = typename remove_noexcept_<T>::type&; };
	template <typename T>
	struct remove_noexcept_<T&&> { using type = typename remove_noexcept_<T>::type&&; };
	template <typename R, typename ...P>
	struct remove_noexcept_<R(P...) noexcept> { using type = R(P...); };
	template <typename R, typename ...P>
	struct remove_noexcept_<R(*)(P...) noexcept> { using type = R(*)(P...); };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept_<R(C::*)(P...) noexcept> { using type = R(C::*)(P...); };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept_<R(C::*)(P...) & noexcept> { using type = R(C::*)(P...)&; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept_<R(C::*)(P...) && noexcept> { using type = R(C::*)(P...)&&; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept_<R(C::*)(P...) const noexcept> { using type = R(C::*)(P...) const; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept_<R(C::*)(P...) const& noexcept> { using type = R(C::*)(P...) const&; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept_<R(C::*)(P...) const&& noexcept> { using type = R(C::*)(P...) const&&; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept_<R(C::*)(P...) volatile noexcept> { using type = R(C::*)(P...) volatile; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept_<R(C::*)(P...) volatile& noexcept> { using type = R(C::*)(P...) volatile&; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept_<R(C::*)(P...) volatile&& noexcept> { using type = R(C::*)(P...) volatile&&; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept_<R(C::*)(P...) const volatile noexcept> { using type = R(C::*)(P...) const volatile; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept_<R(C::*)(P...) const volatile& noexcept> { using type = R(C::*)(P...) const volatile&; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept_<R(C::*)(P...) const volatile&& noexcept> { using type = R(C::*)(P...) const volatile&&; };

	template <typename T>
	struct alignment_of_
	{
		static constexpr size_t value = alignof(T);
	};
	template <> struct alignment_of_<void> { static constexpr size_t value = 1; };
	template <typename R, typename ...P> struct alignment_of_<R(P...)> { static constexpr size_t value = 1; };
	template <typename R, typename ...P> struct alignment_of_<R(P...)noexcept> { static constexpr size_t value = 1; };

	template <typename...> struct largest_;
	template <typename T> struct largest_<T> { using type = T; };
	template <typename T, typename U>
	struct largest_<T, U>
	{
		using type = std::conditional_t<(sizeof(U) < sizeof(T)), T, U>;
	};
	template <typename T, typename U, typename... V>
	struct largest_<T, U, V...>
	{
		using type = typename largest_<T, typename largest_<U, V...>::type>::type;
	};

	template <typename...>	struct smallest_;
	template <typename T>	struct smallest_<T> { using type = T; };
	template <typename T, typename U>
	struct smallest_<T, U>
	{
		using type = std::conditional_t<(sizeof(T) < sizeof(U)), T, U>;
	};
	template <typename T, typename U, typename... V>
	struct smallest_<T, U, V...>
	{
		using type = typename smallest_<T, typename smallest_<U, V...>::type>::type;
	};

	template <typename...>	struct most_aligned_;
	template <typename T>	struct most_aligned_<T> { using type = T; };
	template <typename T>	struct most_aligned_<T, void> { using type = T; };
	template <typename T>	struct most_aligned_<void, T> { using type = T; };
	template <typename T, typename R, typename P> struct most_aligned_<T, R(P...)> { using type = T; };
	template <typename T, typename R, typename P> struct most_aligned_<R(P...), T> { using type = T; };
	template <typename T, typename R, typename P> struct most_aligned_<T, R(P...)noexcept> { using type = T; };
	template <typename T, typename R, typename P> struct most_aligned_<R(P...)noexcept, T> { using type = T; };
	template <typename T, typename U>
	struct most_aligned_<T, U>
	{
		using type = std::conditional_t<(alignment_of_<U>::value < alignment_of_<T>::value), T, U>;
	};
	template <typename T, typename U, typename... V>
	struct most_aligned_<T, U, V...>
	{
		using type = typename most_aligned_<T, typename most_aligned_<U, V...>::type>::type;
	};

	template <typename...>	struct least_aligned_;
	template <typename T>	struct least_aligned_<T> { using type = T; };
	template <typename T>	struct least_aligned_<T, void> { using type = T; };
	template <typename T>	struct least_aligned_<void, T> { using type = T; };
	template <typename T, typename R, typename P> struct least_aligned_<T, R(P...)> { using type = T; };
	template <typename T, typename R, typename P> struct least_aligned_<R(P...), T> { using type = T; };
	template <typename T, typename R, typename P> struct least_aligned_<T, R(P...)noexcept> { using type = T; };
	template <typename T, typename R, typename P> struct least_aligned_<R(P...)noexcept, T> { using type = T; };
	template <typename T, typename U>
	struct least_aligned_<T, U>
	{
		using type = std::conditional_t<(alignment_of_<T>::value < alignment_of_<U>::value), T, U>;
	};
	template <typename T, typename U, typename... V>
	struct least_aligned_<T, U, V...>
	{
		using type = typename least_aligned_<T, typename least_aligned_<U, V...>::type>::type;
	};

	template <size_t Bits> struct signed_integer_;
	template <> struct signed_integer_<8> { using type = int8_t; };
	template <> struct signed_integer_<16> { using type = int16_t; };
	template <> struct signed_integer_<32> { using type = int32_t; };
	template <> struct signed_integer_<64> { using type = int64_t; };
	#if MUU_HAS_INT128
	template <> struct signed_integer_<128> { using type = int128_t; };
	#endif

	template <size_t Bits> struct unsigned_integer_;
	template <> struct unsigned_integer_<8> { using type = uint8_t; };
	template <> struct unsigned_integer_<16> { using type = uint16_t; };
	template <> struct unsigned_integer_<32> { using type = uint32_t; };
	template <> struct unsigned_integer_<64> { using type = uint64_t; };
	#if MUU_HAS_INT128
	template <> struct unsigned_integer_<128> { using type = uint128_t; };
	#endif

	template <typename T> struct make_signed_;
	template <typename T> struct make_signed_<const volatile T> { using type = const volatile typename make_signed_<T>::type; };
	template <typename T> struct make_signed_<volatile T> { using type = volatile typename make_signed_<T>::type; };
	template <typename T> struct make_signed_<const T> { using type = const typename make_signed_<T>::type; };
	template <typename T> struct make_signed_<T&> { using type = typename make_signed_<T>::type&; };
	template <typename T> struct make_signed_<T&&> { using type = typename make_signed_<T>::type&&; };
	template <> struct make_signed_<char> { using type = signed char; };
	template <> struct make_signed_<signed char> { using type = signed char; };
	template <> struct make_signed_<unsigned char> { using type = signed char; };
	template <> struct make_signed_<short> { using type = short; };
	template <> struct make_signed_<unsigned short> { using type = short; };
	template <> struct make_signed_<int> { using type = int; };
	template <> struct make_signed_<unsigned int> { using type = int; };
	template <> struct make_signed_<long> { using type = long; };
	template <> struct make_signed_<unsigned long> { using type = long; };
	template <> struct make_signed_<long long> { using type = long long; };
	template <> struct make_signed_<unsigned long long> { using type = long long; };
	template <> struct make_signed_<half> { using type = half; };
	template <> struct make_signed_<float> { using type = float; };
	template <> struct make_signed_<double> { using type = double; };
	template <> struct make_signed_<long double> { using type = long double; };
	#if MUU_HAS_INT128
	template <> struct make_signed_<int128_t> { using type = int128_t; };
	template <> struct make_signed_<uint128_t> { using type = int128_t; };
	#endif
	#if MUU_HAS_FLOAT128
	template <> struct make_signed_<float128_t> { using type = float128_t; };
	#endif
	#if MUU_HAS_INTERCHANGE_FP16
	template <> struct make_signed_<__fp16> { using type = __fp16; };
	#endif
	#if MUU_HAS_FLOAT16
	template <> struct make_signed_<float16_t> { using type = float16_t; };
	#endif
	template <>
	struct make_signed_<wchar_t>
	{
		using type = std::conditional_t<std::is_signed_v<wchar_t>, wchar_t, signed_integer_<sizeof(wchar_t)* CHAR_BIT>::type>;
	};

	template <typename T> struct make_unsigned_;
	template <typename T> struct make_unsigned_<const volatile T> { using type = const volatile typename make_unsigned_<T>::type; };
	template <typename T> struct make_unsigned_<volatile T> { using type = volatile typename make_unsigned_<T>::type; };
	template <typename T> struct make_unsigned_<const T> { using type = const typename make_unsigned_<T>::type; };
	template <typename T> struct make_unsigned_<T&> { using type = typename make_unsigned_<T>::type&; };
	template <typename T> struct make_unsigned_<T&&> { using type = typename make_unsigned_<T>::type&&; };
	template <> struct make_unsigned_<char> { using type = unsigned char; };
	template <> struct make_unsigned_<signed char> { using type = unsigned char; };
	template <> struct make_unsigned_<unsigned char> { using type = unsigned char; };
	template <> struct make_unsigned_<short> { using type = unsigned short; };
	template <> struct make_unsigned_<unsigned short> { using type = unsigned short; };
	template <> struct make_unsigned_<int> { using type = unsigned int; };
	template <> struct make_unsigned_<unsigned int> { using type = unsigned int; };
	template <> struct make_unsigned_<long> { using type = unsigned long; };
	template <> struct make_unsigned_<unsigned long> { using type = unsigned long; };
	template <> struct make_unsigned_<long long> { using type = unsigned long long; };
	template <> struct make_unsigned_<unsigned long long> { using type = unsigned long long; };
	template <> struct make_unsigned_<char32_t> { using type = char32_t; };
	template <> struct make_unsigned_<char16_t> { using type = char16_t; };
	#ifdef __cpp_char8_t
	template <> struct make_unsigned_<char8_t> { using type = char8_t; };
	#endif
	#if MUU_HAS_INT128
	template <> struct make_unsigned_<int128_t> { using type = uint128_t; };
	template <> struct make_unsigned_<uint128_t> { using type = uint128_t; };
	#endif
	template <>
	struct make_unsigned_<wchar_t>
	{
		using type = std::conditional_t<std::is_unsigned_v<wchar_t>, wchar_t, unsigned_integer_<sizeof(wchar_t)* CHAR_BIT>::type>;
	};

	template <typename...>				struct highest_ranked_;
	template <typename T>				struct highest_ranked_<T>		{ using type = T; };
	template <typename T>				struct highest_ranked_<T, T>	{ using type = T; };
	template <typename T>				struct highest_ranked_<void, T>	{ using type = T; };
	template <typename T>				struct highest_ranked_<T, void>	{ using type = T; };
	template <typename T, typename U>	struct highest_ranked_<T, U>	{ using type = decltype(T{} + U{}); };
	template <typename T, typename U, typename... V>
	struct highest_ranked_<T, U, V...>
	{
		using type = typename highest_ranked_<T, typename highest_ranked_<U, V...>::type>::type;
	};
	#define MUU_HR_SPECIALIZATION(lower, higher)									\
		template <> struct highest_ranked_<lower, higher> { using type = higher; };	\
		template <> struct highest_ranked_<higher, lower> { using type = higher; }
	MUU_HR_SPECIALIZATION(signed char, signed short);
	MUU_HR_SPECIALIZATION(unsigned char, unsigned short);
	MUU_HR_SPECIALIZATION(unsigned char, signed short);
	MUU_HR_SPECIALIZATION(half, float);
	MUU_HR_SPECIALIZATION(half, double);
	MUU_HR_SPECIALIZATION(half, long double);
	#if MUU_HAS_INTERCHANGE_FP16
		MUU_HR_SPECIALIZATION(__fp16, half);
		MUU_HR_SPECIALIZATION(__fp16, float);
		MUU_HR_SPECIALIZATION(__fp16, double);
		MUU_HR_SPECIALIZATION(__fp16, long double);
	#endif
	#if MUU_HAS_FLOAT16
		MUU_HR_SPECIALIZATION(half, float16_t);
		MUU_HR_SPECIALIZATION(float16_t, float);
		MUU_HR_SPECIALIZATION(float16_t, double);
		MUU_HR_SPECIALIZATION(float16_t, long double);
	#endif
	#if MUU_HAS_FLOAT128
		MUU_HR_SPECIALIZATION(half, float128_t);
	#endif
	#if MUU_HAS_INTERCHANGE_FP16 && MUU_HAS_FLOAT16
		MUU_HR_SPECIALIZATION(__fp16, float16_t);
	#endif
	#if MUU_HAS_INTERCHANGE_FP16 && MUU_HAS_FLOAT128
		MUU_HR_SPECIALIZATION(__fp16, float128_t);
	#endif
	#if MUU_HAS_FLOAT16 && MUU_HAS_FLOAT128
		MUU_HR_SPECIALIZATION(float16_t, float128_t);
	#endif
	#undef MUU_HR_SPECIALIZATION
	template <typename... T>
	using highest_ranked = typename highest_ranked_<T...>::type;

	template <size_t Bits> struct code_unit_;
	#ifdef __cpp_char8_t
		template <> struct code_unit_<8> { using type = char8_t; };
	#else
		template <> struct code_unit_<8> { using type = unsigned char; };
	#endif
	template <> struct code_unit_<16> { using type = char16_t; };
	template <> struct code_unit_<32> { using type = char32_t; };
	using char_unicode_t = typename code_unit_<CHAR_BIT>::type;
	using wchar_unicode_t = typename code_unit_<sizeof(wchar_t) * CHAR_BIT>::type;

	template <typename T>
	using iter_reference_t = decltype(*std::declval<T&>());
	template <typename T>
	using iter_value_t = std::remove_reference_t<iter_reference_t<T>>;

	template <typename T>
	struct type_identity_ { using type = T; };

	template <typename T, bool = std::is_pointer_v<T>>
	struct pointer_rank_
	{
		static constexpr size_t value = 0;
	};
	template <typename T>
	struct pointer_rank_<T, true>
	{
		static constexpr size_t value = 1 + pointer_rank_<std::remove_pointer_t<T>>::value;
	};

	template <template <typename...> typename Trait, typename Enabler, typename... Args>
	struct is_detected_ : std::false_type {};
	template <template <typename...> typename Trait, typename... Args>
	struct is_detected_<Trait, std::void_t<Trait<Args...>>, Args...> : std::true_type {};
	template <template <typename...> typename Trait, typename... Args>
	inline constexpr auto is_detected = is_detected_<Trait, void, Args...>::value;

	template <typename T>
	using has_arrow_operator_ = decltype(std::declval<T>().operator->());
	template <typename T>
	using has_unary_plus_operator_ = decltype(+std::declval<T>());

	template <typename T>
	using has_tuple_get_member_ = decltype(std::declval<T>().template get<0>());
	template <typename T>
	using has_tuple_size_ = decltype(std::tuple_size<std::remove_cv_t<std::remove_reference_t<T>>>::value);

	template <typename T, bool = is_detected<has_tuple_size_, T>>
	struct tuple_size_ : std::tuple_size<T>{};
	template <typename T>
	struct tuple_size_<T, false>
	{
		static constexpr size_t value = 0;
	};
}
MUU_IMPL_NAMESPACE_END

MUU_NAMESPACE_START
{
	/// \addtogroup		meta
	/// @{

	/// \brief	Removes the topmost const, volatile and reference qualifiers from a type.
	/// \detail This is equivalent to C++20's std::remove_cvref_t.
	template <typename T>
	using remove_cvref = std::remove_cv_t<std::remove_reference_t<T>>;

	/// \brief	Removes the outer enum wrapper from a type, converting it to the underlying integer equivalent.
	/// \detail This is similar to std::underlying_type_t but preserves cv qualifiers and ref categories, as well as
	/// 		being safe to use in SFINAE contexts (non-enum types are simply returned as-is).
	template <typename T>
	using remove_enum = typename impl::remove_enum_<T>::type;

	/// \brief The largest type from a set of types.
	template <typename... T>
	using largest = typename impl::largest_<T...>::type;

	/// \brief The smallest type from a set of types.
	template <typename... T>
	using smallest = typename impl::smallest_<T...>::type;

	/// \brief Returns the sum of `sizeof()` for all of the types named by T.
	template <typename... T>
	inline constexpr size_t total_size = (size_t{} + ... + sizeof(T));

	/// \brief The default alignment of a type.
	/// \remarks Treats `void` and functions as having an alignment of `1`.
	template <typename T>
	inline constexpr size_t alignment_of = impl::alignment_of_<remove_cvref<T>>::value;

	/// \brief The type with the largest alignment (i.e. having the largest value for `alignment_of<T>`) from a set of types.
	/// \remarks Treats `void` and functions as having an alignment of `1`.
	template <typename... T>
	using most_aligned = typename impl::most_aligned_<T...>::type;

	/// \brief The type with the smallest alignment (i.e. having the smallest value for `alignment_of<T>`) from a set of types.
	/// \remarks Treats `void` and functions as having an alignment of `1`.
	template <typename... T>
	using least_aligned = typename impl::least_aligned_<T...>::type;

	/// \brief	True if T is exactly the same as one or more of the types named by U.
	/// \detail This equivalent to `(std::is_same_v<T, U1> || std::is_same_v<T, U2> || ...)`.
	template <typename T, typename... U>
	inline constexpr bool is_same_as_any = (false || ... || std::is_same_v<T, U>);

	/// \brief	True if all the type arguments are the same type.
	/// \detail This equivalent to `(std::is_same_v<T, U1> && std::is_same_v<T, U2> && ...)`.
	template <typename T, typename... U>
	inline constexpr bool is_same_as_all = (true && ... && std::is_same_v<T, U>);

	/// \brief	True if T is convertible to one or more of the types named by U.
	/// \detail This equivalent to `(std::is_convertible<T, U1> || std::is_convertible<T, U2> || ...)`.
	template <typename T, typename... U>
	inline constexpr bool is_convertible_to_any = (false || ... || std::is_convertible_v<T, U>);

	/// \brief	True if T is convertible to all of the types named by U.
	/// \detail This equivalent to `(std::is_convertible<T, U1> && std::is_convertible<T, U2> && ...)`.
	template <typename T, typename... U>
	inline constexpr bool is_convertible_to_all = (true && ... && std::is_convertible_v<T, U>);

	/// \brief Is a type an enum or reference-to-enum?
	template <typename T>
	inline constexpr bool is_enum = std::is_enum_v<std::remove_reference_t<T>>;

	/// \brief Are any of the named types enums or reference-to-enum?
	template <typename T, typename... U>
	inline constexpr bool any_enum = is_enum<T> || (false || ... || is_enum<U>);

	/// \brief Are all of the named types enums or reference-to-enum?
	template <typename T, typename... U>
	inline constexpr bool all_enum = is_enum<T> && (true && ... && is_enum<U>);

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
	/// \remarks Returns true for native #uint128_t (where supported).
	template <typename T>
	inline constexpr bool is_unsigned = std::is_unsigned_v<remove_enum<remove_cvref<T>>>
		#if MUU_HAS_INT128
		|| std::is_same_v<remove_enum<remove_cvref<T>>, uint128_t>
		#endif
	;

	/// \brief Are any of the named types unsigned or reference-to-unsigned?
	/// \remarks Returns true for enums backed by unsigned integers.
	template <typename T, typename... U>
	inline constexpr bool any_unsigned = is_unsigned<T> || (false || ... || is_unsigned<U>);

	/// \brief Are all of the named types unsigned or reference-to-unsigned?
	/// \remarks Returns true for enums backed by unsigned integers.
	template <typename T, typename... U>
	inline constexpr bool all_unsigned = is_unsigned<T> && (true && ... && is_unsigned<U>);

	/// \brief Is a type signed or reference-to-signed?
	/// \remarks Returns true for enums backed by signed integers.
	/// \remarks Returns true for muu::half.
	/// \remarks Returns true for native #int128_t, #float16_t and #float128_t (where supported).
	template <typename T>
	inline constexpr bool is_signed = std::is_signed_v<remove_enum<remove_cvref<T>>>
		|| is_same_as_any<remove_enum<remove_cvref<T>>,
			half
			#if MUU_HAS_INT128
				, int128_t
			#endif
			#if MUU_HAS_FLOAT128
				, float128_t
			#endif
			#if MUU_HAS_FLOAT16
				, float16_t
			#endif
			#if MUU_HAS_INTERCHANGE_FP16
				, __fp16
			#endif
		>
	;

	/// \brief Are any of the named types signed or reference-to-signed?
	/// \remarks Returns true for enums backed by signed integers.
	template <typename T, typename... U>
	inline constexpr bool any_signed = is_signed<T> || (false || ... || is_signed<U>);

	/// \brief Are all of the named types signed or reference-to-signed?
	/// \remarks Returns true for enums backed by signed integers.
	template <typename T, typename... U>
	inline constexpr bool all_signed = is_signed<T> && (true && ... && is_signed<U>);

	/// \brief Is a type an integral type or a reference to an integral type?
	/// \remarks Returns true for enums.
	/// \remarks Returns true for native #int128_t and #uint128_t (where supported).
	template <typename T>
	inline constexpr bool is_integral = std::is_integral_v<remove_enum<remove_cvref<T>>>
		#if MUU_HAS_INT128
		|| is_same_as_any<remove_enum<remove_cvref<T>>, int128_t, uint128_t>
		#endif
	;

	/// \brief Are any of the named types integral or reference-to-integral?
	/// \remarks Returns true for enums.
	/// \remarks Returns true for native #int128_t and #uint128_t (where supported).
	template <typename T, typename... U>
	inline constexpr bool any_integral = is_integral<T> || (false || ... || is_integral<U>);

	/// \brief Are all of the named types integral or reference-to-integral?
	/// \remarks Returns true for enums.
	/// \remarks Returns true for native #int128_t and #uint128_t (where supported).
	template <typename T, typename... U>
	inline constexpr bool all_integral = is_integral<T> && (true && ... && is_integral<U>);

	/// \brief Is a type a floating-point or reference-to-floating-point?
	/// \remarks Returns true for muu::half.
	/// \remarks Returns true for native #float16_t and #float128_t (where supported).
	template <typename T>
	inline constexpr bool is_floating_point = std::is_floating_point_v<std::remove_reference_t<T>>
		|| is_same_as_any<remove_cvref<T>,
			half
			#if MUU_HAS_FLOAT128
				, float128_t
			#endif
			#if MUU_HAS_FLOAT16
				, float16_t
			#endif
			#if MUU_HAS_INTERCHANGE_FP16
				, __fp16
			#endif
		>
	;

	/// \brief Are any of the named types floating-point or reference-to-floating-point?
	/// \remarks Returns true for muu::half.
	/// \remarks Returns true for native #float16_t and #float128_t (where supported).
	template <typename T, typename... U>
	inline constexpr bool any_floating_point = is_floating_point<T> || (false || ... || is_floating_point<U>);

	/// \brief Are all of the named types floating-point or reference-to-floating-point?
	/// \remarks Returns true for muu::half.
	/// \remarks Returns true for native #float16_t and #float128_t (where supported).
	template <typename T, typename... U>
	inline constexpr bool all_floating_point = is_floating_point<T> && (true && ... && is_floating_point<U>);

	/// \brief Is a type arithmetic or reference-to-arithmetic?
	/// \remarks Returns true for muu::half.
	/// \remarks Returns true for native #int128_t, #uint128_t, #float16_t and #float128_t (where supported).
	template <typename T>
	inline constexpr bool is_arithmetic = std::is_arithmetic_v<std::remove_reference_t<T>>
		|| is_same_as_any<remove_cvref<T>,
			half
			#if MUU_HAS_INT128
				, int128_t, uint128_t
			#endif
			#if MUU_HAS_FLOAT128
				, float128_t
			#endif
			#if MUU_HAS_FLOAT16
				, float16_t
			#endif
			#if MUU_HAS_INTERCHANGE_FP16
				, __fp16
			#endif
		>
	;

	/// \brief Are any of the named types arithmetic or reference-to-arithmetic?
	/// \remarks Returns true for muu::half.
	/// \remarks Returns true for native #int128_t, #uint128_t, #float16_t and #float128_t (where supported).
	template <typename T, typename... U>
	inline constexpr bool any_arithmetic = is_arithmetic<T> || (false || ... || is_arithmetic<U>);

	/// \brief Are all of the named types arithmetic or reference-to-arithmetic?
	/// \remarks Returns true for muu::half.
	/// \remarks Returns true for native #int128_t, #uint128_t, #float16_t and #float128_t (where supported).
	template <typename T, typename... U>
	inline constexpr bool all_arithmetic = is_arithmetic<T> && (true && ... && is_arithmetic<U>);

	/// \brief Is a type const or reference-to-const?
	template <typename T>
	inline constexpr bool is_const = std::is_const_v<std::remove_reference_t<T>>;

	/// \brief Adds a const qualifier to a type or reference.
	template <typename T>
	using add_const = typename impl::rebase_ref_<T, std::add_const_t<std::remove_reference_t<T>>>::type;

	/// \brief Removes the topmost const qualifier from a type or reference.
	template <typename T>
	using remove_const = typename impl::rebase_ref_<T, std::remove_const_t<std::remove_reference_t<T>>>::type;

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
	using add_volatile = typename impl::rebase_ref_<T, std::add_volatile_t<std::remove_reference_t<T>>>::type;

	/// \brief Removes the topmost volatile qualifier from a type or reference.
	template <typename T>
	using remove_volatile = typename impl::rebase_ref_<T, std::remove_volatile_t<std::remove_reference_t<T>>>::type;

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
	using add_cv = typename impl::rebase_ref_<T, std::add_volatile_t<std::add_const_t<std::remove_reference_t<T>>>>::type;

	/// \brief Removes the topmost const and volatile qualifiers from a type or reference.
	template <typename T>
	using remove_cv = typename impl::rebase_ref_<T, std::remove_volatile_t<std::remove_const_t<std::remove_reference_t<T>>>>::type;

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
	using remove_noexcept = typename impl::remove_noexcept_<T>::type;

	/// \brief Does Child inherit from Parent?
	/// \remarks This does not return true when the objects are the same type, unlike std::is_base_of.
	template <typename Parent, typename Child>
	inline constexpr bool inherits_from
		= std::is_base_of_v<remove_cvref<Parent>, remove_cvref<Child>>
		&& !std::is_same_v<remove_cvref<Parent>, remove_cvref<Child>>;

	/// \brief	Rebases a pointer, preserving the const and volatile qualification of the pointed type.
	template <typename Ptr, typename NewBase>
	using rebase_pointer = typename impl::rebase_pointer_<Ptr, NewBase>::type;

	/// \brief	Converts a numeric type to the signed equivalent with the same rank.
	/// \remarks CV qualifiers and reference categories are preserved.
	template <typename T>
	using make_signed = typename impl::make_signed_<T>::type;

	/// \brief	Converts a numeric type to the unsigned equivalent with the same rank.
	/// \remarks CV qualifiers and reference categories are preserved.
	template <typename T>
	using make_unsigned = typename impl::make_unsigned_<T>::type;

	/// \brief	Evaluates to false but with delayed, type-dependent evaluation.
	/// \details Allows you to do this:
	/// \cpp
	///	static_assert(
	///		dependent_false<T>,
	///		"Oh no, T is the wrong type for some reason"
	///	);
	/// \ecpp
	template <typename T>
	inline constexpr bool dependent_false = false;

	/// \brief	Gets the unsigned integer type with a specific number of bits for the target platform.
	template <size_t Bits>
	using unsigned_integer = typename impl::unsigned_integer_<Bits>::type;

	/// \brief	Gets the signed integer type with a specific number of bits for the target platform.
	template <size_t Bits>
	using signed_integer = typename impl::signed_integer_<Bits>::type;

	/// \brief Is a type a Unicode 'code unit' type, or reference to one?
	template <typename T>
	inline constexpr bool is_code_unit = is_same_as_any<remove_cvref<T>,
		char,
		wchar_t,
		char16_t,
		char32_t
		#ifdef __cpp_char8_t
		, char8_t
		#endif
	>;

	/// \brief	Provides an identity type transformation.
	/// \detail This is equivalent to C++20's std::type_identity_t.
	template <typename T>
	using type_identity = typename impl::type_identity_<T>::type;

	/// \brief Returns the rank of a pointer.
	/// \details Answers "how many stars does it have?".
	template <typename T>
	inline constexpr size_t pointer_rank = impl::pointer_rank_<T>::value;

	/// \brief Returns true if the type has an arrow operator.
	template <typename T>
	inline constexpr bool has_arrow_operator = impl::is_detected<impl::has_arrow_operator_, T>
		|| (std::is_pointer_v<T> && (std::is_class_v<std::remove_pointer_t<T>> || std::is_union_v<std::remove_pointer_t<T>>));

	/// \brief Returns true if the type has a unary plus operator.
	template <typename T>
	inline constexpr bool has_unary_plus_operator = impl::is_detected<impl::has_unary_plus_operator_, T>;

	/// \brief Aliases a std::aligned_storage_t with a size and alignment capable of representing all the named types.
	template <typename... T>
	using variant_storage = std::aligned_storage_t<sizeof(largest<T...>), alignof(most_aligned<T...>)>;

	/// \brief Returns true if the type implements the C++17 tuple protocol (for structured binding).
	template <typename T>
	inline constexpr bool is_tuple_like = impl::is_detected<impl::has_tuple_size_, T>
		&& impl::is_detected<impl::has_tuple_get_member_, T>;

	template <typename T, typename U>	inline constexpr bool is_tuple_like<std::pair<T, U>> = true;
	template <typename... T>			inline constexpr bool is_tuple_like<std::tuple<T...>> = true;

	/// \brief Equivalent to std::tuple_size_v, but safe to use in SFINAE contexts.
	/// \detail Returns 0 for types that do not implement std::tuple_size.
	template <typename T>
	inline constexpr size_t tuple_size = impl::tuple_size_<T>::value;

	/// @}
}
MUU_NAMESPACE_END

MUU_IMPL_NAMESPACE_START
{
	template <typename T>
	using promote_if_small_float = std::conditional_t<(is_floating_point<T> && sizeof(T) < sizeof(float)), float, T>;

	#if MUU_WINDOWS
		template <typename T> inline constexpr bool is_win32_iunknown
			= std::is_class_v<remove_cvref<T>>
			&& std::is_base_of_v<IUnknown, T>;
	#else
		template <typename T> inline constexpr bool is_win32_iunknown = false;
	#endif
}
MUU_IMPL_NAMESPACE_END

//=====================================================================================================================
// CONSTANTS
//=====================================================================================================================

MUU_PUSH_WARNINGS
MUU_DISABLE_ARITHMETIC_WARNINGS

MUU_NAMESPACE_START
{
	namespace impl
	{
		//------------- helpers

		template <typename T, int64_t Base, int64_t Power>
		struct power_helper
		{
			static constexpr T value = T{ Base } * power_helper<T, Base, Power - 1>::value;
		};
		template <typename T, int64_t Base>
		struct power_helper<T, Base, 0>
		{
			static constexpr T value = T{ 1 };
		};
		template <typename T, int64_t Base, int64_t Power>
		struct power
		{
			static constexpr int64_t exponent = Power < 0 ? -Power : Power;
			static constexpr T value = Power < 0
				? T{ 1 } / T{ power_helper<T, Base, exponent>::value }
				: T{ power_helper<T, Base, exponent>::value };
		};

		//------------- standalone 'trait' constant classes

		template <typename T>
		struct integer_limits
		{
			/// \brief The lowest representable 'normal' value (equivalent to std::numeric_limits::lowest()).
			static constexpr T lowest = std::numeric_limits<T>::lowest();

			/// \brief The highest representable 'normal' value (equivalent to std::numeric_limits::max()).
			static constexpr T highest = (std::numeric_limits<T>::max)();
		};

		#if MUU_HAS_INT128
		template <>
		struct integer_limits<int128_t>
		{
			static constexpr int128_t highest = static_cast<int128_t>(
				(uint128_t{ 1u } << ((__SIZEOF_INT128__ * CHAR_BIT) - 1)) - 1
			);
			static constexpr int128_t lowest = -highest - int128_t{ 1 };
		};
		template <>
		struct integer_limits<uint128_t>
		{
			static constexpr uint128_t lowest = uint128_t{};
			static constexpr uint128_t highest = (2u * static_cast<uint128_t>(integer_limits<int128_t>::highest)) + 1u;
		};
		#endif // MUU_HAS_INT128

		template <typename T>
		struct integer_positive_constants
		{
			static constexpr T zero = T{ 0 };	///< `0`
			static constexpr T one = T{ 1 };	///< `1`
			static constexpr T two = T{ 2 };	///< `2`
			static constexpr T three = T{ 3 };	///< `3`
			static constexpr T four = T{ 4 };	///< `4`
			static constexpr T five = T{ 5 };	///< `5`
			static constexpr T six = T{ 6 };	///< `6`
			static constexpr T seven = T{ 7 };	///< `7`
			static constexpr T eight = T{ 8 };	///< `8`
			static constexpr T nine = T{ 9 };	///< `9`
			static constexpr T ten = T{ 10 };	///< `10`
		};

		template <typename T>
		struct integer_negative_constants
		{
			static constexpr T minus_one = T{ -1 };		///< `-1`
			static constexpr T minus_two = T{ -2 };		///< `-2`
			static constexpr T minus_three = T{ -3 };	///< `-3`
			static constexpr T minus_four = T{ -4 };	///< `-4`
			static constexpr T minus_five = T{ -5 };	///< `-5`
			static constexpr T minus_six = T{ -6 };		///< `-6`
			static constexpr T minus_seven = T{ -7 };	///< `-7`
			static constexpr T minus_eight = T{ -8 };	///< `-8`
			static constexpr T minus_nine = T{ -9 };	///< `-9`
			static constexpr T minus_ten = T{ -10 };	///< `-10`
		};

		template <typename T>
		struct floating_point_limits
		{
			/// \brief The number of significand (mantissa) digits.
			static constexpr int significand_digits = std::numeric_limits<T>::digits;

			static constexpr T approx_equal_epsilon = T{ 1 } * power<T, 10, -std::numeric_limits<T>::digits10>::value;
		};
		#if MUU_HAS_INTERCHANGE_FP16
		template <>
		struct floating_point_limits<__fp16>
		{
			static constexpr int significand_digits = 11;
			static constexpr __fp16 approx_equal_epsilon = static_cast<__fp16>(0.001);
		};
		#endif
		#if MUU_HAS_FLOAT16
		template <>
		struct floating_point_limits<float16_t>
		{
			static constexpr int significand_digits = 11;
			static constexpr float16_t approx_equal_epsilon = static_cast<float16_t>(0.001);
		};
		#endif
		#if MUU_HAS_FLOAT128
		template <>
		struct floating_point_limits<float128_t>
		{
			static constexpr int significand_digits = __FLT128_MANT_DIG__;
			static constexpr float128_t approx_equal_epsilon = float128_t{ 1 } * power<float128_t, 10, -__FLT128_DIG__>::value;
		};
		#endif

		template <typename T>
		struct floating_point_special_constants
		{
			static constexpr T nan = std::numeric_limits<T>::quiet_NaN();		///< Not-A-Number (quiet)
			static constexpr T snan = std::numeric_limits<T>::signaling_NaN();	///< Not-A-Number (signalling)
			static constexpr T infinity = std::numeric_limits<T>::infinity();	///< Positive infinity
			static constexpr T negative_infinity = -infinity;					///< Negative infinity
			static constexpr T minus_zero = T(-0.0L);							///< `-0.0`
			//
		};

		template <typename T>
		struct floating_point_named_constants
		{
			static constexpr T one_over_two          = T( 0.500000000000000000L ); ///< `1 / 2`
			static constexpr T two_over_three        = T( 0.666666666666666667L ); ///< `2 / 3`
			static constexpr T two_over_five         = T( 0.400000000000000000L ); ///< `2 / 5`
			static constexpr T sqrt_two              = T( 1.414213562373095049L ); ///< `sqrt(2)`
			static constexpr T one_over_sqrt_two     = T( 0.707106781186547524L ); ///< `1 / sqrt(2)`
			static constexpr T one_over_three        = T( 0.333333333333333333L ); ///< `1 / 3`
			static constexpr T three_over_two        = T( 1.500000000000000000L ); ///< `3 / 2`
			static constexpr T three_over_four       = T( 0.750000000000000000L ); ///< `3 / 4`
			static constexpr T three_over_five       = T( 0.600000000000000000L ); ///< `3 / 5`
			static constexpr T sqrt_three            = T( 1.732050807568877294L ); ///< `sqrt(3)`
			static constexpr T one_over_sqrt_three   = T( 0.577350269189625765L ); ///< `1 / sqrt(3)`
			static constexpr T pi                    = T( 3.141592653589793238L ); ///< `pi`
			static constexpr T one_over_pi           = T( 0.318309886183790672L ); ///< `1 / pi`
			static constexpr T pi_over_two           = T( 1.570796326794896619L ); ///< `pi / 2`
			static constexpr T pi_over_three         = T( 1.047197551196597746L ); ///< `pi / 3`
			static constexpr T pi_over_four          = T( 0.785398163397448310L ); ///< `pi / 4`
			static constexpr T pi_over_five          = T( 0.628318530717958648L ); ///< `pi / 5`
			static constexpr T pi_over_six           = T( 0.523598775598298873L ); ///< `pi / 6`
			static constexpr T sqrt_pi               = T( 1.772453850905516027L ); ///< `sqrt(pi)`
			static constexpr T one_over_sqrt_pi      = T( 0.564189583547756287L ); ///< `1 / sqrt(pi)`
			static constexpr T two_pi                = T( 6.283185307179586477L ); ///< `pi`
			static constexpr T sqrt_two_pi           = T( 2.506628274631000502L ); ///< `sqrt(2 * pi)`
			static constexpr T one_over_sqrt_two_pi  = T( 0.398942280401432678L ); ///< `1 / sqrt(2 * pi)`
			static constexpr T e                     = T( 2.718281828459045535L ); ///< `e`
			static constexpr T one_over_e            = T( 0.367879441171442281L ); ///< `1 / e`
			static constexpr T e_over_two            = T( 1.359140914229522767L ); ///< `e / 2`
			static constexpr T e_over_three          = T( 0.906093942819681845L ); ///< `e / 3`
			static constexpr T e_over_four           = T( 0.679570457114761384L ); ///< `e / 4`
			static constexpr T e_over_five           = T( 0.543656365691809107L ); ///< `e / 5`
			static constexpr T e_over_six            = T( 0.453046971409840922L ); ///< `e / 6`
			static constexpr T sqrt_e                = T( 1.648721270700128238L ); ///< `sqrt(e)`
			static constexpr T one_over_sqrt_e       = T( 0.606530659712633390L ); ///< `1 / sqrt(e)`
			static constexpr T phi                   = T( 1.618033988749894848L ); ///< `phi`
			static constexpr T one_over_phi          = T( 0.618033988749894848L ); ///< `1 / phi`
			static constexpr T phi_over_two          = T( 0.809016994374947424L ); ///< `phi / 2`
			static constexpr T phi_over_three        = T( 0.539344662916631616L ); ///< `phi / 3`
			static constexpr T phi_over_four         = T( 0.404508497187473712L ); ///< `phi / 4`
			static constexpr T phi_over_five         = T( 0.323606797749978970L ); ///< `phi / 5`
			static constexpr T phi_over_six          = T( 0.269672331458315808L ); ///< `phi / 6`
			static constexpr T sqrt_phi              = T( 1.272019649514068964L ); ///< `sqrt(phi)`
			static constexpr T one_over_sqrt_phi     = T( 0.786151377757423286L ); ///< `1 / sqrt(phi)`
		};

		#if MUU_HAS_FLOAT128 && MUU_EXTENDED_LITERALS
		template <>
		struct floating_point_named_constants<float128_t>
		{
			static constexpr float128_t one_over_two          = 0.500000000000000000000000000000000q;
			static constexpr float128_t two_over_three        = 0.666666666666666666666666666666667q;
			static constexpr float128_t two_over_five         = 0.400000000000000000000000000000000q;
			static constexpr float128_t sqrt_two              = 1.414213562373095048801688724209698q;
			static constexpr float128_t one_over_sqrt_two     = 0.707106781186547524400844362104849q;
			static constexpr float128_t one_over_three        = 0.333333333333333333333333333333333q;
			static constexpr float128_t three_over_two        = 1.500000000000000000000000000000000q;
			static constexpr float128_t three_over_four       = 0.750000000000000000000000000000000q;
			static constexpr float128_t three_over_five       = 0.600000000000000000000000000000000q;
			static constexpr float128_t sqrt_three            = 1.732050807568877293527446341505872q;
			static constexpr float128_t one_over_sqrt_three   = 0.577350269189625764509148780501957q;
			static constexpr float128_t pi                    = 3.141592653589793238462643383279503q;
			static constexpr float128_t one_over_pi           = 0.318309886183790671537767526745029q;
			static constexpr float128_t pi_over_two           = 1.570796326794896619231321691639751q;
			static constexpr float128_t pi_over_three         = 1.047197551196597746154214461093168q;
			static constexpr float128_t pi_over_four          = 0.785398163397448309615660845819876q;
			static constexpr float128_t pi_over_five          = 0.628318530717958647692528676655901q;
			static constexpr float128_t pi_over_six           = 0.523598775598298873077107230546584q;
			static constexpr float128_t sqrt_pi               = 1.772453850905516027298167483341145q;
			static constexpr float128_t one_over_sqrt_pi      = 0.564189583547756286948079451560773q;
			static constexpr float128_t two_pi                = 6.283185307179586476925286766559006q;
			static constexpr float128_t sqrt_two_pi           = 2.506628274631000502415765284811045q;
			static constexpr float128_t one_over_sqrt_two_pi  = 0.398942280401432677939946059934382q;
			static constexpr float128_t e                     = 2.718281828459045534884808148490265q;
			static constexpr float128_t one_over_e            = 0.367879441171442281059287928010393q;
			static constexpr float128_t e_over_two            = 1.359140914229522767442404074245133q;
			static constexpr float128_t e_over_three          = 0.906093942819681844961602716163422q;
			static constexpr float128_t e_over_four           = 0.679570457114761383721202037122566q;
			static constexpr float128_t e_over_five           = 0.543656365691809106976961629698053q;
			static constexpr float128_t e_over_six            = 0.453046971409840922480801358081711q;
			static constexpr float128_t sqrt_e                = 1.648721270700128237684053351021452q;
			static constexpr float128_t one_over_sqrt_e       = 0.606530659712633390187322401455486q;
			static constexpr float128_t phi                   = 1.618033988749894848204586834365638q;
			static constexpr float128_t one_over_phi          = 0.618033988749894848204586834365638q;
			static constexpr float128_t phi_over_two          = 0.809016994374947424102293417182819q;
			static constexpr float128_t phi_over_three        = 0.539344662916631616068195611455213q;
			static constexpr float128_t phi_over_four         = 0.404508497187473712051146708591410q;
			static constexpr float128_t phi_over_five         = 0.323606797749978969640917366873128q;
			static constexpr float128_t phi_over_six          = 0.269672331458315808034097805727606q;
			static constexpr float128_t sqrt_phi              = 1.272019649514068964252422461737491q;
			static constexpr float128_t one_over_sqrt_phi     = 0.786151377757423286069558585842959q;
		};
		#endif

		//-------------  constant class aggregates

		template <typename T>
		struct ascii_character_constants
			: integer_limits<T>,
			integer_positive_constants<T>
		{
			static constexpr T backspace = T{ 8 };				///< The backspace character.
			static constexpr T tab = T{ 9 };					///< `\t`
			static constexpr T line_feed = T{ 10 };				///< `\n`
			static constexpr T vertical_tab = T{ 11 };			///< `\v`
			static constexpr T form_feed = T{ 12 };				///< `\f`
			static constexpr T carriage_return = T{ 13 };		///< `\r`
			static constexpr T escape = T{ 27 };				///< `ESC`

			static constexpr T space = T{ 32 };					///< `&nbsp;` (space)
			static constexpr T exclamation_mark = T{ 33 };		///< `!`
			static constexpr T quote = T{ 34 };					///< `"`
			static constexpr T number_sign = T{ 35 };			///< `#`
			static constexpr T dollar_sign = T{ 36 };			///< `$`
			static constexpr T percent = T{ 37 };				///< `%`
			static constexpr T ampersand = T{ 38 };				///< `&amp;`
			static constexpr T apostrophe = T{ 39 };			///< `&apos;`

			static constexpr T left_parenthesis = T{ 40 };		///< `(`
			static constexpr T right_parenthesis = T{ 41 };		///< `)`
			static constexpr T asterisk = T{ 42 };				///< `*`
			static constexpr T plus = T{ 43 };					///< `+`
			static constexpr T comma = T{ 44 };					///< `,`
			static constexpr T hyphen = T{ 45 };				///< `-`
			static constexpr T period = T{ 46 };				///< `.`
			static constexpr T forward_slash = T{ 47 };			///< `/`

			static constexpr T digit_0 = T{ 48 };				///< `0`
			static constexpr T digit_1 = T{ 49 };				///< `1`
			static constexpr T digit_2 = T{ 50 };				///< `2`
			static constexpr T digit_3 = T{ 51 };				///< `3`
			static constexpr T digit_4 = T{ 52 };				///< `4`
			static constexpr T digit_5 = T{ 53 };				///< `5`
			static constexpr T digit_6 = T{ 54 };				///< `6`
			static constexpr T digit_7 = T{ 55 };				///< `7`
			static constexpr T digit_8 = T{ 56 };				///< `8`
			static constexpr T digit_9 = T{ 57 };				///< `9`

			static constexpr T colon = T{ 58 };					///< `:`
			static constexpr T semi_colon = T{ 59 };			///< `;`
			static constexpr T less_than = T{ 60 };				///< `&lt;`
			static constexpr T equal = T{ 61 };					///< `=`
			static constexpr T greater_than = T{ 62 };			///< `&gt;`
			static constexpr T question_mark = T{ 63 };			///< `?`
			static constexpr T at = T{ 64 };					///< `@`

			static constexpr T letter_A = T{ 65 };				///< `A`
			static constexpr T letter_B = T{ 66 };				///< `B`
			static constexpr T letter_C = T{ 67 };				///< `C`
			static constexpr T letter_D = T{ 68 };				///< `D`
			static constexpr T letter_E = T{ 69 };				///< `E`
			static constexpr T letter_F = T{ 70 };				///< `F`
			static constexpr T letter_G = T{ 71 };				///< `G`
			static constexpr T letter_H = T{ 72 };				///< `H`
			static constexpr T letter_I = T{ 73 };				///< `I`
			static constexpr T letter_J = T{ 74 };				///< `J`
			static constexpr T letter_K = T{ 75 };				///< `K`
			static constexpr T letter_L = T{ 76 };				///< `L`
			static constexpr T letter_M = T{ 77 };				///< `M`
			static constexpr T letter_N = T{ 78 };				///< `N`
			static constexpr T letter_O = T{ 79 };				///< `O`
			static constexpr T letter_P = T{ 80 };				///< `P`
			static constexpr T letter_Q = T{ 81 };				///< `Q`
			static constexpr T letter_R = T{ 82 };				///< `R`
			static constexpr T letter_S = T{ 83 };				///< `S`
			static constexpr T letter_T = T{ 84 };				///< `T`
			static constexpr T letter_U = T{ 85 };				///< `U`
			static constexpr T letter_V = T{ 86 };				///< `V`
			static constexpr T letter_W = T{ 87 };				///< `W`
			static constexpr T letter_X = T{ 88 };				///< `X`
			static constexpr T letter_Y = T{ 89 };				///< `Y`
			static constexpr T letter_Z = T{ 90 };				///< `Z`

			static constexpr T left_square_bracket = T{ 91 };	///< `[`
			static constexpr T back_slash = T{ 92 };			///< `\\`
			static constexpr T right_square_bracket = T{ 93 };	///< `]`
			static constexpr T hat = T{ 94 };					///< `^`
			static constexpr T underscore = T{ 95 };			///<  `_`
			static constexpr T backtick = T{ 96 };				///<  `&#96;` (backtick)

			static constexpr T letter_a = T{ 97 };				///< `a`
			static constexpr T letter_b = T{ 98 };				///< `b`
			static constexpr T letter_c = T{ 99 };				///< `c`
			static constexpr T letter_d = T{ 100 };				///< `d`
			static constexpr T letter_e = T{ 101 };				///< `e`
			static constexpr T letter_f = T{ 102 };				///< `f`
			static constexpr T letter_g = T{ 103 };				///< `g`
			static constexpr T letter_h = T{ 104 };				///< `h`
			static constexpr T letter_i = T{ 105 };				///< `i`
			static constexpr T letter_j = T{ 106 };				///< `j`
			static constexpr T letter_k = T{ 107 };				///< `k`
			static constexpr T letter_l = T{ 108 };				///< `l`
			static constexpr T letter_m = T{ 109 };				///< `m`
			static constexpr T letter_n = T{ 110 };				///< `n`
			static constexpr T letter_o = T{ 111 };				///< `o`
			static constexpr T letter_p = T{ 112 };				///< `p`
			static constexpr T letter_q = T{ 113 };				///< `q`
			static constexpr T letter_r = T{ 114 };				///< `r`
			static constexpr T letter_s = T{ 115 };				///< `s`
			static constexpr T letter_t = T{ 116 };				///< `t`
			static constexpr T letter_u = T{ 117 };				///< `u`
			static constexpr T letter_v = T{ 118 };				///< `v`
			static constexpr T letter_w = T{ 119 };				///< `w`
			static constexpr T letter_x = T{ 120 };				///< `x`
			static constexpr T letter_y = T{ 121 };				///< `y`
			static constexpr T letter_z = T{ 122 };				///< `z`

			static constexpr T left_brace = T{ 123 };			///< `{`
			static constexpr T bar = T{ 124 };					///< `|`
			static constexpr T right_brace = T{ 125 };			///< `}`
			static constexpr T tilde = T{ 126 };				///< `~`
			static constexpr T del = T{ 127 };					///< `DEL`
		};

		template <typename T>
		struct floating_point_constants
			: integer_limits<T>,
			integer_positive_constants<T>,
			integer_negative_constants<T>,
			floating_point_limits<T>,
			floating_point_special_constants<T>,
			floating_point_named_constants<T>
		{};

		template <typename T>
		struct unsigned_integral_constants
			: integer_limits<T>,
			integer_positive_constants<T>
		{};

		template <typename T>
		struct signed_integral_constants
			: integer_limits<T>,
			integer_positive_constants<T>,
			integer_negative_constants<T>
		{};
	}

	/// \addtogroup		constants
	/// @{

	/// \brief	`float` constants.
	template <> struct constants<float> : impl::floating_point_constants<float> {};

	/// \brief	`double` constants.
	template <> struct constants<double> : impl::floating_point_constants<double> {};

	/// \brief	`long double` constants.
	template <> struct constants<long double> : impl::floating_point_constants<long double> {};

	#if MUU_HAS_INTERCHANGE_FP16
	template <> struct constants<__fp16> : impl::floating_point_constants<__fp16> {};
	#endif

	#if MUU_HAS_FLOAT16
	/// \brief	`float16_t` constants.
	template <> struct constants<float16_t> : impl::floating_point_constants<float16_t> {};
	#endif

	#if MUU_HAS_FLOAT128
	/// \brief	`float128_t` constants.
	template <> struct constants<float128_t> : impl::floating_point_constants<float128_t> {};
	#endif

	/// \brief	`char` constants.
	template <> struct constants<char> : impl::ascii_character_constants<char> {};

	/// \brief	`wchar_t` constants.
	template <> struct constants<wchar_t> : impl::ascii_character_constants<wchar_t> {};

	#ifdef __cpp_char8_t
	/// \brief	`char8_t` constants.
	template <> struct constants<char8_t> : impl::ascii_character_constants<char8_t> {};
	#endif

	/// \brief	`char16_t` constants.
	template <> struct constants<char16_t> : impl::ascii_character_constants<char16_t> {};

	/// \brief	`char32_t` constants.
	template <> struct constants<char32_t> : impl::ascii_character_constants<char32_t> {};

	/// \brief	`signed char` constants.
	template <> struct constants<signed char> : impl::signed_integral_constants<signed char> {};

	/// \brief	`unsigned char` constants.
	template <> struct constants<unsigned char> : impl::unsigned_integral_constants<unsigned char> {};

	/// \brief	`signed short` constants.
	template <> struct constants<signed short> : impl::signed_integral_constants<signed short> {};

	/// \brief	`unsigned short` constants.
	template <> struct constants<unsigned short> : impl::unsigned_integral_constants<unsigned short> {};

	/// \brief	`signed int` constants.
	template <> struct constants<signed int> : impl::signed_integral_constants<signed int> {};

	/// \brief	`unsigned int` constants.
	template <> struct constants<unsigned int> : impl::unsigned_integral_constants<unsigned int> {};

	/// \brief	`signed long` constants.
	template <> struct constants<signed long> : impl::signed_integral_constants<signed long> {};

	/// \brief	`unsigned long` constants.
	template <> struct constants<unsigned long> : impl::unsigned_integral_constants<unsigned long> {};

	/// \brief	`signed long long` constants.
	template <> struct constants<signed long long> : impl::signed_integral_constants<signed long long> {};

	/// \brief	`unsigned long long` constants.
	template <> struct constants<unsigned long long> : impl::unsigned_integral_constants<unsigned long long> {};

	#if MUU_HAS_INT128
	/// \brief	`int128_t` constants.
	template <> struct constants<int128_t> : impl::signed_integral_constants<int128_t> {};

	/// \brief	`uint128_t` constants.
	template <> struct constants<uint128_t> : impl::unsigned_integral_constants<uint128_t> {};
	#endif

	/// @}
}
MUU_NAMESPACE_END

//=====================================================================================================================
// LITERALS, BUILD CONSTANTS AND 'INTRINSICS'
//=====================================================================================================================

MUU_NAMESPACE_START
{
	inline namespace literals
	{
		/// \brief	Creates a size_t.
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(const)
		MUU_CONSTEVAL size_t operator"" _sz(unsigned long long n) noexcept
		{
			return static_cast<size_t>(n);
		}

		/// \brief	Creates a uint8_t.
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(const)
		MUU_CONSTEVAL uint8_t operator"" _u8(unsigned long long n) noexcept
		{
			return static_cast<uint8_t>(n);
		}

		/// \brief	Creates a uint16_t.
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(const)
		MUU_CONSTEVAL uint16_t operator"" _u16(unsigned long long n) noexcept
		{
			return static_cast<uint16_t>(n);
		}

		/// \brief	Creates a uint32_t.
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(const)
		MUU_CONSTEVAL uint32_t operator"" _u32(unsigned long long n) noexcept
		{
			return static_cast<uint32_t>(n);
		}

		/// \brief	Creates a uint64_t.
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(const)
		MUU_CONSTEVAL uint64_t operator"" _u64(unsigned long long n) noexcept
		{
			return static_cast<uint64_t>(n);
		}

		/// \brief	Creates a int8_t.
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(const)
		MUU_CONSTEVAL int8_t operator"" _i8(unsigned long long n) noexcept
		{
			return static_cast<int8_t>(n);
		}

		/// \brief	Creates a int16_t.
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(const)
		MUU_CONSTEVAL int16_t operator"" _i16(unsigned long long n) noexcept
		{
			return static_cast<int16_t>(n);
		}

		/// \brief	Creates a int32_t.
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(const)
		MUU_CONSTEVAL int32_t operator"" _i32(unsigned long long n) noexcept
		{
			return static_cast<int32_t>(n);
		}

		/// \brief	Creates a int64_t.
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(const)
		MUU_CONSTEVAL int64_t operator"" _i64(unsigned long long n) noexcept
		{
			return static_cast<int64_t>(n);
		}

		/// \brief	Creates a std::byte.
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(const)
		MUU_CONSTEVAL std::byte operator"" _byte(unsigned long long n) noexcept
		{
			return static_cast<std::byte>(n);
		}

		#if MUU_HAS_INT128

		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(const)
		MUU_CONSTEVAL int128_t operator"" _i128(unsigned long long n) noexcept
		{
			return static_cast<int128_t>(n);
		}

		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(const)
		MUU_CONSTEVAL uint128_t operator"" _u128(unsigned long long n) noexcept
		{
			return static_cast<uint128_t>(n);
		}

		#endif
	}

	namespace impl
	{
		MUU_DISABLE_WARNINGS // non-determinisitic build

		inline constexpr auto date_str = __DATE__;
		inline constexpr auto date_month_hash = date_str[0] + date_str[1] + date_str[2];
		inline constexpr auto time_str = __TIME__;

		MUU_ENABLE_WARNINGS
	}

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

		/// \brief True if exceptions are enabled.
		inline constexpr bool has_exceptions = !!MUU_EXCEPTIONS;

		/// \brief True if run-time type identification (RTTI) is enabled.
		inline constexpr bool has_rtti = !!MUU_RTTI;

		/// \brief True if the target environment is little-endian.
		inline constexpr bool is_little_endian = !!MUU_LITTLE_ENDIAN;

		/// \brief True if the target environment is big-endian.
		inline constexpr bool is_big_endian = !!MUU_BIG_ENDIAN;
	
		static_assert(pointer_size * bits_per_byte == bitness);
		static_assert(is_little_endian != is_big_endian);
	
	} //::build

	/// \brief	Equivalent to C++20's std::is_constant_evaluated.
	/// \ingroup	intrinsics
	///
	/// \detail Compilers typically implement std::is_constant_evaluated as an intrinsic which is
	/// 		 available regardless of the C++ mode. Using this function on these compilers allows
	/// 		 you to get the same behaviour even when you aren't targeting C++20.
	/// 
	/// \remark On older compilers lacking support for std::is_constant_evaluated this will always return `false`.
	/// 		   You can check for support by examining build::supports_is_constant_evaluated.
	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(flatten)
	constexpr bool is_constant_evaluated() noexcept
	{
		#if MUU_CLANG >= 9			\
			|| MUU_GCC >= 9			\
			|| (MUU_MSVC >= 1925 && !MUU_INTELLISENSE)
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

	/// \brief	Equivalent to C++17's std::launder
	/// \ingroup	intrinsics
	///
	/// \detail Older implementations don't provide this as an intrinsic or have a placeholder
	/// 		 for it in their standard library. Using this version allows you to get around that 
	/// 		 by writing code 'as if' it were there and have it compile just the same.
	template <class T>
	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(flatten)
	constexpr T* launder(T* ptr) noexcept
	{
		static_assert(
			!std::is_function_v<T> && !std::is_void_v<T>,
			"launder() may not be used on pointers to functions or void."
		);

		#if MUU_CLANG >= 8		\
			|| MUU_GCC >= 7		\
			|| MUU_ICC >= 1910	\
			|| MUU_MSVC >= 1914
			return __builtin_launder(ptr);
		#elif defined(__cpp_lib_launder)
			return std::launder(ptr);
		#else
			return ptr;
		#endif
	}

	/// \brief	Unwraps an enum to it's raw integer equivalent.
	/// \ingroup	intrinsics
	///
	/// \tparam	T		An enum type.
	/// \param 	val		The value to unwrap.
	///
	/// \returns	<strong><em>Enum inputs:</em></strong> `static_cast<std::underlying_type_t<T>>(val)` <br>
	/// 			<strong><em>Everything else:</em></strong> A straight pass-through of the input (a no-op).
	template <typename T MUU_SFINAE(is_enum<T>)>
	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(const)
	constexpr auto MUU_VECTORCALL unwrap(T val) noexcept
	{
		return static_cast<std::underlying_type_t<T>>(val);
	}

	#ifndef DOXYGEN

	template <typename T, std::enable_if_t<!is_enum<T>>* = nullptr>
	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(const)
	MUU_ATTR(flatten)
	constexpr decltype(auto) unwrap(T&& val) noexcept
	{
		return std::forward<T>(val);
	}

	#endif // !DOXYGEN

	namespace impl
	{
		template <typename T>
		[[nodiscard]]
		MUU_ATTR(const)
		constexpr int MUU_VECTORCALL countl_zero_native(T val) noexcept
		{
			MUU_ASSUME(val > T{});
			
			using bit_type = largest<T, unsigned>;
			int count = 0;
			bit_type bit = bit_type{ 1 } << (sizeof(T) * CHAR_BIT - 1);
			while (true)
			{
				if ((bit & val))
					break;
				count++;
				bit >>= 1;
			}
			return count;
		}

		template <typename T>
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(const)
		int MUU_VECTORCALL countl_zero_intrinsic(T val) noexcept
		{
			MUU_ASSUME(val > T{});

			#if MUU_GCC || MUU_CLANG
			{
				#define MUU_HAS_INTRINSIC_COUNTL_ZERO 1

				if constexpr (std::is_same_v<T, unsigned long long>)
					return __builtin_clzll(val);
				else if constexpr (std::is_same_v<T, unsigned long>)
					return __builtin_clzl(val);
				else if constexpr (std::is_same_v<T, unsigned int> || sizeof(T) == sizeof(unsigned int))
					return __builtin_clz(static_cast<unsigned int>(val));
				else if constexpr (sizeof(T) < sizeof(unsigned int))
					return __builtin_clz(val) - static_cast<int>((sizeof(unsigned int) - sizeof(T)) * CHAR_BIT);
				else
					static_assert(dependent_false<T>, "Evaluated unreachable branch!");
			}
			#elif MUU_MSVC || MUU_ICC_CL
			{
				#define MUU_HAS_INTRINSIC_COUNTL_ZERO 1
				if constexpr (sizeof(T) == sizeof(unsigned long long))
				{
					#if MUU_ARCH_X64
					{
						unsigned long p;
						_BitScanReverse64(&p, static_cast<unsigned long long>(val));
						return 63 - static_cast<int>(p);
					}
					#else
					{
						if (const auto high = static_cast<unsigned long>(val >> 32); high != 0)
							return countl_zero_intrinsic(high);
						return 32 + countl_zero_intrinsic(static_cast<unsigned long>(val));
					}
					#endif

				}
				else if constexpr (sizeof(T) == sizeof(unsigned long))
				{
					unsigned long p;
					_BitScanReverse(&p, static_cast<unsigned long>(val));
					return 31 - static_cast<int>(p);
				}
				else if constexpr (sizeof(T) < sizeof(unsigned long))
					return countl_zero_intrinsic(static_cast<unsigned long>(val)) - static_cast<int>((sizeof(unsigned long) - sizeof(T)) * CHAR_BIT);
				else
					static_assert(dependent_false<T>, "Evaluated unreachable branch!");
			}
			#else
			{
				#define MUU_HAS_INTRINSIC_COUNTL_ZERO 0

				static_assert(dependent_false<T>, "countl_zero not implemented on this compiler");
			}
			#endif
		}
	}

	/// \brief	Counts the number of consecutive 0 bits, starting from the left.
	/// \ingroup	intrinsics
	///
	/// \detail This is equivalent to C++20's std::countl_zero, with the addition of also being
	/// 		 extended to work with enum types.
	/// 
	/// \tparam	T		An unsigned integer or enum type.
	/// \param 	val		The value to test.
	///
	/// \returns	The number of consecutive zeros from the left end of an integer's bits.
	template <typename T MUU_SFINAE(is_unsigned<T>)>
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr int MUU_VECTORCALL countl_zero(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return countl_zero(unwrap(val));

		#if MUU_HAS_INT128
		else if constexpr (std::is_same_v<T, uint128_t>)
		{
			if (const auto high = countl_zero(static_cast<uint64_t>(val >> 64)); high < 64)
				return high;
			else
				return 64 + countl_zero(static_cast<uint64_t>(val));
		}
		#endif

		else
		{
			if (!val)
				return static_cast<int>(sizeof(T) * CHAR_BIT);

			if constexpr (!build::supports_is_constant_evaluated || !MUU_HAS_INTRINSIC_COUNTL_ZERO)
				return impl::countl_zero_native(val);
			else
			{
				if (is_constant_evaluated())
					return impl::countl_zero_native(val);
				else
					return impl::countl_zero_intrinsic(val);
			}
		}
	}

	namespace impl
	{
		template <typename T>
		[[nodiscard]]
		MUU_ATTR(const)
		constexpr int MUU_VECTORCALL countr_zero_native(T val) noexcept
		{
			MUU_ASSUME(val > T{});

			using bit_type = largest<T, unsigned>;
			int count = 0;
			bit_type bit = 1;
			while (true)
			{
				if ((bit & val))
					break;
				count++;
				bit <<= 1;
			}
			return count;
		}

		template <typename T>
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(const)
		int MUU_VECTORCALL countr_zero_intrinsic(T val) noexcept
		{
			MUU_ASSUME(val > T{});

			#if MUU_GCC || MUU_CLANG
			{
				#define MUU_HAS_INTRINSIC_COUNTR_ZERO 1

				if constexpr (std::is_same_v<T, unsigned long long>)
					return __builtin_ctzll(val);
				else if constexpr (std::is_same_v<T, unsigned long>)
					return __builtin_ctzl(val);
				else if constexpr (std::is_same_v<T, unsigned int> || sizeof(T) <= sizeof(unsigned int))
					return __builtin_ctz(val);
				else
					static_assert(dependent_false<T>, "Evaluated unreachable branch!");
			}
			#elif MUU_MSVC || MUU_ICC_CL
			{
				#define MUU_HAS_INTRINSIC_COUNTR_ZERO 1

				if constexpr (sizeof(T) == sizeof(unsigned long long))
				{
					#if MUU_ARCH_X64
					{
						unsigned long p;
						_BitScanForward64(&p, static_cast<unsigned long long>(val));
						return static_cast<int>(p);
					}
					#else
					{
						if (const auto low = static_cast<unsigned long>(val); low != 0)
							return countr_zero_intrinsic(low);
						return 32 + countr_zero_intrinsic(static_cast<unsigned long>(val >> 32));
					}
					#endif
				}
				else if constexpr (sizeof(T) == sizeof(unsigned long))
				{
					unsigned long p;
					_BitScanForward(&p, static_cast<unsigned long>(val));
					return static_cast<int>(p);
				}
				else if constexpr (sizeof(T) < sizeof(unsigned long))
					return countr_zero_intrinsic(static_cast<unsigned long>(val));
				else
					static_assert(dependent_false<T>, "Evaluated unreachable branch!");
			}
			#else
			{
				#define MUU_HAS_INTRINSIC_COUNTR_ZERO 0

				static_assert(dependent_false<T>, "countr_zero not implemented on this compiler");
			}
			#endif
		}
	}

	/// \brief	Counts the number of consecutive 0 bits, starting from the right.
	/// \ingroup	intrinsics
	///
	/// \detail This is equivalent to C++20's std::countr_zero, with the addition of also being
	/// 		 extended to work with enum types.
	/// 		 
	/// \tparam	T		An unsigned integer or enum type.
	/// \param 	val		The input value.
	///
	/// \returns	The number of consecutive zeros from the right end of an integer's bits.
	template <typename T MUU_SFINAE(is_unsigned<T>)>
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr int MUU_VECTORCALL countr_zero(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return countr_zero(unwrap(val));
		
		#if MUU_HAS_INT128
		else if constexpr (std::is_same_v<T, uint128_t>)
		{
			if (const auto low = countr_zero(static_cast<uint64_t>(val)); low < 64)
				return low;
			else
				return 64 + countr_zero(static_cast<uint64_t>(val >> 64));
		}
		#endif

		else
		{
			if (!val)
				return static_cast<int>(sizeof(T) * CHAR_BIT);

			if constexpr (!build::supports_is_constant_evaluated || !MUU_HAS_INTRINSIC_COUNTR_ZERO)
				return impl::countr_zero_native(val);
			else
			{
				if (is_constant_evaluated())
					return impl::countr_zero_native(val);
				else
					return impl::countr_zero_intrinsic(val);
			}
		}
	}

	/// \brief	Counts the number of consecutive 1 bits, starting from the left.
	/// \ingroup	intrinsics
	///
	/// \detail This is equivalent to C++20's std::countl_one, with the addition of also being
	/// 		 extended to work with enum types.
	/// 
	/// \tparam	T		An unsigned integer or enum type.
	/// \param 	val		The value to test.
	///
	/// \returns	The number of consecutive ones from the left end of an integer's bits.
	template <typename T MUU_SFINAE(is_unsigned<T>)>
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr int MUU_VECTORCALL countl_one(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return countl_one(unwrap(val));
		else
			return countl_zero(static_cast<T>(~val));
	}

	/// \brief	Counts the number of consecutive 1 bits, starting from the right.
	/// \ingroup	intrinsics
	///
	/// \detail This is equivalent to C++20's std::countr_one, with the addition of also being
	/// 		 extended to work with enum types.
	/// 
	/// \tparam	T		An unsigned integer or enum type.
	/// \param 	val		The value to test.
	///
	/// \returns	The number of consecutive ones from the right end of an integer's bits.
	template <typename T MUU_SFINAE(is_unsigned<T>)>
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr int MUU_VECTORCALL countr_one(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return countr_one(unwrap(val));
		else
			return countr_zero(static_cast<T>(~val));
	}


	/// \brief	Finds the smallest integral power of two not less than the given value.
	/// \ingroup	intrinsics
	///
	/// \detail This is equivalent to C++20's std::bit_ceil, with the addition of also being
	/// 		 extended to work with enum types.
	/// 
	/// \tparam	T		An unsigned integer or enum type.
	/// \param 	val		The input value.
	///
	/// \returns	The smallest integral power of two that is not smaller than `val`.
	template <typename T MUU_SFINAE(is_unsigned<T>)>
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr T MUU_VECTORCALL bit_ceil(T val) noexcept
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

	/// \brief	Bitwise-packs integers left-to-right into a larger integer.
	/// \ingroup	intrinsics
	///
	/// \detail \cpp
	/// const auto val = pack(0xAABB_u16, 0xCCDD_u16);
	/// assert(val == 0xAABBCCDD_u32);
	/// const auto val = pack(0xAABB_u16, 0xCCDD_u16, 0xEEFF_u16);
	/// assert(val == 0x0000AABBCCDDEEFF_u64);
	///              // ^^^^ input was zero-padded on the left
	/// \ecpp
	/// 
	/// \tparam	Return	An integer or enum type, or leave as `void` to choose an unsigned type based on the total size of the inputs.
	/// \tparam	T	  	An integer or enum type.
	/// \tparam	U	  	An integer or enum type.
	/// \tparam	V	  	Integer or enum types.
	/// \param 	val1	The left-most value to be packed.
	/// \param 	val2	The second-left-most value to be packed.
	/// \param 	vals	Any remaining values to be packed.
	///
	/// \returns	An integral value containing the input values packed bitwise left-to-right. If the total size of the
	/// 			inputs was less than the return type, the output will be zero-padded on the left.
	template <typename Return = void, typename T, typename U, typename... V MUU_SFINAE(all_integral<T, U, V...>)>
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr auto MUU_VECTORCALL pack(T val1, U val2, V... vals) noexcept
	{
		static_assert(
			!is_cvref<Return>,
			"Return type cannot be const, volatile, or a reference"
		);
		static_assert(
			(total_size<T, U, V...> * CHAR_BIT) <= (MUU_HAS_INT128 ? 128 : 64),
			"No integer type large enough to hold the packed values exists on the target platform"
		);
		using return_type = std::conditional_t<
			std::is_void_v<Return>,
			unsigned_integer<bit_ceil(total_size<T, U, V...> * CHAR_BIT)>,
			Return
		>;
		static_assert(
			total_size<T, U, V...> <= sizeof(return_type),
			"Return type cannot fit all the input values"
		);

		if constexpr (any_enum<return_type, T, U, V...>)
		{
			return static_cast<return_type>(pack<remove_enum<return_type>>(
				static_cast<remove_enum<T>>(val1),
				static_cast<remove_enum<U>>(val2),
				static_cast<remove_enum<V>>(vals)...
			));
		}
		else if constexpr (any_signed<return_type, T, U, V...>)
		{
			return static_cast<return_type>(pack<make_unsigned<return_type>>(
				static_cast<make_unsigned<T>>(val1),
				static_cast<make_unsigned<U>>(val2),
				static_cast<make_unsigned<V>>(vals)...
			 ));
		}
		else if constexpr (sizeof...(V) > 0)
		{
			return static_cast<return_type>(
				static_cast<return_type>(static_cast<return_type>(val1) << (total_size<U, V...> * CHAR_BIT))
				| pack<return_type>(val2, vals...)
			);
		}
		else
		{
			return static_cast<return_type>(
				static_cast<return_type>(static_cast<return_type>(val1) << (sizeof(U) * CHAR_BIT))
				| static_cast<return_type>(val2)
			);
		}
	}

	namespace impl
	{
		MUU_PUSH_WARNINGS
		MUU_DISABLE_LIFETIME_WARNINGS

		template <typename To, typename From>
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		constexpr To bit_cast_fallback(const From& from) noexcept
		{
			static_assert(!std::is_reference_v<To> && !std::is_reference_v<From>);
			static_assert(std::is_trivially_copyable_v<From> && std::is_trivially_copyable_v<To>);
			static_assert(sizeof(To) == sizeof(From));

			if constexpr (std::is_same_v<remove_cv<To>, remove_cv<From>>)
				return from;
			else if constexpr (all_integral<To, From>)
				return static_cast<To>(static_cast<remove_enum<remove_cv<To>>>(unwrap(from)));
			else if constexpr (std::is_default_constructible_v<To>)
			{
				To dst;
				memcpy(&dst, &from, sizeof(To));
				return dst;
			}
		}

		MUU_POP_WARNINGS
	}

	/// \brief	Equivalent to C++20's std::bit_cast.
	/// \ingroup	intrinsics
	///
	/// \detail Compilers implement this as an intrinsic which is typically
	/// 		 available regardless of the C++ mode. Using this function
	/// 		 on these compilers allows you to get the same behaviour
	/// 		 even when you aren't targeting C++20.
	/// 
	/// \remark On older compilers lacking support for std::bit_cast you won't be able to call this function
	/// 		   in constexpr contexts (since it falls back to a memcpy-based implementation).
	/// 		   You can check for constexpr support by examining build::supports_constexpr_bit_cast.
	template <typename To, typename From>
	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(pure)
	MUU_ATTR(flatten)
	constexpr To bit_cast(const From& from) noexcept
	{
		static_assert(
			!std::is_reference_v<To> && !std::is_reference_v<From>,
			"From and To types cannot be references"
		);
		static_assert(
			std::is_trivially_copyable_v<From> && std::is_trivially_copyable_v<To>,
			"From and To types must be trivially-copyable"
		);
		static_assert(
			sizeof(To) == sizeof(From),
			"From and To types must be the same size"
		);

		#if MUU_CLANG >= 9		\
			|| MUU_GCC >= 11	\
			|| MUU_MSVC >= 1926
			return __builtin_bit_cast(To, from);
			#define MUU_HAS_INTRINSIC_BIT_CAST 1
		#else
			return impl::bit_cast_fallback<To>(from);
			#define MUU_HAS_INTRINSIC_BIT_CAST 0
		#endif
	}

	namespace build
	{
		/// \brief	True if using bit_cast() in constexpr contexts is supported on this compiler.
		inline constexpr bool supports_constexpr_bit_cast = !!MUU_HAS_INTRINSIC_BIT_CAST;
	}

	MUU_PUSH_WARNINGS
	MUU_PRAGMA_MSVC(warning(disable: 4191)) // unsafe pointer conversion (that's the whole point)

	/// \brief	Casts between pointers, choosing the most appropriate conversion path.
	/// \ingroup	intrinsics
	///
	/// \detail Doing low-level work with pointers often requires a lot of tedious boilerplate,
	/// 		particularly when moving to/from raw byte representations or dealing with `const`.
	/// 		By using `pointer_cast` instead you can eliminate a lot of that boilerplate,
	/// 		since it will do 'the right thing' via some combination of:
	///	| From                    | To                      | Cast              | Note                  |
	///	|-------------------------|-------------------------|-------------------|-----------------------|
	///	| void\*                  | T\*                     | static_cast       |                       |
	///	| T\*                     | void\*                  | static_cast       |                       |
	///	| T\*                     | const T\*               | static_cast       |                       |
	///	| const T\*               | T\*                     | const_cast        |                       |
	///	| T\*                     | volatile T\*            | static_cast       |                       |
	///	| volatile T\*            | T\*                     | const_cast        |                       |
	///	| IUnknown\*              | IUnknown\*              | QueryInterface    | Windows only          |
	///	| Derived\*               | Base\*                  | static_cast       |                       |
	///	| Base\*                  | Derived\*               | dynamic_cast      | Polymorphic bases     |
	///	| Base\*                  | Derived\*               | reinterpret_cast  | Non-polymorphic bases |
	///	| T\*                     | (u)intptr_t             | reinterpret_cast  |                       |
	///	| (u)intptr_t             | T\*                     | reinterpret_cast  |                       |
	///	| void\*                  | T(\*func_ptr)()         | reinterpret_cast  | Where supported       |
	///	| T(\*func_ptr)()         | void\*                  | reinterpret_cast  | Where supported       |
	///	| T(\*func_ptr)()         | T(\*func_ptr)()noexcept | reinterpret_cast  |                       |
	///	| T(\*func_ptr)()noexcept | T(\*func_ptr)()         | static_cast       |                       |
	///
	///  \warning There are lots of static checks to make sure you don't do something completely insane,
	/// 		 but ultimately the fallback behaviour for casting between unrelated types is to use a
	/// 		 `reinterpret_cast`, and there's nothing stopping you from using multiple `pointer_casts`
	/// 		 through `void*` to make a conversion 'work'. Footguns aplenty!
	/// 
	/// \tparam To	A pointer or integral type large enough to store a pointer
	/// \tparam From A pointer, array, nullptr_t, or an integral type large enough to store a pointer.
	/// \param from The value being cast.
	/// 
	/// \return The input casted to the desired type.
	/// 
	template <typename To, typename From>
	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(flatten)
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
			|| (is_integral<From> && sizeof(From) >= sizeof(void*))
			|| std::is_same_v<From, remove_cv<nullptr_t>>,
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
			"Cannot pointer_cast with function pointers on the target platform"
		);

		// same input and output types (no-op)
		if constexpr (std::is_same_v<From, To>)
			return from;

		// nullptr_t -> *
		else if constexpr (std::is_same_v<From, nullptr_t>)
		{
			(void)from;
			return {};
		}

		// pointer -> integral
		// integral -> pointer
		else if constexpr (is_integral<From> || is_integral<To>)
		{
			static_assert(std::is_pointer_v<To> || std::is_pointer_v<From>);

			// pointer -> integral
			if constexpr (is_integral<To>)
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

			// integral -> pointer
			else
			{
				// enum -> pointer
				if constexpr (is_enum<From>)
					return pointer_cast<To>(unwrap(from));

				// (uintptr_t, intptr_t) -> pointer
				else if constexpr (is_same_as_any<From, uintptr_t, intptr_t>)
					return reinterpret_cast<To>(from);

				// other integers -> pointer
				else
				{
					using cast_t = std::conditional_t<is_signed<From>, intptr_t, uintptr_t>;
					return reinterpret_cast<To>(static_cast<cast_t>(from));
				}
			}
		}

		// pointer -> pointer
		else
		{
			static_assert(std::is_pointer_v<To> && std::is_pointer_v<From>);

			// Foo -> Foo (different cv)
			if constexpr (std::is_same_v<remove_cv<from_base>, remove_cv<to_base>>)
			{
				static_assert(!std::is_same_v<from_base, to_base>);

				// remove const/volatile
				if constexpr (is_const<from_base> || is_volatile<from_base>)
					return static_cast<To>(const_cast<remove_cv<from_base>*>(from));

				// add const/volatile
				else
				{
					static_assert(std::is_same_v<From, remove_cv<from_base>*>);
					static_assert(is_same_as_any<To, const to_base*, const volatile to_base*, volatile to_base*>);
					return static_cast<To>(from);
				}
			}

			// function -> non-function
			// non-function -> function
			// function -> function (different exception specifier)
			else if constexpr (std::is_function_v<from_base> || std::is_function_v<to_base>)
			{
				// function -> function (different exception specifier)
				if constexpr (std::is_function_v<from_base> && std::is_function_v<to_base>)
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

				// function -> non-function
				else if constexpr (std::is_function_v<from_base>)
				{
					static_assert(
						std::is_void_v<to_base>,
						"Cannot cast from a function pointer to a type other than void"
					);

					// function -> void
					return static_cast<To>(reinterpret_cast<void*>(from));
				}

				// non-function -> function
				else
				{
					static_assert(
						std::is_void_v<from_base>,
						"Cannot cast to a function pointer from a type other than void"
					);

					// void -> function
					return reinterpret_cast<To>(pointer_cast<void*>(from));
				}
			}

			// void -> non-void
			// non-void -> void
			// derived -> base
			else if constexpr (std::is_void_v<from_base> || std::is_void_v<to_base> || inherits_from<to_base, from_base>)
				return pointer_cast<To>(static_cast<rebase_pointer<From, remove_cv<to_base>>>(from));

			// IUnknown -> IUnknown (windows only)
			#if MUU_WINDOWS
			else if constexpr (impl::is_win32_iunknown<from_base> && impl::is_win32_iunknown<to_base>)
			{
				if (!from)
					return nullptr;

				// remove const/volatile from source type
				if constexpr (is_const<from_base> || is_volatile<from_base>)
					return pointer_cast<To>(const_cast<remove_cv<from_base>*>(from));

				// remove const/volatile from destination type
				else if constexpr (is_const<to_base> || is_volatile<to_base>)
					return const_cast<To>(pointer_cast<remove_cv<to_base>*>(from));

				else
				{
					static_assert(!is_const<from_base> && !is_volatile<from_base>);
					static_assert(!is_const<to_base> && !is_volatile<to_base>);

					to_base* to = {};
					if (from->QueryInterface(__uuidof(to_base), reinterpret_cast<void**>(&to)) == 0)
						to->Release();
					return to;
				}
			}
			#endif

			// base -> derived
			else if constexpr (inherits_from<from_base, to_base>)
			{
				if constexpr (std::is_polymorphic_v<from_base>)
					return pointer_cast<To>(dynamic_cast<rebase_pointer<From, remove_cv<to_base>>>(from));
				else
					return pointer_cast<To>(reinterpret_cast<rebase_pointer<From, remove_cv<to_base>>>(from));
			}

			// rank 2+ pointer -> *
			else if constexpr (std::is_pointer_v<from_base>)
			{
				return pointer_cast<To>(static_cast<match_cv<void, from_base>*>(from));
			}

			// * -> rank 2+ pointer
			else if constexpr (std::is_pointer_v<to_base>)
			{
				static_assert(!std::is_pointer_v<from_base>);
				return static_cast<To>(pointer_cast<match_cv<void, to_base>*>(from));
			}

			// Foo -> Bar (unrelated types)
			else
			{
				static_assert(!std::is_same_v<remove_cv<from_base>, remove_cv<to_base>>);
				static_assert(!std::is_pointer_v<from_base>);
				static_assert(!std::is_pointer_v<to_base>);
				return pointer_cast<To>(reinterpret_cast<rebase_pointer<From, remove_cv<to_base>>>(from));
			}
		}
	}

	template <typename To, typename From, size_t N>
	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(flatten)
	constexpr To pointer_cast(From(&arr)[N]) noexcept
	{
		return pointer_cast<To, From*>(arr);
	}

	template <typename To, typename From, size_t N>
	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(flatten)
	constexpr To pointer_cast(From(&&arr)[N]) noexcept
	{
		return pointer_cast<To, From*>(arr);
	}

	MUU_POP_WARNINGS // MUU_PRAGMA_MSVC(warning(disable: 4191))

	/// \brief	Applies a byte offset to a pointer.
	/// \ingroup	intrinsics
	///
	/// \tparam	T		The type being pointed to.
	/// \tparam	Offset	An integer type.
	/// \param	ptr	The pointer to offset.
	/// \param	offset	The number of bytes to add to the pointer's address.
	///
	/// \return	The cv-correct equivalent of `(T*)((std::byte*)ptr + offset)`.
	/// 
	/// \warning This function is a simple pointer arithmetic helper; absolutely no consideration
	/// 		 is given to the alignment of the pointed type. If you need to dereference pointers
	/// 		 returned by apply_offset the onus is on you to ensure that the offset made sense
	/// 		 before doing so!
	template <typename T, typename Offset MUU_SFINAE(is_integral<Offset> && is_arithmetic<Offset>)>
	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(const)
	MUU_ATTR(flatten)
	constexpr T* apply_offset(T* ptr, Offset offset) noexcept
	{
		return pointer_cast<T*>(pointer_cast<rebase_pointer<T*, std::byte>>(ptr) + offset);
	}

	/// \brief	Returns the minimum of two values.
	/// \ingroup	intrinsics
	///
	/// \detail This is equivalent to std::min without requiring you to drag in the enormity of `<algorithm>`.
	template <typename T>
	[[nodiscard]]
	MUU_ALWAYS_INLINE
	constexpr const T& (min)(const T& val1, const T& val2) noexcept
	{
		return val1 < val2 ? val1 : val2;
	}

	/// \brief	Returns the maximum of two values.
	/// \ingroup	intrinsics
	///
	/// \detail This is equivalent to std::max without requiring you to drag in the enormity of `<algorithm>`.
	template <typename T>
	[[nodiscard]]
	MUU_ALWAYS_INLINE
	constexpr const T& (max)(const T& val1, const T& val2) noexcept
	{
		return val1 < val2 ? val2 : val1;
	}

	/// \brief	Returns the absolute value of an arithmetic value.
	/// \ingroup	intrinsics
	///
	/// \detail This is similar to std::abs but is `constexpr` and doesn't coerce or promote the input types.
	template <typename T MUU_SFINAE(is_arithmetic<T>)>
	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(const)
	constexpr T MUU_VECTORCALL abs(T val) noexcept
	{
		if constexpr (is_unsigned<T>)
			return val;
		else
			return val < T{} ? -val : val;
	}


	/// \brief	Returns a value clamped between two bounds (inclusive).
	/// \ingroup	intrinsics
	///
	/// \detail This is equivalent to std::clamp without requiring you to drag in the enormity of `<algorithm>`.
	template <typename T>
	[[nodiscard]]
	constexpr const T& clamp(const T& val, const T& min_, const T& max_) noexcept
	{
		return (muu::max)((muu::min)(val, max_), min_);
	}

	/// \brief	Returns true if two values are approximately equal.
	/// \ingroup	intrinsics
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool MUU_VECTORCALL approx_equal(float a, float b, float epsilon = constants<float>::approx_equal_epsilon) noexcept
	{
		return abs(b - a) < epsilon;
	}

	/// \brief	Returns true if two values are approximately equal.
	/// \ingroup	intrinsics
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool MUU_VECTORCALL approx_equal(double a, double b, double epsilon = constants<double>::approx_equal_epsilon) noexcept
	{
		return abs(b - a) < epsilon;
	}

	/// \brief	Returns true if two values are approximately equal.
	/// \ingroup	intrinsics
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool MUU_VECTORCALL approx_equal(long double a, long double b, long double epsilon = constants<long double>::approx_equal_epsilon) noexcept
	{
		return abs(b - a) < epsilon;
	}

	#if MUU_HAS_FLOAT128
	/// \brief	Returns true if two values are approximately equal.
	/// \ingroup	intrinsics
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool MUU_VECTORCALL approx_equal(float128_t a, float128_t b, float128_t epsilon = constants<float128_t>::approx_equal_epsilon) noexcept
	{
		return abs(b - a) < epsilon;
	}
	#endif

	#if MUU_HAS_FLOAT16
	/// \brief	Returns true if two values are approximately equal.
	/// \ingroup	intrinsics
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool MUU_VECTORCALL approx_equal(float16_t a, float16_t b, float16_t epsilon = constants<float16_t>::approx_equal_epsilon) noexcept
	{
		return abs(b - a) < epsilon;
	}
	#endif

	#if MUU_HAS_INTERCHANGE_FP16
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool MUU_VECTORCALL approx_equal(__fp16 a, __fp16 b, __fp16 epsilon = constants<__fp16>::approx_equal_epsilon) noexcept
	{
		return abs(b - a) < epsilon;
	}
	#endif

	/// \brief	Calculates a linear interpolation between two values.
	/// \ingroup	intrinsics
	///
	/// \remark	This is a stand-in for C++20's std::lerp, but does _not_ make the same guarantees about infinities and NaN's.
	/// 			Garbage-in, garbage-out.
	/// 
	/// \returns	The requested linear interpolation between the start and finish values.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr float MUU_VECTORCALL lerp(float start, float finish, float alpha) noexcept
	{
		return start * (1.0f - alpha) + finish * alpha;
	}

	/// \brief	Calculates a linear interpolation between two values.
	/// \ingroup	intrinsics
	///
	/// \remark	This is a stand-in for C++20's std::lerp, but does _not_ make the same guarantees about infinities and NaN's.
	/// 			Garbage-in, garbage-out.
	/// 
	/// \returns	The requested linear interpolation between the start and finish values.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr double MUU_VECTORCALL lerp(double start, double finish, double alpha) noexcept
	{
		return start * (1.0 - alpha) + finish * alpha;
	}

	/// \brief	Calculates a linear interpolation between two values.
	/// \ingroup	intrinsics
	///
	/// \remark	This is a stand-in for C++20's std::lerp, but does _not_ make the same guarantees about infinities and NaN's.
	/// 			Garbage-in, garbage-out.
	/// 
	/// \returns	The requested linear interpolation between the start and finish values.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr long double MUU_VECTORCALL lerp(long double start, long double finish, long double alpha) noexcept
	{
		return start * (1.0L - alpha) + finish * alpha;
	}

	#if MUU_HAS_FLOAT128
	/// \brief	Calculates a linear interpolation between two values.
	/// \ingroup	intrinsics
	///
	/// \remark	This is a stand-in for C++20's std::lerp, but does _not_ make the same guarantees about infinities and NaN's.
	/// 			Garbage-in, garbage-out.
	/// 
	/// \returns	The requested linear interpolation between the start and finish values.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr float128_t MUU_VECTORCALL lerp(float128_t start, float128_t finish, float128_t alpha) noexcept
	{
		return start * (float128_t{ 1 } - alpha) + finish * alpha;
	}
	#endif

	#if MUU_HAS_FLOAT16
	/// \brief	Calculates a linear interpolation between two values.
	/// \ingroup	intrinsics
	///
	/// \remark	This is a stand-in for C++20's std::lerp, but does _not_ make the same guarantees about infinities and NaN's.
	/// 			Garbage-in, garbage-out.
	/// 
	/// \returns	The requested linear interpolation between the start and finish values.
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr float16_t MUU_VECTORCALL lerp(float16_t start, float16_t finish, float16_t alpha) noexcept
	{
		return static_cast<float16_t>(lerp(static_cast<float>(start), static_cast<float>(finish), static_cast<float>(alpha)));
	}
	#endif

	#if MUU_HAS_INTERCHANGE_FP16
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr __fp16 MUU_VECTORCALL lerp(__fp16 start, __fp16 finish, __fp16 alpha) noexcept
	{
		return static_cast<__fp16>(lerp(static_cast<float>(start), static_cast<float>(finish), static_cast<float>(alpha)));
	}
	#endif

	template <typename T, typename U, typename V MUU_SFINAE(all_arithmetic<T, U, V>)>
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr auto MUU_VECTORCALL lerp(T start, U finish, V alpha) noexcept
	{
		using start_type = remove_cv<std::conditional_t<is_floating_point<T>, T, double>>;
		using finish_type = remove_cv<std::conditional_t<is_floating_point<U>, U, double>>;
		using alpha_type = remove_cv<std::conditional_t<is_floating_point<V>, V, double>>;
		static_assert(all_floating_point<start_type, finish_type, alpha_type>);

		using type = impl::highest_ranked<start_type, finish_type, alpha_type>;
		return static_cast<type(MUU_VECTORCALL*)(type,type,type)noexcept>(lerp)(start, finish, alpha);
	}

	/// \brief	Returns true if a value is between two bounds (inclusive).
	/// \ingroup	intrinsics
	template <typename T, typename U>
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool MUU_VECTORCALL is_between(T val, U min_, U max_) noexcept
	{
		if constexpr ((is_arithmetic<T> || is_enum<U>) || (is_arithmetic<T> || is_enum<U>))
		{
			if constexpr (is_enum<T> || is_enum<U>)
				return is_between(unwrap(val), unwrap(min_), unwrap(max_));
			else
			{
				using lhs = remove_cvref<T>;
				using rhs = remove_cvref<U>;
				if constexpr (is_signed<lhs> && is_unsigned<rhs>)
				{
					if (val < lhs{})
						return false;
				}
				else if constexpr (is_unsigned<lhs> && is_signed<rhs>)
				{
					if (max_ < rhs{})
						return false;
				}
				if constexpr (!std::is_same_v<lhs, rhs>)
				{
					using common_type = std::common_type_t<lhs, rhs>;
					return is_between(
						static_cast<common_type>(val),
						static_cast<common_type>(min_),
						static_cast<common_type>(max_)
					);
				}
				else
					return min_ <= val && val <= max_;
			}
		}
		else
			return min_ <= val && val <= max_; // user-defined <= operator, ideally
	}

	namespace impl
	{
		template <size_t> struct popcount_traits;

		template <>
		struct popcount_traits<8>
		{
			static constexpr uint8_t m1 =  0x55_u8;
			static constexpr uint8_t m2 =  0x33_u8;
			static constexpr uint8_t m4 =  0x0f_u8;
			static constexpr uint8_t h01 = 0x01_u8;
			static constexpr int rsh = 0;
		};

		template <>
		struct popcount_traits<16>
		{
			static constexpr uint16_t m1 =  0x5555_u16;
			static constexpr uint16_t m2 =  0x3333_u16;
			static constexpr uint16_t m4 =  0x0f0f_u16;
			static constexpr uint16_t h01 = 0x0101_u16;
			static constexpr int rsh = 8;
		};

		template <>
		struct popcount_traits<32>
		{
			static constexpr uint32_t m1 =  0x55555555_u32;
			static constexpr uint32_t m2 =  0x33333333_u32;
			static constexpr uint32_t m4 =  0x0f0f0f0f_u32;
			static constexpr uint32_t h01 = 0x01010101_u32;
			static constexpr int rsh = 24;
		};

		template <>
		struct popcount_traits<64>
		{
			static constexpr uint64_t m1 =  0x5555555555555555_u64;
			static constexpr uint64_t m2 =  0x3333333333333333_u64;
			static constexpr uint64_t m4 =  0x0f0f0f0f0f0f0f0f_u64;
			static constexpr uint64_t h01 = 0x0101010101010101_u64;
			static constexpr int rsh = 56;
		};

		#if MUU_HAS_INT128
		template <>
		struct popcount_traits<128>
		{
			static constexpr uint128_t m1 =  pack(0x5555555555555555_u64, 0x5555555555555555_u64);
			static constexpr uint128_t m2 =  pack(0x3333333333333333_u64, 0x3333333333333333_u64);
			static constexpr uint128_t m4 =  pack(0x0f0f0f0f0f0f0f0f_u64, 0x0f0f0f0f0f0f0f0f_u64);
			static constexpr uint128_t h01 = pack(0x0101010101010101_u64, 0x0101010101010101_u64);
			static constexpr int rsh = 120;
		};
		#endif

		template <typename T>
		[[nodiscard]]
		MUU_ATTR(const)
		constexpr int MUU_VECTORCALL popcount_native(T val) noexcept
		{
			MUU_ASSUME(val > T{});

			using pt = popcount_traits<sizeof(T) * CHAR_BIT>;
			val -= ((val >> 1) & pt::m1);
			val = (val & pt::m2) + ((val >> 2) & pt::m2);
			return static_cast<int>(static_cast<T>(((val + (val >> 4)) & pt::m4) * pt::h01) >> pt::rsh);
		}

		template <typename T>
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(const)
		auto MUU_VECTORCALL popcount_intrinsic(T val) noexcept
		{
			MUU_ASSUME(val > T{});

			#if MUU_GCC || MUU_CLANG
			{
				#define MUU_HAS_INTRINSIC_POPCOUNT 1

				if constexpr (sizeof(T) <= sizeof(unsigned int))
					return __builtin_popcount(static_cast<unsigned int>(val));
				else if constexpr (std::is_same_v<T, unsigned long>)
					return __builtin_popcountl(val);
				else if constexpr (std::is_same_v<T, unsigned long long>)
					return __builtin_popcountll(val);
				#if MUU_HAS_INT128
				else if constexpr (std::is_same_v<T, uint128_t>)
					return __builtin_popcountll(static_cast<unsigned long long>(val >> 64))
						+ __builtin_popcountll(static_cast<unsigned long long>(val));
				#endif
				else
					static_assert(dependent_false<T>, "Unsupported integer type");
			}
			#elif MUU_ICC
			{
				#define MUU_HAS_INTRINSIC_POPCOUNT 1

				if constexpr (sizeof(T) <= sizeof(int))
					return _popcnt32(static_cast<int>(val));
				else if constexpr (sizeof(T) == sizeof(__int64))
					return _popcnt64(static_cast<__int64>(val));
				else
					static_assert(dependent_false<T>, "Unsupported integer type");
			}
			#elif MUU_MSVC
			{
				#define MUU_HAS_INTRINSIC_POPCOUNT 1

				if constexpr (sizeof(T) <= sizeof(unsigned short))
				{
					#if MUU_MSVC >= 1928 // VS 16.8
						return __popcnt16(static_cast<unsigned short>(val));
					#else
						return popcount_native(val);
					#endif
				}
				else if constexpr (sizeof(T) == sizeof(unsigned int))
					return __popcnt(static_cast<unsigned int>(val));
				else if constexpr (std::is_same_v<T, unsigned __int64>)
				{
					#if MUU_MSVC >= 1928 && MUU_ARCH_X64 // VS 16.8
						return __popcnt64(static_cast<unsigned __int64>(val));
					#else
						return __popcnt(static_cast<unsigned int>(val >> 32))
							+ __popcnt(static_cast<unsigned int>(val));
					#endif
				}
				else
					static_assert(dependent_false<T>, "Unsupported integer type");
			}
			#else
			{
				#define MUU_HAS_INTRINSIC_POPCOUNT 0

				static_assert(dependent_false<T>, "popcount_intrinsic not implemented for this compiler");
			}
			#endif
		}
	}

	/// \brief	Counts the number of set bits (the 'population count') of an unsigned integer.
	/// \ingroup	intrinsics
	///
	/// \detail This is equivalent to C++20's std::popcount, with the addition of also being
	/// 		 extended to work with enum types.
	/// 
	/// \tparam	T		An unsigned integer or enum type.
	/// \param 	val		The input value.
	///
	/// \returns	The number of bits that were set to `1` in `val`.
	template <typename T MUU_SFINAE(is_unsigned<T>)>
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr int MUU_VECTORCALL popcount(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return popcount(unwrap(val));
		else
		{
			if (!val)
				return 0;

			if constexpr (!build::supports_is_constant_evaluated || !MUU_HAS_INTRINSIC_POPCOUNT)
				return impl::popcount_native(val);
			else
			{
				if (is_constant_evaluated())
					return impl::popcount_native(val);
				else
					return static_cast<int>(impl::popcount_intrinsic(val));
			}
		}
	}

	/// \brief	Checks if an integral value has only a single bit set.
	/// \ingroup	intrinsics
	/// 
	/// \detail This is equivalent to C++20's std::has_single_bit, with the addition of also being
	/// 		 extended to work with enum types.
	///
	/// \tparam	T		An unsigned integer or enum type.
	/// \param 	val		The value to test.
	///
	/// \returns	True if the input value had only a single bit set (and thus was a power of two).
	template <typename T MUU_SFINAE(is_unsigned<T>)>
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr bool MUU_VECTORCALL has_single_bit(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return has_single_bit(unwrap(val));
		else
		{
			if constexpr (!build::supports_is_constant_evaluated || !MUU_HAS_INTRINSIC_POPCOUNT)
				return val != T{} && (val & (val - T{ 1 })) == T{};
			else
			{
				if (is_constant_evaluated())
					return val != T{} && (val & (val - T{ 1 })) == T{};
				else
					return impl::popcount_intrinsic(val) == 1;
			}
		}
	}

	/// \brief	Finds the largest integral power of two not greater than the given value.
	/// \ingroup	intrinsics
	/// 
	/// \detail This is equivalent to C++20's std::bit_floor, with the addition of also being
	/// 		 extended to work with enum types.
	///
	/// \tparam	T		An unsigned integer or enum type.
	/// \param 	val		The input value.
	///
	/// \returns	Zero if `val` is zero; otherwise, the largest integral power of two that is not greater than `val`.
	template <typename T MUU_SFINAE(is_unsigned<T>)>
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr T MUU_VECTORCALL bit_floor(T val) noexcept
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
	/// \ingroup	intrinsics
	/// 
	/// \detail This is equivalent to C++20's std::bit_width, with the addition of also being
	/// 		 extended to work with enum types.
	///
	/// \tparam	T		An unsigned integer or enum type.
	/// \param 	val		The input value.
	///
	/// \returns	If `val` is not zero, calculates the number of bits needed to store `val` (i.e. `1 + log2(x)`).
	/// 			Returns `0` if `val` is zero.
	template <typename T MUU_SFINAE(is_unsigned<T>)>
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr T MUU_VECTORCALL bit_width(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return static_cast<T>(bit_width(unwrap(val)));
		else
			return static_cast<T>(sizeof(T) * CHAR_BIT - static_cast<size_t>(countl_zero(val)));
	}

	/// \brief	Returns an unsigned integer filled from the right
	/// 		with the desired number of consecutive ones.
	/// \ingroup	intrinsics
	///
	/// \detail \cpp
	/// const auto val1 = bit_fill_right<uint32_t>(5);
	/// const auto val2 = 0b00000000000000000000000000011111u;
	/// assert(val1 == val2);
	///  \ecpp
	///  
	/// \tparam	T		An unsigned integer type.
	/// \param 	count	Number of consecutive ones.
	///
	/// \returns	An instance of T right-filled with the desired number of ones.
	template <typename T MUU_SFINAE(is_unsigned<T>)>
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr T MUU_VECTORCALL bit_fill_right(size_t count) noexcept
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

	/// \brief	Returns an unsigned integer filled from the left
	/// 		with the desired number of consecutive ones.
	/// \ingroup	intrinsics
	///
	/// \detail \cpp
	/// const auto val1 = bit_fill_left<uint32_t>(5);
	/// const auto val2 = 0b11111000000000000000000000000000u;
	/// assert(val1 == val2);
	///  \ecpp
	///  
	/// \tparam	T		An unsigned integer type.
	/// \param 	count	Number of consecutive ones.
	///
	/// \returns	An instance of T left-filled with the desired number of ones.
	template <typename T MUU_SFINAE(is_unsigned<T>)>
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr T MUU_VECTORCALL bit_fill_left(size_t count) noexcept
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

	/// \brief	Gets a specific byte from an integer.
	/// \ingroup	intrinsics
	///
	/// \detail \cpp
	/// const auto i = 0xAABBCCDDu;
	/// //                ^ ^ ^ ^
	/// // byte indices:  3 2 1 0
	/// 
	/// std::cout << std::hex;
	///	std::cout << "0: " << byte_select<0>(i) << "\n";
	///	std::cout << "1: " << byte_select<1>(i) << "\n";
	///	std::cout << "2: " << byte_select<2>(i) << "\n";
	///	std::cout << "3: " << byte_select<3>(i) << "\n";
	/// \ecpp
	/// 
	/// \out
	/// 0: DD
	/// 1: CC
	/// 2: BB
	/// 3: AA
	/// (on a little-endian system)
	/// \eout
	/// 
	/// \tparam	Index	Index of the byte to retrieve.
	/// \tparam	T		An integer or enum type.
	/// \param 	val		An integer or enum value.
	///
	/// \remark The indexation order of bytes is the _memory_ order, not their
	/// 		 numeric significance (i.e. byte 0 is always the first byte in the integer's
	/// 		 memory allocation, regardless of the endianness of the platform).
	/// 
	/// \returns	The value of the selected byte.
	template <size_t Index, typename T MUU_SFINAE(is_integral<T>)>
	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(const)
	constexpr uint8_t MUU_VECTORCALL byte_select(T val) noexcept
	{
		static_assert(
			Index < sizeof(T),
			"The byte index is out-of-range; it must be less than the size of the input integer"
		);

		if constexpr (is_enum<T>)
			return byte_select<Index>(unwrap(val));
		else if constexpr (is_signed<T>)
			return byte_select<Index>(static_cast<make_unsigned<T>>(val));
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

	/// \brief	Gets a specific byte from an integer.
	/// \ingroup	intrinsics
	///
	/// \detail \cpp
	/// const auto i = 0xAABBCCDDu;
	/// //                ^ ^ ^ ^
	/// // byte indices:  3 2 1 0
	/// 
	/// std::cout << std::hex;
	///	std::cout << "0: " << byte_select(i, 0) << "\n";
	///	std::cout << "1: " << byte_select(i, 1) << "\n";
	///	std::cout << "2: " << byte_select(i, 2) << "\n";
	///	std::cout << "3: " << byte_select(i, 3) << "\n";
	/// \ecpp
	/// 
	/// \out
	/// 0: DD
	/// 1: CC
	/// 2: BB
	/// 3: AA
	/// (on a little-endian system)
	/// \eout
	/// 
	/// \tparam	T		An integer or enum type.
	/// \param 	val		An integer or enum value.
	/// \param	index	Index of the byte to retrieve.
	///
	/// \remark The indexation order of bytes is the _memory_ order, not their
	/// 		 numeric significance (i.e. byte 0 is always the first byte in the integer's
	/// 		 memory allocation, regardless of the endianness of the platform).
	/// 
	/// \returns	The value of the selected byte, or 0 if the index was out-of-range.
	template <typename T MUU_SFINAE(is_integral<T>)>
	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(const)
	constexpr uint8_t MUU_VECTORCALL byte_select(T val, size_t index) noexcept
	{
		if (index > sizeof(T))
			return uint8_t{};

		if constexpr (is_enum<T>)
			return byte_select(unwrap(val), index);
		else if constexpr (is_signed<T>)
			return byte_select(static_cast<make_unsigned<T>>(val), index);
		else if constexpr (sizeof(T) == 1_sz)
			return static_cast<uint8_t>(val);
		else
			return static_cast<uint8_t>(static_cast<T>(val >> (index * CHAR_BIT)) & static_cast<T>(0xFFu));
	}

	namespace impl
	{
		template <typename T>
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(const)
		T MUU_VECTORCALL byte_reverse_intrinsic(T val) noexcept
		{
			#if MUU_GCC || MUU_CLANG
			{
				#define MUU_HAS_INTRINSIC_BYTE_REVERSE 1

				if constexpr (std::is_same_v<T, uint16_t>)
					return __builtin_bswap16(val);
				else if constexpr (std::is_same_v<T, uint32_t>)
					return __builtin_bswap32(val);
				else if constexpr (std::is_same_v<T, uint64_t>)
					return __builtin_bswap64(val);
				#if MUU_HAS_INT128
				else if constexpr (std::is_same_v<T, uint128_t>)
				{
					#if MUU_HAS_BUILTIN(__builtin_bswap128)
						return __builtin_bswap128(val);
					#else
						return (static_cast<uint128_t>(byte_reverse_intrinsic(static_cast<uint64_t>(val))) << 64)
							| byte_reverse_intrinsic(static_cast<uint64_t>(val >> 64));
					#endif
				}
				#endif
				else
					static_assert(dependent_false<T>, "Unsupported integer type");
			}
			#elif MUU_MSVC || MUU_ICC_CL
			{
				#define MUU_HAS_INTRINSIC_BYTE_REVERSE 1

				if constexpr (sizeof(T) == sizeof(unsigned short))
					return static_cast<T>(_byteswap_ushort(static_cast<unsigned short>(val)));
				else if constexpr (sizeof(T) == sizeof(unsigned long))
					return static_cast<T>(_byteswap_ulong(static_cast<unsigned long>(val)));
				else if constexpr (sizeof(T) == sizeof(unsigned long long))
					return static_cast<T>(_byteswap_uint64(static_cast<unsigned long long>(val)));
				else
					static_assert(dependent_false<T>, "Unsupported integer type");
			}
			#else
			{
				#define MUU_HAS_INTRINSIC_BYTE_REVERSE 0

				static_assert(dependent_false<T>, "byte_reverse_intrinsic not implemented for this compiler");
			}
			#endif
		}

		template <typename T>
		[[nodiscard]]
		MUU_ATTR(const)
		constexpr T MUU_VECTORCALL byte_reverse_native(T val) noexcept
		{
			if constexpr (sizeof(T) == sizeof(uint16_t))
			{
				return static_cast<T>(
					static_cast<uint32_t>(val << 8)
					| static_cast<uint32_t>(val >> 8)
				);
			}
			else if constexpr (sizeof(T) == sizeof(uint32_t))
			{
				return (val << 24)
					| ((val << 8) & 0x00FF0000_u32)
					| ((val >> 8) & 0x0000FF00_u32)
					| (val >> 24);
			}
			else if constexpr (sizeof(T) == sizeof(uint64_t))
			{
				return (val << 56)
					| ((val << 40) & 0x00FF000000000000_u64)
					| ((val << 24) & 0x0000FF0000000000_u64)
					| ((val << 8)  & 0x000000FF00000000_u64)
					| ((val >> 8)  & 0x00000000FF000000_u64)
					| ((val >> 24) & 0x0000000000FF0000_u64)
					| ((val >> 40) & 0x000000000000FF00_u64)
					| (val  >> 56);
			}
			#if MUU_HAS_INT128
			else if constexpr (sizeof(T) == sizeof(uint128_t))
			{
				return (static_cast<uint128_t>(
					byte_reverse_native(static_cast<uint64_t>(val))) << 64)
					| byte_reverse_native(static_cast<uint64_t>(val >> 64)
				);
			}
			#endif
			else
				static_assert(dependent_false<T>, "Unsupported integer type");
		}
	}

	/// \brief	Reverses the byte order of an unsigned integral type.
	/// \ingroup	intrinsics
	///
	/// \detail \cpp
	/// const auto i = 0xAABBCCDDu;
	/// const auto j = byte_reverse(i);
	/// std::cout << std::hex << i << "\n" << j;
	/// \ecpp
	/// 
	/// \out
	/// AABBCCDD
	/// DDCCBBAA
	/// \eout
	/// 
	/// \tparam	T	An unsigned integer or enum type.
	/// \param 	val	An unsigned integer or enum value.
	///
	/// \returns	A copy of the input value with the byte order reversed.
	template <typename T MUU_SFINAE(is_unsigned<T>)>
	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(const)
	constexpr T MUU_VECTORCALL byte_reverse(T val) noexcept
	{
		if constexpr (is_enum<T>)
			return static_cast<T>(byte_reverse(unwrap(val)));
		else
		{
			if constexpr (!build::supports_is_constant_evaluated || !MUU_HAS_INTRINSIC_BYTE_REVERSE)
				return impl::byte_reverse_native(val);
			else
			{
				if (is_constant_evaluated())
					return impl::byte_reverse_native(val);
				else
					return impl::byte_reverse_intrinsic(val);
			}
		}
	}

	/// \brief	Select and re-pack arbitrary bytes from an integer.
	/// \ingroup	intrinsics
	///
	/// \detail \cpp
	///
	/// const auto i = 0xAABBCCDDu;
	/// //                ^ ^ ^ ^
	/// // byte indices:  3 2 1 0
	/// 
	/// std::cout << std::hex << std::setfill('0') << std::setw(8);
	///	std::cout << "      <0>: " << swizzle<0>(i) << "\n";
	///	std::cout << "   <1, 0>: " << swizzle<1, 0>(i) << "\n";
	///	std::cout << "<3, 2, 3>: " << swizzle<3, 2, 3>(i) << "\n";
	/// \ecpp
	/// 
	/// \out
	///       <0>: 000000DD
	///    <1, 0>: 0000CCDD
	/// <3, 2, 3>: 00AABBAA
	/// (on a little-endian system)
	/// \eout
	/// 
	/// \tparam	ByteIndices		Indices of the bytes from the source integer, in the order they're to be packed.
	/// \tparam	T				An integer or enum type.
	/// 
	/// \remark The indexation order of bytes is the _memory_ order, not their
	/// 		 numeric significance (i.e. byte 0 is always the first byte in the integer's
	/// 		 memory allocation, regardless of the endianness of the platform).
	/// 
	/// \returns	An integral value containing the selected bytes packed bitwise left-to-right. If the total size of the
	/// 			inputs was less than the return type, the output will be zero-padded on the left.
	template <size_t... ByteIndices, typename T MUU_SFINAE(is_integral<T>)>
	[[nodiscard]]
	MUU_ATTR(const)
	constexpr auto MUU_VECTORCALL swizzle(T val) noexcept
	{
		static_assert(
			sizeof...(ByteIndices) > 0_sz,
			"At least one byte index must be specified."
		);
		static_assert(
			(sizeof...(ByteIndices) * CHAR_BIT) <= (MUU_HAS_INT128 ? 128 : 64),
			"No integer type large enough to hold the swizzled value exists on the target platform"
		);
		static_assert(
			(true && ... && (ByteIndices < sizeof(T))),
			"One or more of the source byte indices was out-of-range"
		);
		using swizzle_type = std::conditional_t<
			is_signed<T>,
			signed_integer<bit_ceil(sizeof...(ByteIndices) * CHAR_BIT)>,
			unsigned_integer<bit_ceil(sizeof...(ByteIndices) * CHAR_BIT)>
		>;
		using return_type = std::conditional_t<
			sizeof...(ByteIndices) == sizeof(T),
			T,
			swizzle_type
		>;

		if constexpr (is_enum<T>)
			return static_cast<return_type>(swizzle<ByteIndices...>(unwrap(val)));
		else if constexpr (is_signed<T>)
			return static_cast<return_type>(swizzle<ByteIndices...>(static_cast<make_unsigned<T>>(val)));
		else if constexpr (sizeof...(ByteIndices) == 1_sz)
			return static_cast<return_type>(byte_select<ByteIndices...>(val));
		else
			return pack<return_type>(byte_select<ByteIndices>(val)...);
	}

	MUU_PRAGMA_GCC("GCC push_options")
	MUU_PRAGMA_GCC("GCC optimize (\"-fno-finite-math-only\")")

	namespace impl
	{
		template <size_t TotalBits, size_t SignificandBits>
		struct is_infinity_or_nan_traits;

		template <>
		struct is_infinity_or_nan_traits<16, 11>
		{
			static constexpr auto mask = 0b0111110000000000_u16;
		};

		template <>
		struct is_infinity_or_nan_traits<32, 24>
		{
			static constexpr auto mask = 0b01111111100000000000000000000000_u32;
		};

		template <>
		struct is_infinity_or_nan_traits<64, 53>
		{
			static constexpr auto mask = 0b0111111111110000000000000000000000000000000000000000000000000000_u64;
		};

		template <>
		struct is_infinity_or_nan_traits<80, 64>
		{
			static constexpr uint16_t mask[] { 0x0000_u16, 0x0000_u16, 0x0000_u16, 0x8000_u16, 0x7FFF_u16 };

			template <typename T>
			[[nodiscard]]
			MUU_ALWAYS_INLINE
			static constexpr bool MUU_VECTORCALL check(const T& val) noexcept
			{
				return (val[3] & mask[3]) == mask[3]
					&& (val[4] & mask[4]) == mask[4];
			}
		};

		template <>
		struct is_infinity_or_nan_traits<128, 64>
		{
			#if MUU_HAS_INT128
			static constexpr auto mask = pack(0x0000000000007FFF_u64, 0x8000000000000000_u64);
			#else
			static constexpr uint64_t mask[]{ 0x8000000000000000_u64, 0x0000000000007FFF_u64 };

			template <typename T>
			[[nodiscard]]
			MUU_ALWAYS_INLINE
			static constexpr bool MUU_VECTORCALL check(const T& val) noexcept
			{
				return (val[0] & mask[0]) == mask[0]
					&& (val[1] & mask[1]) == mask[1];
			}

			#endif
		};

		template <>
		struct is_infinity_or_nan_traits<128, 113>
		{
			#if MUU_HAS_INT128
			static constexpr auto mask = pack(0x7FFF000000000000_u64, 0x0000000000000000_u64);
			#else
			static constexpr uint64_t mask[]{ 0x0000000000000000_u64, 0x7FFF000000000000_u64 };

			template <typename T>
			[[nodiscard]]
			MUU_ALWAYS_INLINE
				static constexpr bool MUU_VECTORCALL check(const T& val) noexcept
			{
				return (val[1] & mask[1]) == mask[1];
			}
			#endif
		};

		template <typename T>
		struct is_infinity_or_nan_traits_typed
			: is_infinity_or_nan_traits<sizeof(T) * CHAR_BIT, constants<T>::significand_digits>
		{};	
	}

	/// \brief	Checks if an arithmetic value is infinity or NaN.
	/// \ingroup	intrinsics
	///
	/// \tparam	T	The value type.
	/// \param 	val	The value to examine.
	///
	/// \returns	True if the value is floating-point infinity or NaN.
	/// 			Always returns false if the value was not a floating-point type.
	/// 
	/// \remark	Older compilers won't provide the necessary machinery for you to be able to call this function in
	/// 			constexpr contexts. You can check for constexpr support by examining
	/// 			build::supports_constexpr_is_infinity_or_nan.
	template <typename T MUU_SFINAE(is_arithmetic<T>)>
	[[nodiscard]]
	MUU_ATTR(const)
	MUU_ATTR(flatten)
	constexpr bool MUU_VECTORCALL is_infinity_or_nan(T val) noexcept
	{
		// Q: "what about fpclassify, isnan, isinf??"
		// A1: They're not constexpr
		// A2: They don't work reliably with -ffast-math
		// A3: https://godbolt.org/z/P9GGdK

		if constexpr (!is_floating_point<T>)
		{
			(void)val;
			return false;
		}
		else
		{
			using traits = impl::is_infinity_or_nan_traits_typed<T>;
			using blit_type = std::remove_const_t<decltype(traits::mask)>;

			if constexpr (is_integral<blit_type>)
			{
				return (bit_cast<blit_type>(val) & traits::mask) == traits::mask;
			}
			else
			{
				return traits::check(bit_cast<blit_type>(val));
			}
		}
	}

	namespace build
	{
		/// \brief	True if using is_infinity_or_nan() in constexpr contexts is supported on this compiler.
		inline constexpr bool supports_constexpr_is_infinity_or_nan = build::supports_constexpr_bit_cast;
	}

	MUU_PRAGMA_GCC("GCC pop_options") // -fno-finite-math-only


	namespace impl
	{
		template <typename T>
		using has_pointer_traits_to_address = decltype(std::pointer_traits<remove_cvref<T>>::to_address(std::declval<remove_cvref<T>>()));
	}

	/// \brief Obtain the address represented by p without forming a reference to the pointee.
	/// \ingroup	intrinsics
	///
	/// \detail This is equivalent to C++20's std::to_address.
	template <typename T>
	[[nodiscard]]
	constexpr T* to_address(T* p) noexcept
	{
		static_assert(!std::is_function_v<T>, "to_address may not be used on functions.");
		return p;
	}

	/// \brief Obtain the address represented by p without forming a reference to the pointee.
	/// \ingroup	intrinsics
	///
	/// \detail This is equivalent to C++20's std::to_address.
	template <typename Ptr>
	[[nodiscard]]
	constexpr auto to_address(const Ptr& p) noexcept //(1)
	{
		if constexpr (impl::is_detected<impl::has_pointer_traits_to_address, Ptr>)
		{
			return std::pointer_traits<Ptr>::to_address(p);
		}
		else
		{
			return to_address(p.operator->());
		}
	}

	/// \brief	Equivalent to C++20's std::assume_aligned.
	/// \ingroup	intrinsics
	///
	/// \detail Compilers typically implement std::assume_aligned as an intrinsic which is
	/// 		 available regardless of the C++ mode. Using this function on these compilers allows
	/// 		 you to get the same behaviour even when you aren't targeting C++20.
	/// 
	/// \see [P1007R1: std::assume_aligned](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1007r1.pdf)
	template <size_t N, typename T>
	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(flatten)
	MUU_ATTR(assume_aligned(N))
	constexpr T* assume_aligned(T* ptr) noexcept
	{
		static_assert(
			N > 0 && has_single_bit(N),
			"assume_aligned() requires a power-of-two alignment value."
		);
		MUU_ASSUME((reinterpret_cast<uintptr_t>(ptr) & (N - uintptr_t{ 1 })) == 0);

		#if MUU_CLANG || MUU_GCC
			return reinterpret_cast<T*>(__builtin_assume_aligned(pointer_cast<const void*>(ptr), N));
		#elif MUU_MSVC
		{
			if constexpr (N < 16384)
				return reinterpret_cast<T*>(__builtin_assume_aligned(pointer_cast<const void*>(ptr), N));
			else
				return ptr;
		}
		#elif MUU_ICC
		{
			__assume_aligned(ptr, N);
			return ptr;
		}
		#elif defined(__cpp_lib_assume_aligned)
			return std::assume_aligned<N>(ptr);
		#else
			return ptr;
		#endif
	}

}
MUU_NAMESPACE_END

MUU_POP_WARNINGS // MUU_DISABLE_ARITHMETIC_WARNINGS

//=====================================================================================================================
// COMPRESSED PAIR
//=====================================================================================================================

MUU_NAMESPACE_START
{
	namespace impl
	{
		template <size_t I, typename T>
		constexpr decltype(auto) compressed_pair_get(T&& cp) noexcept
		{
			static_assert(I <= 1);
			if constexpr (I == 0)
				return std::forward<T>(cp).first();
			else
				return std::forward<T>(cp).second();
		}
	}

	/// \brief	A pair that uses Empty Base Class Optimization to elide storage for one or both of its members where possible.
	/// \ingroup blocks
	/// 
	/// \tparam	First		First member type.
	/// \tparam	Second		Second member type.
	template <typename First, typename Second
		#ifndef DOXYGEN
			, unsigned empty_member = ((std::is_empty_v<First> ? 1u : 0u) | (std::is_empty_v<Second> ? 2u : 0u))
		#endif
	>
	class compressed_pair
	{
		private:
			static_assert(!std::is_empty_v<First> && !std::is_empty_v<Second>);
			First first_;
			Second second_;

		public:

			/// \brief	The pair's first member type.
			using first_type = First;
			/// \brief	The pair's second member type.
			using second_type = Second;

			/// \brief	Default constructor.
			MUU_NODISCARD_CTOR
			constexpr compressed_pair() = default;

			/// \brief	Copy constructor.
			MUU_NODISCARD_CTOR
			constexpr compressed_pair(const compressed_pair&) = default;

			/// \brief	Move constructor.
			MUU_NODISCARD_CTOR
			constexpr compressed_pair(compressed_pair&&) = default;

			/// \brief	Copy-assignment operator.
			constexpr compressed_pair& operator =(const compressed_pair&) = default;

			/// \brief	Move-assignment operator.
			constexpr compressed_pair& operator =(compressed_pair&&) = default;
			
			/// \brief	Constructs a compressed pair from two values.
			/// 
			/// \tparam	F	First member initializer type.
			/// \tparam	S	Second member initializer type.
			/// \param 	first_init 	First member initializer.
			/// \param 	second_init	Second member initializer.
			template <typename F, typename S>
			MUU_NODISCARD_CTOR
			constexpr compressed_pair(F&& first_init, S&& second_init)
				noexcept(std::is_nothrow_constructible_v<First, F&&> && std::is_nothrow_constructible_v<Second, S&&>)
				: first_{ std::forward<F>(first_init) },
				second_{ std::forward<S>(second_init) }
			{}

			/// \brief	Returns an lvalue reference to the first member.
			[[nodiscard]] constexpr first_type& first() & noexcept					{ return first_; }
			/// \brief	Returns an rvalue reference to the first member.
			[[nodiscard]] constexpr first_type&& first() && noexcept				{ return std::move(first_); }
			/// \brief	Returns a const lvalue reference to the first member.
			[[nodiscard]] constexpr const first_type& first() const& noexcept		{ return first_; }
			/// \brief	Returns a const rvalue reference to the first member.
			[[nodiscard]] constexpr const first_type&& first() const&& noexcept		{ return std::move(first_); }

			/// \brief	Returns an lvalue reference to the second member.
			[[nodiscard]] constexpr second_type& second() & noexcept				{ return second_; }
			/// \brief	Returns an rvalue reference to the second member.
			[[nodiscard]] constexpr second_type&& second() && noexcept				{ return std::move(second_); }
			/// \brief	Returns a const lvalue reference to the second member.
			[[nodiscard]] constexpr const second_type& second() const& noexcept		{ return second_; }
			/// \brief	Returns a const rvalue reference to the second member.
			[[nodiscard]] constexpr const second_type&& second() const&& noexcept	{ return std::move(second_); }

			/// \brief	Returns an lvalue reference to the selected member.
			template <size_t I>
			[[nodiscard]]
			constexpr auto& get() & noexcept
			{
				return impl::compressed_pair_get<I>(*this);
			}

			/// \brief	Returns an rvalue reference to the selected member.
			template <size_t I>
			[[nodiscard]]
			constexpr auto&& get() && noexcept
			{
				return impl::compressed_pair_get<I>(std::move(*this));
			}

			/// \brief	Returns a const lvalue reference to the selected member.
			template <size_t I>
			[[nodiscard]]
			constexpr const auto& get() const & noexcept
			{
				return impl::compressed_pair_get<I>(*this);
			}

			/// \brief	Returns a const rvalue reference to the selected member.
			template <size_t I>
			[[nodiscard]]
			constexpr const auto&& get() const && noexcept
			{
				return impl::compressed_pair_get<I>(std::move(*this));
			}
	};

	#ifndef DOXYGEN
	template <typename F, typename S> compressed_pair(F, S) -> compressed_pair<F, S>;

	template <typename First, typename Second>
	class MUU_EMPTY_BASES compressed_pair<First, Second, 1> : private First
	{
		private:
			static_assert(std::is_empty_v<First> && !std::is_empty_v<Second>);
			Second second_;

		public:

			using first_type = First;
			using second_type = Second;

			MUU_NODISCARD_CTOR constexpr compressed_pair() = default;
			MUU_NODISCARD_CTOR constexpr compressed_pair(const compressed_pair&) = default;
			MUU_NODISCARD_CTOR constexpr compressed_pair(compressed_pair&&) = default;
			constexpr compressed_pair& operator =(const compressed_pair&) = default;
			constexpr compressed_pair& operator =(compressed_pair&&) = default;

			template <typename F, typename S>
			MUU_NODISCARD_CTOR
			constexpr compressed_pair(F&& first_init, S&& second_init)
				noexcept(std::is_nothrow_constructible_v<First, F&&> && std::is_nothrow_constructible_v<Second, S&&>)
				: First{ std::forward<F>(first_init) },
				second_{ std::forward<S>(second_init) }
			{}

			[[nodiscard]] constexpr first_type& first() & noexcept					{ return *this; }
			[[nodiscard]] constexpr first_type&& first() && noexcept				{ return std::move(*this); }
			[[nodiscard]] constexpr const first_type& first() const& noexcept		{ return *this; }
			[[nodiscard]] constexpr const first_type&& first() const&& noexcept		{ return std::move(*this); }

			[[nodiscard]] constexpr second_type& second() & noexcept				{ return second_; }
			[[nodiscard]] constexpr second_type&& second() && noexcept				{ return std::move(second_); }
			[[nodiscard]] constexpr const second_type& second() const& noexcept		{ return second_; }
			[[nodiscard]] constexpr const second_type&& second() const&& noexcept	{ return std::move(second_); }

			template <size_t I>
			[[nodiscard]]
			constexpr auto& get() & noexcept
			{
				return impl::compressed_pair_get<I>(*this);
			}

			template <size_t I>
			[[nodiscard]]
			constexpr auto&& get() && noexcept
			{
				return impl::compressed_pair_get<I>(std::move(*this));
			}

			template <size_t I>
			[[nodiscard]]
			constexpr const auto& get() const& noexcept
			{
				return impl::compressed_pair_get<I>(*this);
			}

			template <size_t I>
			[[nodiscard]]
			constexpr const auto&& get() const&& noexcept
			{
				return impl::compressed_pair_get<I>(std::move(*this));
			}
	};

	template <typename First, typename Second>
	class MUU_EMPTY_BASES compressed_pair<First, Second, 2> : private Second
	{
		private:
			static_assert(!std::is_empty_v<First> && std::is_empty_v<Second>);
			First first_;

		public:

			using first_type = First;
			using second_type = Second;

			MUU_NODISCARD_CTOR constexpr compressed_pair() = default;
			MUU_NODISCARD_CTOR constexpr compressed_pair(const compressed_pair&) = default;
			MUU_NODISCARD_CTOR constexpr compressed_pair(compressed_pair&&) = default;
			constexpr compressed_pair& operator =(const compressed_pair&) = default;
			constexpr compressed_pair& operator =(compressed_pair&&) = default;

			template <typename F, typename S>
			MUU_NODISCARD_CTOR
			constexpr compressed_pair(F&& first_init, S&& second_init)
				noexcept(std::is_nothrow_constructible_v<First, F&&> && std::is_nothrow_constructible_v<Second, S&&>)
				: Second{ std::forward<S>(second_init) },
				first_{ std::forward<F>(first_init) }
			{}

			[[nodiscard]] constexpr first_type& first() & noexcept					{ return first_; }
			[[nodiscard]] constexpr first_type&& first() && noexcept				{ return std::move(first_); }
			[[nodiscard]] constexpr const first_type& first() const& noexcept		{ return first_; }
			[[nodiscard]] constexpr const first_type&& first() const&& noexcept		{ return std::move(first_); }

			[[nodiscard]] constexpr second_type& second() & noexcept				{ return *this; }
			[[nodiscard]] constexpr second_type&& second() && noexcept				{ return std::move(*this); }
			[[nodiscard]] constexpr const second_type& second() const& noexcept		{ return *this; }
			[[nodiscard]] constexpr const second_type&& second() const&& noexcept	{ return std::move(*this); }

			template <size_t I>
			[[nodiscard]]
			constexpr auto& get() & noexcept
			{
				return impl::compressed_pair_get<I>(*this);
			}

			template <size_t I>
			[[nodiscard]]
			constexpr auto&& get() && noexcept
			{
				return impl::compressed_pair_get<I>(std::move(*this));
			}

			template <size_t I>
			[[nodiscard]]
			constexpr const auto& get() const& noexcept
			{
				return impl::compressed_pair_get<I>(*this);
			}

			template <size_t I>
			[[nodiscard]]
			constexpr const auto&& get() const&& noexcept
			{
				return impl::compressed_pair_get<I>(std::move(*this));
			}
	};

	template <typename First, typename Second>
	class MUU_EMPTY_BASES compressed_pair<First, Second, 3> : private First, private Second
	{
		private:
			static_assert(std::is_empty_v<First> && std::is_empty_v<Second>);

		public:

			using first_type = First;
			using second_type = Second;

			MUU_NODISCARD_CTOR constexpr compressed_pair() = default;
			MUU_NODISCARD_CTOR constexpr compressed_pair(const compressed_pair&) = default;
			MUU_NODISCARD_CTOR constexpr compressed_pair(compressed_pair&&) = default;
			constexpr compressed_pair& operator =(const compressed_pair&) = default;
			constexpr compressed_pair& operator =(compressed_pair&&) = default;

			template <typename F, typename S>
			MUU_NODISCARD_CTOR
			constexpr compressed_pair(F&& first_init, S&& second_init)
				noexcept(std::is_nothrow_constructible_v<First, F&&> && std::is_nothrow_constructible_v<Second, S&&>)
				: First{ std::forward<F>(first_init) },
				Second{ std::forward<S>(second_init) }
			{}

			[[nodiscard]] constexpr first_type& first() & noexcept					{ return *this; }
			[[nodiscard]] constexpr first_type&& first() && noexcept				{ return std::move(*this); }
			[[nodiscard]] constexpr const first_type& first() const& noexcept		{ return *this; }
			[[nodiscard]] constexpr const first_type&& first() const&& noexcept		{ return std::move(*this); }

			[[nodiscard]] constexpr second_type& second() & noexcept				{ return *this; }
			[[nodiscard]] constexpr second_type&& second() && noexcept				{ return std::move(*this); }
			[[nodiscard]] constexpr const second_type& second() const& noexcept		{ return *this; }
			[[nodiscard]] constexpr const second_type&& second() const&& noexcept	{ return std::move(*this); }

			template <size_t I>
			[[nodiscard]]
			constexpr auto& get() & noexcept
			{
				return impl::compressed_pair_get<I>(*this);
			}

			template <size_t I>
			[[nodiscard]]
			constexpr auto&& get() && noexcept
			{
				return impl::compressed_pair_get<I>(std::move(*this));
			}

			template <size_t I>
			[[nodiscard]]
			constexpr const auto& get() const& noexcept
			{
				return impl::compressed_pair_get<I>(*this);
			}

			template <size_t I>
			[[nodiscard]]
			constexpr const auto&& get() const&& noexcept
			{
				return impl::compressed_pair_get<I>(std::move(*this));
			}
	};

	#endif // DOXYGEN
}
MUU_NAMESPACE_END

namespace std
{
	/// \brief Specialization of std::tuple_size for muu::compressed_pair.
	template <typename First, typename Second>
	struct tuple_size<muu::compressed_pair<First, Second>>
	{
		static constexpr size_t value = 2;
	};

	/// \brief Specialization of std::tuple_element for muu::compressed_pair.
	template <size_t I, typename First, typename Second>
	struct tuple_element<I, muu::compressed_pair<First, Second>>
	{
		static_assert(I <= 1);
		using type = std::conditional_t<I == 1, Second, First>;
	};
}

//=====================================================================================================================
// SCOPE GUARD
//=====================================================================================================================

MUU_NAMESPACE_START
{
	/// \brief	Performs actions when going out of scope.
	/// \ingroup blocks
	///
	/// \detail Use a scope_guard to simplify cleanup routines
	/// 		or code that has acquire/release semantics, e.g. locking: \cpp
	/// 
	/// void do_something()
	/// {
	///		acquire_magic_lock();
	///		scope_guard sg{ release_magic_lock };
	///		something_that_throws();
	/// }
	/// 
	/// \ecpp
	/// 
	/// For comparison's sake, here's the same function without a scope_guard: \cpp
	/// 
	/// void do_something()
	/// {
	///		acquire_magic_lock();
	///		try
	///		{
	///			something_that_throws();
	///		}
	///		catch (...)
	///		{
	///			release_magic_lock();
	///			throw;
	///		}
	///		release_magic_lock();
	/// }
	/// 
	/// \ecpp
	/// 
	/// \tparam	T	A `noexcept` function, lambda or other callable not requiring
	/// 			any arguments (e.g. `void() noexcept`)`.
	template <typename T>
	class scope_guard
	{
		static_assert(
			std::is_invocable_v<std::remove_reference_t<T>>,
			"Types wrapped by a scope guard must be callable (functions, lambdas, etc.)"
		);
		static_assert(
			!build::has_exceptions
			|| std::is_nothrow_invocable_v<std::remove_reference_t<T>>,
			"Callables wrapped by a scope guard must be marked noexcept"
		);
		static_assert(
			!build::has_exceptions
			|| std::is_trivially_destructible_v<std::remove_reference_t<T>>
			|| std::is_nothrow_destructible_v<std::remove_reference_t<T>>,
			"Callables wrapped by a scope guard must be nothrow-destructible"
		);

		private:
			compressed_pair<T, bool> func_and_active;

		public:

			/// \brief	Constructs a scope_guard by wrapping a callable.
			///
			/// \tparam	U	 	A function, lambda or other callable with the signature `void() noexcept`.
			/// 
			/// \param 	func	The callable to invoke when the scope_guard goes out of scope.
			template <typename U>
			MUU_NODISCARD_CTOR
			explicit constexpr scope_guard(U&& func) noexcept
				: func_and_active{ std::forward<U>(func), true }
			{
				static_assert(
					!build::has_exceptions
					|| std::is_nothrow_constructible_v<std::remove_reference_t<T>, U&&>,
					"A scope_guard's callable must be nothrow-constructible from its initializer"
				);
			}

			~scope_guard() noexcept
			{
				if (func_and_active.second())
					func_and_active.first()();
			}

			scope_guard(const scope_guard&) = delete;
			scope_guard(scope_guard&&) = delete;
			scope_guard& operator = (const scope_guard&) = delete;
			scope_guard& operator = (scope_guard&&) = delete;

			/// \brief	Dismisses the scope guard, cancelling invocation of the wrapped callable.
			void dismiss() noexcept
			{
				func_and_active.second() = false;
			}
	};

	template <typename R, typename ...P>
	scope_guard(R(P...)noexcept) -> scope_guard<R(*)(P...)noexcept>;
	template <typename R, typename ...P>
	scope_guard(R(P...)) -> scope_guard<R(*)(P...)>;
	template <typename T>
	scope_guard(T&&) -> scope_guard<T>;
	template <typename T>
	scope_guard(T&) -> scope_guard<T&>;
}
MUU_NAMESPACE_END

#undef MUU_HAS_INTRINSIC_BIT_CAST
#undef MUU_HAS_INTRINSIC_POPCOUNT
#undef MUU_HAS_INTRINSIC_BYTE_REVERSE
#undef MUU_HAS_INTRINSIC_COUNTL_ZERO
#undef MUU_HAS_INTRINSIC_COUNTR_ZERO
MUU_PRAGMA_MSVC(inline_recursion(off))
MUU_POP_WARNINGS // MUU_DISABLE_SPAM_WARNINGS