//# This file is a part of muu and is subject to the the terms of the MIT license.
//# Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
//# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.

/// \file
/// \brief Utilities and preprocessor machinery common to all files.
#pragma once

////////// CONFIGURATION
// clang-format off

#ifdef MUU_CONFIG_HEADER
	#include MUU_CONFIG_HEADER
	#undef MUU_CONFIG_HEADER
#endif

#if !defined(MUU_ALL_INLINE) || (defined(MUU_ALL_INLINE) && MUU_ALL_INLINE)
	#undef MUU_ALL_INLINE
	#define MUU_ALL_INLINE 1
#endif

#if defined(MUU_IMPLEMENTATION) || MUU_ALL_INLINE || defined(__INTELLISENSE__)
	#undef MUU_IMPLEMENTATION
	#define MUU_IMPLEMENTATION 1
#else
	#define MUU_IMPLEMENTATION 0
#endif

#ifndef MUU_API
	#define MUU_API
#endif

#ifndef MUU_UNDEF_MACROS
	#define MUU_UNDEF_MACROS 1
#endif

// MUU_ASSERT

////////// COMPILER, ARCHITECTURE ETC.

#ifndef __cplusplus
	#error muu is a C++ library.
#endif
#if defined(__ia64__) || defined(__ia64) || defined(_IA64) || defined(__IA64__) || defined(_M_IA64)
	#define MUU_ARCH_IA64 1
	#define MUU_ARCH_BITNESS 64
#else
	#define MUU_ARCH_IA64 0
#endif
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_AMD64)
	#define MUU_ARCH_AMD64 1
	#define MUU_ARCH_BITNESS 64
#else
	#define MUU_ARCH_AMD64 0
#endif
#if defined(__i386__) || defined(_M_IX86)
	#define MUU_ARCH_x86 1
	#define MUU_ARCH_BITNESS 32
#else
	#define MUU_ARCH_x86 0
#endif
#if (MUU_ARCH_IA64 + MUU_ARCH_AMD64 + MUU_ARCH_x86) != 1
	#error Could not uniquely identify target architecture.
#endif
#ifdef __clang__

	#define MUU_PUSH_WARNINGS				_Pragma("clang diagnostic push")
	#define MUU_DISABLE_SWITCH_WARNINGS		_Pragma("clang diagnostic ignored \"-Wswitch\"")
	#define MUU_DISABLE_INIT_WARNINGS		_Pragma("clang diagnostic ignored \"-Wmissing-field-initializers\"")
	#define MUU_DISABLE_ALL_WARNINGS		_Pragma("clang diagnostic ignored \"-Weverything\"")
	#define MUU_POP_WARNINGS				_Pragma("clang diagnostic pop")
	#define MUU_ASSUME(cond)				__builtin_assume(cond)
	#define MUU_UNREACHABLE					__builtin_unreachable()
	#if defined(_MSC_VER) // msvc compat mode
		#ifdef __has_declspec_attribute
			#if __has_declspec_attribute(novtable)
				#define MUU_INTERFACE			__declspec(novtable)
			#endif
			#if __has_declspec_attribute(empty_bases)
				#define MUU_EMPTY_BASES			__declspec(empty_bases)
			#endif
			#if __has_declspec_attribute(restrict)
				#define MUU_UNALIASED_ALLOC		__declspec(restrict)
			#endif
			#define MUU_ALWAYS_INLINE			__forceinline
		#endif
	#else // regular ol' clang
		#define MUU_GNU_ATTR(attr)				__attribute__((attr))
		#ifdef __has_attribute
			#if __has_attribute(noinline)
				#define MUU_NEVER_INLINE		__attribute__((__noinline__))
			#endif
			#if __has_attribute(always_inline)
				#define MUU_ALWAYS_INLINE		__attribute__((__always_inline__)) inline
			#endif
			#if __has_attribute(trivial_abi)
				#define MUU_TRIVIAL_ABI			__attribute__((__trivial_abi__))
			#endif
		#endif
	#endif
	#ifdef __EXCEPTIONS
		#define MUU_EXCEPTIONS 1
	#endif

#elif defined(_MSC_VER) || (defined(__INTEL_COMPILER) && defined(__ICL))

	#define MUU_CPP_VERSION					_MSVC_LANG
	#define MUU_PUSH_WARNINGS				__pragma(warning(push))
	#define MUU_DISABLE_SWITCH_WARNINGS		__pragma(warning(disable: 4063))
	#define MUU_DISABLE_ALL_WARNINGS		__pragma(warning(pop))	\
											__pragma(warning(push, 0))
	#define MUU_POP_WARNINGS				__pragma(warning(pop))
	#define MUU_ALWAYS_INLINE				__forceinline
	#define MUU_NEVER_INLINE				__declspec(noinline)
	#define MUU_ASSUME(cond)				__assume(cond)
	#define MUU_UNREACHABLE					__assume(0)
	#define MUU_INTERFACE					__declspec(novtable)
	#define MUU_EMPTY_BASES					__declspec(empty_bases)
	#define MUU_UNALIASED_ALLOC				__declspec(restrict)
	#ifdef _CPPUNWIND
		#define MUU_EXCEPTIONS 1
	#endif

