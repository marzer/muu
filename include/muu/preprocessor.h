// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

// clang-format off

/// \file
/// \brief Compiler feature detection, attributes, string-makers, etc.

#pragma once
#ifndef __cplusplus
	#error muu is a C++ library.
#endif

//=====================================================================================================================
// COMPILER DETECTION
//=====================================================================================================================

#ifdef __INTELLISENSE__
	#define MUU_INTELLISENSE	1
#else
	#define MUU_INTELLISENSE	0
#endif
#ifdef __clang__
	#define MUU_CLANG		__clang_major__
#else
	#define MUU_CLANG		0
#endif
#ifdef __INTEL_COMPILER
	#define MUU_ICC			__INTEL_COMPILER
	#ifdef __ICL
		#define MUU_ICC_CL	MUU_ICC
	#else
		#define MUU_ICC_CL	0
	#endif
#else
	#define MUU_ICC			0
	#define MUU_ICC_CL		0
#endif
#if defined(_MSC_VER) && !MUU_CLANG && !MUU_ICC
	#define MUU_MSVC		_MSC_VER
#else
	#define MUU_MSVC		0
#endif
#if defined(__GNUC__) && !MUU_CLANG && !MUU_ICC
	#define MUU_GCC			__GNUC__
#else
	#define MUU_GCC			0
#endif
#if !MUU_CLANG && !MUU_ICC && !MUU_MSVC && !MUU_GCC
	#error Unknown compiler.
#endif
#if (MUU_CLANG && (MUU_ICC || MUU_MSVC || MUU_GCC)) \
	|| (MUU_ICC && (MUU_CLANG || MUU_MSVC || MUU_GCC)) \
	|| (MUU_MSVC && (MUU_CLANG || MUU_ICC || MUU_GCC)) \
	|| (MUU_GCC && (MUU_CLANG || MUU_ICC || MUU_MSVC))
	#error Could not uniquely identify compiler.
#endif

//=====================================================================================================================
// ARCHITECTURE & ENVIRONMENT
//=====================================================================================================================

// MUU_ARCH_ITANIUM
#if defined(__ia64__) || defined(__ia64) || defined(_IA64) || defined(__IA64__) || defined(_M_IA64)
	#define MUU_ARCH_ITANIUM 1
	#define MUU_ARCH_BITNESS 64
#else
	#define MUU_ARCH_ITANIUM 0
#endif

// MUU_ARCH_AMD64
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_AMD64) || defined(DOXYGEN)
	#define MUU_ARCH_AMD64 1
	#define MUU_ARCH_BITNESS 64
#else
	#define MUU_ARCH_AMD64 0
#endif

// MUU_ARCH_X86
#if defined(__i386__) || defined(_M_IX86)
	#define MUU_ARCH_X86 1
	#define MUU_ARCH_BITNESS 32
#else
	#define MUU_ARCH_X86 0
#endif

// MUU_ARCH_ARM32 and MUU_ARCH_ARM64
#if defined(__aarch64__) || defined(__ARM_ARCH_ISA_A64) || defined(_M_ARM64) || defined(__ARM_64BIT_STATE)
	#define MUU_ARCH_ARM32 0
	#define MUU_ARCH_ARM64 1
	#define MUU_ARCH_ARM 1
	#define MUU_ARCH_BITNESS 64
#elif defined(__arm__) || defined(_M_ARM) || defined(__ARM_32BIT_STATE)
	#define MUU_ARCH_ARM32 1
	#define MUU_ARCH_ARM64 0
	#define MUU_ARCH_ARM 1
	#define MUU_ARCH_BITNESS 32
#else
	#define MUU_ARCH_ARM32 0
	#define MUU_ARCH_ARM64 0
	#define MUU_ARCH_ARM 0
#endif

// MUU_ARCH_X64
#if MUU_ARCH_BITNESS == 64
	#define MUU_ARCH_X64 1
#else
	#define MUU_ARCH_X64 0
#endif

#ifndef DOXYGEN
	#define MUU_ARCH_SUM (MUU_ARCH_ITANIUM + MUU_ARCH_AMD64 + MUU_ARCH_X86 + MUU_ARCH_ARM32 + MUU_ARCH_ARM64)
	#if MUU_ARCH_SUM > 1
		#error Could not uniquely identify target architecture.
	#elif MUU_ARCH_SUM == 0
		#error Unknown target architecture.
	#endif
	#undef MUU_ARCH_SUM
	#ifndef MUU_ARCH_BITNESS
		#error Unknown target architecture bitness.
	#endif
#endif
#ifdef _WIN32
	#define MUU_WINDOWS 1
#else
	#define MUU_WINDOWS 0
#endif
#ifndef MUU_BUILDING
	#define MUU_BUILDING 0
#endif

#define MUU_CONCAT_1(x, y) x##y
#define MUU_CONCAT(x, y) MUU_CONCAT_1(x, y)

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

//=====================================================================================================================
// CLANG
//=====================================================================================================================

