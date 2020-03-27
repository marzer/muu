// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief Typedefs, macros and intrinsics used by all other muu headers.
#pragma once

/// \addtogroup		macros		Preprocessor macros
/// \brief Compiler feature detection, attributes, string-makers, etc.s
/// @{

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

// MUU_ASSERT

//=====================================================================================================================
// COMPILER, ARCHITECTURE & ENVIRONMENT
//=====================================================================================================================

#ifndef __cplusplus
	#error muu is a C++ library.
#endif
#if defined(__ia64__) || defined(__ia64) || defined(_IA64) || defined(__IA64__) || defined(_M_IA64) || defined(DOXYGEN)
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

/// \def MUU_ARCH_IA64
/// \brief `1` when targeting 64-bit Itanium, `0` otherwise.
///
/// \def MUU_ARCH_AMD64
/// \brief `1` when targeting AMD64, `0` otherwise.
///
/// \def MUU_ARCH_x86
/// \brief `1` when targeting 32-bit x86, `0` otherwise.
/// 
/// \def MUU_ARCH_BITNESS
/// \brief The 'bitness' of the current archtecture (e.g. `64` on AMD64).

#ifdef __clang__

	#define MUU_PRAGMA_CLANG(...)			_Pragma(__VA_ARGS__)
	#define MUU_PUSH_WARNINGS				_Pragma("clang diagnostic push")
	#define MUU_DISABLE_SWITCH_WARNINGS		_Pragma("clang diagnostic ignored \"-Wswitch\"")
	#define MUU_DISABLE_INIT_WARNINGS		_Pragma("clang diagnostic ignored \"-Wmissing-field-initializers\"")
	#define MUU_DISABLE_ALL_WARNINGS		_Pragma("clang diagnostic ignored \"-Weverything\"")
	#define MUU_POP_WARNINGS				_Pragma("clang diagnostic pop")
	#define MUU_ASSUME(cond)				__builtin_assume(cond)
	#define MUU_UNREACHABLE					__builtin_unreachable()
	#define MUU_GNU_ATTR(attr)				__attribute__((attr))
	#if defined(_MSC_VER) // msvc compat mode
		#ifdef __has_declspec_attribute
			#if __has_declspec_attribute(novtable)
				#define MUU_INTERFACE		__declspec(novtable)
			#endif
			#if __has_declspec_attribute(empty_bases)
				#define MUU_EMPTY_BASES		__declspec(empty_bases)
			#endif
			#if __has_declspec_attribute(restrict)
				#define MUU_UNALIASED_ALLOC	__declspec(restrict)
			#endif
			#define MUU_ALWAYS_INLINE		__forceinline
			#define MUU_NEVER_INLINE		__declspec(noinline)
			#define MUU_PRAGMA_MSVC(pragma)	__pragma(pragma)
			#define MUU_VECTORCALL			__vectorcall
		#endif
	#endif
	#ifdef __has_attribute
		#if !defined(MUU_ALWAYS_INLINE) && __has_attribute(always_inline)
			#define MUU_ALWAYS_INLINE		__attribute__((__always_inline__)) inline
		#endif
		#if !defined(MUU_NEVER_INLINE) && __has_attribute(noinline)
			#define MUU_NEVER_INLINE		__attribute__((__noinline__))
		#endif
		#if !defined(MUU_TRIVIAL_ABI) && __has_attribute(trivial_abi)
			#define MUU_TRIVIAL_ABI			__attribute__((__trivial_abi__))
		#endif
	#endif
	#ifdef __EXCEPTIONS
		#define MUU_EXCEPTIONS 1
	#else
		#define MUU_EXCEPTIONS 0
	#endif
	#if __has_feature(cxx_rtti)
		#define MUU_RTTI 1
	#else
		#define MUU_RTTI 0
	#endif

#elif defined(_MSC_VER) || (defined(__INTEL_COMPILER) && defined(__ICL))

	#define MUU_CPP_VERSION					_MSVC_LANG
	#define MUU_PRAGMA_MSVC(...)			__pragma(__VA_ARGS__)
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
	#define MUU_VECTORCALL					__vectorcall
	#ifdef _CPPUNWIND
		#define MUU_EXCEPTIONS 1
	#else
		#define MUU_EXCEPTIONS 0
	#endif
	#ifdef _CPPRTTI
		#define MUU_RTTI 1
	#else
		#define MUU_RTTI 0
	#endif