#elif defined(__GNUC__)

	#define MUU_PUSH_WARNINGS				_Pragma("GCC diagnostic push")
	#define MUU_DISABLE_SWITCH_WARNINGS		_Pragma("GCC diagnostic ignored \"-Wswitch\"")
	#define MUU_DISABLE_INIT_WARNINGS		_Pragma("GCC diagnostic ignored \"-Wmissing-field-initializers\"")	\
											_Pragma("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")			\
											_Pragma("GCC diagnostic ignored \"-Wuninitialized\"")
	#define MUU_DISABLE_ALL_WARNINGS		_Pragma("GCC diagnostic ignored \"-Wall\"")							\
											_Pragma("GCC diagnostic ignored \"-Wextra\"")						\
											_Pragma("GCC diagnostic ignored \"-Wchar-subscripts\"")				\
											_Pragma("GCC diagnostic ignored \"-Wtype-limits\"")
	#define MUU_POP_WARNINGS				_Pragma("GCC diagnostic pop")
	#define MUU_GNU_ATTR(attr)				__attribute__((attr))
	#define MUU_ALWAYS_INLINE				__attribute__((__always_inline__)) inline
	#define MUU_NEVER_INLINE				__attribute__((__noinline__))
	#define MUU_UNREACHABLE					__builtin_unreachable()
	#ifdef __cpp_exceptions
		#define MUU_EXCEPTIONS 1
	#endif
	#define MUU_LIKELY // these pass the __has_attribute() test but cause warnings on if/else branches =/
	#define MUU_UNLIKELY

#endif
#ifndef MUU_CPP_VERSION
	#define MUU_CPP_VERSION __cplusplus
#endif
#if MUU_CPP_VERSION >= 202600L
	#define MUU_CPP 26
#elif MUU_CPP_VERSION >= 202300L
	#define MUU_CPP 23
#elif MUU_CPP_VERSION >= 202002L
	#define MUU_CPP 20
#elif MUU_CPP_VERSION >= 201703L
	#define MUU_CPP 17
#else
	#error muu requires C++17 or higher.
#endif
#undef MUU_CPP_VERSION
#ifndef MUU_EXCEPTIONS
	#define MUU_EXCEPTIONS 1
#endif
#ifndef MUU_PUSH_WARNINGS
	#define MUU_PUSH_WARNINGS
#endif
#ifndef MUU_DISABLE_INIT_WARNINGS
	#define	MUU_DISABLE_INIT_WARNINGS
#endif
#ifndef MUU_DISABLE_ALL_WARNINGS
	#define MUU_DISABLE_ALL_WARNINGS
#endif
#ifndef MUU_POP_WARNINGS
	#define MUU_POP_WARNINGS
#endif
#ifndef MUU_GNU_ATTR
	#define MUU_GNU_ATTR(attr)
#endif
#ifndef MUU_INTERFACE
	#define MUU_INTERFACE
#endif
#ifndef MUU_EMPTY_BASES
	#define MUU_EMPTY_BASES
#endif
#ifndef MUU_ALWAYS_INLINE
	#define MUU_ALWAYS_INLINE			inline
#endif
#ifndef MUU_NEVER_INLINE
	#define MUU_NEVER_INLINE
#endif
#ifndef MUU_UNALIASED_ALLOC
	#define MUU_UNALIASED_ALLOC
#endif
#ifndef MUU_ASSUME
	#define MUU_ASSUME(cond)			(void)0
#endif
#ifndef MUU_UNREACHABLE
	#define MUU_UNREACHABLE				MUU_ASSERT(false)
#endif
#define MUU_NO_DEFAULT_CASE				default: MUU_UNREACHABLE
#ifndef __INTELLISENSE__
	#if !defined(MUU_LIKELY) && __has_cpp_attribute(likely)
		#define MUU_LIKELY				[[likely]]
	#endif
	#if !defined(MUU_UNLIKELY) && __has_cpp_attribute(unlikely)
		#define MUU_UNLIKELY			[[unlikely]]
	#endif
	#if !defined(MUU_NO_UNIQUE_ADDRESS) && __has_cpp_attribute(no_unique_address)
		#define MUU_NO_UNIQUE_ADDRESS	[[no_unique_address]]
	#endif
	#if __has_cpp_attribute(nodiscard) >= 201907L
		#define MUU_NODISCARD_CTOR		[[nodiscard]]
	#endif