#if MUU_CLANG

	#if MUU_CLANG >= 7
		#define MUU_MACRO_DISPATCH_CLANG_GE_7(...)	__VA_ARGS__
		#define MUU_MACRO_DISPATCH_CLANG_LT_7(...)
	#else
		#define MUU_MACRO_DISPATCH_CLANG_GE_7(...)
		#define MUU_MACRO_DISPATCH_CLANG_LT_7(...)	__VA_ARGS__
	#endif
	#if MUU_CLANG >= 8
		#define MUU_MACRO_DISPATCH_CLANG_GE_8(...)	__VA_ARGS__
		#define MUU_MACRO_DISPATCH_CLANG_LT_8(...)
	#else
		#define MUU_MACRO_DISPATCH_CLANG_GE_8(...)
		#define MUU_MACRO_DISPATCH_CLANG_LT_8(...)	__VA_ARGS__
	#endif
	#if MUU_CLANG >= 9
		#define MUU_MACRO_DISPATCH_CLANG_GE_9(...)	__VA_ARGS__
		#define MUU_MACRO_DISPATCH_CLANG_LT_9(...)
	#else
		#define MUU_MACRO_DISPATCH_CLANG_GE_9(...)
		#define MUU_MACRO_DISPATCH_CLANG_LT_9(...)	__VA_ARGS__
	#endif
	#if MUU_CLANG >= 10
		#define MUU_MACRO_DISPATCH_CLANG_GE_10(...)	__VA_ARGS__
		#define MUU_MACRO_DISPATCH_CLANG_LT_10(...)
	#else
		#define MUU_MACRO_DISPATCH_CLANG_GE_10(...)
		#define MUU_MACRO_DISPATCH_CLANG_LT_10(...)	__VA_ARGS__
	#endif
	#if MUU_CLANG >= 11
		#define MUU_MACRO_DISPATCH_CLANG_GE_11(...)	__VA_ARGS__
		#define MUU_MACRO_DISPATCH_CLANG_LT_11(...)
	#else
		#define MUU_MACRO_DISPATCH_CLANG_GE_11(...)
		#define MUU_MACRO_DISPATCH_CLANG_LT_11(...)	__VA_ARGS__
	#endif

	#define MUU_PRAGMA_CLANG(decl)			_Pragma(MUU_MAKE_STRING(clang decl))
	#define MUU_PRAGMA_CLANG_GE(ver, decl)	MUU_CONCAT(MUU_MACRO_DISPATCH_CLANG_GE_, ver)(MUU_PRAGMA_CLANG(decl))
	#define MUU_PRAGMA_CLANG_LT(ver, decl)	MUU_CONCAT(MUU_MACRO_DISPATCH_CLANG_LT_, ver)(MUU_PRAGMA_CLANG(decl))

	#define MUU_ATTR(...)					__attribute__((__VA_ARGS__))
	#define MUU_ATTR_CLANG(...)				MUU_ATTR(__VA_ARGS__)
	#define MUU_ATTR_CLANG_GE(ver, ...)		MUU_CONCAT(MUU_MACRO_DISPATCH_CLANG_GE_, ver)(MUU_ATTR(__VA_ARGS__))
	#define MUU_ATTR_CLANG_LT(ver, ...)		MUU_CONCAT(MUU_MACRO_DISPATCH_CLANG_LT_, ver)(MUU_ATTR(__VA_ARGS__))

	#define MUU_PUSH_WARNINGS				MUU_PRAGMA_CLANG(diagnostic push)
	#define MUU_DISABLE_SWITCH_WARNINGS		MUU_PRAGMA_CLANG(diagnostic ignored "-Wswitch")
	#define MUU_DISABLE_LIFETIME_WARNINGS	MUU_PRAGMA_CLANG(diagnostic ignored "-Wmissing-field-initializers")	\
											MUU_PRAGMA_CLANG(diagnostic ignored "-Wglobal-constructors")	\
											MUU_PRAGMA_CLANG(diagnostic ignored "-Wexit-time-destructors")
	#define MUU_DISABLE_ARITHMETIC_WARNINGS	MUU_PRAGMA_CLANG(diagnostic ignored "-Wfloat-equal") \
											MUU_PRAGMA_CLANG(diagnostic ignored "-Wdouble-promotion") \
											MUU_PRAGMA_CLANG(diagnostic ignored "-Wchar-subscripts") \
											MUU_PRAGMA_CLANG(diagnostic ignored "-Wshift-sign-overflow") \
									 MUU_PRAGMA_CLANG_GE(10, diagnostic ignored "-Wimplicit-int-float-conversion")
	#define MUU_DISABLE_SHADOW_WARNINGS		MUU_PRAGMA_CLANG(diagnostic ignored "-Wshadow")	\
											MUU_PRAGMA_CLANG(diagnostic ignored "-Wshadow-field")
	#define MUU_DISABLE_SPAM_WARNINGS		MUU_PRAGMA_CLANG(diagnostic ignored "-Wweak-vtables")			\
											MUU_PRAGMA_CLANG(diagnostic ignored "-Wdouble-promotion")		\
											MUU_PRAGMA_CLANG(diagnostic ignored "-Wweak-template-vtables")	\
											MUU_PRAGMA_CLANG(diagnostic ignored "-Wpadded")					\
									  MUU_PRAGMA_CLANG_GE(9, diagnostic ignored "-Wctad-maybe-unsupported") \
											MUU_PRAGMA_CLANG(diagnostic ignored "-Wc++2a-compat")			\
											MUU_PRAGMA_CLANG(diagnostic ignored "-Wpacked")
	#define MUU_POP_WARNINGS				MUU_PRAGMA_CLANG(diagnostic pop)
	#define MUU_DISABLE_WARNINGS			MUU_PUSH_WARNINGS \
											MUU_PRAGMA_CLANG(diagnostic ignored "-Weverything")
	#define MUU_ENABLE_WARNINGS				MUU_POP_WARNINGS
	#define MUU_ASSUME(cond)				__builtin_assume(cond)
	#define MUU_UNREACHABLE					__builtin_unreachable()
	#define MUU_ALIGN(alignment)			MUU_ATTR(aligned(alignment))
	#if defined(_MSC_VER) // msvc compat mode
		#ifdef __has_declspec_attribute
			#define MUU_DECLSPEC(attr)		__declspec(attr)
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
			#if __has_declspec_attribute(noinline)
				#define MUU_NEVER_INLINE	__declspec(noinline)
			#endif
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
	#if !__has_feature(cxx_rtti)
		#define MUU_RTTI 0
	#endif
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		#define MUU_LITTLE_ENDIAN 1
	#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		#define MUU_BIG_ENDIAN 1
	#endif
	#define MUU_LIKELY(...)					(__builtin_expect(!!(__VA_ARGS__), 1) )
	#define MUU_UNLIKELY(...)				(__builtin_expect(!!(__VA_ARGS__), 0) )
	#ifdef __has_builtin
		#define MUU_HAS_BUILTIN(name)		__has_builtin(name)
	#endif
	#define MUU_OFFSETOF(s, m)				__builtin_offsetof(s,m)

#endif // clang

//=====================================================================================================================
// MSVC
//=====================================================================================================================

#if MUU_MSVC || MUU_ICC_CL

	#define MUU_CPP_VERSION					_MSVC_LANG
	#if MUU_MSVC // !intel-cl
		#define MUU_PRAGMA_MSVC(...)		__pragma(__VA_ARGS__)
		#define MUU_PUSH_WARNINGS			__pragma(warning(push))
		#define MUU_DISABLE_SWITCH_WARNINGS	__pragma(warning(disable: 4063))
		#define MUU_DISABLE_SHADOW_WARNINGS	__pragma(warning(disable: 4458))
		#define MUU_DISABLE_SPAM_WARNINGS	__pragma(warning(disable: 4127)) /* conditional expr is constant */ \
											__pragma(warning(disable: 4324)) /* structure was padded due to alignment specifier */
		#define MUU_POP_WARNINGS			__pragma(warning(pop))
		#define MUU_DISABLE_WARNINGS		__pragma(warning(push, 0)) \
											__pragma(warning(disable: 4348))
		#define MUU_ENABLE_WARNINGS			MUU_POP_WARNINGS
	#endif
	#define MUU_DECLSPEC(attr)				__declspec(attr)
	#define MUU_ALIGN(alignment)			MUU_DECLSPEC(align(alignment))
	#define MUU_ALWAYS_INLINE				__forceinline
	#define MUU_NEVER_INLINE				__declspec(noinline)
	#define MUU_ASSUME(cond)				__assume(cond)
	#define MUU_UNREACHABLE					__assume(0)
	#if !MUU_INTELLISENSE
		#define MUU_INTERFACE				__declspec(novtable)
		#define MUU_EMPTY_BASES				__declspec(empty_bases)
	#endif
	#define MUU_UNALIASED_ALLOC				__declspec(restrict)
	#define MUU_VECTORCALL					__vectorcall
	#ifndef _CPPRTTI
		#define MUU_RTTI 0
	#endif
	#define MUU_LITTLE_ENDIAN				1
	#define MUU_BIG_ENDIAN					0
	#define MUU_OFFSETOF(s, m)				__builtin_offsetof(s,m)

