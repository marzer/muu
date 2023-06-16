// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::scope_guard.

#include "compressed_pair.h"
#include "impl/std_exception.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;

namespace muu
{
	/// \cond
	namespace impl
	{
		enum class scope_guard_flags : unsigned
		{
			none			= 0,
			stores_callable = 1,
			dismissable		= 2,
			exception_aware = 4,
			all				= stores_callable | dismissable | exception_aware
		};
		MUU_MAKE_FLAGS(scope_guard_flags);

		template <typename T, scope_guard_flags Flags>
		class scope_guard_storage;

		template <typename T>
		class scope_guard_storage<T, scope_guard_flags::none>
		{};

		template <typename T>
		class scope_guard_storage<T, scope_guard_flags::stores_callable>
		{
		  private:
			MUU_NO_UNIQUE_ADDRESS T callable_;

		  protected:
			template <typename U>
			MUU_NODISCARD_CTOR
			scope_guard_storage(U&& callable) //
				noexcept(!build::supports_exceptions || noexcept(T{ std::declval<U&&>() }))
				: callable_{ static_cast<U&&>(callable) }
			{}

			MUU_DELETE_MOVE(scope_guard_storage);
			MUU_DELETE_COPY(scope_guard_storage);

			MUU_PURE_INLINE_GETTER
			auto& callable() noexcept
			{
				return callable_;
			}
		};

		template <typename T>
		class scope_guard_storage<T, scope_guard_flags::dismissable>
		{
			static_assert(!std::is_reference_v<T> && is_trivially_manifestable<T>);

		  private:
			bool dismissed_ = false;

		  public:
			MUU_PURE_INLINE_GETTER
			bool dismissed() const noexcept
			{
				return dismissed_;
			}

			MUU_ALWAYS_INLINE
			void dismiss() noexcept
			{
				dismissed_ = true;
			}
		};

		template <typename T>
		class scope_guard_storage<T, scope_guard_flags::exception_aware>
		{
			static_assert(!std::is_reference_v<T> && is_trivially_manifestable<T>);

		  private:
			int uncaught_exceptions_ = std::uncaught_exceptions();

		  protected:
			MUU_PURE_INLINE_GETTER
			int uncaught_exceptions() const noexcept
			{
				return uncaught_exceptions_;
			}
		};

		template <typename T>
		class scope_guard_storage<T, scope_guard_flags::stores_callable | scope_guard_flags::dismissable>
		{
		  private:
			compressed_pair<T, bool> data_;

		  protected:
			template <typename U>
			MUU_NODISCARD_CTOR
			scope_guard_storage(U&& callable) //
				noexcept(!build::supports_exceptions || noexcept(decltype(data_){ std::declval<U&&>(), false }))
				: data_{ static_cast<U&&>(callable), false }
			{}

			MUU_DELETE_MOVE(scope_guard_storage);
			MUU_DELETE_COPY(scope_guard_storage);

			MUU_PURE_INLINE_GETTER
			auto& callable() noexcept
			{
				return data_.first();
			}

		  public:
			MUU_PURE_INLINE_GETTER
			bool dismissed() const noexcept
			{
				return data_.second();
			}

			MUU_ALWAYS_INLINE
			void dismiss() noexcept
			{
				data_.second() = true;
			}
		};

		template <typename T>
		class scope_guard_storage<T, scope_guard_flags::stores_callable | scope_guard_flags::exception_aware>
		{
		  private:
			compressed_pair<T, int> data_;

		  protected:
			template <typename U>
			MUU_NODISCARD_CTOR
			scope_guard_storage(U&& callable) //
				noexcept(!build::supports_exceptions || noexcept(decltype(data_){ std::declval<U&&>(), {} }))
				: data_{ static_cast<U&&>(callable), std::uncaught_exceptions() }
			{}

			MUU_DELETE_MOVE(scope_guard_storage);
			MUU_DELETE_COPY(scope_guard_storage);

			MUU_PURE_INLINE_GETTER
			auto& callable() noexcept
			{
				return data_.first();
			}

			MUU_PURE_INLINE_GETTER
			int uncaught_exceptions() const noexcept
			{
				return data_.second();
			}
		};

		inline constexpr unsigned scope_guard_dismissed_mask = 1u << (sizeof(unsigned) * CHAR_BIT - 1);
		inline constexpr unsigned scope_guard_excount_mask	 = ~scope_guard_dismissed_mask;

