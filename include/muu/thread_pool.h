// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::thread_pool.

#include "string_param.h"
#include "iterators.h"
#include "aligned_alloc.h"
#include "launder.h"
#include "impl/std_utility.h"
#include "impl/std_memcpy.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_CLANG(diagnostic ignored "-Wignored-attributes")
MUU_PRAGMA_MSVC(warning(disable : 26495)) // core guidelines: uninitialized member

/// \cond
namespace muu::impl
{
	inline constexpr size_t thread_pool_alignment = 64;

	/*
		All of the following is a combination of type-erasure, devirtualization and small function optimization,
		tailored to allow for heap-free moving of a task across a pimpl'd ABI boundary.

		thread_pool_task			captures the callable object and a function for invoking/moving/deleting the task
									(the function is responsible for preserving functionality post type-erasure, and
									is specific to the type-erased thread_pool_task's payload)

		thread_pool_task_action		encodes the arguments passed to a specific invocation of the thread_pool_task's
									invoke/move/delete function

	*/

	struct thread_pool_task;

	struct thread_pool_task_action
	{
		thread_pool_task& task;

		enum class types : uint8_t
		{
			move,
			invoke,
			destroy
		} type;

		union data_type
		{
			size_t thread_index;
			thread_pool_task* source;
		} data;

		MUU_NODISCARD_CTOR
		thread_pool_task_action(thread_pool_task& task_, thread_pool_task&& source_) noexcept //
			: task{ task_ },
			  type{ types::move }
		{
			data.source = muu::assume_aligned<thread_pool_alignment>(&source_);
		}

		MUU_NODISCARD_CTOR
		thread_pool_task_action(thread_pool_task& task_, size_t thread_) noexcept //
			: task{ task_ },
			  type{ types::invoke },
			  data{ thread_ }
		{}

		MUU_NODISCARD_CTOR
		thread_pool_task_action(thread_pool_task& task_) noexcept //
			: task{ task_ },
			  type{ types::destroy }
		{}
	};

	struct alignas(thread_pool_alignment) thread_pool_task
	{
		using action_invoker_type = void(MUU_CALLCONV*)(thread_pool_task_action&&) noexcept;

		alignas(thread_pool_alignment) //
			std::byte buffer[thread_pool_alignment - sizeof(action_invoker_type)];

		static constexpr size_t buffer_capacity = sizeof(buffer);

		action_invoker_type action_invoker;

		template <typename T>
		MUU_NODISCARD_CTOR
		thread_pool_task(T&& callable_) noexcept;

		MUU_NODISCARD_CTOR
		thread_pool_task(thread_pool_task&& task) noexcept
		{
			task.action_invoker(thread_pool_task_action{ *this, static_cast<thread_pool_task&&>(task) });
		}

		~thread_pool_task() noexcept
		{
			action_invoker(thread_pool_task_action{ *this });
		}

		MUU_ALWAYS_INLINE
		void operator()(size_t worker_index) noexcept
		{
			action_invoker(thread_pool_task_action{ *this, worker_index });
		}
	};

	static_assert(sizeof(thread_pool_task) == thread_pool_alignment);
	static_assert(alignof(thread_pool_task) == thread_pool_alignment);
	static_assert(MUU_OFFSETOF(thread_pool_task, buffer) == 0);
	static_assert(std::is_standard_layout_v<thread_pool_task>);

	template <typename T>
	struct thread_pool_task_traits_pointer_to_callable
	{
		using storage_type	= remove_cvref<T>;
		using callable_type = std::remove_pointer_t<storage_type>;

		static_assert(std::is_pointer_v<storage_type>);
		static_assert(std::is_function_v<callable_type> //
					  || std::is_class_v<callable_type> //
					  || std::is_union_v<callable_type>);

		static constexpr bool requires_destruction = false;
		static constexpr bool is_function_pointer  = std::is_function_v<callable_type>;
		static constexpr bool is_referential	   = !is_function_pointer;

		template <typename U>
		MUU_PURE_INLINE_GETTER
		MUU_ATTR(returns_nonnull)
		static storage_type select(U&& callable) noexcept
		{
			if constexpr (std::is_function_v<std::remove_reference_t<U>>)
				return callable;
			else if constexpr (decays_to_function_pointer_by_unary_plus<U&&>)
			{
				using decay_type = std::remove_reference_t<decltype(+std::declval<U&&>())>;
				if constexpr (is_convertible<decay_type, storage_type>)
					return static_cast<storage_type>(+static_cast<U&&>(callable));
				else
					return &callable;
			}
			else
				return &callable;
		}

		template <typename U>
		MUU_ATTR(nonnull)
		static void store(void* buffer, U&& callable) noexcept
		{
			MUU_ASSUME(buffer);

			storage_type ptr = select(static_cast<U&&>(callable));
			MUU_MEMCPY(buffer, &ptr, sizeof(storage_type));
		}

		MUU_ATTR(nonnull)
		static void move(void* from, void* to) noexcept
		{
			MUU_ASSUME(from);
			MUU_ASSUME(to);

			MUU_MEMCPY(to, from, sizeof(storage_type));
		}