#endif //__INTELLISENSE__
#ifndef MUU_LIKELY
	#define MUU_LIKELY
#endif
#ifndef MUU_UNLIKELY
	#define MUU_UNLIKELY
#endif
#ifndef MUU_NO_UNIQUE_ADDRESS
	#define MUU_NO_UNIQUE_ADDRESS
#endif
#ifndef MUU_NODISCARD_CTOR
	#define MUU_NODISCARD_CTOR
#endif
#ifndef MUU_TRIVIAL_ABI
	#define MUU_TRIVIAL_ABI
#endif
#ifdef __cpp_consteval
	#define MUU_CONSTEVAL				consteval
#else
	#define MUU_CONSTEVAL				constexpr
#endif
#if MUU_ALL_INLINE
	#define MUU_FUNC_EXTERNAL_LINKAGE	inline
	#define MUU_FUNC_INTERNAL_LINKAGE	inline
#else
	#define MUU_FUNC_EXTERNAL_LINKAGE
	#define MUU_FUNC_INTERNAL_LINKAGE	static
#endif
#define MUU_PREPEND_R_1(S)				R##S
#define MUU_PREPEND_R(S)				MUU_PREPEND_R_1(S)
#define MUU_ADD_PARENTHESES_1(S)		(S)
#define MUU_ADD_PARENTHESES(S)			MUU_ADD_PARENTHESES_1(S)
#define MUU_MAKE_STRING_3(S)			#S
#define MUU_MAKE_STRING_2(S)			MUU_MAKE_STRING_3(S)
#define MUU_MAKE_STRING_1(S)			MUU_MAKE_STRING_2(MUU_ADD_PARENTHESES(S))
#define MUU_MAKE_STRING(S)				MUU_PREPEND_R(MUU_MAKE_STRING_1(S))
#define MUU_APPEND_SV_1(S)				S##sv
#define MUU_APPEND_SV(S)				MUU_APPEND_SV_1(S)
#define MUU_MAKE_STRING_VIEW(S)			MUU_APPEND_SV(MUU_MAKE_STRING(S))
#define MUU_VERSION_MAJOR				0
#define MUU_VERSION_MINOR				1
#define MUU_VERSION_PATCH				0

////////// INCLUDES

MUU_PUSH_WARNINGS
MUU_DISABLE_ALL_WARNINGS

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <climits>
#include <type_traits>
#include <utility>
#ifndef MUU_ASSERT
	#include <cassert>
	#define MUU_ASSERT(expr)			assert(expr)
#endif
#if __has_include(<version>)
	#include <version>
#endif

MUU_POP_WARNINGS

////////// TYPE TRAITS AND METAFUNCTIONS

namespace muu::impl
{
	template <typename T, bool = std::is_enum<T>::value>
	struct underlying_type : std::underlying_type<T> {};
	template <typename T>
	struct underlying_type<T, false>
	{
		using type = T;
	};

	template <typename T, typename... U>
	struct is_one_of : std::integral_constant<bool,
		(false || ... || std::is_same_v<T, U>)
	> {};

	template <typename T> struct remove_const { using type = T; };
	template <typename T> struct remove_const<const T> { using type = T; };
	template <typename T> struct remove_const<T&> { using type = T&; };
	template <typename T> struct remove_const<const T&> { using type = T&; };
	template <typename T> struct remove_const<T&&> { using type = T&&; };
	template <typename T> struct remove_const<const T&&> { using type = T&&; };

	template <typename T> struct remove_volatile { using type = T; };
	template <typename T> struct remove_volatile<volatile T> { using type = T; };
	template <typename T> struct remove_volatile<T&> { using type = T&; };
	template <typename T> struct remove_volatile<volatile T&> { using type = T&; };
	template <typename T> struct remove_volatile<T&&> { using type = T&&; };
	template <typename T> struct remove_volatile<volatile T&&> { using type = T&&; };

	template <typename T, bool cond>
	struct conditionally_add_const
	{
		using type = std::conditional_t<cond, std::add_const_t<T>, T>;
	};

	template <typename T, bool cond>
	struct conditionally_add_const<T&, cond>
	{
		using type = std::conditional_t<cond, std::add_const_t<T>&, T&>;
	};

	template <typename T, bool cond>
	struct conditionally_add_const<T&&, cond>
	{
		using type = std::conditional_t<cond, std::add_const_t<T>&&, T&&>;
	};

	template <typename T, bool cond>
	struct conditionally_add_volatile
	{
		using type = std::conditional_t<cond, std::add_volatile_t<T>, T>;
	};

	template <typename T, bool cond>
	struct conditionally_add_volatile<T&, cond>
	{
		using type = std::conditional_t<cond, std::add_volatile_t<T>&, T&>;
	};