		template <typename T>
		class scope_guard_storage<T, scope_guard_flags::dismissable | scope_guard_flags::exception_aware>
		{
			static_assert(!std::is_reference_v<T> && is_trivially_manifestable<T>);

		  private:
			unsigned data_ = static_cast<unsigned>(std::uncaught_exceptions());

		  protected:
			MUU_PURE_INLINE_GETTER
			int uncaught_exceptions() const noexcept
			{
				return static_cast<int>(data_ & scope_guard_excount_mask);
			}

		  public:
			MUU_PURE_INLINE_GETTER
			bool dismissed() const noexcept
			{
				return static_cast<bool>(data_ & scope_guard_dismissed_mask);
			}

			MUU_ALWAYS_INLINE
			void dismiss() noexcept
			{
				data_ |= scope_guard_dismissed_mask;
			}
		};

		template <typename T>
		class scope_guard_storage<T, scope_guard_flags::all>
		{
		  private:
			compressed_pair<T, unsigned> data_;

		  protected:
			template <typename U>
			MUU_NODISCARD_CTOR
			scope_guard_storage(U&& callable) //
				noexcept(!build::supports_exceptions || noexcept(decltype(data_){ std::declval<U&&>(), false }))
				: data_{ static_cast<U&&>(callable), static_cast<unsigned>(std::uncaught_exceptions()) }
			{}

			MUU_DELETE_MOVE(scope_guard_storage);
			MUU_DELETE_COPY(scope_guard_storage);

			MUU_PURE_INLINE_GETTER
			auto& callable() noexcept
			{
				return data_.first();
			}

			MUU_PURE_INLINE_GETTER
			int uncaught_exceptions() const noexcept
			{
				return static_cast<int>(data_.second() & scope_guard_excount_mask);
			}

		  public:
			MUU_PURE_INLINE_GETTER
			bool dismissed() const noexcept
			{
				return static_cast<bool>(data_.second() & scope_guard_dismissed_mask);
			}

			MUU_ALWAYS_INLINE
			void dismiss() noexcept
			{
				data_.second() |= scope_guard_dismissed_mask;
			}
		};

		enum class scope_guard_mode : unsigned
		{
			invoke_always,
			invoke_on_fail,
			invoke_on_success,
		};

		template <typename T, scope_guard_mode Mode, bool Dismissable>
		inline constexpr scope_guard_flags scope_guard_base_flags =
			(Dismissable ? scope_guard_flags::dismissable : scope_guard_flags::none) //
			| (Mode != scope_guard_mode::invoke_always ? scope_guard_flags::exception_aware : scope_guard_flags::none)
			| (std::is_reference_v<T> || !is_trivially_manifestable<T> ? scope_guard_flags::stores_callable
																	   : scope_guard_flags::none);

		template <typename T, scope_guard_mode Mode, bool Dismissable = true>
		class MUU_EMPTY_BASES scope_guard_base
			: public scope_guard_storage<T, scope_guard_base_flags<T, Mode, Dismissable>>
		{
			static_assert(!std::is_rvalue_reference_v<T>,
						  "Callables wrapped by a scope guard may not be rvalue references");
			static_assert(!build::supports_exceptions
							  || std::is_nothrow_invocable_v<std::add_lvalue_reference_t<std::remove_reference_t<T>>>,
						  "Callables wrapped by a scope guard must be nothrow-invocable (functions, lambdas, etc.)");
			static_assert(std::is_invocable_v<std::add_lvalue_reference_t<std::remove_reference_t<T>>>,
						  "Callables wrapped by a scope guard must be invocable (functions, lambdas, etc.)");
			static_assert(!build::supports_exceptions || std::is_reference_v<T> || std::is_nothrow_destructible_v<T>,
						  "Callables wrapped by a scope guard must be nothrow-destructible");
			static_assert(std::is_reference_v<T> || std::is_destructible_v<T>,
						  "Callables wrapped by a scope guard must be destructible");

			static constexpr scope_guard_flags flags = scope_guard_base_flags<T, Mode, Dismissable>;
			using base								 = scope_guard_storage<T, flags>;

		  public:
			MUU_CONSTRAINED_TEMPLATE(F& scope_guard_flags::stores_callable, typename U, auto F = flags)
			MUU_NODISCARD_CTOR
			explicit scope_guard_base(U&& callable) noexcept(std::is_nothrow_constructible_v<base, U&&>)
				: base{ static_cast<U&&>(callable) }
			{
				static_assert(muu::is_convertible<U&&, T>,
							  "A scope guard's callable must be constructible from its initializer");
			}

			MUU_CONSTRAINED_TEMPLATE(!(F & scope_guard_flags::stores_callable), typename U, auto F = flags)
			MUU_NODISCARD_CTOR
			explicit scope_guard_base(U&&) noexcept
			{}

			~scope_guard_base() noexcept
			{
				if constexpr (Mode == scope_guard_mode::invoke_on_fail)
				{
					if (base::uncaught_exceptions() == std::uncaught_exceptions())
						return;
				}
				else if constexpr (Mode == scope_guard_mode::invoke_on_success)
				{
					if (base::uncaught_exceptions() != std::uncaught_exceptions())
						return;
				}

				if constexpr (!!(flags & scope_guard_flags::dismissable))
				{
					if MUU_UNLIKELY(base::dismissed())
						return;
				}

				if constexpr (!!(flags & scope_guard_flags::stores_callable))
				{
					base::callable()();
				}
				else
				{
					static_assert(!std::is_reference_v<T> && is_trivially_manifestable<T>);
					T{}();
				}
			}

			MUU_DELETE_MOVE(scope_guard_base);
			MUU_DELETE_COPY(scope_guard_base);
		};

