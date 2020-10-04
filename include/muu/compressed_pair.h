// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::compressed_pair.

#pragma once
#include "../muu/core.h"

MUU_PUSH_WARNINGS
MUU_DISABLE_SPAM_WARNINGS
MUU_PRAGMA_CLANG("clang diagnostic ignored \"-Wreorder\"")
MUU_PRAGMA_GCC("GCC diagnostic ignored \"-Wreorder\"")

#ifndef DOXYGEN
MUU_IMPL_NAMESPACE_START
{
	enum class compressed_pair_flags : unsigned
	{
		none,
		first_empty = 1,
		second_empty = 2,
		both_empty = first_empty | second_empty
	};

	template <typename First, typename Second>
	[[nodiscard]]
	MUU_CONSTEVAL compressed_pair_flags get_compressed_pair_flags_for() noexcept
	{
		return static_cast<compressed_pair_flags>(
			unwrap(std::is_empty_v<First> && !std::is_final_v<First> ? compressed_pair_flags::first_empty : compressed_pair_flags::none)
			| unwrap(std::is_empty_v<Second> && !std::is_final_v<Second> ? compressed_pair_flags::second_empty : compressed_pair_flags::none)
		);
	}

	#define COMPRESSED_PAIR_BASE_DEFAULTS(first_initializer, second_initializer)									\
																													\
		template <typename F, typename S																			\
			MUU_SFINAE(std::is_constructible_v<First, F&&>&& std::is_constructible_v<Second, S&&>)					\
		>																											\
		MUU_NODISCARD_CTOR																							\
		constexpr compressed_pair_base(F&& first_init, S&& second_init)												\
			noexcept(std::is_nothrow_constructible_v<First, F&&>&& std::is_nothrow_constructible_v<Second, S&&>)	\
			: first_initializer{ std::forward<F>(first_init) },														\
			second_initializer{ std::forward<S>(second_init) }														\
		{}																											\
																													\
		MUU_NODISCARD_CTOR constexpr compressed_pair_base() = default;												\
		MUU_NODISCARD_CTOR constexpr compressed_pair_base(const compressed_pair_base&) = default;					\
		MUU_NODISCARD_CTOR constexpr compressed_pair_base(compressed_pair_base&&) = default;						\
		constexpr compressed_pair_base& operator =(const compressed_pair_base&) = default;							\
		constexpr compressed_pair_base& operator =(compressed_pair_base&&) = default


	#define COMPRESSED_PAIR_BASE_GETTERS(type, name, expression)													\
		MUU_ALWAYS_INLINE MUU_ATTR(pure) constexpr type& get_##name() noexcept { return expression; }				\
		MUU_ALWAYS_INLINE MUU_ATTR(pure) constexpr const type& get_##name() const noexcept { return expression; }

	template <typename First, typename Second, compressed_pair_flags Flags = get_compressed_pair_flags_for<First, Second>()>
	struct compressed_pair_base
	{
		static_assert(Flags == compressed_pair_flags::none);
		First first_;
		Second second_;

		COMPRESSED_PAIR_BASE_DEFAULTS(first_, second_);
		COMPRESSED_PAIR_BASE_GETTERS(First, first, first_)
		COMPRESSED_PAIR_BASE_GETTERS(Second, second, second_)
	};

	template <typename First, typename Second>
	struct compressed_pair_base<First, Second, compressed_pair_flags::first_empty> : First
	{
		Second second_;

		COMPRESSED_PAIR_BASE_DEFAULTS(First, second_);
		COMPRESSED_PAIR_BASE_GETTERS(First, first, *this)
		COMPRESSED_PAIR_BASE_GETTERS(Second, second, second_)
	};

	template <typename First, typename Second>
	struct compressed_pair_base<First, Second, compressed_pair_flags::second_empty> : Second
	{
		First first_;

		COMPRESSED_PAIR_BASE_DEFAULTS(first_, Second);
		COMPRESSED_PAIR_BASE_GETTERS(First, first, first_)
		COMPRESSED_PAIR_BASE_GETTERS(Second, second, *this)
	};

	template <typename First, typename Second>
	struct compressed_pair_base<First, Second, compressed_pair_flags::both_empty> : First, Second
	{
		COMPRESSED_PAIR_BASE_DEFAULTS(First, Second);
		COMPRESSED_PAIR_BASE_GETTERS(First, first, *this)
		COMPRESSED_PAIR_BASE_GETTERS(Second, second, *this)
	};

	#undef COMPRESSED_PAIR_BASE_DEFAULTS
	#undef COMPRESSED_PAIR_BASE_CONVERSIONS

	template <size_t I, typename T>
	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(pure)
	MUU_ATTR(flatten)
	constexpr decltype(auto) compressed_pair_get(T&& cp) noexcept
	{
		static_assert(I <= 1);
		if constexpr (I == 0)
			return std::forward<T>(cp).first();
		else
			return std::forward<T>(cp).second();
	}
}
MUU_IMPL_NAMESPACE_END
#endif // !DOXYGEN

