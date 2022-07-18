// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Type traits and metafunctions.

#include "fwd.h"
#include "impl/std_type_traits.h"
#include "impl/std_utility.h"
#include "impl/is_constant_evaluated.h"
MUU_DISABLE_WARNINGS;
#if MUU_HAS_VECTORCALL
	#include <intrin.h>
#endif
MUU_ENABLE_WARNINGS;
#include "impl/header_start.h"
MUU_PRAGMA_MSVC(warning(disable : 4296)) // condition always true/false

#define MUU_ANY_VARIADIC_T(trait) (false || ... || trait<T>)
#define MUU_ALL_VARIADIC_T(trait) ((sizeof...(T) > 0) && ... && trait<T>)

namespace muu
{
	/// \addtogroup		meta
	/// @{

#if MUU_DOXYGEN || !defined(__cpp_lib_remove_cvref) || __cpp_lib_remove_cvref < 201711

	/// \brief	Removes the topmost const, volatile and reference qualifiers from a type.
	/// \remark This is equivalent to C++20's std::remove_cvref_t.
	template <typename T>
	using remove_cvref = std::remove_cv_t<std::remove_reference_t<T>>;

#else

	template <typename T>
	using remove_cvref = std::remove_cvref_t<T>;

#endif

	/// \brief	Evaluates to false but with delayed, type-dependent evaluation.
	/// \details Allows you to do things like this:
	/// \cpp
	/// if constexpr (is_fancy_type<T>)
	///		// ...
	/// else
	///		static_assert(always_false<T>, "Oh no, T wasn't fancy enough!");
	/// \ecpp
	template <typename T>
	inline constexpr bool always_false = false;

	/// \cond
	namespace impl
	{
		// note that anything sharing a name with a member of the top-level 'muu' namespace ends in underscores;
		// this is a disambiguation mechanism for code in the impl namespace.

		template <typename T, typename U>
		struct rebase_ref_
		{
			using type = U;
		};
		template <typename T, typename U>
		struct rebase_ref_<T&, U>
		{
			using type = std::add_lvalue_reference_t<U>;
		};
		template <typename T, typename U>
		struct rebase_ref_<T&&, U>
		{
			using type = std::add_rvalue_reference_t<U>;
		};

		template <typename T, typename CopyFrom>
		struct copy_ref_
		{
			using type = std::remove_reference_t<T>;
		};
		template <typename T, typename CopyFrom>
		struct copy_ref_<T, CopyFrom&>
		{
			using type = std::add_lvalue_reference_t<std::remove_reference_t<T>>;
		};
		template <typename T, typename CopyFrom>
		struct copy_ref_<T, CopyFrom&&>
		{
			using type = std::add_rvalue_reference_t<std::remove_reference_t<T>>;
		};

		template <typename T, typename U>
		struct rebase_pointer_
		{
			static_assert(std::is_pointer_v<T>);
			using type = std::add_pointer_t<U>;
		};
		template <typename T, typename U>
		struct rebase_pointer_<const volatile T*, U>
		{
			using type = std::add_pointer_t<std::add_const_t<std::add_volatile_t<U>>>;
		};
		template <typename T, typename U>
		struct rebase_pointer_<volatile T*, U>
		{
			using type = std::add_pointer_t<std::add_volatile_t<U>>;
		};
		template <typename T, typename U>
		struct rebase_pointer_<const T*, U>
		{
			using type = std::add_pointer_t<std::add_const_t<U>>;
		};
		template <typename T, typename U>
		struct rebase_pointer_<T&, U>
		{
			using type = std::add_lvalue_reference_t<typename rebase_pointer_<T, U>::type>;
		};
		template <typename T, typename U>
		struct rebase_pointer_<T&&, U>
		{
			using type = std::add_rvalue_reference_t<typename rebase_pointer_<T, U>::type>;
		};

		template <typename T, bool = std::is_enum_v<std::remove_reference_t<T>>>
		struct remove_enum_
		{
			using type = std::underlying_type_t<T>;
		};
		template <typename T>
		struct remove_enum_<T, false>
		{
			using type = T;
		};
		template <typename T>
		struct remove_enum_<const volatile T, true>
		{
			using type = const volatile typename remove_enum_<T>::type;
		};
		template <typename T>
		struct remove_enum_<volatile T, true>
		{
			using type = volatile typename remove_enum_<T>::type;
		};
		template <typename T>
		struct remove_enum_<const T, true>
		{
			using type = const typename remove_enum_<T>::type;
		};
		template <typename T>
		struct remove_enum_<T&, true>
		{
			using type = std::add_lvalue_reference_t<typename remove_enum_<T>::type>;
		};
		template <typename T>
		struct remove_enum_<T&&, true>
		{
			using type = std::add_rvalue_reference_t<typename remove_enum_<T>::type>;
		};

		// === size of ====================

		template <typename T, bool = (std::is_void_v<T> || std::is_function_v<std::remove_reference_t<T>>)>
		struct size_of_ : std::integral_constant<size_t, sizeof(T)>
		{};
		template <typename T>
		struct size_of_<T, true> : std::integral_constant<size_t, 0>
		{};

		// === largest ====================

		template <typename...>
		struct largest_;
		template <typename T>
		struct largest_<T>
		{
			using type = T;
		};
		template <typename T, typename U>
		struct largest_<T, U>
		{
			using type = std::conditional_t<(size_of_<U>::value > size_of_<T>::value), U, T>;
		};
		template <typename T, typename U, typename... V>
		struct largest_<T, U, V...>
		{
			using type = typename largest_<T, typename largest_<U, V...>::type>::type;
		};

		// === smallest ====================

		template <typename...>
		struct smallest_;
		template <typename T>
		struct smallest_<T>
		{
			using type = T;
		};
		template <typename T, typename U>
		struct smallest_<T, U>
		{
			using type = std::conditional_t<(size_of_<U>::value < size_of_<T>::value), U, T>;
		};
		template <typename T, typename U, typename... V>
		struct smallest_<T, U, V...>
		{
			using type = typename smallest_<T, typename smallest_<U, V...>::type>::type;
		};

		// === alignment of  ====================

		template <typename T, bool = (std::is_void_v<T> || std::is_function_v<std::remove_reference_t<T>>)>
		struct alignment_of_ : std::integral_constant<size_t, alignof(T)>
		{};
		template <typename T>
		struct alignment_of_<T, true> : std::integral_constant<size_t, 0>
		{};

		// === most aligned ====================

		template <typename...>
		struct most_aligned_;
		template <typename T>
		struct most_aligned_<T>
		{
			using type = T;
		};
		template <typename T, typename U>
		struct most_aligned_<T, U>
		{
			using type = std::conditional_t<(alignment_of_<U>::value > alignment_of_<T>::value), U, T>;
		};
		template <typename T, typename U, typename... V>
		struct most_aligned_<T, U, V...>
		{
			using type = typename most_aligned_<T, typename most_aligned_<U, V...>::type>::type;
		};

		// === least aligned ====================

		template <typename...>
		struct least_aligned_;
		template <typename T>
		struct least_aligned_<T>
		{
			using type = T;
		};
		template <typename T, typename U>
		struct least_aligned_<T, U>
		{
			using type = std::conditional_t<(alignment_of_<U>::value < alignment_of_<T>::value), U, T>;
		};
		template <typename T, typename U, typename... V>
		struct least_aligned_<T, U, V...>
		{
			using type = typename least_aligned_<T, typename least_aligned_<U, V...>::type>::type;
		};

		// === fixed-width signed integers ====================

		template <size_t Bits>
		struct signed_integer_;
		template <>
		struct signed_integer_<8>
		{
			using type = int8_t;
		};
		template <>
		struct signed_integer_<16>
		{
			using type = int16_t;
		};
		template <>
		struct signed_integer_<32>
		{
			using type = int32_t;
		};
		template <>
		struct signed_integer_<64>
		{
			using type = int64_t;
		};
#if MUU_HAS_INT128
		template <>
		struct signed_integer_<128>
		{
			using type = int128_t;
		};
#endif

		// === fixed-width unsigned integers ====================

		template <size_t Bits>
		struct unsigned_integer_;
		template <>
		struct unsigned_integer_<8>
		{
			using type = uint8_t;
		};
		template <>
		struct unsigned_integer_<16>
		{
			using type = uint16_t;
		};
		template <>
		struct unsigned_integer_<32>
		{
			using type = uint32_t;
		};
		template <>
		struct unsigned_integer_<64>
		{
			using type = uint64_t;
		};
#if MUU_HAS_INT128
		template <>
		struct unsigned_integer_<128>
		{
			using type = uint128_t;
		};
#endif

		template <typename T>
		struct make_signed_
		{
			using type = void;
		};
		template <typename T>
		struct make_signed_<const volatile T>
		{
			using type = const volatile typename make_signed_<T>::type;
		};
		template <typename T>
		struct make_signed_<volatile T>
		{
			using type = volatile typename make_signed_<T>::type;
		};
		template <typename T>
		struct make_signed_<const T>
		{
			using type = const typename make_signed_<T>::type;
		};
		template <typename T>
		struct make_signed_<T&>
		{
			using type = std::add_lvalue_reference_t<typename make_signed_<T>::type>;
		};
		template <typename T>
		struct make_signed_<T&&>
		{
			using type = std::add_rvalue_reference_t<typename make_signed_<T>::type>;
		};
		template <>
		struct make_signed_<char>
		{
			using type = signed char;
		};
		template <>
		struct make_signed_<signed char>
		{
			using type = signed char;
		};
		template <>
		struct make_signed_<unsigned char>
		{
			using type = signed char;
		};
		template <>
		struct make_signed_<short>
		{
			using type = short;
		};
		template <>
		struct make_signed_<unsigned short>
		{
			using type = short;
		};
		template <>
		struct make_signed_<int>
		{
			using type = int;
		};
		template <>
		struct make_signed_<unsigned int>
		{
			using type = int;
		};
		template <>
		struct make_signed_<long>
		{
			using type = long;
		};
		template <>
		struct make_signed_<unsigned long>
		{
			using type = long;
		};
		template <>
		struct make_signed_<long long>
		{
			using type = long long;
		};
		template <>
		struct make_signed_<unsigned long long>
		{
			using type = long long;
		};
		template <>
		struct make_signed_<half>
		{
			using type = half;
		};
		template <>
		struct make_signed_<float>
		{
			using type = float;
		};
		template <>
		struct make_signed_<double>
		{
			using type = double;
		};
		template <>
		struct make_signed_<long double>
		{
			using type = long double;
		};
#if MUU_HAS_INT128
		template <>
		struct make_signed_<int128_t>
		{
			using type = int128_t;
		};
		template <>
		struct make_signed_<uint128_t>
		{
			using type = int128_t;
		};
#endif
#if MUU_HAS_FLOAT128
		template <>
		struct make_signed_<float128_t>
		{
			using type = float128_t;
		};
#endif
#if MUU_HAS_FLOAT16
		template <>
		struct make_signed_<_Float16>
		{
			using type = _Float16;
		};
#endif
#if MUU_HAS_FP16
		template <>
		struct make_signed_<__fp16>
		{
			using type = __fp16;
		};
#endif
		template <>
		struct make_signed_<wchar_t>
		{
			using type = std::
				conditional_t<std::is_signed_v<wchar_t>, wchar_t, signed_integer_<sizeof(wchar_t) * CHAR_BIT>::type>;
		};
		template <typename Scalar, size_t Dimensions>
		struct make_signed_<vector<Scalar, Dimensions>>
		{
			using type = vector<typename make_signed_<Scalar>::type, Dimensions>;
		};
		template <typename Scalar>
		struct make_signed_<quaternion<Scalar>>
		{
			using type = quaternion<Scalar>; // unsigned quaternions are illegal, no logic required here
		};
		template <typename Scalar, size_t Rows, size_t Columns>
		struct make_signed_<matrix<Scalar, Rows, Columns>>
		{
			using type = matrix<typename make_signed_<Scalar>::type, Rows, Columns>;
		};