#endif // msvc (and icc in msvc mode)

//=====================================================================================================================
// ICC
//=====================================================================================================================

#if MUU_ICC
	
	#define MUU_PRAGMA_ICC(...)				__pragma(__VA_ARGS__)
	#define MUU_PUSH_WARNINGS				__pragma(warning(push))
	#define MUU_DISABLE_SPAM_WARNINGS		__pragma(warning(disable: 82))	/* storage class is not first */ \
											__pragma(warning(disable: 111))	/* statement unreachable (false-positive) */ \
											__pragma(warning(disable: 869)) /* unreferenced parameter */ \
											__pragma(warning(disable: 1011)) /* missing return (false-positive) */ \
											__pragma(warning(disable: 2261)) /* assume expr side-effects discarded */
	#define MUU_POP_WARNINGS				__pragma(warning(pop))
	#define MUU_DISABLE_WARNINGS			__pragma(warning(push, 0)) MUU_DISABLE_SPAM_WARNINGS
	#define MUU_ENABLE_WARNINGS				MUU_POP_WARNINGS
	#ifndef MUU_OFFSETOF
		#define MUU_OFFSETOF(s, m)			__builtin_offsetof(s,m) // ??
	#endif
	#ifndef MUU_ASSUME
		#define MUU_ASSUME(cond)			__builtin_assume(cond) // ??
	#endif
	#ifndef MUU_UNREACHABLE
		#define MUU_UNREACHABLE				__builtin_unreachable() // ??
	#endif
	#define MUU_UNREACHABLE_RETURN(r)		MUU_UNREACHABLE; return r

#endif // icc

//=====================================================================================================================
// GCC
//=====================================================================================================================

#if MUU_GCC

	#if MUU_GCC >= 7
		#define MUU_MACRO_DISPATCH_GCC_GE_7(...)	__VA_ARGS__
		#define MUU_MACRO_DISPATCH_GCC_LT_7(...)
	#else
		#define MUU_MACRO_DISPATCH_GCC_GE_7(...)
		#define MUU_MACRO_DISPATCH_GCC_LT_7(...)	__VA_ARGS__
	#endif
	#if MUU_GCC >= 8
		#define MUU_MACRO_DISPATCH_GCC_GE_8(...)	__VA_ARGS__
		#define MUU_MACRO_DISPATCH_GCC_LT_8(...)
	#else
		#define MUU_MACRO_DISPATCH_GCC_GE_8(...)
		#define MUU_MACRO_DISPATCH_GCC_LT_8(...)	__VA_ARGS__
	#endif
	#if MUU_GCC >= 9
		#define MUU_MACRO_DISPATCH_GCC_GE_9(...)	__VA_ARGS__
		#define MUU_MACRO_DISPATCH_GCC_LT_9(...)
	#else
		#define MUU_MACRO_DISPATCH_GCC_GE_9(...)
		#define MUU_MACRO_DISPATCH_GCC_LT_9(...)	__VA_ARGS__
	#endif
	#if MUU_GCC >= 10
		#define MUU_MACRO_DISPATCH_GCC_GE_10(...)	__VA_ARGS__
		#define MUU_MACRO_DISPATCH_GCC_LT_10(...)
	#else
		#define MUU_MACRO_DISPATCH_GCC_GE_10(...)
		#define MUU_MACRO_DISPATCH_GCC_LT_10(...)	__VA_ARGS__
	#endif
	#if MUU_GCC >= 11
		#define MUU_MACRO_DISPATCH_GCC_GE_11(...)	__VA_ARGS__
		#define MUU_MACRO_DISPATCH_GCC_LT_11(...)
	#else
		#define MUU_MACRO_DISPATCH_GCC_GE_11(...)
		#define MUU_MACRO_DISPATCH_GCC_LT_11(...)	__VA_ARGS__
	#endif

	#define MUU_PRAGMA_GCC(decl)			_Pragma(MUU_MAKE_STRING(GCC decl))
	#define MUU_PRAGMA_GCC_GE(ver, decl)	MUU_CONCAT(MUU_MACRO_DISPATCH_GCC_GE_, ver)(MUU_PRAGMA_GCC(decl))
	#define MUU_PRAGMA_GCC_LT(ver, decl)	MUU_CONCAT(MUU_MACRO_DISPATCH_GCC_LT_, ver)(MUU_PRAGMA_GCC(decl))

	#define MUU_ATTR(...)					__attribute__((__VA_ARGS__))
	#define MUU_ATTR_GCC(...)				MUU_ATTR(__VA_ARGS__)
	#define MUU_ATTR_GCC_GE(ver, ...)		MUU_CONCAT(MUU_MACRO_DISPATCH_GCC_GE_, ver)(MUU_ATTR(__VA_ARGS__))
	#define MUU_ATTR_GCC_LT(ver, ...)		MUU_CONCAT(MUU_MACRO_DISPATCH_GCC_LT_, ver)(MUU_ATTR(__VA_ARGS__))

	#define MUU_PUSH_WARNINGS				MUU_PRAGMA_GCC(diagnostic push)
	#define MUU_DISABLE_SWITCH_WARNINGS		MUU_PRAGMA_GCC(diagnostic ignored "-Wswitch")						\
											MUU_PRAGMA_GCC(diagnostic ignored "-Wswitch-enum")					\
											MUU_PRAGMA_GCC(diagnostic ignored "-Wswitch-default")
	#define MUU_DISABLE_LIFETIME_WARNINGS	MUU_PRAGMA_GCC(diagnostic ignored "-Wmissing-field-initializers")	\
											MUU_PRAGMA_GCC(diagnostic ignored "-Wmaybe-uninitialized")			\
											MUU_PRAGMA_GCC(diagnostic ignored "-Wuninitialized")				\
									  MUU_PRAGMA_GCC_GE(8, diagnostic ignored "-Wclass-memaccess")
	#define MUU_DISABLE_ARITHMETIC_WARNINGS	MUU_PRAGMA_GCC(diagnostic ignored "-Wfloat-equal")					\
											MUU_PRAGMA_GCC(diagnostic ignored "-Wsign-conversion")				\
											MUU_PRAGMA_GCC(diagnostic ignored "-Wchar-subscripts")
	#define MUU_DISABLE_SHADOW_WARNINGS		MUU_PRAGMA_GCC(diagnostic ignored "-Wshadow")
	#define MUU_DISABLE_SUGGEST_WARNINGS	MUU_PRAGMA_GCC(diagnostic ignored "-Wsuggest-attribute=const")		\
											MUU_PRAGMA_GCC(diagnostic ignored "-Wsuggest-attribute=pure")
	#define MUU_DISABLE_SPAM_WARNINGS		MUU_PRAGMA_GCC(diagnostic ignored "-Wpadded")						\
											MUU_PRAGMA_GCC(diagnostic ignored "-Wcast-align")					\
											MUU_PRAGMA_GCC(diagnostic ignored "-Wcomment")						\
											MUU_PRAGMA_GCC(diagnostic ignored "-Wsubobject-linkage")			\
											MUU_PRAGMA_GCC(diagnostic ignored "-Wtype-limits")
	#define MUU_POP_WARNINGS				MUU_PRAGMA_GCC(diagnostic pop)
	#define MUU_DISABLE_WARNINGS			MUU_PUSH_WARNINGS \
											MUU_PRAGMA_GCC(diagnostic ignored "-Wall")							\
											MUU_PRAGMA_GCC(diagnostic ignored "-Wextra")						\
											MUU_PRAGMA_GCC(diagnostic ignored "-Wpedantic")						\
											MUU_DISABLE_SWITCH_WARNINGS											\
											MUU_DISABLE_LIFETIME_WARNINGS										\
											MUU_DISABLE_ARITHMETIC_WARNINGS										\
											MUU_DISABLE_SHADOW_WARNINGS											\
											MUU_DISABLE_SUGGEST_WARNINGS										\
											MUU_DISABLE_SPAM_WARNINGS
	#define MUU_ENABLE_WARNINGS				MUU_POP_WARNINGS
	#define MUU_ALIGN(alignment)			MUU_ATTR(aligned(alignment))
	#define MUU_ALWAYS_INLINE				__attribute__((__always_inline__)) inline
	#define MUU_NEVER_INLINE				__attribute__((__noinline__))
	#define MUU_UNREACHABLE					__builtin_unreachable()
	#define MUU_UNALIASED_ALLOC				__attribute__((__malloc__))
	#ifndef __GXX_RTTI
		#define MUU_RTTI 0
	#endif
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		#define MUU_LITTLE_ENDIAN 1
	#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		#define MUU_BIG_ENDIAN 1
	#endif
	#define MUU_LIKELY(...)					(__builtin_expect(!!(__VA_ARGS__), 1) )
	#define MUU_UNLIKELY(...)				(__builtin_expect(!!(__VA_ARGS__), 0) )
	#ifdef __has_builtin
		#define MUU_HAS_BUILTIN(name)		__has_builtin(name)
	#endif
	#define MUU_OFFSETOF(s, m)				__builtin_offsetof(s,m)

