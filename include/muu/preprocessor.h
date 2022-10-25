// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Compiler feature detection, attributes, string-makers, etc.

/// \defgroup	preprocessor		Preprocessor
/// \brief		Compiler feature detection, attributes, string-makers, etc.
/// @{

//======================================================================================================================
// LIBRARY VERSION
//======================================================================================================================

#define MUU_VERSION_MAJOR 0
#define MUU_VERSION_MINOR 1
#define MUU_VERSION_PATCH 0

//======================================================================================================================
// C++ VERSION
//======================================================================================================================

#ifndef __cplusplus
	#error muu is a C++ library.
#endif
#ifdef _MSVC_LANG
	#if _MSVC_LANG > __cplusplus
		#define MUU_CPP _MSVC_LANG
	#endif
#endif
#ifndef MUU_CPP
	#define MUU_CPP __cplusplus
#endif
#if MUU_CPP >= 202600L
	#undef MUU_CPP
	#define MUU_CPP 26
#elif MUU_CPP >= 202300L
	#undef MUU_CPP
	#define MUU_CPP 23
#elif MUU_CPP >= 202002L
	#undef MUU_CPP
	#define MUU_CPP 20
#elif MUU_CPP >= 201703L
	#undef MUU_CPP
	#define MUU_CPP 17
#else
	#error muu requires C++17 or higher.
#endif
/// \def MUU_CPP
/// \brief The currently-targeted C++ standard. `17` for C++17, `20` for C++20, etc.

//======================================================================================================================
// COMPILER DETECTION
//======================================================================================================================

#ifdef __INTELLISENSE__
	#define MUU_INTELLISENSE 1
#else
	#define MUU_INTELLISENSE 0
#endif
/// \def MUU_INTELLISENSE
/// \brief `1` when the code being compiled by an IDE's 'intellisense' compiler, otherwise `0`.

#if defined(DOXYGEN) || defined(__DOXYGEN) || defined(__DOXYGEN__) || defined(__doxygen__) || defined(__POXY__)        \
	|| defined(__poxy__)
	#define MUU_DOXYGEN 1
#else
	#define MUU_DOXYGEN 0
#endif
/// \def MUU_DOXYGEN
/// \brief `1` when the code being interpreted by Doygen or some other documentation generator, otherwise `0`.

#ifdef __clang__
	#define MUU_CLANG __clang_major__
#else
	#define MUU_CLANG 0
#endif
/// \def MUU_CLANG
/// \brief The value of `__clang_major__` when the code is being compiled by LLVM/Clang, otherwise `0`.
/// \see https://sourceforge.net/p/predef/wiki/Compilers/

#ifdef __INTEL_COMPILER
	#define MUU_ICC __INTEL_COMPILER
	#ifdef __ICL
		#define MUU_ICC_CL MUU_ICC
	#else
		#define MUU_ICC_CL 0
	#endif
#else
	#define MUU_ICC	   0
	#define MUU_ICC_CL 0
#endif
/// \def MUU_ICC
/// \brief The value of `__INTEL_COMPILER` when the code is being compiled by ICC, otherwise `0`.
/// \see http://scv.bu.edu/computation/bladecenter/manpages/icc.html

#ifdef _MSC_VER
	#define MUU_MSVC_LIKE _MSC_VER
#else
	#define MUU_MSVC_LIKE 0
#endif
/// \def MUU_MSVC_LIKE
/// \brief The value of `_MSC_VER` when it is defined by the compiler, otherwise `0`.
/// \see https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros

#if MUU_MSVC_LIKE && !MUU_CLANG && !MUU_ICC
	#define MUU_MSVC MUU_MSVC_LIKE
#else
	#define MUU_MSVC 0
#endif
/// \def MUU_MSVC
/// \brief The value of `_MSC_VER` when the code is being compiled by MSVC specifically, otherwise `0`.
/// \see https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros

#ifdef __GNUC__
	#define MUU_GCC_LIKE __GNUC__
#else
	#define MUU_GCC_LIKE 0
#endif
/// \def MUU_GCC_LIKE
/// \brief The value of `__GNUC__` when it is defined by the compiler, otherwise `0`.
/// \see https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html

#if MUU_GCC_LIKE && !MUU_CLANG && !MUU_ICC
	#define MUU_GCC MUU_GCC_LIKE
#else
	#define MUU_GCC 0
#endif
/// \def MUU_GCC
/// \brief The value of `__GNUC__` when the code is being compiled by GCC specifically, otherwise `0`.
/// \see https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html

/// \cond
#if !MUU_CLANG && !MUU_ICC && !MUU_MSVC && !MUU_GCC
	#error Unknown compiler! Please file an issue at https://github.com/marzer/muu/issues to help add support.
#endif
#if (!!MUU_CLANG + !!MUU_ICC + !!MUU_MSVC + !!MUU_GCC) > 1
	#error Could not uniquely identify compiler. Please file an issue at https://github.com/marzer/muu/issues.
#endif
/// \endcond

//======================================================================================================================
// ARCHITECTURE & ENVIRONMENT
//======================================================================================================================

#if defined(__ia64__) || defined(__ia64) || defined(_IA64) || defined(__IA64__) || defined(_M_IA64)
	#define MUU_ARCH_ITANIUM 1
	#define MUU_ARCH_BITNESS 64
#else
	#define MUU_ARCH_ITANIUM 0
#endif
/// \def MUU_ARCH_ITANIUM
/// \brief `1` when targeting 64-bit Itanium, otherwise `0`.

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_AMD64)
	#define MUU_ARCH_AMD64	 1
	#define MUU_ARCH_BITNESS 64
#else
	#define MUU_ARCH_AMD64 0
#endif
/// \def MUU_ARCH_AMD64
/// \brief `1` when targeting AMD64, otherwise `0`.

#if defined(__i386__) || defined(_M_IX86)
	#define MUU_ARCH_X86	 1
	#define MUU_ARCH_BITNESS 32
#else
	#define MUU_ARCH_X86 0
#endif
/// \def MUU_ARCH_X86
/// \brief `1` when targeting 32-bit x86, otherwise `0`.

#if defined(__aarch64__) || defined(__ARM_ARCH_ISA_A64) || defined(_M_ARM64) || defined(__ARM_64BIT_STATE)             \
	|| defined(_M_ARM64EC)
	#define MUU_ARCH_ARM32	 0
	#define MUU_ARCH_ARM64	 1
	#define MUU_ARCH_ARM	 1
	#define MUU_ARCH_BITNESS 64
#elif defined(__arm__) || defined(_M_ARM) || defined(__ARM_32BIT_STATE)
	#define MUU_ARCH_ARM32	 1
	#define MUU_ARCH_ARM64	 0
	#define MUU_ARCH_ARM	 1
	#define MUU_ARCH_BITNESS 32
#else
	#define MUU_ARCH_ARM32 0
	#define MUU_ARCH_ARM64 0
	#define MUU_ARCH_ARM   0
#endif
/// \def MUU_ARCH_ARM32
/// \brief `1` when targeting 32-bit ARM, otherwise `0`.
///
/// \def MUU_ARCH_ARM64
/// \brief `1` when targeting 64-bit ARM, otherwise `0`.
///
/// \def MUU_ARCH_ARM
/// \brief `1` when targeting any flavour of ARM, otherwise `0`.

#if MUU_ARCH_BITNESS == 64
	#define MUU_ARCH_X64 1
#else
	#define MUU_ARCH_X64 0
#endif
/// \def MUU_ARCH_X64
/// \brief `1` when targeting any 64-bit architecture, otherwise `0`.

#ifndef MUU_ARCH_BITNESS
	#define MUU_ARCH_BITNESS 0
#endif
/// \def MUU_ARCH_BITNESS
/// \brief The 'bitness' of the target architecture (e.g. `64` on AMD64).
/// \see build::bitness

/// \cond
#ifndef MUU_DISABLE_ENVIRONMENT_CHECKS
	#define MUU_ARCH_SUM (MUU_ARCH_ITANIUM + MUU_ARCH_AMD64 + MUU_ARCH_X86 + MUU_ARCH_ARM32 + MUU_ARCH_ARM64)
	#if MUU_ARCH_SUM != 1 || !MUU_ARCH_BITNESS
		#error If you are seeing this error it is because you are building muu for an architecture that could not be uniquely \
identified by the preprocessor machinery. You can try disabling the checks by defining MUU_DISABLE_ENVIRONMENT_CHECKS, \
but libary functionality mileage may vary. Please consider filing an issue at github.com/marzer/muu/issues to \
help me improve support for your target architecture. Thanks!
	#endif
	#undef MUU_ARCH_SUM
#endif // MUU_DISABLE_ENVIRONMENT_CHECKS
/// \endcond

#ifdef _WIN32
	#define MUU_WINDOWS 1
#else
	#define MUU_WINDOWS 0
#endif
#ifdef __unix__
	#define MUU_UNIX 1
#else
	#define MUU_UNIX 0
#endif
#ifdef __linux__
	#define MUU_LINUX 1
#else
	#define MUU_LINUX 0
#endif
/// \def MUU_WINDOWS
/// \brief `1` when building for the Windows operating system, otherwise `0`.
///
/// \def MUU_UNIX
/// \brief `1` when building for a GNU/Unix variant, otherwise `0`.
///
/// \def MUU_LINUX
/// \brief `1` when building for a Linux distro, otherwise `0`.

#if defined(__MMX__) || MUU_MSVC
	#define MUU_ISET_MMX 1
#else
	#define MUU_ISET_MMX 0
#endif
#if defined(__SSE__) || (MUU_MSVC && (defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 1)))
	#define MUU_ISET_SSE 1
#else
	#define MUU_ISET_SSE 0