		template <typename T>
		struct make_unsigned_
		{
			using type = void;
		};
		template <typename T>
		struct make_unsigned_<const volatile T>
		{
			using type = const volatile typename make_unsigned_<T>::type;
		};
		template <typename T>
		struct make_unsigned_<volatile T>
		{
			using type = volatile typename make_unsigned_<T>::type;
		};
		template <typename T>
		struct make_unsigned_<const T>
		{
			using type = const typename make_unsigned_<T>::type;
		};
		template <typename T>
		struct make_unsigned_<T&>
		{
			using type = std::add_lvalue_reference_t<typename make_unsigned_<T>::type>;
		};
		template <typename T>
		struct make_unsigned_<T&&>
		{
			using type = std::add_rvalue_reference_t<typename make_unsigned_<T>::type>;
		};
		template <>
		struct make_unsigned_<char>
		{
			using type = unsigned char;
		};
		template <>
		struct make_unsigned_<signed char>
		{
			using type = unsigned char;
		};
		template <>
		struct make_unsigned_<unsigned char>
		{
			using type = unsigned char;
		};
		template <>
		struct make_unsigned_<short>
		{
			using type = unsigned short;
		};
		template <>
		struct make_unsigned_<unsigned short>
		{
			using type = unsigned short;
		};
		template <>
		struct make_unsigned_<int>
		{
			using type = unsigned int;
		};
		template <>
		struct make_unsigned_<unsigned int>
		{
			using type = unsigned int;
		};
		template <>
		struct make_unsigned_<long>
		{
			using type = unsigned long;
		};
		template <>
		struct make_unsigned_<unsigned long>
		{
			using type = unsigned long;
		};
		template <>
		struct make_unsigned_<long long>
		{
			using type = unsigned long long;
		};
		template <>
		struct make_unsigned_<unsigned long long>
		{
			using type = unsigned long long;
		};
		template <>
		struct make_unsigned_<char32_t>
		{
			using type = char32_t;
		};
		template <>
		struct make_unsigned_<char16_t>
		{
			using type = char16_t;
		};
#if MUU_HAS_CHAR8
		template <>
		struct make_unsigned_<char8_t>
		{
			using type = char8_t;
		};
#endif
#if MUU_HAS_INT128
		template <>
		struct make_unsigned_<int128_t>
		{
			using type = uint128_t;
		};
		template <>
		struct make_unsigned_<uint128_t>
		{
			using type = uint128_t;
		};
#endif
		template <>
		struct make_unsigned_<wchar_t>
		{
			using type = std::conditional_t<std::is_unsigned_v<wchar_t>,
											wchar_t,
											unsigned_integer_<sizeof(wchar_t) * CHAR_BIT>::type>;
		};
		template <typename Scalar, size_t Dimensions>
		struct make_unsigned_<vector<Scalar, Dimensions>>
		{
			using type = vector<typename make_unsigned_<Scalar>::type, Dimensions>;
		};
		template <typename Scalar, size_t Rows, size_t Columns>
		struct make_unsigned_<matrix<Scalar, Rows, Columns>>
		{
			using type = matrix<typename make_unsigned_<Scalar>::type, Rows, Columns>;
		};

		template <typename...>
		struct highest_ranked_;
		template <typename T>
		struct highest_ranked_<T>
		{
			using type = T;
		};
		template <typename T>
		struct highest_ranked_<T, T>
		{
			using type = T;
		};
		template <typename T>
		struct highest_ranked_<void, T>
		{
			using type = T;
		};
		template <typename T>
		struct highest_ranked_<T, void>
		{
			using type = T;
		};
		template <typename T, typename U>
		struct highest_ranked_<T, U>
		{
			using type = decltype(T{} + U{});
		};
		template <typename T, typename U, typename... V>
		struct highest_ranked_<T, U, V...>
		{
			using type = typename highest_ranked_<T, typename highest_ranked_<U, V...>::type>::type;
		};
#define MUU_HR_SPECIALIZATION(lower, higher)                                                                           \
	template <>                                                                                                        \
	struct highest_ranked_<lower, higher>                                                                              \
	{                                                                                                                  \
		using type = higher;                                                                                           \
	};                                                                                                                 \
	template <>                                                                                                        \
	struct highest_ranked_<higher, lower>                                                                              \
	{                                                                                                                  \
		using type = higher;                                                                                           \
	}
		MUU_HR_SPECIALIZATION(signed char, signed short);
		MUU_HR_SPECIALIZATION(unsigned char, unsigned short);
		MUU_HR_SPECIALIZATION(unsigned char, signed short);
		MUU_HR_SPECIALIZATION(half, float);
		MUU_HR_SPECIALIZATION(half, double);
		MUU_HR_SPECIALIZATION(half, long double);
#if MUU_HAS_FP16
		MUU_HR_SPECIALIZATION(__fp16, half);
		MUU_HR_SPECIALIZATION(__fp16, float);
		MUU_HR_SPECIALIZATION(__fp16, double);
		MUU_HR_SPECIALIZATION(__fp16, long double);
#endif
#if MUU_HAS_FLOAT16
		MUU_HR_SPECIALIZATION(half, _Float16);
		MUU_HR_SPECIALIZATION(_Float16, float);
		MUU_HR_SPECIALIZATION(_Float16, double);
		MUU_HR_SPECIALIZATION(_Float16, long double);
#endif
#if MUU_HAS_FLOAT128
		MUU_HR_SPECIALIZATION(half, float128_t);
#endif
#if MUU_HAS_FP16 && MUU_HAS_FLOAT16
		MUU_HR_SPECIALIZATION(__fp16, _Float16);
#endif
#if MUU_HAS_FP16 && MUU_HAS_FLOAT128
		MUU_HR_SPECIALIZATION(__fp16, float128_t);
#endif
#if MUU_HAS_FLOAT16 && MUU_HAS_FLOAT128
		MUU_HR_SPECIALIZATION(_Float16, float128_t);
#endif
#undef MUU_HR_SPECIALIZATION
		template <typename... T>
		using highest_ranked = typename highest_ranked_<std::remove_cv_t<std::remove_reference_t<T>>...>::type;

		template <typename T>
		using iter_reference_t = decltype(*std::declval<T&>());
		template <typename T>
		using iter_value_t = std::remove_reference_t<iter_reference_t<T>>;

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

		template <typename T>
		struct remove_all_pointers_
		{
			using type = T;
		};
		template <typename T>
		struct remove_all_pointers_<T*> : remove_all_pointers_<T>
		{};
		template <typename T>
		struct remove_all_pointers_<T* const> : remove_all_pointers_<T>
		{};
		template <typename T>
		struct remove_all_pointers_<T* volatile> : remove_all_pointers_<T>
		{};
		template <typename T>
		struct remove_all_pointers_<T* const volatile> : remove_all_pointers_<T>
		{};

#if defined(__cpp_lib_is_nothrow_convertible) && __cpp_lib_is_nothrow_convertible >= 201806

		template <typename From, typename To>
		inline constexpr bool is_implicitly_nothrow_convertible_ = std::is_nothrow_convertible_v<From, To>;

#else

		template <typename From,
				  typename To,
				  int = (std::is_void_v<From> && std::is_void_v<To> ? 2 : (std::is_convertible_v<From, To> ? 1 : 0))>
		inline constexpr bool is_implicitly_nothrow_convertible_ = false;

		template <typename From, typename To>
		inline constexpr bool is_implicitly_nothrow_convertible_<From, To, 1> =
			noexcept(static_cast<To>(std::declval<From>()));

		template <typename From, typename To>
		inline constexpr bool is_implicitly_nothrow_convertible_<From, To, 2> = true; // both void

#endif

		template <typename T>
		struct is_bounded_array_ : std::false_type
		{};
		template <typename T, size_t N>
		struct is_bounded_array_<T[N]> : std::true_type
		{};

		template <typename T>
		struct is_unbounded_array_ : std::false_type
		{};
		template <typename T>
		struct is_unbounded_array_<T[]> : std::true_type
		{};

		template <typename T>
		struct remove_rvalue_reference_
		{
			using type = T;
		};
		template <typename T>
		struct remove_rvalue_reference_<T&&>
		{
			using type = T;
		};

		template <typename T>
		struct remove_lvalue_reference_
		{
			using type = T;
		};
		template <typename T>
		struct remove_lvalue_reference_<T&>
		{
			using type = T;
		};

		template <typename T, size_t I, typename... Types>
		struct index_of_type_
		{
			static_assert(always_false<T>, "type not found in list");
		};

		template <typename T, size_t I, typename... Types>
		struct index_of_type_<T, I, T, Types...>
		{
			static constexpr size_t value = I;
		};

		template <typename T, size_t I, typename U, typename... Types>
		struct index_of_type_<T, I, U, Types...> : index_of_type_<T, I + 1, Types...>
		{};
	}
	/// \endcond

	/// \brief	Removes the outer enum wrapper from a type, converting it to the underlying integer equivalent.
	/// \remark This is similar to std::underlying_type_t but preserves cv qualifiers and ref categories, as well as
	/// 		being safe to use in SFINAE contexts (non-enum types are simply returned as-is).
	template <typename T>
	using remove_enum = typename impl::remove_enum_<T>::type;

	/// \brief Removes reference qualification from a type if (and only if) it is an lvalue reference.
	template <typename T>
	using remove_lvalue_reference = typename impl::remove_lvalue_reference_<T>::type;

	/// \brief Removes reference qualification from a type if (and only if) it is an rvalue reference.
	template <typename T>
	using remove_rvalue_reference = typename impl::remove_rvalue_reference_<T>::type;

	/// \brief The largest type from a set of types.
	/// \remarks Treats `void` and functions as having size of `0`.
	template <typename T, typename... U>
	using largest = typename impl::largest_<T, U...>::type;

	/// \brief The smallest type from a set of types.
	/// \remarks Treats `void` and functions as having size of `0`.
	template <typename T, typename... U>
	using smallest = typename impl::smallest_<T, U...>::type;

	/// \brief The sum of `sizeof()` for all of the types named by T.
	/// \remarks Treats `void` and functions as having size of `0`.
	template <typename... T>
	inline constexpr size_t total_size = (size_t{} + ... + impl::size_of_<T>::value);

	/// \brief The default alignment of a type.
	/// \remarks Treats `void` and functions as having an alignment of `0`.
	template <typename T>
	inline constexpr size_t alignment_of = impl::alignment_of_<remove_cvref<T>>::value;

	/// \brief The type with the largest alignment (i.e. having the largest value for `alignment_of<T>`) from a set of types.
	/// \remarks Treats `void` and functions as having an alignment of `0`.
	template <typename T, typename... U>
	using most_aligned = typename impl::most_aligned_<T, U...>::type;

	/// \brief The type with the smallest alignment (i.e. having the smallest value for `alignment_of<T>`) from a set of types.
	/// \remarks Treats `void` and functions as having an alignment of `0`.
	template <typename T, typename... U>
	using least_aligned = typename impl::least_aligned_<T, U...>::type;

