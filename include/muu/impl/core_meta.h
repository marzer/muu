// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "../fwd.h"

MUU_DISABLE_WARNINGS;
#include <utility>
#include <type_traits>
#if MUU_HAS_VECTORCALL
	#include <intrin.h>
#endif
MUU_ENABLE_WARNINGS;

// replace the muu homebrew remove_reference_ with remove_reference_t after including <type_traits>
// because major compilers often memoize things in <type_traits> for improved throughput
#undef MUU_MOVE
#define MUU_MOVE(...) static_cast<std::remove_reference_t<decltype(__VA_ARGS__)>&&>(__VA_ARGS__)

#include "header_start.h"
MUU_PRAGMA_MSVC(warning(disable : 4296)) // condition always true/false

namespace muu
{
	/// \addtogroup		meta
	/// @{

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
			using type = typename make_signed_<T>::type&;
		};
		template <typename T>
		struct make_signed_<T&&>
		{
			using type = typename make_signed_<T>::type&&;
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
			using type = typename make_unsigned_<T>::type&;
		};
		template <typename T>
		struct make_unsigned_<T&&>
		{
			using type = typename make_unsigned_<T>::type&&;
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

		template <typename T>
		struct type_identity_
		{
			using type = T;
		};

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

		template <template <typename...> typename Trait, typename Enabler, typename... Args>
		struct is_detected_impl : std::false_type
		{};
		template <template <typename...> typename Trait, typename... Args>
		struct is_detected_impl<Trait, std::void_t<Trait<Args...>>, Args...> : std::true_type
		{};
		template <template <typename...> typename Trait, typename... Args>
		inline constexpr auto is_detected_ = is_detected_impl<Trait, void, Args...>::value;

		template <typename T>
		using has_arrow_operator_ = decltype(std::declval<T>().operator->());
		template <typename T>
		using has_unary_plus_operator_ = decltype(+std::declval<T>());

		template <typename T>
		using has_tuple_size_ = decltype(std::tuple_size<std::remove_cv_t<std::remove_reference_t<T>>>::value);
		template <typename T>
		using has_tuple_element_ = std::tuple_element_t<0, std::remove_cv_t<std::remove_reference_t<T>>>;
		template <typename T>
		using has_tuple_get_member_ = decltype(std::declval<T>().template get<0>());

		template <typename T, bool = is_detected_<has_tuple_size_, T>>
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
		MUU_ATTR(pure)
		constexpr decltype(auto) get_from_tuple_like(T&& tuple_like) noexcept
		{
			if constexpr (is_detected_<has_tuple_get_member_, T&&>)
			{
				return static_cast<T&&>(tuple_like).template get<I>();
			}
			else // adl
			{
				using std::get;
				return get<I>(static_cast<T&&>(tuple_like));
			}
		}

#ifdef __cpp_lib_is_nothrow_convertible

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
	}
	/// \endcond

	/// \brief	Removes the topmost const, volatile and reference qualifiers from a type.
	/// \remark This is equivalent to C++20's std::remove_cvref_t.
	template <typename T>
	using remove_cvref = std::remove_cv_t<std::remove_reference_t<T>>;

	/// \brief	Removes the outer enum wrapper from a type, converting it to the underlying integer equivalent.
	/// \remark This is similar to std::underlying_type_t but preserves cv qualifiers and ref categories, as well as
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
	/// \remark This equivalent to `(std::is_same_v<T, U1> || std::is_same_v<T, U2> || ...)`.
	template <typename T, typename... U>
	inline constexpr bool is_same_as_any = (false || ... || std::is_same_v<T, U>);

	/// \brief	True if all the types named by T and U are exactly the same.
	/// \remark This equivalent to `(std::is_same_v<T, U1> && std::is_same_v<T, U2> && ...)`.
	template <typename T, typename... U>
	inline constexpr bool all_same = (true && ... && std::is_same_v<T, U>);

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
	inline constexpr bool any_enum = is_enum<T> || (false || ... || is_enum<U>);

	/// \brief Are all of the named types enums or reference-to-enum?
	template <typename T, typename... U>
	inline constexpr bool all_enum = is_enum<T> && (true && ... && is_enum<U>);

	/// \brief Is a type a C++11 scoped enum class, or reference to one?
	template <typename T>
	inline constexpr bool is_scoped_enum =
		is_enum<T> && !std::is_convertible_v<remove_cvref<T>, remove_enum<remove_cvref<T>>>;

	/// \brief Is a type a pre-C++11 unscoped enum, or reference to one?
	template <typename T>
	inline constexpr bool is_legacy_enum =
		is_enum<T>&& std::is_convertible_v<remove_cvref<T>, remove_enum<remove_cvref<T>>>;

	/// \brief Is a type unsigned or reference-to-unsigned?
	/// \remarks Returns true for enums backed by unsigned integers.
	/// \remarks Returns true for #uint128_t (where supported).
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
	/// \remarks Returns true for #int128_t, __fp16, _Float16 and #float128_t (where supported).
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
			, _Float16
#endif
#if MUU_HAS_FP16
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
	/// \remarks Returns true for #int128_t and #uint128_t (where supported).
	template <typename T>
	inline constexpr bool is_integral = std::is_integral_v<remove_enum<remove_cvref<T>>>
#if MUU_HAS_INT128
									 || is_same_as_any<remove_enum<remove_cvref<T>>, int128_t, uint128_t>
#endif
		;

	/// \brief Are any of the named types integral or reference-to-integral?
	/// \remarks Returns true for enums.
	/// \remarks Returns true for #int128_t and #uint128_t (where supported).
	template <typename T, typename... U>
	inline constexpr bool any_integral = is_integral<T> || (false || ... || is_integral<U>);

	/// \brief Are all of the named types integral or reference-to-integral?
	/// \remarks Returns true for enums.
	/// \remarks Returns true for #int128_t and #uint128_t (where supported).
	template <typename T, typename... U>
	inline constexpr bool all_integral = is_integral<T> && (true && ... && is_integral<U>);

	/// \brief Is a type a floating-point or reference-to-floating-point?
	/// \remarks Returns true for muu::half.
	/// \remarks Returns true for __fp16, _Float16 and #float128_t (where supported).
	template <typename T>
	inline constexpr bool is_floating_point = std::is_floating_point_v<std::remove_reference_t<T>>
		|| is_same_as_any<remove_cvref<T>,
			half