#endif
#if defined(__SSE2__) || (MUU_MSVC && (defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)))
	#define MUU_ISET_SSE2 1
#else
	#define MUU_ISET_SSE2 0
#endif
#if defined(__AVX__)
	#define MUU_ISET_AVX 1
#else
	#define MUU_ISET_AVX 0
#endif
#if defined(__AVX2__)
	#define MUU_ISET_AVX2 1
#else
	#define MUU_ISET_AVX2 0
#endif
// clang-format off
#if defined(__AVX512BW__)		\
	|| defined(__AVX512CD__)	\
	|| defined(__AVX512DQ__)	\
	|| defined(__AVX512ER__)	\
	|| defined(__AVX512F__)		\
	|| defined(__AVX512PF__)	\
	|| defined(__AVX512VL__)
	#define MUU_ISET_AVX512 1
#else
	#define MUU_ISET_AVX512 0
#endif
// clang-format on
/// \def MUU_ISET_MMX
/// \brief `1` when the target supports the MMX instruction set, otherwise `0`.
///
/// \def MUU_ISET_SSE
/// \brief `1` when the target supports the SSE instruction set, otherwise `0`.
///
/// \def MUU_ISET_SSE2
/// \brief `1` when the target supports the SSE2 instruction set, otherwise `0`.
///
/// \def MUU_ISET_AVX
/// \brief `1` when the target supports the AVX instruction set, otherwise `0`.
///
/// \def MUU_ISET_AVX2
/// \brief `1` when the target supports the AVX2 instruction set, otherwise `0`.
///
/// \def MUU_ISET_AVX512
/// \brief `1` when the target supports any of the AVX512 instruction sets, otherwise `0`.

#if MUU_MSVC_LIKE || MUU_DOXYGEN
	#define MUU_LITTLE_ENDIAN 1
	#define MUU_BIG_ENDIAN	  0
#elif defined(__BYTE_ORDER__)
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		#define MUU_LITTLE_ENDIAN 1
		#define MUU_BIG_ENDIAN	  0
	#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		#define MUU_LITTLE_ENDIAN 0
		#define MUU_BIG_ENDIAN	  1
	#endif
#else
	#error Unknown platform endianness.
#endif
/// \def MUU_LITTLE_ENDIAN
/// \brief `1` when the target environment is little-endian, otherwise `0`.
/// \see build::is_little_endian
///
/// \def MUU_BIG_ENDIAN
/// \brief `1` when the target environment is big-endian, otherwise `0`.
/// \see build::is_big_endian

//======================================================================================================================
// COMPILER FEATURE DETECTION
//======================================================================================================================

#ifdef __has_include
	#define MUU_HAS_INCLUDE(header)		__has_include(header)
#else
	#define MUU_HAS_INCLUDE(header) 0
#endif
/// \def MUU_HAS_INCLUDE(header)
/// \brief The result of `__has_include(header)` if supported by the compiler, otherwise `0`.

#ifdef __has_builtin
	#define MUU_HAS_BUILTIN(name) __has_builtin(name)
#else
	#define MUU_HAS_BUILTIN(name) 0
#endif
/// \def MUU_HAS_BUILTIN(name)
/// \brief The result of `__has_builtin(name)` if supported by the compiler, otherwise `0`.

#ifdef __has_feature
	#define MUU_HAS_FEATURE(name) __has_feature(name)
#else
	#define MUU_HAS_FEATURE(name) 0
#endif
/// \def MUU_HAS_FEATURE(name)
/// \brief The result of `__has_feature(name)` if supported by the compiler, otherwise `0`.

#ifdef __has_attribute
	#define MUU_HAS_ATTR(attr) __has_attribute(attr)
#else
	#define MUU_HAS_ATTR(attr) 0
#endif
/// \def MUU_HAS_ATTR(attr)
/// \brief The result of `__has_attribute(attr)` if supported by the compiler, otherwise `0`.

#ifdef __has_cpp_attribute
	#define MUU_HAS_CPP_ATTR(attr) __has_cpp_attribute(attr)
#else
	#define MUU_HAS_CPP_ATTR(attr) 0
#endif
/// \def MUU_HAS_CPP_ATTR(attr)
/// \brief The result of `__has_cpp_attribute(attr)` if supported by the compiler, otherwise `0`.

#if defined(__EXCEPTIONS) || defined(_CPPUNWIND) || defined(__cpp_exceptions)
	#define MUU_HAS_EXCEPTIONS 1
#else
	#define MUU_HAS_EXCEPTIONS 0
#endif
/// \def MUU_HAS_EXCEPTIONS
/// \brief `1` when C++ exceptions are supported and enabled, otherwise `0`.
/// \see build::supports_exceptions

#if defined(_CPPRTTI) || defined(__GXX_RTTI) || MUU_HAS_FEATURE(cxx_rtti)
	#define MUU_HAS_RTTI 1
#else
	#define MUU_HAS_RTTI 0
#endif
/// \def MUU_HAS_RTTI
/// \brief `1` when C++ run-time type identification (RTTI) is supported and enabled, otherwise `0`.
/// \see build::supports_rtti

#if defined(__SIZEOF_INT128__)
	#define MUU_HAS_INT128 1
#else
	#define MUU_HAS_INT128 MUU_DOXYGEN
#endif
/// \def MUU_HAS_INT128
/// \brief `1` when the target environment has 128-bit integers, otherwise `0`.
/// \see
/// 	- #muu::int128_t
/// 	- #muu::uint128_t

#if defined(__cpp_char8_t) && __cpp_char8_t >= 201811
	#define MUU_HAS_CHAR8 1
#else
	#define MUU_HAS_CHAR8 MUU_DOXYGEN
#endif
/// \def MUU_HAS_CHAR8
/// \brief `1` when the compiler supports C++20's char8_t, otherwise `0`.

#if defined(__cpp_consteval) && __cpp_consteval >= 201811 && (!MUU_MSVC || MUU_MSVC >= 1934)

// Q: "why !MUU_MSVC?"
//
// A: consteval is basically fucked in msvc:
// https://developercommunity.visualstudio.com/t/Erroneous-C7595-error-with-consteval-in/1404234
// https://developercommunity.visualstudio.com/t/using-consteval-rvalues-results-in-compi/10083735
// https://developercommunity.visualstudio.com/t/consteval-conversion-function-fails/1579014
// https://developercommunity.visualstudio.com/t/Function-call-expression-of-consteval-fu/1557055

	#define MUU_HAS_CONSTEVAL 1
#else
	#define MUU_HAS_CONSTEVAL MUU_DOXYGEN
#endif
/// \def MUU_HAS_CONSTEVAL
/// \brief `1` when the compiler supports C++20's `consteval` immediate functions, otherwise `0`.

#if defined(__cpp_if_consteval) && __cpp_if_consteval >= 202106
	#define MUU_HAS_CONSTEVAL_IF 1
#else
	#define MUU_HAS_CONSTEVAL_IF 0
#endif
/// \def MUU_HAS_CONSTEVAL_IF
/// \brief `1` when the compiler supports C++23's `if consteval { }`, otherwise `0`.

//======================================================================================================================
// EXPORT VISIBILITY
//======================================================================================================================

#if !MUU_MSVC_LIKE
	#undef MUU_DLL
#endif
#ifndef MUU_DLL
	#if MUU_MSVC_LIKE
		#define MUU_DLL 1
	#else
		#define MUU_DLL 0
	#endif
#endif

#ifndef MUU_BUILDING
	#define MUU_BUILDING 0
#endif

#ifndef MUU_API
	#if MUU_MSVC_LIKE && MUU_DLL
		#if MUU_BUILDING
			#define MUU_API __declspec(dllexport)
		#else
			#define MUU_API __declspec(dllimport)
		#endif
	#elif MUU_CLANG || MUU_GCC
		#define MUU_API __attribute__((visibility("default")))
	#else
		#define MUU_API
	#endif
#endif

#if MUU_WINDOWS
	#define MUU_CALLCONV __cdecl
#else
	#define MUU_CALLCONV
#endif

//======================================================================================================================
// ATTRIBUTES, UTILITY MACROS ETC
//======================================================================================================================

#if MUU_CLANG || MUU_GCC_LIKE
	#define MUU_ATTR(...) __attribute__((__VA_ARGS__))
#else
	#define MUU_ATTR(...)
#endif
/// \def MUU_ATTR(...)
/// \brief Expands to `__attribute__(( ... ))` when compiling with a compiler that supports GNU-style attributes.

#ifdef NDEBUG
	#define MUU_ATTR_NDEBUG(...) MUU_ATTR(__VA_ARGS__)
#else
	#define MUU_ATTR_NDEBUG(...)
#endif
/// \def MUU_ATTR_NDEBUG(...)
/// \brief Expands to `__attribute__(( ... ))` when compiling with a compiler that supports GNU-style attributes
/// 	   and NDEBUG is defined.

#if MUU_MSVC_LIKE
	#define MUU_DECLSPEC(...) __declspec(__VA_ARGS__)
#else
	#define MUU_DECLSPEC(...)
#endif
/// \def MUU_DECLSPEC(...)
/// \brief Expands to `__declspec( ... )` when compiling with MSVC (or another compiler in MSVC-mode).

#if MUU_MSVC_LIKE
	#define MUU_UNREACHABLE __assume(0)
#elif MUU_ICC || MUU_CLANG || MUU_GCC_LIKE || MUU_HAS_BUILTIN(__builtin_unreachable)
	#define MUU_UNREACHABLE __builtin_unreachable()
#else
	#define MUU_UNREACHABLE static_assert(true)
#endif
/// \def MUU_UNREACHABLE
/// \brief Marks a position in the code as being unreachable.
/// \warning Using this incorrectly can lead to seriously mis-compiled code!

