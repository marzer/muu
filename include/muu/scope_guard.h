//# This file is a part of muu and is subject to the the terms of the MIT license.
//# Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
//# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.

#pragma once
#include "../muu/common.h"

namespace muu
{
	template <typename T>
	class scope_guard final
	{
		static_assert(
			std::is_invocable_v<T>,
			"Types wrapped by a scope guard must be callable (functions, lambdas, etc.)"
		);
		static_assert(
			std::is_same_v<decltype(std::declval<T>()()), void>,
			"Callables wrapped by a scope guard must return void"
			" (there would be no way to recover the returned value)"
		);
		static_assert(
			!build::exceptions_enabled || std::is_nothrow_invocable_v<T>,
			"Callables wrapped by a scope guard must be marked noexcept"
			" (throwing from destructors is a universally terrible idea)"
		);
		static_assert(
			!build::exceptions_enabled || std::is_trivially_destructible_v<T> || std::is_nothrow_destructible_v<T>,
			"Callables wrapped by a scope guard must be nothrow-destructible"
		);

		private:
			T func_;
			bool suppressed = false;

		public:

			template <typename U, typename = std::enable_if_t<std::is_constructible_v<T, U&&>>>
			MUU_NODISCARD_CTOR
			explicit constexpr scope_guard(U&& func) noexcept(std::is_nothrow_constructible_v<T, U&&>)
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

			void suppress() noexcept
			{
				suppressed = true;
			}
	};

	template <typename R, typename ...P>
	scope_guard(R(P...) noexcept) -> scope_guard<R(*)(P...) noexcept>;
	template <typename R, typename ...P>
	scope_guard(R(P...)) -> scope_guard<R(*)(P...)>;
	template <typename T>
	scope_guard(T&&) -> scope_guard<T>;
	template <typename T>
	scope_guard(T&) -> scope_guard<T>;
}
