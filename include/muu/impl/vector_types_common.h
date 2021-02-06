// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "../../muu/core.h"
#include "../../muu/math.h"

MUU_DISABLE_WARNINGS;
#include <iosfwd>
MUU_ENABLE_WARNINGS;

MUU_PUSH_WARNINGS;
MUU_DISABLE_SPAM_WARNINGS;

/// \cond
namespace muu::impl
{
	template <typename T, typename U>
	using equality_check_type = std::conditional_t<
		is_signed<T> != is_signed<U> || is_floating_point<T> != is_floating_point<U>,
		promote_if_small_float<highest_ranked<T, U>>,
		highest_ranked<T, U>
	>;

	template <typename T>
	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(const)
	constexpr auto MUU_VECTORCALL raw_modulo(T lhs, T rhs) noexcept // todo: constexpr fmod
	{
		if constexpr (is_floating_point<T>)
		{
			if constexpr (is_standard_arithmetic<T>)
				return std::fmod(lhs, rhs);
			#if MUU_HAS_QUADMATH
			else if constexpr (std::is_same_v<float128_t, T>)
				return ::fmodq(lhs, rhs);
			#endif
			else
				return static_cast<T>(std::fmod(
					static_cast<clamp_to_standard_float<T>>(lhs),
					static_cast<clamp_to_standard_float<T>>(rhs)
				));
		}
		else
			return lhs % rhs;
	}

	MUU_CONSTRAINED_TEMPLATE(
		(pass_readonly_by_reference<T, U> || !MUU_ENABLE_PAIRED_FUNCS),
		typename Return,
		typename T,
		typename U
	)
	[[nodiscard]]
	MUU_ATTR(pure)
	inline constexpr Return raw_cross(const T& lhs, const U& rhs) noexcept
	{
		MUU_FMA_BLOCK;
		using lhs_scalar = decltype(lhs.x);
		using rhs_scalar = decltype(rhs.x);
		using return_scalar = remove_cvref<decltype(std::declval<Return>().x)>;

		using type = promote_if_small_float<highest_ranked<
			std::conditional_t<is_integral<lhs_scalar>,    highest_ranked<make_signed<lhs_scalar>,    int>, lhs_scalar>,
			std::conditional_t<is_integral<rhs_scalar>,    highest_ranked<make_signed<rhs_scalar>,    int>, rhs_scalar>,
			std::conditional_t<is_integral<return_scalar>, highest_ranked<make_signed<return_scalar>, int>, return_scalar>
		>>;

		return Return
		{
			static_cast<return_scalar>(static_cast<type>(lhs.y) * static_cast<type>(rhs.z) - static_cast<type>(lhs.z) * static_cast<type>(rhs.y)),
			static_cast<return_scalar>(static_cast<type>(lhs.z) * static_cast<type>(rhs.x) - static_cast<type>(lhs.x) * static_cast<type>(rhs.z)),
			static_cast<return_scalar>(static_cast<type>(lhs.x) * static_cast<type>(rhs.y) - static_cast<type>(lhs.y) * static_cast<type>(rhs.x))
		};
	}

	#if MUU_ENABLE_PAIRED_FUNCS

	MUU_CONSTRAINED_TEMPLATE_2(
		(pass_readonly_by_value<T, U>),
		typename Return,
		typename T,
		typename U
	)
	[[nodiscard]]
	MUU_ATTR(const)
	inline constexpr Return MUU_VECTORCALL raw_cross(T lhs, U rhs) noexcept
	{
		MUU_FMA_BLOCK;
		using lhs_scalar = decltype(lhs.x);
		using rhs_scalar = decltype(rhs.x);
		using return_scalar = remove_cvref<decltype(std::declval<Return>().x)>;

		using type = promote_if_small_float<highest_ranked<
			std::conditional_t<is_integral<lhs_scalar>,    highest_ranked<make_signed<lhs_scalar>,    int>, lhs_scalar>,
			std::conditional_t<is_integral<rhs_scalar>,    highest_ranked<make_signed<rhs_scalar>,    int>, rhs_scalar>,
			std::conditional_t<is_integral<return_scalar>, highest_ranked<make_signed<return_scalar>, int>, return_scalar>
		>>;

		return Return
		{
			static_cast<return_scalar>(static_cast<type>(lhs.y) * static_cast<type>(rhs.z) - static_cast<type>(lhs.z) * static_cast<type>(rhs.y)),
			static_cast<return_scalar>(static_cast<type>(lhs.z) * static_cast<type>(rhs.x) - static_cast<type>(lhs.x) * static_cast<type>(rhs.z)),
			static_cast<return_scalar>(static_cast<type>(lhs.x) * static_cast<type>(rhs.y) - static_cast<type>(lhs.y) * static_cast<type>(rhs.x))
		};
	}