#if MUU_MSVC_LIKE
	#define MUU_ASSUME(cond) __assume(cond)
#elif MUU_ICC || MUU_CLANG || MUU_HAS_BUILTIN(__builtin_assume)
	#define MUU_ASSUME(cond) __builtin_assume(cond)
#else
	#define MUU_ASSUME(cond) MUU_NOOP
#endif
/// \def MUU_ASSUME
/// \brief Optimizer hint for signalling various assumptions about state at specific points in code.
/// \details \cpp
/// 	void do_the_thing(int flags) noexcept
/// 	{
/// 		MUU_ASSUME(flags > 0);
///			// ... do some some performance-critical code stuff that assumes flags is > 0 here ...
/// 	}
/// \ecpp
/// \warning Using this incorrectly can lead to seriously mis-compiled code!

#if MUU_MSVC_LIKE
	#define MUU_MALLOC MUU_DECLSPEC(restrict)
#elif MUU_CLANG || MUU_GCC_LIKE || MUU_HAS_ATTR(__malloc__)
	#define MUU_MALLOC MUU_ATTR(__malloc__)
#else
	#define MUU_MALLOC
#endif
/// \def MUU_MALLOC
/// \brief Optimizer hint that marks an allocating function's pointer return value as representing a newly allocated memory region free from aliasing.
/// \see [__declspec(restrict)](https://docs.microsoft.com/en-us/cpp/cpp/restrict?view=vs-2019)

#if MUU_HAS_CONSTEVAL
	#define MUU_CONSTEVAL consteval
#else
	#define MUU_CONSTEVAL constexpr
#endif
/// \def MUU_CONSTEVAL
/// \brief Expands to C++20's `consteval` if supported by your compiler, otherwise `constexpr`.
/// \see [consteval](https://en.cppreference.com/w/cpp/language/consteval)

#if MUU_MSVC_LIKE
	#define MUU_ALWAYS_INLINE __forceinline
#elif MUU_CLANG || MUU_GCC_LIKE || MUU_HAS_ATTR(__always_inline__)
	#define MUU_ALWAYS_INLINE                                                                                          \
		MUU_ATTR(__always_inline__)                                                                                    \
		inline
#else
	#define MUU_ALWAYS_INLINE inline
#endif
/// \def MUU_ALWAYS_INLINE
/// \brief The same linkage semantics as the `inline` keyword, with an additional hint to
/// 	   the optimizer that you'd really, _really_ like a function inlined.
/// \details \cpp
/// 	MUU_ALWAYS_INLINE
/// 	void always_inline_me() noexcept
/// 	{
/// 		// ...
/// 	}
/// \ecpp

#if MUU_MSVC_LIKE
	#define MUU_NEVER_INLINE MUU_DECLSPEC(noinline)
#elif MUU_CLANG || MUU_GCC_LIKE || MUU_HAS_ATTR(__noinline__)
	#define MUU_NEVER_INLINE MUU_ATTR(__noinline__)
#else
	#define MUU_NEVER_INLINE
#endif
/// \def MUU_NEVER_INLINE
/// \brief A strong hint to the optimizer that you really, _really_ do not want a function inlined.
/// \details \cpp
/// 	MUU_NEVER_INLINE
/// 	int never_inline_me() noexcept
/// 	{
/// 		// ...
/// 	}
/// \ecpp

#if MUU_MSVC_LIKE
	#define MUU_ABSTRACT_INTERFACE MUU_DECLSPEC(novtable)
#else
	#define MUU_ABSTRACT_INTERFACE
#endif
/// \def MUU_ABSTRACT_INTERFACE
/// \brief Marks a class being interface-only and not requiring a vtable.
/// \details Useful for abstract base classes:\cpp
/// 	class MUU_ABSTRACT_INTERFACE virtual_base
/// 	{
/// 		virtual void fooify() noexcept = 0;
/// 		virtual ~virtual_base() noexcept = default;
/// 	};
/// \ecpp

#if MUU_MSVC_LIKE
	#define MUU_EMPTY_BASES MUU_DECLSPEC(empty_bases)
#else
	#define MUU_EMPTY_BASES
#endif
/// \def MUU_EMPTY_BASES
/// \brief Marks a class as having only empty base classes.
/// \details This is required for some compilers to use Empty Base Class Optimization:\cpp
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

#if MUU_CLANG || MUU_HAS_ATTR(__trivial_abi__)
	#define MUU_TRIVIAL_ABI MUU_ATTR(__trivial_abi__)
#else
	#define MUU_TRIVIAL_ABI
#endif
/// \def MUU_TRIVIAL_ABI
/// \brief Marks a simple type which might otherwise be considered non-trivial
/// 	   as being trivial over ABI boundaries.
/// \details This is useful for simple wrappers and 'strong typedef' types, e.g.:  \cpp
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

#if MUU_CPP >= 20 && MUU_HAS_CPP_ATTR(likely) >= 201803
	#define MUU_LIKELY(...) (__VA_ARGS__) [[likely]]
#elif MUU_CLANG || MUU_GCC_LIKE || MUU_HAS_BUILTIN(__builtin_expect)
	#define MUU_LIKELY(...) (__builtin_expect(!!(__VA_ARGS__), 1))
#else
	#define MUU_LIKELY(...) (__VA_ARGS__)
#endif
/// \def MUU_LIKELY
/// \brief Expands a conditional to include an optimizer intrinsic (or C++20's [[likely]], if available)
/// 	   indicating that an if/else conditional is the likely path.
/// \details \cpp
/// 	if MUU_LIKELY(condition_that_is_almost_always_true)
/// 	{
/// 		do_the_thing();
/// 	}
/// \ecpp
/// \see
/// 	- [\[\[likely\]\]](https://en.cppreference.com/w/cpp/language/attributes/likely)
/// 	- [__builtin_expect()](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html)

#if MUU_CPP >= 20 && MUU_HAS_CPP_ATTR(unlikely) >= 201803
	#define MUU_UNLIKELY(...) (__VA_ARGS__) [[unlikely]]
#elif MUU_CLANG || MUU_GCC_LIKE || MUU_HAS_BUILTIN(__builtin_expect)
	#define MUU_UNLIKELY(...) (__builtin_expect(!!(__VA_ARGS__), 0))
#else
	#define MUU_UNLIKELY(...) (__VA_ARGS__)
#endif
/// \def MUU_UNLIKELY
/// \brief Expands a conditional to include an optimizer intrinsic (or C++20's [[unlikely]], if available)
/// 	   indicating that an if/else conditional is the unlikely path.
/// \details \cpp
/// 	if MUU_UNLIKELY(condition_that_is_almost_always_false)
/// 	{
/// 		do_the_thing();
/// 	}
/// \ecpp
/// \see
/// 	- [\[\[unlikely\]\]](https://en.cppreference.com/w/cpp/language/attributes/likely)
/// 	- [__builtin_expect()](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html)

#if MUU_CPP >= 20 && MUU_HAS_CPP_ATTR(no_unique_address) >= 201803
	#define MUU_NO_UNIQUE_ADDRESS [[no_unique_address]]
#else
	#define MUU_NO_UNIQUE_ADDRESS
#endif
/// \def MUU_NO_UNIQUE_ADDRESS
/// \brief Expands to C++20's `[[no_unique_address]]` if supported by your compiler.

#if MUU_HAS_CPP_ATTR(nodiscard) >= 201603
	#define MUU_NODISCARD		[[nodiscard]]
	#define MUU_NODISCARD_CLASS [[nodiscard]]
#elif MUU_CLANG || MUU_GCC_LIKE || MUU_HAS_ATTR(warn_unused_result)
	#define MUU_NODISCARD MUU_ATTR(warn_unused_result)
#else
	#define MUU_NODISCARD
#endif
#ifndef MUU_NODISCARD_CLASS
	#define MUU_NODISCARD_CLASS
#endif
/// \def MUU_NODISCARD
/// \brief Expands to `[[nodiscard]]` or `__attribute__((warn_unused_result))`.

#if MUU_HAS_CPP_ATTR(nodiscard) >= 201907
	#define MUU_NODISCARD_CTOR [[nodiscard]]
#else
	#define MUU_NODISCARD_CTOR
#endif
/// \def MUU_NODISCARD_CTOR
/// \brief Expands to `[[nodiscard]]` if your compiler supports it on constructors.
/// \details This is useful for RAII helper types like locks:\cpp
/// 	class raii_lock
/// 	{
/// 		MUU_NODISCARD_CTOR
/// 		raii_lock() noexcept
/// 		{
/// 			super_special_global_lock();
/// 		}
///
/// 		~raii_lock() noexcept
/// 		{
/// 			super_special_global_unlock();
/// 		}
/// 	};
///
/// 	int whoops() noexcept
/// 	{
/// 		raii_lock{}; // oh noes! forgot to give a name; would be destroyed immediately.
/// 		             // MUU_NODISCARD_CTOR makes the compiler emit a warning here.
///
/// 		something_that_requires_the_lock();
/// 	}
/// \ecpp

#define MUU_CONCAT_1(x, y) x##y
#define MUU_CONCAT(x, y)   MUU_CONCAT_1(x, y)
/// \def MUU_CONCAT(x, y)
/// \brief Concatenates two preprocessor inputs. Equivalent to the `##` operator, but with macro expansion.

#define MUU_MAKE_STRING_1(s) #s
#define MUU_MAKE_STRING(s)	 MUU_MAKE_STRING_1(s)
/// \def MUU_MAKE_STRING(s)
/// \brief Stringifies the input, converting it into a string literal.
/// \details \cpp
/// // these are equivalent:
///	constexpr auto str1 = MUU_MAKE_STRING(Oh noes!);
///	constexpr auto str2 = "Oh noes!";
/// \ecpp
/// \see [String literals in C++](https://en.cppreference.com/w/cpp/language/string_literal)