#elif defined(__GNUC__)

	#define MUU_PRAGMA_GCC(...)				_Pragma(__VA_ARGS__)
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
	#else
		#define MUU_EXCEPTIONS 0
	#endif
	#ifdef __GXX_RTTI
		#define MUU_RTTI 1
	#else
		#define MUU_RTTI 0
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
#ifndef MUU_RTTI
	#define MUU_RTTI 1
#endif
#ifndef MUU_PRAGMA_CLANG
	#define MUU_PRAGMA_CLANG(...)
#endif
#ifndef MUU_PRAGMA_MSVC
	#define MUU_PRAGMA_MSVC(...)
#endif
#ifndef MUU_PRAGMA_GCC
	#define MUU_PRAGMA_GCC(...)
#endif
#ifndef MUU_PUSH_WARNINGS
	#define MUU_PUSH_WARNINGS
#endif
#ifndef MUU_DISABLE_SWITCH_WARNINGS
	#define	MUU_DISABLE_SWITCH_WARNINGS
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
#ifdef MUU_VECTORCALL
	#define MUU_VECTORCALL_ARG
#else
	#define MUU_VECTORCALL
	#define MUU_VECTORCALL_ARG const&
#endif
#if defined(__cpp_consteval) && !defined(__INTELLISENSE__)
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

