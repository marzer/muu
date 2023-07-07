// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::compressed_pair.

#include "meta.h"
#include "impl/header_start.h"

namespace muu
{
	/// \addtogroup		core
	/// @{
	//% compressed_pair start

	/// \cond
	namespace impl
	{

#define MUU_COMPRESSED_PAIR_BASE_GETTERS(type, name, expression)                                                       \
	MUU_PURE_INLINE_GETTER                                                                                             \
	constexpr type& get_##name() noexcept                                                                              \
	{                                                                                                                  \
		return expression;                                                                                             \
	}                                                                                                                  \
	MUU_PURE_INLINE_GETTER                                                                                             \
	constexpr const type& get_##name() const noexcept                                                                  \
	{                                                                                                                  \
		return expression;                                                                                             \
	}                                                                                                                  \
	static_assert(true)

		// primary template - neither element can be a base
		template <typename First,
				  typename Second,
				  bool FirstCanBeBase  = std::is_empty_v<First> && !std::is_final_v<First>,
				  bool SecondCanBeBase = std::is_empty_v<Second> && !std::is_final_v<Second>>
		struct compressed_pair_base
		{
			static_assert(!FirstCanBeBase);
			static_assert(!SecondCanBeBase);
			MUU_NO_UNIQUE_ADDRESS First first_;
			MUU_NO_UNIQUE_ADDRESS Second second_;

			compressed_pair_base() = default;
			MUU_DEFAULT_MOVE(compressed_pair_base);
			MUU_DEFAULT_COPY(compressed_pair_base);

			MUU_CONSTRAINED_TEMPLATE((std::is_constructible_v<First, F&&> && std::is_constructible_v<Second, S&&>),
									 typename F,
									 typename S)
			constexpr compressed_pair_base(F&& first_init, S&& second_init) noexcept(
				std::is_nothrow_constructible_v<First, F&&>&& std::is_nothrow_constructible_v<Second, S&&>)
				: first_{ static_cast<F&&>(first_init) },
				  second_{ static_cast<S&&>(second_init) }
			{}

			MUU_COMPRESSED_PAIR_BASE_GETTERS(First, first, first_);
			MUU_COMPRESSED_PAIR_BASE_GETTERS(Second, second, second_);
		};

		// secondary template - First is a base
		template <typename First, typename Second>
		struct MUU_EMPTY_BASES compressed_pair_base<First, Second, true, false> //
			: First
		{
			MUU_NO_UNIQUE_ADDRESS Second second_;

			compressed_pair_base() = default;
			MUU_DEFAULT_MOVE(compressed_pair_base);
			MUU_DEFAULT_COPY(compressed_pair_base);

			MUU_CONSTRAINED_TEMPLATE((std::is_constructible_v<First, F&&> && std::is_constructible_v<Second, S&&>),
									 typename F,
									 typename S)
			constexpr compressed_pair_base(F&& first_init, S&& second_init) noexcept(
				std::is_nothrow_constructible_v<First, F&&>&& std::is_nothrow_constructible_v<Second, S&&>)
				: First{ static_cast<F&&>(first_init) },
				  second_{ static_cast<S&&>(second_init) }
			{}

			MUU_COMPRESSED_PAIR_BASE_GETTERS(First, first, *this);
			MUU_COMPRESSED_PAIR_BASE_GETTERS(Second, second, second_);
		};


		// secondary template - Second is a base
		template <typename First, typename Second>
		struct MUU_EMPTY_BASES compressed_pair_base<First, Second, false, true> //
			: Second
		{
			MUU_NO_UNIQUE_ADDRESS First first_;

			compressed_pair_base() = default;
			MUU_DEFAULT_MOVE(compressed_pair_base);
			MUU_DEFAULT_COPY(compressed_pair_base);

			MUU_CONSTRAINED_TEMPLATE((std::is_constructible_v<First, F&&> && std::is_constructible_v<Second, S&&>),
									 typename F,
									 typename S)
			constexpr compressed_pair_base(F&& first_init, S&& second_init) noexcept(
				std::is_nothrow_constructible_v<First, F&&>&& std::is_nothrow_constructible_v<Second, S&&>)
				: Second{ static_cast<S&&>(second_init) },
				  first_{ static_cast<F&&>(first_init) }

			{}

			MUU_COMPRESSED_PAIR_BASE_GETTERS(First, first, first_);
			MUU_COMPRESSED_PAIR_BASE_GETTERS(Second, second, *this);
		};

		// secondary template - both are bases
		template <typename First, typename Second>
		struct MUU_EMPTY_BASES compressed_pair_base<First, Second, true, true> //
			: First, Second
		{
			compressed_pair_base() = default;
			MUU_DEFAULT_MOVE(compressed_pair_base);
			MUU_DEFAULT_COPY(compressed_pair_base);

			MUU_CONSTRAINED_TEMPLATE((std::is_constructible_v<First, F&&> && std::is_constructible_v<Second, S&&>),
									 typename F,
									 typename S)
			constexpr compressed_pair_base(F&& first_init, S&& second_init) noexcept(
				std::is_nothrow_constructible_v<First, F&&>&& std::is_nothrow_constructible_v<Second, S&&>)
				: First{ static_cast<F&&>(first_init) },
				  Second{ static_cast<S&&>(second_init) }
			{}

			MUU_COMPRESSED_PAIR_BASE_GETTERS(First, first, *this);
			MUU_COMPRESSED_PAIR_BASE_GETTERS(Second, second, *this);
		};

#undef MUU_COMPRESSED_PAIR_BASE_DEFAULTS
#undef MUU_COMPRESSED_PAIR_BASE_GETTERS