#define MUU_PREPEND_R_1(s)		 R##s
#define MUU_PREPEND_R(s)		 MUU_PREPEND_R_1(s)
#define MUU_ADD_PARENTHESES_1(s) (s)
#define MUU_ADD_PARENTHESES(s)	 MUU_ADD_PARENTHESES_1(s)
#define MUU_MAKE_RAW_STRING_1(s) MUU_MAKE_STRING(MUU_ADD_PARENTHESES(s))
#define MUU_MAKE_RAW_STRING(s)	 MUU_PREPEND_R(MUU_MAKE_RAW_STRING_1(s))
/// \def MUU_MAKE_RAW_STRING
/// \brief Stringifies the input, converting it verbatim into a raw string literal.
/// \details \cpp
/// // these are equivalent:
/// constexpr auto str1 = MUU_MAKE_RAW_STRING("It's trap!" the admiral cried.);
/// constexpr auto str2 = R"("It's trap!" the admiral cried.)";
/// \ecpp
/// \see [String literals in C++](https://en.cppreference.com/w/cpp/language/string_literal)

#define MUU_APPEND_SV_1(s)		s##sv
#define MUU_APPEND_SV(s)		MUU_APPEND_SV_1(s)
#define MUU_MAKE_STRING_VIEW(s) MUU_APPEND_SV(MUU_MAKE_RAW_STRING(s))
/// \def MUU_MAKE_STRING_VIEW(s)
/// \brief Stringifies the input, converting it verbatim into a raw string view literal.
/// \details \cpp
/// // these are equivalent:
///	constexpr std::string_view str1 = MUU_MAKE_STRING_VIEW("It's trap!" the admiral cried.);
///	constexpr std::string_view str2 = R"("It's trap!" the admiral cried.)"sv;
/// \ecpp
/// \see [String literals in C++](https://en.cppreference.com/w/cpp/language/string_literal)

#if MUU_CLANG
	#include "impl/preprocessor_macro_dispatch_clang.h"

	#define MUU_ATTR_CLANG(...)			MUU_ATTR(__VA_ARGS__)
	#define MUU_ATTR_CLANG_GE(ver, ...) MUU_CONCAT(MUU_MACRO_DISPATCH_CLANG_GE_, ver)(MUU_ATTR(__VA_ARGS__))
	#define MUU_ATTR_CLANG_LT(ver, ...) MUU_CONCAT(MUU_MACRO_DISPATCH_CLANG_LT_, ver)(MUU_ATTR(__VA_ARGS__))

	#define MUU_PRAGMA_CLANG(decl)		   _Pragma(MUU_MAKE_STRING(clang decl))
	#define MUU_PRAGMA_CLANG_GE(ver, decl) MUU_CONCAT(MUU_MACRO_DISPATCH_CLANG_GE_, ver)(MUU_PRAGMA_CLANG(decl))
	#define MUU_PRAGMA_CLANG_LT(ver, decl) MUU_CONCAT(MUU_MACRO_DISPATCH_CLANG_LT_, ver)(MUU_PRAGMA_CLANG(decl))
#else
	#define MUU_ATTR_CLANG(...)
	#define MUU_ATTR_CLANG_GE(ver, ...)
	#define MUU_ATTR_CLANG_LT(ver, ...)

	#define MUU_PRAGMA_CLANG(...)
	#define MUU_PRAGMA_CLANG_GE(ver, ...)
	#define MUU_PRAGMA_CLANG_LT(ver, ...)
#endif
/// \def MUU_ATTR_CLANG(...)
/// \brief Expands to `__attribute__(( ... ))` when compiling with Clang.
///
/// \def MUU_ATTR_CLANG_GE(ver, ...)
/// \brief Expands to `__attribute__(( ... ))` when compiling with Clang and `__clang_major__` >= `ver`.
///
/// \def MUU_ATTR_CLANG_LT(ver, ...)
/// \brief Expands to `__attribute__(( ... ))` when compiling with Clang and `__clang_major__` < `ver`.
///
/// \def MUU_PRAGMA_CLANG(...)
/// \brief Expands to `_Pragma("clang ...")` when compiling with Clang.
///
/// \def MUU_PRAGMA_CLANG_GE(ver, ...)
/// \brief Expands to `_Pragma("clang ...")` when compiling with Clang and `__clang_major__` >= `ver`.
///
/// \def MUU_PRAGMA_CLANG_LT(ver, ...)
/// \brief Expands to `_Pragma("clang ...")` when compiling with Clang and `__clang_major__` < `ver`.

#if MUU_GCC
	#include "impl/preprocessor_macro_dispatch_gcc.h"

	#define MUU_ATTR_GCC(...)		  MUU_ATTR(__VA_ARGS__)
	#define MUU_ATTR_GCC_GE(ver, ...) MUU_CONCAT(MUU_MACRO_DISPATCH_GCC_GE_, ver)(MUU_ATTR(__VA_ARGS__))
	#define MUU_ATTR_GCC_LT(ver, ...) MUU_CONCAT(MUU_MACRO_DISPATCH_GCC_LT_, ver)(MUU_ATTR(__VA_ARGS__))

	#define MUU_PRAGMA_GCC(decl)		 _Pragma(MUU_MAKE_STRING(GCC decl))
	#define MUU_PRAGMA_GCC_GE(ver, decl) MUU_CONCAT(MUU_MACRO_DISPATCH_GCC_GE_, ver)(MUU_PRAGMA_GCC(decl))
	#define MUU_PRAGMA_GCC_LT(ver, decl) MUU_CONCAT(MUU_MACRO_DISPATCH_GCC_LT_, ver)(MUU_PRAGMA_GCC(decl))
#else
	#define MUU_ATTR_GCC(...)
	#define MUU_ATTR_GCC_GE(ver, ...)
	#define MUU_ATTR_GCC_LT(ver, ...)

	#define MUU_PRAGMA_GCC(...)
	#define MUU_PRAGMA_GCC_GE(ver, ...)
	#define MUU_PRAGMA_GCC_LT(ver, ...)
#endif
/// \def MUU_ATTR_GCC(...)
/// \brief Expands to `__attribute__(( ... ))` when compiling with GCC.
///
/// \def MUU_ATTR_GCC_GE(ver, ...)
/// \brief Expands to `__attribute__(( ... ))` when compiling with GCC and `__GNUC__` >= `ver`.
///
/// \def MUU_ATTR_GCC_LT(ver, ...)
/// \brief Expands to `__attribute__(( ... ))` when compiling with GCC and `__GNUC__` < `ver`.
///
/// \def MUU_PRAGMA_GCC(...)
/// \brief Expands to `_Pragma("GCC ...")` when compiling with GCC.
///
/// \def MUU_PRAGMA_GCC_GE(ver, ...)
/// \brief Expands to `_Pragma("GCC ...")` when compiling with GCC and `__GNUC__` >= `ver`.
///
/// \def MUU_PRAGMA_GCC_LT(ver, ...)
/// \brief Expands to `_Pragma("GCC ...")` when compiling with GCC and `__GNUC__` < `ver`.

#if MUU_MSVC
	#include "impl/preprocessor_macro_dispatch_msvc.h"

	#define MUU_PRAGMA_MSVC(...)		  __pragma(__VA_ARGS__)
	#define MUU_PRAGMA_MSVC_GE(ver, decl) MUU_CONCAT(MUU_MACRO_DISPATCH_MSVC_GE_, ver)(MUU_PRAGMA_MSVC(decl))
	#define MUU_PRAGMA_MSVC_LT(ver, decl) MUU_CONCAT(MUU_MACRO_DISPATCH_MSVC_LT_, ver)(MUU_PRAGMA_MSVC(decl))
#else
	#define MUU_PRAGMA_MSVC(...)
	#define MUU_PRAGMA_MSVC_GE(ver, ...)
	#define MUU_PRAGMA_MSVC_LT(ver, ...)
#endif
///
/// \def MUU_PRAGMA_MSVC(...)
/// \brief Expands to `__pragma(...)` when compiling with MSVC.
///
/// \def MUU_PRAGMA_MSVC_GE(ver, ...)
/// \brief Expands to `__pragma(...)` when compiling with MSVC and `_MSC_VER` >= `ver`.
///
/// \def MUU_PRAGMA_MSVC_LT(ver, ...)
/// \brief Expands to `__pragma(...)` when compiling with MSVC and `_MSC_VER` < `ver`.

#if MUU_ICC
	#define MUU_PRAGMA_ICC(...) __pragma(__VA_ARGS__)
#else
	#define MUU_PRAGMA_ICC(...)
#endif
/// \def MUU_PRAGMA_ICC(...)
/// \brief Expands to `_pragma(...)` when compiling with ICC.

#define MUU_DELETE_MOVE(T)                                                                                             \
	T(T&&)			  = delete;                                                                                        \
	T& operator=(T&&) = delete
/// \def MUU_DELETE_MOVE(T)
/// \brief Explicitly deletes the move constructor and move-assignment operator of a class or struct.
/// \details \cpp
/// class immovable_type
/// {
/// 	immovable_type() {}
///
/// 	MUU_DELETE_MOVE(immovable_type);
/// };
///
/// //equivalent to:
/// class immovable_type
/// {
/// 	immovable_type() {}
///
/// 	immovable_type(immovable_type&&) = delete;
/// 	immovable_type& operator=(immovable_type&&) = delete;
/// };
/// \ecpp
/// \see https://cpppatterns.com/patterns/rule-of-five.html

#define MUU_DELETE_COPY(T)                                                                                             \
	T(const T&)			   = delete;                                                                                   \
	T& operator=(const T&) = delete