/// \def MUU_EXCEPTIONS
/// \brief `1` when support for C++ exceptions is enabled, `0` otherwise.
/// 
/// \def MUU_RTTI
/// \brief `1` when support for C++ run-time type identification (RTTI) is enabled, `0` otherwise.
/// 
/// \def MUU_CPP_VERSION
/// \brief The currently-targeted C++ standard. `17` for C++17, `20` for C++20, etc.
/// 
/// \def MUU_PRAGMA_CLANG(...)
/// \brief Expands to a `_Pragma()` directive when compiling with Clang.
/// 
/// \def MUU_PRAGMA_MSVC(...)
/// \brief Expands to a `_pragma()` directive when compiling with MSVC.
///
/// \def MUU_PRAGMA_GCC(...)
/// \brief Expands to a `_Pragma()` directive when compiling with GCC.
/// 
/// \def MUU_PUSH_WARNINGS
/// \brief Pushes the current compiler warning state onto the stack.
/// \detail Use this in tandem with the other warning macros to demarcate regions of code
/// 	that should have different warning semantics, e.g.: \cpp
/// 	MUU_PUSH_WARNINGS
/// 	MUU_DISABLE_SWITCH_WARNINGS
/// 	
/// 	int do_the_thing(my_enum val) noexcept
/// 	{
/// 		switch (val)
/// 		{
/// 			case my_enum::one: return 1;
/// 			case my_enum::two: return 2;
/// 			//case my_enum::three: return 3;
/// 		}
/// 		return 0;
/// 	}
/// 	
/// 	MUU_POP_WARNINGS
/// \ecpp
/// 
/// \def MUU_DISABLE_SWITCH_WARNINGS
/// \brief Disables compiler warnings relating to the use of switch statements.
/// \see MUU_PUSH_WARNINGS
///
/// \def MUU_DISABLE_INIT_WARNINGS
/// \brief Disables compiler warnings relating to variable initialization.
/// \see MUU_PUSH_WARNINGS
/// 
/// \def MUU_DISABLE_ALL_WARNINGS
/// \brief Disables ALL compiler warnings.
/// \see MUU_PUSH_WARNINGS
/// 
/// \def MUU_POP_WARNINGS
/// \brief Pops the current compiler warning state off the stack.
/// \see MUU_PUSH_WARNINGS
///
/// \def MUU_ASSUME(cond)
/// \brief Optimizer hint for signalling various assumptions about state at specific points in code.
/// \warning Using this incorrectly can lead to seriously mis-compiled code!
/// 
/// \def MUU_UNREACHABLE
/// \brief Marks a position in the code as being unreachable.
/// \warning Using this incorrectly can lead to seriously mis-compiled code!
/// 
/// \def MUU_NO_DEFAULT_CASE
/// \brief Marks a switch statement as not being in need of a default clause.
/// \warning Using this incorrectly can lead to seriously mis-compiled code!
/// \detail \cpp
/// 	enum class my_enum
/// 	{
/// 		one,
/// 		two,
/// 		three
/// 	};
/// 	
/// 	int do_the_thing(my_enum val) noexcept
/// 	{
/// 		switch (val)
/// 		{
/// 			case my_enum::one: return 1;
/// 			case my_enum::two: return 2;
/// 			MUU_NO_DEFAULT_CASE;
/// 		}
/// 	}
/// 	
/// 	// obviously makes sense only if do_the_thing()
/// 	// is never called with any other values
/// \ecpp
/// 
/// \def MUU_INTERFACE
/// \brief Marks a class being interface-only and not requiring a vtable.
/// \detail Useful for abstract base classes:\cpp
/// 	class MUU_INTERFACE virtual_base
/// 	{
/// 		virtual void fooify() noexcept = 0;
/// 		virtual ~virtual_base() noexcept = default;
/// 	};
/// \ecpp
/// 
/// \def MUU_EMPTY_BASES
/// \brief Marks a class as having only empty base classes.
/// \detail This is required for some compilers to use Empty Base Class Optimization:\cpp
/// 	class empty_parent
/// 	{
/// 		using value_type = int;
/// 	};
/// 	
/// 	class MUU_EMPTY_BASES nonempty_child : empty_parent
/// 	{
/// 		using empty_parent::value_type;
/// 		
/// 		value_type value;
/// 	};
/// 	
/// 	static_assert(sizeof(nonempty_child) == sizeof(nonempty_child::value_type));
/// \ecpp
/// \see [__declspec(empty_bases)](https://devblogs.microsoft.com/cppblog/optimizing-the-layout-of-empty-base-classes-in-vs2015-update-2-3/)
/// 
/// \def MUU_UNALIASED_ALLOC
/// \brief Optimizer hint that marks an allocating function's pointer return value as being free from aliasing.
/// \see [__declspec(restrict)](https://docs.microsoft.com/en-us/cpp/cpp/restrict?view=vs-2019)
/// 
/// \def MUU_ALWAYS_INLINE
/// \brief The same linkage semantics as the `inline` keyword, with an additional hint to
/// 	   the optimizer that you'd really, _really_ like a function inlined.
/// \detail \cpp
/// 	MUU_ALWAYS_INLINE
/// 	void always_inline_me() noexcept
/// 	{
/// 		// ...
/// 	}
/// \ecpp
///
/// \def MUU_NEVER_INLINE
/// \brief A strong hint to the optimizer that you really, _really_ do not want a function inlined.
/// \detail \cpp
/// 	MUU_NEVER_INLINE
/// 	int never_inline_me() noexcept
/// 	{
/// 		// ...
/// 	}
/// \ecpp
///
/// \def MUU_TRIVIAL_ABI
/// \brief Marks a simple type which might otherwise be considered non-trivial
/// 	   as being trivial over ABI boundaries.
/// \detail This is useful for simple wrappers and 'strong typedef' types, e.g.:  \cpp
/// 	template <typename T>
/// 	struct MUU_TRIVIAL_ABI strong_typedef
/// 	{
/// 		T value;
/// 		~strong_typedef() noexcept
/// 		{
/// 			// non-trivial for some reason, maybe debug checks
/// 		}
/// 	}
/// \ecpp
/// \see [\[\[trivial_abi\]\]](https://quuxplusone.github.io/blog/2018/05/02/trivial-abi-101/)
///
/// \def MUU_LIKELY
/// \brief Expands to C++20's `[[likely]]` if supported by your compiler.
///
/// \def MUU_UNLIKELY
/// \brief Expands to C++20's `[[unlikely]]` if supported by your compiler.
///
/// \def MUU_NO_UNIQUE_ADDRESS
/// \brief Expands to C++20's `[[no_unique_address]]` if supported by your compiler.
/// 
/// \def MUU_NODISCARD_CTOR
/// \brief Expands to `[[nodiscard]]` if your compiler supports it on constructors.
/// \detail This is useful for RAII helper types like locks:\cpp
/// 	class raii_lock
/// 	{
/// 		MUU_NODISCARD_CTOR
/// 		raii_lock() noexcept
/// 		{
///				super_special_global_lock();
/// 		}
/// 		
/// 		~raii_lock() noexcept
/// 		{
///				super_special_global_unlock();
/// 		}
/// 	};
/// 	
/// 	int whoops() noexcept
/// 	{
/// 		raii_lock{}; // whoops! forgot to give a name; would be destroyed immediately.
/// 		             // MUU_NODISCARD_CTOR makes the compiler emit a warning here.
///
/// 		something_that_requires_the_lock();
/// 	}
/// \ecpp
/// 
/// \def MUU_CONSTEVAL
/// \brief Expands to C++20's `consteval` if supported by your compiler, or `constexpr` otherwise.
/// \see [consteval](https://en.cppreference.com/w/cpp/language/consteval)
/// 
/// \def MUU_VECTORCALL
/// \brief Expands to `__vectorcall` on compilers that support it.
/// \see [__vectorcall](https://docs.microsoft.com/en-us/cpp/cpp/vectorcall?view=vs-2019)
/// 
/// \def MUU_MAKE_STRING(str)
/// \brief Stringifies the input, converting it verbatim into a raw string literal.
/// \detail \cpp
/// // these are equivalent:
///	constexpr auto str1 = MUU_MAKE_STRING("It's trap!", he bellowed.);
///	constexpr auto str2 = "\"It's trap!\", he bellowed.";
/// \ecpp
/// \see [String literals in C++](https://en.cppreference.com/w/cpp/language/string_literal)
/// 
/// \def MUU_MAKE_STRING_VIEW(str)
/// \brief Stringifies the input, converting it verbatim into a string view literal.
/// \detail \cpp
/// // these are equivalent:
///	constexpr std::string_view str1 = MUU_MAKE_STRING_VIEW("It's trap!", he bellowed.);
///	constexpr std::string_view str2 = "\"It's trap!\", he bellowed."sv;
/// \ecpp
/// \see [String literals in C++](https://en.cppreference.com/w/cpp/language/string_literal)


