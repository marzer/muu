// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

#include "../meta.h"
#include "../is_constant_evaluated.h"
#include "../assume_aligned.h"
#include "../pointer_cast.h"
#include "../launder.h"
#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS; // these should be considered "intrinsics"

namespace muu
{
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

	/// \cond
	namespace impl
	{
		template <typename Func, typename T, T... N>
		inline constexpr bool for_sequence_impl_noexcept_ =
			sizeof...(N) == 0 || ((noexcept(std::declval<Func>()(std::integral_constant<T, N>{})) && ...));

		template <typename Func, typename T, T... N>
		constexpr void for_sequence_impl(Func&& func, std::integer_sequence<T, N...>) noexcept(
			for_sequence_impl_noexcept_<Func&&, T, N...>)
		{
			(static_cast<void>(static_cast<Func&&>(func)(std::integral_constant<T, N>{})), ...);
		}

		template <auto N, typename Func>
		inline constexpr bool for_sequence_noexcept_ =
			noexcept(for_sequence_impl(std::declval<Func>(), std::make_integer_sequence<decltype(N), N>{}));

		template <typename Func, typename T, T... N1, typename U, U... N2>
		MUU_NODISCARD
		MUU_CONSTEVAL
		bool for_product_impl_noexcept(std::integer_sequence<T, N1...>, std::integer_sequence<U, N2...>) noexcept
		{
			if constexpr (sizeof...(N1) == 0 || sizeof...(N2) == 0)
				return true;
			else
			{
				bool value = true;

				const auto inner = [&](auto i, auto j) noexcept
				{
					value = noexcept(std::declval<Func>()(std::integral_constant<T, decltype(i)::value>{},
														  std::integral_constant<U, decltype(j)::value>{}))
						 && value;
				};

				const auto outer = [&](auto i) noexcept
				{ (static_cast<void>(inner(i, std::integral_constant<U, N2>{})), ...); };

				(static_cast<void>(outer(std::integral_constant<T, N1>{})), ...);

				return value;
			}
		}

		template <typename Func, typename T, T... N1, typename U, U... N2>
		constexpr void for_product_impl(Func&& func,
										std::integer_sequence<T, N1...>,
										std::integer_sequence<U,
															  N2...>) //
			noexcept(for_product_impl_noexcept<Func&&>(std::integer_sequence<T, N1...>{},
													   std::integer_sequence<U, N2...>{}))
		{
			const auto inner = [&](auto i, auto j)
			{
				static_cast<Func&&>(func)(std::integral_constant<T, decltype(i)::value>{},
										  std::integral_constant<U, decltype(j)::value>{});
			};

			const auto outer = [&](auto i) { (static_cast<void>(inner(i, std::integral_constant<U, N2>{})), ...); };

			(static_cast<void>(outer(std::integral_constant<T, N1>{})), ...);
		}

		template <auto N1, auto N2, typename Func>
		inline constexpr bool for_product_noexcept_ =
			noexcept(for_product_impl(std::declval<Func>(),
									  std::make_integer_sequence<decltype(N1), N1>{},
									  std::make_integer_sequence<decltype(N2), N2>{}));

	}
	/// \endcond

	/// \brief	Generates a series of sequential function calls by pack expansion.
	/// \ingroup core
	///
	/// \details Generates a std::integral_constant sequence, [0...N1), which is passed into the callable: \cpp
	///
	/// auto vals = std::tuple{ 0, "1"sv, 2.3f };
	/// for_sequence<3>([&](auto i)
	/// {
	///		std::cout << std::get<decltype(i)::value>(vals) << "\n";
	/// });
	/// \ecpp
	///
	/// \out
	/// 0
	/// 1
	/// 2.3
	/// \eout
	///
	/// \tparam N		The length of the sequence (the number of calls). Cannot be negative.
	/// \tparam Func	A callable type with the signature `void(auto)`.
	template <auto N, typename Func>
	constexpr void for_sequence(Func&& func) noexcept(impl::for_sequence_noexcept_<N, Func>)
	{
		using n_type = decltype(N);
		static_assert(N >= n_type{}, "N cannot be negative.");

		if constexpr (N > n_type{})
			impl::for_sequence_impl(static_cast<Func&&>(func), std::make_integer_sequence<n_type, N>{});
		else
			MUU_UNUSED(func);
	}

	/// \brief	Generates a series of sequential function calls by pack expansion.
	/// \ingroup core
	///
	/// \details Generates a std::integral_constant product sequence, [0...N1) * [0...N2), which is passed into the callable: \cpp
	///
	/// auto vals_1 = std::pair{ 1, 2 };
	/// auto vals_2 = std::pair{ 3, 4 };
	/// for_product<2, 2>([&](auto i, auto j)
	/// {
	///		const auto lhs = std::get<decltype(i)::value>(vals_1);
	///		const auto rhs = std::get<decltype(j)::value>(vals_2);
	///		std::cout << lhs
	///		          << " * "
	///		          << rhs
	///		          << " = "
	///		          << lhs * rhs;
	/// });
	/// \ecpp
	///
	/// \out
	/// 1 * 3 = 3
	/// 1 * 4 = 4
	/// 2 * 3 = 6
	/// 2 * 4 = 8
	/// \eout
	///
	/// \tparam N1		The length of the LHS sequence. Cannot be negative.
	/// \tparam N2		The length of the RHS sequence. Cannot be negative.
	/// \tparam Func	A callable type with the signature `void(auto, auto)`.
	template <auto N1, auto N2, typename Func>
	constexpr void for_product(Func&& func) noexcept(impl::for_product_noexcept_<N1, N2, Func>)
	{
		using n1_type = decltype(N1);
		static_assert(N1 >= n1_type{}, "N1 cannot be negative.");

		using n2_type = decltype(N2);
		static_assert(N2 >= n2_type{}, "N2 cannot be negative.");

		if constexpr (N1 > n1_type{} && N2 > n2_type{})
			impl::for_product_impl(static_cast<Func&&>(func),
								   std::make_integer_sequence<n1_type, N1>{},
								   std::make_integer_sequence<n2_type, N2>{});
		else
			MUU_UNUSED(func);
	}