/// \def MUU_DELETE_COPY(T)
/// \brief Explicitly deletes the copy constructor and copy-assignment operator of a class or struct.
/// \details \cpp
/// class uncopyable
/// {
/// 	uncopyable() {}
/// 	MUU_DELETE_COPY(uncopyable);
/// };
///
/// //equivalent to:
/// class uncopyable
/// {
/// 	uncopyable() {}
///
/// 	uncopyable(const uncopyable&) = delete;
/// 	uncopyable& operator=(const uncopyable&) = delete;
/// };
/// \ecpp
/// \see https://cpppatterns.com/patterns/rule-of-five.html

#define MUU_DEFAULT_MOVE(T)                                                                                            \
	T(T&&)			  = default;                                                                                       \
	T& operator=(T&&) = default
/// \def MUU_DEFAULT_MOVE
/// \brief Explicitly defaults the move constructor and move-assignment operator of a class or struct.
/// \details \cpp
/// class movable_type
/// {
/// 	movable_type() {}
///
/// 	MUU_DEFAULT_MOVE(movable_type);
/// };
///
/// //equivalent to:
/// class movable_type
/// {
/// 	movable_type() {}
///
/// 	movable_type(movable_type&&) = default;
/// 	movable_type& operator=(movable_type&&) = default;
/// };
/// \ecpp
/// \see https://cpppatterns.com/patterns/rule-of-five.html

#define MUU_DEFAULT_COPY(T)                                                                                            \
	T(const T&)			   = default;                                                                                  \
	T& operator=(const T&) = default
/// \def MUU_DEFAULT_COPY
/// \brief Explicitly defaults the copy constructor and copy-assignment operator of a class or struct.
/// \details \cpp
/// class copyable_type
/// {
/// 	copyable_type() {}
/// 	MUU_DEFAULT_COPY(copyable_type);
/// };
///
/// //equivalent to:
/// class copyable_type
/// {
/// 	copyable_type() {}
///
/// 	copyable_type(const copyable_type&) = default;
/// 	copyable_type& operator=(const copyable_type&) = default;
/// };
/// \ecpp
/// \see https://cpppatterns.com/patterns/rule-of-five.html

#define MUU_UNUSED(...) static_cast<void>(__VA_ARGS__)
/// \def MUU_UNUSED(expr)
/// \brief Explicitly denotes the result of an expression as being unused.

#define MUU_NOOP MUU_UNUSED(0)
/// \def MUU_NOOP
/// \brief Expands to a no-op expression.

// clang-format off
#ifdef NDEBUG
	#define MUU_PURE					MUU_DECLSPEC(noalias)	MUU_ATTR(pure)
	#define MUU_CONST					MUU_DECLSPEC(noalias)	MUU_ATTR(const)
	#define MUU_PURE_GETTER				MUU_NODISCARD						MUU_PURE
	#define MUU_CONST_GETTER			MUU_NODISCARD						MUU_CONST
	#define MUU_PURE_INLINE_GETTER		MUU_NODISCARD	MUU_ALWAYS_INLINE	MUU_PURE
	#define MUU_CONST_INLINE_GETTER		MUU_NODISCARD	MUU_ALWAYS_INLINE	MUU_CONST
#else
	#define MUU_PURE
	#define MUU_CONST
	#define MUU_PURE_GETTER				MUU_NODISCARD	
	#define MUU_CONST_GETTER			MUU_NODISCARD	
	#define MUU_PURE_INLINE_GETTER		MUU_NODISCARD	MUU_ALWAYS_INLINE
	#define MUU_CONST_INLINE_GETTER		MUU_NODISCARD	MUU_ALWAYS_INLINE
#endif
// clang-format on

#if MUU_MSVC_LIKE && (MUU_ARCH_X86 || MUU_ARCH_AMD64) && MUU_ISET_SSE2
	#define MUU_VECTORCALL	   __vectorcall
	#define MUU_HAS_VECTORCALL 1
#else
	#define MUU_VECTORCALL
	#define MUU_HAS_VECTORCALL 0
#endif
#if !MUU_DOXYGEN && !MUU_INTELLISENSE
	#define MUU_VPARAM(...) ::muu::vector_param<__VA_ARGS__>
#else
	#define MUU_VPARAM(...) const __VA_ARGS__&
#endif
/// \def MUU_VECTORCALL
/// \brief Expands to `__vectorcall` on compilers that support it.
/// \see [__vectorcall](https://docs.microsoft.com/en-us/cpp/cpp/vectorcall?view=vs-2019)

#define MUU_MAKE_FLAGS_2(T, op, linkage)                                                                               \
	MUU_CONST_INLINE_GETTER                                                                                            \
	linkage constexpr T operator op(T lhs, T rhs) noexcept                                                             \
	{                                                                                                                  \
		using under = std::underlying_type_t<T>;                                                                       \
		return static_cast<T>(static_cast<under>(lhs) op static_cast<under>(rhs));                                     \
	}                                                                                                                  \
                                                                                                                       \
	linkage constexpr T& operator MUU_CONCAT(op, =)(T & lhs, T rhs) noexcept                                           \
	{                                                                                                                  \
		return lhs = (lhs op rhs);                                                                                     \
	}                                                                                                                  \
                                                                                                                       \
	static_assert(true)

#define MUU_MAKE_FLAGS_1(T, linkage)                                                                                   \
	static_assert(std::is_enum_v<T>);                                                                                  \
                                                                                                                       \
	MUU_MAKE_FLAGS_2(T, &, linkage);                                                                                   \
	MUU_MAKE_FLAGS_2(T, |, linkage);                                                                                   \
	MUU_MAKE_FLAGS_2(T, ^, linkage);                                                                                   \
                                                                                                                       \
	MUU_CONST_INLINE_GETTER                                                                                            \
	linkage constexpr T operator~(T val) noexcept                                                                      \
	{                                                                                                                  \
		using under = std::underlying_type_t<T>;                                                                       \
		return static_cast<T>(~static_cast<under>(val));                                                               \
	}                                                                                                                  \
                                                                                                                       \
	MUU_CONST_INLINE_GETTER                                                                                            \
	linkage constexpr bool operator!(T val) noexcept                                                                   \
	{                                                                                                                  \
		using under = std::underlying_type_t<T>;                                                                       \
		return !static_cast<under>(val);                                                                               \
	}                                                                                                                  \
                                                                                                                       \
	static_assert(true)

#define MUU_MAKE_FLAGS(T) MUU_MAKE_FLAGS_1(T, )
/// \def MUU_MAKE_FLAGS(T)
/// \brief Stamps out operators for enum 'flags' types.
/// \details \cpp
/// enum class my_flags : unsigned
/// {
/// 	none   = 0,
/// 	alpha  = 1,
/// 	beta   = 2,
/// 	gamma  = 4
/// };
/// MUU_MAKE_FLAGS(my_flags);
///
/// // emits these operators:
/// constexpr my_flags  operator &  (my_flags,  my_flags) noexcept;
/// constexpr my_flags& operator &= (my_flags&, my_flags) noexcept;
/// constexpr my_flags  operator |  (my_flags,  my_flags) noexcept;
/// constexpr my_flags& operator |= (my_flags&, my_flags) noexcept;
/// constexpr my_flags  operator ^  (my_flags,  my_flags) noexcept;
/// constexpr my_flags& operator ^= (my_flags&, my_flags) noexcept;
/// constexpr my_flags  operator ~  (my_flags) noexcept;
/// constexpr bool      operator !  (my_flags) noexcept;
/// \ecpp

//======================================================================================================================
// COMPILER-SPECIFIC WARNING MANAGEMENT
//======================================================================================================================

#if MUU_CLANG

	#define MUU_PUSH_WARNINGS                                                                                          \
		MUU_PRAGMA_CLANG(diagnostic push)                                                                              \
		static_assert(true)

	#define MUU_DISABLE_SWITCH_WARNINGS                                                                                \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Wswitch")                                                                \
		static_assert(true)

	#define MUU_DISABLE_LIFETIME_WARNINGS                                                                              \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Wmissing-field-initializers")                                            \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Wglobal-constructors")                                                   \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Wexit-time-destructors")                                                 \
		static_assert(true)

	#define MUU_DISABLE_ARITHMETIC_WARNINGS                                                                            \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Wfloat-equal")                                                           \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Wdouble-promotion")                                                      \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Wchar-subscripts")                                                       \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Wshift-sign-overflow")                                                   \
		MUU_PRAGMA_CLANG_GE(10, diagnostic ignored "-Wimplicit-int-float-conversion")                                  \
		static_assert(true)

	#define MUU_DISABLE_SHADOW_WARNINGS                                                                                \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Wshadow")                                                                \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Wshadow-field")                                                          \
		static_assert(true)

	#define MUU_DISABLE_SPAM_WARNINGS                                                                                  \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Wweak-vtables")                                                          \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Wdouble-promotion")                                                      \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Wweak-template-vtables")                                                 \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Wpadded")                                                                \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Wc++2a-compat")                                                          \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Wtautological-pointer-compare")                                          \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Wmissing-field-initializers")                                            \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Wpacked")                                                                \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Wdisabled-macro-expansion")                                              \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Wused-but-marked-unused")                                                \
		MUU_PRAGMA_CLANG_GE(9, diagnostic ignored "-Wctad-maybe-unsupported")                                          \
		static_assert(true)

	#define MUU_POP_WARNINGS                                                                                           \
		MUU_PRAGMA_CLANG(diagnostic pop)                                                                               \
		static_assert(true)

	#define MUU_DISABLE_WARNINGS                                                                                       \
		MUU_PRAGMA_CLANG(diagnostic push)                                                                              \
		MUU_PRAGMA_CLANG(diagnostic ignored "-Weverything")                                                            \
		static_assert(true, "")

	#define MUU_ENABLE_WARNINGS                                                                                        \
		MUU_PRAGMA_CLANG(diagnostic pop)                                                                               \
		static_assert(true)