	template <typename T, bool cond>
	struct conditionally_add_volatile<T&&, cond>
	{
		using type = std::conditional_t<cond, std::add_volatile_t<T>&&, T&&>;
	};

	template <typename T>
	struct remove_noexcept						{ using type = T; };
	template <typename T>
	struct remove_noexcept<const T>				{ using type = const typename remove_noexcept<T>::type; };
	template <typename T>
	struct remove_noexcept<volatile T>			{ using type = volatile typename remove_noexcept<T>::type; };
	template <typename T>
	struct remove_noexcept<const volatile T>	{ using type = const volatile typename remove_noexcept<T>::type; };
	template <typename T>
	struct remove_noexcept<T&>					{ using type = typename remove_noexcept<T>::type&; };
	template <typename T>
	struct remove_noexcept<T&&>					{ using type = typename remove_noexcept<T>::type&&; };

	template <typename R, typename ...P>
	struct remove_noexcept<R(P...) noexcept>						{ using type = R(P...); };
	template <typename R, typename ...P>
	struct remove_noexcept<R(*)(P...) noexcept>						{ using type = R(*)(P...); };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) noexcept>					{ using type = R(C::*)(P...); };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) & noexcept>				{ using type = R(C::*)(P...) &; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) && noexcept>				{ using type = R(C::*)(P...) &&; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) const noexcept>			{ using type = R(C::*)(P...) const; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) const & noexcept>			{ using type = R(C::*)(P...) const &; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) const && noexcept>			{ using type = R(C::*)(P...) const &&; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) volatile noexcept>			{ using type = R(C::*)(P...) volatile; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) volatile & noexcept>		{ using type = R(C::*)(P...) volatile &; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) volatile && noexcept>		{ using type = R(C::*)(P...) volatile &&; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) const volatile noexcept>	{ using type = R(C::*)(P...) const volatile; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) const volatile & noexcept>	{ using type = R(C::*)(P...) const volatile &; };
	template <typename C, typename R, typename ...P>
	struct remove_noexcept<R(C::*)(P...) const volatile && noexcept>{ using type = R(C::*)(P...) const volatile &&; };

	template <typename T>
	struct constify_pointer
	{
		static_assert(std::is_pointer_v<T>);
		using type = std::add_pointer_t<std::add_const_t<std::remove_pointer_t<T>>>;
	};
	template <> struct constify_pointer<void*> { using type = const void*; };
	template <> struct constify_pointer<const void*> { using type = const void*; };
	template <> struct constify_pointer<volatile void*> { using type = const volatile void*; };
	template <> struct constify_pointer<const volatile void*> { using type = const volatile void*; };
}

/// \defgroup Traits Type traits and metafunctions to supplement the standard ones found in <type_traits>.
/// @{

namespace muu
{
	/// \brief	Gets the underlying integer of an enum type.
	/// \remarks This is safe to use on non-enums and in SFINAE contexts,
	/// 		 unlike std::underlying_type.
	template <typename T>
	using underlying_type = typename impl::underlying_type<T>::type;

	#if defined(__cpp_lib_remove_cvref) || (defined(_MSC_VER) && defined(_HAS_CXX20) && _HAS_CXX20)
		template <typename T>
		using remove_cvref = std::remove_cvref_t<T>;
	#else
		template <typename T>
		using remove_cvref = std::remove_cv_t<std::remove_reference_t<T>>;
	#endif

	/// \brief	True if T is exactly the same as one or more of the types named by U.
	template <typename T, typename... U>
	inline constexpr bool is_one_of = impl::is_one_of<T, U...>::value;

	/// \brief Is a type unsigned?
	/// \remarks For references this is equivalent to asking "is the referenced type unsigned?"
	template <typename T>
	inline constexpr bool is_unsigned = std::is_unsigned_v<underlying_type<std::remove_reference_t<T>>>;

	/// \brief Is a type signed?
	/// \remarks For references this is equivalent to asking "is the referenced type signed?"
	template <typename T>
	inline constexpr bool is_signed = std::is_signed_v<underlying_type<std::remove_reference_t<T>>>;

	/// \brief Is a type const?
	/// \remarks For references this is equivalent to asking "is the referenced type `const`?"
	template <typename T>
	inline constexpr bool is_const = std::is_const_v<std::remove_reference_t<T>>;

	/// \brief Removes the outermost const qualifier from a type, if it has one.
	/// \remarks For references this is equivalent to removing const from the
	/// 		 referenced type (e.g. `const int&` becomes `int&`).
	template <typename T>
	using remove_const = typename impl::remove_const<T>::type;

	/// \brief Shorthand for conditionally adding const to a type.
	template <typename T, bool cond>
	using conditionally_add_const = typename impl::conditionally_add_const<T, cond>::type;