	/// \brief	True if T is exactly the same as one or more of the types named by U.
	/// \remark This equivalent to `(std::is_same_v<T, U1> || std::is_same_v<T, U2> || ...)`.
	template <typename T, typename... U>
	inline constexpr bool any_same = (false || ... || std::is_same_v<T, U>);

	/// \brief	True if all the named types are exactly the same.
	/// \remark This equivalent to `(std::is_same_v<T, U1> && std::is_same_v<T, U2> && ...)`.
	template <typename T, typename... U>
	inline constexpr bool all_same = (true && ... && std::is_same_v<T, U>);

	/// \brief	The index of the first appearance of the type T in the type list [U, V...].
	template <typename T, typename U, typename... V>
	inline constexpr size_t index_of_type = impl::index_of_type_<T, 0, U, V...>::value;

	/// \brief	True if `From` is implicitly convertible to `To`.
	template <typename From, typename To>
	inline constexpr bool is_implicitly_convertible = std::is_convertible_v<From, To>;

	/// \brief	True if `From` is implicitly convertible to any of the types named by `To`.
	template <typename From, typename... To>
	inline constexpr bool is_implicitly_convertible_to_any = (false || ... || is_implicitly_convertible<From, To>);

	/// \brief	True if `From` is implicitly convertible to all of the types named by `To`.
	template <typename From, typename... To>
	inline constexpr bool is_implicitly_convertible_to_all = ((sizeof...(To) > 0) && ...
															  && is_implicitly_convertible<From, To>);

	/// \brief	True if all of the types named by `From` are implicitly convertible to `To`.
	template <typename To, typename... From>
	inline constexpr bool all_implicitly_convertible_to = ((sizeof...(From) > 0) && ...
														   && is_implicitly_convertible<From, To>);

	/// \brief	True if `From` is explicitly convertible to `To`.
	template <typename From, typename To>
	inline constexpr bool is_explicitly_convertible =
		!std::is_convertible_v<From, To> && std::is_constructible_v<To, From>;

	/// \brief	True if `From` is explicitly convertible to any of the types named by `To`.
	template <typename From, typename... To>
	inline constexpr bool is_explicitly_convertible_to_any = (false || ... || is_explicitly_convertible<From, To>);

	/// \brief	True if `From` is explicitly convertible to all of the types named by `To`.
	template <typename From, typename... To>
	inline constexpr bool is_explicitly_convertible_to_all = ((sizeof...(To) > 0) && ...
															  && is_explicitly_convertible<From, To>);

	/// \brief	True if all of the types named by `From` are explicitly convertible to `To`.
	template <typename To, typename... From>
	inline constexpr bool all_explicitly_convertible_to = ((sizeof...(From) > 0) && ...
														   && is_explicitly_convertible<From, To>);

	/// \brief	True if `From` is implicitly _or_ explicitly convertible to `To`.
	template <typename From, typename To>
	inline constexpr bool is_convertible = std::is_convertible_v<From, To> || std::is_constructible_v<To, From>;

	/// \brief	True if `From` is implicitly _or_ explicitly convertible to any of the types named by `To`.
	template <typename From, typename... To>
	inline constexpr bool is_convertible_to_any = (false || ... || is_convertible<From, To>);

	/// \brief	True if `From` is implicitly _or_ explicitly convertible to all of the types named by `To`.
	template <typename From, typename... To>
	inline constexpr bool is_convertible_to_all = ((sizeof...(To) > 0) && ... && is_convertible<From, To>);

	/// \brief	True if all of the types named by `From` are implicitly _or_ explicitly convertible to `To`.
	template <typename To, typename... From>
	inline constexpr bool all_convertible_to = ((sizeof...(From) > 0) && ... && is_convertible<From, To>);

	/// \brief	True if `From` is implicitly nothrow-convertible to `To`.
	template <typename From, typename To>
	inline constexpr bool is_implicitly_nothrow_convertible = impl::is_implicitly_nothrow_convertible_<From, To>;

	/// \brief	True if `From` is implicitly nothrow-convertible to any of the types named by `To`.
	template <typename From, typename... To>
	inline constexpr bool is_implicitly_nothrow_convertible_to_any = (false || ...
																	  || is_implicitly_nothrow_convertible<From, To>);

	/// \brief	True if `From` is implicitly nothrow-convertible to all of the types named by `To`.
	template <typename From, typename... To>
	inline constexpr bool is_implicitly_nothrow_convertible_to_all = ((sizeof...(To) > 0) && ...
																	  && is_implicitly_nothrow_convertible<From, To>);

	/// \brief	True if all of the types named by `From` are implicitly nothrow-convertible to `To`.
	template <typename To, typename... From>
	inline constexpr bool all_implicitly_nothrow_convertible_to = ((sizeof...(From) > 0) && ...
																   && is_implicitly_nothrow_convertible<From, To>);

	/// \brief	True if `From` is explicitly nothrow-convertible to `To`.
	template <typename From, typename To>
	inline constexpr bool is_explicitly_nothrow_convertible =
		!is_implicitly_nothrow_convertible<From, To> && std::is_nothrow_constructible_v<To, From>;

	/// \brief	True if `From` is explicitly nothrow-convertible to any of the types named by `To`.
	template <typename From, typename... To>
	inline constexpr bool is_explicitly_nothrow_convertible_to_any = (false || ...
																	  || is_explicitly_nothrow_convertible<From, To>);

	/// \brief	True if `From` is explicitly nothrow-convertible to all of the types named by `To`.
	template <typename From, typename... To>
	inline constexpr bool is_explicitly_nothrow_convertible_to_all = ((sizeof...(To) > 0) && ...
																	  && is_explicitly_nothrow_convertible<From, To>);

	/// \brief	True if all of the types named by `From` are explicitly nothrow-convertible to `To`.
	template <typename To, typename... From>
	inline constexpr bool all_explicitly_nothrow_convertible_to = ((sizeof...(From) > 0) && ...
																   && is_explicitly_nothrow_convertible<From, To>);

	/// \brief	True if `From` is implicitly _or_ explicitly nothrow-convertible to `To`.
	template <typename From, typename To>
	inline constexpr bool is_nothrow_convertible =
		is_implicitly_nothrow_convertible<From, To> || std::is_nothrow_constructible_v<To, From>;

	/// \brief	True if `From` is implicitly _or_ explicitly nothrow-convertible to any of the types named by `To`.
	template <typename From, typename... To>
	inline constexpr bool is_nothrow_convertible_to_any = (false || ... || is_nothrow_convertible<From, To>);

	/// \brief	True if `From` is implicitly _or_ explicitly nothrow-convertible to all of the types named by `To`.
	template <typename From, typename... To>
	inline constexpr bool is_nothrow_convertible_to_all = ((sizeof...(To) > 0) && ...
														   && is_nothrow_convertible<From, To>);

	/// \brief	True if all of the types named by `From` are implicitly _or_ explicitly nothrow-convertible to `To`.
	template <typename To, typename... From>
	inline constexpr bool all_nothrow_convertible_to = ((sizeof...(From) > 0) && ...
														&& is_nothrow_convertible<From, To>);

	/// \brief Is a type an enum or reference-to-enum?
	template <typename T>
	inline constexpr bool is_enum = std::is_enum_v<std::remove_reference_t<T>>;

	/// \brief Are any of the named types enums or reference-to-enum?
	template <typename... T>
	inline constexpr bool any_enum = MUU_ANY_VARIADIC_T(is_enum);

	/// \brief Are all of the named types enums or reference-to-enum?
	template <typename... T>
	inline constexpr bool all_enum = MUU_ALL_VARIADIC_T(is_enum);

	/// \brief Is a type a C++11 scoped enum class, or reference to one?
	template <typename T>
	inline constexpr bool is_scoped_enum =
		is_enum<T> && !std::is_convertible_v<remove_cvref<T>, remove_enum<remove_cvref<T>>>;

	/// \brief Are any of the named types C++11 scoped enum classes, or references to one?
	template <typename... T>
	inline constexpr bool any_scoped_enum = MUU_ANY_VARIADIC_T(is_scoped_enum);

	/// \brief Are all of the named types C++11 scoped enum classes, or references to one?
	template <typename... T>
	inline constexpr bool all_scoped_enum = MUU_ALL_VARIADIC_T(is_scoped_enum);

	/// \brief Is a type a pre-C++11 unscoped enum, or reference to one?
	template <typename T>
	inline constexpr bool is_legacy_enum =
		is_enum<T>&& std::is_convertible_v<remove_cvref<T>, remove_enum<remove_cvref<T>>>;

	/// \brief Are any of the named types pre-C++11 unscoped enums, or references to one?
	template <typename... T>
	inline constexpr bool any_legacy_enum = MUU_ANY_VARIADIC_T(is_legacy_enum);

	/// \brief Are all of the named types pre-C++11 unscoped enums, or references to one?
	template <typename... T>
	inline constexpr bool all_legacy_enum = MUU_ALL_VARIADIC_T(is_legacy_enum);

	/// \brief Is a type unsigned or reference-to-unsigned?
	/// \remarks True for enums backed by unsigned integers.
	/// \remarks True for #uint128_t (where supported).
	template <typename T>
	inline constexpr bool is_unsigned = std::is_unsigned_v<remove_enum<remove_cvref<T>>>
#if MUU_HAS_INT128
									 || std::is_same_v<remove_enum<remove_cvref<T>>, uint128_t>
#endif
		;

	/// \brief Are any of the named types unsigned or reference-to-unsigned?
	/// \remarks True for enums backed by unsigned integers.
	template <typename... T>
	inline constexpr bool any_unsigned = MUU_ANY_VARIADIC_T(is_unsigned);

	/// \brief Are all of the named types unsigned or reference-to-unsigned?
	/// \remarks True for enums backed by unsigned integers.
	template <typename... T>
	inline constexpr bool all_unsigned = MUU_ALL_VARIADIC_T(is_unsigned);

	/// \brief Is a type signed or reference-to-signed?
	/// \remarks True for enums backed by signed integers.
	/// \remarks True for muu::half.
	/// \remarks True for #int128_t, __fp16, _Float16 and #float128_t (where supported).
	template <typename T>
	inline constexpr bool is_signed = std::is_signed_v<remove_enum<remove_cvref<T>>>
		|| any_same<remove_enum<remove_cvref<T>>,
#if MUU_HAS_INT128
		int128_t,
#endif
#if MUU_HAS_FLOAT128
		float128_t,
#endif
#if MUU_HAS_FLOAT16
		_Float16,
#endif
#if MUU_HAS_FP16
		__fp16,
#endif
		half>;

	/// \brief Are any of the named types signed or reference-to-signed?
	/// \remarks True for enums backed by signed integers.
	template <typename... T>
	inline constexpr bool any_signed = MUU_ANY_VARIADIC_T(is_signed);

	/// \brief Are all of the named types signed or reference-to-signed?
	/// \remarks True for enums backed by signed integers.
	template <typename... T>
	inline constexpr bool all_signed = MUU_ALL_VARIADIC_T(is_signed);

	/// \brief Is a type an integral type or a reference to an integral type?
	/// \remarks True for enums.
	/// \remarks True for #int128_t and #uint128_t (where supported).
	template <typename T>
	inline constexpr bool is_integral = std::is_integral_v<remove_enum<remove_cvref<T>>>
#if MUU_HAS_INT128
									 || any_same<remove_enum<remove_cvref<T>>, int128_t, uint128_t>
#endif
		;