		template <typename... Args>
		static void invoke(storage_type callable,
						   [[maybe_unused]] size_t index,
						   [[maybe_unused]] Args&&... args) noexcept
		{
			MUU_ASSUME(callable);

			if constexpr (std::is_nothrow_invocable_v<callable_type&, Args&&..., size_t>)
				(*callable)(static_cast<Args&&>(args)..., index);
			else if constexpr (std::is_nothrow_invocable_v<callable_type&, Args&&...>)
			{
				MUU_UNUSED(index);
				(*callable)(static_cast<Args&&>(args)...);
			}
			else if constexpr (std::is_nothrow_invocable_v<callable_type&>)
			{
				MUU_UNUSED(index);
				(MUU_UNUSED(args), ...);
				(*callable)();
			}
			else
				static_assert(always_false<T>, "Evaluated unreachable branch!");
		}

		template <typename... Args>
		static void invoke(void* buffer, size_t index, Args&&... args) noexcept
		{
			MUU_ASSUME(buffer);

			storage_type callable;
			MUU_MEMCPY(&callable, buffer, sizeof(storage_type));
			invoke(callable, index, static_cast<Args&&>(args)...);
		}
	};

	template <typename T, size_t StoragePenalty = 0, typename BareT = remove_cvref<T>>
	struct thread_pool_task_store_ok
		: std::bool_constant<

			  // not a function pointer or some other weird junk
			  (std::is_class_v<BareT> || std::is_union_v<BareT>)

			  // small enough to entirely fit in a task buffer
			  &&((sizeof(BareT) + StoragePenalty) <= thread_pool_task::buffer_capacity)

			  // alignment compatible with task buffers
			  && (alignof(BareT) <= thread_pool_alignment)

			  // nothrow-destructible
			  && std::is_nothrow_destructible_v<BareT>

			  // can actually be moved or copied into storage
			  && (std::is_trivially_copyable_v<BareT>
				  || (

					  // initial move into storage
					  (std::is_nothrow_constructible_v<BareT, T>
					   || (std::is_nothrow_default_constructible_v<BareT> && std::is_nothrow_assignable_v<BareT&, T>))

					  // move or copy from queue -> local scope during invocation
					  && (std::is_nothrow_move_constructible_v<BareT> || std::is_nothrow_copy_constructible_v<BareT>
						  || (std::is_nothrow_default_constructible_v<BareT>
							  && (std::is_nothrow_move_assignable_v<BareT>
								  || std::is_nothrow_copy_assignable_v<BareT>)))))>
	{};

	template <typename T>
	struct thread_pool_task_traits_stored_callable
	{
		static_assert(!is_cvref<T>);
		static_assert(std::is_class_v<T> || std::is_union_v<T>);
		static_assert(sizeof(T) <= thread_pool_task::buffer_capacity);
		static_assert(alignof(T) <= thread_pool_alignment);
		static_assert(thread_pool_task_store_ok<T>::value);

		using storage_type	= T;
		using callable_type = T;

		static constexpr bool requires_destruction = !std::is_trivially_destructible_v<callable_type>;
		static constexpr bool is_function_pointer  = false;
		static constexpr bool is_referential	   = false;

		template <typename U>
		MUU_CONST_INLINE_GETTER
		static U&& select(U&& callable) noexcept
		{
			return static_cast<U&&>(callable);
		}

		template <typename U>
		MUU_ATTR(nonnull)
		static void store(void* buffer, U&& callable) noexcept
		{
			static_assert(std::is_same_v<callable_type, remove_cvref<U>>);
			MUU_ASSUME(buffer);

			if constexpr (std::is_trivially_copyable_v<callable_type>)
				MUU_MEMCPY(buffer, &callable, sizeof(callable_type));
			else if constexpr (std::is_nothrow_constructible_v<callable_type, U&&>)
			{
				if constexpr (std::is_aggregate_v<callable_type>)
					::new (buffer) callable_type{ static_cast<U&&>(callable) };
				else
					::new (buffer) callable_type(static_cast<U&&>(callable));
			}
			else if constexpr (std::is_nothrow_default_constructible_v<callable_type> //
							   && std::is_nothrow_assignable_v<callable_type&, U&&>)
			{
				auto val = ::new (buffer) callable_type;
				*val	 = static_cast<U&&>(callable);
			}
		}

