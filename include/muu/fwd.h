// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief The bare-minimum include for forward declarations of muu functions and types.

#pragma once
#include "../muu/preprocessor.h"

#ifndef DOXYGEN // undocumented forward declarations are hidden from doxygen because they fuck it up =/

//=====================================================================================================================
// NON-MUU TYPEDEFS AND FORWARD DECLARATIONS
//=====================================================================================================================

MUU_DISABLE_WARNINGS
#include <cstdint>
#include <cstddef>
MUU_ENABLE_WARNINGS

namespace std
{
	template <typename>				struct	hash;
	template <typename>				struct	pointer_traits;
	template <typename>				struct	tuple_size;
	template <size_t, typename>		struct	tuple_element;
	template <typename, typename>	struct	pair;

	template <typename, size_t>		class	array;
	template <typename>				class	optional;
	template <typename...>			class	tuple;
}

#if MUU_WINDOWS
#ifndef _MINWINDEF_
extern "C"
{
	using ULONG = unsigned long;
	using PULONG = ULONG*;
	using USHORT = unsigned short;
	using PUSHORT = USHORT* ;
	using UCHAR = unsigned char;
	using PUCHAR = UCHAR*;
	using DWORD = unsigned long;
	using BOOL = int;
	using BYTE = unsigned char;
	using WORD = unsigned short;
	using FLOAT = float;
	using PFLOAT = FLOAT*;
	using PBOOL = BOOL*;
	using LPBOOL = BOOL*;
	using PBYTE = BYTE*;
	using LPBYTE = BYTE*;
	using PINT = int*;
	using LPINT = int*;
	using PWORD = WORD*;
	using LPWORD = WORD*;
	using LPLONG = long*;
	using PDWORD = DWORD*;
	using LPDWORD = DWORD*;
	using LPVOID = void*;
	using LPCVOID = const void*;
	using INT = int;
	using UINT = unsigned int;
	using PUINT = unsigned int*;
}
#endif // !_MINWINDEF_
#ifndef __unknwnbase_h__
struct IUnknown;
#endif
#endif // MUU_WINDOWS

//=====================================================================================================================
// TYPEDEFS AND FORWARD DECLARATIONS - UNDOCUMENTED
//=====================================================================================================================

namespace muu // non-abi namespace; this is not an error
{
	using ::std::size_t;
	using ::std::intptr_t;
	using ::std::uintptr_t;
	using ::std::ptrdiff_t;
	using ::std::nullptr_t;
	using ::std::int8_t;
	using ::std::int16_t;
	using ::std::int32_t;
	using ::std::int64_t;
	using ::std::uint8_t;
	using ::std::uint16_t;
	using ::std::uint32_t;
	using ::std::uint64_t;
	using ::std::int_least8_t;
	using ::std::int_least16_t;
	using ::std::int_least32_t;
	using ::std::int_least64_t;
	using ::std::uint_least8_t;
	using ::std::uint_least16_t;
	using ::std::uint_least32_t;
	using ::std::uint_least64_t;
	using ::std::int_fast8_t;
	using ::std::int_fast16_t;
	using ::std::int_fast32_t;
	using ::std::int_fast64_t;
	using ::std::uint_fast8_t;
	using ::std::uint_fast16_t;
	using ::std::uint_fast32_t;
	using ::std::uint_fast64_t;
	using ::std::intmax_t;
	using ::std::uintmax_t;
}

MUU_NAMESPACE_START // abi namespace
{
	struct									half;
	struct									semver;
	struct									uuid;
	template <typename, size_t>		struct	vector;

	class									bitset;
	class									blob;
	class									sha1;
	class									string_param;
	class									thread_pool;
	template <typename, typename>	class	compressed_pair;
	template <typename>				class	emplacement_array;
	template <size_t>				class	fnv1a;
	template <size_t>				class	hash_combiner;
	template <typename>				class	scope_guard;
	template <typename, size_t>		class	tagged_ptr;

	template <typename, size_t = static_cast<size_t>(-1)>
	class span;

	namespace impl
	{
		template <typename>
		struct basic_accumulator;

		template <typename T>
		struct kahan_accumulator;

		template <typename T>	struct default_accumulator				{ using type = basic_accumulator<T>; };
		template <>				struct default_accumulator<float>		{ using type = kahan_accumulator<float>; };
		template <>				struct default_accumulator<double>		{ using type = kahan_accumulator<double>; };
		template <>				struct default_accumulator<long double>	{ using type = kahan_accumulator<long double>; };
		template <>				struct default_accumulator<half>		{ using type = kahan_accumulator<half>; };
	}
	template <typename T, typename = typename impl::default_accumulator<T>::type>
	class accumulator;
}
MUU_NAMESPACE_END

