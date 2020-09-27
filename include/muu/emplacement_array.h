// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief Contains the definition of muu::emplacement_array.

#pragma once
#include "../muu/core.h"
#include "../muu/aligned_alloc.h"

MUU_PUSH_WARNINGS
MUU_DISABLE_SPAM_WARNINGS

MUU_NAMESPACE_START
{
	/// \brief	Simple std::vector-like array type for immovable or move-only types.
	///
	/// \tparam	T	Element type stored in the array.
	template <typename T>
	class emplacement_array
	{
		static_assert(
			!std::is_reference_v<T>,
			"emplacement_array element type cannot be a reference."
		);
		static_assert(
			std::is_nothrow_destructible_v<T>,
			"emplacement_array element type must be nothrow-destructible."
		);

		private:
			size_t count = {}, cap;
			std::byte* storage = nullptr;

			[[nodiscard]]
			MUU_ATTR(returns_nonnull)
			T* ptr(size_t index) noexcept
			{
				return launder(reinterpret_cast<T*>(storage + index * sizeof(T)));
			}

			[[nodiscard]]
			MUU_ATTR(returns_nonnull)
			const T* ptr(size_t index) const noexcept
			{
				return launder(reinterpret_cast<const T*>(storage + index * sizeof(T)));
			}

			void destroy_all_elements()
				noexcept(std::is_nothrow_destructible_v<T>)
			{
				if constexpr (std::is_trivially_destructible_v<T>)
					count = {};
				else
				{
					for (size_t i = count; i --> 0u;)
					{
						ptr(i)->~T();
						if constexpr (!std::is_nothrow_destructible_v<T>)
							count--;
					}
					if constexpr (std::is_nothrow_destructible_v<T>)
						count = {};
				}

			}

			void release()
				noexcept(std::is_nothrow_destructible_v<T>)
			{
				destroy_all_elements();
				aligned_free(storage);
				storage = nullptr;
			}

		public:

			/// \brief	The array's value type.
			using value_type = T;

			/// \brief	Unsigned index type.
			using size_type = size_t;

			/// \brief	Signed index type.
			using difference_type = ptrdiff_t;

			/// \brief	Reference to the array's value type.
			using reference = value_type&;

			/// \brief	Const reference to the array's value type.
			using const_reference = const value_type&;

			/// \brief	Pointer to the array's value type.
			using pointer = value_type*;

			/// \brief	Const pointer to the array's value type.
			using const_pointer = const value_type*;

			/// \brief	Iterator to a member of the array.
			using iterator = pointer;

			/// \brief	Const iterator to a member of the array.
			using const_iterator = const_pointer;

			/// \brief	Constructor.
			///
			/// \param	capacity	The maximum number of elements the array needs to be able to store.
			MUU_NODISCARD_CTOR
			explicit emplacement_array(size_t capacity = 0) noexcept
				: cap{ capacity },
				storage{ cap ? reinterpret_cast<std::byte*>(aligned_alloc((max)(alignof(T), size_t{ __STDCPP_DEFAULT_NEW_ALIGNMENT__ }), sizeof(T) * cap)) : nullptr }
			{}

			/// \brief	Move constructor.
			emplacement_array(emplacement_array&& other) noexcept
				: count{ other.count },
				cap{ other.cap },
				storage{ other.storage }
			{
				other.count = 0;
				other.capacity = 0;
				other.storage = nullptr;
			}

			/// \brief Move-assignment operator.
			emplacement_array& operator= (emplacement_array&& rhs) noexcept
			{
				release();
				count = rhs.count;
				cap = rhs.cap;
				storage = rhs.storage;
				rhs.count = 0;
				rhs.cap = 0;
				rhs.storage = nullptr;
				return *this;
			}

			/// \brief Destructor.
			~emplacement_array() noexcept
			{
				release();
			}

			MUU_DELETE_COPY(emplacement_array);

			/// \brief	Constructs one new element at the end of the array.
			///
			/// \tparam	Args	Constructor argument types.
			/// \param	args	Arguments to pass to the element's constructor.
			///
			/// \return	A reference to the newly-constructed element.
			template <typename... Args>
			reference emplace_back(Args &&... args)
				noexcept(std::is_nothrow_constructible_v<T, Args&&...>)
			{
				MUU_ASSERT(count < cap && "The emplacement_array is at capacity");

				if constexpr (std::is_nothrow_constructible_v<T, Args&&...>)
				{
					return *(new (storage + (sizeof(T) * count++)) T{ std::forward<Args>(args)... });
				}
				else
				{
					auto& newVal = *(new (storage + (sizeof(T) * count)) T{ std::forward<Args>(args)... });
					count++;
					return newVal;
				}
			}

			/// \brief	Constructs one new element in the array using a construction functor for the emplacement.
			///
			/// \tparam EmplacementFunc		A callable type with the signature `T*(void*)`.
			/// \param func					A callable responsible for performing the placement-new construction.
			///
			/// \return	A reference to the newly-constructed element.
			template <typename EmplacementFunc, typename... ARGS>
			reference emplace_back_with(EmplacementFunc&& func)
				noexcept(std::is_nothrow_invocable_r_v<T&, EmplacementFunc&&, void*>)
			{
				MUU_ASSERT(count < cap && "The emplacement_array is at capacity");

				if constexpr (std::is_nothrow_invocable_r_v<T&, EmplacementFunc&&, void*>)
				{
					return *std::forward<EmplacementFunc>(func)(static_cast<void*>(storage + (sizeof(T) * count++)));
				}
				else
				{
					auto& newVal = *std::forward<EmplacementFunc>(func)(static_cast<void*>(storage + (sizeof(T) * count)));
					count++;
					return newVal;
				}
			}

			/// \brief	Returns a reference to the element at the selected index.
			[[nodiscard]]
			reference operator[] (size_t index) noexcept
			{
				return *ptr(index);
			}

			/// \brief	Returns a const reference to the element at the selected index.
			[[nodiscard]]
			const_reference operator[] (size_t index) const noexcept
			{
				return *ptr(index);
			}

			/// \brief	The number of elements in the array.
			[[nodiscard]]
			size_type size() const noexcept
			{
				return count;
			}

			/// \brief	The maximum number of elements that can be stored in the array.
			[[nodiscard]]
			size_type capacity() const noexcept
			{
				return cap;
			}

			/// \brief	Returns true if the array is empty.
			[[nodiscard]]
			bool empty() const noexcept
			{
				return count == 0u;
			}

			/// \brief	Returns the first element in the array.
			[[nodiscard]]
			reference front() noexcept
			{
				return (*this)[0];
			}

			/// \brief	Returns the first element in the array.
			[[nodiscard]]
			const_reference front() const noexcept
			{
				return (*this)[0];
			}

			/// \brief	Returns the last element in the array.
			[[nodiscard]]
			reference back() noexcept
			{
				return (*this)[count - 1_sz];
			}

			/// \brief	Returns the last element in the array.
			[[nodiscard]]
			const_reference back() const noexcept
			{
				return (*this)[count - 1_sz];
			}

			/// \brief	Returns a pointer to the first element in the array.
			[[nodiscard]]
			iterator begin() noexcept
			{
				return ptr(0);
			}

			/// \brief	Returns a pointer to one-past-the-last element in the array.
			[[nodiscard]]
			iterator end() noexcept
			{
				return ptr(count);
			}

			/// \brief	Returns a const pointer to the first element in the array.
			[[nodiscard]]
			const_iterator begin() const noexcept
			{
				return ptr(0);
			}

			/// \brief	Returns a const pointer to one-past-the-last element in the array.
			[[nodiscard]]
			const_iterator end() const noexcept
			{
				return ptr(count);
			}

			/// \brief	Returns a const pointer to the first element in the array.
			[[nodiscard]]
			const_iterator cbegin() const noexcept
			{
				return ptr(0);
			}

			/// \brief	Returns a const pointer to one-past-the-last element in the array.
			[[nodiscard]]
			const_iterator cend() const noexcept
			{
				return ptr(count);
			}

			/// \brief	Returns a pointer to the first element in the array.
			[[nodiscard]]
			pointer data() noexcept
			{
				return ptr(0);
			}

			/// \brief	Returns a const pointer to the first element in the array.
			[[nodiscard]]
			const_pointer data() const noexcept
			{
				return ptr(0);
			}

			/// \brief	Clears the array, destroying all currently allocated elements and resetting size to zero.
			void clear()
				noexcept(std::is_nothrow_destructible_v<T>)
			{
				destroy_all_elements();
			}
	};
}
MUU_NAMESPACE_END

MUU_POP_WARNINGS // MUU_DISABLE_SPAM_WARNINGS