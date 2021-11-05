// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

#include "../fwd.h"

MUU_DISABLE_WARNINGS;
#include "std_type_traits.h"
#include "std_utility.h"
#if MUU_HAS_VECTORCALL
	#include <intrin.h>
#endif
MUU_ENABLE_WARNINGS;
#include "header_start.h"
MUU_PRAGMA_MSVC(warning(disable : 4296)) // condition always true/false

namespace muu
{
	/// \addtogroup		meta
	/// @{

#if defined(__cpp_lib_remove_cvref) && __cpp_lib_remove_cvref >= 201711

	template <typename T>
	using remove_cvref = std::remove_cvref_t<T>;

#else

	/// \brief	Removes the topmost const, volatile and reference qualifiers from a type.
	/// \remark This is equivalent to C++20's std::remove_cvref_t.
	template <typename T>
	using remove_cvref = std::remove_cv_t<std::remove_reference_t<T>>;

#endif

	/// \brief	Evaluates to false but with delayed, type-dependent evaluation.
	/// \details Allows you to do things like this:
	/// \cpp
	/// if constexpr (is_some_fancy_type<T>)
	///		// ...
	/// else
	///		static_assert(always_false<T>, "Oh no, T was bad!");
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

		template <typename T, typename U>
		struct rebase_pointer_
		{
			static_assert(std::is_pointer_v<T>);
			using type = U*;
		};
		template <typename T, typename U>
		struct rebase_pointer_<const volatile T*, U>
		{
			using type = std::add_const_t<std::add_volatile_t<U>>*;
		};
		template <typename T, typename U>
		struct rebase_pointer_<volatile T*, U>
		{
			using type = std::add_volatile_t<U>*;
		};
		template <typename T, typename U>
		struct rebase_pointer_<const T*, U>
		{
			using type = std::add_const_t<U>*;
		};
		template <typename T, typename U>
		struct rebase_pointer_<T&, U>
		{
			using type = typename rebase_pointer_<T, U>::type&;
		};
		template <typename T, typename U>
		struct rebase_pointer_<T&&, U>
		{
			using type = typename rebase_pointer_<T, U>::type&&;
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
			using type = typename remove_enum_<T>::type&;
		};
		template <typename T>
		struct remove_enum_<T&&, true>
		{
			using type = typename remove_enum_<T>::type&&;
		};

		template <typename T>
		struct remove_noexcept_
		{
			using type = T;
		};
		template <typename T>
		struct remove_noexcept_<const T>
		{
			using type = const typename remove_noexcept_<T>::type;
		};
		template <typename T>
		struct remove_noexcept_<volatile T>
		{
			using type = volatile typename remove_noexcept_<T>::type;
		};
		template <typename T>
		struct remove_noexcept_<const volatile T>
		{
			using type = const volatile typename remove_noexcept_<T>::type;
		};
		template <typename T>
		struct remove_noexcept_<T&>
		{
			using type = typename remove_noexcept_<T>::type&;
		};
		template <typename T>
		struct remove_noexcept_<T&&>
		{
			using type = typename remove_noexcept_<T>::type&&;
		};
		template <typename R, typename... P>
		struct remove_noexcept_<R(P...) noexcept>
		{
			using type = R(P...);
		};
		template <typename R, typename... P>
		struct remove_noexcept_<R (*)(P...) noexcept>
		{
			using type = R (*)(P...);
		};
		template <typename C, typename R, typename... P>
		struct remove_noexcept_<R (C::*)(P...) noexcept>
		{
			using type = R (C::*)(P...);
		};
		template <typename C, typename R, typename... P>
		struct remove_noexcept_<R (C::*)(P...)& noexcept>
		{
			using type = R (C::*)(P...) &;
		};
		template <typename C, typename R, typename... P>
		struct remove_noexcept_<R (C::*)(P...)&& noexcept>
		{
			using type = R (C::*)(P...) &&;
		};
		template <typename C, typename R, typename... P>
		struct remove_noexcept_<R (C::*)(P...) const noexcept>
		{
			using type = R (C::*)(P...) const;
		};
		template <typename C, typename R, typename... P>
		struct remove_noexcept_<R (C::*)(P...) const& noexcept>
		{
			using type = R (C::*)(P...) const&;
		};
		template <typename C, typename R, typename... P>
		struct remove_noexcept_<R (C::*)(P...) const&& noexcept>
		{
			using type = R (C::*)(P...) const&&;
		};
		template <typename C, typename R, typename... P>
		struct remove_noexcept_<R (C::*)(P...) volatile noexcept>
		{
			using type = R (C::*)(P...) volatile;
		};
		template <typename C, typename R, typename... P>
		struct remove_noexcept_<R (C::*)(P...) volatile& noexcept>
		{
			using type = R (C::*)(P...) volatile&;
		};
		template <typename C, typename R, typename... P>
		struct remove_noexcept_<R (C::*)(P...) volatile&& noexcept>
		{
			using type = R (C::*)(P...) volatile&&;
		};
		template <typename C, typename R, typename... P>
		struct remove_noexcept_<R (C::*)(P...) const volatile noexcept>
		{
			using type = R (C::*)(P...) const volatile;
		};
		template <typename C, typename R, typename... P>
		struct remove_noexcept_<R (C::*)(P...) const volatile& noexcept>
		{
			using type = R (C::*)(P...) const volatile&;
		};
		template <typename C, typename R, typename... P>
		struct remove_noexcept_<R (C::*)(P...) const volatile&& noexcept>
		{
			using type = R (C::*)(P...) const volatile&&;
		};

