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
// CONFIGURATION
//=====================================================================================================================

#ifdef MUU_CONFIG_HEADER
	#include MUU_CONFIG_HEADER
	#undef MUU_CONFIG_HEADER
#endif

#if !defined(MUU_ALL_INLINE) || (defined(MUU_ALL_INLINE) && MUU_ALL_INLINE) || defined(__INTELLISENSE__)
	#undef MUU_ALL_INLINE
	#define MUU_ALL_INLINE 1
#endif

#if defined(MUU_IMPLEMENTATION) || MUU_ALL_INLINE
	#undef MUU_IMPLEMENTATION
	#define MUU_IMPLEMENTATION 1
#else
	#define MUU_IMPLEMENTATION 0
#endif

#ifndef MUU_API
	#define MUU_API
#endif

//=====================================================================================================================
// COMPILER DETECTION
//=====================================================================================================================

#ifdef DOXYGEN
	#undef	MUU_DOXYGEN
	#define MUU_DOXYGEN			1
#endif
#ifndef MUU_DOXYGEN
	#define MUU_DOXYGEN			0
#endif
#ifdef __INTELLISENSE__
	#define MUU_INTELLISENSE	1
#else
	#define MUU_INTELLISENSE	0
#endif
#ifdef __clang__
	#define MUU_CLANG			__clang_major__
#else
	#define MUU_CLANG			0
#endif
#ifdef __INTEL_COMPILER
	#define MUU_ICC				__INTEL_COMPILER
#else
	#define MUU_ICC				0
#endif
#if defined(_MSC_VER) && !MUU_CLANG && !MUU_ICC
	#define MUU_MSVC			_MSC_VER
#else
	#define MUU_MSVC			0
#endif
#if defined(__GNUC__) && !MUU_CLANG && !MUU_ICC
	#define MUU_GCC				__GNUC__
#else
	#define MUU_GCC				0
#endif
#if !MUU_DOXYGEN
	#if !MUU_CLANG && !MUU_ICC && !MUU_MSVC && !MUU_GCC
		#error Unknown compiler.
	#endif
	#if (MUU_CLANG && (MUU_ICC || MUU_MSVC || MUU_GCC)) \
		|| (MUU_ICC && (MUU_CLANG || MUU_MSVC || MUU_GCC)) \
		|| (MUU_MSVC && (MUU_CLANG || MUU_ICC || MUU_GCC)) \
		|| (MUU_GCC && (MUU_CLANG || MUU_ICC || MUU_MSVC))
		#error Could not uniquely identify compiler.
	#endif
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
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_AMD64)
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
	#define MUU_ARCH_BITNESS 64
#elif defined(__arm__) || defined(_M_ARM) || defined(__ARM_32BIT_STATE)
	#define MUU_ARCH_ARM32 1
	#define MUU_ARCH_ARM64 0
	#define MUU_ARCH_BITNESS 32
#else
	#define MUU_ARCH_ARM32 0
	#define MUU_ARCH_ARM64 0
#endif

#define MUU_ARCH_SUM (MUU_ARCH_ITANIUM + MUU_ARCH_AMD64 + MUU_ARCH_X86 + MUU_ARCH_ARM32 + MUU_ARCH_ARM64)
#if MUU_ARCH_SUM > 1
	#error Could not uniquely identify target architecture.
#elif MUU_ARCH_SUM == 0
	#error Unknown target architecture.
#endif
#undef MUU_ARCH_SUM
#ifdef _WIN32
	#define MUU_WINDOWS 1
#else
	#define MUU_WINDOWS 0
#endif

#define MUU_CONCAT_1(x, y) x##y
#define MUU_CONCAT(x, y) MUU_CONCAT_1(x, y)

//=====================================================================================================================
// CLANG
//=====================================================================================================================