#elif MUU_MSVC

	#define MUU_PUSH_WARNINGS                                                                                          \
		__pragma(warning(push))                                                                                        \
		static_assert(true)

	#if MUU_HAS_INCLUDE(<CodeAnalysis/Warnings.h>)
		#pragma warning(push, 0)
		#include <CodeAnalysis/Warnings.h>
		#pragma warning(pop)
		#define MUU_DISABLE_CODE_ANALYSIS_WARNINGS                                                                     \
			__pragma(warning(disable : ALL_CODE_ANALYSIS_WARNINGS))                                                    \
			static_assert(true)
	#else
		#define MUU_DISABLE_CODE_ANALYSIS_WARNINGS static_assert(true)
	#endif

	#define MUU_DISABLE_SWITCH_WARNINGS                                                                                \
		__pragma(warning(disable : 4061))                                                                              \
		__pragma(warning(disable : 4062))                                                                              \
		__pragma(warning(disable : 4063))                                                                              \
		__pragma(warning(disable : 4468))  /* 'fallthrough': attribute must be followed by a case label */             \
		__pragma(warning(disable : 5262))  /* implicit through */                                                      \
		__pragma(warning(disable : 26819)) /* cg: unannotated fallthrough */                                           \
		static_assert(true)

	#define MUU_DISABLE_SHADOW_WARNINGS                                                                                \
		__pragma(warning(disable : 4458))                                                                              \
		static_assert(true)

	#define MUU_DISABLE_DEPRECATION_WARNINGS                                                                           \
		__pragma(warning(disable : 4996))                                                                              \
		static_assert(true)

	#define MUU_DISABLE_SPAM_WARNINGS                                                                                  \
		__pragma(warning(disable : 4127)) /* conditional expr is constant */                                           \
		__pragma(warning(disable : 4324)) /* structure was padded due to alignment specifier */                        \
		__pragma(warning(disable : 4348))                                                                              \
		__pragma(warning(disable : 4464))  /* relative include path contains '..' */                                   \
		__pragma(warning(disable : 4505))  /* unreferenced local function removed */                                   \
		__pragma(warning(disable : 4514))  /* unreferenced inline function has been removed */                         \
		__pragma(warning(disable : 4582))  /* constructor is not implicitly called */                                  \
		__pragma(warning(disable : 4619))  /*  there is no warning number 'XXXX' */                                    \
		__pragma(warning(disable : 4623))  /* default constructor was implicitly defined as deleted */                 \
		__pragma(warning(disable : 4625))  /* copy constructor was implicitly defined as deleted */                    \
		__pragma(warning(disable : 4626))  /* assignment operator was implicitly defined as deleted */                 \
		__pragma(warning(disable : 4686))  /* possible change in behavior, change in UDT return calling convention */  \
		__pragma(warning(disable : 4710))  /* function not inlined */                                                  \
		__pragma(warning(disable : 4711))  /* function selected for automatic expansion */                             \
		__pragma(warning(disable : 4820))  /* N bytes padding added */                                                 \
		__pragma(warning(disable : 4866))  /* compiler may not enforce left-to-right evaluation order for call */      \
		__pragma(warning(disable : 4946))  /* reinterpret_cast used between related classes */                         \
		__pragma(warning(disable : 5026))  /* move constructor was implicitly defined as deleted */                    \
		__pragma(warning(disable : 5027))  /* move assignment operator was implicitly defined as deleted */            \
		__pragma(warning(disable : 5039))  /* potentially throwing function passed to 'extern "C"' function */         \
		__pragma(warning(disable : 5045))  /* Compiler will insert Spectre mitigation */                               \
		__pragma(warning(disable : 5246))  /* initialization of a subobject should be wrapped in braces */             \
		__pragma(warning(disable : 5264))  /* const variable is not used (false-positive) */                           \
		__pragma(warning(disable : 26490)) /* cg: dont use reinterpret_cast */                                         \
		__pragma(warning(disable : 26812)) /* cg: Prefer 'enum class' over 'enum' */                                   \
		static_assert(true)

	#define MUU_DISABLE_ARITHMETIC_WARNINGS                                                                            \
		__pragma(warning(disable : 4365)) /* argument signed/unsigned mismatch */                                      \
		__pragma(warning(disable : 4738)) /* storing 32-bit float result in memory */                                  \
		__pragma(warning(disable : 5219)) /* implicit conversion from integral to float */                             \
		static_assert(true)

	#define MUU_POP_WARNINGS                                                                                           \
		__pragma(warning(pop))                                                                                         \
		static_assert(true)

	#define MUU_DISABLE_WARNINGS                                                                                       \
		__pragma(warning(push, 0))                                                                                     \
		__pragma(warning(disable : 4348))                                                                              \
		__pragma(warning(disable : 4668))                                                                              \
		__pragma(warning(disable : 5105))                                                                              \
		MUU_DISABLE_CODE_ANALYSIS_WARNINGS;                                                                            \
		MUU_DISABLE_SWITCH_WARNINGS;                                                                                   \
		MUU_DISABLE_SHADOW_WARNINGS;                                                                                   \
		MUU_DISABLE_DEPRECATION_WARNINGS;                                                                              \
		MUU_DISABLE_SPAM_WARNINGS;                                                                                     \
		MUU_DISABLE_ARITHMETIC_WARNINGS;                                                                               \
		static_assert(true)

	#define MUU_ENABLE_WARNINGS                                                                                        \
		__pragma(warning(pop))                                                                                         \
		static_assert(true)

#elif MUU_ICC

	#define MUU_PUSH_WARNINGS                                                                                          \
		__pragma(warning(push))                                                                                        \
		static_assert(true)

	#define MUU_DISABLE_SPAM_WARNINGS                                                                                  \
		__pragma(warning(disable : 82))	  /* storage class is not first */                                             \
		__pragma(warning(disable : 111))  /* statement unreachable (false-positive) */                                 \
		__pragma(warning(disable : 869))  /* unreferenced parameter */                                                 \
		__pragma(warning(disable : 1011)) /* missing return (false-positive) */                                        \
		__pragma(warning(disable : 2261)) /* assume expr side-effects discarded */                                     \
		static_assert(true)

	#define MUU_POP_WARNINGS                                                                                           \
		__pragma(warning(pop))                                                                                         \
		static_assert(true)

	#define MUU_DISABLE_WARNINGS                                                                                       \
		__pragma(warning(push, 0))                                                                                     \
		MUU_DISABLE_SPAM_WARNINGS

	#define MUU_ENABLE_WARNINGS                                                                                        \
		__pragma(warning(pop))                                                                                         \
		static_assert(true)

#elif MUU_GCC

	#define MUU_PUSH_WARNINGS                                                                                          \
		MUU_PRAGMA_GCC(diagnostic push)                                                                                \
		static_assert(true)

	#define MUU_DISABLE_SWITCH_WARNINGS                                                                                \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wswitch")                                                                  \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wswitch-enum")                                                             \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wswitch-default")                                                          \
		static_assert(true)

	#define MUU_DISABLE_LIFETIME_WARNINGS                                                                              \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wmissing-field-initializers")                                              \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wmaybe-uninitialized")                                                     \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wuninitialized")                                                           \
		MUU_PRAGMA_GCC_GE(8, diagnostic ignored "-Wclass-memaccess")                                                   \
		static_assert(true)

	#define MUU_DISABLE_ARITHMETIC_WARNINGS                                                                            \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wfloat-equal")                                                             \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wsign-conversion")                                                         \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wchar-subscripts")                                                         \
		static_assert(true)

	#define MUU_DISABLE_SHADOW_WARNINGS                                                                                \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wshadow")                                                                  \
		static_assert(true)

	#define MUU_DISABLE_SUGGEST_WARNINGS                                                                               \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wsuggest-attribute=const")                                                 \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wsuggest-attribute=pure")                                                  \
		static_assert(true)

	#define MUU_DISABLE_SPAM_WARNINGS                                                                                  \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wpadded")                                                                  \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wcast-align")                                                              \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wcomment")                                                                 \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wsubobject-linkage")                                                       \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wuseless-cast")                                                            \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wmissing-field-initializers")                                              \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wmaybe-uninitialized")                                                     \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wtype-limits")                                                             \
		MUU_PRAGMA_GCC_LT(9, diagnostic ignored "-Wattributes")                                                        \
		static_assert(true)

	#define MUU_POP_WARNINGS                                                                                           \
		MUU_PRAGMA_GCC(diagnostic pop)                                                                                 \
		static_assert(true)

	#define MUU_DISABLE_WARNINGS                                                                                       \
		MUU_PRAGMA_GCC(diagnostic push)                                                                                \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wall")                                                                     \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wextra")                                                                   \
		MUU_PRAGMA_GCC(diagnostic ignored "-Wpedantic")                                                                \
		MUU_PRAGMA_GCC_GE(12, diagnostic ignored "-Winterference-size")                                                \
		MUU_DISABLE_SWITCH_WARNINGS;                                                                                   \
		MUU_DISABLE_LIFETIME_WARNINGS;                                                                                 \
		MUU_DISABLE_ARITHMETIC_WARNINGS;                                                                               \
		MUU_DISABLE_SHADOW_WARNINGS;                                                                                   \
		MUU_DISABLE_SUGGEST_WARNINGS;                                                                                  \
		MUU_DISABLE_SPAM_WARNINGS;                                                                                     \
		static_assert(true)

	#define MUU_ENABLE_WARNINGS                                                                                        \
		MUU_PRAGMA_GCC(diagnostic pop)                                                                                 \
		static_assert(true)

#endif // gcc