		template <typename T>
		struct alignment_of_
		{
			static constexpr size_t value = alignof(T);
		};
		template <>
		struct alignment_of_<void>
		{
			static constexpr size_t value = 1;
		};
		template <typename R, typename... P>
		struct alignment_of_<R(P...)>
		{
			static constexpr size_t value = 1;
		};
		template <typename R, typename... P>
		struct alignment_of_<R(P...) noexcept>
		{
			static constexpr size_t value = 1;
		};

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
			using type = std::conditional_t<(sizeof(U) < sizeof(T)), T, U>;
		};
		template <typename T, typename U, typename... V>
		struct largest_<T, U, V...>
		{
			using type = typename largest_<T, typename largest_<U, V...>::type>::type;
		};

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
			using type = std::conditional_t<(sizeof(U) < sizeof(T)), U, T>;
		};
		template <typename T, typename U, typename... V>
		struct smallest_<T, U, V...>
		{
			using type = typename smallest_<T, typename smallest_<U, V...>::type>::type;
		};

		template <typename...>
		struct most_aligned_;
		template <typename T>
		struct most_aligned_<T>
		{
			using type = T;
		};
		template <typename T>
		struct most_aligned_<T, void>
		{
			using type = T;
		};
		template <typename T>
		struct most_aligned_<void, T>
		{
			using type = T;
		};
		template <typename T, typename R, typename P>
		struct most_aligned_<T, R(P...)>
		{
			using type = T;
		};
		template <typename T, typename R, typename P>
		struct most_aligned_<R(P...), T>
		{
			using type = T;
		};
		template <typename T, typename R, typename P>
		struct most_aligned_<T, R(P...) noexcept>
		{
			using type = T;
		};
		template <typename T, typename R, typename P>
		struct most_aligned_<R(P...) noexcept, T>
		{
			using type = T;
		};
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

		template <typename...>
		struct least_aligned_;
		template <typename T>
		struct least_aligned_<T>
		{
			using type = T;
		};
		template <typename T>
		struct least_aligned_<T, void>
		{
			using type = T;
		};
		template <typename T>
		struct least_aligned_<void, T>
		{
			using type = T;
		};
		template <typename T, typename R, typename P>
		struct least_aligned_<T, R(P...)>
		{
			using type = T;
		};
		template <typename T, typename R, typename P>
		struct least_aligned_<R(P...), T>
		{
			using type = T;
		};
		template <typename T, typename R, typename P>
		struct least_aligned_<T, R(P...) noexcept>
		{
			using type = T;
		};
		template <typename T, typename R, typename P>
		struct least_aligned_<R(P...) noexcept, T>
		{
			using type = T;
		};
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
	template <typename... T>
	using largest = typename impl::largest_<T...>::type;

	/// \brief The smallest type from a set of types.
	template <typename... T>
	using smallest = typename impl::smallest_<T...>::type;

	/// \brief The sum of `sizeof()` for all of the types named by T.
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
	/// \remark This equivalent to `(std::is_same_v<T, U1> || std::is_same_v<T, U2> || ...)`.
	template <typename T, typename... U>
	inline constexpr bool any_same = (false || ... || std::is_same_v<T, U>);

	/// \brief	True if all the types named by T and U are exactly the same.
	/// \remark This equivalent to `(std::is_same_v<T, U1> && std::is_same_v<T, U2> && ...)`.
	template <typename T, typename... U>
	inline constexpr bool all_same = (true && ... && std::is_same_v<T, U>);

	/// \brief	Returns the index of the first appearance of the type T in the type list U.
	template <typename T, typename... U>
	inline constexpr size_t index_of_type = impl::index_of_type_<T, 0, U...>::value;

	/// \brief	True if `From` is implicitly convertible to `To`.
	template <typename From, typename To>
	inline constexpr bool is_implicitly_convertible = std::is_convertible_v<From, To>;

	/// \brief	True if `From` is implicitly convertible to any of the types named by `To`.
	template <typename From, typename... To>
	inline constexpr bool is_implicitly_convertible_to_any = (false || ... || is_implicitly_convertible<From, To>);

	/// \brief	True if `From` is implicitly convertible to all of the types named by `To`.
	template <typename From, typename... To>
	inline constexpr bool is_implicitly_convertible_to_all = (sizeof...(To) > 0)
														  && (true && ... && is_implicitly_convertible<From, To>);

	/// \brief	True if all of the types named by `From` are implicitly convertible to `To`.
	template <typename To, typename... From>
	inline constexpr bool all_implicitly_convertible_to = (sizeof...(From) > 0)
													   && (true && ... && is_implicitly_convertible<From, To>);

	/// \brief	True if `From` is explicitly convertible to `To`.
	template <typename From, typename To>
	inline constexpr bool is_explicitly_convertible =
		!std::is_convertible_v<From, To> && std::is_constructible_v<To, From>;

	/// \brief	True if `From` is explicitly convertible to any of the types named by `To`.
	template <typename From, typename... To>
	inline constexpr bool is_explicitly_convertible_to_any = (false || ... || is_explicitly_convertible<From, To>);

	/// \brief	True if `From` is explicitly convertible to all of the types named by `To`.
	template <typename From, typename... To>
	inline constexpr bool is_explicitly_convertible_to_all = (sizeof...(To) > 0)
														  && (true && ... && is_explicitly_convertible<From, To>);

	/// \brief	True if all of the types named by `From` are explicitly convertible to `To`.
	template <typename To, typename... From>
	inline constexpr bool all_explicitly_convertible_to = (sizeof...(From) > 0)
													   && (true && ... && is_explicitly_convertible<From, To>);

	/// \brief	True if `From` is implicitly _or_ explicitly convertible to `To`.
	template <typename From, typename To>
	inline constexpr bool is_convertible = std::is_convertible_v<From, To> || std::is_constructible_v<To, From>;

	/// \brief	True if `From` is implicitly _or_ explicitly convertible to any of the types named by `To`.
	template <typename From, typename... To>
	inline constexpr bool is_convertible_to_any = (false || ... || is_convertible<From, To>);

	/// \brief	True if `From` is implicitly _or_ explicitly convertible to all of the types named by `To`.
	template <typename From, typename... To>
	inline constexpr bool is_convertible_to_all = (sizeof...(To) > 0) && (true && ... && is_convertible<From, To>);

	/// \brief	True if all of the types named by `From` are implicitly _or_ explicitly convertible to `To`.
	template <typename To, typename... From>
	inline constexpr bool all_convertible_to = (sizeof...(From) > 0) && (true && ... && is_convertible<From, To>);

	/// \brief	True if `From` is implicitly nothrow-convertible to `To`.
	template <typename From, typename To>
	inline constexpr bool is_implicitly_nothrow_convertible = impl::is_implicitly_nothrow_convertible_<From, To>;

	/// \brief	True if `From` is implicitly nothrow-convertible to any of the types named by `To`.
	template <typename From, typename... To>
	inline constexpr bool is_implicitly_nothrow_convertible_to_any = (false || ...
																	  || is_implicitly_nothrow_convertible<From, To>);

	/// \brief	True if `From` is implicitly nothrow-convertible to all of the types named by `To`.
	template <typename From, typename... To>
	inline constexpr bool is_implicitly_nothrow_convertible_to_all = (sizeof...(To) > 0)
																  && (true && ...
																	  && is_implicitly_nothrow_convertible<From, To>);

	/// \brief	True if all of the types named by `From` are implicitly nothrow-convertible to `To`.
	template <typename To, typename... From>
	inline constexpr bool all_implicitly_nothrow_convertible_to = (sizeof...(From) > 0)
															   && (true && ...
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
	inline constexpr bool is_explicitly_nothrow_convertible_to_all = (sizeof...(To) > 0)
																  && (true && ...
																	  && is_explicitly_nothrow_convertible<From, To>);

	/// \brief	True if all of the types named by `From` are explicitly nothrow-convertible to `To`.
	template <typename To, typename... From>
	inline constexpr bool all_explicitly_nothrow_convertible_to = (sizeof...(From) > 0)
															   && (true && ...
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
	inline constexpr bool is_nothrow_convertible_to_all = (sizeof...(To) > 0)
													   && (true && ... && is_nothrow_convertible<From, To>);

	/// \brief	True if all of the types named by `From` are implicitly _or_ explicitly nothrow-convertible to `To`.
	template <typename To, typename... From>
	inline constexpr bool all_nothrow_convertible_to = (sizeof...(From) > 0)
													&& (true && ... && is_nothrow_convertible<From, To>);

	/// \brief Is a type an enum or reference-to-enum?
	template <typename T>
	inline constexpr bool is_enum = std::is_enum_v<std::remove_reference_t<T>>;

	/// \brief Are any of the named types enums or reference-to-enum?
	template <typename T, typename... U>
	inline constexpr bool any_enum = (is_enum<T> || ... || is_enum<U>);

	/// \brief Are all of the named types enums or reference-to-enum?
	template <typename T, typename... U>
	inline constexpr bool all_enum = (is_enum<T> && ... && is_enum<U>);

	/// \brief Is a type a C++11 scoped enum class, or reference to one?
	template <typename T>
	inline constexpr bool is_scoped_enum =
		is_enum<T> && !std::is_convertible_v<remove_cvref<T>, remove_enum<remove_cvref<T>>>;

	/// \brief Is a type a pre-C++11 unscoped enum, or reference to one?
	template <typename T>
	inline constexpr bool is_legacy_enum =
		is_enum<T>&& std::is_convertible_v<remove_cvref<T>, remove_enum<remove_cvref<T>>>;

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
	template <typename T, typename... U>
	inline constexpr bool any_unsigned = (is_unsigned<T> || ... || is_unsigned<U>);

	/// \brief Are all of the named types unsigned or reference-to-unsigned?
	/// \remarks True for enums backed by unsigned integers.
	template <typename T, typename... U>
	inline constexpr bool all_unsigned = (is_unsigned<T> && ... && is_unsigned<U>);

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
	template <typename T, typename... U>
	inline constexpr bool any_signed = (is_signed<T> || ... || is_signed<U>);

	/// \brief Are all of the named types signed or reference-to-signed?
	/// \remarks True for enums backed by signed integers.
	template <typename T, typename... U>
	inline constexpr bool all_signed = (is_signed<T> && ... && is_signed<U>);

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
	template <typename T, typename... U>
	inline constexpr bool any_integral = (is_integral<T> || ... || is_integral<U>);

	/// \brief Are all of the named types integral or reference-to-integral?
	/// \remarks True for enums.
	/// \remarks True for #int128_t and #uint128_t (where supported).
	template <typename T, typename... U>
	inline constexpr bool all_integral = (is_integral<T> && ... && is_integral<U>);

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
	template <typename T, typename... U>
	inline constexpr bool any_integer = (is_integer<T> || ... || is_integer<U>);

	/// \brief Are all of the named types arithmetic integers, or reference to them?
	/// \remarks True for #int128_t and #uint128_t (where supported).
	/// \remarks False for enums, booleans, and character types.
	template <typename T, typename... U>
	inline constexpr bool all_integer = (is_integer<T> && ... && is_integer<U>);

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
	template <typename T, typename... U>
	inline constexpr bool any_floating_point = (is_floating_point<T> || ... || is_floating_point<U>);

	/// \brief Are all of the named types floating-point or reference-to-floating-point?
	/// \remarks True for muu::half.
	/// \remarks True for _Float16 and #float128_t (where supported).
	template <typename T, typename... U>
	inline constexpr bool all_floating_point = (is_floating_point<T> && ... && is_floating_point<U>);

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
	template <typename T, typename... U>
	inline constexpr bool any_arithmetic = (is_arithmetic<T> || ... || is_arithmetic<U>);

	/// \brief Are all of the named types arithmetic or reference-to-arithmetic?
	/// \remarks True for muu::half.
	/// \remarks True for #int128_t, #uint128_t, _Float16 and #float128_t (where supported).
	template <typename T, typename... U>
	inline constexpr bool all_arithmetic = (is_arithmetic<T> && ... && is_arithmetic<U>);

	/// \brief Is a type const or reference-to-const?
	template <typename T>
	inline constexpr bool is_const = std::is_const_v<std::remove_reference_t<T>>;

	/// \brief Are any of the named types const or reference-to-const?
	template <typename T, typename... U>
	inline constexpr bool any_const = (is_const<T> || ... || is_const<U>);

	/// \brief Are all of the named types const or reference-to-const?
	template <typename T, typename... U>
	inline constexpr bool all_const = (is_const<T> && ... && is_const<U>);

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
	template <typename T, typename... U>
	inline constexpr bool any_volatile = (is_volatile<T> || ... || is_volatile<U>);

	/// \brief Are all of the named types volatile or reference-to-volatile?
	template <typename T, typename... U>
	inline constexpr bool all_volatile = (is_volatile<T> && ... && is_volatile<U>);

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
	template <typename T, typename... U>
	inline constexpr bool any_cv = (is_cv<T> || ... || is_cv<U>);

	/// \brief Are all of the named types const and/or volatile-qualified, or references to const and/or volatile-qualified?
	template <typename T, typename... U>
	inline constexpr bool all_cv = (is_cv<T> && ... && is_cv<U>);

	/// \brief Is a type const, volatile, or a reference?
	template <typename T>
	inline constexpr bool is_cvref = std::is_const_v<T> || std::is_volatile_v<T> || std::is_reference_v<T>;

	/// \brief Are any of the named types const, volatile, or a reference?
	template <typename T, typename... U>
	inline constexpr bool any_cvref = (is_cvref<T> || ... || is_cvref<U>);

	/// \brief Are all of the named types const, volatile, or a reference?
	template <typename T, typename... U>
	inline constexpr bool all_cvref = (is_cvref<T> && ... && is_cvref<U>);

	/// \brief Removes any `noexcept` specifier from a functional type.
	template <typename T>
	using remove_noexcept = typename impl::remove_noexcept_<T>::type;

	/// \brief Does Child inherit from Parent?
	/// \remarks This does not return true when the objects are the same type, unlike std::is_base_of.
	template <typename Parent, typename Child>
	inline constexpr bool inherits_from =
		std::is_base_of_v<remove_cvref<Parent>,
						  remove_cvref<Child>> && !std::is_same_v<remove_cvref<Parent>, remove_cvref<Child>>;

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
	template <typename T, typename... U>
	inline constexpr bool any_unbounded_array = (is_unbounded_array<T> || ... || is_unbounded_array<U>);

	/// \brief Are all of the named types an unbounded array (`T[]`), or a reference to one?
	template <typename T, typename... U>
	inline constexpr bool all_unbounded_array = (is_unbounded_array<T> && ... && is_unbounded_array<U>);

	/// \brief Is a type a bounded array (`T[N]`), or reference to one?
	/// \remark This is similar to C++20's std::is_bounded_array.
	template <typename T>
	inline constexpr bool is_bounded_array = impl::is_bounded_array_<std::remove_reference_t<T>>::value;

	/// \brief Are any of the named types a bounded array (`T[N]`), or a reference to one?
	template <typename T, typename... U>
	inline constexpr bool any_bounded_array = (is_bounded_array<T> || ... || is_bounded_array<U>);

	/// \brief Are all of the named types a bounded array (`T[N]`), or a reference to one?
	template <typename T, typename... U>
	inline constexpr bool all_bounded_array = (is_bounded_array<T> && ... && is_bounded_array<U>);

	/// \brief Is a type an array, or reference to one?
	template <typename T>
	inline constexpr bool is_array = is_unbounded_array<T> || is_bounded_array<T>;

	/// \brief Are any of the named types arrays, or references to arrays?
	template <typename T, typename... U>
	inline constexpr bool any_array = (is_array<T> || ... || is_array<U>);

	/// \brief Are all of the named types arrays, or references to arrays?
	template <typename T, typename... U>
	inline constexpr bool all_array = (is_array<T> && ... && is_array<U>);

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
	///
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

		template <typename T>
		using has_pre_increment_operator_ = decltype(++std::declval<T>());
		template <typename T>
		using has_pre_decrement_operator_ = decltype(--std::declval<T>());
		template <typename T>
		using has_post_increment_operator_ = decltype(std::declval<T>()++);
		template <typename T>
		using has_post_decrement_operator_ = decltype(std::declval<T>()--);

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
		template <typename T>
		inline constexpr bool is_vector_ = false;
		template <typename S, size_t D>
		inline constexpr bool is_vector_<::muu::vector<S, D>> = true;

		template <typename T>
		inline constexpr bool is_quaternion_ = false;
		template <typename S>
		inline constexpr bool is_quaternion_<::muu::quaternion<S>> = true;

		template <typename T,
				  size_t MinR = 0,
				  size_t MinC = 0,
				  size_t MaxR = static_cast<size_t>(-1),
				  size_t MaxC = static_cast<size_t>(-1)>
		inline constexpr bool is_matrix_ = false;
		template <typename S, size_t MinR, size_t MinC, size_t MaxR, size_t MaxC, size_t R, size_t C>
		inline constexpr bool is_matrix_<::muu::matrix<S, R, C>, MinR, MinC, MaxR, MaxC> = (R >= MinR && C >= MinC
																							&& R <= MaxR && C <= MaxC);

		// promotes ints to doubles, keeps floats as-is, as per the behaviour of std::sqrt, std::lerp, etc.
		template <typename... T>
		using std_math_common_type = highest_ranked<std::conditional_t<is_integer<T>, double, T>...>;

		struct any_type
		{
			template <typename T>
			/*implicit*/ constexpr operator T() const noexcept;
		};

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
			// - have between 1 and 4 members (if any of the members are arrays they count as N members where N is the
			// extent of the array)
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
					return std::bool_constant<
						is_valid_hva_<T, decltype(a), decltype(b), decltype(c), decltype(d)>::value>{};
				}
				else if constexpr (is_detected<is_aggregate_3_args_, T>) // three
				{
					auto&& [a, b, c] = static_cast<T&&>(obj);
					return std::bool_constant<is_valid_hva_<T, decltype(a), decltype(b), decltype(c)>::value>{};
				}
				else if constexpr (is_detected<is_aggregate_2_args_, T>) // two
				{
					auto&& [a, b] = static_cast<T&&>(obj);
					return std::bool_constant<is_valid_hva_<T, decltype(a), decltype(b)>::value>{};
				}
				else if constexpr (is_detected<is_aggregate_1_arg_, T>) // one
				{
					auto&& [a] = static_cast<T&&>(obj);
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

		template <typename T, bool Vectorcall = false>
		struct readonly_param_base_
		{
			using type = std::conditional_t<
			is_floating_point<T>
			|| is_integral<T>
			|| std::is_scalar_v<T>
			|| (Vectorcall
				&& MUU_HAS_VECTORCALL
				&& (is_vectorcall_simd_intrinsic<T> || (!MUU_ARCH_X86 && is_hva<T>))
					// HVAs cause a bunch of codegen bugs when passed by value with vectorcall on x86
			)
			|| ((std::is_class_v<T> || std::is_union_v<T>)
				&& (std::is_trivially_copyable_v<T> || std::is_nothrow_copy_constructible_v<T>)
				&& std::is_nothrow_destructible_v<T>
				&& sizeof(T) <= sizeof(void*)),
			T,
			std::add_lvalue_reference_t<std::add_const_t<T>>
		>;
		};
		template <typename T>
		struct readonly_param_base_<T&>
		{
			using type = T&;
		};
		template <typename T>
		struct readonly_param_base_<T&&>
		{
			using type = T&&;
		};
		template <>
		struct readonly_param_base_<half>
		{
			using type = half;
		};
		template <typename T>
		struct readonly_param_base_<T&, true> // vectorcall
		{
			using type = T&;
		};
		template <typename T>
		struct readonly_param_base_<T&&, true> // vectorcall
		{
			using type = T&&;
		};
		template <>
		struct readonly_param_base_<half, true> // vectorcall
		{
			using type = half;
		};

		template <typename T>
		struct readonly_param_ : readonly_param_base_<T, false>
		{};
		template <typename T>
		struct vectorcall_param_ : readonly_param_base_<T, true> // vectorcall
		{};

		template <typename T>
		using readonly_param = typename readonly_param_<T>::type;
		template <typename T>
		using vectorcall_param = typename vectorcall_param_<T>::type;

		template <typename... T>
		inline constexpr bool pass_readonly_by_reference = sizeof...(T) == 0
														|| (std::is_reference_v<readonly_param<T>> || ...);

		template <typename... T>
		inline constexpr bool pass_readonly_by_value = !pass_readonly_by_reference<T...>;

		template <typename... T>
		inline constexpr bool pass_vectorcall_by_reference = sizeof...(T) == 0
														  || (std::is_reference_v<vectorcall_param<T>> || ...);

		template <typename... T>
		inline constexpr bool pass_vectorcall_by_value = !pass_vectorcall_by_reference<T...>;

		template <typename T, bool = has_unary_plus_operator<T>>
		inline constexpr bool decays_to_function_pointer_by_unary_plus_ = false;

		template <typename T>
		inline constexpr bool decays_to_function_pointer_by_unary_plus_<T, true> =
			std::is_pointer_v<std::remove_reference_t<decltype(+std::declval<T>())>> //
				&& std::is_function_v<std::remove_pointer_t<std::remove_reference_t<decltype(+std::declval<T>())>>>;
	}
	/// \endcond

	/// \brief  A common epsilon type when comparing floating-point types named by T.
	template <typename... T>
	using epsilon_type = impl::std_math_common_type<remove_cvref<T>...>;

	/// \brief  The default floating-point epsilon value used when comparing floating-point types named by T.
	template <typename... T>
	inline constexpr epsilon_type<T...> default_epsilon = constants<epsilon_type<T...>>::default_epsilon;

	/// \brief A tag type for encoding/parameterizing a single index.
	template <size_t N>
	using index_tag = std::integral_constant<size_t, N>;

	/// \brief Evaluates to true if an instance of `T` decays to a free-function pointer by explicit unary plus.
	template <typename T>
	inline constexpr bool decays_to_function_pointer_by_unary_plus = impl::decays_to_function_pointer_by_unary_plus_<T>;

	/** @} */ // meta

	/// \cond
	// deprecations
#if !MUU_INTELLISENSE
	template <typename T, typename CopyFrom>
	using match_const = copy_const<T, CopyFrom>;
	template <typename T, typename CopyFrom>
	using match_volatile = copy_volatile<T, CopyFrom>;
	template <typename T, typename CopyFrom>
	using match_cv = copy_cv<T, CopyFrom>;
	template <typename T, typename... U>
	inline constexpr bool is_same_as_any = any_same<T, U...>;
#endif
	/// \endcond
}

#include "header_end.h"