	/// \brief	Rounds an unsigned value up to the next multiple of the given alignment.
	/// \ingroup core
	///
	/// \tparam 	Alignment	The alignment to round up to. Must be a power of two.
	/// \tparam	T	An unsigned integer or enum type.
	/// \param 	val	The unsigned value being aligned.
	MUU_CONSTRAINED_TEMPLATE(is_unsigned<T>, size_t Alignment, typename T)
	MUU_CONST_INLINE_GETTER
	constexpr T apply_alignment(T val) noexcept
	{
		static_assert(Alignment, "alignment cannot be zero");
		static_assert((Alignment & (Alignment - 1u)) == 0u, "alignment must be a power of two");

		if constexpr (is_enum<T>)
		{
			return static_cast<T>(apply_alignment(static_cast<std::underlying_type_t<T>>(val)));
		}
		else
		{
			using uint = impl::highest_ranked<T, size_t>;
			return static_cast<T>((val + Alignment - uint{ 1 }) & ~(Alignment - uint{ 1 }));
		}
	}

	/// \brief	Rounds a pointer up to the byte offset that is the next multiple of the given alignment.
	/// \ingroup core
	///
	/// \tparam 		Alignment	The alignment to round up to. Must be a power of two.
	/// \tparam	T		An object type (or void).
	/// \param 	ptr		The pointer being aligned.
	template <size_t Alignment, typename T>
	MUU_CONST_INLINE_GETTER
	MUU_ATTR(nonnull)
	MUU_ATTR(returns_nonnull)
	MUU_ATTR(assume_aligned(Alignment))
	constexpr T* apply_alignment(T* ptr) noexcept
	{
		static_assert(!std::is_function_v<T>, "apply_alignment() may not be used on pointers to functions.");
		static_assert(Alignment, "alignment cannot be zero");
		static_assert((Alignment & (Alignment - 1u)) == 0u, "alignment must be a power of two");
		static_assert(Alignment >= alignment_of<std::remove_pointer_t<T>>, "cannot under-align types.");

		return muu::assume_aligned<Alignment>(
			reinterpret_cast<T*>(apply_alignment<Alignment>(reinterpret_cast<uintptr_t>(ptr))));
	}

	/// \brief	Rounds an unsigned value up to the next multiple of the given alignment.
	/// \ingroup core
	///
	/// \tparam	T			An unsigned integer or enum type.
	/// \param 	val			The unsigned value being aligned.
	/// \param 	alignment	The alignment to round up to. Must be a power of two.
	MUU_CONSTRAINED_TEMPLATE(is_unsigned<T>, typename T)
	MUU_CONST_GETTER
	constexpr T apply_alignment(T val, size_t alignment) noexcept
	{
		MUU_CONSTEXPR_SAFE_ASSERT(alignment && "alignment cannot be zero");
		MUU_CONSTEXPR_SAFE_ASSERT(((alignment & (alignment - 1u)) == 0u) && "alignment must be a power of two");
		MUU_ASSUME(alignment > 0u);
		MUU_ASSUME((alignment & (alignment - 1u)) == 0u);

		if constexpr (is_enum<T>)
		{
			return static_cast<T>(apply_alignment(static_cast<std::underlying_type_t<T>>(val), alignment));
		}
		else
		{
			using uint = impl::highest_ranked<T, size_t>;
			return static_cast<T>((val + alignment - uint{ 1 }) & ~(alignment - uint{ 1 }));
		}
	}

	/// \brief	Rounds a pointer up to the byte offset that is the next multiple of the given alignment.
	/// \ingroup core
	///
	/// \tparam	T			An object type (or void).
	/// \param 	ptr			The pointer being aligned.
	/// \param 	alignment	The alignment to round up to. Must be a power of two.
	template <typename T>
	MUU_CONST_GETTER
	MUU_ATTR(nonnull)
	MUU_ATTR(returns_nonnull)
	constexpr T* apply_alignment(T* ptr, size_t alignment) noexcept
	{
		static_assert(!std::is_function_v<T>, "apply_alignment() may not be used on pointers to functions.");
		MUU_CONSTEXPR_SAFE_ASSERT(alignment && "alignment cannot be zero");
		MUU_CONSTEXPR_SAFE_ASSERT(((alignment & (alignment - 1u)) == 0u) && "alignment must be a power of two");
		MUU_CONSTEXPR_SAFE_ASSERT(alignment >= alignment_of<std::remove_pointer_t<T>> && "cannot under-align types.");

		return reinterpret_cast<T*>(apply_alignment(reinterpret_cast<uintptr_t>(ptr), alignment));
	}
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