#endif // !DOXYGEN

//=====================================================================================================================
// TYPEDEFS AND FORWARD DECLARATIONS
//=====================================================================================================================

/// \defgroup		meta			Metafunctions and type traits
/// \brief			Type traits and metaprogramming utilities.
/// \remarks	Many of these are mirrors of (or supplementary to) traits found in the standard library's
///				`<type_traits>`, but with simpler/saner default behaviour (e.g. most of the `is_X` metafunctions do
///				not make a distinction between T and T&).

/// \defgroup		intrinsics			Intrinsics
/// \brief Small, generally-useful functions, many of which map to compiler built-ins.

/// \defgroup		building_blocks		Building blocks
/// \brief Small, generally-useful classes and types.

/// \defgroup		constants			Constants
/// \brief Compile-time constant values (Pi, et cetera.).

/// \defgroup		characters			Characters
/// \brief Utilities for manipulating individual characters (or 'code points').

/// \defgroup		strings				Strings
/// \brief Utilities to simplify working with strings.

/// \defgroup		hashing			Hashing
/// \brief Utilities for generating (non-cryptographic) hashes.

/// \defgroup		mem				Memory management
/// \brief Utilities for allocating, destroying and manipulating memory.

/// \brief	The root namespace for all muu functions and types.
namespace muu { }

namespace muu // non-abi namespace; this is not an error
{
	#if MUU_HAS_INT128
	/// \brief	A 128-bit signed integer.
	/// \ingroup building_blocks
	/// 
	/// \attention This typedef is only present when 128-bit integers are supported by your target platform.
	/// 		 You can check support using #MUU_HAS_INT128.
	using int128_t = __int128_t;

	/// \brief	A 128-bit unsigned integer.
	/// \ingroup building_blocks
	/// 
	/// \attention This typedef is only present when 128-bit integers are supported by your target platform.
	/// 		 You can check support using #MUU_HAS_INT128.
	using uint128_t = __uint128_t;
	#endif

	#if MUU_HAS_FLOAT128
	/// \brief	A 128-bit quad-precision float.
	/// \ingroup building_blocks
	/// 
	/// \attention This typedef is only present when 128-bit floats are supported by your target platform.
	/// 		 You can check support using #MUU_HAS_FLOAT128.
	using quad = __float128;
	#endif
}

MUU_NAMESPACE_START // abi namespace
{
	/// \brief Literal operators.
	inline namespace literals {}

	/// \brief Build environment information (compiler, date/time, support for various things...)
	/// \ingroup constants
	namespace build {}

	/// \brief	A typed container for static constants, similar to std::numeric_limits.
	/// \ingroup constants
	template <typename T>
	struct constants {};
	template <typename T> struct constants<T&> : constants<T> {};
	template <typename T> struct constants<T&&> : constants<T> {};
	template <typename T> struct constants<const T> : constants<T> {};
	template <typename T> struct constants<volatile T> : constants<T> {};
	template <typename T> struct constants<const volatile T> : constants<T> {};
}
MUU_NAMESPACE_END

//=====================================================================================================================
// TYPEDEFS AND FORWARD DECLARATIONS - UNDOCUMENTED (redux)
//=====================================================================================================================

#ifndef DOXYGEN

MUU_IMPL_NAMESPACE_START
{
	#if MUU_HAS_FP16
	template <> struct default_accumulator<__fp16>		{ using type = kahan_accumulator<__fp16>; };
	#endif
	#if MUU_HAS_FLOAT16
	template <> struct default_accumulator<_Float16>	{ using type = kahan_accumulator<_Float16>; };
	#endif
	#if MUU_HAS_FLOAT128
	template <> struct default_accumulator<quad>		{ using type = kahan_accumulator<quad>; };
	#endif
}
MUU_IMPL_NAMESPACE_END

#endif // !DOXYGEN
