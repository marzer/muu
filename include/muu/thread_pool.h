// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::thread_pool.

#pragma once
#include "../muu/core.h"
#include "../muu/tagged_ptr.h"
#include "../muu/string_param.h"
MUU_DISABLE_WARNINGS
#ifdef __cpp_lib_hardware_interference_size
	#include <new>
#endif
#include <memory>
MUU_ENABLE_WARNINGS

#define MUU_MOVE_CHECK	MUU_ASSERT(pimpl_ != nullptr && "The thread_pool has been moved from!")
#if MUU_ARCH_AMD64
	#define	MUU_COMPRESSED_THREAD_POOL_TASK 1
#else
	#define	MUU_COMPRESSED_THREAD_POOL_TASK 0
#endif

MUU_PUSH_WARNINGS
MUU_DISABLE_SPAM_WARNINGS

MUU_IMPL_NAMESPACE_START
{
	template <typename Func, typename Arg>
	inline constexpr bool is_thread_pool_task = 
		std::is_nothrow_invocable_v<Func, Arg, size_t>
		|| std::is_nothrow_invocable_v<Func, Arg>;

	template <typename T>
	inline constexpr bool is_trivial_task
		= std::is_function_v<std::remove_pointer_t<std::remove_reference_t<T>>>
		|| std::is_empty_v<remove_cvref<T>>
		|| (std::is_class_v<remove_cvref<T>> && sizeof(T) == 1_sz && has_unary_plus_operator<T>)
		|| std::is_trivially_copyable_v<remove_cvref<T>>;

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

		enum class callable_storage_modes : uint8_t
		{
			stored_directly = 0,
			pointer_to_heap = 1,
			pointer_to_source = 2
		};

		template <typename T>
		struct callable_traits
		{
			using base_type = std::conditional_t<
				std::is_function_v<remove_cvref<T>>,
				std::add_pointer_t<remove_cvref<T>>,
				remove_cvref<T>
			>;
			static_assert(
				std::is_invocable_v<base_type, size_t>
				|| std::is_invocable_v<base_type>
			);

			static constexpr bool small_size = sizeof(base_type) <= sizeof(callable_buffer_type);
			static constexpr bool small_alignment = alignof(base_type) <= thread_pool_task_granularity;
			static constexpr bool constructible = std::is_constructible_v<base_type, T>; //move or copy
			static constexpr callable_storage_modes storage_mode =
				callable_traits::constructible
					? (callable_traits::small_size && callable_traits::small_alignment
						? callable_storage_modes::stored_directly
						: callable_storage_modes::pointer_to_heap)
					: callable_storage_modes::pointer_to_source;
			static constexpr bool requires_explicit_destruction = storage_mode == callable_storage_modes::pointer_to_heap
				|| (storage_mode == callable_storage_modes::stored_directly && !std::is_trivially_destructible_v<base_type>);
			using callable_type = std::conditional_t<
				storage_mode == callable_storage_modes::pointer_to_source,
				std::remove_reference_t<T>, // preserve cv qualifiers
				base_type
			>;
		};

		template <typename T>
		MUU_NODISCARD_CTOR
		thread_pool_task(T && callable_) noexcept
		{
			using traits = callable_traits<T&&>;
			using callable_type = typename traits::callable_type;

			// store callable
			if constexpr (traits::storage_mode == callable_storage_modes::stored_directly)
				new (callable_buffer) callable_type{ std::forward<T>(callable_) };
			else if constexpr (traits::storage_mode == callable_storage_modes::pointer_to_heap)
			{
				auto ptr = new callable_type{ std::forward<T>(callable_) };
				memcpy(callable_buffer, &ptr, sizeof(ptr));
			}
			else if constexpr (traits::storage_mode == callable_storage_modes::pointer_to_source)
				memcpy(callable_buffer, &callable_, sizeof(callable_type*));
			else
				static_assert(dependent_false<T>, "Evaluated unreachable branch!");

			// create invoker/mover/deleter
			action_invoker = [](thread_pool_task_action&& action) noexcept
			{
				switch (action.type)
				{
					case thread_pool_task_action::types::invoke:
					{
						if constexpr (traits::requires_explicit_destruction)
						{
							MUU_ASSERT(!action.task.has_state(states::destroyed));
						}

						// fetch callable
						callable_type* callable;
						if constexpr (traits::storage_mode == callable_storage_modes::stored_directly)
							callable = launder(reinterpret_cast<callable_type*>(action.task.callable_buffer));
						else
							memcpy(&callable, action.task.callable_buffer, sizeof(callable));
						MUU_ASSERT(callable);

						//invoke callable
						if constexpr (std::is_invocable_v<callable_type, size_t>)
							(*callable)(action.data.thread_index);
						else
							(*callable)();

						return;
					}

					case thread_pool_task_action::types::move:
					{
						if constexpr (traits::requires_explicit_destruction)
						{
							MUU_ASSERT(!action.task.has_state(states::destroyed));
						}

						// copy invoker/mover/deleter and flags
						action.task.action_invoker = action.data.source->action_invoker;
						#if !MUU_COMPRESSED_THREAD_POOL_TASK
						action.task.set_state(action.data.source->state());
						#endif

						// move/copy callable
						if constexpr (traits::storage_mode == callable_storage_modes::stored_directly)
						{
							if constexpr (std::is_trivially_copyable_v<callable_type>)
								memcpy(&action.task.callable_buffer, action.data.source->callable_buffer, sizeof(callable_type));
							else if constexpr (std::is_move_constructible_v<callable_type>)
							{
								new (action.task.callable_buffer) callable_type{ std::move(
									*launder(reinterpret_cast<callable_type*>(action.data.source->callable_buffer))
								) };
							}
							else
							{
								new (action.task.callable_buffer) callable_type{
									*launder(reinterpret_cast<callable_type*>(action.data.source->callable_buffer))
								};
							}
						}
						else
							memcpy(&action.task.callable_buffer, action.data.source->callable_buffer, sizeof(callable_type*));

						if constexpr (traits::storage_mode == callable_storage_modes::pointer_to_heap)
							action.data.source->add_state(states::destroyed);

						return;
					}

					case thread_pool_task_action::types::destroy:
					{
						if constexpr (traits::requires_explicit_destruction)
						{
							MUU_ASSERT(action.type == thread_pool_task_action::types::destroy);
							MUU_ASSERT(!action.task.has_state(states::destroyed));

							if constexpr (traits::storage_mode == callable_storage_modes::stored_directly)
								launder(reinterpret_cast<callable_type*>(action.task.callable_buffer))->~callable_type();
							else if constexpr (traits::storage_mode == callable_storage_modes::pointer_to_heap)
							{
								callable_type* callable;
								memcpy(&callable, action.task.callable_buffer, sizeof(callable));
								delete callable;
							}
							else
								static_assert(dependent_false<T>, "Evaluated unreachable branch!");

							return;
						}
						else
							MUU_UNREACHABLE;
					}

					MUU_NO_DEFAULT_CASE;
				}

				MUU_UNREACHABLE;
			};
			MUU_ASSERT(action_invoker);

			if constexpr (traits::requires_explicit_destruction)
				set_state(states::requires_explicit_destruction);
		}

		thread_pool_task(thread_pool_task&& task) noexcept
		{
			MUU_ASSERT(task.action_invoker);
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
			MUU_ASSERT(action_invoker);
			action_invoker(thread_pool_task_action{ *this, worker_index });
		}
	};
	
	MUU_PRAGMA_MSVC(pack(pop))

	static_assert(sizeof(thread_pool_task) <= thread_pool_task_granularity);
	static_assert(alignof(thread_pool_task) == thread_pool_task_granularity);
	static_assert(MUU_OFFSETOF(thread_pool_task, callable_buffer) == 0);
	static_assert(std::is_standard_layout_v<thread_pool_task>);

	template <typename T>
	class batch_size_generator final
	{
		private:
			T remaining;
			T constant;
			T overflow;

		public:
			batch_size_generator(T job_count, T batch_count) noexcept
				: remaining{ job_count },
				constant{ job_count / batch_count },
				overflow{ job_count % batch_count }
			{}

			[[nodiscard]]
			MUU_ALWAYS_INLINE
			T operator() () noexcept
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
MUU_IMPL_NAMESPACE_END

MUU_NAMESPACE_START
{
	/// \brief A thread pool.
	class thread_pool
	{
		private:
			struct pimpl;
			pimpl* pimpl_ = nullptr;

			[[nodiscard]]
			MUU_API
			size_t lock() noexcept;

			[[nodiscard]]
			MUU_API
			MUU_ATTR(returns_nonnull)
			MUU_ATTR(assume_aligned(impl::thread_pool_task_granularity))
			void* acquire(size_t) noexcept;

			template <typename T>
			void enqueue(size_t queue_index, T&& task) noexcept
			{
				new (assume_aligned<impl::thread_pool_task_granularity>(acquire(queue_index))) impl::thread_pool_task{ std::forward<T>(task) };
			}

			MUU_API
			void unlock(size_t) noexcept;

			template <typename T>
			class for_each_task final
			{
				private:
					mutable T task;

				public:

					template <typename Arg>
					void operator()([[maybe_unused]] Arg&& arg, [[maybe_unused]] size_t tidx) const noexcept
					{
						if constexpr (std::is_invocable_v<T, Arg&&, size_t>)
							task(std::forward<Arg>(arg), tidx);
						else if constexpr (std::is_invocable_v<T, Arg&&>)
							task(std::forward<Arg>(arg));
						else if constexpr (std::is_invocable_v<T>)
							task();
						else
							static_assert(dependent_false<Arg>, "Evaluated unreachable branch!");
					}

					MUU_ALWAYS_INLINE for_each_task& operator* () noexcept { return *this; }
					MUU_ALWAYS_INLINE const for_each_task& operator* () const noexcept { return *this; }

					template <typename U>
					for_each_task(U&& t) noexcept
						: task{ std::forward<U>(t) }
					{}
			};

		public:

			/// \brief	Constructs a thread pool.
			///
			/// \param	worker_count		The number of worker threads in the pool. Use `0` for 'automatic'.
			/// \param	task_queue_size		Max tasks that can be stored in the internal queue without blocking. Use `0` for 'automatic'.
			/// \param	name 	The name of your threadpool (for debugging purposes).
			MUU_NODISCARD_CTOR
			MUU_API
			explicit thread_pool(size_t worker_count = 0, size_t task_queue_size = 0, string_param name = {}) noexcept;

			/// \brief	Constructs a thread_pool.
			///
			/// \param	name 	The name of your thread pool (for debugging purposes).
			MUU_NODISCARD_CTOR
			explicit thread_pool(string_param name) noexcept
				: thread_pool{ 0, 0, std::move(name) }
			{}

			/// \brief	Move constructor.
			MUU_API
			thread_pool(thread_pool&&) noexcept;

			/// \brief	Move-assignment operator.
			MUU_API
			thread_pool& operator= (thread_pool&&) noexcept;

			/// \brief	Destructor.
			/// \warning Any enqueued tasks *currently being executed* are allowed to finish;
			/// 		 attempting to destroy a thread_pool with an enqueued task of indeterminate
			/// 		 length may lead to a deadlock.
			MUU_API
			~thread_pool() noexcept;

			MUU_DELETE_COPY(thread_pool);

			/// \brief	The number of worker threads in the thread pool.
			[[nodiscard]]
			MUU_API
			MUU_ATTR(pure)
			size_t size() const noexcept;

			/// \brief	Waits for the thread pool to finish all of its current work.
			/// 
			/// \warning Do not call this from one of the thread pool's workers.
			MUU_API
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
					std::is_nothrow_invocable_v<Task&&, size_t>
					|| std::is_nothrow_invocable_v<Task&&>,
					"Tasks passed to thread_pool::enqueue() must be callable as void() noexcept or void(size_t) noexcept"
				);
				MUU_MOVE_CHECK;

				const auto qindex = lock();
				enqueue(qindex, std::forward<Task>(task));
				unlock(qindex);
				return *this;
			}

		private:

			template <typename Iter, typename Task>
			void enqueue_for_each_batch(Iter batch_start, Iter batch_end, Task&& task) noexcept
			{
				const auto qindex = lock();
				enqueue(
					qindex,
					[
						i = batch_start,
						e = batch_end,
						t = std::forward<Task>(task)
					]
					(size_t worker_index) mutable noexcept
					{
						while (i != e)
						{
							(*t)(*i, worker_index);
							std::advance(i, 1);
						}
					}
				);
				unlock(qindex);
			}

			template <typename Begin, typename Task>
			thread_pool& for_each_impl(Begin begin, size_t job_count, Task&& task) noexcept
			{
				using elem_reference = impl::iter_reference_t<Begin>;
				static_assert(
					std::is_nothrow_invocable_v<Task&&, elem_reference, size_t>
					|| std::is_nothrow_invocable_v<Task&&, elem_reference>,
					"Tasks passed to thread_pool::for_each() must be callable as void(elem) noexcept or void(elem, size_t) noexcept"
				);

				//determine batch count and distribute iterators
				auto batch_generator = impl::batch_size_generator<size_t>{ job_count, this->size() };
				size_t batch_size = batch_generator();
				auto batch_start = begin;
				auto batch_end = std::next(begin, static_cast<ptrdiff_t>(batch_size));

				//dispatch tasks (trivial callables)
				using for_each_task_type = for_each_task<remove_cvref<Task&&>>;
				if (impl::is_trivial_task<Task&&> || this->size() == 1u)
				{
					while (batch_size)
					{
						enqueue_for_each_batch(batch_start, batch_end, for_each_task_type{ std::forward<Task>(task) });
						batch_start = batch_end;
						batch_size = batch_generator();
						if (batch_size)
							std::advance(batch_end, static_cast<ptrdiff_t>(batch_size));
					}
					return *this;
				}

				//dispatch tasks (nontrivial callables)
				if constexpr (!impl::is_trivial_task<Task&&>)
				{
					auto task_ptr = std::make_shared<for_each_task_type>(std::forward<Task>(task));
					while (batch_size)
					{
						enqueue_for_each_batch(batch_start, batch_end, task_ptr);
						batch_start = batch_end;
						batch_size = batch_generator();
						if (batch_size)
							std::advance(batch_end, static_cast<ptrdiff_t>(batch_size));
					}
				}
				return *this;
			}

		public:

			/// \brief	Enqueues a task to execute on every element in a collection.
			/// 
			/// \details Tasks must be callables with one parameter matching the elements in your collection,
			/// 		 and optionally a second `size_t` parameter to recieve the index of the worker invoking the task:
			/// \cpp
			/// std::array<int, 10> vals;
			/// pool.for_each(vals.begin(), vals.end(), [](int& i) noexcept
			/// {
			///		// i is one of the elements of vals
			///	});
			/// pool.for_each(vals.begin(), vals.end(), [](int& i, size_t worker_index) noexcept
			/// {
			///		// i is one of the elements of vals
			///		// worker_index is in the range [0, pool.size())
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
			template <typename Iter, typename Task>
			thread_pool& for_each(Iter begin, Iter end, Task&& task) noexcept
			{
				MUU_MOVE_CHECK;

				const auto job_count = std::distance(begin, end);
				if (job_count <= 0)
					return *this;

				return for_each_impl(
					begin,
					static_cast<size_t>(job_count),
					std::forward<Task>(task)
				);
			}

			/// \brief	Enqueues a task to execute on every element in a collection.
			/// 
			/// \details Tasks must be callables with one parameter matching the elements in your collection,
			/// 		 and optionally a second `size_t` parameter to recieve the index of the worker invoking the task:
			/// \cpp
			/// std::array<int, 10> vals;
			/// pool.for_each(vals, [](int& i, size_t worker_index) noexcept
			/// {
			///		// i is one of the elements of vals
			///		// worker_index is in the range [0, pool.size())
			///	});
			/// pool.for_each(vals, [](int& i) noexcept
			/// {
			///		// i is one of the elements of vals
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
			template <typename T, typename Task>
			thread_pool& for_each(T&& collection, Task&& task) noexcept
			{
				MUU_MOVE_CHECK;

				using std::size;
				const size_t job_count = size(collection); //static_cast<size_t>(std::distance(b, end(std::forward<T>(collection))));
				if (!job_count)
					return *this;

				using std::begin;
				return for_each_impl(
					begin(std::forward<T>(collection)),
					job_count,
					std::forward<Task>(task)
				);
			}

		private:

			template <typename ValueType, typename T, typename Task>
			void enqueue_for_range_batch(T batch_start, T batch_end, Task&& task) noexcept
			{
				const auto qindex = lock();
				enqueue(
					qindex,
					[
						i = batch_start,
						e = batch_end,
						t = std::forward<Task>(task)
					]
					(size_t worker_index) mutable noexcept
					{
						if (i < e)
						{
							for (; i < e; i++)
								(*t)(static_cast<ValueType>(i), worker_index);
						}
						else
						{
							for (; i > e; i--)
								(*t)(static_cast<ValueType>(i), worker_index);
						}
					}
				);
				unlock(qindex);
			}

		public:
	
			/// \brief	Enqueues a task to execute once for every value in a range.
			/// 
			/// \details	Tasks must be callables that take 0-2 argument, with the first argument being the range's
			///				integer type, and the second beign a `size_t` corresponding to the index of
			///				the worker invoking the task:
			/// \cpp
			/// pool.for_range(0, 10, [](int i, size_t worker_index) noexcept
			/// {
			///		// i is in the range [0, 10)
			///		// worker_index is in the range [0, pool.size())
			///	});
			/// pool.for_range(0, 10, [](int i) noexcept
			/// {
			///		// i is in the range [0, 10)
			///	});
			/// pool.for_range(0, 10, []() noexcept
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
			template <typename T, typename Task>
			thread_pool& for_range(T start, T end, Task&& task) noexcept
			{
				static_assert(
					std::is_nothrow_invocable_v<Task&&, T, size_t>
					|| std::is_nothrow_invocable_v<Task&&, T>
					|| std::is_nothrow_invocable_v<Task&&>,
					"Tasks passed to thread_pool::for_range() must be callable as void() noexcept, void(T) noexcept or void(T, size_t) noexcept"
				);
				MUU_MOVE_CHECK;

				//determine batch count and distribute indices
				using value_type = remove_cvref<T>;
				using offset_type = largest<
					remove_enum<value_type>,
					std::conditional_t<is_signed<remove_enum<value_type>>, signed, unsigned>
				>;
				using size_type = largest<make_unsigned<offset_type>, size_t>;
				const auto job_count = static_cast<size_type>(
					static_cast<offset_type>((max)(unwrap(start), unwrap(end)))
					- static_cast<offset_type>((min)(unwrap(start), unwrap(end)))
				);
				if (!job_count)
					return *this;
				auto batch_generator = impl::batch_size_generator<size_type>{ job_count, this->size() };
				offset_type batch_start = unwrap(start);
				size_type batch_size = batch_generator();

				//dispatch tasks (trivial callables)
				using for_each_task_type = for_each_task<remove_cvref<Task&&>>;
				if (impl::is_trivial_task<Task&&> || this->size() == 1u)
				{
					while (batch_size)
					{
						offset_type batch_end = start < end
							? static_cast<offset_type>(batch_start + batch_size)
							: static_cast<offset_type>(batch_start - batch_size);
						enqueue_for_range_batch<value_type>(batch_start, batch_end, for_each_task_type{ std::forward<Task>(task) });
						batch_start = batch_end;
						batch_size = batch_generator();
					}
					return *this;
				}

				//dispatch tasks (nontrivial callables)
				if constexpr (!impl::is_trivial_task<Task&&>)
				{
					auto task_ptr = std::make_shared<for_each_task_type>(std::forward<Task>(task));
					while (batch_size)
					{
						offset_type batch_end = start < end
							? static_cast<offset_type>(batch_start + batch_size)
							: static_cast<offset_type>(batch_start - batch_size);
						enqueue_for_range_batch<value_type>(batch_start, batch_end, task_ptr);
						batch_start = batch_end;
						batch_size = batch_generator();
					}
				}
				return *this;
			}
	};
}
MUU_NAMESPACE_END

MUU_POP_WARNINGS // MUU_DISABLE_SPAM_WARNINGS

#undef MUU_MOVE_CHECK