		MUU_ATTR(nonnull)
		static void move(void* from, void* to) noexcept
		{
			MUU_ASSUME(from);
			MUU_ASSUME(to);

			if constexpr (std::is_trivially_copyable_v<callable_type>)
				MUU_MEMCPY(to, from, sizeof(callable_type));
			else if constexpr (std::is_nothrow_move_constructible_v<callable_type>)
			{
				::new (to) callable_type{ MUU_MOVE(*muu::launder(static_cast<callable_type*>(from))) };
			}
			else if constexpr (std::is_nothrow_default_constructible_v<callable_type> //
							   && std::is_nothrow_move_assignable_v<callable_type>)
			{
				auto val = ::new (to) callable_type;
				*val	 = MUU_MOVE(*muu::launder(static_cast<callable_type*>(from)));
			}
			else if constexpr (std::is_nothrow_copy_constructible_v<callable_type>)
			{
				::new (to) callable_type{ *MUU_LAUNDER(static_cast<callable_type*>(from)) };
			}
			else if constexpr (std::is_nothrow_default_constructible_v<callable_type> //
							   && std::is_nothrow_copy_assignable_v<callable_type>)
			{
				auto val = ::new (to) callable_type;
				*val	 = *MUU_LAUNDER(static_cast<callable_type*>(from));
			}
			else
				static_assert(always_false<T>, "Evaluated unreachable branch!");
		}

		template <typename... Args>
		static void invoke(storage_type& callable,
						   [[maybe_unused]] size_t index,
						   [[maybe_unused]] Args&&... args) noexcept
		{
			if constexpr (std::is_nothrow_invocable_v<callable_type&, Args&&..., size_t>)
				callable(static_cast<Args&&>(args)..., index);
			else if constexpr (std::is_nothrow_invocable_v<callable_type&, Args&&...>)
			{
				MUU_UNUSED(index);
				callable(static_cast<Args&&>(args)...);
			}
			else if constexpr (std::is_nothrow_invocable_v<callable_type&>)
			{
				MUU_UNUSED(index);
				(MUU_UNUSED(args), ...);
				callable();
			}
			else
				static_assert(always_false<T>, "Evaluated unreachable branch!");
		}

		template <typename... Args>
		static void invoke(void* buffer, size_t index, Args&&... args) noexcept
		{
			MUU_ASSUME(buffer);

			storage_type* callable;
			if constexpr (std::is_trivially_copyable_v<callable_type>)
				callable = static_cast<storage_type*>(buffer);
			else
				callable = MUU_LAUNDER(static_cast<storage_type*>(buffer));
			MUU_ASSUME(callable != nullptr);

			invoke(*callable, index, static_cast<Args&&>(args)...);
		}

		MUU_HIDDEN_CONSTRAINT(sfinae, bool sfinae = requires_destruction)
		MUU_ATTR(nonnull)
		static void destroy(void* buffer) noexcept
		{
			MUU_ASSUME(buffer);

			MUU_LAUNDER(static_cast<callable_type*>(buffer))->~callable_type();
		}
	};

	template <typename T, size_t StoragePenalty>
	MUU_CONST_GETTER
	MUU_CONSTEVAL
	auto thread_pool_task_traits_selector() noexcept
	{
		using bare_type = remove_cvref<T>;

		// function references
		if constexpr (std::is_function_v<bare_type>)
		{
			return thread_pool_task_traits_pointer_to_callable<std::add_pointer_t<bare_type>>{};
		}

		// function pointers
		else if constexpr (std::is_function_v<std::remove_pointer_t<bare_type>>)
		{
			return thread_pool_task_traits_pointer_to_callable<bare_type>{};
		}

		// 'storables'
		else if constexpr (thread_pool_task_store_ok<T, StoragePenalty>::value)
		{
			return thread_pool_task_traits_stored_callable<bare_type>{};
		}

		// stateless lambdas (can decay to function pointers)
		else if constexpr (is_stateless_lambda<T>)
		{
			return thread_pool_task_traits_pointer_to_callable<std::remove_reference_t<decltype(+std::declval<T>())>>{};
		}

		// callable objects
		else if constexpr (std::is_class_v<bare_type> || std::is_union_v<bare_type>)
		{
			if constexpr (std::is_rvalue_reference_v<T>)
			{
				// getting here means an it's an rvalue object that did not satisfy thread_pool_task_store_ok,
				// so it would need to be taken by pointer. since it's an rvalue we have to assume it might go out of
				// the enqueuing scope before pool gets to it, which is potentially disastrous.
				// better to just prohibit this entirely and let the programmer know _why_ we're doing so.

				static_assert((sizeof(bare_type) + StoragePenalty) <= thread_pool_task::buffer_capacity,
							  "Rvalue-referenced tasks must be small enough to be "
							  "able to store them internally.");

				static_assert(alignof(bare_type) <= thread_pool_alignment,
							  "Rvalue-referenced tasks must have small enough alignment to be "
							  "able to store them internally.");

				static_assert(std::is_nothrow_destructible_v<bare_type>,
							  "Rvalue-referenced tasks must be nothrow-destructible to be safely stored internally.");

				static_assert(

					std::is_trivially_copyable_v<bare_type> ||

						// initial move into storage
						((std::is_nothrow_constructible_v<bare_type, T>			 //
						  || (std::is_nothrow_default_constructible_v<bare_type> //
							  && std::is_nothrow_assignable_v<bare_type&, T>))

						 // move or copy from queue -> local scope during invocation
						 && (std::is_nothrow_move_constructible_v<bare_type>			  //
							 || std::is_nothrow_copy_constructible_v<bare_type>			  //
							 || (std::is_nothrow_default_constructible_v<bare_type>		  //
								 && (std::is_nothrow_move_assignable_v<bare_type>		  //
									 || std::is_nothrow_copy_assignable_v<bare_type>)))), //

					"Rvalue-referenced tasks must be trivially-copyable, nothrow-movable or nothrow-copyable to be "
					"able to store them internally.");

				// shouldn't get here; this last statement is just for the sake of having a return value.

				return std::false_type{};
			}
			else
			{
				return thread_pool_task_traits_pointer_to_callable<std::add_pointer_t<std::remove_reference_t<T>>>{};
			}
			return thread_pool_task_traits_pointer_to_callable<std::add_pointer_t<std::remove_reference_t<T>>>{};
		}

		// ???
		else
			static_assert(always_false<T>, "Evaluated unreachable branch!");
	}