	/// \brief Shorthand for marking a type or reference with the same constness as another type or reference.
	template <typename T, typename match_with>
	using match_const = conditionally_add_const<remove_const<T>, is_const<match_with>>;

	/// \brief Is a type or reference volatile?
	/// \remarks For references this is equivalent to asking "is the referenced type `volatile`?"
	template <typename T>
	inline constexpr bool is_volatile = std::is_volatile_v<std::remove_reference_t<T>>;

	/// \brief Removes the outermost volatile qualifier from a type or reference.
	/// \remarks For references this is equivalent to removing volatile from the
	/// 		 referenced type (e.g. `volatile int&` becomes `int&`).
	template <typename T>
	using remove_volatile = typename impl::remove_volatile<T>::type;

	/// \brief Shorthand for conditionally adding volatility to a type or reference.
	template <typename T, bool cond>
	using conditionally_add_volatile = typename impl::conditionally_add_volatile<T, cond>::type;

	/// \brief Shorthand for marking a type or reference with the same volatility
	/// 	   as another type or reference.
	template <typename T, typename match_with>
	using match_volatile = conditionally_add_volatile<remove_volatile<T>, is_volatile<match_with>>;

	/// \brief Removes the outermost const and volatile qualifiers from a type or reference.
	/// \remarks For references this is equivalent to removing const and volatile from the
	/// 		 referenced type (e.g. `const volatile int&` becomes `int&`).
	template <typename T>
	using remove_cv = remove_const<remove_volatile<T>>;

	/// \brief Shorthand for marking a type or reference with the same constness and volatility
	/// 	   as another type or reference.
	template <typename T, typename match_with>
	using match_cv = match_const<match_volatile<T, match_with>, match_with>;

	/// \brief Returns a functional type without any `noexcept` specifier.
	template <typename T>
	using remove_noexcept = typename impl::remove_noexcept<T>::type;

	/// \brief Does T inherit from ancestor?
	/// \remarks This does not return true when the objects are the same type, unlike std::is_base_of.
	template <typename T, typename ancestor>
	inline constexpr bool inherits_from
		= std::is_base_of_v<remove_cvref<ancestor>, remove_cvref<T>>
		&& !std::is_same_v<remove_cvref<ancestor>, remove_cvref<T>>;

	/// \brief	Adds const to the pointed-to type of a pointer, correctly handling pointers-to-void.
	template <typename T>
	using constify_pointer = typename impl::constify_pointer<T>::type;
}

/// @}

////////// FORWARD DECLARATIONS, TYPEDEFS & 'INTRINSICS'

/// \brief	The root namespace for all muu functions and types.
namespace muu
{
	using size_t = std::size_t;
	using intptr_t = std::intptr_t;
	using uintptr_t = std::uintptr_t;
	using ptrdiff_t = std::ptrdiff_t;
	using nullptr_t = std::nullptr_t;
	using byte = std::byte;

	struct uuid;
	struct semver;
	struct half;

	class blob;
	class bitset;
	class thread_pool;
	template <typename>				class scope_guard;
	template <typename, size_t>		class tagged_ptr;

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

	/// \brief Compile-time build constants.
	namespace build
	{
		/// \brief The current C++ language version (17, 20...)
		static constexpr uint32_t cpp_version = MUU_CPP;
		static_assert(
			cpp_version == 17
			|| cpp_version == 20
			|| cpp_version == 23 //??
			|| cpp_version == 26 //??
		);

		/// \brief The current year.
		static constexpr uint32_t year =
			(impl::date_str[7] - '0') * 1000
			+ (impl::date_str[8] - '0') * 100
			+ (impl::date_str[9] - '0') * 10
			+ (impl::date_str[10] - '0');
		static_assert(year >= 2019u);

		/// \brief The current month of the year (1-12).
		static constexpr uint32_t month =
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
		static constexpr uint32_t day =
			(impl::date_str[4] == ' ' ? 0 : impl::date_str[4] - '0') * 10
			+ (impl::date_str[5] - '0');
		static_assert(day >= 1 && day <= 31);

		/// \brief The current hour of the day (0-23).
		static constexpr uint32_t hour =
			(impl::time_str[0] == ' ' ? 0 : impl::time_str[0] - '0') * 10
			+ (impl::time_str[1] - '0');
		static_assert(hour >= 0 && hour <= 23);

		/// \brief The current minute (0-59).
		static constexpr uint32_t minute =
			(impl::time_str[3] == ' ' ? 0 : impl::time_str[3] - '0') * 10
			+ (impl::time_str[4] - '0');
		static_assert(minute >= 0 && minute <= 59);

