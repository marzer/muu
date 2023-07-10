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

#define MUU_COMPRESSED_PAIR_BASE_GETTERS(T, name, expression)                                                          \
	MUU_PURE_INLINE_GETTER                                                                                             \
	constexpr T& name()& noexcept                                                                                      \
	{                                                                                                                  \
		return static_cast<T&>(expression);                                                                            \
	}                                                                                                                  \
	MUU_PURE_INLINE_GETTER                                                                                             \
	constexpr const T& name() const& noexcept                                                                          \
	{                                                                                                                  \
		return static_cast<const T&>(expression);                                                                      \
	}                                                                                                                  \
	MUU_PURE_INLINE_GETTER                                                                                             \
	constexpr T&& name()&& noexcept                                                                                    \
	{                                                                                                                  \
		return static_cast<T&&>(expression);                                                                           \
	}                                                                                                                  \
	MUU_PURE_INLINE_GETTER                                                                                             \
	constexpr const T&& name() const&& noexcept                                                                        \
	{                                                                                                                  \
		return static_cast<const T&&>(expression);                                                                     \
	}                                                                                                                  \
	static_assert(true)

		// primary template - neither element can be a base
		template <typename First,
				  typename Second,
				  bool FirstCanBeBase  = std::is_empty_v<First> && !std::is_final_v<First>,
				  bool SecondCanBeBase = std::is_empty_v<Second> && !std::is_final_v<Second>>
		class compressed_pair_base
		{
		  private:
			static_assert(!FirstCanBeBase);
			static_assert(!SecondCanBeBase);
			MUU_NO_UNIQUE_ADDRESS First first_;
			MUU_NO_UNIQUE_ADDRESS Second second_;

		  public:
			compressed_pair_base() = default;
			MUU_DEFAULT_MOVE(compressed_pair_base);
			MUU_DEFAULT_COPY(compressed_pair_base);

			MUU_CONSTRAINED_TEMPLATE((std::is_constructible_v<First, F&&> && std::is_constructible_v<Second, S&&>),
									 typename F,
									 typename S)
			constexpr compressed_pair_base(F&& first_init, S&& second_init) //
				noexcept(std::is_nothrow_constructible_v<First, F&&>&& std::is_nothrow_constructible_v<Second, S&&>)
				: first_{ static_cast<F&&>(first_init) },
				  second_{ static_cast<S&&>(second_init) }
			{}

			MUU_COMPRESSED_PAIR_BASE_GETTERS(First, first, first_);
			MUU_COMPRESSED_PAIR_BASE_GETTERS(Second, second, second_);
		};

		// secondary template - First is a base
		template <typename First, typename Second>
		class MUU_EMPTY_BASES compressed_pair_base<First, Second, true, false> //
			: private First
		{
		  private:
			MUU_NO_UNIQUE_ADDRESS Second second_;

		  public:
			compressed_pair_base() = default;
			MUU_DEFAULT_MOVE(compressed_pair_base);
			MUU_DEFAULT_COPY(compressed_pair_base);

			MUU_CONSTRAINED_TEMPLATE((std::is_constructible_v<First, F&&> && std::is_constructible_v<Second, S&&>),
									 typename F,
									 typename S)
			constexpr compressed_pair_base(F&& first_init, S&& second_init) //
				noexcept(std::is_nothrow_constructible_v<First, F&&>&& std::is_nothrow_constructible_v<Second, S&&>)
				: First{ static_cast<F&&>(first_init) },
				  second_{ static_cast<S&&>(second_init) }
			{}

			MUU_COMPRESSED_PAIR_BASE_GETTERS(First, first, *this);
			MUU_COMPRESSED_PAIR_BASE_GETTERS(Second, second, second_);
		};

		// secondary template - Second is a base
		template <typename First, typename Second>
		class MUU_EMPTY_BASES compressed_pair_base<First, Second, false, true> //
			: private Second
		{
		  private:
			MUU_NO_UNIQUE_ADDRESS First first_;

		  public:
			compressed_pair_base() = default;
			MUU_DEFAULT_MOVE(compressed_pair_base);
			MUU_DEFAULT_COPY(compressed_pair_base);

			MUU_CONSTRAINED_TEMPLATE((std::is_constructible_v<First, F&&> && std::is_constructible_v<Second, S&&>),
									 typename F,
									 typename S)
			constexpr compressed_pair_base(F&& first_init, S&& second_init) //
				noexcept(std::is_nothrow_constructible_v<First, F&&>&& std::is_nothrow_constructible_v<Second, S&&>)
				: Second{ static_cast<S&&>(second_init) },
				  first_{ static_cast<F&&>(first_init) }

			{}

			MUU_COMPRESSED_PAIR_BASE_GETTERS(First, first, first_);
			MUU_COMPRESSED_PAIR_BASE_GETTERS(Second, second, *this);
		};

		// secondary template - both are bases
		template <typename First, typename Second>
		class MUU_EMPTY_BASES compressed_pair_base<First, Second, true, true> //
			: private First, private Second
		{
		  public:
			compressed_pair_base() = default;
			MUU_DEFAULT_MOVE(compressed_pair_base);
			MUU_DEFAULT_COPY(compressed_pair_base);

			MUU_CONSTRAINED_TEMPLATE((std::is_constructible_v<First, F&&> && std::is_constructible_v<Second, S&&>),
									 typename F,
									 typename S)
			constexpr compressed_pair_base(F&& first_init, S&& second_init) //
				noexcept(std::is_nothrow_constructible_v<First, F&&>&& std::is_nothrow_constructible_v<Second, S&&>)
				: First{ static_cast<F&&>(first_init) },
				  Second{ static_cast<S&&>(second_init) }
			{}

			MUU_COMPRESSED_PAIR_BASE_GETTERS(First, first, *this);
			MUU_COMPRESSED_PAIR_BASE_GETTERS(Second, second, *this);
		};