	template <typename T, size_t StoragePenalty = 0>
	struct thread_pool_task_traits : decltype(thread_pool_task_traits_selector<T, StoragePenalty>())
	{};

	template <typename T>
	inline thread_pool_task::thread_pool_task(T&& callable) noexcept
	{
		using traits		= thread_pool_task_traits<T&&>;
		using callable_type = typename traits::callable_type;

		static_assert(std::is_nothrow_invocable_v<callable_type&, size_t> //
					  || std::is_nothrow_invocable_v<callable_type&>);

		// store callable
		traits::store(buffer, static_cast<T&&>(callable));

		// create invoker/mover/deleter
		action_invoker = [](thread_pool_task_action&& action) noexcept
		{
			switch (action.type)
			{
				case thread_pool_task_action::types::move:
				{
					action.task.action_invoker = action.data.source->action_invoker;
					traits::move(action.data.source->buffer, action.task.buffer);
					break;
				}

				case thread_pool_task_action::types::invoke:
				{
					traits::invoke(action.task.buffer, action.data.thread_index);
					break;
				}

				case thread_pool_task_action::types::destroy:
				{
					if constexpr (traits::requires_destruction)
					{
						traits::destroy(action.task.buffer);
						return;
					}
					break;
				}

				default: MUU_UNREACHABLE;
			}
		};
		MUU_ASSERT(action_invoker);
	}

	template <typename T>
	class batch_size_generator
	{
	  private:
		T remaining;
		T constant;
		T overflow;

	  public:
		batch_size_generator(T job_count, T batch_count) noexcept //
			: remaining{ job_count },
			  constant{ job_count / batch_count },
			  overflow{ job_count % batch_count }
		{}

		MUU_NODISCARD
		MUU_ALWAYS_INLINE
		T operator()() noexcept
		{
			if (!remaining)
				return {};
			T i = constant;
			if (overflow)
			{
				i++;
				overflow--;
			}
			remaining -= i;
			return i;
		}
	};
}

extern "C" //
{
	MUU_NODISCARD
	MUU_API
	MUU_ATTR(returns_nonnull)
	void* MUU_CALLCONV muu_impl_thread_pool_create(size_t, size_t, muu::string_param*, void*);

	MUU_API
	MUU_ATTR(nonnull)
	void MUU_CALLCONV muu_impl_thread_pool_destroy(void*) noexcept;

	MUU_NODISCARD
	MUU_API
	MUU_ATTR(nonnull)
	size_t MUU_CALLCONV muu_impl_thread_pool_lock_multiple(void*, size_t) noexcept;

	MUU_NODISCARD
	MUU_API
	MUU_ATTR(nonnull)
	size_t MUU_CALLCONV muu_impl_thread_pool_lock(void*) noexcept;

	MUU_NODISCARD
	MUU_API
	MUU_ATTR(nonnull)
	MUU_ATTR(returns_nonnull)
	MUU_ATTR(assume_aligned(muu::impl::thread_pool_alignment))
	void* MUU_CALLCONV muu_impl_thread_pool_acquire(void*, size_t) noexcept;

	MUU_API
	MUU_ATTR(nonnull)
	void MUU_CALLCONV muu_impl_thread_pool_unlock(void*, size_t) noexcept;

	MUU_API
	MUU_ATTR(nonnull)
	void MUU_CALLCONV muu_impl_thread_pool_wait(void*) noexcept;

	MUU_PURE_GETTER
	MUU_API
	size_t MUU_CALLCONV muu_impl_thread_pool_workers(void*) noexcept;

	MUU_PURE_GETTER
	MUU_API
	size_t MUU_CALLCONV muu_impl_thread_pool_capacity(void*) noexcept;
}
/// \endcond

namespace muu
{
	/// \brief A thread pool.
	class thread_pool
	{
	  private:
		void* storage_ = nullptr;

		template <typename T>
		MUU_ALWAYS_INLINE
		void enqueue(size_t queue_index, T&& task) noexcept
		{
			::new (
				muu::assume_aligned<impl::thread_pool_alignment>(::muu_impl_thread_pool_acquire(storage_, queue_index)))
				impl::thread_pool_task{ static_cast<T&&>(task) };
		}