	#endif // MUU_ENABLE_PAIRED_FUNCS

	template <typename T>
	[[nodiscard]]
	MUU_ATTR_NDEBUG(pure)
	MUU_ATTR(nonnull)
	inline T initialize_trivial_by_memcpy(const void* ptr) noexcept
	{
		static_assert(!std::is_reference_v<T>);
		static_assert(std::is_trivially_default_constructible_v<T>);
		static_assert(std::is_trivially_copyable_v<T>);

		MUU_CONSTEXPR_SAFE_ASSERT(
			ptr != nullptr
			&& "ptr cannot be nullptr"
		);
		MUU_ASSUME(ptr != nullptr);

		T val;
		memcpy(&val, ptr, sizeof(T));
		return val;
	}

	MUU_API void print_vector(std::ostream& stream, const half*, size_t);
	MUU_API void print_vector(std::ostream& stream, const float*, size_t);
	MUU_API void print_vector(std::ostream& stream, const double*, size_t);
	MUU_API void print_vector(std::ostream& stream, const long double*, size_t);
	MUU_API void print_vector(std::ostream& stream, const signed char*, size_t);
	MUU_API void print_vector(std::ostream& stream, const signed short*, size_t);
	MUU_API void print_vector(std::ostream& stream, const signed int*, size_t);
	MUU_API void print_vector(std::ostream& stream, const signed long*, size_t);
	MUU_API void print_vector(std::ostream& stream, const signed long long*, size_t);
	MUU_API void print_vector(std::ostream& stream, const unsigned char*, size_t);
	MUU_API void print_vector(std::ostream& stream, const unsigned short*, size_t);
	MUU_API void print_vector(std::ostream& stream, const unsigned int*, size_t);
	MUU_API void print_vector(std::ostream& stream, const unsigned long*, size_t);
	MUU_API void print_vector(std::ostream& stream, const unsigned long long*, size_t);
	#if MUU_HAS_FLOAT16
	MUU_API void print_vector(std::ostream& stream, const _Float16*, size_t);
	#endif
	#if MUU_HAS_FP16
	MUU_API void print_vector(std::ostream& stream, const __fp16*, size_t);
	#endif
	#if MUU_HAS_FLOAT128
	MUU_API void print_vector(std::ostream& stream, const float128_t*, size_t);
	#endif
	#if MUU_HAS_INT128
	MUU_API void print_vector(std::ostream& stream, const int128_t*, size_t);
	MUU_API void print_vector(std::ostream& stream, const uint128_t*, size_t);
	#endif

	MUU_API void print_vector(std::wostream& stream, const half*, size_t);
	MUU_API void print_vector(std::wostream& stream, const float*, size_t);
	MUU_API void print_vector(std::wostream& stream, const double*, size_t);
	MUU_API void print_vector(std::wostream& stream, const long double*, size_t);
	MUU_API void print_vector(std::wostream& stream, const signed char*, size_t);
	MUU_API void print_vector(std::wostream& stream, const signed short*, size_t);
	MUU_API void print_vector(std::wostream& stream, const signed int*, size_t);
	MUU_API void print_vector(std::wostream& stream, const signed long*, size_t);
	MUU_API void print_vector(std::wostream& stream, const signed long long*, size_t);
	MUU_API void print_vector(std::wostream& stream, const unsigned char*, size_t);
	MUU_API void print_vector(std::wostream& stream, const unsigned short*, size_t);
	MUU_API void print_vector(std::wostream& stream, const unsigned int*, size_t);
	MUU_API void print_vector(std::wostream& stream, const unsigned long*, size_t);
	MUU_API void print_vector(std::wostream& stream, const unsigned long long*, size_t);
	#if MUU_HAS_FLOAT16
	MUU_API void print_vector(std::wostream& stream, const _Float16*, size_t);
	#endif
	#if MUU_HAS_FP16
	MUU_API void print_vector(std::wostream& stream, const __fp16*, size_t);
	#endif
	#if MUU_HAS_FLOAT128
	MUU_API void print_vector(std::wostream& stream, const float128_t*, size_t);
	#endif
	#if MUU_HAS_INT128
	MUU_API void print_vector(std::wostream& stream, const int128_t*, size_t);
	MUU_API void print_vector(std::wostream& stream, const uint128_t*, size_t);
	#endif