#if MUU_HAS_FLOAT128
			, float128_t
#endif
#if MUU_HAS_FLOAT16
			, _Float16
#endif
#if MUU_HAS_FP16
			, __fp16
#endif
		>
	;

	/// \brief Are any of the named types floating-point or reference-to-floating-point?
	/// \remarks Returns true for muu::half.
	/// \remarks Returns true for _Float16 and #float128_t (where supported).
	template <typename T, typename... U>
	inline constexpr bool any_floating_point = is_floating_point<T> || (false || ... || is_floating_point<U>);

	/// \brief Are all of the named types floating-point or reference-to-floating-point?
	/// \remarks Returns true for muu::half.
	/// \remarks Returns true for _Float16 and #float128_t (where supported).
	template <typename T, typename... U>
	inline constexpr bool all_floating_point = is_floating_point<T> && (true && ... && is_floating_point<U>);

	/// \brief Is a type one of the standard c++ arithmetic types, or a reference to one?
	template <typename T>
	inline constexpr bool is_standard_arithmetic = std::is_arithmetic_v<std::remove_reference_t<T>>;

	/// \brief Is a type a nonstandard 'extended' arithmetic type, or a reference to one?
	/// \remarks Returns true for muu::half.
	/// \remarks Returns true for #int128_t, #uint128_t, __fp16, _Float16 and #float128_t (where supported).
	template <typename T>
	inline constexpr bool is_extended_arithmetic = is_same_as_any<remove_cvref<T>,
																  half
#if MUU_HAS_INT128
																  ,
																  int128_t,
																  uint128_t
#endif
#if MUU_HAS_FLOAT128
																  ,
																  float128_t
#endif
#if MUU_HAS_FLOAT16
																  ,
																  _Float16
#endif
#if MUU_HAS_FP16
																  ,
																  __fp16
#endif
																  >;

	/// \brief Is a type arithmetic or reference-to-arithmetic?
	/// \remarks Returns true for muu::half.
	/// \remarks Returns true for #int128_t, #uint128_t, __fp16, _Float16 and #float128_t (where supported).
	template <typename T>
	inline constexpr bool is_arithmetic = is_standard_arithmetic<T> || is_extended_arithmetic<T>;

	/// \brief Are any of the named types arithmetic or reference-to-arithmetic?
	/// \remarks Returns true for muu::half.
	/// \remarks Returns true for #int128_t, #uint128_t, _Float16 and #float128_t (where supported).
	template <typename T, typename... U>
	inline constexpr bool any_arithmetic = is_arithmetic<T> || (false || ... || is_arithmetic<U>);

	/// \brief Are all of the named types arithmetic or reference-to-arithmetic?
	/// \remarks Returns true for muu::half.
	/// \remarks Returns true for #int128_t, #uint128_t, _Float16 and #float128_t (where supported).
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

	/// \brief Sets the unsigned-ness of a numeric type or reference according to a boolean.
	template <typename T, bool Unsigned>
	using set_unsigned = std::conditional_t<Unsigned, make_unsigned<T>, make_signed<T>>;

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

	/// \brief Is a type a Unicode 'code unit' type, or reference to one?
	template <typename T>
	inline constexpr bool is_code_unit = is_same_as_any<remove_cvref<T>,
														char,
														wchar_t,
														char16_t,
														char32_t
#if MUU_HAS_CHAR8
														,
														char8_t