/// @}

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
#ifndef MUU_ASSERT
	#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
		#include <cassert>
		#define MUU_ASSERT(expr)			assert(expr)
	#else
		#define MUU_ASSERT(expr)			(void)0
	#endif
#endif
#if __has_include(<version>)
	#include <version>
#endif

MUU_POP_WARNINGS

////////// FORWARD DECLARATIONS

/// \brief	The root namespace for all muu functions and types.
namespace muu
{
	using size_t = std::size_t;
	using intptr_t = std::intptr_t;
	using uintptr_t = std::uintptr_t;
	using ptrdiff_t = std::ptrdiff_t;
	using nullptr_t = std::nullptr_t;

	#ifndef DOXYGEN

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
	template <> struct constify_pointer<const volatile void*> { using type = const volatile void*; };
	template <> struct constify_pointer<volatile void*> { using type = const volatile void*; };
	template <> struct constify_pointer<const void*> { using type = const void*; };
	template <> struct constify_pointer<void*> { using type = const void*; };

	template <typename T, typename U>
	struct rebase_pointer;
	template <typename T, typename U>
	struct rebase_pointer<const volatile T*, U> { using type = std::add_const_t<std::add_volatile_t<U>>*; };
	template <typename T, typename U>
	struct rebase_pointer<volatile T*, U>		{ using type = std::add_volatile_t<U>*; };
	template <typename T, typename U>
	struct rebase_pointer<const T*, U>			{ using type = std::add_const_t<U>*; };
	template <typename T, typename U>
	struct rebase_pointer<T*, U>				{ using type = U*; };

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
}

namespace muu
{
	/// \addtogroup		traits		Type traits and metafunctions
	/// @{
	
	/// \brief	Gets the underlying integer of an enum type.
	/// \remarks This is safe to use on non-enums and in SFINAE contexts,
	/// 		 unlike std::underlying_type_t.
	template <typename T>
	using underlying_type = typename impl::underlying_type<T>::type;

	#if defined(__cpp_lib_remove_cvref) || (defined(_MSC_VER) && defined(_HAS_CXX20) && _HAS_CXX20) || defined(DOXYGEN)
		/// \brief	Removes the topmost const, volatile and reference qualifiers from a type.
		/// \remarks This is equivalent to C++20's std::remove_cvref_t.
		template <typename T>
		using remove_cvref = std::remove_cvref_t<T>;
	#else
		template <typename T>
		using remove_cvref = std::remove_cv_t<std::remove_reference_t<T>>;
	#endif