#if MUU_CLANG

	#define MUU_PRAGMA_CLANG(...)					_Pragma(__VA_ARGS__)

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

	#define MUU_PRAGMA_CLANG_GE(ver, ...)	MUU_CONCAT(MUU_MACRO_DISPATCH_CLANG_GE_, ver)(_Pragma(__VA_ARGS__))
	#define MUU_PRAGMA_CLANG_LT(ver, ...)	MUU_CONCAT(MUU_MACRO_DISPATCH_CLANG_LT_, ver)(_Pragma(__VA_ARGS__))

	#define MUU_PUSH_WARNINGS				_Pragma("clang diagnostic push")
	#define MUU_DISABLE_SWITCH_WARNINGS		_Pragma("clang diagnostic ignored \"-Wswitch\"")
	#define MUU_DISABLE_INIT_WARNINGS		_Pragma("clang diagnostic ignored \"-Wmissing-field-initializers\"")	\
											_Pragma("clang diagnostic ignored \"-Wglobal-constructors\"")
	#define MUU_DISABLE_VTABLE_WARNINGS		_Pragma("clang diagnostic ignored \"-Weverything\"") \
											_Pragma("clang diagnostic ignored \"-Wweak-vtables\"")
	#define MUU_DISABLE_PADDING_WARNINGS	_Pragma("clang diagnostic ignored \"-Wpadded\"")
	#define MUU_DISABLE_ARITHMETIC_WARNINGS	_Pragma("clang diagnostic ignored \"-Wfloat-equal\"") \
											_Pragma("clang diagnostic ignored \"-Wdouble-promotion\"") \
											_Pragma("clang diagnostic ignored \"-Wshift-sign-overflow\"") \
							MUU_PRAGMA_CLANG_GE(10, "clang diagnostic ignored \"-Wimplicit-int-float-conversion\"")
	#define MUU_DISABLE_SHADOW_WARNINGS		_Pragma("clang diagnostic ignored \"-Wshadow\"")
	#define MUU_DISABLE_ALL_WARNINGS		_Pragma("clang diagnostic ignored \"-Weverything\"")
	#define MUU_POP_WARNINGS				_Pragma("clang diagnostic pop")
	#define MUU_ASSUME(cond)				__builtin_assume(cond)
	#define MUU_UNREACHABLE					__builtin_unreachable()
	#define MUU_ATTR(attr)					__attribute__((attr))
	#define MUU_ATTR_CLANG(attr)			MUU_ATTR(attr)
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
	#define MUU_LITTLE_ENDIAN				(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
	#define MUU_BIG_ENDIAN					(__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
	#define MUU_LIKELY(...)					(__builtin_expect(!!(__VA_ARGS__), 1) )
	#define MUU_UNLIKELY(...)				(__builtin_expect(!!(__VA_ARGS__), 0) )
	#ifdef __has_builtin
		#define MUU_HAS_BUILTIN(name)	__has_builtin(name)
	#endif

#endif // clang

//=====================================================================================================================
// MSVC
//=====================================================================================================================

#if MUU_MSVC || (defined(_MSC_VER) && MUU_ICC && defined(__ICL))

	#define MUU_CPP_VERSION					_MSVC_LANG
	#define MUU_PRAGMA_MSVC(...)			__pragma(__VA_ARGS__)
	#define MUU_PUSH_WARNINGS				__pragma(warning(push))
	#define MUU_DISABLE_SWITCH_WARNINGS		__pragma(warning(disable: 4063))
	#define MUU_DISABLE_ALL_WARNINGS		__pragma(warning(pop))	\
											__pragma(warning(push, 0))
	#define MUU_POP_WARNINGS				__pragma(warning(pop))
	#define MUU_DECLSPEC(attr)				__declspec(attr)
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
	#define MUU_LITTLE_ENDIAN				1
	#define MUU_BIG_ENDIAN					0

#endif // msvc

//=====================================================================================================================
// GCC
//=====================================================================================================================