		template <typename Task>
		class batched_task
		{
		  public:
			using traits		= impl::thread_pool_task_traits<Task, sizeof(void*) * 3>;
			using storage_type	= typename traits::storage_type;
			using callable_type = typename traits::callable_type;

		  private:
			storage_type task_;

		  public:
			template <typename Arg>
			void operator()(Arg&& arg, size_t batch_index) noexcept
			{
				traits::invoke(task_, batch_index, static_cast<Arg&&>(arg));
			}

			template <typename Arg>
			void operator()(Arg&& arg) noexcept
			{
				traits::invoke(task_, {}, static_cast<Arg&&>(arg));
			}

			template <typename U>
			MUU_NODISCARD_CTOR
			batched_task(U&& task) noexcept //
				: task_{ traits::select(static_cast<U&&>(task)) }
			{}
		};

		template <typename OriginalTask, typename Task>
		MUU_NODISCARD
		MUU_ALWAYS_INLINE
		static auto wrap_batched_task(Task&& task) noexcept
		{
			static_assert(std::is_reference_v<OriginalTask>);
			static_assert(std::is_same_v<remove_cvref<OriginalTask>, remove_cvref<Task>>);

			if constexpr (std::is_lvalue_reference_v<OriginalTask>)
			{
				static_assert(std::is_same_v<OriginalTask, Task&&>);
				return batched_task<OriginalTask>{ task };
			}
			else
			{
				if constexpr (std::is_rvalue_reference_v<Task&&>)
					return batched_task<Task&&>{ static_cast<Task&&>(task) };
				else
					return batched_task<remove_cvref<Task>&&>{ remove_cvref<Task>{ task } };
			}
		}

	  public:
		/// \brief	Constructs a thread pool.
		///
		/// \param	worker_count		The number of worker threads in the pool. Leave as `0` for 'automatic'.
		/// \param	task_queue_size		Max tasks that can be stored in the internal queue without blocking. Leave as `0` for 'automatic'.
		/// \param	name 				The name of your threadpool (for debugging purposes).
		MUU_NODISCARD_CTOR
		explicit thread_pool(size_t worker_count = 0, size_t task_queue_size = 0, string_param name = {}) //
			: storage_{ ::muu_impl_thread_pool_create(worker_count, task_queue_size, &name, nullptr) }
		{}

		/// \brief	Constructs a thread pool.
		///
		/// \param	name 		The name of your thread pool (for debugging purposes).
		MUU_NODISCARD_CTOR
		explicit thread_pool(string_param name) //
			: thread_pool{ 0, 0, static_cast<string_param&&>(name) }
		{}

		/// \brief	Move constructor.
		MUU_NODISCARD_CTOR
		thread_pool(thread_pool&& other) noexcept //
			: storage_{ std::exchange(other.storage_, nullptr) }
		{}

		/// \brief	Move-assignment operator.
		thread_pool& operator=(thread_pool&& rhs) noexcept
		{
			storage_ = std::exchange(rhs.storage_, nullptr);
			return *this;
		}

		/// \brief	Destructor.
		/// \warning Any enqueued tasks *currently being executed* are allowed to finish;
		/// 		 attempting to destroy a thread_pool with an enqueued task of indeterminate
		/// 		 length may lead to a deadlock.
		~thread_pool() noexcept
		{
			if (auto storage = std::exchange(storage_, nullptr))
				::muu_impl_thread_pool_destroy(storage);
		}

		MUU_DELETE_COPY(thread_pool);

		/// \brief	The number of worker threads in the thread pool.
		MUU_PURE_INLINE_GETTER
		size_t workers() const noexcept
		{
			return ::muu_impl_thread_pool_workers(storage_);
		}

		/// \brief	The maximum tasks that may be enqueued without blocking.
		MUU_PURE_INLINE_GETTER
		size_t capacity() const noexcept
		{
			return ::muu_impl_thread_pool_capacity(storage_);
		}

		/// \brief	Waits for the thread pool to finish all of its current work.
		///
		/// \warning Do not call this from one of the thread pool's workers.
		MUU_ALWAYS_INLINE
		void wait() noexcept
		{
			::muu_impl_thread_pool_wait(storage_);
		}

		/// \brief	Enqueues a task.
		///
		/// \details Tasks must be callables with no parameters, or one parameter to recieve
		/// 		 the index of the worker invoking the task:
		/// \cpp
		/// pool.enqueue([](size_t worker_index) noexcept
		/// {
		///		// worker_index is in the range [0, pool.workers() - 1]
		///	});
		/// pool.enqueue([]() noexcept
		/// {
		///		//...
		///	});
		/// \ecpp
		///
		/// \remarks Tasks must be finite, otherwise the pool will fill and wait() calls will never return.
		/// \remarks Tasks must not throw exceptions.
		///
		/// \warning Do not call this from one of the thread_pool's workers.
		///
		/// \tparam	Task	The type of the task being enqueued.
		/// \param	task  	The task to enqueue.
		///
		/// \returns	A reference to the thread pool.
		template <typename Task>
		thread_pool& enqueue(Task&& task) noexcept
		{
			static_assert(
				std::is_nothrow_invocable_v<Task&, size_t> //
					|| std::is_nothrow_invocable_v<Task&>,
				"Tasks passed to thread_pool::enqueue() must be callable as void() noexcept or void(size_t) noexcept");

			const auto qindex = ::muu_impl_thread_pool_lock(storage_);
			enqueue(qindex, static_cast<Task&&>(task));
			::muu_impl_thread_pool_unlock(storage_, qindex);
			return *this;
		}