	/// \brief The largest type from a set of types.
	template <typename... T>
	using largest = typename impl::largest<T...>::type;

	/// \brief The smallest type from a set of types.
	template <typename... T>
	using smallest = typename impl::smallest<T...>::type;

	/// \brief The default alignment of a type.
	/// \remarks This returns an alignment of `1` for `void` and functions.
	template <typename T>
	inline constexpr size_t alignment_of = impl::alignment_of<remove_cvref<T>>::value;

	/// \brief The type with the largest alignment (i.e. having the largest value for `alignof(T)`) from a set of types.
	/// \remarks `void` and functions are considered as having an alignment of `1`.
	template <typename... T>
	using most_aligned = typename impl::most_aligned<T...>::type;

	/// \brief The type with the smallest alignment (i.e. having the smallest value for `alignof(T)`) from a set of types.
	/// \remarks `void` and functions are considered as having an alignment of `1`.
	template <typename... T>
	using least_aligned = typename impl::least_aligned<T...>::type;

	/// \brief	True if T is exactly the same as one or more of the types named by U.
	template <typename T, typename... U>
	inline constexpr bool is_one_of = impl::is_one_of<T, U...>::value;

	/// \brief Is a type unsigned or reference-to-unsigned?
	template <typename T>
	inline constexpr bool is_unsigned = std::is_unsigned_v<underlying_type<std::remove_reference_t<T>>>;

	/// \brief Is a type signed or reference-to-signed?
	/// \remarks This will also return true for muu::float16.
	template <typename T>
	inline constexpr bool is_signed = std::is_signed_v<underlying_type<std::remove_reference_t<T>>>
		|| std::is_same_v<remove_cvref<T>, float16>;

	/// \brief Is a type a floating-point or reference-to-floating-point?
	/// \remarks This will also return true for muu::float16.
	template <typename T>
	inline constexpr bool is_floating_point = std::is_floating_point_v<std::remove_reference_t<T>>
		|| std::is_same_v<remove_cvref<T>, float16>;

	/// \brief Is a type arithmetic or reference-to-arithmetic?
	/// \remarks This will also return true for muu::float16.
	template <typename T>
	inline constexpr bool is_arithmetic = std::is_arithmetic_v<std::remove_reference_t<T>>
		|| std::is_same_v<remove_cvref<T>, float16>;

	/// \brief Is a type const or reference-to-const?
	template <typename T>
	inline constexpr bool is_const = std::is_const_v<std::remove_reference_t<T>>;

	/// \brief Removes the topmost const qualifier from a type or reference.
	template <typename T>
	using remove_const = typename impl::remove_const<T>::type;

	/// \brief Shorthand for conditionally adding const to a type or reference.
	template <typename T, bool cond>
	using conditionally_add_const = typename impl::conditionally_add_const<T, cond>::type;

	/// \brief Shorthand for marking a type or reference with the same constness as another type or reference.
	template <typename T, typename match_with>
	using match_const = conditionally_add_const<remove_const<T>, is_const<match_with>>;

	/// \brief Is a type volatile or reference-to-volatile?
	template <typename T>
	inline constexpr bool is_volatile = std::is_volatile_v<std::remove_reference_t<T>>;

	/// \brief Removes the topmost volatile qualifier from a type or reference.
	template <typename T>
	using remove_volatile = typename impl::remove_volatile<T>::type;

	/// \brief Shorthand for conditionally adding volatile to a type or reference.
	template <typename T, bool cond>
	using conditionally_add_volatile = typename impl::conditionally_add_volatile<T, cond>::type;

	/// \brief Shorthand for marking a type or reference with the same volatility as another type or reference.
	template <typename T, typename match_with>
	using match_volatile = conditionally_add_volatile<remove_volatile<T>, is_volatile<match_with>>;

	/// \brief Removes the topmost const and volatile qualifiers from a type or reference.
	template <typename T>
	using remove_cv = remove_const<remove_volatile<T>>;

