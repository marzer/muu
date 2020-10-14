// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::span.

#pragma once
#include "../muu/core.h"
#include "../muu/compressed_pair.h"
MUU_DISABLE_WARNINGS
#include <iterator>
#ifdef __cpp_lib_span
#include <span>
#endif
MUU_ENABLE_WARNINGS

MUU_NAMESPACE_START
{
	#ifdef DOXYGEN
	/// \brief Indicates the number of elements covered by a span should be dynamically-determined at runtime.
	/// \ingroup	mem
	inline constexpr size_t dynamic_extent = static_cast<size_t>(-1);
	//(actually defined in fwd.h)
	#endif

	namespace impl
	{
		struct span_empty_size
		{
			constexpr span_empty_size() noexcept = default;
			constexpr span_empty_size(size_t) noexcept {}
		};

		template <typename From, typename To>
		struct is_qualifier_compatible { static constexpr bool value = true; };
		template <typename From, typename To>
		struct is_qualifier_compatible<From, const To> { static constexpr bool value = true; };
		template <typename From, typename To>
		struct is_qualifier_compatible<From, volatile To> { static constexpr bool value = true; };
		template <typename From, typename To>
		struct is_qualifier_compatible<From, const volatile To> { static constexpr bool value = true; };
		template <typename From, typename To>
		struct is_qualifier_compatible<const From, const To> { static constexpr bool value = true; };
		template <typename From, typename To>
		struct is_qualifier_compatible<const From, volatile To> { static constexpr bool value = false; };
		template <typename From, typename To>
		struct is_qualifier_compatible<const From, const volatile To> { static constexpr bool value = true; };
		template <typename From, typename To>
		struct is_qualifier_compatible<volatile From, const To> { static constexpr bool value = false; };
		template <typename From, typename To>
		struct is_qualifier_compatible<volatile From, volatile To> { static constexpr bool value = true; };
		template <typename From, typename To>
		struct is_qualifier_compatible<volatile From, const volatile To> { static constexpr bool value = true; };
		template <typename From, typename To>
		struct is_qualifier_compatible<const volatile From, const To> { static constexpr bool value = false; };
		template <typename From, typename To>
		struct is_qualifier_compatible<const volatile From, volatile To> { static constexpr bool value = false; };
		template <typename From, typename To>
		struct is_qualifier_compatible<const volatile From, const volatile To> { static constexpr bool value = true; };

		template <typename From, typename To>
		inline constexpr bool is_qualifier_conversion_only =
			std::is_same_v<remove_cvref<From>, remove_cvref<To>>
			&& is_qualifier_compatible<std::remove_reference_t<From>, std::remove_reference_t<To>>::value;
	}

	/// \brief	A non-owning view of contiguous elements.
	/// \ingroup	building_blocks mem
	/// 
	/// \remarks This is equivalent to C++20's std::span.
	///
	/// \tparam	T	  	The span's element type.
	/// \tparam	Extent	The number of elements represented by the span.
	/// 				Use muu::dynamic_extent for dynamically-sized spans.
	template <typename T, size_t Extent
		#ifdef DOXYGEN
		= dynamic_extent
		#endif
	>
	class span
	{
		static_assert(
			!std::is_reference_v<T>,
			"Spans cannot store references"
		);

		public:
			using element_type = T; ///< The span's element type.
			using value_type = std::remove_cv_t<T>; ///< The span's base value type.
			using size_type = size_t; ///< std::size_t
			using difference_type = ptrdiff_t; ///< std::ptrdiff_t
			using pointer = std::add_pointer_t<T>; ///< A pointer to the span's element type.
			using const_pointer = std::add_pointer_t<std::add_const_t<T>>; ///< A const pointer to the span's element type.
			using reference = std::add_lvalue_reference_t<T>; ///< A reference to the span's element type.
			using const_reference = std::add_lvalue_reference_t<std::add_const_t<T>>; ///< A const reference to the span's element type.
			using iterator = pointer; ///< A LegacyRandomAccessIterator for the elements in the span.
			using reverse_iterator = std::reverse_iterator<iterator>; ///< std::reverse_iterator<iterator>

			static constexpr size_t extent = Extent; ///< The number of elements in the span, or muu::dynamic_extent.

		private:
			using pair_size_type = std::conditional_t<Extent == dynamic_extent, size_t, impl::span_empty_size>;
			compressed_pair<pointer, pair_size_type> ptr_and_size{};

		public:

			#if defined(DOXYGEN) || MUU_CONCEPTS

			/// \brief Default constructor.
			/// \note This constructor is not available for statically-sized spans.
			MUU_NODISCARD_CTOR
			constexpr span() noexcept
				MUU_REQUIRES(Extent == 0 || Extent == dynamic_extent)
				= default;

			#else

			template <size_t E = Extent MUU_SFINAE(E == 0 || E == dynamic_extent)>
			MUU_NODISCARD_CTOR
			constexpr span() noexcept {}

			#endif

			/// \brief Constructs a span from a contiguous iterator and an element count.
			template <typename It MUU_SFINAE_NO_CONCEPTS(
				impl::is_qualifier_conversion_only<impl::iter_reference_t<It>, element_type>
			)>
			MUU_NODISCARD_CTOR
			MUU_EXPLICIT(Extent != dynamic_extent)
			constexpr span(It first, size_t count) noexcept
				MUU_REQUIRES(
					std::contiguous_iterator<It>
					&& impl::is_qualifier_conversion_only<impl::iter_reference_t<It>, element_type>
				)
				: ptr_and_size{ pointer_cast<pointer>(muu::to_address(first)), count }
			{
				if constexpr (Extent != dynamic_extent)
					MUU_CONSTEXPR_SAFE_ASSERT(
						count == Extent
						&& "count must be equal to span extent for statically-sized spans"
					);
				if constexpr (Extent > 0)
					MUU_CONSTEXPR_SAFE_ASSERT(
						(count == 0_sz || ptr_and_size.first() != nullptr)
						&& "a nullptr span is undefined behaviour"
					);
			}

			/// \brief Constructs a span from a pair of contiguous iterators.
			template <typename It, typename End MUU_SFINAE_NO_CONCEPTS(
				impl::is_qualifier_conversion_only<impl::iter_reference_t<It>, element_type>
				&& !std::is_convertible_v<End, size_t>
			)>
			MUU_NODISCARD_CTOR
			MUU_EXPLICIT(Extent != dynamic_extent)
			constexpr span(It first, End last)
				noexcept(noexcept(last - first))
				MUU_REQUIRES(
					std::contiguous_iterator<It>
					&& std::sized_sentinel_for<End, It>
					&& impl::is_qualifier_conversion_only<impl::iter_reference_t<It>, element_type>
					&& !std::is_convertible_v<End, size_t>
				)
				: ptr_and_size{ pointer_cast<pointer>(muu::to_address(first)), static_cast<size_t>(last - first) }
			{
				if constexpr (Extent != dynamic_extent)
					MUU_CONSTEXPR_SAFE_ASSERT(
						static_cast<size_t>(last - first) == size()
						&& "(last - first) must be equal to span extent for statically-sized spans"
					);
				if constexpr (Extent > 0)
					MUU_CONSTEXPR_SAFE_ASSERT(
						(ptr_and_size.second() == 0_sz || ptr_and_size.first() != nullptr)
						&& "a nullptr span is undefined behaviour"
					);
			}

			/// \brief Constructs a span from an array.
			template <size_t N MUU_SFINAE_NO_CONCEPTS(Extent == dynamic_extent || N == Extent)>
			MUU_NODISCARD_CTOR
			constexpr span(type_identity<element_type>(&arr)[N]) noexcept
				MUU_REQUIRES(Extent == dynamic_extent || N == Extent)
				: ptr_and_size{ arr, N }
			{}

			/// \brief Constructs a span from an array.
			template <typename U, size_t N MUU_SFINAE_NO_CONCEPTS(
				impl::is_qualifier_conversion_only<U, element_type>
				&& (Extent == dynamic_extent || N == Extent)
			)>
			MUU_NODISCARD_CTOR
			constexpr span(std::array<U, N>& arr) noexcept
				MUU_REQUIRES(
					impl::is_qualifier_conversion_only<U, element_type>
					&& (Extent == dynamic_extent || N == Extent)
				)
				: ptr_and_size{ pointer_cast<pointer>(arr.data()), N }
			{}

			/// \brief Constructs a span from an array.
			template <typename U, size_t N MUU_SFINAE_NO_CONCEPTS(
				impl::is_qualifier_conversion_only<std::add_const_t<U>, element_type>
				&& (Extent == dynamic_extent || N == Extent)
			)>
			MUU_NODISCARD_CTOR
			constexpr span(const std::array<U, N>& arr) noexcept
				MUU_REQUIRES(
					impl::is_qualifier_conversion_only<std::add_const_t<U>, element_type>
					&& (Extent == dynamic_extent || N == Extent)
				)
				: ptr_and_size{ pointer_cast<pointer>(arr.data()), N }
			{}

			/// \brief Constructs a span from another span.
			template <typename U, size_t E MUU_SFINAE_NO_CONCEPTS(
				impl::is_qualifier_conversion_only<U, element_type>
				&& (Extent == dynamic_extent || E == dynamic_extent || E == Extent)
			)>
			MUU_NODISCARD_CTOR
			MUU_EXPLICIT(Extent != dynamic_extent && E == dynamic_extent)
			constexpr span(const span<U, E>& s) noexcept
				MUU_REQUIRES(
					impl::is_qualifier_conversion_only<U, element_type>
					&& (Extent == dynamic_extent || E == dynamic_extent || E == Extent)
				)
				: ptr_and_size{ pointer_cast<pointer>(s.data()), s.size() }
			{}


			/// \brief Copy constructor.
			constexpr span(const span&) noexcept = default;

			/// \brief Copy-assignment operator.
			constexpr span& operator=(const span&) noexcept = default;

			#ifdef __cpp_lib_span

			/// \brief Constructs a muu::span from a C++20 std::span (if available).
			template <typename U, size_t E MUU_SFINAE_NO_CONCEPTS(
				impl::is_qualifier_conversion_only<U, element_type>
				&& (Extent == dynamic_extent || E == dynamic_extent || E == Extent)
			)>
			MUU_NODISCARD_CTOR
			explicit
			constexpr span(const std::span<U, E>& s) noexcept
				MUU_REQUIRES(
					impl::is_qualifier_conversion_only<U, element_type>
					&& (Extent == dynamic_extent || E == dynamic_extent || E == Extent)
				)
				: ptr_and_size{ pointer_cast<pointer>(s.data()), s.size() }
			{}

			/// \brief Constructs a C++20 std::span from a muu::span (if std::span is available).
			template <typename U, size_t E MUU_SFINAE_NO_CONCEPTS(
				impl::is_qualifier_conversion_only<U, element_type>
				&& (Extent == dynamic_extent || E == dynamic_extent || E == Extent)
			)>
			[[nodiscard]]
			/* implicit */
			constexpr operator std::span<U, E>() const noexcept
				MUU_REQUIRES(
					impl::is_qualifier_conversion_only<U, element_type>
					&& (Extent == dynamic_extent || E == dynamic_extent || E == Extent)
				)
			{
				return std::span<U, E>{ data(), size() };
			}

			#endif // __cpp_lib_span

			/// \brief Returns the number of elements covered by the span.
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr size_t size() const noexcept
			{
				if constexpr (Extent == dynamic_extent)
					return ptr_and_size.second();
				else
					return Extent;
			}

			/// \brief Returns the total size of the elements covered by the span in bytes.
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr size_t size_bytes() const noexcept
			{
				if constexpr (Extent == dynamic_extent)
					return ptr_and_size.second() * sizeof(element_type);
				else
					return Extent * sizeof(element_type);
			}

			/// \brief Returns true if the span is empty (i.e. covers zero elements).
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr bool empty() const noexcept
			{
				return !size();
			}

			/// \brief Returns an iterator to the beginning of the span.
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr iterator begin() const noexcept
			{
				return ptr_and_size.first();
			}

			/// \brief Returns an iterator to end of the span.
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr iterator end() const noexcept
			{
				return ptr_and_size.first() + size();
			}

			/// \brief Returns a reverse iterator to the beginning of the span.
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr reverse_iterator rbegin() const noexcept
			{
				return reverse_iterator{ end() };
			}

			/// \brief Returns a reverse iterator to the end of the span.
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr reverse_iterator rend() const noexcept
			{
				return reverse_iterator{ begin() };
			}

			/// \brief Returns a reference to the first element in the span.
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr reference front() const noexcept
			{
				return *ptr_and_size.first();
			}

			/// \brief Returns a reference to the last element in the span.
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr reference back() const noexcept
			{
				return ptr_and_size.first()[size() - 1u];
			}

			/// \brief Returns a reference to an arbitrary element in the span.
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr reference operator[](size_t idx) const noexcept
			{
				return ptr_and_size.first()[idx];
			}

			/// \brief Returns a pointer to the first element in the span.
			[[nodiscard]]
			MUU_ATTR(pure)
			constexpr pointer data() const noexcept
			{
				return ptr_and_size.first();
			}

			/// \brief Returns a subspan representing the first N elements of this span.
			template <size_t Count>
			[[nodiscard]]
			MUU_ATTR_NDEBUG(pure)
			constexpr span<element_type, Count> first() const noexcept
			{
				static_assert(
					Count <= Extent,
					"span::first() count cannot exceed the size of the source span"
				);
				MUU_CONSTEXPR_SAFE_ASSERT(Count <= size());
				return span<element_type, Count>{ ptr_and_size.first(), Count };
			}

			/// \brief Returns a subspan representing the first N elements of this span.
			[[nodiscard]]
			MUU_ATTR_NDEBUG(pure)
			constexpr span<element_type> first(size_t count) const noexcept
			{
				MUU_CONSTEXPR_SAFE_ASSERT(count <= size());
				return span<element_type>{ ptr_and_size.first(), count };
			}

			/// \brief Returns a subspan representing the last N elements of this span.
			template <size_t Count>
			[[nodiscard]]
			MUU_ATTR_NDEBUG(pure)
			constexpr span<element_type, Count> last() const noexcept
			{
				static_assert(
					Count <= Extent,
					"span::last() count cannot exceed the size of the source span"
				);
				MUU_CONSTEXPR_SAFE_ASSERT(Count <= size());
				return span<element_type, Count>{ ptr_and_size.first() + (size() - Count), Count };
			}

			/// \brief Returns a subspan representing the last N elements of this span.
			[[nodiscard]]
			MUU_ATTR_NDEBUG(pure)
			constexpr span<element_type> last(size_t count) const noexcept
			{
				MUU_CONSTEXPR_SAFE_ASSERT(count <= size());
				return span<element_type>{ ptr_and_size.first() + (size() - count), count };
			}

			/// \brief Returns an arbitrary subspan.
			template <size_t Offset, size_t Count = dynamic_extent>
			[[nodiscard]]
			MUU_ATTR_NDEBUG(pure)
			constexpr auto subspan() const noexcept
			{
				static_assert(
					Offset < Extent,
					"span::subspan() Offset cannot exceed the size of the source span"
				);
				static_assert(
					Count == dynamic_extent
					|| Count <= (Extent - Offset),
					"span::subspan() Count cannot exceed the size of the source span"
				);

				MUU_CONSTEXPR_SAFE_ASSERT(Offset < size());

				if constexpr (Count == dynamic_extent)
				{
					return span<element_type>{ ptr_and_size.first() + Offset, size() - Offset };
				}
				else
				{
					MUU_CONSTEXPR_SAFE_ASSERT(Count <= (size() - Offset));
					return span<element_type, Count>{ ptr_and_size.first() + Offset, Count };
				}
			}

			/// \brief Returns an arbitrary subspan.
			[[nodiscard]]
			MUU_ATTR_NDEBUG(pure)
			constexpr span<element_type> subspan(size_t offset, size_t count = dynamic_extent) const noexcept
			{
				MUU_CONSTEXPR_SAFE_ASSERT(offset < size());

				if (count == dynamic_extent)
				{
					return span<element_type>{ ptr_and_size.first() + offset, size() - offset };
				}
				else
				{
					MUU_CONSTEXPR_SAFE_ASSERT(count <= (size() - offset));
					return span<element_type>{ ptr_and_size.first() + offset, count };
				}
			}
	};
	#ifndef DOXYGEN
	template <typename T>				span(T*, size_t)				-> span<T>;
	template <typename T, size_t N>		span(T(&)[N])					-> span<T, N>;
	template <typename T, size_t N>		span(T(&&)[N])					-> span<T, N>;
	template <typename T, size_t N>		span(std::array<T, N>&)			-> span<T, N>;
	template <typename T, size_t N>		span(const std::array<T, N>&)	-> span<const T, N>;
	#if MUU_CONCEPTS
	template <std::contiguous_iterator It, typename E> span(It, E)		-> span<impl::iter_value_t<It>>;
	#else
	template <typename It, typename E>	span(It, E)						->	span<impl::iter_value_t<It>>;
	template <typename It>				span(It, It)					->	span<impl::iter_value_t<It>>;
	#endif
	#endif // !DOXYGEN

	/// \brief	Convenience alias for `span<const T>`.
	/// \ingroup	mem
	template <typename T>
	using const_span = span<const T>;

	/// \brief	Convenience alias for `span<std::byte>`.
	/// \ingroup	mem
	using byte_span = span<std::byte>;

	/// \brief	Convenience alias for `span<const std::byte>`.
	/// \ingroup	mem
	using const_byte_span = span<const std::byte>;

	/// \brief	Reinterprets a span as an immutable view of the underlying bytes.
	/// \ingroup	mem
	///
	/// \details Equivalent to C++20's std::as_bytes.
	template <typename T, size_t N>
	[[nodiscard]]
	auto as_bytes(span<T, N> s) noexcept
	{
		if constexpr (N == dynamic_extent)
		{
			return span<const std::byte*>{ pointer_cast<const std::byte*>(s.data()), s.size_bytes() };
		}
		else
		{
			return span<const std::byte*, sizeof(T) * N>{ pointer_cast<const std::byte*>(s.data()), sizeof(T)* N };
		}
	}

	/// \brief	Reinterprets a span as a view of the underlying bytes.
	/// \ingroup	mem
	///
	/// \details Equivalent to C++20's std::as_writable_bytes.
	template <typename T, size_t N>
	[[nodiscard]]
	auto as_writable_bytes(span<T, N> s) noexcept
	{
		if constexpr (N == dynamic_extent)
		{
			return span<std::byte*>{ pointer_cast<std::byte*>(s.data()), s.size_bytes() };
		}
		else
		{
			return span<std::byte*, sizeof(T)* N>{ pointer_cast<std::byte*>(s.data()), sizeof(T)* N };
		}
	}
}
MUU_NAMESPACE_END