#if MUU_GCC

	#define MUU_PRAGMA_GCC(...)				_Pragma(__VA_ARGS__)

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

	#define MUU_PRAGMA_GCC_GE(ver, ...)		MUU_CONCAT(MUU_MACRO_DISPATCH_GCC_GE_, ver)(_Pragma(__VA_ARGS__))
	#define MUU_PRAGMA_GCC_LT(ver, ...)		MUU_CONCAT(MUU_MACRO_DISPATCH_GCC_LT_, ver)(_Pragma(__VA_ARGS__))

	#define MUU_PUSH_WARNINGS				_Pragma("GCC diagnostic push")
	#define MUU_DISABLE_SWITCH_WARNINGS		_Pragma("GCC diagnostic ignored \"-Wswitch\"")						\
											_Pragma("GCC diagnostic ignored \"-Wswitch-enum\"")					\
											_Pragma("GCC diagnostic ignored \"-Wswitch-default\"")
	#define MUU_DISABLE_INIT_WARNINGS		_Pragma("GCC diagnostic ignored \"-Wmissing-field-initializers\"")	\
											_Pragma("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")			\
											_Pragma("GCC diagnostic ignored \"-Wuninitialized\"")				\
							   MUU_PRAGMA_GCC_GE(8, "GCC diagnostic ignored \"-Wclass-memaccess\"")
	#define MUU_DISABLE_PADDING_WARNINGS	_Pragma("GCC diagnostic ignored \"-Wpadded\"")
	#define MUU_DISABLE_ARITHMETIC_WARNINGS	_Pragma("GCC diagnostic ignored \"-Wfloat-equal\"")					\
											_Pragma("GCC diagnostic ignored \"-Wsign-conversion\"")
	#define MUU_DISABLE_SHADOW_WARNINGS		_Pragma("GCC diagnostic ignored \"-Wshadow\"")
	#define MUU_DISABLE_SUGGEST_WARNINGS	_Pragma("GCC diagnostic ignored \"-Wsuggest-attribute=const\"")		\
											_Pragma("GCC diagnostic ignored \"-Wsuggest-attribute=pure\"")
	#define MUU_DISABLE_ALL_WARNINGS		_Pragma("GCC diagnostic ignored \"-Wall\"")							\
											_Pragma("GCC diagnostic ignored \"-Wextra\"")						\
											_Pragma("GCC diagnostic ignored \"-Wchar-subscripts\"")				\
											_Pragma("GCC diagnostic ignored \"-Wtype-limits\"")					\
											MUU_DISABLE_SUGGEST_WARNINGS										\
											MUU_DISABLE_SWITCH_WARNINGS											\
											MUU_DISABLE_INIT_WARNINGS											\
											MUU_DISABLE_PADDING_WARNINGS										\
											MUU_DISABLE_ARITHMETIC_WARNINGS										\
											MUU_DISABLE_SHADOW_WARNINGS
	#define MUU_POP_WARNINGS				_Pragma("GCC diagnostic pop")
	#define MUU_ATTR(attr)					__attribute__((attr))
	#define MUU_ATTR_GCC(attr)				MUU_ATTR(attr)
	#define MUU_ALWAYS_INLINE				__attribute__((__always_inline__)) inline
	#define MUU_NEVER_INLINE				__attribute__((__noinline__))
	#define MUU_UNREACHABLE					__builtin_unreachable()
	#define MUU_UNALIASED_ALLOC				__attribute__((__malloc__))
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
	#define MUU_LITTLE_ENDIAN				(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
	#define MUU_BIG_ENDIAN					(__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
	#define MUU_LIKELY(...)					(__builtin_expect(!!(__VA_ARGS__), 1) )
	#define MUU_UNLIKELY(...)				(__builtin_expect(!!(__VA_ARGS__), 0) )
	#ifdef __has_builtin
		#define MUU_HAS_BUILTIN(name)	__has_builtin(name)
	#endif

#endif // gcc

//=====================================================================================================================
// ALL COMPILERS
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

#ifndef MUU_EXCEPTIONS
	#define MUU_EXCEPTIONS 1
#endif

#ifndef MUU_RTTI
	#define MUU_RTTI 1
#endif

#ifndef MUU_LITTLE_ENDIAN
	#define MUU_LITTLE_ENDIAN 0
#endif
#ifndef MUU_BIG_ENDIAN
	#define MUU_BIG_ENDIAN 0