		template <typename T>
		using scope_guard_param_selector =
			std::conditional_t<is_trivially_manifestable<remove_cvref<T>>, remove_cvref<T>, T>;
	}
	/// \endcond

	/// \brief	Performs actions when going out of scope.
	/// \ingroup core
	///
	/// \details Use a scope_guard to simplify cleanup routines
	/// 		or code that has acquire/release semantics, e.g. locking: \cpp
	///
	/// void do_work()
	/// {
	///		acquire_magic_lock();
	///		muu::scope_guard sg{ release_magic_lock };
	///		something_that_throws();
	/// }
	///
	/// \ecpp
	///
	/// For comparison's sake, here's the same function without a scope_guard: \cpp
	///
	/// void do_work()
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
	///
	/// \see
	/// 	- scope_fail
	/// 	- scope_success
	template <typename T>
	class MUU_EMPTY_BASES scope_guard //
		MUU_HIDDEN_BASE(public impl::scope_guard_base<impl::scope_guard_param_selector<T>, impl::scope_guard_mode::invoke_always>)
	{
		using base = impl::scope_guard_base<impl::scope_guard_param_selector<T>, impl::scope_guard_mode::invoke_always>;

	  public:
		/// \brief	Constructs a scope_guard by wrapping a callable.
		///
		/// \tparam	U	 	A function, lambda or other callable with the signature `void() noexcept`.
		///
		/// \param 	func	The callable to invoke when the scope_guard goes out of scope.
		template <typename U>
		MUU_NODISCARD_CTOR
		explicit scope_guard(U&& func) noexcept(std::is_nothrow_constructible_v<base, U&&>)
			: base{ static_cast<U&&>(func) }
		{}

		MUU_DELETE_MOVE(scope_guard);
		MUU_DELETE_COPY(scope_guard);

#if MUU_DOXYGEN
		/// \brief	Dismisses the scope guard, cancelling invocation of the wrapped callable.
		void dismiss() noexcept;

		/// \brief	Returns true if the scope guard has been dismissed.
		bool dismissed() const noexcept;
#endif
	};

	/// \cond

	template <typename R, typename... P>
	scope_guard(R(P...) noexcept) -> scope_guard<R (*)(P...) noexcept>;

	template <typename R, typename... P>
	scope_guard(R(P...)) -> scope_guard<R (*)(P...)>;

	template <typename T>
	scope_guard(T&&) -> scope_guard<impl::scope_guard_param_selector<T>>;

	template <typename T>
	scope_guard(T&) -> scope_guard<impl::scope_guard_param_selector<T&>>;

	/// \endcond