#endif // gcc

//=====================================================================================================================
// USER CONFIGURATION
//=====================================================================================================================

#ifdef MUU_CONFIG_HEADER
	#include MUU_CONFIG_HEADER
#endif

#if !MUU_WINDOWS
	#undef MUU_DLL
#endif
#ifndef MUU_DLL
	#define MUU_DLL 0
#endif

#ifndef MUU_API
	#if MUU_DLL
		#if MUU_BUILDING
			#define MUU_API __declspec(dllexport)
		#else
			#define MUU_API __declspec(dllimport)
		#endif
	#else
		#define MUU_API
	#endif
#endif

#ifndef MUU_EXTENDED_LITERALS
	#define MUU_EXTENDED_LITERALS 0
#endif

//=====================================================================================================================
// ATTRIBUTES, UTILITY MACROS ETC
//=====================================================================================================================

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

#ifdef __has_include
	#define MUU_HAS_INCLUDE(header)		__has_include(header)
#else
	#define MUU_HAS_INCLUDE(header)		0
#endif
#if MUU_HAS_INCLUDE(<version>)
	#include <version>
#endif

#ifndef MUU_HAS_BUILTIN
	#define MUU_HAS_BUILTIN(name)		0
#endif

#if defined(__EXCEPTIONS) || defined(_CPPUNWIND) || defined(__cpp_exceptions)
	#define MUU_EXCEPTIONS 1
#else
	#define MUU_EXCEPTIONS 0
#endif

#ifndef MUU_RTTI
	#define MUU_RTTI 1
#endif

#ifndef MUU_LITTLE_ENDIAN
	#ifdef DOXYGEN
		#define MUU_LITTLE_ENDIAN 1
	#else
		#define MUU_LITTLE_ENDIAN 0
	#endif
#endif
#ifndef MUU_BIG_ENDIAN
	#define MUU_BIG_ENDIAN 0
#endif
#if MUU_BIG_ENDIAN == MUU_LITTLE_ENDIAN
	#error Unknown platform endianness.
#endif

#ifndef MUU_PRAGMA_CLANG
	#define MUU_PRAGMA_CLANG(...)
	#define MUU_PRAGMA_CLANG_GE(...)
	#define MUU_PRAGMA_CLANG_LT(...)
#endif
#ifndef MUU_PRAGMA_GCC
	#define MUU_PRAGMA_GCC(...)
	#define MUU_PRAGMA_GCC_GE(...)
	#define MUU_PRAGMA_GCC_LT(...)
#endif
#ifndef MUU_PRAGMA_ICC
	#define MUU_PRAGMA_ICC(...)
#endif
#ifndef MUU_PRAGMA_MSVC
	#define MUU_PRAGMA_MSVC(...)
#endif

#ifndef MUU_ATTR
	#define MUU_ATTR(...)
#endif
#ifdef NDEBUG
	#define MUU_ATTR_NDEBUG(...)	MUU_ATTR(__VA_ARGS__)
#else
	#define MUU_ATTR_NDEBUG(...)
#endif
#ifndef MUU_ATTR_CLANG
	#define MUU_ATTR_CLANG(...)
	#define MUU_ATTR_CLANG_GE(...)
	#define MUU_ATTR_CLANG_LT(...)