	  private:
		static constexpr size_t no_available_queue = static_cast<size_t>(-1);

		template <typename OriginalTask, typename ValueType, size_t Arity, typename T, typename Task>
		void enqueue_for_each_batch(size_t shared_queue_index,
									T batch_start,
									T batch_end,
									[[maybe_unused]] size_t batch_index,
									Task&& task) noexcept
		{
			MUU_ASSERT(batch_start < batch_end);

			static_assert(Arity <= 2);
			MUU_ASSERT(batch_index < workers());

			const auto queue_index = shared_queue_index == no_available_queue //
									   ? ::muu_impl_thread_pool_lock(storage_)
									   : shared_queue_index;

			enqueue(queue_index,
					[=, batch = wrap_batched_task<OriginalTask>(static_cast<Task&&>(task))]() mutable noexcept
					{
						for (; batch_start < batch_end; batch_start++)
						{
							if constexpr (Arity == 2)
								batch(static_cast<ValueType>(batch_start), batch_index);
							else
								batch(static_cast<ValueType>(batch_start));
						}
					});

			if (shared_queue_index == no_available_queue)
				::muu_impl_thread_pool_unlock(storage_, queue_index);
		}

	  public:
		/// \brief	Enqueues a task to execute once for every value in a range.
		///
		/// \details	Tasks must be callables which accept at most two arguments: <br>
		/// 			Argument 0: The current value from the range <br>
		/// 			Argument 1: The task's batch (in the range `[0, pool.workers() - 1]`) <br>
		/// \cpp
		/// pool.for_each(0, 10, [](int i, size_t batch_index) noexcept
		/// {
		///		// i is in the range [0, 9]
		///		// batch_index is in the range [0, pool.workers() - 1]
		///	});
		/// pool.for_each(0, 10, [](int i) noexcept
		/// {
		///		// i is in the range [0, 9]
		///	});
		/// pool.for_each(0, 10, []() noexcept
		/// {
		///		// no args is OK too
		///	});
		/// \ecpp
		///
		/// \remarks Tasks must be finite, otherwise the pool will fill and Wait() calls will never return.
		/// \remarks Tasks must not throw exceptions.
		///
		/// \warning Do not call this from one of the thread_pool's workers.
		///
		/// \tparam	T 			An integer or enum type.
		/// \tparam	Task		The type of the task being enqueued.
		/// \param	start		The start of the value range (inclusive).
		/// \param	end			The end of the value range (exclusive).
		/// \param	task		The task to enqueue.
		///
		/// \return	A reference to the threadpool.
		MUU_CONSTRAINED_TEMPLATE(muu::is_integral<T>, typename T, typename Task)
		thread_pool& for_each(T start, T end, Task&& task) noexcept
		{
			static_assert(std::is_nothrow_invocable_v<Task&, T, size_t> //
							  || std::is_nothrow_invocable_v<Task&, T>	//
							  || std::is_nothrow_invocable_v<Task&>,
						  "Tasks passed to thread_pool::for_each() must be callable as void() noexcept, void(T) "
						  "noexcept or void(T, size_t) noexcept");

			if (start == end)
				return *this;

			using value_type  = remove_cvref<T>;
			using offset_type = largest<remove_enum<value_type>,
										std::conditional_t<is_signed<remove_enum<value_type>>, signed, unsigned>>;
			using size_type	  = largest<make_unsigned<offset_type>, size_t>;

			// reverse the start and end if they're backwards (this is a parallel for; order should be irrelevant)
			if (start > end)
			{
				T temp = end;
				end	   = T{ static_cast<remove_enum<T>>(start) + remove_enum<T>{ 1 } };
				start  = T{ static_cast<remove_enum<T>>(temp) + remove_enum<T>{ 1 } };
			}

			// determine batch count and distribute indices
			const auto job_count =
				static_cast<size_type>(static_cast<offset_type>(end) - static_cast<offset_type>(start));
			MUU_ASSERT(job_count);
			const auto worker_count		 = this->workers();
			auto batch_generator		 = impl::batch_size_generator<size_type>{ job_count, worker_count };
			offset_type next_batch_start = unwrap(start);
			size_type next_batch_size	 = batch_generator();
			size_t batch_index			 = 0u;
			auto batch_count			 = muu::min(job_count, worker_count);

			// try to get a shared queue for all the allocations
			const auto shared_queue_index = ::muu_impl_thread_pool_lock_multiple(storage_, batch_count);

			// dispatch tasks
			static constexpr size_t task_arity =
				(std::is_nothrow_invocable_v<Task&, T, size_t> ? 2 : (std::is_nothrow_invocable_v<Task&, T> ? 1 : 0));
			while (true)
			{
				const auto batch_start = next_batch_start;
				next_batch_start	   = static_cast<offset_type>(batch_start + next_batch_size);
				next_batch_size		   = batch_generator();

				if (next_batch_size)
					enqueue_for_each_batch<Task&&, value_type, task_arity>(shared_queue_index,
																		   batch_start,
																		   next_batch_start,
																		   batch_index,
																		   task);
				else
				{
					enqueue_for_each_batch<Task&&, value_type, task_arity>(shared_queue_index,
																		   batch_start,
																		   next_batch_start,
																		   batch_index,
																		   static_cast<Task&&>(task));
					break;
				}
				batch_index++;
			}

			// unlock shared queue
			if (shared_queue_index != no_available_queue)
				::muu_impl_thread_pool_unlock(storage_, shared_queue_index);

			return *this;
		}