#endif
														>;

	/// \brief Returns the rank of a pointer.
	/// \remark Answers "how many stars does it have?".
	template <typename T>
	inline constexpr size_t pointer_rank = impl::pointer_rank_<T>::value;

	/// \brief Strips every level of pointer from a type.
	template <typename T>
	using remove_all_pointers = typename impl::remove_all_pointers_<T>::type;

	/// \brief Detects if a type supports an interface.
	/// \see
	///		- [Detection Idiom](https://blog.tartanllama.xyz/detection-idiom/)
	///		- [std::experimental::is_detected](https://en.cppreference.com/w/cpp/experimental/is_detected)
	template <template <typename...> typename Trait, typename... Args>
	inline constexpr auto is_detected = impl::is_detected_<Trait, Args...>;

	/// \brief Returns true if the type has an arrow operator.
	template <typename T>
	inline constexpr bool has_arrow_operator =
		is_detected<
			impl::has_arrow_operator_,
			T> || (std::is_pointer_v<T> && (std::is_class_v<std::remove_pointer_t<T>> || std::is_union_v<std::remove_pointer_t<T>>));

	/// \brief Returns true if the type has a unary plus operator.
	template <typename T>
	inline constexpr bool has_unary_plus_operator = is_detected<impl::has_unary_plus_operator_, T>;

	/// \brief Returns true if the type implements std::tuple_size and std::tuple_element.
	template <typename T>
	inline constexpr bool is_tuple_like =
		is_detected<impl::has_tuple_size_, T>&& is_detected<impl::has_tuple_element_, T>
		//&& is_detected<impl::has_tuple_get_member_, T>
		;

	/// \brief Equivalent to std::tuple_size_v, but safe to use in SFINAE contexts.
	/// \remark Returns 0 for types that do not implement std::tuple_size.
	template <typename T>
	inline constexpr size_t tuple_size = impl::tuple_size_<T>::value;

	/// \cond
	namespace impl
	{
		template <typename T>
		inline constexpr bool is_small_float_ = is_floating_point<T> && sizeof(T) < sizeof(float)
											 && is_extended_arithmetic<T>;
		template <typename T>
		inline constexpr bool is_large_float_ = is_floating_point<T> && sizeof(T) >= sizeof(long double)
											 && is_extended_arithmetic<T>;

		template <typename T>
		inline constexpr bool is_vector_ = false;
		template <typename S, size_t D>
		inline constexpr bool is_vector_<::muu::vector<S, D>> = true;

		template <typename T>
		inline constexpr bool is_quaternion_ = false;
		template <typename S>
		inline constexpr bool is_quaternion_<::muu::quaternion<S>> = true;

		template <typename T>
		inline constexpr bool is_matrix_ = false;
		template <typename S, size_t R, size_t C>
		inline constexpr bool is_matrix_<::muu::matrix<S, R, C>> = true;

		template <typename T>
		using promote_if_small_float = std::conditional_t<is_small_float_<T>, float, T>;
		template <typename T>
		using demote_if_large_float = std::conditional_t<is_large_float_<T>, long double, T>;
		template <typename T>
		using clamp_to_standard_float = demote_if_large_float<promote_if_small_float<T>>;

		// promotes ints to doubles, keeps floats as-is, as per the behaviour of std::sqrt, std::lerp, etc.
		template <typename... T>
		using std_math_common_type = highest_ranked<std::conditional_t<is_integral<T> && !is_enum<T>, double, T>...>;

		struct any_type
		{
			template <typename T>
			constexpr operator T() const noexcept; // non-explicit
		};

#if MUU_HAS_VECTORCALL

		template <typename T>
		inline constexpr bool is_vectorcall_simd_intrinsic = is_same_as_any<remove_cvref<T>,
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
			is_same_as_any<T, float, double, long double> || is_vectorcall_simd_intrinsic<T>;

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
	}
	/// \endcond

	/// \brief  A common epsilon type when comparing floating-point types named by T.
	template <typename... T>
	using epsilon_type = impl::std_math_common_type<remove_cvref<T>...>;

	/// \brief  The default floating-point epsilon value used when comparing floating-point types named by T.
	template <typename... T>
	inline constexpr epsilon_type<T...> default_epsilon = constants<epsilon_type<T...>>::default_epsilon;

	/// \cond
	template <typename... T>
	struct type_list;

	namespace impl
	{
		// Q: "why are there so many specializations?"
		// A: compilers, if left to their own devices, will happily exhaust all the available system memory
		//    when instantiating recursive compiler nonsense. These are all optimizations to speed up compilation and
		//    stop the compiler from destroying the universe.

		inline constexpr size_t type_list_spec_max = 63;

		enum class type_list_selector_spec : int
		{
			first,
			skip_chunk,
			by_index
		};

		// clang-format off

		// selector
		template <typename List, size_t N, type_list_selector_spec Specialization = (
				N == 0 ? type_list_selector_spec::first : (
				N > type_list_spec_max ? type_list_selector_spec::skip_chunk : (
				type_list_selector_spec::by_index
			)))
		>
		struct type_list_selector_;

		// clang-format on

		// selector - first element
		template <typename T0, typename... T, size_t N>
		struct type_list_selector_<type_list<T0, T...>, N, type_list_selector_spec::first>
		{
			using type = T0;
		};

		// selector - skip whole chunks (multiples of type_list_spec_max)
		template <typename List, size_t N>
		struct type_list_selector_<List, N, type_list_selector_spec::skip_chunk>
		{
			// invokes the skip_chunk specialization of the slicer then selects by (now offset) index
			using type = typename type_list_selector_<typename List::template slice<type_list_spec_max>,
													  N - type_list_spec_max>::type;
		};

		// selector - arbitrary low-index elements
		// clang-format off

		#define MAKE_INDEXED_TPARAM(N)	MUU_COMMA typename MUU_CONCAT(T, N)
		#define MAKE_INDEXED_TARG(N)	MUU_COMMA MUU_CONCAT(T, N)
		#define INDICES_0_TO_15			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
		#define INDICES_0_TO_31			INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
		#define INDICES_0_TO_47			INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47
		#define INDICES_0_TO_63			INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63

		#define MAKE_SELECTOR_1(N, N0, ...)                                                                            \
			template <typename T##N0 MUU_FOR_EACH(MAKE_INDEXED_TPARAM, __VA_ARGS__), typename... T>                    \
			struct type_list_selector_<type_list<T##N0 MUU_FOR_EACH(MAKE_INDEXED_TARG, __VA_ARGS__), T...>,            \
									   N,                                                                              \
									   type_list_selector_spec::by_index>                                              \
			{                                                                                                          \
				using type = T##N;                                                                                     \
			}
		#define MAKE_SELECTOR(...) MAKE_SELECTOR_1(__VA_ARGS__)

		template <typename T0, typename T1, typename... T>
		struct type_list_selector_<type_list<T0, T1, T...>, 1, type_list_selector_spec::by_index>
		{
			using type = T1;
		};

		MAKE_SELECTOR(2,  0, 1, 2);
		MAKE_SELECTOR(3,  0, 1, 2, 3);
		MAKE_SELECTOR(4,  0, 1, 2, 3, 4);
		MAKE_SELECTOR(5,  0, 1, 2, 3, 4, 5);
		MAKE_SELECTOR(6,  0, 1, 2, 3, 4, 5, 6);
		MAKE_SELECTOR(7,  0, 1, 2, 3, 4, 5, 6, 7);
		MAKE_SELECTOR(8,  0, 1, 2, 3, 4, 5, 6, 7, 8);
		MAKE_SELECTOR(9,  0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
		MAKE_SELECTOR(10, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
		MAKE_SELECTOR(11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
		MAKE_SELECTOR(12, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
		MAKE_SELECTOR(13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13);
		MAKE_SELECTOR(14, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
		MAKE_SELECTOR(15, INDICES_0_TO_15);
		MAKE_SELECTOR(16, INDICES_0_TO_15, 16);
		MAKE_SELECTOR(17, INDICES_0_TO_15, 16, 17);
		MAKE_SELECTOR(18, INDICES_0_TO_15, 16, 17, 18);
		MAKE_SELECTOR(19, INDICES_0_TO_15, 16, 17, 18, 19);
		MAKE_SELECTOR(20, INDICES_0_TO_15, 16, 17, 18, 19, 20);
		MAKE_SELECTOR(21, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21);
		MAKE_SELECTOR(22, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22);
		MAKE_SELECTOR(23, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23);
		MAKE_SELECTOR(24, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24);
		MAKE_SELECTOR(25, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25);
		MAKE_SELECTOR(26, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26);
		MAKE_SELECTOR(27, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27);
		MAKE_SELECTOR(28, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28);
		MAKE_SELECTOR(29, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29);
		MAKE_SELECTOR(30, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30);
		MAKE_SELECTOR(31, INDICES_0_TO_31);
		MAKE_SELECTOR(32, INDICES_0_TO_31, 32);
		MAKE_SELECTOR(33, INDICES_0_TO_31, 32, 33);
		MAKE_SELECTOR(34, INDICES_0_TO_31, 32, 33, 34);
		MAKE_SELECTOR(35, INDICES_0_TO_31, 32, 33, 34, 35);
		MAKE_SELECTOR(36, INDICES_0_TO_31, 32, 33, 34, 35, 36);
		MAKE_SELECTOR(37, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37);
		MAKE_SELECTOR(38, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38);
		MAKE_SELECTOR(39, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39);
		MAKE_SELECTOR(40, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40);
		MAKE_SELECTOR(41, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41);
		MAKE_SELECTOR(42, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42);
		MAKE_SELECTOR(43, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43);
		MAKE_SELECTOR(44, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44);
		MAKE_SELECTOR(45, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45);
		MAKE_SELECTOR(46, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46);
		MAKE_SELECTOR(47, INDICES_0_TO_47);
		MAKE_SELECTOR(48, INDICES_0_TO_47, 48);
		MAKE_SELECTOR(49, INDICES_0_TO_47, 48, 49);
		MAKE_SELECTOR(50, INDICES_0_TO_47, 48, 49, 50);
		MAKE_SELECTOR(51, INDICES_0_TO_47, 48, 49, 50, 51);
		MAKE_SELECTOR(52, INDICES_0_TO_47, 48, 49, 50, 51, 52);
		MAKE_SELECTOR(53, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53);
		MAKE_SELECTOR(54, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54);
		MAKE_SELECTOR(55, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55);
		MAKE_SELECTOR(56, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56);
		MAKE_SELECTOR(57, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57);
		MAKE_SELECTOR(58, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58);
		MAKE_SELECTOR(59, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59);
		MAKE_SELECTOR(60, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60);
		MAKE_SELECTOR(61, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61);
		MAKE_SELECTOR(62, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62);
		MAKE_SELECTOR(63, INDICES_0_TO_63);

		#undef MAKE_SELECTOR_1
		#undef MAKE_SELECTOR
		// clang-format on

		// selector - helper typedef
		template <typename List, size_t N>
		using type_list_select_ = typename type_list_selector_<List, N>::type;

		enum class type_list_slicer_spec : int
		{
			empty,
			first,
			all,
			single_by_index,
			prefix,
			skip_first_N,
			skip_chunk,
			big_range,
		};

		// clang-format off

		// slicer
		template <typename List, size_t Start, size_t Length, type_list_slicer_spec Specialization = (
				List::length == 0 || Length == 0 ? type_list_slicer_spec::empty : (
				Start == 0 && Length == 1 ? type_list_slicer_spec::first : (
				Start == 0 && List::length == Length ? type_list_slicer_spec::all : (
				Start > type_list_spec_max ? type_list_slicer_spec::skip_chunk : (
				Length == 1 ? type_list_slicer_spec::single_by_index : (
				Start == 0 && Length <= type_list_spec_max+1 ? type_list_slicer_spec::prefix : (
				Start > 0 ? type_list_slicer_spec::skip_first_N : (
				type_list_slicer_spec::big_range
			)))))))
		)>
		struct type_list_slicer_;

		// clang-format on

		// slicer - empty range
		template <typename List, size_t Start, size_t Length>
		struct type_list_slicer_<List, Start, Length, type_list_slicer_spec::empty>
		{
			using type = type_list<>;
		};

		// slicer - first element
		template <typename T0, typename... T, size_t Start, size_t Length>
		struct type_list_slicer_<type_list<T0, T...>, Start, Length, type_list_slicer_spec::first>
		{
			using type = type_list<T0>;
		};

		// slicer - all (range is entire list)
		template <typename List, size_t Start, size_t Length>
		struct type_list_slicer_<List, Start, Length, type_list_slicer_spec::all>
		{
			using type = List;
		};

		// slicer - skip whole chunks (multiples of type_list_spec_max)
		template <typename List, size_t Start, size_t Length>
		struct type_list_slicer_<List, Start, Length, type_list_slicer_spec::skip_chunk>
		{
			// invokes the skip_first_N specialization repeatedly until Start is <= type_list_spec_max
			using type = typename type_list_slicer_<typename List::template slice<type_list_spec_max>,
													Start - type_list_spec_max,
													Length>::type;
		};

		// slicer - arbitrary low-index elements
		// clang-format off

		#define MAKE_SINGLE_ELEMENT_SLICER_1(N, N0, ...)                                                               \
			template <typename T##N0 MUU_FOR_EACH(MAKE_INDEXED_TPARAM, __VA_ARGS__), typename... T>                    \
			struct type_list_slicer_<type_list<T##N0 MUU_FOR_EACH(MAKE_INDEXED_TARG, __VA_ARGS__), T...>,              \
									 N,                                                                                \
									 1,                                                                                \
									 type_list_slicer_spec::single_by_index>                                           \
			{                                                                                                          \
				using type = type_list<T##N>;                                                                          \
			}
		#define MAKE_SINGLE_ELEMENT_SLICER(...) MAKE_SINGLE_ELEMENT_SLICER_1(__VA_ARGS__)

		template <typename T0, typename T1, typename... T>
		struct type_list_slicer_<type_list<T0, T1, T...>, 1, 1, type_list_slicer_spec::single_by_index>
		{
			using type = type_list<T1>;
		};

		MAKE_SINGLE_ELEMENT_SLICER(2,  0, 1, 2);
		MAKE_SINGLE_ELEMENT_SLICER(3,  0, 1, 2, 3);
		MAKE_SINGLE_ELEMENT_SLICER(4,  0, 1, 2, 3, 4);
		MAKE_SINGLE_ELEMENT_SLICER(5,  0, 1, 2, 3, 4, 5);
		MAKE_SINGLE_ELEMENT_SLICER(6,  0, 1, 2, 3, 4, 5, 6);
		MAKE_SINGLE_ELEMENT_SLICER(7,  0, 1, 2, 3, 4, 5, 6, 7);
		MAKE_SINGLE_ELEMENT_SLICER(8,  0, 1, 2, 3, 4, 5, 6, 7, 8);
		MAKE_SINGLE_ELEMENT_SLICER(9,  0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
		MAKE_SINGLE_ELEMENT_SLICER(10, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
		MAKE_SINGLE_ELEMENT_SLICER(11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
		MAKE_SINGLE_ELEMENT_SLICER(12, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
		MAKE_SINGLE_ELEMENT_SLICER(13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13);
		MAKE_SINGLE_ELEMENT_SLICER(14, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
		MAKE_SINGLE_ELEMENT_SLICER(15, INDICES_0_TO_15);
		MAKE_SINGLE_ELEMENT_SLICER(16, INDICES_0_TO_15, 16);
		MAKE_SINGLE_ELEMENT_SLICER(17, INDICES_0_TO_15, 16, 17);
		MAKE_SINGLE_ELEMENT_SLICER(18, INDICES_0_TO_15, 16, 17, 18);
		MAKE_SINGLE_ELEMENT_SLICER(19, INDICES_0_TO_15, 16, 17, 18, 19);
		MAKE_SINGLE_ELEMENT_SLICER(20, INDICES_0_TO_15, 16, 17, 18, 19, 20);
		MAKE_SINGLE_ELEMENT_SLICER(21, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21);
		MAKE_SINGLE_ELEMENT_SLICER(22, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22);
		MAKE_SINGLE_ELEMENT_SLICER(23, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23);
		MAKE_SINGLE_ELEMENT_SLICER(24, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24);
		MAKE_SINGLE_ELEMENT_SLICER(25, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25);
		MAKE_SINGLE_ELEMENT_SLICER(26, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26);
		MAKE_SINGLE_ELEMENT_SLICER(27, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27);
		MAKE_SINGLE_ELEMENT_SLICER(28, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28);
		MAKE_SINGLE_ELEMENT_SLICER(29, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29);
		MAKE_SINGLE_ELEMENT_SLICER(30, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30);
		MAKE_SINGLE_ELEMENT_SLICER(31, INDICES_0_TO_31);
		MAKE_SINGLE_ELEMENT_SLICER(32, INDICES_0_TO_31, 32);
		MAKE_SINGLE_ELEMENT_SLICER(33, INDICES_0_TO_31, 32, 33);
		MAKE_SINGLE_ELEMENT_SLICER(34, INDICES_0_TO_31, 32, 33, 34);
		MAKE_SINGLE_ELEMENT_SLICER(35, INDICES_0_TO_31, 32, 33, 34, 35);
		MAKE_SINGLE_ELEMENT_SLICER(36, INDICES_0_TO_31, 32, 33, 34, 35, 36);
		MAKE_SINGLE_ELEMENT_SLICER(37, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37);
		MAKE_SINGLE_ELEMENT_SLICER(38, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38);
		MAKE_SINGLE_ELEMENT_SLICER(39, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39);
		MAKE_SINGLE_ELEMENT_SLICER(40, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40);
		MAKE_SINGLE_ELEMENT_SLICER(41, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41);
		MAKE_SINGLE_ELEMENT_SLICER(42, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42);
		MAKE_SINGLE_ELEMENT_SLICER(43, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43);
		MAKE_SINGLE_ELEMENT_SLICER(44, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44);
		MAKE_SINGLE_ELEMENT_SLICER(45, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45);
		MAKE_SINGLE_ELEMENT_SLICER(46, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46);
		MAKE_SINGLE_ELEMENT_SLICER(47, INDICES_0_TO_47);
		MAKE_SINGLE_ELEMENT_SLICER(48, INDICES_0_TO_47, 48);
		MAKE_SINGLE_ELEMENT_SLICER(49, INDICES_0_TO_47, 48, 49);
		MAKE_SINGLE_ELEMENT_SLICER(50, INDICES_0_TO_47, 48, 49, 50);
		MAKE_SINGLE_ELEMENT_SLICER(51, INDICES_0_TO_47, 48, 49, 50, 51);
		MAKE_SINGLE_ELEMENT_SLICER(52, INDICES_0_TO_47, 48, 49, 50, 51, 52);
		MAKE_SINGLE_ELEMENT_SLICER(53, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53);
		MAKE_SINGLE_ELEMENT_SLICER(54, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54);
		MAKE_SINGLE_ELEMENT_SLICER(55, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55);
		MAKE_SINGLE_ELEMENT_SLICER(56, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56);
		MAKE_SINGLE_ELEMENT_SLICER(57, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57);
		MAKE_SINGLE_ELEMENT_SLICER(58, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58);
		MAKE_SINGLE_ELEMENT_SLICER(59, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59);
		MAKE_SINGLE_ELEMENT_SLICER(60, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60);
		MAKE_SINGLE_ELEMENT_SLICER(61, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61);
		MAKE_SINGLE_ELEMENT_SLICER(62, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62);
		MAKE_SINGLE_ELEMENT_SLICER(63, INDICES_0_TO_63);

		#undef MAKE_SINGLE_ELEMENT_SLICER_1
		#undef MAKE_SINGLE_ELEMENT_SLICER

		// slicer - arbitrary prefixes
		#define MAKE_PREFIX_SLICER_1(N0, ...)                                                                          \
			template <typename T##N0 MUU_FOR_EACH(MAKE_INDEXED_TPARAM, __VA_ARGS__), typename... T>                    \
			struct type_list_slicer_<type_list<T##N0 MUU_FOR_EACH(MAKE_INDEXED_TARG, __VA_ARGS__), T...>,              \
									 0,                                                                                \
									 MUU_COUNT_VA_ARGS(__VA_ARGS__) + 1,                                               \
									 type_list_slicer_spec::prefix>                                                    \
			{                                                                                                          \
				using type = type_list<T0 MUU_FOR_EACH(MAKE_INDEXED_TARG, __VA_ARGS__)>;                               \
			}
		#define MAKE_PREFIX_SLICER(...) MAKE_PREFIX_SLICER_1(__VA_ARGS__)

		template <typename T0, typename T1, typename... T>
		struct type_list_slicer_<type_list<T0, T1, T...>,
								 0,
								 2,
								 type_list_slicer_spec::prefix>
		{
			using type = type_list<T0, T1>;
		};

		MAKE_PREFIX_SLICER(0, 1, 2);
		MAKE_PREFIX_SLICER(0, 1, 2, 3);
		MAKE_PREFIX_SLICER(0, 1, 2, 3, 4);
		MAKE_PREFIX_SLICER(0, 1, 2, 3, 4, 5);
		MAKE_PREFIX_SLICER(0, 1, 2, 3, 4, 5, 6);
		MAKE_PREFIX_SLICER(0, 1, 2, 3, 4, 5, 6, 7);
		MAKE_PREFIX_SLICER(0, 1, 2, 3, 4, 5, 6, 7, 8);
		MAKE_PREFIX_SLICER(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
		MAKE_PREFIX_SLICER(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
		MAKE_PREFIX_SLICER(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
		MAKE_PREFIX_SLICER(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
		MAKE_PREFIX_SLICER(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13);
		MAKE_PREFIX_SLICER(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
		MAKE_PREFIX_SLICER(INDICES_0_TO_15);
		MAKE_PREFIX_SLICER(INDICES_0_TO_15, 16);
		MAKE_PREFIX_SLICER(INDICES_0_TO_15, 16, 17);
		MAKE_PREFIX_SLICER(INDICES_0_TO_15, 16, 17, 18);
		MAKE_PREFIX_SLICER(INDICES_0_TO_15, 16, 17, 18, 19);
		MAKE_PREFIX_SLICER(INDICES_0_TO_15, 16, 17, 18, 19, 20);
		MAKE_PREFIX_SLICER(INDICES_0_TO_15, 16, 17, 18, 19, 20, 21);
		MAKE_PREFIX_SLICER(INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22);
		MAKE_PREFIX_SLICER(INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23);
		MAKE_PREFIX_SLICER(INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24);
		MAKE_PREFIX_SLICER(INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25);
		MAKE_PREFIX_SLICER(INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26);
		MAKE_PREFIX_SLICER(INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27);
		MAKE_PREFIX_SLICER(INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28);
		MAKE_PREFIX_SLICER(INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29);
		MAKE_PREFIX_SLICER(INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30);
		MAKE_PREFIX_SLICER(INDICES_0_TO_31);
		MAKE_PREFIX_SLICER(INDICES_0_TO_31, 32);
		MAKE_PREFIX_SLICER(INDICES_0_TO_31, 32, 33);
		MAKE_PREFIX_SLICER(INDICES_0_TO_31, 32, 33, 34);
		MAKE_PREFIX_SLICER(INDICES_0_TO_31, 32, 33, 34, 35);
		MAKE_PREFIX_SLICER(INDICES_0_TO_31, 32, 33, 34, 35, 36);
		MAKE_PREFIX_SLICER(INDICES_0_TO_31, 32, 33, 34, 35, 36, 37);
		MAKE_PREFIX_SLICER(INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38);
		MAKE_PREFIX_SLICER(INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39);
		MAKE_PREFIX_SLICER(INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40);
		MAKE_PREFIX_SLICER(INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41);
		MAKE_PREFIX_SLICER(INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42);
		MAKE_PREFIX_SLICER(INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43);
		MAKE_PREFIX_SLICER(INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44);
		MAKE_PREFIX_SLICER(INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45);
		MAKE_PREFIX_SLICER(INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46);
		MAKE_PREFIX_SLICER(INDICES_0_TO_47);
		MAKE_PREFIX_SLICER(INDICES_0_TO_47, 48);
		MAKE_PREFIX_SLICER(INDICES_0_TO_47, 48, 49);
		MAKE_PREFIX_SLICER(INDICES_0_TO_47, 48, 49, 50);
		MAKE_PREFIX_SLICER(INDICES_0_TO_47, 48, 49, 50, 51);
		MAKE_PREFIX_SLICER(INDICES_0_TO_47, 48, 49, 50, 51, 52);
		MAKE_PREFIX_SLICER(INDICES_0_TO_47, 48, 49, 50, 51, 52, 53);
		MAKE_PREFIX_SLICER(INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54);
		MAKE_PREFIX_SLICER(INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55);
		MAKE_PREFIX_SLICER(INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56);
		MAKE_PREFIX_SLICER(INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57);
		MAKE_PREFIX_SLICER(INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58);
		MAKE_PREFIX_SLICER(INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59);
		MAKE_PREFIX_SLICER(INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60);
		MAKE_PREFIX_SLICER(INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61);
		MAKE_PREFIX_SLICER(INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62);
		MAKE_PREFIX_SLICER(INDICES_0_TO_63);

		#undef MAKE_PREFIX_SLICER_1
		#undef MAKE_PREFIX_SLICER

		// slicer - skip first N
		#define MAKE_SKIP_N_SLICER_1(N, N0, N1, ...)                                                                   \
			template <typename T##N0, typename T##N1 MUU_FOR_EACH(MAKE_INDEXED_TPARAM, __VA_ARGS__), typename... T, size_t Length>      \
			struct type_list_slicer_<type_list<T##N0, T##N1 MUU_FOR_EACH(MAKE_INDEXED_TARG, __VA_ARGS__), T...>,              \
									 N,																		\
									 Length,																				\
									 type_list_slicer_spec::skip_first_N>												\
			{																											\
				using type = typename type_list_slicer_<type_list<T##N, T...>, 0, Length>::type;\
			}
		#define MAKE_SKIP_N_SLICER(...) MAKE_SKIP_N_SLICER_1(__VA_ARGS__)

		template <typename T0, typename T1, typename... T, size_t Length>
		struct type_list_slicer_<type_list<T0, T1, T...>,
								 1,
								 Length,
								 type_list_slicer_spec::skip_first_N>
		{
			using type = typename type_list_slicer_<type_list<T1, T...>, 0, Length>::type;
		};

		template <typename T0, typename T1, typename T2, typename... T, size_t Length>
		struct type_list_slicer_<type_list<T0, T1, T2, T...>,
								 2,
								 Length,
								 type_list_slicer_spec::skip_first_N>
		{
			using type = typename type_list_slicer_<type_list<T2, T...>, 0, Length>::type;
		};

		MAKE_SKIP_N_SLICER(3,  0, 1, 2, 3);
		MAKE_SKIP_N_SLICER(4,  0, 1, 2, 3, 4);
		MAKE_SKIP_N_SLICER(5,  0, 1, 2, 3, 4, 5);
		MAKE_SKIP_N_SLICER(6,  0, 1, 2, 3, 4, 5, 6);
		MAKE_SKIP_N_SLICER(7,  0, 1, 2, 3, 4, 5, 6, 7);
		MAKE_SKIP_N_SLICER(8,  0, 1, 2, 3, 4, 5, 6, 7, 8);
		MAKE_SKIP_N_SLICER(9,  0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
		MAKE_SKIP_N_SLICER(10, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
		MAKE_SKIP_N_SLICER(11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
		MAKE_SKIP_N_SLICER(12, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
		MAKE_SKIP_N_SLICER(13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13);
		MAKE_SKIP_N_SLICER(14, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
		MAKE_SKIP_N_SLICER(15, INDICES_0_TO_15);
		MAKE_SKIP_N_SLICER(16, INDICES_0_TO_15, 16);
		MAKE_SKIP_N_SLICER(17, INDICES_0_TO_15, 16, 17);
		MAKE_SKIP_N_SLICER(18, INDICES_0_TO_15, 16, 17, 18);
		MAKE_SKIP_N_SLICER(19, INDICES_0_TO_15, 16, 17, 18, 19);
		MAKE_SKIP_N_SLICER(20, INDICES_0_TO_15, 16, 17, 18, 19, 20);
		MAKE_SKIP_N_SLICER(21, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21);
		MAKE_SKIP_N_SLICER(22, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22);
		MAKE_SKIP_N_SLICER(23, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23);
		MAKE_SKIP_N_SLICER(24, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24);
		MAKE_SKIP_N_SLICER(25, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25);
		MAKE_SKIP_N_SLICER(26, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26);
		MAKE_SKIP_N_SLICER(27, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27);
		MAKE_SKIP_N_SLICER(28, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28);
		MAKE_SKIP_N_SLICER(29, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29);
		MAKE_SKIP_N_SLICER(30, INDICES_0_TO_15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30);
		MAKE_SKIP_N_SLICER(31, INDICES_0_TO_31);
		MAKE_SKIP_N_SLICER(32, INDICES_0_TO_31, 32);
		MAKE_SKIP_N_SLICER(33, INDICES_0_TO_31, 32, 33);
		MAKE_SKIP_N_SLICER(34, INDICES_0_TO_31, 32, 33, 34);
		MAKE_SKIP_N_SLICER(35, INDICES_0_TO_31, 32, 33, 34, 35);
		MAKE_SKIP_N_SLICER(36, INDICES_0_TO_31, 32, 33, 34, 35, 36);
		MAKE_SKIP_N_SLICER(37, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37);
		MAKE_SKIP_N_SLICER(38, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38);
		MAKE_SKIP_N_SLICER(39, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39);
		MAKE_SKIP_N_SLICER(40, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40);
		MAKE_SKIP_N_SLICER(41, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41);
		MAKE_SKIP_N_SLICER(42, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42);
		MAKE_SKIP_N_SLICER(43, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43);
		MAKE_SKIP_N_SLICER(44, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44);
		MAKE_SKIP_N_SLICER(45, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45);
		MAKE_SKIP_N_SLICER(46, INDICES_0_TO_31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46);
		MAKE_SKIP_N_SLICER(47, INDICES_0_TO_47);
		MAKE_SKIP_N_SLICER(48, INDICES_0_TO_47, 48);
		MAKE_SKIP_N_SLICER(49, INDICES_0_TO_47, 48, 49);
		MAKE_SKIP_N_SLICER(50, INDICES_0_TO_47, 48, 49, 50);
		MAKE_SKIP_N_SLICER(51, INDICES_0_TO_47, 48, 49, 50, 51);
		MAKE_SKIP_N_SLICER(52, INDICES_0_TO_47, 48, 49, 50, 51, 52);
		MAKE_SKIP_N_SLICER(53, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53);
		MAKE_SKIP_N_SLICER(54, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54);
		MAKE_SKIP_N_SLICER(55, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55);
		MAKE_SKIP_N_SLICER(56, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56);
		MAKE_SKIP_N_SLICER(57, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57);
		MAKE_SKIP_N_SLICER(58, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58);
		MAKE_SKIP_N_SLICER(59, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59);
		MAKE_SKIP_N_SLICER(60, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60);
		MAKE_SKIP_N_SLICER(61, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61);
		MAKE_SKIP_N_SLICER(62, INDICES_0_TO_47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62);
		MAKE_SKIP_N_SLICER(63, INDICES_0_TO_63);
		MAKE_SKIP_N_SLICER(64, INDICES_0_TO_63, 64);

		#undef MAKE_SKIP_N_SLICER_1
		#undef MAKE_SKIP_N_SLICER
		#undef INDICES_0_TO_15
		#undef INDICES_0_TO_31
		#undef INDICES_0_TO_47
		#undef INDICES_0_TO_63
		#undef MAKE_INDEXED_TPARAM
		#undef MAKE_INDEXED_TARG
		// clang-format on

		// slicer - arbitrary ranges with length > (type_list_spec_max + 1)
		template <typename, size_t, typename>
		struct type_list_index_sequence_slicer_;
		template <typename List, size_t Start, size_t... Seq>
		struct type_list_index_sequence_slicer_<List, Start, std::index_sequence<Seq...>>
		{
			using type = type_list<type_list_select_<List, Start + Seq>...>;
		};
		template <typename List, size_t Start, size_t Length>
		struct type_list_slicer_<List, Start, Length, type_list_slicer_spec::big_range>
			: type_list_index_sequence_slicer_<List, Start, std::make_index_sequence<Length>>
		{};

		// slicer - helper typedef
		template <typename List, size_t Start, size_t Length>
		using type_list_slice_ = typename type_list_slicer_<List, Start, Length>::type;
	}

	template <>
	struct type_list<>
	{
		static constexpr size_t length = 0;

		// select and slice are clearly nonsensical in this case
		// but templates will break if they're not here

		template <size_t Index>
		using select = impl::type_list_select_<type_list<>, Index>;

		template <size_t Start, size_t Length = (length - Start)>
		using slice = impl::type_list_slice_<type_list<>, Start, Length>;
	};

	template <typename T>
	struct type_list<T>
	{
		static constexpr size_t length = 1;

		using first = T;
		using type	= first;

		template <size_t Index>
		using select = impl::type_list_select_<type_list<T>, Index>;

		template <size_t Start, size_t Length = (length - Start)>
		using slice = impl::type_list_slice_<type_list<T>, Start, Length>;
	};

	template <typename T0, typename... T>
	struct type_list<T0, T...>
	{
		static constexpr size_t length = 1 + sizeof...(T);

		using first = T0;
		using type	= first;

		template <size_t Index>
		using select = impl::type_list_select_<type_list<T0, T...>, Index>;

		template <size_t Start, size_t Length = (length - Start)>
		using slice = impl::type_list_slice_<type_list<T0, T...>, Start, Length>;
	};

	/// \endcond

#ifdef DOXYGEN

	/// \brief A 'tag' type for encoding/parameterizing lists of types (without the instantiation heft of std::tuple).
	///
	/// \detail	[set_class m-note m-success] This type is _not_ simply std::tuple without the container functionality!
	///			The underlying template machinery is highly optimized to reduce instantiation burden for even very
	///			massive type lists. Very compiler-friendly.
	///
	/// \tparam T	The list of types represented by the list.
	template <typename... T>
	struct type_list
	{
		/// \brief	The number of types in the list.
		///
		/// \detail \cpp
		/// std::cout << muu::type_list<int, float, char, void>::length << "\n";
		/// \ecpp
		/// \out
		/// 4
		/// \eout
		static constexpr size_t length = sizeof...(T);

		/// \brief The first type in the list.
		///
		/// \detail \cpp
		/// using first_type = muu::type_list<int, float, char, void>::first;
		///
		/// std::cout << "is int:   " << std::is_same_v<first_type, int> << "\n";
		/// std::cout << "is float: " << std::is_same_v<first_type, float> << "\n";
		/// std::cout << "is char:  " << std::is_same_v<first_type, char> << "\n";
		/// std::cout << "is void:  " << std::is_same_v<first_type, void> << "\n";
		/// \ecpp
		/// \out
		/// is int:   true
		/// is float: false
		/// is char:  false
		/// is void:  false
		/// \eout
		using first = POXY_IMPLEMENTATION_DETAIL;

		/// \brief Alias for #first when `length == 1`.
		/// \availability This member is only present when `length == 1`.
		using type = first;

		/// \brief Selects a single type from the list.
		///
		/// \detail \cpp
		/// using selected_type = muu::type_list<int, float, char, void>::select<2>;
		///
		/// std::cout << "is int:   " << std::is_same_v<selected_type, int> << "\n";
		/// std::cout << "is float: " << std::is_same_v<selected_type, float> << "\n";
		/// std::cout << "is char:  " << std::is_same_v<selected_type, char> << "\n";
		/// std::cout << "is void:  " << std::is_same_v<selected_type, void> << "\n";
		/// \ecpp
		/// \out
		/// is int:   false
		/// is float: false
		/// is char:  true
		/// is void:  false
		/// \eout
		template <size_t Index>
		using select = POXY_IMPLEMENTATION_DETAIL;

		/// \brief Selects a slice of types from the list.
		/// \detail \cpp
		/// using types = muu::type_list<int, float, char, void>;
		///
		/// static_assert(std::is_same_v<
		///		types::slice<2, 2>,
		///		muu::type_list<char, void>
		/// >);
		/// \ecpp
		template <size_t Start, size_t Length = (length - Start)>
		using slice = POXY_IMPLEMENTATION_DETAIL;
	};

#endif

	/// \brief A tag type for encoding/parameterizing a single index.
	template <size_t N>
	using index_tag = std::integral_constant<size_t, N>;

	/// \brief A tag type for encoding/parameterizing a single type.
	template <typename T>
	using type_tag = type_list<T>;

	/** @} */ // meta
}

#include "header_end.h"