	/// \brief Are any of the named types integral or reference-to-integral?
	/// \remarks True for enums.
	/// \remarks True for #int128_t and #uint128_t (where supported).
	template <typename... T>
	inline constexpr bool any_integral = MUU_ANY_VARIADIC_T(is_integral);

	/// \brief Are all of the named types integral or reference-to-integral?
	/// \remarks True for enums.
	/// \remarks True for #int128_t and #uint128_t (where supported).
	template <typename... T>
	inline constexpr bool all_integral = MUU_ALL_VARIADIC_T(is_integral);

	/// \brief Is a type an arithmetic integer type, or reference to one?
	/// \remarks True for #int128_t and #uint128_t (where supported).
	/// \remarks False for enums, booleans, and character types.
	template <typename T>
	inline constexpr bool is_integer = any_same<remove_cvref<T>,
#if MUU_HAS_INT128
												int128_t,
												uint128_t,
#endif
												signed char,
												signed short,
												signed int,
												signed long,
												signed long long,
												unsigned char,
												unsigned short,
												unsigned int,
												unsigned long,
												unsigned long long>;

	/// \brief Are any of the named types arithmetic integers, or reference to them?
	/// \remarks True for #int128_t and #uint128_t (where supported).
	/// \remarks False for enums, booleans, and character types.
	template <typename... T>
	inline constexpr bool any_integer = MUU_ANY_VARIADIC_T(is_integer);

	/// \brief Are all of the named types arithmetic integers, or reference to them?
	/// \remarks True for #int128_t and #uint128_t (where supported).
	/// \remarks False for enums, booleans, and character types.
	template <typename... T>
	inline constexpr bool all_integer = MUU_ALL_VARIADIC_T(is_integer);

	/// \brief Is a type a floating-point or reference-to-floating-point?
	/// \remarks True for muu::half.
	/// \remarks True for __fp16, _Float16 and #float128_t (where supported).
	template <typename T>
	inline constexpr bool is_floating_point = std::is_floating_point_v<std::remove_reference_t<T>>
		|| any_same<remove_cvref<T>,
#if MUU_HAS_FLOAT128
		float128_t,
#endif
#if MUU_HAS_FLOAT16
		_Float16,
#endif
#if MUU_HAS_FP16
		__fp16,
#endif
		half>
		;

	/// \brief Are any of the named types floating-point or reference-to-floating-point?
	/// \remarks True for muu::half.
	/// \remarks True for _Float16 and #float128_t (where supported).
	template <typename... T>
	inline constexpr bool any_floating_point = MUU_ANY_VARIADIC_T(is_floating_point);

	/// \brief Are all of the named types floating-point or reference-to-floating-point?
	/// \remarks True for muu::half.
	/// \remarks True for _Float16 and #float128_t (where supported).
	template <typename... T>
	inline constexpr bool all_floating_point = MUU_ALL_VARIADIC_T(is_floating_point);

	/// \brief Is a type one of the standard c++ arithmetic types, or a reference to one?
	template <typename T>
	inline constexpr bool is_standard_arithmetic = std::is_arithmetic_v<std::remove_reference_t<T>>;

	/// \brief Is a type a nonstandard 'extended' arithmetic type, or a reference to one?
	/// \remarks True for muu::half.
	/// \remarks True for #int128_t, #uint128_t, __fp16, _Float16 and #float128_t (where supported).
	template <typename T>
	inline constexpr bool is_extended_arithmetic = any_same<remove_cvref<T>,
#if MUU_HAS_INT128
															int128_t,
															uint128_t,
#endif
#if MUU_HAS_FLOAT128
															float128_t,
#endif
#if MUU_HAS_FLOAT16
															_Float16,
#endif
#if MUU_HAS_FP16
															__fp16,
#endif
															half>;

	/// \brief Is a type arithmetic or reference-to-arithmetic?
	/// \remarks True for muu::half.
	/// \remarks True for #int128_t, #uint128_t, __fp16, _Float16 and #float128_t (where supported).
	template <typename T>
	inline constexpr bool is_arithmetic = is_standard_arithmetic<T> || is_extended_arithmetic<T>;

	/// \brief Are any of the named types arithmetic or reference-to-arithmetic?
	/// \remarks True for muu::half.
	/// \remarks True for #int128_t, #uint128_t, _Float16 and #float128_t (where supported).
	template <typename... T>
	inline constexpr bool any_arithmetic = MUU_ANY_VARIADIC_T(is_arithmetic);

	/// \brief Are all of the named types arithmetic or reference-to-arithmetic?
	/// \remarks True for muu::half.
	/// \remarks True for #int128_t, #uint128_t, _Float16 and #float128_t (where supported).
	template <typename... T>
	inline constexpr bool all_arithmetic = MUU_ALL_VARIADIC_T(is_arithmetic);

	/// \brief Is a type const or reference-to-const?
	template <typename T>
	inline constexpr bool is_const = std::is_const_v<std::remove_reference_t<T>>;

	/// \brief Are any of the named types const or reference-to-const?
	template <typename... T>
	inline constexpr bool any_const = MUU_ANY_VARIADIC_T(is_const);

	/// \brief Are all of the named types const or reference-to-const?
	template <typename... T>
	inline constexpr bool all_const = MUU_ALL_VARIADIC_T(is_const);

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

	/// \brief Copies the constness (or lack thereof) from one type or reference to another.
	template <typename T, typename CopyFrom>
	using copy_const = set_const<T, is_const<CopyFrom>>;

	/// \brief Is a type volatile or reference-to-volatile?
	template <typename T>
	inline constexpr bool is_volatile = std::is_volatile_v<std::remove_reference_t<T>>;

	/// \brief Are any of the named types volatile or reference-to-volatile?
	template <typename... T>
	inline constexpr bool any_volatile = MUU_ANY_VARIADIC_T(is_volatile);

	/// \brief Are all of the named types volatile or reference-to-volatile?
	template <typename... T>
	inline constexpr bool all_volatile = MUU_ALL_VARIADIC_T(is_volatile);

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

	/// \brief Copies the volatility (or lack thereof) from one type or reference to another.
	template <typename T, typename CopyFrom>
	using copy_volatile = set_volatile<T, is_volatile<CopyFrom>>;

	/// \brief Adds const and volatile qualifiers to a type or reference.
	template <typename T>
	using add_cv =
		typename impl::rebase_ref_<T, std::add_volatile_t<std::add_const_t<std::remove_reference_t<T>>>>::type;

	/// \brief Removes the topmost const and volatile qualifiers from a type or reference.
	template <typename T>
	using remove_cv =
		typename impl::rebase_ref_<T, std::remove_volatile_t<std::remove_const_t<std::remove_reference_t<T>>>>::type;

	/// \brief Sets the constness and volatility of a type or reference according to a boolean.
	template <typename T, bool ConstVolatile>
	using set_cv = std::conditional_t<ConstVolatile, add_cv<T>, remove_cv<T>>;

	/// \brief Shorthand for conditionally adding const and volatile qualifiers to a type or reference.
	template <typename T, bool AddConstVolatile>
	using conditionally_add_cv = std::conditional_t<AddConstVolatile, add_cv<T>, T>;

	/// \brief Copies consteness and volatility (or lack thereof) from one type or reference to another.
	template <typename T, typename CopyFrom>
	using copy_cv = copy_const<copy_volatile<T, CopyFrom>, CopyFrom>;

	/// \brief Is a type const and/or volatile-qualified?
	template <typename T>
	inline constexpr bool is_cv = std::is_const_v<T> || std::is_volatile_v<T>;

	/// \brief Are any of the named types const and/or volatile-qualified, or references to const and/or volatile-qualified?
	template <typename... T>
	inline constexpr bool any_cv = MUU_ANY_VARIADIC_T(is_cv);

	/// \brief Are all of the named types const and/or volatile-qualified, or references to const and/or volatile-qualified?
	template <typename... T>
	inline constexpr bool all_cv = MUU_ALL_VARIADIC_T(is_cv);

	/// \brief Is a type const, volatile, or a reference?
	template <typename T>
	inline constexpr bool is_cvref = std::is_const_v<T> || std::is_volatile_v<T> || std::is_reference_v<T>;

	/// \brief Are any of the named types const, volatile, or a reference?
	template <typename... T>
	inline constexpr bool any_cvref = MUU_ANY_VARIADIC_T(is_cvref);

	/// \brief Are all of the named types const, volatile, or a reference?
	template <typename... T>
	inline constexpr bool all_cvref = MUU_ALL_VARIADIC_T(is_cvref);

	/// \brief Copies the reference category (or lack thereof) from one type or to another.
	template <typename T, typename CopyFrom>
	using copy_ref = typename impl::copy_ref_<T, CopyFrom>::type;

	/// \brief Copies consteness, volatility and reference category (or lack thereof) from one type or to another.
	template <typename T, typename CopyFrom>
	using copy_cvref = copy_ref<copy_cv<std::remove_reference_t<T>, std::remove_reference_t<CopyFrom>>, CopyFrom>;

	/// \brief Does Child inherit from Parent?
	/// \remarks This does _not_ consider `Child == Parent` as being an "inherits from" relationship, unlike std::is_base_of.
	template <typename Child, typename Parent>
	inline constexpr bool inherits_from = std::is_base_of_v<remove_cvref<Parent>, remove_cvref<Child>> //
									   && !std::is_same_v<remove_cvref<Parent>, remove_cvref<Child>>;

	/// \brief Does Child inherit from any of the types named by Parent?
	/// \remarks This does _not_ consider `Child == Parent` as being an "inherits from" relationship, unlike std::is_base_of.
	template <typename Child, typename Parent, typename... Parents>
	inline constexpr bool inherits_from_any = (inherits_from<Child, Parent> || ... || inherits_from<Child, Parents>);

	/// \brief Does Child inherit from all of the types named by Parent?
	/// \remarks This does _not_ consider `Child == Parent` as being an "inherits from" relationship, unlike std::is_base_of.
	template <typename Child, typename Parent, typename... Parents>
	inline constexpr bool inherits_from_all = (inherits_from<Child, Parent> && ... && inherits_from<Child, Parents>);

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

	/// \brief Sets the signed-ness of a numeric type or reference according to a boolean.
	template <typename T, bool Signed>
	using set_signed = std::conditional_t<Signed, make_signed<T>, make_unsigned<T>>;

	/// \brief Copies the signed-ness (or lack thereof) from one numeric type or reference to another.
	template <typename T, typename CopyFrom>
	using copy_signed = set_signed<T, is_signed<CopyFrom>>;

	/// \brief Sets the unsigned-ness of a numeric type or reference according to a boolean.
	template <typename T, bool Unsigned>
	using set_unsigned = std::conditional_t<Unsigned, make_unsigned<T>, make_signed<T>>;

	/// \brief Copies the unsigned-ness (or lack thereof) from one numeric type or reference to another.
	template <typename T, typename CopyFrom>
	using copy_unsigned = set_unsigned<T, is_unsigned<CopyFrom>>;

	/// \brief	Provides an identity type transformation.
	/// \remark This is equivalent to C++20's std::type_identity_t.
	template <typename T>
	using type_identity = typename impl::type_identity_<T>::type;

	/// \brief	Gets the unsigned integer type with a specific number of bits for the target platform.
	template <size_t Bits>
	using unsigned_integer = typename impl::unsigned_integer_<Bits>::type;

	/// \brief	Gets the signed integer type with a specific number of bits for the target platform.
	template <size_t Bits>
	using signed_integer = typename impl::signed_integer_<Bits>::type;

