// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::thread_pool.

#pragma once
#include "../muu/core.h"
#include "../muu/tagged_ptr.h"
MUU_DISABLE_WARNINGS
#ifdef __cpp_lib_hardware_interference_size
	#include <new>
#endif
#include <string_view>
MUU_ENABLE_WARNINGS

#define MUU_MOVE_CHECK	MUU_ASSERT(pimpl_ != nullptr && "The thread_pool has been moved from!")
#if MUU_ARCH_AMD64
	#define	MUU_COMPRESSED_THREAD_POOL_TASK 1
#else
	#define	MUU_COMPRESSED_THREAD_POOL_TASK 0
#endif

MUU_PUSH_WARNINGS
MUU_DISABLE_SPAM_WARNINGS
MUU_DISABLE_LIFETIME_WARNINGS

MUU_IMPL_NAMESPACE_START
{
	template <typename Func, typename Arg>
	inline constexpr bool is_thread_pool_task = 
		std::is_nothrow_invocable_v<Func, Arg, size_t>
		|| std::is_nothrow_invocable_v<Func, Arg>;

	template <typename T>
	inline constexpr bool is_stateless_task
		= std::is_function_v<std::remove_pointer_t<std::remove_reference_t<T>>>
		|| std::is_empty_v<muu::remove_cvref<T>>
		|| (std::is_class_v<muu::remove_cvref<T>> && sizeof(T) == 1_sz && muu::has_unary_plus_operator<T>);

	#ifdef __cpp_lib_hardware_interference_size
		inline constexpr size_t thread_pool_task_granularity = bit_ceil((max)(std::hardware_destructive_interference_size, 64_sz));
	#else
		inline constexpr size_t thread_pool_task_granularity = 64;
	#endif

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

	struct thread_pool_task_action final
	{
		thread_pool_task& task;
		enum class types : uint8_t
		{
			move,
			invoke,
			destroy
		}
		type;
		union data_type
		{
			size_t thread_index;
			thread_pool_task* source;
		}
		data;

		MUU_NODISCARD_CTOR
		thread_pool_task_action(thread_pool_task& task_, thread_pool_task&& source_) noexcept
			: task{ task_ },
			type{ types::move }
		{
			data.source = assume_aligned<thread_pool_task_granularity>(&source_);
		}

		MUU_NODISCARD_CTOR
		thread_pool_task_action(thread_pool_task& task_, size_t thread_) noexcept
			: task{ task_ },
			type{ types::invoke },
			data{ thread_ }
		{}

		MUU_NODISCARD_CTOR
		thread_pool_task_action(thread_pool_task& task_) noexcept
			: task{ task_ },
			type{ types::destroy }
		{}
	};

	MUU_PRAGMA_MSVC(pack(push, 1))

	struct
	MUU_TRIVIAL_ABI
	MUU_ATTR(packed)
	MUU_ALIGN(thread_pool_task_granularity)
	thread_pool_task final
	{
		using action_invoker_func = void(thread_pool_task_action&&)noexcept;
		#if MUU_COMPRESSED_THREAD_POOL_TASK
		using action_invoker_type = tagged_ptr<action_invoker_func>;
		using states_int_type = action_invoker_type::tag_type;
		using callable_buffer_type = std::byte[thread_pool_task_granularity - sizeof(action_invoker_type)];
		#else
		using action_invoker_type = action_invoker_func*;
		using states_int_type = uint8_t;
		using callable_buffer_type = std::byte[thread_pool_task_granularity - sizeof(action_invoker_type) - sizeof(states_int_type)];
		#endif
		static constexpr auto heap_free_threshold = sizeof(callable_buffer_type);
		template <typename T>
		static constexpr bool requires_heap = sizeof(T) > heap_free_threshold || alignof(T) > thread_pool_task_granularity;

		enum class states : states_int_type
		{
			none = 0,
			requires_explicit_destruction = 1,
			destroyed = 2
		};

		callable_buffer_type callable_buffer;
		action_invoker_type action_invoker;
		#if !MUU_COMPRESSED_THREAD_POOL_TASK
		states state_ = states::none;
		#endif

		[[nodiscard]]
		MUU_ALWAYS_INLINE
		states state() const noexcept
		{
			#if MUU_COMPRESSED_THREAD_POOL_TASK
			return states{ action_invoker.tag() };
			#else
			return state_;
			#endif
		}

		[[nodiscard]]
		MUU_ALWAYS_INLINE
		bool has_state(states s) const noexcept
		{
			return !!(unwrap(state()) & unwrap(s));
		}

		void set_state(states s) noexcept
		{
			#if MUU_COMPRESSED_THREAD_POOL_TASK
			action_invoker.tag(s);
			#else
			state_ = s;
			#endif
		}

		void add_state(states s) noexcept
		{
			set_state(static_cast<states>( unwrap(state()) | unwrap(s) ));
		}

		template <typename T>
		MUU_NODISCARD_CTOR
		thread_pool_task(T && callable_) noexcept
		{
			using callable_type = std::remove_reference_t<T>;
			static_assert(
				std::is_invocable_v<callable_type, size_t>
				|| std::is_invocable_v<callable_type>
			);

			static constexpr auto requires_explicit_destruction =
				requires_heap<callable_type>
				|| !std::is_trivially_destructible_v<callable_type>
			;

			// move object into storage
			if constexpr (requires_heap<callable_type>)
			{
				static_assert(sizeof(callable_buffer) >= sizeof(callable_type*));
				auto ptr = new callable_type{ std::forward<T>(callable_) };
				memcpy(callable_buffer, &ptr, sizeof(ptr));
			}
			else
			{
				static_assert(sizeof(callable_buffer) >= sizeof(callable_type));
				new (callable_buffer) callable_type{ std::forward<T>(callable_) };
			}

			// create invoker/mover/deleter
			action_invoker = [](thread_pool_task_action&& action) noexcept
			{
				switch (action.type)
				{
					case thread_pool_task_action::types::invoke:
					{
						if constexpr (requires_explicit_destruction)
						{
							MUU_ASSERT(!action.task.has_state(states::destroyed));
						}

						callable_type* callable;
						if constexpr (requires_heap<callable_type>)
							memcpy(&callable, action.task.callable_buffer, sizeof(callable));
						else
							callable = launder(reinterpret_cast<callable_type*>(action.task.callable_buffer));

						if constexpr (std::is_invocable_v<callable_type, size_t>)
							(*callable)(action.data.thread_index);
						else
							(*callable)();
						return;
					}

					case thread_pool_task_action::types::move:
					{
						if constexpr (requires_explicit_destruction)
						{
							MUU_ASSERT(!action.task.has_state(states::destroyed));
						}

						if constexpr (requires_heap<callable_type> || std::is_trivially_copyable_v<callable_type>)
						{
							memcpy(&action.task, action.data.source, sizeof(thread_pool_task));
							if constexpr (requires_heap<callable_type>)
								action.data.source->add_state(states::destroyed);
						}
						else
						{
							action.task.action_invoker = action.data.source->action_invoker;
							#if !MUU_COMPRESSED_THREAD_POOL_TASK
							action.task.set_state(action.data.source->state());
							#endif

							new (action.task.callable_buffer) callable_type{ std::move(
								*launder(reinterpret_cast<callable_type*>(action.data.source->callable_buffer))
							) };
						}
						return;
					}

					case thread_pool_task_action::types::destroy:
					{
						if constexpr (requires_explicit_destruction)
						{
							MUU_ASSERT(action.type == thread_pool_task_action::types::destroy);
							MUU_ASSERT(!action.task.has_state(states::destroyed));

							if constexpr (requires_heap<callable_type>)
							{
								callable_type* callable;
								memcpy(&callable, action.task.callable_buffer, sizeof(callable));
								delete callable;
							}
							else
								launder(reinterpret_cast<callable_type*>(action.task.callable_buffer))->~callable_type();
							return;
						}
						else
							MUU_UNREACHABLE;
					}
					MUU_NO_DEFAULT_CASE;
				}
				MUU_UNREACHABLE;
			};
			
			if constexpr (requires_explicit_destruction)
				set_state(states::requires_explicit_destruction);

		}

		thread_pool_task(thread_pool_task&& task) noexcept
		{
			task.action_invoker(thread_pool_task_action{ *this, std::move(task) });
		}

		~thread_pool_task() noexcept
		{
			if (has_state(states::requires_explicit_destruction) && !has_state(states::destroyed))
			{
				action_invoker(thread_pool_task_action{ *this });
				add_state(states::destroyed);
			}
		}

		void operator()(size_t worker_index) noexcept
		{
			action_invoker(thread_pool_task_action{ *this, worker_index });
		}
	};
	
	MUU_PRAGMA_MSVC(pack(pop))

	static_assert(sizeof(thread_pool_task) <= thread_pool_task_granularity);
	static_assert(alignof(thread_pool_task) == thread_pool_task_granularity);
	static_assert(MUU_OFFSETOF(thread_pool_task, callable_buffer) == 0);
	static_assert(std::is_standard_layout_v<thread_pool_task>);
}
MUU_IMPL_NAMESPACE_END