	/// \brief Shorthand for marking a type or reference with the same constness and volatility
	/// 	   as another type or reference.
	template <typename T, typename match_with>
	using match_cv = match_const<match_volatile<T, match_with>, match_with>;

	/// \brief Is a type const, volatile, or a reference?
	template <typename T>
	inline constexpr bool is_cvref = std::is_const_v<T> || std::is_volatile_v<T> || std::is_reference_v<T>;

	/// \brief Removes any `noexcept` modifier from a functional type.
	template <typename T>
	using remove_noexcept = typename impl::remove_noexcept<T>::type;

	/// \brief Does CHILD inherit from PARENT?
	/// \remarks This does not return true when the objects are the same type, unlike std::is_base_of.
	template <typename CHILD, typename PARENT>
	inline constexpr bool inherits_from
		= std::is_base_of_v<remove_cvref<PARENT>, remove_cvref<CHILD>>
		&& !std::is_same_v<remove_cvref<PARENT>, remove_cvref<CHILD>>;

	/// \brief	Adds const to the pointed-to type of a pointer, correctly handling pointers-to-void.
	template <typename T>
	using constify_pointer = typename impl::constify_pointer<T>::type;

	/// \brief	Rebases a pointer, preserving the const and volatile qualification of the pointed type.
	template <typename PTR, typename BASE>
	using rebase_pointer = typename impl::rebase_pointer<PTR, BASE>::type;

	/// @}
}