	/// \brief Is a type a built-in text code unit (character) type, or reference to one?
	template <typename T>
	inline constexpr bool is_code_unit = any_same<remove_cvref<T>,
#if MUU_HAS_CHAR8
												  char8_t,
#endif
												  char,
												  wchar_t,
												  char16_t,
												  char32_t>;

	/// \brief Is a type a built-in text character (code unit) type, or reference to one?
	/// \remark This an alias for #muu::is_code_unit.
	template <typename T>
	inline constexpr bool is_character = is_code_unit<T>;

	/// \brief The rank of a pointer.
	/// \remark Answers "how many stars does it have?".
	template <typename T>
	inline constexpr size_t pointer_rank = impl::pointer_rank_<T>::value;

	/// \brief Strips every level of pointer from a type.
	template <typename T>
	using remove_all_pointers = typename impl::remove_all_pointers_<T>::type;

	/// \brief Is a type an unbounded array (`T[]`), or reference to one?
	/// \remark This is similar to C++20's std::is_unbounded_array.
	template <typename T>
	inline constexpr bool is_unbounded_array = impl::is_unbounded_array_<std::remove_reference_t<T>>::value;

	/// \brief Are any of the named types an unbounded array (`T[]`), or a reference to one?
	template <typename... T>
	inline constexpr bool any_unbounded_array = MUU_ANY_VARIADIC_T(is_unbounded_array);

	/// \brief Are all of the named types an unbounded array (`T[]`), or a reference to one?
	template <typename... T>
	inline constexpr bool all_unbounded_array = MUU_ALL_VARIADIC_T(is_unbounded_array);

	/// \brief Is a type a bounded array (`T[N]`), or reference to one?
	/// \remark This is similar to C++20's std::is_bounded_array.
	template <typename T>
	inline constexpr bool is_bounded_array = impl::is_bounded_array_<std::remove_reference_t<T>>::value;

	/// \brief Are any of the named types a bounded array (`T[N]`), or a reference to one?
	template <typename... T>
	inline constexpr bool any_bounded_array = MUU_ANY_VARIADIC_T(is_bounded_array);

	/// \brief Are all of the named types a bounded array (`T[N]`), or a reference to one?
	template <typename... T>
	inline constexpr bool all_bounded_array = MUU_ALL_VARIADIC_T(is_bounded_array);

	/// \brief Is a type an array, or reference to one?
	template <typename T>
	inline constexpr bool is_array = is_unbounded_array<T> || is_bounded_array<T>;

	/// \brief Are any of the named types arrays, or references to arrays?
	template <typename... T>
	inline constexpr bool any_array = MUU_ANY_VARIADIC_T(is_array);

	/// \brief Are all of the named types arrays, or references to arrays?
	template <typename... T>
	inline constexpr bool all_array = MUU_ALL_VARIADIC_T(is_array);

	/// \brief Is a type a free/static function, or reference to one?
	template <typename T>
	inline constexpr bool is_function = std::is_function_v<remove_cvref<T>>;

	/// \brief Are any of the named types free/static functions, or references to them?
	template <typename... T>
	inline constexpr bool any_function = MUU_ANY_VARIADIC_T(is_function);

	/// \brief Are all of the named types free/static functions, or references to them?
	template <typename... T>
	inline constexpr bool all_function = MUU_ALL_VARIADIC_T(is_function);

	/// \brief Is a type a free/static function pointer, or reference to one?
	template <typename T>
	inline constexpr bool is_function_pointer = std::is_pointer_v<std::remove_reference_t<T>> //
		&& is_function<std::remove_pointer_t<std::remove_reference_t<T>>>;

	/// \brief Are any of the named types free/static function pointers, or references to them?
	template <typename... T>
	inline constexpr bool any_function_pointer = MUU_ANY_VARIADIC_T(is_function_pointer);

	/// \brief Are all of the named types free/static function pointers, or references to them?
	template <typename... T>
	inline constexpr bool all_function_pointer = MUU_ALL_VARIADIC_T(is_function_pointer);

	/// \cond
	// clang-format off
	namespace impl
	{
		template <typename T>
		struct remove_callconv_
		{
			using type = T;
		};
		template <typename T>
		struct remove_callconv_<T*>
		{
			using type = std::add_pointer_t<typename remove_callconv_<T>::type>;
		};
		
		// free functions + pointers
		#define muu_make_remove_callconv(callconv, noex)                                                               \
		                                                                                                               \
			template <typename R, typename... Args>                                                                    \
			struct remove_callconv_<R callconv(Args...) noex>                                                          \
			{                                                                                                          \
				using type = R(Args...) noex;                                                                          \
			};                                                                                                         \
		                                                                                                               \
			template <typename R, typename... Args>                                                                    \
			struct remove_callconv_<R(callconv*)(Args...) noex>                                                        \
			{                                                                                                          \
				using type = R (*)(Args...) noex;                                                                      \
			};
		
		MUU_FOR_EACH_CALLCONV_NOEXCEPT(muu_make_remove_callconv)
		#undef muu_make_remove_callconv

		// member function pointers
		#define muu_make_remove_callconv(callconv, cvref, noex)                                                        \
		                                                                                                               \
			template <typename C, typename R, typename... P>                                                           \
			struct remove_callconv_<R (callconv C::*)(P...) cvref noex>                                                \
			{                                                                                                          \
				using type = R (C::*)(P...) cvref noex;                                                                \
			};
		
		MUU_FOR_EACH_MEMBER_CALLCONV_CVREF_NOEXCEPT(muu_make_remove_callconv)
		#undef muu_make_remove_callconv

	}
	// clang-format on
	/// \endcond

	/// \brief Removes any explicit calling convention specifiers from functions, function pointers and function references.
	template <typename T>
	using remove_callconv = copy_cvref<typename impl::remove_callconv_<remove_cvref<T>>::type, T>;

	/// \cond
	// clang-format off
	namespace impl
	{
		template <typename T>
		struct add_noexcept_
		{
			using type = T;
		};

		// free functions + pointers
		#define muu_make_add_noexcept(callconv)                                                                        \
		                                                                                                               \
			template <typename R, typename... P>                                                                       \
			struct add_noexcept_<R callconv(P...)>                                                                     \
			{                                                                                                          \
				using type = R callconv(P...) noexcept;                                                                \
			};                                                                                                         \
		                                                                                                               \
			template <typename R, typename... P>                                                                       \
			struct add_noexcept_<R(callconv*)(P...)>                                                                   \
			{                                                                                                          \
				using type = R(callconv*)(P...) noexcept;                                                              \
			};
		
		MUU_FOR_EACH_CALLCONV(muu_make_add_noexcept)
		#undef muu_make_add_noexcept

		// member function pointers
		#define muu_make_add_noexcept(callconv, cvref)                                                                 \
		                                                                                                               \
			template <typename C, typename R, typename... P>                                                           \
			struct add_noexcept_<R (callconv C::*)(P...) cvref>                                                        \
			{                                                                                                          \
				using type = R (callconv C::*)(P...) cvref noexcept;                                                   \
			};
		
		MUU_FOR_EACH_MEMBER_CALLCONV_CVREF(muu_make_add_noexcept)
		#undef muu_make_add_noexcept
	}
	// clang-format on
	/// \endcond

	/// \brief Adds a `noexcept` specifier to a functional type (or reference to one).
	template <typename T>
	using add_noexcept = copy_cvref<typename impl::add_noexcept_<remove_cvref<T>>::type, T>;

	/// \cond
	// clang-format off
	namespace impl
	{
		template <typename T>
		struct remove_noexcept_
		{
			using type = T;
		};

		// free functions + pointers
		#define muu_make_remove_noexcept(callconv)                                                                     \
		                                                                                                               \
			template <typename R, typename... P>                                                                       \
			struct remove_noexcept_<R callconv(P...) noexcept>                                                         \
			{                                                                                                          \
				using type = R callconv(P...);                                                                         \
			};                                                                                                         \
		                                                                                                               \
			template <typename R, typename... P>                                                                       \
			struct remove_noexcept_<R(callconv*)(P...) noexcept>                                                       \
			{                                                                                                          \
				using type = R(callconv*)(P...);                                                                       \
			};

		MUU_FOR_EACH_CALLCONV(muu_make_remove_noexcept)
		#undef muu_make_remove_noexcept

		// member function pointers
		#define muu_make_remove_noexcept(callconv, cvref)                                                              \
		                                                                                                               \
			template <typename C, typename R, typename... P>                                                           \
			struct remove_noexcept_<R (callconv C::*)(P...) cvref noexcept>                                            \
			{                                                                                                          \
				using type = R (callconv C::*)(P...) cvref;                                                            \
			};

		MUU_FOR_EACH_MEMBER_CALLCONV_CVREF(muu_make_remove_noexcept)
		#undef muu_make_remove_noexcept
	}
	// clang-format on
	/// \endcond

	/// \brief Removes any `noexcept` specifier from a functional type (or reference to one).
	template <typename T>
	using remove_noexcept = copy_cvref<typename impl::remove_noexcept_<remove_cvref<T>>::type, T>;

	/// \cond
	namespace impl
	{
		template <template <typename...> typename Trait, typename Enabler, typename... Args>
		struct is_detected_impl : std::false_type
		{};
		template <template <typename...> typename Trait, typename... Args>
		struct is_detected_impl<Trait, std::void_t<Trait<Args...>>, Args...> : std::true_type
		{};
		template <template <typename...> typename Trait, typename... Args>
		inline constexpr auto is_detected_ = is_detected_impl<Trait, void, Args...>::value;
	}
	/// \endcond

	/// \brief Detects if a type supports an interface.
	/// \see
	///		- [Detection Idiom](https://blog.tartanllama.xyz/detection-idiom/)
	///		- [std::experimental::is_detected](https://en.cppreference.com/w/cpp/experimental/is_detected)
	template <template <typename...> typename Trait, typename... Args>
	inline constexpr auto is_detected = impl::is_detected_<Trait, Args...>;

	/// \cond
	namespace impl
	{
		template <typename T>
		using has_indirection_operator_ = decltype(*std::declval<T>());
		template <typename T>
		using has_arrow_operator_ = decltype(std::declval<T>().operator->());
		template <typename T, typename U>
		using has_subscript_operator_ = decltype(std::declval<T>()[std::declval<U>()]);

		template <typename T>
		using has_unary_plus_operator_ = decltype(+std::declval<T>());
		template <typename T>
		using has_unary_minus_operator_ = decltype(-std::declval<T>());

		template <typename T, typename U>
		using has_addition_operator_ = decltype(std::declval<T>() + std::declval<U>());
		template <typename T, typename U>
		using has_subtraction_operator_ = decltype(std::declval<T>() - std::declval<U>());
		template <typename T, typename U>
		using has_division_operator_ = decltype(std::declval<T>() / std::declval<U>());
		template <typename T, typename U>
		using has_multiplication_operator_ = decltype(std::declval<T>() * std::declval<U>());
		template <typename T, typename U>
		using has_modulo_operator_ = decltype(std::declval<T>() % std::declval<U>());

		template <typename T>
		using has_bitwise_not_operator_ = decltype(~std::declval<T>());
		template <typename T, typename U>
		using has_bitwise_and_operator_ = decltype(std::declval<T>() & std::declval<U>());
		template <typename T, typename U>
		using has_bitwise_or_operator_ = decltype(std::declval<T>() | std::declval<U>());
		template <typename T, typename U>
		using has_bitwise_xor_operator_ = decltype(std::declval<T>() ^ std::declval<U>());
		template <typename T, typename U>
		using has_bitwise_lsh_operator_ = decltype(std::declval<T>() << std::declval<U>());
		template <typename T, typename U>
		using has_bitwise_rsh_operator_ = decltype(std::declval<T>() >> std::declval<U>());