	MUU_API void print_matrix(std::ostream& stream, const half*, size_t, size_t);
	MUU_API void print_matrix(std::ostream& stream, const float*, size_t, size_t);
	MUU_API void print_matrix(std::ostream& stream, const double*, size_t, size_t);
	MUU_API void print_matrix(std::ostream& stream, const long double*, size_t, size_t);
	MUU_API void print_matrix(std::ostream& stream, const signed char*, size_t, size_t);
	MUU_API void print_matrix(std::ostream& stream, const signed short*, size_t, size_t);
	MUU_API void print_matrix(std::ostream& stream, const signed int*, size_t, size_t);
	MUU_API void print_matrix(std::ostream& stream, const signed long*, size_t, size_t);
	MUU_API void print_matrix(std::ostream& stream, const signed long long*, size_t, size_t);
	MUU_API void print_matrix(std::ostream& stream, const unsigned char*, size_t, size_t);
	MUU_API void print_matrix(std::ostream& stream, const unsigned short*, size_t, size_t);
	MUU_API void print_matrix(std::ostream& stream, const unsigned int*, size_t, size_t);
	MUU_API void print_matrix(std::ostream& stream, const unsigned long*, size_t, size_t);
	MUU_API void print_matrix(std::ostream& stream, const unsigned long long*, size_t, size_t);
	#if MUU_HAS_FLOAT16
	MUU_API void print_matrix(std::ostream& stream, const _Float16*, size_t, size_t);
	#endif
	#if MUU_HAS_FP16
	MUU_API void print_matrix(std::ostream& stream, const __fp16*, size_t, size_t);
	#endif
	#if MUU_HAS_FLOAT128
	MUU_API void print_matrix(std::ostream& stream, const float128_t*, size_t, size_t);
	#endif
	#if MUU_HAS_INT128
	MUU_API void print_matrix(std::ostream& stream, const int128_t*, size_t, size_t);
	MUU_API void print_matrix(std::ostream& stream, const uint128_t*, size_t, size_t);
	#endif

	MUU_API void print_matrix(std::wostream& stream, const half*, size_t, size_t);
	MUU_API void print_matrix(std::wostream& stream, const float*, size_t, size_t);
	MUU_API void print_matrix(std::wostream& stream, const double*, size_t, size_t);
	MUU_API void print_matrix(std::wostream& stream, const long double*, size_t, size_t);
	MUU_API void print_matrix(std::wostream& stream, const signed char*, size_t, size_t);
	MUU_API void print_matrix(std::wostream& stream, const signed short*, size_t, size_t);
	MUU_API void print_matrix(std::wostream& stream, const signed int*, size_t, size_t);
	MUU_API void print_matrix(std::wostream& stream, const signed long*, size_t, size_t);
	MUU_API void print_matrix(std::wostream& stream, const signed long long*, size_t, size_t);
	MUU_API void print_matrix(std::wostream& stream, const unsigned char*, size_t, size_t);
	MUU_API void print_matrix(std::wostream& stream, const unsigned short*, size_t, size_t);
	MUU_API void print_matrix(std::wostream& stream, const unsigned int*, size_t, size_t);
	MUU_API void print_matrix(std::wostream& stream, const unsigned long*, size_t, size_t);
	MUU_API void print_matrix(std::wostream& stream, const unsigned long long*, size_t, size_t);
	#if MUU_HAS_FLOAT16
	MUU_API void print_matrix(std::wostream& stream, const _Float16*, size_t, size_t);
	#endif
	#if MUU_HAS_FP16
	MUU_API void print_matrix(std::wostream& stream, const __fp16*, size_t, size_t);
	#endif
	#if MUU_HAS_FLOAT128
	MUU_API void print_matrix(std::wostream& stream, const float128_t*, size_t, size_t);
	#endif
	#if MUU_HAS_INT128
	MUU_API void print_matrix(std::wostream& stream, const int128_t*, size_t, size_t);
	MUU_API void print_matrix(std::wostream& stream, const uint128_t*, size_t, size_t);
	#endif