#undef MUU_COMPRESSED_PAIR_BASE_DEFAULTS
#undef MUU_COMPRESSED_PAIR_BASE_GETTERS
	}
	/// \endcond

	/// \brief	A pair that uses Empty Base Class Optimization
	///			to elide storage for one or both of its members where possible.
	///
	/// \tparam	First		First member type.
	/// \tparam	Second		Second member type.
	template <typename First, typename Second>
	class MUU_EMPTY_BASES compressed_pair //
		MUU_HIDDEN_BASE(public impl::compressed_pair_base<First, Second>)
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

#if MUU_DOXYGEN
		//# {{

		/// \brief	Constructs a compressed pair from two values.
		///
		/// \tparam	F	First member initializer type.
		/// \tparam	S	Second member initializer type.
		/// \param 	first_init 	First member initializer.
		/// \param 	second_init	Second member initializer.
		template <typename F, typename S>
		constexpr compressed_pair(F&& first_init,
								  S&& second_init) noexcept(std::is_nothrow_constructible_v<base, F&&, S&&>);

		/// \brief	Returns an lvalue reference to the first member.
		constexpr first_type& first() & noexcept;

		/// \brief	Returns an rvalue reference to the first member.
		constexpr first_type&& first() && noexcept;

		/// \brief	Returns a const lvalue reference to the first member.
		constexpr const first_type& first() const& noexcept;

		/// \brief	Returns a const rvalue reference to the first member.
		constexpr const first_type&& first() const&& noexcept;

		/// \brief	Returns an lvalue reference to the second member.
		constexpr second_type& second() & noexcept;

		/// \brief	Returns an rvalue reference to the second member.
		constexpr second_type&& second() && noexcept;

		/// \brief	Returns a const lvalue reference to the second member.
		constexpr const second_type& second() const& noexcept;

		/// \brief	Returns a const rvalue reference to the second member.
		constexpr const second_type&& second() const&& noexcept;

		//# }}
#else
		using impl::compressed_pair_base<First, Second>::compressed_pair_base; // inherit constructor
#endif

	  private:
		template <size_t I, typename T>
		MUU_PURE_INLINE_GETTER
		static constexpr decltype(auto) do_get(T&& cp) noexcept
		{
			static_assert(I <= 1);
			if constexpr (I == 0)
				return static_cast<T&&>(cp).first();
			else
				return static_cast<T&&>(cp).second();
		}

	  public:
		/// \brief	Returns an lvalue reference to the selected member.
		template <size_t I>
		MUU_PURE_INLINE_GETTER
		constexpr auto& get() & noexcept
		{
			return do_get<I>(*this);
		}

		/// \brief	Returns an rvalue reference to the selected member.
		template <size_t I>
		MUU_PURE_INLINE_GETTER
		constexpr auto&& get() && noexcept
		{
			return do_get<I>(static_cast<compressed_pair&&>(*this));
		}

		/// \brief	Returns a const lvalue reference to the selected member.
		template <size_t I>
		MUU_PURE_INLINE_GETTER
		constexpr const auto& get() const& noexcept
		{
			return do_get<I>(*this);
		}

		/// \brief	Returns a const rvalue reference to the selected member.
		template <size_t I>
		MUU_PURE_INLINE_GETTER
		constexpr const auto&& get() const&& noexcept
		{
			return do_get<I>(static_cast<const compressed_pair&&>(*this));
		}

		/// \brief	Swaps two pairs.
		/// \availability This function is only available when std::is_swappable_v is true for both element types.
		MUU_HIDDEN_CONSTRAINT(std::is_swappable_v<F>&& std::is_swappable_v<S>, typename F = First, typename S = Second)
		void swap(compressed_pair& other) noexcept(std::is_nothrow_swappable_v<F>&& std::is_nothrow_swappable_v<S>)
		{
			using std::swap;
			swap(base::first(), other.first());
			swap(base::second(), other.second());
		}
	};

	/// \cond

	template <typename F, typename S>
	compressed_pair(F&&, S&&) -> compressed_pair<remove_cvref<F>, remove_cvref<S>>;

	/// \endcond

	/// \brief	Swaps two compressed pairs.
	/// \availability This overload is only available when std::is_swappable_v is true for both element types.
	MUU_CONSTRAINED_TEMPLATE(std::is_swappable_v<F>&& std::is_swappable_v<S>, typename F, typename S)
	MUU_ALWAYS_INLINE
	void swap(compressed_pair<F, S>& lhs,
			  compressed_pair<F, S>& rhs) noexcept(std::is_nothrow_swappable_v<F>&& std::is_nothrow_swappable_v<S>)
	{
		lhs.swap(rhs);
	}

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