		template <typename T>
		using has_logical_not_operator_ = decltype(!std::declval<T>());
		template <typename T, typename U>
		using has_logical_and_operator_ = decltype(std::declval<T>() && std::declval<U>());
		template <typename T, typename U>
		using has_logical_or_operator_ = decltype(std::declval<T>() || std::declval<U>());

		template <typename T, typename U>
		using has_equality_operator_ = decltype(std::declval<T>() == std::declval<U>());
		template <typename T, typename U>
		using has_inequality_operator_ = decltype(std::declval<T>() != std::declval<U>());

		template <typename T, typename U>
		using has_less_than_operator_ = decltype(std::declval<T>() < std::declval<U>());
		template <typename T, typename U>
		using has_less_than_or_equal_operator_ = decltype(std::declval<T>() <= std::declval<U>());

		template <typename T, typename U>
		using has_greater_than_operator_ = decltype(std::declval<T>() > std::declval<U>());
		template <typename T, typename U>
		using has_greater_than_or_equal_operator_ = decltype(std::declval<T>() >= std::declval<U>());

		template <typename T>
		using has_pre_increment_operator_ = decltype(++std::declval<T>());
		template <typename T>
		using has_pre_decrement_operator_ = decltype(--std::declval<T>());
		template <typename T>
		using has_post_increment_operator_ = decltype(std::declval<T>()++);
		template <typename T>
		using has_post_decrement_operator_ = decltype(std::declval<T>()--);

		template <typename T>
		using has_unambiguous_function_call_operator_ = decltype(&T::operator());

		template <typename T>
		using has_begin_member_func_ = decltype(std::declval<T>().begin());
		template <typename T>
		using has_end_member_func_ = decltype(std::declval<T>().end());

		template <typename B, typename E>
		inline constexpr bool compatible_iterators_ =
			!std::is_void_v<remove_cvref<B>>											//
			&& !std::is_void_v<remove_cvref<E>>											//
			&& is_detected<has_indirection_operator_, B>								//
			&& is_detected<has_pre_increment_operator_, std::add_lvalue_reference_t<B>> //
			&& is_detected<has_equality_operator_, B, E>								//
			&& is_detected<has_inequality_operator_, B, E>;

		template <typename T, bool = (is_detected<has_begin_member_func_, T> && is_detected<has_end_member_func_, T>)>
		inline constexpr bool has_iterator_member_funcs_ =
			compatible_iterators_<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>;
		template <typename T>
		inline constexpr bool has_iterator_member_funcs_<T, false> = false;

		template <typename T>
		using has_begin_std_func_ = decltype(std::begin(std::declval<T>()));
		template <typename T>
		using has_end_std_func_ = decltype(std::end(std::declval<T>()));

		template <typename T, bool = (is_detected<has_begin_std_func_, T> && is_detected<has_end_std_func_, T>)>
		inline constexpr bool has_iterator_std_funcs_ =
			compatible_iterators_<decltype(std::begin(std::declval<T>())), decltype(std::end(std::declval<T>()))>;
		template <typename T>
		inline constexpr bool has_iterator_std_funcs_<T, false> = false;

		template <typename T>
		using has_begin_adl_func_ = decltype(begin(std::declval<T>()));
		template <typename T>
		using has_end_adl_func_ = decltype(end(std::declval<T>()));

		template <typename T, bool = (is_detected<has_begin_adl_func_, T> && is_detected<has_end_adl_func_, T>)>
		inline constexpr bool has_iterator_adl_funcs_ =
			compatible_iterators_<decltype(begin(std::declval<T>())), decltype(end(std::declval<T>()))>;
		template <typename T>
		inline constexpr bool has_iterator_adl_funcs_<T, false> = false;

		template <typename T>
		using has_tuple_size_ = decltype(std::tuple_size<std::remove_cv_t<std::remove_reference_t<T>>>::value);
		template <typename T>
		using has_tuple_element_ = std::tuple_element_t<0, std::remove_cv_t<std::remove_reference_t<T>>>;
		template <typename T>
		using has_tuple_get_member_ = decltype(std::declval<T>().template get<0>());

		template <typename T, bool = is_detected<has_tuple_size_, T>>
		struct tuple_size_ : std::tuple_size<T>
		{};
		template <typename T>
		struct tuple_size_<T, false>
		{
			static constexpr size_t value = 0;
		};

		template <size_t I, typename T>
		MUU_NODISCARD
		MUU_ALWAYS_INLINE
		constexpr decltype(auto) get_from_tuple_like(T&& tuple_like) noexcept
		{
			if constexpr (is_detected<has_tuple_get_member_, T&&>)
			{
				return static_cast<T&&>(tuple_like).template get<I>();
			}
			else // adl
			{
				using std::get;
				return get<I>(static_cast<T&&>(tuple_like));
			}
		}

		template <typename T>
		using has_data_member_func_impl_ = decltype(std::declval<T>().data());
		template <typename T, bool = is_detected<has_data_member_func_impl_, T>>
		inline constexpr bool has_data_member_func_ =
			std::is_pointer_v<std::remove_reference_t<decltype(std::declval<T>().data())>>;
		template <typename T>
		inline constexpr bool has_data_member_func_<T, false> = false;

		template <typename T>
		using has_size_member_func_impl_ = decltype(std::declval<T>().size());
		template <typename T, bool = is_detected<has_size_member_func_impl_, T>>
		inline constexpr bool has_size_member_func_ = is_integer<decltype(std::declval<T>().size())>;
		template <typename T>
		inline constexpr bool has_size_member_func_<T, false> = false;
	}
	/// \endcond

	/// \brief True if a type has an indirection operator (`*T`).
	template <typename T>
	inline constexpr bool has_indirection_operator = is_detected<impl::has_indirection_operator_, T>;

	/// \brief True if a type has an arrow operator (`T->`).
	template <typename T>
	inline constexpr bool has_arrow_operator =
		is_detected<impl::has_arrow_operator_, T>								   //
		|| (std::is_pointer_v<std::remove_reference_t<T>>						   //
			&& (std::is_class_v<std::remove_pointer_t<std::remove_reference_t<T>>> //
				|| std::is_union_v<std::remove_pointer_t<std::remove_reference_t<T>>>));

	/// \brief True if a pair of types has a subscript operator (`T[U]`).
	template <typename T, typename U = size_t>
	inline constexpr bool has_subscript_operator = is_detected<impl::has_subscript_operator_, T, U>;

	/// \brief True if a type has a unary plus operator (`+T`).
	template <typename T>
	inline constexpr bool has_unary_plus_operator = is_detected<impl::has_unary_plus_operator_, T>;

	/// \brief True if a type has a unary minus operator (`-T`).
	template <typename T>
	inline constexpr bool has_unary_minus_operator = is_detected<impl::has_unary_minus_operator_, T>;

	/// \brief True if a pair of types has an addition operator (`T + U`).
	template <typename T, typename U = T>
	inline constexpr bool has_addition_operator = is_detected<impl::has_addition_operator_, T, U>;

	/// \brief True if a pair of types has a subtraction operator (`T - U`).
	template <typename T, typename U = T>
	inline constexpr bool has_subtraction_operator = is_detected<impl::has_subtraction_operator_, T, U>;

	/// \brief True if a pair of types has a division operator (`T / U`).
	template <typename T, typename U = T>
	inline constexpr bool has_division_operator = is_detected<impl::has_division_operator_, T, U>;

	/// \brief True if a pair of types has a multiplication operator (`T * U`).
	template <typename T, typename U = T>
	inline constexpr bool has_multiplication_operator = is_detected<impl::has_multiplication_operator_, T, U>;

	/// \brief True if a pair of types has an modulo operator (`T % U`).
	template <typename T, typename U = T>
	inline constexpr bool has_modulo_operator = is_detected<impl::has_modulo_operator_, T, U>;

	/// \brief True if a type has a unary bitwise NOT operator (`~T`).
	template <typename T>
	inline constexpr bool has_bitwise_not_operator = is_detected<impl::has_bitwise_not_operator_, T>;

	/// \brief True if a pair of types has a bitwise AND operator (`T & U`).
	template <typename T, typename U = T>
	inline constexpr bool has_bitwise_and_operator = is_detected<impl::has_bitwise_and_operator_, T, U>;

	/// \brief True if a pair of types has a bitwise OR operator (`T | U`).
	template <typename T, typename U = T>
	inline constexpr bool has_bitwise_or_operator = is_detected<impl::has_bitwise_or_operator_, T, U>;

	/// \brief True if a pair of types has a bitwise XOR operator (`T ^ U`).
	template <typename T, typename U = T>
	inline constexpr bool has_bitwise_xor_operator = is_detected<impl::has_bitwise_xor_operator_, T, U>;

	/// \brief True if a pair of types has a bitwise left-shift operator (`T << U`).
	template <typename T, typename U = T>
	inline constexpr bool has_bitwise_lsh_operator = is_detected<impl::has_bitwise_lsh_operator_, T, U>;

	/// \brief True if a pair of types has a bitwise right-shift operator (`T >> U`).
	template <typename T, typename U = T>
	inline constexpr bool has_bitwise_rsh_operator = is_detected<impl::has_bitwise_rsh_operator_, T, U>;

	/// \brief True if a type has a unary logical NOT operator (`!T`).
	template <typename T>
	inline constexpr bool has_logical_not_operator = is_detected<impl::has_logical_not_operator_, T>;

	/// \brief True if a pair of types has a logical AND operator (`T && U`).
	template <typename T, typename U = T>
	inline constexpr bool has_logical_and_operator = is_detected<impl::has_logical_and_operator_, T, U>;

	/// \brief True if a pair of types has a logical OR operator (`T || U`).
	template <typename T, typename U = T>
	inline constexpr bool has_logical_or_operator = is_detected<impl::has_logical_or_operator_, T, U>;

	/// \brief True if a pair of types has an equality operator (`T == U`).
	template <typename T, typename U = T>
	inline constexpr bool has_equality_operator = is_detected<impl::has_equality_operator_, T, U>;

	/// \brief True if a pair of types has an inequality operator (`T != U`).
	template <typename T, typename U = T>
	inline constexpr bool has_inequality_operator = is_detected<impl::has_inequality_operator_, T, U>;

	/// \brief True if a pair of types has a less-than operator (`T < U`).
	template <typename T, typename U = T>
	inline constexpr bool has_less_than_operator = is_detected<impl::has_less_than_operator_, T, U>;

	/// \brief True if a pair of types has a less-than-or-equal operator (`T <= U`).
	template <typename T, typename U = T>
	inline constexpr bool has_less_than_or_equal_operator = is_detected<impl::has_less_than_or_equal_operator_, T, U>;

	/// \brief True if a pair of types has a greater-than operator (`T > U`).
	template <typename T, typename U = T>
	inline constexpr bool has_greater_than_operator = is_detected<impl::has_greater_than_operator_, T, U>;

	/// \brief True if a pair of types has a greater-than-or-equal operator (`T >= U`).
	template <typename T, typename U = T>
	inline constexpr bool has_greater_than_or_equal_operator =
		is_detected<impl::has_greater_than_or_equal_operator_, T, U>;

	/// \brief True if a type has a pre-increment operator (`++T`).
	template <typename T>
	inline constexpr bool has_pre_increment_operator = is_detected<impl::has_pre_increment_operator_, T>;