#endif
#if MUU_BIG_ENDIAN == MUU_LITTLE_ENDIAN
	#error Unknown platform endianness.
#endif

#ifndef MUU_PRAGMA_CLANG
	#define MUU_PRAGMA_CLANG(...)
#endif
#ifndef MUU_PRAGMA_CLANG_GE
	#define MUU_PRAGMA_CLANG_GE(...)
#endif
#ifndef MUU_PRAGMA_CLANG_LT
	#define MUU_PRAGMA_CLANG_LT(...)
#endif
#ifndef MUU_PRAGMA_MSVC
	#define MUU_PRAGMA_MSVC(...)
#endif
#ifndef MUU_PRAGMA_GCC
	#define MUU_PRAGMA_GCC(...)
#endif
#ifndef MUU_PRAGMA_GCC_GE
	#define MUU_PRAGMA_GCC_GE(...)
#endif
#ifndef MUU_PRAGMA_GCC_LT
	#define MUU_PRAGMA_GCC_LT(...)
#endif

#ifndef MUU_ATTR
	#define MUU_ATTR(attr)
#endif
#ifndef MUU_ATTR_CLANG
	#define MUU_ATTR_CLANG(attr)
#endif
#ifndef MUU_ATTR_GCC
	#define MUU_ATTR_GCC(attr)
#endif
#ifndef MUU_DECLSPEC
	#define MUU_DECLSPEC(attr)
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
#ifndef MUU_DISABLE_VTABLE_WARNINGS
	#define MUU_DISABLE_VTABLE_WARNINGS
#endif
#ifndef MUU_DISABLE_PADDING_WARNINGS
	#define MUU_DISABLE_PADDING_WARNINGS
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
#ifndef MUU_DISABLE_ALL_WARNINGS
	#define MUU_DISABLE_ALL_WARNINGS
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
	#define MUU_UNREACHABLE				MUU_ASSERT(false)
#endif

#define MUU_NO_DEFAULT_CASE				default: MUU_UNREACHABLE

#if !MUU_DOXYGEN && !MUU_INTELLISENSE
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
#ifdef MUU_VECTORCALL
	#define MUU_VECTORCALL_CONSTREF
#else
	#define MUU_VECTORCALL
	#define MUU_VECTORCALL_CONSTREF const&
#endif

#if defined(__cpp_consteval) && !MUU_INTELLISENSE
	#define MUU_CONSTEVAL				consteval
#else
	#define MUU_CONSTEVAL				constexpr
#endif

#if MUU_ALL_INLINE
	#define MUU_EXTERNAL_LINKAGE			inline
	#define MUU_INTERNAL_LINKAGE			inline
	#define MUU_INTERNAL_NAMESPACE			muu::impl
	#define MUU_USING_INTERNAL_NAMESPACE	using namespace ::MUU_INTERNAL_NAMESPACE
#else
	#define MUU_EXTERNAL_LINKAGE
	#define MUU_INTERNAL_LINKAGE			static
	#define MUU_INTERNAL_NAMESPACE
	#define MUU_USING_INTERNAL_NAMESPACE	using namespace ::muu_this_is_not_a_real_namespace
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

MUU_PUSH_WARNINGS
MUU_DISABLE_ALL_WARNINGS
#ifndef MUU_ASSERT
	#ifdef NDEBUG
		#define MUU_ASSERT(expr)	(void)0
	#else
		#include <cassert>
		#define MUU_ASSERT(expr)	assert(expr)
	#endif
#endif
MUU_POP_WARNINGS

#ifndef MUU_HAS_INT128
	#ifdef __SIZEOF_INT128__
		#define MUU_HAS_INT128 1
	#else
		#define MUU_HAS_INT128 0
	#endif
#endif

#ifndef MUU_HAS_BUILTIN
	#define MUU_HAS_BUILTIN(name)	0
#endif

/// \addtogroup		preprocessor		Preprocessor magic
/// \brief		Compiler feature detection, attributes, string-makers, etc.
/// @{