#ifndef MUU_PUSH_WARNINGS
	#define MUU_PUSH_WARNINGS static_assert(true)
#endif
/// \def MUU_PUSH_WARNINGS
/// \brief Pushes the current compiler warning state onto the stack.
/// \details Use this in tandem with the other warning macros to demarcate regions of code that should have different warning semantics, e.g.: \cpp
/// 	MUU_PUSH_WARNINGS;
/// 	MUU_DISABLE_SWITCH_WARNINGS;
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
/// 	MUU_POP_WARNINGS;
/// \ecpp

#ifndef MUU_POP_WARNINGS
	#define MUU_POP_WARNINGS static_assert(true)
#endif
/// \def MUU_POP_WARNINGS
/// \brief Pops the current compiler warning state off the stack.
/// \see MUU_PUSH_WARNINGS

#ifndef MUU_DISABLE_SWITCH_WARNINGS
	#define MUU_DISABLE_SWITCH_WARNINGS static_assert(true)
#endif
/// \def MUU_DISABLE_SWITCH_WARNINGS
/// \brief Disables compiler warnings relating to the use of switch statements.
/// \see MUU_PUSH_WARNINGS

#ifndef MUU_DISABLE_LIFETIME_WARNINGS
	#define MUU_DISABLE_LIFETIME_WARNINGS static_assert(true)
#endif
/// \def MUU_DISABLE_LIFETIME_WARNINGS
/// \brief Disables compiler warnings relating to object lifetime (initialization, destruction, magic statics, et cetera).
/// \see MUU_PUSH_WARNINGS

#ifndef MUU_DISABLE_SPAM_WARNINGS
	#define MUU_DISABLE_SPAM_WARNINGS static_assert(true)
#endif
/// \def MUU_DISABLE_SPAM_WARNINGS
/// \brief Disables compiler warnings that are generally spammy/superfluous (padding, double promotion, cast alignment, etc.)
/// \see MUU_PUSH_WARNINGS

#ifndef MUU_DISABLE_ARITHMETIC_WARNINGS
	#define MUU_DISABLE_ARITHMETIC_WARNINGS static_assert(true)
#endif
/// \def MUU_DISABLE_ARITHMETIC_WARNINGS
/// \brief Disables compiler warnings relating to integer and floating-point arithmetic.
/// \see MUU_PUSH_WARNINGS

#ifndef MUU_DISABLE_SHADOW_WARNINGS
	#define MUU_DISABLE_SHADOW_WARNINGS static_assert(true)
#endif
/// \def MUU_DISABLE_SHADOW_WARNINGS
/// \brief Disables compiler warnings relating to variable, class and function name shadowing.
/// \see MUU_PUSH_WARNINGS

#ifndef MUU_DISABLE_DEPRECATION_WARNINGS
	#define MUU_DISABLE_DEPRECATION_WARNINGS static_assert(true)
#endif
/// \def MUU_DISABLE_DEPRECATION_WARNINGS
/// \brief Disables compiler warnings relating to deprecated functions, types, etc.
/// \see MUU_PUSH_WARNINGS

#ifndef MUU_DISABLE_SUGGEST_WARNINGS
	#define MUU_DISABLE_SUGGEST_WARNINGS static_assert(true)
#endif
/// \def MUU_DISABLE_SUGGEST_WARNINGS
/// \brief Disables compiler warnings resulting from `-Wsuggest=...` and friends.
/// \see MUU_PUSH_WARNINGS

#ifndef MUU_DISABLE_CODE_ANALYSIS_WARNINGS
	#define MUU_DISABLE_CODE_ANALYSIS_WARNINGS static_assert(true)
#endif
/// \def MUU_DISABLE_CODE_ANALYSIS_WARNINGS
/// \brief Disables interactive code analysis warnings (e.g. Visual Studio's background analysis engine).
/// \see MUU_PUSH_WARNINGS

#ifndef MUU_DISABLE_WARNINGS
	#define MUU_DISABLE_WARNINGS static_assert(true)
#endif
/// \def MUU_DISABLE_WARNINGS
/// \brief Pushes the current compiler warning state onto the stack then disables ALL compiler warnings.

#ifndef MUU_ENABLE_WARNINGS
	#define MUU_ENABLE_WARNINGS static_assert(true)
#endif
/// \def MUU_ENABLE_WARNINGS
/// \brief Re-enables compiler warnings again after using #MUU_DISABLE_WARNINGS.

//======================================================================================================================
// MORE ATTRIBUTES, UTILITY MACROS ETC
//======================================================================================================================

MUU_DISABLE_WARNINGS;
#if MUU_HAS_INCLUDE(<version>)
	#include <version>
#elif MUU_MSVC_LIKE && MUU_HAS_INCLUDE(<yvals_core.h>)
	#include <yvals_core.h>
#elif MUU_CPP <= 17
	#include <ciso646>
#else
	#include <iso646.h>
#endif
MUU_ENABLE_WARNINGS;

#define MUU_PUSH_PRECISE_MATH                                                                                          \
	MUU_PRAGMA_MSVC(float_control(precise, on, push))                                                                  \
	MUU_PRAGMA_GCC(push_options)                                                                                       \
	MUU_PRAGMA_GCC(optimize("-fno-fast-math"))                                                                         \
	static_assert(true)

#define MUU_POP_PRECISE_MATH                                                                                           \
	MUU_PRAGMA_GCC(pop_options)                                                                                        \
	MUU_PRAGMA_MSVC(float_control(pop))                                                                                \
	static_assert(true)

#define MUU_FMA_BLOCK                                                                                                  \
	MUU_PRAGMA_CLANG(fp contract(fast))                                                                                \
	static_assert(true)

#ifndef MUU_FORCE_NDEBUG_OPTIMIZATIONS
	#define MUU_FORCE_NDEBUG_OPTIMIZATIONS                                                                             \
		MUU_PRAGMA_GCC(push_options)                                                                                   \
		MUU_PRAGMA_GCC(optimize("O3"))                                                                                 \
		MUU_PRAGMA_MSVC(optimize("gt", on))                                                                            \
		MUU_PRAGMA_MSVC(runtime_checks("", off))                                                                       \
		MUU_PRAGMA_MSVC(strict_gs_check(push, off))                                                                    \
		static_assert(true)
#endif

#ifndef MUU_RESET_NDEBUG_OPTIMIZATIONS
	#define MUU_RESET_NDEBUG_OPTIMIZATIONS                                                                             \
		MUU_PRAGMA_MSVC(strict_gs_check(pop))                                                                          \
		MUU_PRAGMA_MSVC(runtime_checks("", restore))                                                                   \
		MUU_PRAGMA_MSVC(optimize("", on))                                                                              \
		MUU_PRAGMA_GCC(pop_options)                                                                                    \
		static_assert(true)
#endif

/// \cond
#if !MUU_DOXYGEN
	#define MUU_HIDDEN(...) __VA_ARGS__
	#define MUU_HIDDEN_BASE(...)	: __VA_ARGS__
	#define MUU_DOXYGEN_ONLY(...)
	#define MUU_IF_DOXYGEN(A, B) B
#endif
/// \endcond
#ifndef MUU_HIDDEN
	#define MUU_HIDDEN(...)
	#define MUU_HIDDEN_BASE(...)
	#define MUU_DOXYGEN_ONLY(...) __VA_ARGS__
	#define MUU_IF_DOXYGEN(A, B)  A
#endif

/// \cond
namespace muu::impl
{
	template <typename T>
	struct as_rvalue_
	{
		using type = T&&;
	};

	template <typename T>
	struct as_rvalue_<T&>
	{
		using type = T&&;
	};
}
#define MUU_MOVE(...) static_cast<typename ::muu::impl::as_rvalue_<decltype(__VA_ARGS__)>::type>(__VA_ARGS__)
/// \endcond
#ifndef MUU_MOVE
	#define MUU_MOVE(...) __VA_ARGS__
#endif

/// \cond
namespace muu::impl
{
	template <typename T>
	struct type_identity_
	{
		using type = T;
	};
}
#define MUU_TYPE_IDENTITY(...) typename ::muu::impl::type_identity_<__VA_ARGS__>::type
/// \endcond
#ifndef MUU_TYPE_IDENTITY
	#define MUU_TYPE_IDENTITY(...) __VA_ARGS__
#endif

#ifndef MUU_BLANK
	#define MUU_BLANK_3
	#define MUU_BLANK_2 MUU_BLANK_3
	#define MUU_BLANK_1 MUU_BLANK_2
	#define MUU_BLANK	MUU_BLANK_1
#endif

#include "impl/preprocessor_for_each.h"
#include "impl/preprocessor_for_each_attributes.h"

#if !defined(__POXY__) && !defined(POXY_IMPLEMENTATION_DETAIL)
	#define POXY_IMPLEMENTATION_DETAIL(...) __VA_ARGS__
#endif

#if MUU_CLANG || MUU_GCC_LIKE || MUU_ICC || MUU_MSVC_LIKE || MUU_HAS_BUILTIN(__builtin_offsetof)
	#define MUU_OFFSETOF(type, member) __builtin_offsetof(type, member)
#else
	#ifndef offsetof
MUU_DISABLE_WARNINGS;
		#include <cstddef>
MUU_ENABLE_WARNINGS;
	#endif
	#define MUU_OFFSETOF(type, member) offsetof(type, member)
#endif

//======================================================================================================================
// SFINAE AND CONCEPTS
//======================================================================================================================

#if defined(__cpp_concepts) && __cpp_concepts >= 201907
	#define MUU_CONCEPTS 1
#else
	#define MUU_CONCEPTS 0
#endif

/// \cond
namespace muu::impl
{
	template <bool, typename T = void>
	struct enable_if_;