	/// \brief True if a type has a pre-decrement operator (`--T`).
	template <typename T>
	inline constexpr bool has_pre_decrement_operator = is_detected<impl::has_pre_decrement_operator_, T>;

	/// \brief True if a type has a post-increment operator (`T++`).
	template <typename T>
	inline constexpr bool has_post_increment_operator = is_detected<impl::has_post_increment_operator_, T>;

	/// \brief True if a type has a post-decrement operator (`T--`).
	template <typename T>
	inline constexpr bool has_post_decrement_operator = is_detected<impl::has_post_decrement_operator_, T>;

	/// \brief True if a type has single non-template operator() overload.
	template <typename T>
	inline constexpr bool has_unambiguous_function_call_operator =
		is_detected<impl::has_unambiguous_function_call_operator_, remove_cvref<T>>;

	/// \brief True if a type has a member function `data()` returning a pointer.
	template <typename T>
	inline constexpr bool has_data_member_function = impl::has_data_member_func_<T>;

	/// \brief True if a type has a member function `size()` returning an arithmetic integral type.
	template <typename T>
	inline constexpr bool has_size_member_function = impl::has_size_member_func_<T>;

	/// \brief	Returns true if a type is a bounded array or a class type having `begin()` and `end()` iterators
	///			(as members, in the `std` namespace, or found via ADL).
	template <typename T>
	inline constexpr bool is_iterable = impl::has_iterator_member_funcs_<T> //
									 || impl::has_iterator_std_funcs_<T>	//
									 || impl::has_iterator_adl_funcs_<T>	//
									 || is_bounded_array<T>;

	/// \brief True if the type implements std::tuple_size and std::tuple_element.
	template <typename T>
	inline constexpr bool is_tuple_like =
		is_detected<impl::has_tuple_size_, T>&& is_detected<impl::has_tuple_element_, T>;

	/// \brief Equivalent to std::tuple_size_v, but safe to use in SFINAE contexts.
	/// \remark Returns 0 for types that do not implement std::tuple_size.
	template <typename T>
	inline constexpr size_t tuple_size = impl::tuple_size_<T>::value;

	/// \cond
	namespace impl
	{
		template <typename T>
		inline constexpr bool is_small_float_ = is_floating_point<T> && sizeof(T) <= sizeof(float)
											 && is_extended_arithmetic<T>;

		template <typename... T>
		inline constexpr bool any_small_float_ = MUU_ANY_VARIADIC_T(is_small_float_);

		template <typename T>
		inline constexpr bool is_large_float_ = is_floating_point<T> && sizeof(T) >= sizeof(long double)
											 && is_extended_arithmetic<T>;
	}
	/// \endcond

	/// \brief Promotes 'small' extended arithmetic floating-point types (e.g. __fp16, etc.) to `float`.
	/// \remarks CV qualifiers and reference categories are preserved.
	template <typename T>
	using promote_if_small_float = std::conditional_t<impl::is_small_float_<T>, //
													  typename impl::rebase_ref_<T, copy_cv<float, T>>::type,
													  T>;

	/// \brief Demotes 'large' extended arithmetic floating-point types (e.g. __float128) to `long double`.
	/// \remarks CV qualifiers and reference categories are preserved.
	template <typename T>
	using demote_if_large_float = std::conditional_t<impl::is_large_float_<T>, //
													 typename impl::rebase_ref_<T, copy_cv<long double, T>>::type,
													 T>;

	/// \brief Clamps extended arithmetic floating-point types (e.g. __fp16, __float128) to `float` or `long double`.
	/// \remarks CV qualifiers and reference categories are preserved.
	template <typename T>
	using clamp_to_standard_float = demote_if_large_float<promote_if_small_float<T>>;

	/// \cond
	namespace impl
	{
		template <typename T, size_t MinD = 0, size_t MaxD = static_cast<size_t>(-1)>
		inline constexpr bool is_vector_ = false;

		template <typename S, size_t D, size_t MinD, size_t MaxD>
		inline constexpr bool is_vector_<::muu::vector<S, D>, MinD, MaxD> = (D >= MinD && D <= MaxD);

		template <typename S, size_t D, size_t MinD, size_t MaxD>
		inline constexpr bool is_vector_<::muu::impl::vector_base<S, D>, MinD, MaxD> = (D >= MinD && D <= MaxD);

		template <typename T>
		inline constexpr bool is_quaternion_ = false;
		template <typename S>
		inline constexpr bool is_quaternion_<::muu::quaternion<S>> = true;
		template <typename S>
		inline constexpr bool is_quaternion_<::muu::impl::quaternion_<S>> = true;

		template <typename T,
				  size_t MinR = 0,
				  size_t MinC = 0,
				  size_t MaxR = static_cast<size_t>(-1),
				  size_t MaxC = static_cast<size_t>(-1)>
		inline constexpr bool is_matrix_ = false;

		template <typename S, size_t R, size_t C, size_t MinR, size_t MinC, size_t MaxR, size_t MaxC>
		inline constexpr bool is_matrix_<::muu::matrix<S, R, C>, MinR, MinC, MaxR, MaxC> = //
			(R >= MinR && C >= MinC && R <= MaxR && C <= MaxC);

		template <typename S, size_t R, size_t C, size_t MinR, size_t MinC, size_t MaxR, size_t MaxC>
		inline constexpr bool is_matrix_<::muu::impl::matrix_<S, R, C>, MinR, MinC, MaxR, MaxC> = //
			(R >= MinR && C >= MinC && R <= MaxR && C <= MaxC);

		// promotes ints to doubles, keeps floats as-is, as per the behaviour of std::sqrt, std::lerp, etc.
		template <typename... T>
		using std_math_common_type = highest_ranked<std::conditional_t<is_integer<T>, double, T>...>;

		struct any_type
		{
			template <typename T>
			/*implicit*/ constexpr operator T() const noexcept;
		};

		template <typename T>
		inline constexpr bool always_pass_readonly_param_by_value = false;

#if MUU_HAS_VECTORCALL

		template <typename T>
		inline constexpr bool is_vectorcall_simd_intrinsic = any_same<remove_cvref<T>,
																	  __m64,
																	  __m128,
																	  __m128i,
																	  __m128d,
																	  __m256,
																	  __m256d,
																	  __m256i,
																	  __m512,
																	  __m512d,
																	  __m512i>;

		template <typename T>
		using is_aggregate_5_args_ =
			decltype(T{ { any_type{} }, { any_type{} }, { any_type{} }, { any_type{} }, { any_type{} } });
		template <typename T>
		using is_aggregate_4_args_ = decltype(T{ { any_type{} }, { any_type{} }, { any_type{} }, { any_type{} } });
		template <typename T>
		using is_aggregate_3_args_ = decltype(T{ { any_type{} }, { any_type{} }, { any_type{} } });
		template <typename T>
		using is_aggregate_2_args_ = decltype(T{ { any_type{} }, { any_type{} } });
		template <typename T>
		using is_aggregate_1_arg_ = decltype(T{ { any_type{} } });

		template <typename T>
		struct hva_member_
		{
			using type					  = T;
			static constexpr size_t arity = 1;
		};
		template <typename T, size_t N>
		struct hva_member_<T[N]>
		{
			using type					  = T;
			static constexpr size_t arity = N;
		};

		template <typename T>
		inline constexpr bool is_hva_scalar =
			any_same<T, float, double, long double> || is_vectorcall_simd_intrinsic<T>;

		template <typename Scalar, typename T>
		inline constexpr bool can_be_hva_of = std::is_class_v<T>						   //
										   && !std::is_empty_v<T>						   //
										   && std::is_standard_layout_v<T>				   //
										   && std::is_trivially_default_constructible_v<T> //
										   && std::is_trivially_copyable_v<T>			   //
										   && std::is_trivially_destructible_v<T>		   //
										   && is_hva_scalar<Scalar>						   //
										   && sizeof(T) >= sizeof(Scalar)				   //
										   && sizeof(T) <= sizeof(Scalar) * 4			   //
										   && sizeof(T) % sizeof(Scalar) == 0			   //
										   && alignof(T) == alignof(Scalar);

		template <typename T>
		inline constexpr bool can_be_hva = can_be_hva_of<float, T>		 //
										|| can_be_hva_of<double, T>		 //
										|| can_be_hva_of<long double, T> //
										|| can_be_hva_of<__m64, T>		 //
										|| can_be_hva_of<__m128, T>		 //
										|| can_be_hva_of<__m128i, T>	 //
										|| can_be_hva_of<__m128d, T>	 //
										|| can_be_hva_of<__m256, T>		 //
										|| can_be_hva_of<__m256d, T>	 //
										|| can_be_hva_of<__m256i, T>	 //
										|| can_be_hva_of<__m512, T>		 //
										|| can_be_hva_of<__m512d, T>	 //
										|| can_be_hva_of<__m512i, T>;

		template <typename T, typename... Members>
		struct is_valid_hva_
		{
			// "What is an HVA?"
			// https://docs.microsoft.com/en-us/cpp/cpp/vectorcall?view=vs-2019
			//
			// a "homogeneous vector aggregate" must:
			// - have between 1 and 4 members (if any members are N-length arrays they count as N members)
			// - have all members be the same type
			// - have the member type be float or simd intrinsic vector types
			// - have the same alignment as its member type (no padding/over-alignment)
			//
			// some of these points were ambiguous in the vectorcall doc, but i figured them out via experimentation
			// on godbolt https://godbolt.org/z/vn8aKv

			static constexpr bool value =

				// all members are non-reference, non-volatile floats or simd intrinsics
				(true && ...
				 && (is_hva_scalar<typename hva_member_<Members>::type> //
					 && !std::is_reference_v<Members>					//
					 && !std::is_volatile_v<Members>))					//

				// min 1 member
				&& (0 + ... + hva_member_<Members>::arity) >= 1

				// max 4 members
				&& (0 + ... + hva_member_<Members>::arity) <= 4

				// all members the same type
				&& all_same<typename hva_member_<Members>::type...>

				// no padding
				&& sizeof(T) == (0u + ... + sizeof(Members))

				// alignment matches member type
				&& alignof(T) == alignof(most_aligned<typename hva_member_<Members>::type...>);
		};

		template <typename T>
		constexpr auto is_hva_(T&& obj) noexcept
		{
			if constexpr (can_be_hva<T> && std::is_aggregate_v<T>)
			{
				if constexpr (is_detected<is_aggregate_5_args_, T>) // five or more
					return std::false_type{};
				else if constexpr (is_detected<is_aggregate_4_args_, T>) // four
				{
					auto&& [a, b, c, d] = static_cast<T&&>(obj);
					MUU_UNUSED(a);
					MUU_UNUSED(b);
					MUU_UNUSED(c);
					MUU_UNUSED(d);
					return std::bool_constant<
						is_valid_hva_<T, decltype(a), decltype(b), decltype(c), decltype(d)>::value>{};
				}
				else if constexpr (is_detected<is_aggregate_3_args_, T>) // three
				{
					auto&& [a, b, c] = static_cast<T&&>(obj);
					MUU_UNUSED(a);
					MUU_UNUSED(b);
					MUU_UNUSED(c);
					return std::bool_constant<is_valid_hva_<T, decltype(a), decltype(b), decltype(c)>::value>{};
				}
				else if constexpr (is_detected<is_aggregate_2_args_, T>) // two
				{
					auto&& [a, b] = static_cast<T&&>(obj);
					MUU_UNUSED(a);
					MUU_UNUSED(b);
					return std::bool_constant<is_valid_hva_<T, decltype(a), decltype(b)>::value>{};
				}
				else if constexpr (is_detected<is_aggregate_1_arg_, T>) // one
				{
					auto&& [a] = static_cast<T&&>(obj);
					MUU_UNUSED(a);
					return std::bool_constant<is_valid_hva_<T, decltype(a)>::value>{};
				}
				else
					return std::false_type{};
			}
			else
				return std::false_type{};
		}