#endif
#ifndef MUU_ATTR_GCC
	#define MUU_ATTR_GCC(...)
	#define MUU_ATTR_GCC_GE(...)
	#define MUU_ATTR_GCC_LT(...)
#endif
#ifndef MUU_DECLSPEC
	#define MUU_DECLSPEC(attr)
#endif
#ifndef MUU_ALIGN
	#define MUU_ALIGN(alignment)	alignas(alignment)
#endif

#ifndef MUU_PUSH_WARNINGS
	#define MUU_PUSH_WARNINGS
#endif
#ifndef MUU_DISABLE_SWITCH_WARNINGS
	#define	MUU_DISABLE_SWITCH_WARNINGS
#endif
#ifndef MUU_DISABLE_LIFETIME_WARNINGS
	#define	MUU_DISABLE_LIFETIME_WARNINGS
#endif
#ifndef MUU_DISABLE_SPAM_WARNINGS
	#define MUU_DISABLE_SPAM_WARNINGS
#endif
#ifndef MUU_DISABLE_ARITHMETIC_WARNINGS
	#define MUU_DISABLE_ARITHMETIC_WARNINGS
#endif
#ifndef MUU_DISABLE_SHADOW_WARNINGS
	#define MUU_DISABLE_SHADOW_WARNINGS
#endif
#ifndef MUU_DISABLE_SUGGEST_WARNINGS
	#define MUU_DISABLE_SUGGEST_WARNINGS
#endif
#ifndef MUU_DISABLE_WARNINGS
	#define MUU_DISABLE_WARNINGS
#endif
#ifndef MUU_ENABLE_WARNINGS
	#define MUU_ENABLE_WARNINGS
#endif
#ifndef MUU_POP_WARNINGS
	#define MUU_POP_WARNINGS
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
	#define MUU_UNREACHABLE				(void)0
#endif
#ifndef MUU_UNREACHABLE_RETURN
	#define MUU_UNREACHABLE_RETURN(r)	MUU_UNREACHABLE
#endif

#define MUU_NO_DEFAULT_CASE				default: MUU_UNREACHABLE

#if !defined(DOXYGEN) && !MUU_INTELLISENSE
	#if !defined(MUU_LIKELY) && __has_cpp_attribute(likely)
		#define MUU_LIKELY(...)	(__VA_ARGS__) [[likely]]
	#endif
	#if !defined(MUU_UNLIKELY) && __has_cpp_attribute(unlikely)
		#define MUU_UNLIKELY(...)	(__VA_ARGS__) [[unlikely]]
	#endif
	#if !defined(MUU_NO_UNIQUE_ADDRESS) && __has_cpp_attribute(no_unique_address)
		#define MUU_NO_UNIQUE_ADDRESS	[[no_unique_address]]
	#endif
	#if __has_cpp_attribute(nodiscard) >= 201907L
		#define MUU_NODISCARD_CTOR		[[nodiscard]]
	#endif
#endif
#ifndef MUU_LIKELY
	#define MUU_LIKELY(...)	(__VA_ARGS__)
#endif
#ifndef MUU_UNLIKELY
	#define MUU_UNLIKELY(...)	(__VA_ARGS__)
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

#if (!MUU_ARCH_X86 && !MUU_ARCH_AMD64) || !MUU_ISET_SSE2
	#undef MUU_VECTORCALL
#endif
#ifdef MUU_VECTORCALL
	#define MUU_HAS_VECTORCALL 1
#else
	#define MUU_HAS_VECTORCALL 0
	#define MUU_VECTORCALL
#endif

#if defined(__cpp_consteval) && !MUU_INTELLISENSE
	#define MUU_CONSTEVAL				consteval
#else
	#define MUU_CONSTEVAL				constexpr
#endif

#if !defined(DOXYGEN) && defined(__cpp_conditional_explicit)
	#define MUU_EXPLICIT(...)			explicit(__VA_ARGS__)
#else
	#define MUU_EXPLICIT(...)			explicit
#endif

#define MUU_PREPEND_R_1(S)				R##S
#define MUU_PREPEND_R(S)				MUU_PREPEND_R_1(S)
#define MUU_ADD_PARENTHESES_1(S)		(S)
#define MUU_ADD_PARENTHESES(S)			MUU_ADD_PARENTHESES_1(S)
#define MUU_MAKE_STRING_1(S)			#S
#define MUU_MAKE_STRING(S)				MUU_MAKE_STRING_1(S)
#define MUU_MAKE_RAW_STRING_1(S)		MUU_MAKE_STRING(MUU_ADD_PARENTHESES(S))
#define MUU_MAKE_RAW_STRING(S)			MUU_PREPEND_R(MUU_MAKE_RAW_STRING_1(S))
#define MUU_APPEND_SV_1(S)				S##sv
#define MUU_APPEND_SV(S)				MUU_APPEND_SV_1(S)
#define MUU_MAKE_STRING_VIEW(S)			MUU_APPEND_SV(MUU_MAKE_RAW_STRING(S))

#define MUU_EVAL_1(T, F)		T
#define MUU_EVAL_0(T, F)		F
#define MUU_EVAL(cond, T, F)	MUU_CONCAT(MUU_EVAL_, cond)(T, F)

#define MUU_DELETE_MOVE(TYPE)				\
	TYPE(TYPE&&) = delete;					\
	TYPE& operator=(TYPE&&) = delete

#define MUU_DELETE_COPY(TYPE)				\
	TYPE(const TYPE&) = delete;				\
	TYPE& operator=(const TYPE&) = delete

#ifndef MUU_TRACE // spam^H^H^H^H debugging hook
	#define MUU_TRACE(...) (void)0
#endif

#define MUU_MAKE_BITOPS_(type, op)											\
	[[nodiscard]]															\
	MUU_ALWAYS_INLINE														\
	MUU_ATTR(flatten)														\
	constexpr type operator op (const type& lhs, const type& rhs) noexcept	\
	{																		\
		return static_cast<type>(::muu::unwrap(lhs) op ::muu::unwrap(rhs));	\
	}																		\
	[[nodiscard]]															\
	MUU_ALWAYS_INLINE														\
	MUU_ATTR(flatten)														\
	constexpr type& operator op= (type& lhs, const type& rhs) noexcept		\
	{																		\
		return lhs op (lhs & rhs);											\
	}

#define MUU_MAKE_BITOPS(type)												\
	MUU_MAKE_BITOPS_(type, &)												\
	MUU_MAKE_BITOPS_(type, |)												\
	MUU_MAKE_BITOPS_(type, ^)												\
	[[nodiscard]]															\
	MUU_ALWAYS_INLINE														\
	MUU_ATTR(flatten)														\
	constexpr type operator ~ (const type& val) noexcept					\
	{																		\
		return static_cast<type>(~::muu::unwrap(val));						\
	}