		/// \brief The current second (0-59).
		static constexpr uint32_t second =
			(impl::time_str[6] == ' ' ? 0 : impl::time_str[6] - '0') * 10
			+ (impl::time_str[7] - '0');
		static_assert(second >= 0 && second <= 60); // 60 b/c leap seconds

		/// \brief	The bitness of the current architecture.
		static constexpr size_t bitness = MUU_ARCH_BITNESS;

		/// \brief The number of bits in a byte (aka CHAR_BIT).
		static constexpr size_t bits_per_byte = CHAR_BIT;

		/// \brief The number of bytes required to store a pointer.
		static constexpr size_t pointer_size = sizeof(void*);

		/// \brief The number of bits required to store a pointer.
		static constexpr size_t pointer_bits = pointer_size * bits_per_byte;

		/// \brief True if exceptions are enabled.
		static constexpr bool exceptions_enabled = !!MUU_EXCEPTIONS;
	
	} //::build

	/// \defgroup Intrinsics Small functions which map to compiler intrinsics wherever possible.
	/// @{

	/// \brief	Equivalent to C++20's std::is_constant_evaluated().
	/// 
	/// \details Compilers implement this as an intrinsic which is typically
	/// 		 available regardless of the C++ mode. Using this function
	/// 		 on these compilers allows you to get the same behaviour
	/// 		 even when you aren't targeting C++20.
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

	/// \brief	Equivalent to C++17's std::launder().
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

	template <typename T>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr auto unbox(T val) noexcept
	{
		if constexpr (std::is_enum_v<T>)
			return static_cast<std::underlying_type_t<T>>(val);
		else
			return val;
	}

	template <typename T, typename = std::enable_if_t<is_unsigned<T>>>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr bool has_single_bit(T val) noexcept
	{
		if constexpr (std::is_enum_v<T>)
			return has_single_bit(unbox(val));
		else
			return val != T{} && (val & (val - T{ 1 })) == T{};
	}