MUU_NAMESPACE_START
{
	class MUU_API thread_pool
	{
		private:
			struct pimpl;
			pimpl* pimpl_ = nullptr;

			[[nodiscard]]
			size_t lock() noexcept;

			[[nodiscard]]
			MUU_UNALIASED_ALLOC
			MUU_ATTR(returns_nonnull)
			MUU_ATTR(assume_aligned(impl::thread_pool_task_granularity))
			void* acquire(size_t) noexcept;

			template <typename T>
			void enqueue(size_t queue_index, T&& task) noexcept
			{
				new (assume_aligned<impl::thread_pool_task_granularity>(acquire(queue_index))) impl::thread_pool_task{ std::forward<T>(task) };
			}

			void unlock(size_t) noexcept;

		public:

			/// \brief	Constructs a thread_pool.
			///
			/// \param	worker_count		The number of worker threads in the pool. Use `0` for 'automatic'.
			/// \param	worker_queue_size	The max length of each worker's task queue. Use `0` for 'automatic'.
			/// \param	name 	The name of your threadpool (for debugging purposes).
			MUU_NODISCARD_CTOR
			explicit thread_pool(size_t worker_count = 0, size_t worker_queue_size = 0, std::string_view name = {}) noexcept;

			/// \brief	Constructs a thread_pool.
			///
			/// \param	name 	The name of your thread pool (for debugging purposes).
			MUU_NODISCARD_CTOR
			explicit thread_pool(std::string_view name) noexcept
				: thread_pool{ 0, 0, name }
			{}

			/// \brief	Move constructor.
			thread_pool(thread_pool&&) noexcept;

			/// \brief	Move-assignment operator.
			thread_pool& operator= (thread_pool&&) noexcept;

			/// \brief	Destructor.
			/// \warning Any enqueued tasks *currently being executed* are allowed to finish;
			/// 		 attempting to destroy a thread_pool with an enqueued task of indeterminate
			/// 		 length may lead to a deadlock.
			~thread_pool() noexcept;

			MUU_DELETE_COPY(thread_pool);

			/// \brief	The number of worker threads in the thread pool.
			[[nodiscard]]
			MUU_ATTR(pure)
			size_t size() const noexcept;

			/// \brief	Waits for the thread pool to finish all of its current work.
			/// 
			/// \warning Do not call this from one of the thread pool's workers.
			void wait() noexcept;

			/// \brief	Enqueues a task.
			/// 
			/// \details Tasks must be callables with no parameters, or one parameter to recieve
			/// 		 the index of the worker invoking the task:
			/// \cpp
			/// pool.enqueue([]() noexcept
			/// {
			///		//...
			///	});
			/// pool.enqueue([](size_t worker_index) noexcept
			/// {
			///		// worker_index is in the range [0, pool.size())
			///	});
			/// \ecpp
			/// 
			/// \remarks Tasks must be finite, otherwise the pool will fill and wait() calls will never return.
			/// \remarks Tasks must not throw exceptions.
			///
			/// \tparam	T		The type of the task being enqueued.
			/// \param	task  	The task to enqueue.
			///
			/// \returns	A reference to the thread pool.
			template <typename T>
			thread_pool& enqueue(T && task) noexcept
			{
				static_assert(
					std::is_nothrow_invocable_v<T&&, size_t>
					|| std::is_nothrow_invocable_v<T&&>,
					"Tasks passed to thread_pool::enqueue() must be callable as void() noexcept or void(size_t) noexcept"
				);
				MUU_MOVE_CHECK;

				const auto qindex = lock();
				enqueue(qindex, std::forward<T>(task));
				unlock(qindex);
				return *this;
			}
	};
}
MUU_NAMESPACE_END

MUU_POP_WARNINGS // MUU_DISABLE_SPAM_WARNINGS

#undef MUU_MOVE_CHECK

#if MUU_IMPLEMENTATION
	#include "../muu/impl/thread_pool.hpp"
#endif