	MUU_API void print_compound_vector(std::ostream& stream, const half*, size_t, bool, const half*, size_t, bool);
	MUU_API void print_compound_vector(std::ostream& stream, const float*, size_t, bool, const float*, size_t, bool);
	MUU_API void print_compound_vector(std::ostream& stream, const double*, size_t, bool, const double*, size_t, bool);
	MUU_API void print_compound_vector(std::ostream& stream, const long double*, size_t, bool, const long double*, size_t, bool);
	MUU_API void print_compound_vector(std::ostream& stream, const signed char*, size_t, bool, const signed char*, size_t, bool);
	MUU_API void print_compound_vector(std::ostream& stream, const signed short*, size_t, bool, const signed short*, size_t, bool);
	MUU_API void print_compound_vector(std::ostream& stream, const signed int*, size_t, bool, const signed int*, size_t, bool);
	MUU_API void print_compound_vector(std::ostream& stream, const signed long*, size_t, bool, const signed long*, size_t, bool);
	MUU_API void print_compound_vector(std::ostream& stream, const signed long long*, size_t, bool, const signed long long*, size_t, bool);
	MUU_API void print_compound_vector(std::ostream& stream, const unsigned char*, size_t, bool, const unsigned char*, size_t, bool);
	MUU_API void print_compound_vector(std::ostream& stream, const unsigned short*, size_t, bool, const unsigned short*, size_t, bool);
	MUU_API void print_compound_vector(std::ostream& stream, const unsigned int*, size_t, bool, const unsigned int*, size_t, bool);
	MUU_API void print_compound_vector(std::ostream& stream, const unsigned long*, size_t, bool, const unsigned long*, size_t, bool);
	MUU_API void print_compound_vector(std::ostream& stream, const unsigned long long*, size_t, bool, const unsigned long long*, size_t, bool);
	#if MUU_HAS_FLOAT16
	MUU_API void print_compound_vector(std::ostream& stream, const _Float16*, size_t, bool, const _Float16*, size_t, bool);
	#endif
	#if MUU_HAS_FP16
	MUU_API void print_compound_vector(std::ostream& stream, const __fp16*, size_t, bool, const __fp16*, size_t, bool);
	#endif
	#if MUU_HAS_FLOAT128
	MUU_API void print_compound_vector(std::ostream& stream, const float128_t*, size_t, bool, const float128_t*, size_t, bool);
	#endif
	#if MUU_HAS_INT128
	MUU_API void print_compound_vector(std::ostream& stream, const int128_t*, size_t, bool, const int128_t*, size_t, bool);
	MUU_API void print_compound_vector(std::ostream& stream, const uint128_t*, size_t, bool, const uint128_t*, size_t, bool);
	#endif

	MUU_API void print_compound_vector(std::wostream& stream, const half*, size_t, bool, const half*, size_t, bool);
	MUU_API void print_compound_vector(std::wostream& stream, const float*, size_t, bool, const float*, size_t, bool);
	MUU_API void print_compound_vector(std::wostream& stream, const double*, size_t, bool, const double*, size_t, bool);
	MUU_API void print_compound_vector(std::wostream& stream, const long double*, size_t, bool, const long double*, size_t, bool);
	MUU_API void print_compound_vector(std::wostream& stream, const signed char*, size_t, bool, const signed char*, size_t, bool);
	MUU_API void print_compound_vector(std::wostream& stream, const signed short*, size_t, bool, const signed short*, size_t, bool);
	MUU_API void print_compound_vector(std::wostream& stream, const signed int*, size_t, bool, const signed int*, size_t, bool);
	MUU_API void print_compound_vector(std::wostream& stream, const signed long*, size_t, bool, const signed long*, size_t, bool);
	MUU_API void print_compound_vector(std::wostream& stream, const signed long long*, size_t, bool, const signed long long*, size_t, bool);
	MUU_API void print_compound_vector(std::wostream& stream, const unsigned char*, size_t, bool, const unsigned char*, size_t, bool);
	MUU_API void print_compound_vector(std::wostream& stream, const unsigned short*, size_t, bool, const unsigned short*, size_t, bool);
	MUU_API void print_compound_vector(std::wostream& stream, const unsigned int*, size_t, bool, const unsigned int*, size_t, bool);
	MUU_API void print_compound_vector(std::wostream& stream, const unsigned long*, size_t, bool, const unsigned long*, size_t, bool);
	MUU_API void print_compound_vector(std::wostream& stream, const unsigned long long*, size_t, bool, const unsigned long long*, size_t, bool);
	#if MUU_HAS_FLOAT16
	MUU_API void print_compound_vector(std::wostream& stream, const _Float16*, size_t, bool, const _Float16*, size_t, bool);
	#endif
	#if MUU_HAS_FP16
	MUU_API void print_compound_vector(std::wostream& stream, const __fp16*, size_t, bool, const __fp16*, size_t, bool);
	#endif
	#if MUU_HAS_FLOAT128
	MUU_API void print_compound_vector(std::wostream& stream, const float128_t*, size_t, bool, const float128_t*, size_t, bool);
	#endif
	#if MUU_HAS_INT128
	MUU_API void print_compound_vector(std::wostream& stream, const int128_t*, size_t, bool, const int128_t*, size_t, bool);
	MUU_API void print_compound_vector(std::wostream& stream, const uint128_t*, size_t, bool, const uint128_t*, size_t, bool);
	#endif
}
/// \endcond

MUU_POP_WARNINGS; // MUU_DISABLE_SPAM_WARNINGS
