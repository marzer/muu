// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::scope_guard.

#pragma once
#include "../muu/core.h"
#include "../muu/compressed_pair.h"

MUU_PUSH_WARNINGS;
MUU_DISABLE_SPAM_WARNINGS;

namespace muu
{
	MUU_ABI_VERSION_START(0);

	/// \brief	Performs actions when going out of scope.
	/// \ingroup core
	///
	/// \details Use a scope_guard to simplify cleanup routines
	/// 		or code that has acquire/release semantics, e.g. locking: \cpp
	/// 
	/// void do_something()
	/// {
	///		acquire_magic_lock();
	///		scope_guard sg{ release_magic_lock };
	///		something_that_throws();
	/// }
	/// 
	/// \ecpp
	/// 
	/// For comparison's sake, here's the same function without a scope_guard: \cpp
	/// 
	/// void do_something()
	/// {
	///		acquire_magic_lock();
	///		try
	///		{
	///			something_that_throws();
	///		}
	///		catch (...)
	///		{
	///			release_magic_lock();
	///			throw;
	///		}
	///		release_magic_lock();
	/// }
	/// 
	/// \ecpp
	/// 
	/// \tparam	T	A `noexcept` function, lambda or other callable not requiring
	/// 			any arguments (e.g. `void() noexcept`)`.
	template <typename T>
	class scope_guard
	{
		static_assert(
			std::is_invocable_v<std::remove_reference_t<T>>,
			"Types wrapped by a scope guard must be callable (functions, lambdas, etc.)"
		);
		static_assert(
			!build::has_exceptions
			|| std::is_nothrow_invocable_v<std::remove_reference_t<T>>,
			"Callables wrapped by a scope guard must be marked noexcept"
		);
		static_assert(
			!build::has_exceptions
			|| std::is_nothrow_destructible_v<std::remove_reference_t<T>>,
			"Callables wrapped by a scope guard must be nothrow-destructible"
		);

		private:
			compressed_pair<T, bool> func_and_active;

		public:

			/// \brief	Constructs a scope_guard by wrapping a callable.
			///
			/// \tparam	U	 	A function, lambda or other callable with the signature `void() noexcept`.
			/// 
			/// \param 	func	The callable to invoke when the scope_guard goes out of scope.
			template <typename U>
			MUU_NODISCARD_CTOR
			explicit
			constexpr scope_guard(U&& func) noexcept
				: func_and_active{ static_cast<U&&>(func), true }
			{
				static_assert(
					!build::has_exceptions
					|| std::is_nothrow_constructible_v<std::remove_reference_t<T>, U&&>,
					"A scope_guard's callable must be nothrow-constructible from its initializer"
				);
			}

			~scope_guard() noexcept
			{
				if (func_and_active.second())
					func_and_active.first()();
			}

			scope_guard(const scope_guard&) = delete;
			scope_guard(scope_guard&&) = delete;
			scope_guard& operator = (const scope_guard&) = delete;
			scope_guard& operator = (scope_guard&&) = delete;

			/// \brief	Dismisses the scope guard, cancelling invocation of the wrapped callable.
			void dismiss() noexcept
			{
				func_and_active.second() = false;
			}
	};

	/// \cond deduction_guides

	template <typename R, typename ...P>
	scope_guard(R(P...)noexcept) -> scope_guard<R(*)(P...)noexcept>;
	template <typename R, typename ...P>
	scope_guard(R(P...)) -> scope_guard<R(*)(P...)>;
	template <typename T>
	scope_guard(T&&) -> scope_guard<T>;
	template <typename T>
	scope_guard(T&) -> scope_guard<T&>;

	/// \endcond

	MUU_ABI_VERSION_END;
}

MUU_POP_WARNINGS; // MUU_DISABLE_SPAM_WARNINGS