		/// \brief	Enqueues a task to execute once for every value in a range.
		///
		/// \details	Tasks must be callables which accept at most two arguments: <br>
		/// 			Argument 0: The current value from the range <br>
		/// 			Argument 1: The task's batch (in the range `[0, pool.workers() - 1]`) <br>
		/// \cpp
		/// pool.for_each(10, [](int i, size_t batch_index) noexcept
		/// {
		///		// i is in the range [0, 9]
		///		// batch_index is in the range [0, pool.workers() - 1]
		///	});
		/// pool.for_each(10, [](int i) noexcept
		/// {
		///		// i is in the range [0, 9]
		///	});
		/// pool.for_each(10, []() noexcept
		/// {
		///		// no args is OK too
		///	});
		/// \ecpp
		///
		/// \remarks Tasks must be finite, otherwise the pool will fill and Wait() calls will never return.
		/// \remarks Tasks must not throw exceptions.
		///
		/// \warning Do not call this from one of the thread_pool's workers.
		///
		/// \tparam	T 			An integer or enum type.
		/// \tparam	Task		The type of the task being enqueued.
		/// \param	end			The end of the value range (exclusive).
		/// \param	task		The task to enqueue.
		///
		/// \return	A reference to the threadpool.
		MUU_CONSTRAINED_TEMPLATE(muu::is_integral<T>, typename T, typename Task)
		MUU_ALWAYS_INLINE
		thread_pool& for_each(T end, Task&& task) noexcept
		{
			return for_each(T{}, end, static_cast<Task&&>(task));
		}

		/// \cond
		template <typename T, typename Task>
		MUU_ALWAYS_INLINE
		thread_pool& for_range(T start, T end, Task&& task) noexcept
		{
			return for_each(start, end, static_cast<Task&&>(task));
		}
		/// \endcond

	  private:
		template <typename OriginalTask, size_t Arity, typename Iter, typename Task>
		void enqueue_for_each_with_iterators_batch(size_t shared_queue_index,
												   Iter batch_start,
												   Iter batch_end,
												   [[maybe_unused]] size_t batch_index,
												   Task&& task) noexcept
		{
			static_assert(Arity <= 2);
			MUU_ASSERT(batch_index < workers());

			const auto queue_index = shared_queue_index == no_available_queue //
									   ? ::muu_impl_thread_pool_lock(storage_)
									   : shared_queue_index;

			enqueue(queue_index,
					[=, batch = wrap_batched_task<OriginalTask>(static_cast<Task&&>(task))]() mutable noexcept
					{
						while (batch_start != batch_end)
						{
							if constexpr (Arity == 2)
								batch(*batch_start, batch_index);
							else
								batch(*batch_start);

							std::advance(batch_start, 1);
						}
					});

			if (shared_queue_index == no_available_queue)
				::muu_impl_thread_pool_unlock(storage_, queue_index);
		}

		template <typename Begin, typename Task>
		thread_pool& for_each_with_iterators(Begin begin, size_t job_count, Task&& task) noexcept
		{
			using elem_reference = impl::iter_reference_t<Begin>;
			static_assert(
				std::is_nothrow_invocable_v<Task&, elem_reference, size_t> //
					|| std::is_nothrow_invocable_v<Task&, elem_reference>  //
					|| std::is_nothrow_invocable_v<Task&>,
				"Tasks passed to thread_pool::for_each() must be callable as void() noexcept, void(T) noexcept or "
				"void(T, size_t) noexcept");

			// determine batch count and distribute iterators
			const auto worker_count = this->workers();
			auto batch_generator	= impl::batch_size_generator<size_t>{ job_count, worker_count };
			size_t next_batch_size	= batch_generator();
			auto batch_start		= begin;
			auto batch_end			= std::next(begin, static_cast<ptrdiff_t>(next_batch_size));
			size_t batch_index		= 0u;
			auto batch_count		= muu::min(job_count, worker_count);

			// try to get a shared queue for all the allocations
			const auto shared_queue_index = ::muu_impl_thread_pool_lock_multiple(storage_, batch_count);

			// dispatch tasks
			static constexpr size_t task_arity =
				(std::is_nothrow_invocable_v<Task&, elem_reference, size_t>
					 ? 2
					 : (std::is_nothrow_invocable_v<Task&, elem_reference, size_t> ? 1 : 0));
			while (true)
			{
				next_batch_size = batch_generator();
				if (next_batch_size)
				{
					enqueue_for_each_with_iterators_batch<Task&&, task_arity>(shared_queue_index,
																			  batch_start,
																			  batch_end,
																			  batch_index,
																			  task);
					batch_start = batch_end;
					std::advance(batch_end, static_cast<ptrdiff_t>(next_batch_size));
				}
				else
				{
					enqueue_for_each_with_iterators_batch<Task&&, task_arity>(shared_queue_index,
																			  batch_start,
																			  batch_end,
																			  batch_index,
																			  static_cast<Task&&>(task));
					break;
				}
				batch_index++;
			}

			// unlock shared queue
			if (shared_queue_index != no_available_queue)
				::muu_impl_thread_pool_unlock(storage_, shared_queue_index);

			return *this;
		}