/// \def MUU_ARCH_ITANIUM
/// \brief `1` when targeting 64-bit Itanium, `0` otherwise.
///
/// \def MUU_ARCH_AMD64
/// \brief `1` when targeting AMD64, `0` otherwise.
///
/// \def MUU_ARCH_X86
/// \brief `1` when targeting 32-bit x86, `0` otherwise.
/// 
/// \def MUU_ARCH_ARM32
/// \brief `1` when targeting 32-bit ARM, `0` otherwise.
/// 
/// \def MUU_ARCH_ARM64
/// \brief `1` when targeting 64-bit ARM, `0` otherwise.
/// 
/// \def MUU_ARCH_BITNESS
/// \brief The 'bitness' of the current architecture (e.g. `64` on AMD64).
/// 
/// \def MUU_CLANG
/// \brief The value of `__clang_major__` when the code is being compiled by LLVM/Clang, `0` otherwise.
/// \see https://sourceforge.net/p/predef/wiki/Compilers/
/// 
/// \def MUU_MSVC
/// \brief The value of `_MSC_VER` when the code is being compiled by MSVC, `0` otherwise.
/// \see https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
/// 
/// \def MUU_ICC
/// \brief The value of `__INTEL_COMPILER` when the code is being compiled by Intel ICC, `0` otherwise.
/// \see http://scv.bu.edu/computation/bladecenter/manpages/icc.html
/// 
/// \def MUU_GCC
/// \brief The value of `__GNUC__` when the code is being compiled by GCC, `0` otherwise.
/// \see https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
/// 
/// \def MUU_WINDOWS
/// \brief `1` when building for the Windows operating system, `0` otherwise.
/// 
/// \def MUU_INTELLISENSE
/// \brief `1` when the code being compiled by an IDE's 'intellisense' compiler, `0` otherwise.
/// 
/// \def MUU_EXCEPTIONS
/// \brief `1` when support for C++ exceptions is enabled, `0` otherwise.
/// 
/// \def MUU_RTTI
/// \brief `1` when support for C++ run-time type identification (RTTI) is enabled, `0` otherwise.
/// 
/// \def MUU_LITTLE_ENDIAN
/// \brief `1` when the target environment is little-endian, `0` otherwise.
/// 
/// \def MUU_BIG_ENDIAN
/// \brief `1` when the target environment is big-endian, `0` otherwise.
/// 
/// \def MUU_CPP
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
/// \def MUU_ATTR(attr)
/// \brief Expands to `__attribute__(( attr ))` when compiling with a compiler that supports GNU-style attributes.
/// 
/// \def MUU_ATTR_CLANG(attr)
/// \brief Expands to `__attribute__(( attr ))` when compiling with Clang.
///
/// \def MUU_ATTR_GCC(attr)
/// \brief Expands to `__attribute__(( attr ))` when compiling with GCC.
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
/// \brief Expands a conditional to include an optimizer intrinsic (or C++20's [[likely]], if available)
/// 	   indicating that an if/else conditional is the likely path.
/// \detail \cpp
/// 	if MUU_LIKELY(condition_that_is_almost_always_true)
/// 	{
/// 		do_the_thing();
/// 	}
/// \ecpp
/// \see
///		- [\[\[likely\]\]](https://en.cppreference.com/w/cpp/language/attributes/likely)
///		- [__builtin_expect()](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html)
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
///		- [\[\[likely\]\]](https://en.cppreference.com/w/cpp/language/attributes/likely)
///		- [__builtin_expect()](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html)
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
/// 		raii_lock{}; // oh noes! forgot to give a name; would be destroyed immediately.
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
///	constexpr auto str1 = MUU_MAKE_STRING("It's trap!" the admiral cried.);
///	constexpr auto str2 = R"("It's trap!" the admiral cried.)";
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
/// \brief `1` when the target environment has 128-bit integers, `0` otherwise.
///
/// \def MUU_HAS_BUILTIN(name)
/// \brief Expands to `__has_builtin(name)` when supported by the compiler, `0` otherwise.

/// @}

// clang-format on