	/// \brief	Performs actions when going out of scope due to an exception being thrown.
	/// \ingroup core
	///
	/// \details Use a scope_fail to simplify cleanup routines
	/// 		or code that has acquire/release semantics: \cpp
	///
	/// void* get_initialized_buffer()
	/// {
	///		void* buffer = acquire(1024);
	///		muu::scope_fail err{ [=]() noexcept { release(buffer); }};
	///		//
	///		// ...a bunch of initialization code that might throw...
	///		//
	///		return buffer;
	/// }
	///
	/// \ecpp
	///
	/// For comparison's sake, here's the same function without a scope_fail: \cpp
	///
	/// void* get_initialized_buffer()
	/// {
	///		void* buffer = acquire(1024);
	///		try
	///		{
	///			//
	///			// ...a bunch of initialization code that might throw...
	///			//
	///		}
	///		catch (...)
	///		{
	///			release(buffer);
	///			throw;
	///		}
	///		return buffer;
	/// }
	///
	/// \ecpp
	///
	/// \tparam	T	A `noexcept` function, lambda or other callable not requiring
	/// 			any arguments (e.g. `void() noexcept`)`.
	///
	/// \see
	/// 	- scope_guard
	/// 	- scope_success
	template <typename T>
	class MUU_EMPTY_BASES scope_fail //
		MUU_HIDDEN_BASE(public impl::scope_guard_base<impl::scope_guard_param_selector<T>, impl::scope_guard_mode::invoke_on_fail>)
	{
		using base =
			impl::scope_guard_base<impl::scope_guard_param_selector<T>, impl::scope_guard_mode::invoke_on_fail>;

	  public:
		/// \brief	Constructs a scope_fail by wrapping a callable.
		///
		/// \tparam	U	 	A function, lambda or other callable with the signature `void() noexcept`.
		///
		/// \param 	func	The callable to invoke when the scope guard goes out of scope.
		template <typename U>
		MUU_NODISCARD_CTOR
		explicit scope_fail(U&& func) noexcept(std::is_nothrow_constructible_v<base, U&&>)
			: base{ static_cast<U&&>(func) }
		{}

#if MUU_DOXYGEN
		/// \brief	Dismisses the scope guard, cancelling invocation of the wrapped callable.
		void dismiss() noexcept;

		/// \brief	Returns true if the scope guard has been dismissed.
		bool dismissed() const noexcept;
#endif
	};

	/// \cond

	template <typename R, typename... P>
	scope_fail(R(P...) noexcept) -> scope_fail<R (*)(P...) noexcept>;

	template <typename R, typename... P>
	scope_fail(R(P...)) -> scope_fail<R (*)(P...)>;

	template <typename T>
	scope_fail(T&&) -> scope_fail<impl::scope_guard_param_selector<T>>;

	template <typename T>
	scope_fail(T&) -> scope_fail<impl::scope_guard_param_selector<T&>>;

	/// \endcond

	/// \brief	Performs actions when going out of scope only if no exceptions have been thrown.
	/// \ingroup core
	///
	/// \tparam	T	A `noexcept` function, lambda or other callable not requiring
	/// 			any arguments (e.g. `void() noexcept`)`.
	///
	/// \see
	/// 	- scope_guard
	/// 	- scope_fail
	template <typename T>
	class MUU_EMPTY_BASES scope_success //
		MUU_HIDDEN_BASE(public impl::scope_guard_base<impl::scope_guard_param_selector<T>, impl::scope_guard_mode::invoke_on_success>)
	{
		using base =
			impl::scope_guard_base<impl::scope_guard_param_selector<T>, impl::scope_guard_mode::invoke_on_success>;

	  public:
		/// \brief	Constructs a scope_success by wrapping a callable.
		///
		/// \tparam	U	 	A function, lambda or other callable with the signature `void() noexcept`.
		///
		/// \param 	func	The callable to invoke when the scope guard goes out of scope.
		template <typename U>
		MUU_NODISCARD_CTOR
		explicit scope_success(U&& func) noexcept(std::is_nothrow_constructible_v<base, U&&>)
			: base{ static_cast<U&&>(func) }
		{}

#if MUU_DOXYGEN
		/// \brief	Dismisses the scope guard, cancelling invocation of the wrapped callable.
		void dismiss() noexcept;

		/// \brief	Returns true if the scope guard has been dismissed.
		bool dismissed() const noexcept;
#endif
	};

	/// \cond

	template <typename R, typename... P>
	scope_success(R(P...) noexcept) -> scope_success<R (*)(P...) noexcept>;

	template <typename R, typename... P>
	scope_success(R(P...)) -> scope_success<R (*)(P...)>;

	template <typename T>
	scope_success(T&&) -> scope_success<impl::scope_guard_param_selector<T>>;

	template <typename T>
	scope_success(T&) -> scope_success<impl::scope_guard_param_selector<T&>>;

	/// \endcond
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