	  public:
		/// \brief	Enqueues a task to execute on every element in a collection.
		///
		/// \details	Tasks must be callables which accept at most two arguments: <br>
		/// 			Argument 0: The current element from the collection <br>
		/// 			Argument 1: The task's batch (in the range `[0, pool.workers() - 1]`) <br>
		/// \cpp
		/// std::array<int, 10> vals;
		/// pool.for_each(vals.begin(), vals.end(), [](int& i, size_t batch_index) noexcept
		/// {
		///		// i is one of the elements of vals
		///		// batch_index is in the range [0, pool.workers() - 1]
		///	});
		/// pool.for_each(vals.begin(), vals.end(), [](int& i) noexcept
		/// {
		///		// i is one of the elements of vals
		///	});
		/// pool.for_each(vals.begin(), vals.end(), []() noexcept
		/// {
		///		// no args is OK too (though unusual)
		///	});
		/// \ecpp
		///
		/// \remarks Tasks must be finite, otherwise the pool will fill and wait() calls will never return.
		/// \remarks Tasks must not throw exceptions.
		///
		/// \warning Do not call this from one of the thread_pool's workers.
		///
		/// \tparam	Iter 		Collection iterator type.
		/// \tparam	Task		The type of the task being enqueued.
		/// \param	begin		Iterator to the beginning of the collection.
		/// \param	end			Iterator to the end of the collection.
		/// \param	task  		The task to execute for each member of the collection.
		///
		/// \return	A reference to the thread pool.
		MUU_CONSTRAINED_TEMPLATE(!muu::is_integral<Iter>, typename Iter, typename Task)
		thread_pool& for_each(Iter begin, Iter end, Task&& task) noexcept
		{
			if constexpr (has_less_than_or_equal_operator<Iter>)
			{
				if (end <= begin)
					return *this;
			}
			else
			{
				if constexpr (has_less_than_operator<Iter>)
				{
					if (end < begin)
						return *this;
				}
				if (begin == end)
					return *this;
			}

			const auto job_count = iterator_distance(begin, end);
			if (job_count <= 0)
				return *this;

			return for_each_with_iterators(begin, static_cast<size_t>(job_count), static_cast<Task&&>(task));
		}

		/// \brief	Enqueues a task to execute on every element in a collection.
		///
		/// \details	Tasks must be callables which accept at most two arguments: <br>
		/// 			Argument 0: The current element from the collection <br>
		/// 			Argument 1: The task's batch (in the range `[0, pool.workers() - 1]`) <br>
		/// \cpp
		/// std::array<int, 10> vals;
		/// pool.for_each(vals, [](int& i, size_t batch_index) noexcept
		/// {
		///		// i is one of the elements of vals
		///		// batch_index is in the range [0, pool.workers() - 1]
		///	});
		/// pool.for_each(vals, [](int& i) noexcept
		/// {
		///		// i is one of the elements of vals
		///	});
		/// pool.for_each(vals, []() noexcept
		/// {
		///		// no args is OK too (though unusual)
		///	});
		/// \ecpp
		///
		/// \remarks Tasks must be finite, otherwise the pool will fill and wait() calls will never return.
		/// \remarks Tasks must not throw exceptions.
		///
		/// \warning Do not call this from one of the thread_pool's workers.
		///
		/// \tparam	T 			Type of the collection.
		/// \tparam	Task		The type of the task being enqueued.
		/// \param	collection	The collection.
		/// \param	task  		The task to execute for each member of the collection.
		///
		/// \return	A reference to the thread pool.
		MUU_CONSTRAINED_TEMPLATE((!muu::is_integral<T> && muu::is_iterable<T&&>), typename T, typename Task)
		MUU_ALWAYS_INLINE
		thread_pool& for_each(T&& collection, Task&& task) noexcept
		{
			return for_each(begin_iterator(static_cast<T&&>(collection)),
							end_iterator(static_cast<T&&>(collection)),
							static_cast<Task&&>(task));
		}
	};
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
