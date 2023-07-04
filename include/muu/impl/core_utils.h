// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

#include "../meta.h"
#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS; // these should be considered "intrinsics"

namespace muu
{
	//% unwrap start
	/// \brief	Unwraps an enum to it's raw integer equivalent.
	/// \ingroup core
	///
	/// \tparam	T		An enum type.
	/// \param 	val		The value to unwrap.
	///
	/// \returns	\conditional_return{Enum inputs} `static_cast<std::underlying_type_t<T>>(val)`
	/// 			 <br>
	/// 			\conditional_return{Everything else} `T&&` (a no-op).
	MUU_CONSTRAINED_TEMPLATE(is_enum<T>, typename T)
	MUU_CONST_INLINE_GETTER
	constexpr std::underlying_type_t<T> MUU_VECTORCALL unwrap(T val) noexcept
	{
		return static_cast<std::underlying_type_t<T>>(val);
	}

	/// \cond

	MUU_CONSTRAINED_TEMPLATE(!is_enum<T>, typename T)
	MUU_CONST_INLINE_GETTER
	constexpr T&& unwrap(T&& val) noexcept
	{
		return static_cast<T&&>(val);
	}

	/// \endcond
	//% unwrap end

	//% min start
	/// \brief	Returns the minimum of two or more values.
	/// \ingroup core
	///
	/// \remark This is a variadic version of std::min.
	template <typename T, typename... U>
	MUU_PURE_GETTER
	constexpr const T& min(const T& val1, const T& val2, const U&... vals) noexcept
	{
		if constexpr (sizeof...(vals) == 0u)
		{
			return val1 < val2 ? val1 : val2;
		}
		else if constexpr (sizeof...(vals) == 2u)
		{
			return muu::min(muu::min(val1, val2), muu::min(vals...));
		}
		else
		{
			return muu::min(muu::min(val1, val2), vals...);
		}
	}
	//% min end

	//% max start
	/// \brief	Returns the maximum of two or more values.
	/// \ingroup core
	///
	/// \remark This is a variadic version of std::max.
	template <typename T, typename... U>
	MUU_PURE_GETTER
	constexpr const T& max(const T& val1, const T& val2, const U&... vals) noexcept
	{
		if constexpr (sizeof...(vals) == 0u)
		{
			return val1 < val2 ? val2 : val1;
		}
		else if constexpr (sizeof...(vals) == 2u)
		{
			return muu::max(muu::max(val1, val2), muu::max(vals...));
		}
		else
		{
			return muu::max(muu::max(val1, val2), vals...);
		}
	}
	//% max end

	//% clamp start
	/// \brief	Returns a value clamped between two bounds (inclusive).
	/// \ingroup core
	///
	/// \remark This is equivalent to std::clamp without requiring you to drag in the enormity of &lt;algorithm&gt;.
	template <typename T>
	MUU_PURE_GETTER
	constexpr const T& clamp(const T& val, const T& low, const T& high) noexcept
	{
		return val < low ? low : ((high < val) ? high : val);
	}
	//% clamp end

	/// \brief	Returns true if a value is between two bounds (inclusive).
	/// \ingroup core
	template <typename T, typename U>
	MUU_PURE_GETTER
	MUU_ATTR(flatten)
	constexpr bool MUU_VECTORCALL between(const T& val, const U& low, const U& high) noexcept
	{
		if constexpr ((is_arithmetic<T> || is_enum<T>)&&(is_arithmetic<U> || is_enum<U>))
		{
			if constexpr (is_enum<T> || is_enum<U>)
				return between(unwrap(val), unwrap(low), unwrap(high));
			else
			{
				using lhs = remove_cvref<T>;
				using rhs = remove_cvref<U>;
				if constexpr (is_signed<lhs> && is_unsigned<rhs>)
				{
					if (val < lhs{})
						return false;
				}
				else if constexpr (is_unsigned<lhs> && is_signed<rhs>)
				{
					if (high < rhs{})
						return false;
				}
				if constexpr (!std::is_same_v<lhs, rhs>)
				{
					using common_type = std::common_type_t<lhs, rhs>;
					return between(static_cast<common_type>(val),
								   static_cast<common_type>(low),
								   static_cast<common_type>(high));
				}
				else
					return low <= val && val <= high;
			}
		}
		else
			return low <= val && val <= high; // user-defined <= operator, ideally
	}

	/// \brief	Applies a byte offset to a pointer.
	/// \ingroup core
	///
	/// \tparam	T		The type being pointed to.
	/// \tparam	Offset	An integer type.
	/// \param	ptr	The pointer to offset.
	/// \param	offset	The number of bytes to add to the pointer's address.
	///
	/// \return	The cv-correct equivalent of `(T*)((std::byte*)ptr + offset)`.
	///
	/// \warning This function is a simple pointer arithmetic helper; absolutely no consideration
	/// 		 is given to the alignment of the pointed type. If you need to dereference pointers
	/// 		 returned by apply_offset the onus is on you to ensure that the offset made sense
	/// 		 before doing so!
	MUU_CONSTRAINED_TEMPLATE((is_integral<Offset> && is_arithmetic<Offset>), typename T, typename Offset)
	MUU_CONST_INLINE_GETTER
	constexpr T* apply_offset(T* ptr, Offset offset) noexcept
	{
		if constexpr (any_same<remove_cv<T>, char, signed char, unsigned char, std::byte>)
		{
			return ptr + offset;
		}
		else
		{
			using char_ptr = rebase_pointer<T*, unsigned char>;
			if constexpr (std::is_void_v<T>)
				return static_cast<T*>(static_cast<char_ptr>(ptr) + offset);
			else
				return reinterpret_cast<T*>(reinterpret_cast<char_ptr>(ptr) + offset);
		}
	}

	/// \cond
	namespace impl
	{
		template <typename T>
		using has_pointer_traits_to_address_ =
			decltype(std::pointer_traits<remove_cvref<T>>::to_address(std::declval<remove_cvref<T>>()));
	}
	/// \endcond

	/// \brief Obtain the address represented by p without forming a reference to the pointee.
	/// \ingroup core
	///
	/// \remark This is equivalent to C++20's std::to_address.
	template <typename T>
	MUU_CONST_INLINE_GETTER
	constexpr T* to_address(T* p) noexcept
	{
		static_assert(!std::is_function_v<T>, "to_address may not be used on functions.");
		return p;
	}

	/// \brief Obtain the address represented by p without forming a reference to the pointee.
	/// \ingroup core
	///
	/// \remark This is equivalent to C++20's std::to_address.
	template <typename Ptr>
	MUU_NODISCARD
	constexpr auto to_address(const Ptr& p) noexcept
	{
		if constexpr (is_detected<impl::has_pointer_traits_to_address_, Ptr>)
		{
			return std::pointer_traits<Ptr>::to_address(p);
		}
		else
		{
			return to_address(p.operator->());
		}
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