MUU_NAMESPACE_START
{
	/// \brief	A pair that uses Empty Base Class Optimization to elide storage for one or both of its members where possible.
	/// \ingroup building_blocks
	/// 
	/// \tparam	First		First member type.
	/// \tparam	Second		Second member type.
	template <typename First, typename Second>
	class compressed_pair
		#ifndef DOXYGEN
		: private impl::compressed_pair_base<First, Second>
		#endif
	{
		#ifndef DOXYGEN
		using base = impl::compressed_pair_base<First, Second>;
		#endif

		public:

			/// \brief	The pair's first member type.
			using first_type = First;
			/// \brief	The pair's second member type.
			using second_type = Second;

			/// \brief	Default constructor.
			MUU_NODISCARD_CTOR
			constexpr compressed_pair() = default;

			/// \brief	Copy constructor.
			MUU_NODISCARD_CTOR
			constexpr compressed_pair(const compressed_pair&) = default;

			/// \brief	Move constructor.
			MUU_NODISCARD_CTOR
			constexpr compressed_pair(compressed_pair&&) = default;

			/// \brief	Copy-assignment operator.
			constexpr compressed_pair& operator =(const compressed_pair&) = default;

			/// \brief	Move-assignment operator.
			constexpr compressed_pair& operator =(compressed_pair&&) = default;

			/// \brief	Constructs a compressed pair from two values.
			/// 
			/// \tparam	F	First member initializer type.
			/// \tparam	S	Second member initializer type.
			/// \param 	first_init 	First member initializer.
			/// \param 	second_init	Second member initializer.
			template <typename F, typename S
				MUU_SFINAE(std::is_constructible_v<First, F&&>&& std::is_constructible_v<Second, S&&>)
			>
			MUU_NODISCARD_CTOR
				constexpr compressed_pair(F&& first_init, S&& second_init)
				noexcept(std::is_nothrow_constructible_v<First, F&&>&& std::is_nothrow_constructible_v<Second, S&&>)
				#ifndef DOXYGEN
				: base{ std::forward<F>(first_init), std::forward<S>(second_init) }
				#endif
			{}

			/// \brief	Returns an lvalue reference to the first member.
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr first_type& first() & noexcept
			{
				return base::get_first();
			}

			/// \brief	Returns an rvalue reference to the first member.
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr first_type&& first() && noexcept
			{
				return std::move(base::get_first());
			}

			/// \brief	Returns a const lvalue reference to the first member.
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr const first_type& first() const& noexcept
			{
				return base::get_first();
			}

			/// \brief	Returns a const rvalue reference to the first member.
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr const first_type&& first() const&& noexcept
			{
				return std::move(base::get_first());
			}


			/// \brief	Returns an lvalue reference to the second member.
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr second_type& second() & noexcept
			{
				return base::get_second();
			}

			/// \brief	Returns an rvalue reference to the second member.
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr second_type&& second() && noexcept
			{
				return std::move(base::get_second());
			}

			/// \brief	Returns a const lvalue reference to the second member.
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr const second_type& second() const& noexcept
			{
				return base::get_second();
			}

			/// \brief	Returns a const rvalue reference to the second member.
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr const second_type&& second() const&& noexcept
			{
				return std::move(base::get_second());
			}


			/// \brief	Returns an lvalue reference to the selected member.
			template <size_t I>
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr auto& get() & noexcept
			{
				return impl::compressed_pair_get<I>(*this);
			}

			/// \brief	Returns an rvalue reference to the selected member.
			template <size_t I>
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr auto&& get() && noexcept
			{
				return impl::compressed_pair_get<I>(std::move(*this));
			}

			/// \brief	Returns a const lvalue reference to the selected member.
			template <size_t I>
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr const auto& get() const & noexcept
			{
				return impl::compressed_pair_get<I>(*this);
			}

			/// \brief	Returns a const rvalue reference to the selected member.
			template <size_t I>
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr const auto&& get() const && noexcept
			{
				return impl::compressed_pair_get<I>(std::move(*this));
			}
	};
}
MUU_NAMESPACE_END

namespace std
{
	/// \brief Specialization of std::tuple_size for muu::compressed_pair.
	template <typename First, typename Second>
	struct tuple_size<muu::compressed_pair<First, Second>>
	{
		static constexpr size_t value = 2;
	};

	/// \brief Specialization of std::tuple_element for muu::compressed_pair.
	template <size_t I, typename First, typename Second>
	struct tuple_element<I, muu::compressed_pair<First, Second>>
	{
		static_assert(I < 2);
		using type = std::conditional_t<I == 1, Second, First>;
	};
}

MUU_POP_WARNINGS // MUU_DISABLE_SPAM_WARNINGS, -Wreorder