#define MUU_PUSH_PRECISE_MATH												\
	MUU_PRAGMA_MSVC(float_control(precise, on, push))						\
	MUU_PRAGMA_CLANG_GE(11, "float_control(precise, on, push)")				\
	MUU_PRAGMA_GCC(push_options)											\
	MUU_PRAGMA_GCC(optimize("-fno-fast-math"))

#define MUU_POP_PRECISE_MATH												\
	MUU_PRAGMA_GCC(pop_options)												\
	MUU_PRAGMA_CLANG_GE(11, "float_control(pop)")							\
	MUU_PRAGMA_MSVC(float_control(pop))

//=====================================================================================================================
// SFINAE AND CONCEPTS
//=====================================================================================================================

#ifdef DOXYGEN
	#define MUU_SFINAE(...)
	#define MUU_SFINAE_2(...)
	#define MUU_CONCEPTS 0
	#define MUU_SFINAE_NO_CONCEPTS(...)
	#define MUU_SFINAE_2_NO_CONCEPTS(...)
	#define MUU_REQUIRES(...)
#else
	#define MUU_SFINAE(...)						, std::enable_if_t<(__VA_ARGS__), int> = 0
	#define MUU_SFINAE_2(...)					, typename = std::enable_if_t<(__VA_ARGS__)>
	#if defined(__cpp_concepts) && defined(__cpp_lib_concepts) && !MUU_INTELLISENSE
		#define MUU_CONCEPTS 1
		#define MUU_REQUIRES(...)				requires(__VA_ARGS__)
		#define MUU_SFINAE_NO_CONCEPTS(...)
		#define MUU_SFINAE_2_NO_CONCEPTS(...)
	#else
		#define MUU_CONCEPTS 0
		#define MUU_REQUIRES(...)
		#define MUU_SFINAE_NO_CONCEPTS(...)		MUU_SFINAE(__VA_ARGS__)
		#define MUU_SFINAE_2_NO_CONCEPTS(...)	MUU_SFINAE_2(__VA_ARGS__)
	#endif
#endif

//=====================================================================================================================
// WHAT THE HELL IS WCHAR_T?
//=====================================================================================================================

#if MUU_WINDOWS
	#define MUU_WCHAR_BYTES			2
	#define MUU_WCHAR_BITS			16
#elif defined(__SIZEOF_WCHAR_T__)
	#define MUU_WCHAR_BYTES			__SIZEOF_WCHAR_T__
	#if __SIZEOF_WCHAR_T__ == 4
		#define MUU_WCHAR_BITS		32
	#elif __SIZEOF_WCHAR_T__ == 2
		#define MUU_WCHAR_BITS		16
	#elif __SIZEOF_WCHAR_T__ == 1
		#define MUU_WCHAR_BITS		8
	#endif
#endif

//=====================================================================================================================
// EXTENDED INT AND FLOAT TYPES
//=====================================================================================================================

#if defined(DOXYGEN) || (MUU_ARCH_ARM && (MUU_GCC || MUU_CLANG))
	#define MUU_HAS_FP16	1
#else
	#define MUU_HAS_FP16	0
#endif

#ifdef __FLT16_MANT_DIG__
	// #pragma message("__FLT_RADIX__        " MUU_MAKE_STRING(__FLT_RADIX__))
	// #pragma message("__FLT16_MANT_DIG__   " MUU_MAKE_STRING(__FLT16_MANT_DIG__))
	// #pragma message("__FLT16_DIG__        " MUU_MAKE_STRING(__FLT16_DIG__))
	// #pragma message("__FLT16_MIN_EXP__    " MUU_MAKE_STRING(__FLT16_MIN_EXP__))
	// #pragma message("__FLT16_MIN_10_EXP__ " MUU_MAKE_STRING(__FLT16_MIN_10_EXP__))
	// #pragma message("__FLT16_MAX_EXP__    " MUU_MAKE_STRING(__FLT16_MAX_EXP__))
	// #pragma message("__FLT16_MAX_10_EXP__ " MUU_MAKE_STRING(__FLT16_MAX_10_EXP__))
	#if __FLT_RADIX__ == 2					\
			&& __FLT16_MANT_DIG__ == 11		\
			&& __FLT16_DIG__ == 3			\
			&& __FLT16_MIN_EXP__ == -13		\
			&& __FLT16_MIN_10_EXP__ == -4	\
			&& __FLT16_MAX_EXP__ == 16		\
			&& __FLT16_MAX_10_EXP__ == 4
		#if MUU_ARCH_ARM && MUU_CLANG // not present in g++
			#define MUU_HAS_FLOAT16 1
		#endif
	#endif
#endif
#ifndef MUU_HAS_FLOAT16
	#ifdef DOXYGEN
		#define MUU_HAS_FLOAT16 1
	#else
		#define MUU_HAS_FLOAT16 0
	#endif
#endif

#if defined(DOXYGEN) || (defined(__SIZEOF_FLOAT128__)	\
	&& defined(__FLT128_MANT_DIG__)						\
	&& defined(__LDBL_MANT_DIG__)						\
	&& __FLT128_MANT_DIG__ > __LDBL_MANT_DIG__)
	#define MUU_HAS_FLOAT128 1
#else
	#define MUU_HAS_FLOAT128 0
#endif

#if defined(DOXYGEN) || defined(__SIZEOF_INT128__)
	#define MUU_HAS_INT128 1
#else
	#define MUU_HAS_INT128 0
#endif

//=====================================================================================================================
// VERSIONS AND NAMESPACES
//=====================================================================================================================

#define MUU_VERSION_MAJOR				0
#define MUU_VERSION_MINOR				1
#define MUU_VERSION_PATCH				0

#ifndef MUU_ABI_NAMESPACES
	#ifdef DOXYGEN
		#define MUU_ABI_NAMESPACES 0
	#else
		#define MUU_ABI_NAMESPACES 1
	#endif
#endif
#if MUU_ABI_NAMESPACES
	#define MUU_NAMESPACE_START			namespace muu { inline namespace MUU_CONCAT(v, MUU_VERSION_MAJOR)
	#define MUU_NAMESPACE_END			}
	#define MUU_NAMESPACE				::muu::MUU_CONCAT(v, MUU_VERSION_MAJOR)
#else
	#define MUU_NAMESPACE_START			namespace muu
	#define MUU_NAMESPACE_END
	#define MUU_NAMESPACE				muu
#endif
#define MUU_IMPL_NAMESPACE_START		MUU_NAMESPACE_START { namespace impl
#define MUU_IMPL_NAMESPACE_END			} MUU_NAMESPACE_END