		template <size_t I, typename T>
		MUU_PURE_INLINE_GETTER
		constexpr decltype(auto) compressed_pair_get(T&& cp) noexcept
		{
			static_assert(I <= 1);
			if constexpr (I == 0)
				return static_cast<T&&>(cp).first();
			else
				return static_cast<T&&>(cp).second();
		}
	}
	/// \endcond

	/// \brief	A pair that uses Empty Base Class Optimization
	///			to elide storage for one or both of its members where possible.
	///
	/// \tparam	First		First member type.
	/// \tparam	Second		Second member type.
	template <typename First, typename Second>
	class MUU_EMPTY_BASES compressed_pair //
		MUU_HIDDEN_BASE(private impl::compressed_pair_base<First, Second>)
	{
	  private:
		/// \cond

		using base = impl::compressed_pair_base<First, Second>;

		//# {{
		// mode hooks for debuggers etc.
		static constexpr bool first_is_base_  = std::is_empty_v<First> && !std::is_final_v<First>;
		static constexpr bool second_is_base_ = std::is_empty_v<Second> && !std::is_final_v<Second>;
		//# }}

		/// \endcond

	  public:
		/// \brief	The pair's first member type.
		using first_type = First;
		/// \brief	The pair's second member type.
		using second_type = Second;

		/// \brief	Default constructor.
		MUU_NODISCARD_CTOR
		compressed_pair() = default;

		/// \brief	Copy constructor.
		MUU_NODISCARD_CTOR
		compressed_pair(const compressed_pair&) = default;

		/// \brief	Move constructor.
		MUU_NODISCARD_CTOR
		compressed_pair(compressed_pair&&) = default;

		/// \brief	Copy-assignment operator.
		compressed_pair& operator=(const compressed_pair&) = default;

		/// \brief	Move-assignment operator.
		compressed_pair& operator=(compressed_pair&&) = default;

		/// \brief	Constructs a compressed pair from two values.
		///
		/// \tparam	F	First member initializer type.
		/// \tparam	S	Second member initializer type.
		/// \param 	first_init 	First member initializer.
		/// \param 	second_init	Second member initializer.
		MUU_CONSTRAINED_TEMPLATE((std::is_constructible_v<base, F&&, S&&>), typename F, typename S)
		MUU_NODISCARD_CTOR
		constexpr compressed_pair(F&& first_init,
								  S&& second_init) noexcept(std::is_nothrow_constructible_v<base, F&&, S&&>)
			: base{ static_cast<F&&>(first_init), static_cast<S&&>(second_init) }
		{}

		/// \brief	Returns an lvalue reference to the first member.
		MUU_PURE_INLINE_GETTER
		constexpr first_type& first() & noexcept
		{
			return base::get_first();
		}

		/// \brief	Returns an rvalue reference to the first member.
		MUU_PURE_INLINE_GETTER
		constexpr first_type&& first() && noexcept
		{
			return static_cast<first_type&&>(base::get_first());
		}

		/// \brief	Returns a const lvalue reference to the first member.
		MUU_PURE_INLINE_GETTER
		constexpr const first_type& first() const& noexcept
		{
			return base::get_first();
		}

		/// \brief	Returns a const rvalue reference to the first member.
		MUU_PURE_INLINE_GETTER
		constexpr const first_type&& first() const&& noexcept
		{
			return static_cast<const first_type&&>(base::get_first());
		}

		/// \brief	Returns an lvalue reference to the second member.
		MUU_PURE_INLINE_GETTER
		constexpr second_type& second() & noexcept
		{
			return base::get_second();
		}

		/// \brief	Returns an rvalue reference to the second member.
		MUU_PURE_INLINE_GETTER
		constexpr second_type&& second() && noexcept
		{
			return static_cast<second_type&&>(base::get_second());
		}

		/// \brief	Returns a const lvalue reference to the second member.
		MUU_PURE_INLINE_GETTER
		constexpr const second_type& second() const& noexcept
		{
			return base::get_second();
		}

		/// \brief	Returns a const rvalue reference to the second member.
		MUU_PURE_INLINE_GETTER
		constexpr const second_type&& second() const&& noexcept
		{
			return static_cast<const second_type&&>(base::get_second());
		}

		/// \brief	Returns an lvalue reference to the selected member.
		template <size_t I>
		MUU_PURE_INLINE_GETTER
		constexpr auto& get() & noexcept
		{
			return impl::compressed_pair_get<I>(*this);
		}

		/// \brief	Returns an rvalue reference to the selected member.
		template <size_t I>
		MUU_PURE_INLINE_GETTER
		constexpr auto&& get() && noexcept
		{
			return impl::compressed_pair_get<I>(static_cast<compressed_pair&&>(*this));
		}

		/// \brief	Returns a const lvalue reference to the selected member.
		template <size_t I>
		MUU_PURE_INLINE_GETTER
		constexpr const auto& get() const& noexcept
		{
			return impl::compressed_pair_get<I>(*this);
		}

		/// \brief	Returns a const rvalue reference to the selected member.
		template <size_t I>
		MUU_PURE_INLINE_GETTER
		constexpr const auto&& get() const&& noexcept
		{
			return impl::compressed_pair_get<I>(static_cast<const compressed_pair&&>(*this));
		}
	};

	/// \cond

	template <typename F, typename S>
	compressed_pair(F&&, S&&) -> compressed_pair<remove_cvref<F>, remove_cvref<S>>;

	/// \endcond

	//% compressed_pair end
	/// @}
}

namespace std
{
	//% compressed_pair::std start

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

	//% compressed_pair::std end
}

#include "impl/header_end.h"