		template <typename T>
		inline constexpr bool is_hva = decltype(is_hva_(std::declval<T>()))::value;

#else // ^^^ MUU_HAS_VECTORCALL / vvv !MUU_HAS_VECTORCALL

		template <typename T>
		inline constexpr bool is_vectorcall_simd_intrinsic = false;

		template <typename T>
		inline constexpr bool is_hva_scalar = false;

		template <typename Scalar, typename T>
		inline constexpr bool can_be_hva_of = false;

		template <typename T>
		inline constexpr bool can_be_hva = false;

		template <typename T>
		inline constexpr bool is_hva = false;

#endif // !MUU_HAS_VECTORCALL

		template <typename T, bool Vector = false>
		struct readonly_param_base_
		{
			static_assert(!std::is_reference_v<T>);

			using raw_type = std::remove_cv_t<T>;

			using type = std::conditional_t<
				is_floating_point<raw_type>																			 //
					|| is_integral<raw_type>																		 //
					|| std::is_scalar_v<raw_type>																	 //
					|| std::is_fundamental_v<raw_type>																 //
					|| std::is_same_v<raw_type, muu::half>															 //
					|| always_pass_readonly_param_by_value<raw_type>												 //
					|| (Vector																						 //
						&& MUU_HAS_VECTORCALL																		 //
						&& (is_vectorcall_simd_intrinsic<raw_type> || is_hva<raw_type>))							 //
					|| ((std::is_class_v<raw_type> || std::is_union_v<raw_type>)									 //
						&&(std::is_trivially_copyable_v<raw_type> || std::is_nothrow_copy_constructible_v<raw_type>) //
						&&std::is_nothrow_destructible_v<raw_type>													 //
						&& sizeof(raw_type) <= (sizeof(void*) * 2u)),
				raw_type,
				std::add_lvalue_reference_t<std::add_const_t<T>>>;
		};
		template <typename T>
		struct readonly_param_ : readonly_param_base_<T, false>
		{};
		template <typename T>
		struct vector_param_ : readonly_param_base_<T, true> // vector semantics
		{};

	}
	/// \endcond

	/// \brief Resolves to `T` or `const T&` based on the performance characteristics of `T` being passed by value into a function.
	/// \detail This trait examines things like:
	/// - Is `T` an integer/float/pointer/enum/'scalar' type?
	/// - Is `T` trivially copyable or nothrow copyable?
	/// - Is `T` large or small?
	template <typename T>
	using readonly_param = typename impl::readonly_param_<std::remove_reference_t<T>>::type;

	/// \brief Same as #readonly_param, but also takes SIMD vector types and MSVC's `__vectorcall` into account.
	/// \see readonly_param
	template <typename T>
	using vector_param = typename impl::vector_param_<std::remove_reference_t<T>>::type;

	/// \cond
	namespace impl
	{
		template <typename... T>
		inline constexpr bool pass_readonly_by_reference = (true || ... || std::is_reference_v<readonly_param<T>>);

		template <typename... T>
		inline constexpr bool pass_readonly_by_value = !pass_readonly_by_reference<T...>;

		template <typename... T>
		inline constexpr bool pass_vector_by_reference = (true || ... || std::is_reference_v<vector_param<T>>);

		template <typename... T>
		inline constexpr bool pass_vector_by_value = !pass_vector_by_reference<T...>;

		template <typename T, bool = has_unary_plus_operator<T>>
		inline constexpr bool decays_to_pointer_by_unary_plus_ = false;

		template <typename T>
		inline constexpr bool decays_to_pointer_by_unary_plus_<T, true> =
			std::is_pointer_v<decltype(+std::declval<T>())>;

		template <typename T, bool = has_unary_plus_operator<T>>
		inline constexpr bool decays_to_function_pointer_by_unary_plus_ = false;

		template <typename T>
		inline constexpr bool decays_to_function_pointer_by_unary_plus_<T, true> =
			is_function_pointer<decltype(+std::declval<T>())>;
	}
	/// \endcond

	/// \brief Evaluates to true if an instance of `T` decays to a pointer by explicit unary plus.
	template <typename T>
	inline constexpr bool decays_to_pointer_by_unary_plus = impl::decays_to_pointer_by_unary_plus_<T>;

	/// \brief Evaluates to true if an instance of `T` decays to a free-function pointer by explicit unary plus.
	template <typename T>
	inline constexpr bool decays_to_function_pointer_by_unary_plus = impl::decays_to_function_pointer_by_unary_plus_<T>;

	/// \brief  A common epsilon type when comparing floating-point types named by T.
	template <typename... T>
	using epsilon_type = impl::std_math_common_type<remove_cvref<T>...>;

	/// \brief  The default floating-point epsilon value used when comparing floating-point types named by T.
	template <typename... T>
	inline constexpr epsilon_type<T...> default_epsilon = constants<epsilon_type<T...>>::default_epsilon;

	/// \brief A tag type for encoding/parameterizing a single index.
	template <size_t N>
	using index_tag = std::integral_constant<size_t, N>;

	/// \brief An index_tag specialization for representing the X axis.
	using x_axis_tag = index_tag<0>;

	/// \brief An index_tag specialization for representing the Y axis.
	using y_axis_tag = index_tag<1>;

	/// \brief An index_tag specialization for representing the Z axis.
	using z_axis_tag = index_tag<2>;

	/// \brief An index_tag specialization for representing the W axis.
	using w_axis_tag = index_tag<3>;

	/// \cond
	namespace impl
	{
		//=======================================================================
		// is_stateless_lambda - false (fails basic traits test)
		//=======================================================================

		// clang-format off

		template <typename T>
		inline constexpr bool could_be_stateless_lambda // see https://en.cppreference.com/w/cpp/language/lambda
			 = std::is_class_v<T>
			&& std::is_empty_v<T>
			&& (size_of_<T>::value <= 1)
			&& (std::is_trivially_default_constructible_v<T> || !std::is_default_constructible_v<T>)
			&& (std::is_trivially_copy_assignable_v<T> || !std::is_copy_assignable_v<T>)
			&& (std::is_trivially_move_assignable_v<T> || !std::is_move_assignable_v<T>)
			&& std::is_trivially_copy_constructible_v<T>
			&& std::is_trivially_move_constructible_v<T>
			&& std::is_trivially_destructible_v<T>;

		// clang-format on

		template <typename T, typename FuncPtr, bool = could_be_stateless_lambda<T>>
		struct is_stateless_lambda_ : std::false_type
		{
			static_assert(std::is_void_v<FuncPtr> || is_function_pointer<FuncPtr>,
						  "CompatibleWithFunc must be one of: void, function, function pointer, function reference");
		};

		//=======================================================================
		// is_stateless_lambda - user-specified "compatible with" function
		//=======================================================================

		template <typename T, typename FuncPtr>
		inline constexpr bool is_invocable_as_ = false;
		template <typename T, typename R, typename... Args>
		inline constexpr bool is_invocable_as_<T, R (*)(Args...)> = std::is_invocable_r_v<R, T, Args...>;
		template <typename T, typename R, typename... Args>
		inline constexpr bool is_invocable_as_<T, R (*)(Args...) noexcept> =
			std::is_nothrow_invocable_r_v<R, T, Args...>;

		template <typename T, typename FuncPtr>
		struct is_stateless_lambda_<T, FuncPtr, true>
			: std::bool_constant<is_implicitly_nothrow_convertible<T, FuncPtr> //
								 || is_invocable_as_<T, FuncPtr>>
		{
			static_assert(is_function_pointer<FuncPtr>,
						  "CompatibleWithFunc must be one of: void, function, function pointer, function reference");
		};

		//=======================================================================
		// is_stateless_lambda - determine function type by decay
		//=======================================================================

		template <typename T, bool = decays_to_function_pointer_by_unary_plus<T>>
		struct is_stateless_lambda_by_decay_ : is_stateless_lambda_<T, decltype(+std::declval<T>())>
		{};
		template <typename T>
		struct is_stateless_lambda_by_decay_<T, false> : std::false_type
		{};

		template <typename T>
		struct is_stateless_lambda_<T, void, true> // no "compatible with" function; have to do magic
			: is_stateless_lambda_by_decay_<T>
		{};

	}
	/// \endcond

	/// \brief Evaluates to true if `T` is a stateless lambda type, or a reference to one.
	///
	/// \attention	The language provides no way of genuinely identifying a stateless lambda. This trait is based on
	///				a highly-selective set of traits that _in the general case_ will only select for stateless lambdas.
	///				It is still possible to manually construct a type that meets the criteria without being an actual
	///				C++ lambda.
	template <typename T, typename CompatibleWithFunc = void>
	inline constexpr bool is_stateless_lambda = POXY_IMPLEMENTATION_DETAIL(
		impl::is_stateless_lambda_<remove_cvref<T>,
								   std::conditional_t<std::is_void_v<CompatibleWithFunc>,
													  void,
													  std::add_pointer_t<remove_callconv<std::remove_pointer_t<
														  std::remove_reference_t<CompatibleWithFunc>>>> //
													  >													 //
								   >::value);

	/// \cond
	// clang-format off
	namespace impl
	{
		template <typename T>
		using arity_remove_decorations_ = remove_callconv<remove_noexcept<std::remove_pointer_t<remove_cvref<T>>>>;

		template <typename T,
				  bool = (std::is_class_v<T> || std::is_union_v<T>) && has_unambiguous_function_call_operator<T>,
				  bool = is_stateless_lambda<T> || decays_to_function_pointer_by_unary_plus<T>>
		struct arity_ : std::integral_constant<size_t, 0>
		{};

		// free functions
		template <typename R, typename... P>
		struct arity_<R(P...), false, false> : std::integral_constant<size_t, sizeof...(P)>
		{};
		template <typename R, typename... P>
		struct arity_<R(P...), false, true> : arity_<R(P...), false, false>
		{};

		// classes/unions with an unambiguous function call operator
		template <typename T, bool SL>
		struct arity_<T, true, SL> : arity_<arity_remove_decorations_<decltype(&remove_cvref<T>::operator())>>
		{};

		// stateless lambdas (and things that decay to function pointer by unary plus)
		template <typename T>
		struct arity_<T, false, true> : arity_<arity_remove_decorations_<decltype(+std::declval<T>())>>
		{};

		// member function pointers
		#define muu_make_arity(cvref)                                                                                  \
		                                                                                                               \
			template <typename C, typename R, typename... P>                                                           \
			struct arity_<R (C::*)(P...) cvref, false, false> : std::integral_constant<size_t, sizeof...(P)>           \
			{};

		MUU_FOR_EACH_CVREF(muu_make_arity)
		#undef muu_make_arity

	}
	// clang-format on
	/// \endcond

	/// \brief	Returns the arity (parameter count) of a function, function pointer, stateless lambda,
	///			or class/union type with an unambiguous `operator()` overload.
	template <typename T>
	inline constexpr size_t arity = impl::arity_<impl::arity_remove_decorations_<T>>::value;

	/** @} */ // meta
}

#undef MUU_ANY_VARIADIC_T
#undef MUU_ALL_VARIADIC_T

#include "impl/header_end.h"
