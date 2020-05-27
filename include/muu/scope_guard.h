// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::scope_guard.
#pragma once
#include "../muu/common.h"

MUU_PUSH_WARNINGS
MUU_DISABLE_PADDING_WARNINGS

namespace muu
{
	/// \brief	Performs actions when going out of scope.
	/// 
	/// \detail Use a scope_guard to simplify cleanup routines
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
			std::is_invocable_v<T>,
			"Types wrapped by a scope guard must be callable (functions, lambdas, etc.)"
		);
		static_assert(
			!build::has_exceptions || std::is_nothrow_invocable_v<T>,
			"Callables wrapped by a scope guard must be marked noexcept"
			" (throwing from destructors is a almost never a good move)"
		);
		static_assert(
			!build::has_exceptions || std::is_trivially_destructible_v<T> || std::is_nothrow_destructible_v<T>,
			"Callables wrapped by a scope guard must be nothrow-destructible"
		);

		private:
			T func_;
			bool suppressed = false;

		public:

			/// \brief	Constructs a scope_guard by wrapping a callable.
			///
			/// \tparam	U	 	A function, lambda or other callable with the signature `void() noexcept`.
			/// 
			/// \param 	func	The callable to invoke when the scope_guard goes out of scope.
			template <typename U, typename = std::enable_if_t<std::is_constructible_v<T, U&&>>>
			MUU_NODISCARD_CTOR
			explicit constexpr scope_guard(U&& func)
				noexcept(std::is_nothrow_constructible_v<T, U&&>)
				: func_{ std::forward<U>(func) }
			{}

			~scope_guard() noexcept
			{
				if (!suppressed)
					func_();
			}

			scope_guard(const scope_guard&) = delete;
			scope_guard(scope_guard&&) = delete;
			scope_guard& operator = (const scope_guard&) = delete;
			scope_guard& operator = (scope_guard&&) = delete;

			/// \brief	Suppresses invocation of the the scope_guard's wrapped callable.
			///
			/// \remarks This 'disables' a scope_guard, preventing the callable from being
			/// 		called when the scope_guard goes out of scope. In general it's better to
			/// 		structure RAII code to not require this sort of manoeuvring, but suppress()
			/// 		is provided as an 'escape hatch' if you have no other choice.
			void suppress() noexcept
			{
				suppressed = true;
			}
	};

	template <typename R, typename ...P>
	scope_guard(R(P...)noexcept) -> scope_guard<R(*)(P...)noexcept>;
	template <typename R, typename ...P>
	scope_guard(R(P...)) -> scope_guard<R(*)(P...)>;
	template <typename T>
	scope_guard(T&&) -> scope_guard<T>;
	template <typename T>
	scope_guard(T&) -> scope_guard<T>;
}

MUU_POP_WARNINGS // MUU_DISABLE_PADDING_WARNINGS