	template <typename T>
	struct enable_if_<true, T>
	{
		using type = T;
	};
}
#define MUU_ENABLE_IF(...) , typename ::muu::impl::enable_if_<(__VA_ARGS__), int>::type = 0

#if MUU_CONCEPTS
	#define MUU_REQUIRES(...) requires(__VA_ARGS__)
#endif

#define MUU_HIDDEN_CONSTRAINT(condition, ...)                                                                          \
	template <__VA_ARGS__ MUU_ENABLE_IF(condition)>                                                                    \
	MUU_REQUIRES(condition)

#define MUU_CONSTRAINED_TEMPLATE(condition, ...)                                                                       \
	template <__VA_ARGS__ MUU_ENABLE_IF(condition)>                                                                    \
	MUU_REQUIRES(condition)
/// \endcond

#ifndef MUU_REQUIRES
	#define MUU_REQUIRES(...)
#endif
#ifndef MUU_ENABLE_IF
	#define MUU_ENABLE_IF(...)
#endif
#ifndef MUU_HIDDEN_CONSTRAINT
	#define MUU_HIDDEN_CONSTRAINT(condition, ...)
#endif
#ifndef MUU_CONSTRAINED_TEMPLATE
	#define MUU_CONSTRAINED_TEMPLATE(condition, ...) template <__VA_ARGS__>
#endif
#if MUU_CONCEPTS && defined(__cpp_lib_concepts) && __cpp_lib_concepts >= 202002
	#define MUU_STD_CONCEPT(...) __VA_ARGS__
#else
	#define MUU_STD_CONCEPT(...) true
#endif

#define MUU_COMMA			  ,
#define MUU_HIDDEN_PARAM(...) MUU_HIDDEN(MUU_COMMA __VA_ARGS__)

//======================================================================================================================
// WHAT THE HELL IS WCHAR_T?
//======================================================================================================================

#if MUU_WINDOWS
	#define MUU_WCHAR_BYTES 2
#elif defined(__SIZEOF_WCHAR_T__)
	#define MUU_WCHAR_BYTES __SIZEOF_WCHAR_T__
#else
	#error Could not determine MUU_WCHAR_BYTES!
#endif

// Q: "why not #define MUU_WCHAR_BITS (MUU_WCHAR_BYTES * 8)?
// A: Because it's converted to a string literal by the preprocessor in a few places
//    so needs to simply be a single integer. Meh.

#if MUU_WCHAR_BYTES == 8
	#define MUU_WCHAR_BITS 64
#elif MUU_WCHAR_BYTES == 4
	#define MUU_WCHAR_BITS 32
#elif MUU_WCHAR_BYTES == 2
	#define MUU_WCHAR_BITS 16
#elif MUU_WCHAR_BYTES == 1
	#define MUU_WCHAR_BITS 8
#else
	#error Could not determine MUU_WCHAR_BITS!
#endif

//======================================================================================================================
// EXTENDED FLOAT TYPES
//======================================================================================================================

#ifdef __FLT16_MANT_DIG__
	#define MUU_FLOAT16_MANT_DIG   __FLT16_MANT_DIG__
	#define MUU_FLOAT16_DIG		   __FLT16_DIG__
	#define MUU_FLOAT16_MIN_EXP	   __FLT16_MIN_EXP__
	#define MUU_FLOAT16_MIN_10_EXP __FLT16_MIN_10_EXP__
	#define MUU_FLOAT16_MAX_EXP	   __FLT16_MAX_EXP__
	#define MUU_FLOAT16_MAX_10_EXP __FLT16_MAX_10_EXP__
	#if 0
		#pragma message("__FLT16_MANT_DIG__   " MUU_MAKE_STRING(__FLT16_MANT_DIG__))
		#pragma message("__FLT16_DIG__        " MUU_MAKE_STRING(__FLT16_DIG__))
		#pragma message("__FLT16_MIN_EXP__    " MUU_MAKE_STRING(__FLT16_MIN_EXP__))
		#pragma message("__FLT16_MIN_10_EXP__ " MUU_MAKE_STRING(__FLT16_MIN_10_EXP__))
		#pragma message("__FLT16_MAX_EXP__    " MUU_MAKE_STRING(__FLT16_MAX_EXP__))
		#pragma message("__FLT16_MAX_10_EXP__ " MUU_MAKE_STRING(__FLT16_MAX_10_EXP__))
	#endif
#else
	#define MUU_FLOAT16_MANT_DIG   0
	#define MUU_FLOAT16_DIG		   0
	#define MUU_FLOAT16_MIN_EXP	   0
	#define MUU_FLOAT16_MIN_10_EXP 0
	#define MUU_FLOAT16_MAX_EXP	   0
	#define MUU_FLOAT16_MAX_10_EXP 0
#endif

#if (MUU_FLOAT16_MANT_DIG && MUU_FLOAT16_DIG && MUU_FLOAT16_MIN_EXP && MUU_FLOAT16_MIN_10_EXP && MUU_FLOAT16_MAX_EXP   \
	 && MUU_FLOAT16_MAX_10_EXP)
	#define MUU_FLOAT16_LIMITS_SET 1
#else
	#define MUU_FLOAT16_LIMITS_SET 0
#endif

#if (MUU_FLOAT16_MANT_DIG == 11 && MUU_FLOAT16_DIG == 3 && MUU_FLOAT16_MIN_EXP == -13 && MUU_FLOAT16_MIN_10_EXP == -4  \
	 && MUU_FLOAT16_MAX_EXP == 16 && MUU_FLOAT16_MAX_10_EXP == 4)
	#define MUU_FLOAT16_LIMITS_IEEE754 1
#else
	#define MUU_FLOAT16_LIMITS_IEEE754 0
#endif

#if MUU_CLANG // >= 15
	/*
		_Float16 is currently only supported on the following targets,
		with further targets pending ABI standardization:

			32-bit ARM
			64-bit ARM (AArch64)
			AMDGPU
			SPIR
			X86 as long as SSE2 is available

		- https://clang.llvm.org/docs/LanguageExtensions.html
	*/
	#if (MUU_ARCH_ARM || MUU_ARCH_X86 || MUU_ARCH_AMD64) && MUU_FLOAT16_LIMITS_SET
		#define MUU_HAS_FLOAT16 1
		#define MUU_HAS_FP16	1
	#endif
#elif MUU_GCC
	/*
		The _Float16 type is supported on AArch64 systems by default, on ARM systems when the IEEE format for
		16-bit floating-point types is selected with -mfp16-format=ieee and,
		for both C and C++, on x86 systems with SSE2 enabled.

		- https://gcc.gnu.org/onlinedocs/gcc/Floating-Types.html

		*** except: the bit about x86 seems incorrect?? ***
	 */
	#if (MUU_ARCH_ARM /*|| MUU_ARCH_X86 || MUU_ARCH_AMD64*/) && MUU_FLOAT16_LIMITS_SET
		#define MUU_HAS_FLOAT16 1
		#define MUU_HAS_FP16	1
	#endif
#endif
#ifndef MUU_HAS_FLOAT16
	#define MUU_HAS_FLOAT16 MUU_DOXYGEN
#endif
/// \def MUU_HAS_FLOAT16
/// \brief `1` when the target environment has the 16-bit floating point type _Float16.
/// \remarks This is completely unrelated to the class muu::half, which is always available.

#ifndef MUU_HAS_FP16
	#if MUU_CLANG
		#if MUU_CLANG >= 15 || MUU_ARCH_ARM
			#define MUU_HAS_FP16 1
		#endif
	#elif MUU_GCC
		#if MUU_ARCH_ARM
			#define MUU_HAS_FP16 1
		#endif
	#endif
#endif
#ifndef MUU_HAS_FP16
	#define MUU_HAS_FP16 MUU_DOXYGEN
#endif
/// \def MUU_HAS_FP16
/// \brief `1` when the target environment has the storage-only 16-bit floating point type __fp16.
/// \remarks This is completely unrelated to the class muu::half, which is always available.

#if defined(__SIZEOF_FLOAT128__) && defined(__FLT128_MANT_DIG__)
	#define MUU_HAS_FLOAT128 1
#else
	#define MUU_HAS_FLOAT128 MUU_DOXYGEN
#endif
/// \def MUU_HAS_FLOAT128
/// \brief `1` when the target environment has 128-bit floats, otherwise `0`.
/// \see #muu::float128_t

#if MUU_HAS_FLOAT128 && MUU_HAS_INCLUDE(<quadmath.h>)
	#define MUU_HAS_QUADMATH 1
#else
	#define MUU_HAS_QUADMATH 0
#endif

//======================================================================================================================
// ASSERT
//======================================================================================================================

#ifndef MUU_ASSERT
	#ifdef NDEBUG
		#define MUU_ASSERT(cond) MUU_NOOP
	#else
		#ifndef assert
MUU_DISABLE_WARNINGS;
			#include <cassert>
MUU_ENABLE_WARNINGS;
		#endif
		#define MUU_ASSERT(cond) assert(cond)
	#endif
#endif
#ifdef NDEBUG
	// ensure any overrides respect NDEBUG
	#undef MUU_ASSERT
	#define MUU_ASSERT(cond)				MUU_NOOP
	#define MUU_CONSTEXPR_SAFE_ASSERT(cond) MUU_NOOP
#else
	#define MUU_CONSTEXPR_SAFE_ASSERT(cond)                                                                            \
		do                                                                                                             \
		{                                                                                                              \
			if constexpr (::muu::build::supports_is_constant_evaluated)                                                \
			{                                                                                                          \
				MUU_IF_RUNTIME                                                                                         \
				{                                                                                                      \
					MUU_ASSERT(cond);                                                                                  \
				}                                                                                                      \
			}                                                                                                          \
		}                                                                                                              \
		while (false)
#endif

/// @}
