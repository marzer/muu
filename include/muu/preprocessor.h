// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

// clang-format off

/// \file
/// \brief Compiler feature detection, attributes, string-makers, etc.
#pragma once

//=====================================================================================================================
// CONFIGURATION
//=====================================================================================================================

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

//=====================================================================================================================
// ARCHITECTURE & ENVIRONMENT
//=====================================================================================================================

#ifndef MUU_DOXYGEN
	#define MUU_DOXYGEN 0
#endif
#ifndef __cplusplus
	#error muu is a C++ library.
#endif
#if defined(__ia64__) || defined(__ia64) || defined(_IA64) || defined(__IA64__) || defined(_M_IA64) || MUU_DOXYGEN
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
	#define MUU_ARCH_X86 1
	#define MUU_ARCH_BITNESS 32
#else
	#define MUU_ARCH_X86 0
#endif
#if defined(__arm__) || defined(_M_ARM) || defined(__ARM_32BIT_STATE)
	#define MUU_ARCH_ARM 1
	#define MUU_ARCH_ARM64 0
	#define MUU_ARCH_BITNESS 32
#elif defined(__aarch64__) || defined(__ARM_ARCH_ISA_A64) || defined(_M_ARM64) || defined(__ARM_64BIT_STATE)
	#define MUU_ARCH_ARM 0
	#define MUU_ARCH_ARM64 1
	#define MUU_ARCH_BITNESS 64
#else
	#define MUU_ARCH_ARM 0
	#define MUU_ARCH_ARM64 0
#endif
#define MUU_ARCH_SUM (MUU_ARCH_IA64 + MUU_ARCH_AMD64 + MUU_ARCH_X86 + MUU_ARCH_ARM + MUU_ARCH_ARM64)
#if MUU_ARCH_SUM > 1
	#error Could not uniquely identify target architecture.
#elif MUU_ARCH_SUM == 0
	#error Unknown target architecture.
#endif
#undef MUU_ARCH_SUM

//=====================================================================================================================
// CLANG
//=====================================================================================================================

#ifdef __clang__

	#define MUU_PRAGMA_CLANG(...)			_Pragma(__VA_ARGS__)
	#define MUU_PUSH_WARNINGS				_Pragma("clang diagnostic push")
	#define MUU_DISABLE_SWITCH_WARNINGS		_Pragma("clang diagnostic ignored \"-Wswitch\"")
	#define MUU_DISABLE_INIT_WARNINGS		_Pragma("clang diagnostic ignored \"-Wmissing-field-initializers\"")
	#define MUU_DISABLE_VTABLE_WARNINGS		_Pragma("clang diagnostic ignored \"-Weverything\"") \
											_Pragma("clang diagnostic ignored \"-Wweak-vtables\"")
	#define MUU_DISABLE_PADDING_WARNINGS	_Pragma("clang diagnostic ignored \"-Wpadded\"")
	#define MUU_DISABLE_FLOAT_WARNINGS		_Pragma("clang diagnostic ignored \"-Wfloat-equal\"") \
											_Pragma("clang diagnostic ignored \"-Wdouble-promotion\"")
	#define MUU_DISABLE_SHADOW_WARNINGS		_Pragma("clang diagnostic ignored \"-Wshadow\"")
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
	#define MUU_LITTLE_ENDIAN				(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
	#define MUU_BIG_ENDIAN					(__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
	#define MUU_LIKELY(...)					(__builtin_expect(!!(__VA_ARGS__), 1) )
	#define MUU_UNLIKELY(...)				(__builtin_expect(!!(__VA_ARGS__), 0) )

//=====================================================================================================================
// MSVC
//=====================================================================================================================

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
	#define MUU_LITTLE_ENDIAN				1
	#define MUU_BIG_ENDIAN					0

//=====================================================================================================================
// GCC
//=====================================================================================================================

