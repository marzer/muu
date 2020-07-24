// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief The bare-minimum include for forward declarations of muu functions and types.

#pragma once
#include "../muu/preprocessor.h"

#if !MUU_DOXYGEN // undocumented forward declarations are hidden from doxygen because they fuck it up =/

//=====================================================================================================================
// STANDARD-LIBRARY / NON-MUU TYPEDEFS AND FORWARD DECLARATIONS
//=====================================================================================================================

namespace std
{
	template <typename>				struct hash;
	template <typename>				struct pointer_traits;
	template <typename, typename>	class basic_ostream;
	template <typename, typename>	class basic_istream;
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
// MUU TYPEDEFS AND CLASS FORWARD DECLARATIONS
//=====================================================================================================================

MUU_PUSH_WARNINGS
MUU_DISABLE_ALL_WARNINGS
#include <cstdint>
#include <cstddef>
MUU_POP_WARNINGS

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
}

MUU_NAMESPACE_START // abi namespace
{
	struct uuid;
	struct semver;
	struct half;

	class blob;
	class string_param;
	class bitset;
	class thread_pool;
	class sha1;

	template <typename, size_t>		struct array;

	template <size_t>				class hash_combiner;
	template <size_t>				class fnv1a;
	template <typename>				class scope_guard;
	template <typename, size_t>		class tagged_ptr;
	template <typename, typename>	class accumulator;
}
MUU_NAMESPACE_END

#endif // !MUU_DOXYGEN

/// \brief	The root namespace for all muu functions and types.
namespace muu { }

namespace muu // non-abi namespace; this is not an error
{
	#if MUU_HAS_INT128 || MUU_DOXYGEN

	/// \brief	A 128-bit signed integer.
	/// 
	/// \attention This typedef is only present when 128-bit integers are supported by your target platform.
	/// 		 You can check support using #MUU_HAS_INT128.
	using int128_t = __int128_t;

	/// \brief	A 128-bit unsigned integer.
	/// 
	/// \attention This typedef is only present when 128-bit integers are supported by your target platform.
	/// 		 You can check support using #MUU_HAS_INT128.
	using uint128_t = __uint128_t;

	#endif

	#if MUU_HAS_FLOAT128 || MUU_DOXYGEN

	/// \brief	A 128-bit quad-precision float.
	/// 
	/// \attention This typedef is only present when 128-bit floats are supported by your target platform.
	/// 		 You can check support using #MUU_HAS_FLOAT128.
	using float128_t = __float128;

	#endif

	#if MUU_HAS_FLOAT16 || MUU_DOXYGEN

	/// \brief	A 16-bit half-precision float.
	/// 
	/// \attention This will be an alias for your target platform's native IEC559 16-bit float type
	/// 		 if present (e.g. `_Float16`), otherwise it will alias muu::half. You can check support using
	/// 		 #MUU_HAS_FLOAT16.
	using float16_t = _Float16;

	#else

	using float16_t = half;

	#endif
}

MUU_NAMESPACE_START // abi namespace
{
	namespace impl {}

	/// \brief Literal operators.
	inline namespace literals {}

	/// \addtogroup		constants		Compile-time constants
	/// @{

	/// \brief Build environment information (compiler, date/time, support for various things...)
	namespace build {}

	/// \brief	A container for typed static constants, similar to std::numeric_limits.
	///
	/// \tparam	T	The constant value type.
	template <typename T>
	struct constants {};
	template <typename T> struct constants<T&> : constants<T> {};
	template <typename T> struct constants<T&&> : constants<T> {};
	template <typename T> struct constants<const T> : constants<T> {};
	template <typename T> struct constants<volatile T> : constants<T> {};
	template <typename T> struct constants<const volatile T> : constants<T> {};

	/// @}
}
MUU_NAMESPACE_END