//=====================================================================================================================
// ASSERT
//=====================================================================================================================

#ifndef MUU_ASSERT
	#ifdef NDEBUG
		#define MUU_ASSERT(expr)	(void)0
	#else
		#ifndef assert
			MUU_DISABLE_WARNINGS
			#include <cassert>
			MUU_ENABLE_WARNINGS
		#endif
		#define MUU_ASSERT(expr) assert(expr)
	#endif
#endif
#ifdef NDEBUG
	// ensure any overrides respect NDEBUG
	#undef MUU_ASSERT
	#define MUU_ASSERT(expr)					(void)0
	#define MUU_CONSTEXPR_SAFE_ASSERT(expr)		(void)0
#else
	#define MUU_CONSTEXPR_SAFE_ASSERT(expr)									\
		do																	\
		{																	\
			if constexpr (::muu::build::supports_is_constant_evaluated)		\
			{																\
				if (!::muu::is_constant_evaluated())						\
				{															\
					MUU_ASSERT(expr);										\
				}															\
			}																\
		}																	\
		while(false)
#endif


//=====================================================================================================================
// OFFSETOF
//=====================================================================================================================

#ifndef MUU_OFFSETOF
	#ifndef offsetof
		MUU_DISABLE_WARNINGS
		#include <cstddef>
		MUU_ENABLE_WARNINGS
	#endif
	#define MUU_OFFSETOF(s, m) offsetof(s, m)
#endif

//=====================================================================================================================
// DOXYGEN SPAM
//=====================================================================================================================

#if defined(DOXYGEN)