#elif defined(__GNUC__)

	#define MUU_PRAGMA_GCC(...)				_Pragma(__VA_ARGS__)
	#define MUU_PUSH_WARNINGS				_Pragma("GCC diagnostic push")
	#define MUU_DISABLE_SWITCH_WARNINGS		_Pragma("GCC diagnostic ignored \"-Wswitch\"")						\
											_Pragma("GCC diagnostic ignored \"-Wswitch-enum\"")					\
											_Pragma("GCC diagnostic ignored \"-Wswitch-default\"")
	#define MUU_DISABLE_INIT_WARNINGS		_Pragma("GCC diagnostic ignored \"-Wmissing-field-initializers\"")	\
											_Pragma("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")			\
											_Pragma("GCC diagnostic ignored \"-Wuninitialized\"")
	#define MUU_DISABLE_PADDING_WARNINGS	_Pragma("GCC diagnostic ignored \"-Wpadded\"")
	#define MUU_DISABLE_FLOAT_WARNINGS		_Pragma("GCC diagnostic ignored \"-Wfloat-equal\"")
	#define MUU_DISABLE_SHADOW_WARNINGS		_Pragma("GCC diagnostic ignored \"-Wshadow\"")
	#define MUU_DISABLE_ALL_WARNINGS		_Pragma("GCC diagnostic ignored \"-Wall\"")							\
											_Pragma("GCC diagnostic ignored \"-Wextra\"")						\
											_Pragma("GCC diagnostic ignored \"-Wchar-subscripts\"")				\
											_Pragma("GCC diagnostic ignored \"-Wtype-limits\"")					\
											MUU_DISABLE_SWITCH_WARNINGS											\
											MUU_DISABLE_INIT_WARNINGS											\
											MUU_DISABLE_PADDING_WARNINGS										\
											MUU_DISABLE_FLOAT_WARNINGS											\
											MUU_DISABLE_SHADOW_WARNINGS
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
	#define MUU_LITTLE_ENDIAN				(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
	#define MUU_BIG_ENDIAN					(__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
	#define MUU_LIKELY(...)					(__builtin_expect(!!(__VA_ARGS__), 1) )
	#define MUU_UNLIKELY(...)				(__builtin_expect(!!(__VA_ARGS__), 0) )

#endif

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
	#define MUU_LITTLE_ENDIAN 1
#endif
#ifndef MUU_BIG_ENDIAN
	#define MUU_BIG_ENDIAN 0
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
#ifndef MUU_GNU_ATTR
	#define MUU_GNU_ATTR(attr)
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
#ifndef MUU_DISABLE_FLOAT_WARNINGS
	#define MUU_DISABLE_FLOAT_WARNINGS
#endif
#ifndef MUU_DISABLE_SHADOW_WARNINGS
	#define MUU_DISABLE_SHADOW_WARNINGS
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

#if !MUU_DOXYGEN && !defined(__INTELLISENSE__)
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
#endif //__INTELLISENSE__
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
	#define MUU_EXTERNAL_LINKAGE	inline
	#define MUU_INTERNAL_LINKAGE	inline
	#define MUU_INTERNAL_NAMESPACE	muu::impl
#else
	#define MUU_EXTERNAL_LINKAGE
	#define MUU_INTERNAL_LINKAGE	static
	#define MUU_INTERNAL_NAMESPACE
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

#if MUU_DOXYGEN


/// \def MUU_ARCH_IA64
/// \brief `1` when targeting 64-bit Itanium, `0` otherwise.
///
/// \def MUU_ARCH_AMD64
/// \brief `1` when targeting AMD64, `0` otherwise.
///
/// \def MUU_ARCH_X86
/// \brief `1` when targeting 32-bit x86, `0` otherwise.
/// 
/// \def MUU_ARCH_ARM
/// \brief `1` when targeting 32-bit ARM, `0` otherwise.
/// 
/// \def MUU_ARCH_ARM64
/// \brief `1` when targeting 64-bit ARM, `0` otherwise.
/// 
/// \def MUU_ARCH_BITNESS
/// \brief The 'bitness' of the current architecture (e.g. `64` on AMD64).

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
/// \brief Expands to C++20's `[[likely]]` or a compiler-specific 'likely' intrinsic.
///
/// \def MUU_UNLIKELY
/// \brief Expands to C++20's `[[unlikely]]` or a compiler-specific 'unlikely' intrinsic.
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



#endif // MUU_DOXYGEN

// clang-format on