	template <typename T, typename = std::enable_if_t<is_unsigned<T>>>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr int countl_zero(T val) noexcept
	{
		if constexpr (std::is_enum_v<T>)
			return static_cast<T>(countl_zero(unbox(val)));
		else
		{
			#if MUU_HAS_BIT
				return std::countl_zero(val);
			#else
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
				#elif 
					// ...
				#endif
			}
			#endif
		}
	}

	template <typename T, typename = std::enable_if_t<is_unsigned<T>>>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr int countr_zero(T val) noexcept
	{
		if constexpr (std::is_enum_v<T>)
			return static_cast<T>(countr_zero(unbox(val)));
		else
		{
			#if MUU_HAS_BIT
				return std::countr_zero(val);
			#else
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
				#elif 
					// ...
				#endif
			}
			#endif
		}
	}

	template <typename T, typename = std::enable_if_t<is_unsigned<T>>>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr T bit_ceil(T val) noexcept
	{
		if constexpr (std::is_enum_v<T>)
			return static_cast<T>(bit_ceil(unbox(val)));
		else
		{
			if (!val)
				return T{ 1 };
			return T{ 1 } << (sizeof(T) * CHAR_BIT - countl_zero(static_cast<T>(val - T{ 1 })));
		}
	}

	template <typename T, typename = std::enable_if_t<is_unsigned<T>>>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr T bit_floor(T val) noexcept
	{
		if constexpr (std::is_enum_v<T>)
			return static_cast<T>(bit_floor(unbox(val)));
		else
		{
			if (!val)
				return T{ 0 };
			return T{ 1 } << (sizeof(T) * CHAR_BIT - 1 - countl_zero(val));
		}
	}

	template <typename T, typename = std::enable_if_t<is_unsigned<T>>>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr T bit_width(T val) noexcept
	{
		if constexpr (std::is_enum_v<T>)
			return static_cast<T>(bit_width(unbox(val)));
		else
			return static_cast<T>(sizeof(T) * CHAR_BIT - countl_zero(val));
	}

	template <typename T, typename = std::enable_if_t<is_unsigned<T>>>
	[[nodiscard]]
	constexpr T bit_fill_right(size_t count) noexcept
	{
		if constexpr (std::is_enum_v<T>)
			return T{ bit_fill_right<underlying_type<T>>(count) };
		else
		{
			if (!count)
				return T{};
			if (count >= build::bits_per_byte * sizeof(T))
				return static_cast<T>(~T{});
			return (T{ 1 } << count) - T{ 1 };
		}
	}

	template <typename T, typename = std::enable_if_t<is_unsigned<T>>>
	[[nodiscard]]
	constexpr T bit_fill_left(size_t count) noexcept
	{
		if constexpr (std::is_enum_v<T>)
			return T{ bit_fill_left<underlying_type<T>>(count) };
		else
		{
			if (!count)
				return T{};
			if (count >= build::bits_per_byte * sizeof(T))
				return static_cast<T>(~T{});
			return bit_fill_right<T>(count) << (build::bits_per_byte * sizeof(T) - count);
		}
	}

	template <typename TO, typename FROM>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr TO bit_cast(const FROM& from) noexcept
	{
		static_assert(
			!std::is_reference_v<TO> && !std::is_reference_v<FROM>,
			"FROM and TO types cannot be references"
		);
		static_assert(
			std::is_trivially_copyable_v<FROM> && std::is_trivially_copyable_v<TO>,
			"FROM and TO types must be trivially-copyable"
		);
		static_assert(
			sizeof(TO) == sizeof(FROM),
			"FROM and TO types must be the same size"
		);

		#define MUU_FALLBACK_IMPL				\
			TO dst;								\
			memcpy(&dst, &from, sizeof(TO));	\
			return dst

		#if defined(__clang__)
			#if __has_builtin(__builtin_bit_cast)
				return __builtin_bit_cast(TO, from);
			#else
				MUU_FALLBACK_IMPL;
			#endif
		#elif defined(__GNUC__) && __GNUC__ >= 10 // ??
			return __builtin_bit_cast(TO, from);
		#elif defined(_MSC_VER) && _MSC_VER >= 1926 // Visual Studio 2019 version 16.6
			return __builtin_bit_cast(TO, from);
		#else
			MUU_FALLBACK_IMPL;
		#endif

		#undef MUU_FALLBACK_IMPL
	}

	template <typename T>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr T min(T val1, T val2) noexcept
	{
		if constexpr (std::is_enum_v<T>)
			return static_cast<T>(min(unbox(val1), unbox(val2)));
		else
			return val1 < val2 ? val1 : val2;
	}

	template <typename T>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr T max(T val1, T val2) noexcept
	{
		if constexpr (std::is_enum_v<T>)
			return static_cast<T>(max(unbox(val1), unbox(val2)));
		else
			return val1 < val2 ? val2 : val1;
	}

	template <typename T>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr T clamp(T val, T min, T max) noexcept
	{
		if constexpr (std::is_enum_v<T>)
			return static_cast<T>(clamp(unbox(val), unbox(min), unbox(max)));
		else
			return muu::max(muu::min(val, max), min);
	}

	template <typename T>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr bool is_between(T val, T min, T max) noexcept
	{
		if constexpr (std::is_enum_v<T>)
			return is_between(unbox(val), unbox(min), unbox(max));
		else
			return val >= min && val <= max;
	}

	/// \brief	Casts between pointers, choosing the most appropriate conversion path.
	/// 
	/// \tparam TO	A pointer or integer large enough to store a pointer
	/// \tparam FROM A pointer, array, nullptr_t, or an integer large enough to store a pointer.
	/// \param from The value being cast.
	/// 
	/// \return The input value casted to the desired type.
	/// 
	/// \warning This is a huge footgun if you're not careful with it!
	template <typename TO, typename FROM>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr TO pointer_cast(FROM from) noexcept
	{
		static_assert(
			!std::is_reference_v<TO>
			&& !std::is_reference_v<FROM>, // will never be deduced as a reference but it might be specified explicitly
			"FROM and TO types cannot be references"
		);

		static_assert(
			!(std::is_integral_v<FROM> && std::is_integral_v<TO>),
			"FROM and TO types cannot both be integers (did you mean static_cast?)"
		);

		static_assert(
			std::is_pointer_v<TO>
			|| (std::is_integral_v<TO> && sizeof(TO) >= sizeof(void*)),
			"TO type must be a pointer or an integer large enough to store a pointer"
		);

		static_assert(
			std::is_pointer_v<FROM>
			|| std::is_same_v<FROM, nullptr_t>
			|| (std::is_integral_v<FROM> && sizeof(FROM) >= sizeof(void*)),
			"FROM type must be a pointer, array, nullptr_t, or an integer large enough to store a pointer"
		);

		static_assert(
			!std::is_member_pointer_v<TO> && !std::is_member_pointer_v<FROM>,
			"FROM and TO types cannot be pointers to members"
		);

		// same input and output types (no-op)
		if constexpr (std::is_same_v<FROM, TO>)
			return from;

		// nullptr -> *
		else if constexpr (std::is_same_v<FROM, nullptr_t>)
		{
			(void)from;
			return {};
		}

		// pointer -> integer
		else if constexpr (std::is_integral_v<TO>)
		{
			// widening conversion
			if constexpr (sizeof(TO) > sizeof(FROM))
			{
				using cast_t = std::conditional_t<is_signed<TO>, intptr_t, uintptr_t>;
				return static_cast<TO>(reinterpret_cast<cast_t>(from));
			}
			// same size
			else
			{
				static_assert(sizeof(TO) == sizeof(FROM));
				return reinterpret_cast<TO>(from);
			}
		}

		// * -> pointer
		else
		{
			// integer -> pointer
			if constexpr (std::is_integral_v<FROM>)
			{
				// (uintptr_t, intptr_t) -> pointer
				if constexpr (is_one_of<FROM, uintptr_t, intptr_t>)
				{
					return reinterpret_cast<TO>(from);
				}

				// other integers -> pointer
				else
				{
					using cast_t = std::conditional_t<is_signed<FROM>, intptr_t, uintptr_t>;
					return reinterpret_cast<TO>(static_cast<cast_t>(from));
				}
			}

			// pointer -> pointer
			else
			{
				using from_t = std::remove_pointer_t<FROM>;
				using to_t = std::remove_pointer_t<TO>;

				// function -> *
				if constexpr (std::is_function_v<from_t> && !std::is_function_v<to_t>)
				{
					static_assert(
						std::is_void_v<to_t>,
						"Cannot cast from a function pointer to a type other than void or another function pointer"
					);

					return pointer_cast<TO>(reinterpret_cast<match_cv<to_t, from_t>*>(from));
				}

				// * -> function
				else if constexpr (!std::is_function_v<from_t> && std::is_function_v<to_t>)
				{
					static_assert(
						std::is_void_v<from_t>,
						"Cannot cast to a function pointer from a type other than void or another function pointer"
					);

					return pointer_cast<TO>(reinterpret_cast<match_cv<to_t, from_t>*>(from));
				}

				// function -> function
				else if constexpr (std::is_function_v<from_t> && std::is_function_v<to_t>)
				{
					static_assert(
						std::is_same_v<from_t, to_t>
						|| std::is_same_v<std::remove_pointer_t<remove_noexcept<FROM>>, to_t>,
						"Cannot cast between pointers to two different function types"
					);
					
					return pointer_cast<TO>(reinterpret_cast<match_cv<to_t, from_t>*>(from));
				}

				// non-void -> void
				// void -> non-void
				// derived -> base
				else if constexpr (std::is_void_v<from_t> != std::is_void_v<to_t> || inherits_from<from_t, to_t>)
				{
					return static_cast<TO>(pointer_cast<match_cv<from_t, to_t>*>(from));
				}

				// base -> derived
				else if constexpr (inherits_from<to_t, from_t>)
				{
					return dynamic_cast<TO>(pointer_cast<match_cv<from_t, to_t>*>(from));
				}

				// A -> B (unrelated types)
				else if constexpr (!std::is_same_v<std::remove_cv_t<from_t>, std::remove_cv_t<to_t>>)
				{
					return reinterpret_cast<TO>(pointer_cast<match_cv<from_t, to_t>*>(from));
				}

				// A -> A (different cv)
				else
				{
					static_assert(!std::is_same_v<from_t, to_t>);
					static_assert(std::is_same_v<remove_cv<from_t>, remove_cv<to_t>>);

					// remove const
					if constexpr (is_const<from_t> && !is_const<to_t>)
					{
						return pointer_cast<TO>(const_cast<remove_const<from_t>*>(from));
					}

					// remove volatile
					else if constexpr (is_volatile<from_t> && !is_volatile<to_t>)
					{
						return pointer_cast<TO>(const_cast<remove_volatile<from_t>*>(from));
					}

					// add const
					else if constexpr (!is_const<from_t> && is_const<to_t>)
					{
						return pointer_cast<TO>(static_cast<std::add_const<from_t>*>(from));
					}

					// add volatile
					else if constexpr (!is_volatile<from_t> && is_volatile<to_t>)
					{
						return pointer_cast<TO>(static_cast<std::add_volatile<from_t>*>(from));
					}
				}
			}
		}
	}

	template <typename TO, typename FROM, size_t N>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr TO pointer_cast(FROM(&arr)[N]) noexcept
	{
		return pointer_cast<TO>(&arr[0]);
	}

	template <typename TO, typename FROM, size_t N>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr TO pointer_cast(FROM(&&arr)[N]) noexcept
	{
		return pointer_cast<TO>(&arr[0]);
	}

	/// \brief	Applies a byte offset to a pointer.
	///
	/// \tparam	T	The type being pointed to.
	/// \param	ptr	The pointer to offset.
	/// \param	offset	The number of bytes to add to the pointer's address.
	///
	/// \return	A pointer to T containing the result of `static_cast<char>(ptr) + offset`.
	template <typename T>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr T* apply_offset(T* ptr, ptrdiff_t offset) noexcept
	{
		return pointer_cast<T*>(pointer_cast<match_cv<byte, T>*>(ptr) + offset);
	}

	/// @}
}

// clang-format on
