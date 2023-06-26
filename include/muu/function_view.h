// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the definition of muu::function_view.

#include "meta.h"
#include "bit_cast.h"
#include "impl/core_utils.h"
#include "impl/std_memory.h" // addressof
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;

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

		template <typename Func,
				  bool = (std::is_class_v<remove_cvref<Func>>	 //
						  && std::is_empty_v<remove_cvref<Func>> //
						  && decays_to_function_pointer_by_unary_plus<Func>)>
		struct lambda_free_function_decay
		{
			using type = decltype(+std::declval<Func>());
		};

		template <typename Func>
		struct lambda_free_function_decay<Func, false>
		{
			using type = void;
		};

		struct function_view_payload
		{
			alignas(void*) unsigned char bytes[sizeof(void*)];
		};

		template <typename Func, typename Return, typename... Args>
		class function_view_base
		{
			// implementation is based on this article:
			// https://vittorioromeo.info/index/blog/passing_functions_to_functions.html

			static_assert(std::is_function_v<Func>);

		  private:
			template <typename U>
			static constexpr bool is_invocable = std::is_invocable_r_v<Return, U, Args...>;
			template <typename U>
			static constexpr bool is_nothrow_invocable = std::is_nothrow_invocable_r_v<Return, U, Args...>;

			static_assert(is_invocable<Func>);

			using invoker_func =
				std::conditional_t<is_nothrow_invocable<Func>,
								   Return(function_view_payload&, function_view_forwarding_arg<Args>...) noexcept,
								   Return(function_view_payload&, function_view_forwarding_arg<Args>...)>;

			mutable invoker_func* invoker_		   = {};
			mutable function_view_payload payload_ = {};

		  public:
			MUU_NODISCARD_CTOR
			constexpr function_view_base() noexcept = default;

			MUU_NODISCARD_CTOR
			constexpr function_view_base(const function_view_base&) noexcept = default;

			MUU_NODISCARD_CTOR
			constexpr function_view_base& operator=(const function_view_base&) noexcept = default;

			template <typename U>
			MUU_NODISCARD_CTOR
			explicit constexpr function_view_base([[maybe_unused]] U&& func) noexcept
			{
				static_assert(is_invocable<U&&>,
							  "the function_view's invocation target must match the function_view's signature");

				// noexcept -> throwing: ok
				// noexcept -> noexcept: ok
				// throwing -> throwing: ok
				// throwing -> noexcept: no dice
				static_assert(
					is_nothrow_invocable<U&&> || !is_nothrow_invocable<Func>,
					"a non-throwing function_view cannot invoke a throwing function (are you missing a 'noexcept'?)");

				// something we can manifest without needing to store at all
				if constexpr (is_trivially_manifestable<U>)
				{
					invoker_ = [](function_view_payload&,
								  function_view_forwarding_arg<Args>... args) noexcept(is_nothrow_invocable<Func>)
						-> decltype(auto) //
					{					  //
						return remove_cvref<U>{}(static_cast<function_view_forwarding_arg<Args>>(args)...);
					};
				}

				// already a free function pointer
				else if constexpr (std::is_pointer_v<remove_cvref<U>>
								   && std::is_function_v<std::remove_pointer_t<remove_cvref<U>>>)
				{
					payload_ = bit_cast<function_view_payload>(func);

					invoker_ = [](function_view_payload& payload,												   //
								  function_view_forwarding_arg<Args>... args) noexcept(is_nothrow_invocable<Func>) //
						-> decltype(auto)																		   //
					{																							   //
						return bit_cast<remove_cvref<U>>(payload)(
							static_cast<function_view_forwarding_arg<Args>>(args)...);
					};
				}

				// stateless lambdas (store as a free function pointer)
				else if constexpr (is_invocable<typename lambda_free_function_decay<std::remove_reference_t<U>>::type>)
				{
					payload_ = bit_cast<function_view_payload>(+func);

					invoker_ = [](function_view_payload& payload,												   //
								  function_view_forwarding_arg<Args>... args) noexcept(is_nothrow_invocable<Func>) //
						-> decltype(auto)																		   //
					{																							   //
						return bit_cast<decltype(+func)>(payload)(
							static_cast<function_view_forwarding_arg<Args>>(args)...);
					};
				}

				// anything else
				else
				{
					payload_ = bit_cast<function_view_payload>(std::addressof(func));

					invoker_ = [](function_view_payload& payload,												   //
								  function_view_forwarding_arg<Args>... args) noexcept(is_nothrow_invocable<Func>) //
						-> decltype(auto)																		   //
					{																							   //
						using ptr = std::add_pointer_t<std::remove_reference_t<U>>;

						return static_cast<U&&>(*bit_cast<ptr>(payload))(
							static_cast<function_view_forwarding_arg<Args>>(args)...);
					};
				}
			}

			MUU_ALWAYS_INLINE
			constexpr decltype(auto) operator()(Args... args) const noexcept(std::is_nothrow_invocable_v<Func, Args...>)
			{
				return invoker_(payload_, static_cast<function_view_forwarding_arg<Args>>(args)...);
			}

			MUU_PURE_INLINE_GETTER
			explicit constexpr operator bool() const noexcept
			{
				return !!invoker_;
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
	}
	/// \endcond

	/// \brief A non-owning, type-erasing view of a callable object (functions, lambdas, etc.).
	///
	/// \tparam Func A nominal function signature for callables that will be view by instances of this class, e.g.
	///				 `void(int)`.
	template <typename Func>
	class function_view //
		MUU_HIDDEN_BASE(public impl::function_view_base_selector<Func>::type)
	{
		static_assert(std::is_function_v<Func>, "Func must be a function type.");
		static_assert(!std::is_pointer_v<Func>, "Func may not be pointer type.");
		static_assert(!is_cvref<Func>, "Func may not be cvref-qualified.");
		static_assert(std::is_same_v<remove_callconv<Func>, Func>, "Func may not have an explicit calling convention.");

	  private:
		using base = typename impl::function_view_base_selector<Func>::type;

	  public:
		/// \brief the nomimal function type signature of this view.
		using func_type = Func;

		/// \brief Default-constructs a 'null' function view.
		MUU_NODISCARD_CTOR
		constexpr function_view() noexcept = default;

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr function_view(const function_view&) noexcept = default;

		/// \brief Copy-assignment operator.
		MUU_NODISCARD_CTOR
		constexpr function_view& operator=(const function_view&) noexcept = default;

		/// \brief	Constructs a function_view by wrapping a callable.
		///
		/// \tparam	F	 	A function, lambda or other callable type with a signature compatible with #func_type.
		///
		/// \param 	func	The callable to view/invoke.
		template <typename F>
		MUU_NODISCARD_CTOR
		/* implicit */ constexpr function_view(F&& func) noexcept //
			: base{ static_cast<F&&>(func) }
		{}

#if MUU_DOXYGEN

		/// \brief	Invokes the wrapped callable with the given arguments
		constexpr decltype(auto) operator()(Args&&...) const noexcept(...);

		/// \brief	Returns true if the view represents a callable (i.e. is not null).
		explicit constexpr operator bool() const noexcept;
#endif
	};

	// clang-format off
	/// \cond
#if 1

	// deduction guides for free function pointers + refs
	#define muu_make_function_view_deduction_guides(callconv, noex)                                                    \
	template <typename R, typename... P>                                                                               \
		function_view(R(callconv*)(P...) noex) -> function_view<R(P...) noex>; 

	MUU_FOR_EACH_CALLCONV_NOEXCEPT(muu_make_function_view_deduction_guides)
	#undef muu_make_function_view_deduction_guides

	namespace impl
	{
		template <typename Func,
				  bool = (std::is_class_v<std::remove_reference_t<Func>> || std::is_union_v<std::remove_reference_t<Func>>)
					&& has_unambiguous_function_call_operator<Func>,
				  bool = is_stateless_lambda<Func> || decays_to_function_pointer_by_unary_plus<Func>>
		struct function_view_deduction;

		// member function pointers
		template <typename>
		struct function_view_deduction_from_memptr;

		#define muu_make_function_view_deduction_guides(cvref, noex)                                                   \
			template <typename C, typename R, typename... P>                                                           \
			struct function_view_deduction_from_memptr<R (C::*)(P...) cvref noex>                                      \
			{                                                                                                          \
				using type = R(P...) noex;                                                                             \
			};

		MUU_FOR_EACH_CVREF_NOEXCEPT(muu_make_function_view_deduction_guides)
		#undef muu_make_function_view_deduction_guides

		// classes/unions with an unambiguous function call operator
		template <typename Func, bool SL>
		struct function_view_deduction<Func, true, SL>
			: function_view_deduction_from_memptr<muu::remove_callconv<decltype(&remove_cvref<Func>::operator())>>
		{};

		// stateless lambdas (and things that decay to function pointer by unary plus)
		template <typename Func>
		struct function_view_deduction<Func, false, true>
		{
			using type = muu::remove_callconv<std::remove_pointer_t<decltype(+std::declval<Func>())>>;
		};
	}

	// deduction guides for stateless lambdas and classes/unions with an unambiguous operator()
	MUU_CONSTRAINED_TEMPLATE(
		((std::is_class_v<std::remove_reference_t<Func>> || std::is_union_v<std::remove_reference_t<Func>>)
			&& has_unambiguous_function_call_operator<Func&&>)
		|| muu::is_stateless_lambda<Func&&>
		|| muu::decays_to_function_pointer_by_unary_plus<Func&&>,
		typename Func)
	function_view(Func&&) -> function_view<typename impl::function_view_deduction<Func&&>::type>;


#endif
	// clang-format on
	/// \endcond
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
