// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief Contains the definition of muu::function_view.

#pragma once
#include "impl/core_meta.h"
#include "impl/core_utils.h"

MUU_DISABLE_WARNINGS;
#include <memory> // addressof
MUU_ENABLE_WARNINGS;

#include "impl/header_start.h"

namespace muu
{
	/// \cond
	namespace impl
	{
		template <typename T>
		using function_view_forwarding_arg = std::conditional_t<(std::is_reference_v<T>	  //
																 || muu::is_arithmetic<T> //
																 || std::is_scalar_v<T>),
																T,
																T&&>;

		template <typename T,
				  bool = (std::is_class_v<muu::remove_cvref<T>>	   //
						  && std::is_empty_v<muu::remove_cvref<T>> //
						  && decays_to_function_pointer_by_unary_plus<T>)>
		struct lambda_free_function_decay
		{
			using type = decltype(+std::declval<T>());
		};

		template <typename T>
		struct lambda_free_function_decay<T, false>
		{
			using type = void;
		};

		template <typename T, typename Return, typename... Args>
		class function_view_base
		{
			// implementation is based on this article:
			// https://vittorioromeo.info/index/blog/passing_functions_to_functions.html

		  private:
			template <typename U>
			static constexpr bool is_invocable = std::is_invocable_r_v<Return, U, Args...>;
			template <typename U>
			static constexpr bool is_nothrow_invocable = std::is_nothrow_invocable_r_v<Return, U, Args...>;

			static_assert(is_invocable<T>);

			using invoker_func = std::conditional_t<is_nothrow_invocable<T>,
													Return(void*, function_view_forwarding_arg<Args>...) noexcept,
													Return(void*, function_view_forwarding_arg<Args>...)>;

			mutable void* func_			   = {};
			mutable invoker_func* invoker_ = {};

			// note: this _was_ a lambda but MSVC fails to parse it, finding an erroneous comma where none exists =/
			template <typename U>
			static decltype(auto) invoker(void* fn, function_view_forwarding_arg<Args>... args) //
				noexcept(is_nothrow_invocable<U>)
			{
				// free function pointers
				if constexpr (std::is_pointer_v<std::remove_reference_t<U>>)
				{
					using func_ptr = std::remove_reference_t<U>;
					return pointer_cast<func_ptr>(fn)(static_cast<function_view_forwarding_arg<Args>>(args)...);
				}

				// stateless lambdas
				else if constexpr (is_invocable<typename lambda_free_function_decay<U>::type>)
				{
					using func_ptr = typename lambda_free_function_decay<U>::type;
					return pointer_cast<func_ptr>(fn)(static_cast<function_view_forwarding_arg<Args>>(args)...);
				}

				// anything else
				else
				{
					using func_ptr = std::add_pointer_t<std::remove_reference_t<U>>;
					using ref	   = std::conditional_t<std::is_rvalue_reference_v<U>, U, std::remove_reference_t<U>&>;
					return static_cast<ref>(*pointer_cast<func_ptr>(fn))(
						static_cast<function_view_forwarding_arg<Args>>(args)...);
				}
			}

		  public:
			MUU_NODISCARD_CTOR
			constexpr function_view_base() noexcept = default;

			template <typename U>
			MUU_NODISCARD_CTOR
			explicit constexpr function_view_base(U&& func) noexcept
			{
				static_assert(is_invocable<U&&>,
							  "the function_view's invocation target must match the function_view's signature");

				// noexcept -> throwing: ok
				// noexcept -> noexcept: ok
				// throwing -> throwing: ok
				// throwing -> noexcept: no dice
				static_assert(
					is_nothrow_invocable<U&&> || !is_nothrow_invocable<T>,
					"a non-throwing function_view cannot invoke a throwing function (are you missing a 'noexcept'?)");

				// already a callable pointer (free function pointers)
				if constexpr (std::is_pointer_v<std::remove_reference_t<U>>)
					func_ = pointer_cast<void*>(func);

				// stateless lambdas (store as a free function pointer)
				else if constexpr (is_invocable<typename lambda_free_function_decay<U&&>::type>)
					func_ = pointer_cast<void*>(+static_cast<U&&>(func));

				// anything else
				else
					func_ = pointer_cast<void*>(std::addressof(func));

				invoker_ = invoker<U&&>;
			}

			constexpr decltype(auto) operator()(Args... args) const noexcept(std::is_nothrow_invocable_v<T, Args...>)
			{
				return invoker_(func_, static_cast<function_view_forwarding_arg<Args>>(args)...);
			}

			MUU_PURE_INLINE_GETTER
			explicit constexpr operator bool() const noexcept
			{
				return !!func_;
			}
		};

		template <typename>
		struct function_view_base_selector;

		template <typename Return, typename... Args>
		struct function_view_base_selector<Return(Args...) noexcept>
		{
			using type = function_view_base<Return(Args...) noexcept, Return, Args...>;
		};

		template <typename Return, typename... Args>
		struct function_view_base_selector<Return(Args...)>
		{
			using type = function_view_base<Return(Args...), Return, Args...>;
		};

		template <typename T>
		using function_view_ =
			typename function_view_base_selector<std::remove_pointer_t<std::remove_reference_t<T>>>::type;
	}
	/// \endcond

	/// \brief A non-owning view of a callable object (functions, lambdas, etc.).
	template <typename T>
	class function_view //
		MUU_HIDDEN_BASE(public impl::function_view_<T>)
	{
	  private:
		using base = impl::function_view_<T>;

	  public:
		/// \brief the nomimal function type signature of this view.
		using func_type = T;

		/// \brief Default-constructs a 'null' function view.
		MUU_NODISCARD_CTOR
		constexpr function_view() noexcept = default;

		/// \brief	Constructs a function_view by wrapping a callable.
		///
		/// \tparam	U	 	A function, lambda or other callable type with a signature compatible with #func_type.
		///
		/// \param 	func	The callable to view/invoke.
		template <typename U>
		MUU_NODISCARD_CTOR
		explicit constexpr function_view(U&& func) noexcept //
			: base{ static_cast<U&&>(func) }
		{}

#ifdef DOXYGEN

		/// \brief	Invokes the wrapped callable with the given arguments
		constexpr decltype(auto) operator()(Args&&...) const noexcept(...);

		/// \brief	Returns true if the view represents a callable (i.e. is not null).
		explicit constexpr operator bool() const noexcept;
#endif
	};

	/// \cond

	template <typename R, typename... P>
	function_view(R (*)(P...) noexcept) -> function_view<R(P...) noexcept>;

	template <typename R, typename... P>
	function_view(R (*)(P...)) -> function_view<R(P...)>;

	/// \endcond
}

#include "impl/header_end.h"