//=====================================================================================================================
// LITERALS, BUILD CONSTANTS AND 'INTRINSICS'
//=====================================================================================================================

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
		static constexpr bool has_exceptions = !!MUU_EXCEPTIONS;

		/// \brief True if run-time type identification (RTTI) is enabled.
		static constexpr bool has_rtti = !!MUU_RTTI;
	
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

	/// \brief	'Unboxes' an enum to it's raw integer equivalent.
	///
	/// \tparam	T		An integral or enum type.
	/// \param 	val		The value to unbox
	///
	/// \returns	`static_cast<std::underlying_type_t<T>>(val)`
	template <typename T, typename = std::enable_if_t<
		std::is_enum_v<T>
		|| std::is_integral_v<T>
	>>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr auto unbox(T val) noexcept
	{
		if constexpr (std::is_enum_v<T>)
			return static_cast<std::underlying_type_t<T>>(val);
		else
			return val;
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
		if constexpr (std::is_enum_v<T>)
			return has_single_bit(unbox(val));
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
		if constexpr (std::is_enum_v<T>)
			return static_cast<T>(countl_zero(unbox(val)));
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
		if constexpr (std::is_enum_v<T>)
			return static_cast<T>(countr_zero(unbox(val)));
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
		if constexpr (std::is_enum_v<T>)
			return static_cast<T>(bit_ceil(unbox(val)));
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
		if constexpr (std::is_enum_v<T>)
			return static_cast<T>(bit_floor(unbox(val)));
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
		if constexpr (std::is_enum_v<T>)
			return static_cast<T>(bit_width(unbox(val)));
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
		if constexpr (std::is_enum_v<T>)
			return T{ bit_fill_right<underlying_type<T>>(count) };
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
		if constexpr (std::is_enum_v<T>)
			return T{ bit_fill_left<underlying_type<T>>(count) };
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
				#undef MUU_HAS_CONSTEXPR_BIT_CAST
				#define MUU_HAS_CONSTEXPR_BIT_CAST 0
			#endif
		#elif defined(__GNUC__) && __GNUC__ >= 10 // ??
			return __builtin_bit_cast(TO, from);
		#elif defined(_MSC_VER) && _MSC_VER >= 1926 // Visual Studio 2019 version 16.6
			return __builtin_bit_cast(TO, from);
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
	constexpr bool is_between(const T& val, const T& min, const T& max) noexcept
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
	/// \tparam TO	A pointer or integer large enough to store a pointer
	/// \tparam FROM A pointer, array, nullptr_t, or an integer large enough to store a pointer.
	/// \param from The value being cast.
	/// 
	/// \return The input casted to the desired type.
	/// 
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
			!(!std::is_same_v<FROM, remove_cv<nullptr_t>> && std::is_same_v<remove_cv<nullptr_t>, TO>),
			"TO type cannot be nullptr_t (such a conversion is nonsensical)"
		);
		static_assert(
			std::is_pointer_v<FROM>
			|| std::is_same_v<FROM, remove_cv<nullptr_t>>
			|| (std::is_integral_v<FROM> && sizeof(FROM) >= sizeof(void*)),
			"FROM type must be a pointer, array, nullptr_t, or an integer large enough to store a pointer"
		);
		static_assert(
			!std::is_member_pointer_v<TO> && !std::is_member_pointer_v<FROM>,
			"FROM and TO types cannot be pointers to members"
		);

		using from_base = std::remove_pointer_t<FROM>;
		using to_base = std::remove_pointer_t<TO>;

		static_assert(
			(!std::is_function_v<from_base> || sizeof(FROM) == sizeof(void*))
			&& (!std::is_function_v<to_base> || sizeof(TO) == sizeof(void*)),
			"Cannot pointer_cast with function pointers on this platform"
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
					return reinterpret_cast<TO>(from);

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
				// function -> *
				if constexpr (std::is_function_v<from_base> && !std::is_function_v<to_base>)
				{
					static_assert(
						std::is_void_v<to_base>,
						"Cannot cast from a function pointer to a type other than void"
					);

					// function -> void
					return static_cast<TO>(reinterpret_cast<void*>(from));
				}

				// * -> function
				else if constexpr (!std::is_function_v<from_base> && std::is_function_v<to_base>)
				{
					static_assert(
						std::is_void_v<from_base>,
						"Cannot cast to a function pointer from a type other than void"
					);

					// void -> function
					return reinterpret_cast<TO>(pointer_cast<void*>(from));
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
						return reinterpret_cast<TO>(from);

					// function (noexcept) -> function
					else
						return static_cast<TO>(from);
				}

				// void -> void (different cv)
				else if constexpr (std::is_void_v<from_base> && std::is_void_v<to_base>)
				{
					// remove const/volatile
					if constexpr (is_one_of<FROM, const void*, const volatile void*, volatile void*>)
						return static_cast<TO>(const_cast<void*>(from));

					// add const/volatile
					else
					{
						static_assert(std::is_same_v<FROM, void*>);
						static_assert(is_one_of<TO, const void*, const volatile void*, volatile void*>);
						return static_cast<TO>(from);
					}
				}

				// void -> *
				// * -> void
				// derived -> base
				else if constexpr (std::is_void_v<from_base> || std::is_void_v<to_base> || inherits_from<from_base, to_base>)
					return pointer_cast<TO>(static_cast<rebase_pointer<FROM, remove_cv<to_base>>>(from));

				// base -> derived
				else if constexpr (inherits_from<to_base, from_base>)
					return pointer_cast<TO>(dynamic_cast<rebase_pointer<FROM, remove_cv<to_base>>>(from));

				// A -> B (unrelated types)
				else if constexpr (!std::is_same_v<remove_cv<from_base>, remove_cv<to_base>>)
					return pointer_cast<TO>(reinterpret_cast<rebase_pointer<FROM, remove_cv<to_base>>>(from));

				// A -> A (different cv)
				else
				{
					static_assert(!std::is_same_v<from_base, to_base>);
					static_assert(std::is_same_v<remove_cv<from_base>, remove_cv<to_base>>);

					// remove const/volatile
					if constexpr (is_const<from_base> || is_volatile<from_base>)
						return static_cast<TO>(const_cast<remove_cv<from_base>*>(from));

					// add const/volatile
					else
					{
						static_assert(std::is_same_v<FROM, remove_cv<from_base>*>);
						static_assert(is_one_of<TO, const to_base*, const volatile to_base*, volatile to_base*>);
						return static_cast<TO>(from);
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

	/// \brief	Checks if a value is infinity or NaN.
	///
	/// \tparam	T	The value type.
	/// \param 	val	The value to examine.
	///
	/// \returns	True if the value is floating-point infinity or NaN.
	/// 			Always returns false if the value was not a floating-point type.
	template <typename T>
	[[nodiscard]] MUU_ALWAYS_INLINE
	constexpr bool MUU_VECTORCALL is_infinity_or_nan(T val) noexcept
	{
		if constexpr (std::is_floating_point_v<T>) //not muu::is_floating_point; TINAE
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

	/// @}
}

// clang-format on