/// \defgroup	preprocessor		Preprocessor magic
/// \brief		Compiler feature detection, attributes, string-makers, etc.
/// @{
///
/// \def MUU_ARCH_ITANIUM
/// \brief `1` when targeting 64-bit Itanium, otherwise `0`.
///
/// \def MUU_ARCH_AMD64
/// \brief `1` when targeting AMD64, otherwise `0`.
///
/// \def MUU_ARCH_X86
/// \brief `1` when targeting 32-bit x86, otherwise `0`.
/// 
/// \def MUU_ARCH_ARM32
/// \brief `1` when targeting 32-bit ARM, otherwise `0`.
/// 
/// \def MUU_ARCH_ARM64
/// \brief `1` when targeting 64-bit ARM, otherwise `0`.
/// 
/// \def MUU_ARCH_ARM
/// \brief `1` when targeting any flavour of ARM, otherwise `0`.
/// 
/// \def MUU_ARCH_BITNESS
/// \brief The 'bitness' of the current architecture (e.g. `64` on AMD64).
/// 
/// \def MUU_ARCH_X64
/// \brief `1` when targeting any 64-bit architecture, otherwise `0`.
/// 
/// \def MUU_CLANG
/// \brief The value of `__clang_major__` when the code is being compiled by LLVM/Clang, otherwise `0`.
/// \see https://sourceforge.net/p/predef/wiki/Compilers/
/// 
/// \def MUU_MSVC
/// \brief The value of `_MSC_VER` when the code is being compiled by MSVC, otherwise `0`.
/// \see https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
/// 
/// \def MUU_ICC
/// \brief The value of `__INTEL_COMPILER` when the code is being compiled by ICC, otherwise `0`.
/// \see http://scv.bu.edu/computation/bladecenter/manpages/icc.html
/// 
/// \def MUU_GCC
/// \brief The value of `__GNUC__` when the code is being compiled by GCC, otherwise `0`.
/// \see https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
/// 
/// \def MUU_WINDOWS
/// \brief `1` when building for the Windows operating system, otherwise `0`.
/// 
/// \def MUU_INTELLISENSE
/// \brief `1` when the code being compiled by an IDE's 'intellisense' compiler, otherwise `0`.
/// 
/// \def MUU_EXCEPTIONS
/// \brief `1` when support for C++ exceptions is enabled, otherwise `0`.
/// 
/// \def MUU_RTTI
/// \brief `1` when support for C++ run-time type identification (RTTI) is enabled, otherwise `0`.
/// 
/// \def MUU_LITTLE_ENDIAN
/// \brief `1` when the target environment is little-endian, otherwise `0`.
/// 
/// \def MUU_BIG_ENDIAN
/// \brief `1` when the target environment is big-endian, otherwise `0`.
/// 
/// \def MUU_CPP
/// \brief The currently-targeted C++ standard. `17` for C++17, `20` for C++20, etc.
/// 
/// \def MUU_PRAGMA_CLANG(...)
/// \brief Expands to `_Pragma("pragma clang ...")` when compiling with Clang.
/// 
/// \def MUU_PRAGMA_CLANG_GE(ver, ...)
/// \brief Expands to `_Pragma("pragma clang ...")` when compiling with Clang and `__clang_major__` >= `ver`.
/// 
/// \def MUU_PRAGMA_CLANG_LT(ver, ...)
/// \brief Expands to `_Pragma("pragma clang ...")` when compiling with Clang and `__clang_major__` < `ver`.
///
/// \def MUU_PRAGMA_MSVC(...)
/// \brief Expands to `_pragma(...)` directive when compiling with MSVC.
/// 
/// \def MUU_PRAGMA_ICC(...)
/// \brief Expands to `_pragma(...)` directive when compiling with ICC.
/// 
/// \def MUU_PRAGMA_GCC(...)
/// \brief Expands to `_Pragma("pragma GCC ...")` directive when compiling with GCC.
///
/// \def MUU_PRAGMA_GCC_GE(ver, ...)
/// \brief Expands to `_Pragma("pragma GCC ...")` when compiling with GCC and `__GNUC__` >= `ver`.
/// 
/// \def MUU_PRAGMA_GCC_LT(ver, ...)
/// \brief Expands to `_Pragma("pragma GCC ...")` when compiling with GCC and `__GNUC__` < `ver`.
/// 
/// \def MUU_ATTR(attr)
/// \brief Expands to `__attribute__(( attr ))` when compiling with a compiler that supports GNU-style attributes.
/// 
/// \def MUU_ATTR_NDEBUG(attr)
/// \brief Expands to `__attribute__(( attr ))` when compiling with a compiler that supports GNU-style attributes
/// 	   and NDEBUG is defined.
/// 
/// \def MUU_ATTR_CLANG(attr)
/// \brief Expands to `__attribute__(( attr ))` when compiling with Clang.
///
/// \def MUU_ATTR_CLANG_GE(ver, attr)
/// \brief Expands to `__attribute__(( attr ))` when compiling with Clang and `__clang_major__` >= `ver`.
/// 
/// \def MUU_ATTR_CLANG_LT(ver, attr)
/// \brief Expands to `__attribute__(( attr ))` when compiling with Clang and `__clang_major__` < `ver`.
///
/// \def MUU_ATTR_GCC(attr)
/// \brief Expands to `__attribute__(( attr ))` when compiling with GCC.
///
/// \def MUU_ATTR_GCC_GE(ver, attr)
/// \brief Expands to `__attribute__(( attr ))` when compiling with GCC and `__GNUC__` >= `ver`.
/// 
/// \def MUU_ATTR_GCC_LT(ver, attr)
/// \brief Expands to `__attribute__(( attr ))` when compiling with GCC and `__GNUC__` < `ver`.
///
/// \def MUU_DECLSPEC(attr)
/// \brief Expands to `__declspec( attr )` when compiling with MSVC (or another compiler in MSVC-mode).
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
/// \def MUU_DISABLE_LIFETIME_WARNINGS
/// \brief Disables compiler warnings relating to object lifetime
/// 	   (initialization, destruction, magic statics, et cetera).
/// \see MUU_PUSH_WARNINGS
///
/// \def MUU_DISABLE_WARNINGS
/// \brief Disables ALL compiler warnings.
///
/// \def MUU_ENABLE_WARNINGS
/// \brief Re-enables compiler warnings again after using #MUU_DISABLE_WARNINGS.
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
/// \brief Expands a conditional to include an optimizer intrinsic (or C++20's [[likely]], if available)
/// 	   indicating that an if/else conditional is the likely path.
/// \detail \cpp
/// 	if MUU_LIKELY(condition_that_is_almost_always_true)
/// 	{
/// 		do_the_thing();
/// 	}
/// \ecpp
/// \see
/// 	- [\[\[likely\]\]](https://en.cppreference.com/w/cpp/language/attributes/likely)
/// 	- [__builtin_expect()](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html)
/// 	 
/// \def MUU_UNLIKELY
/// \brief Expands a conditional to include an optimizer intrinsic (or C++20's [[unlikely]], if available)
/// 	   indicating that an if/else conditional is the unlikely path.
/// \detail \cpp
/// 	if MUU_UNLIKELY(condition_that_is_almost_always_false)
/// 	{
/// 		do_the_thing();
/// 	}
/// \ecpp
/// \see
/// 	- [\[\[likely\]\]](https://en.cppreference.com/w/cpp/language/attributes/likely)
/// 	- [__builtin_expect()](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html)
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
/// 
/// \def MUU_CONSTEVAL
/// \brief Expands to C++20's `consteval` if supported by your compiler, otherwise `constexpr`.
/// \see [consteval](https://en.cppreference.com/w/cpp/language/consteval)
/// 
/// \def MUU_VECTORCALL
/// \brief Expands to `__vectorcall` on compilers that support it.
/// \see [__vectorcall](https://docs.microsoft.com/en-us/cpp/cpp/vectorcall?view=vs-2019)
/// 
/// \def MUU_MAKE_STRING(str)
/// \brief Stringifies the input, converting it into a string literal.
/// \detail \cpp
/// // these are equivalent:
///	constexpr auto str1 = MUU_MAKE_STRING(Oh noes!);
///	constexpr auto str2 = "Oh noes!";
/// \ecpp
/// \see [String literals in C++](https://en.cppreference.com/w/cpp/language/string_literal)
/// 
/// \def MUU_MAKE_RAW_STRING(str)
/// \brief Stringifies the input, converting it verbatim into a raw string literal.
/// \detail \cpp
/// // these are equivalent:
/// constexpr auto str1 = MUU_MAKE_RAW_STRING("It's trap!" the admiral cried.);
/// constexpr auto str2 = R"("It's trap!" the admiral cried.)";
/// \ecpp
/// \see [String literals in C++](https://en.cppreference.com/w/cpp/language/string_literal)
/// 
/// \def MUU_MAKE_STRING_VIEW(str)
/// \brief Stringifies the input, converting it verbatim into a raw string view literal.
/// \detail \cpp
/// // these are equivalent:
///	constexpr std::string_view str1 = MUU_MAKE_STRING_VIEW("It's trap!" the admiral cried.);
///	constexpr std::string_view str2 = R"("It's trap!" the admiral cried.)"sv;
/// \ecpp
/// \see [String literals in C++](https://en.cppreference.com/w/cpp/language/string_literal)
///
/// \def MUU_HAS_INT128
/// \brief `1` when the target environment has 128-bit integers, otherwise `0`.
/// 
/// \def MUU_HAS_FLOAT128
/// \brief `1` when the target environment has 128-bit floats ('quads'), otherwise `0`.
/// 	   
/// \def MUU_HAS_FLOAT16
/// \brief `1` when the target environment has native IEC559 16-bit floats ('halfs'), otherwise `0`.
/// \remarks This is completely unrelated to the class muu::half, which is always available.
///
/// \def MUU_HAS_BUILTIN(name)
/// \brief Expands to `__has_builtin(name)` when supported by the compiler, otherwise `0`.
///
/// \def MUU_HAS_INCLUDE(header)
/// \brief Expands to `__has_include(name)` when supported by the compiler, otherwise `0`.
/// 
/// \def MUU_OFFSETOF(type, member)
/// \brief Constexpr-friendly alias of `offsetof()`.
/// 
/// \def MUU_CONCEPTS
/// \brief `1` when both the compiler and standard library implementation support C++20 constraints and concepts, otherwise `0`.
/// \see [Constraints and concepts](https://en.cppreference.com/w/cpp/language/constraints)
/// 
/// \def MUU_EXPLICIT(...)
/// \brief Expands a C++20 conditional `explicit(...)` specifier if supported by the compiler, otherwise `explicit`.
///
/// \def MUU_DELETE_MOVE(name)
/// \brief Explicitly deletes the move constructor and move-assignment operator of a class or struct.
/// \detail \cpp
/// class Foo
/// {
///		Foo() {}
///		MUU_DELETE_MOVE(Foo);
///	};
///	//instances of Foo cannot be move-constructed or move-assigned.
/// \ecpp
/// \see https://cpppatterns.com/patterns/rule-of-five.html
///
/// \def MUU_DELETE_COPY(name)
/// \brief Explicitly deletes the copy constructor and copy-assignment operator of a class or struct.
/// \detail \cpp
/// class Foo
/// {
///		Foo() {}
///		MUU_DELETE_COPY(Foo);
///	};
///	//instances of Foo cannot be copy-constructed or copy-assigned.
/// \ecpp
/// \see https://cpppatterns.com/patterns/rule-of-five.html
///
/// 
/// @}

#endif // DOXYGEN

// clang-format on

