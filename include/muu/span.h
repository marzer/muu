// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the implementation of #muu::span.

#include "iterators.h"
#include "pointer_cast.h"
#include "assume_aligned.h"
#include "impl/header_start.h"

namespace muu
{
#if MUU_DOXYGEN

	/// \brief Indicates the number of elements covered by a span should be dynamically-determined at runtime.
	///
	/// \ingroup	memory
	inline constexpr size_t dynamic_extent = static_cast<size_t>(-1);
	//(here just for doxygen; actually defined in fwd.h)

#endif // DOXYGEN

	/// \cond
	namespace impl
	{
		template <typename From, typename To>
		struct is_qualifier_compatible
		{
			static constexpr bool value = true;
		};
		template <typename From, typename To>
		struct is_qualifier_compatible<From, const To>
		{
			static constexpr bool value = true;
		};
		template <typename From, typename To>
		struct is_qualifier_compatible<From, volatile To>
		{
			static constexpr bool value = true;
		};
		template <typename From, typename To>
		struct is_qualifier_compatible<From, const volatile To>
		{
			static constexpr bool value = true;
		};
		template <typename From, typename To>
		struct is_qualifier_compatible<const From, const To>
		{
			static constexpr bool value = true;
		};
		template <typename From, typename To>
		struct is_qualifier_compatible<const From, volatile To>
		{
			static constexpr bool value = false;
		};
		template <typename From, typename To>
		struct is_qualifier_compatible<const From, const volatile To>
		{
			static constexpr bool value = true;
		};
		template <typename From, typename To>
		struct is_qualifier_compatible<volatile From, const To>
		{
			static constexpr bool value = false;
		};
		template <typename From, typename To>
		struct is_qualifier_compatible<volatile From, volatile To>
		{
			static constexpr bool value = true;
		};
		template <typename From, typename To>
		struct is_qualifier_compatible<volatile From, const volatile To>
		{
			static constexpr bool value = true;
		};
		template <typename From, typename To>
		struct is_qualifier_compatible<const volatile From, const To>
		{
			static constexpr bool value = false;
		};
		template <typename From, typename To>
		struct is_qualifier_compatible<const volatile From, volatile To>
		{
			static constexpr bool value = false;
		};
		template <typename From, typename To>
		struct is_qualifier_compatible<const volatile From, const volatile To>
		{
			static constexpr bool value = true;
		};

		template <typename From, typename To>
		inline constexpr bool is_qualifier_conversion_only =
			std::is_same_v<remove_cvref<From>, remove_cvref<To>>
			&& is_qualifier_compatible<std::remove_reference_t<From>, std::remove_reference_t<To>>::value;

		template <typename T, size_t Extent>
		inline constexpr size_t as_bytes_extent = Extent == dynamic_extent ? dynamic_extent : (sizeof(T) * Extent);

		// span storage base - static extent
		template <typename T, size_t Extent, size_t Alignment>
		class MUU_TRIVIAL_ABI span_storage
		{
		  public:
			using element_type = T;
			using pointer	   = std::add_pointer_t<T>;

		  protected:
			pointer ptr_ = {};

		  public:
			span_storage()													= delete;
			constexpr span_storage(const span_storage&) noexcept			= default;
			constexpr span_storage& operator=(const span_storage&) noexcept = default;

			constexpr span_storage(pointer p, size_t) noexcept //
				: ptr_{ p }
			{
				MUU_CONSTEXPR_SAFE_ASSERT(ptr_ && "a nullptr span is undefined behaviour");

				if constexpr (Alignment > alignof(T))
					MUU_CONSTEXPR_SAFE_ASSERT(reinterpret_cast<uintptr_t>(ptr_) % Alignment == 0u
											  && "pointer alignment must be >= span's Alignment parameter");
			}

			MUU_CONST_INLINE_GETTER
			constexpr size_t size() const noexcept
			{
				return Extent;
			}

			MUU_CONST_INLINE_GETTER
			constexpr size_t size_bytes() const noexcept
			{
				return Extent * sizeof(element_type);
			}

			MUU_CONST_INLINE_GETTER
			constexpr bool empty() const noexcept
			{
				return false;
			}
		};

		// span storage base - 0 extent
		template <typename T, size_t Alignment>
		class MUU_TRIVIAL_ABI span_storage<T, 0, Alignment>
		{
		  public:
			using element_type = T;
			using pointer	   = std::add_pointer_t<T>;

		  protected:
			pointer ptr_ = {};

		  public:
			constexpr span_storage() noexcept								= default;
			constexpr span_storage(const span_storage&) noexcept			= default;
			constexpr span_storage& operator=(const span_storage&) noexcept = default;

			constexpr span_storage(pointer p, size_t) noexcept //
				: ptr_{ p }
			{
				if constexpr (Alignment > alignof(T))
					MUU_CONSTEXPR_SAFE_ASSERT(reinterpret_cast<uintptr_t>(ptr_) % Alignment == 0u
											  && "pointer alignment must be >= span's Alignment parameter");
			}

			MUU_CONST_INLINE_GETTER
			constexpr size_t size() const noexcept
			{
				return 0;
			}

			MUU_CONST_INLINE_GETTER
			constexpr size_t size_bytes() const noexcept
			{
				return 0;
			}

			MUU_CONST_INLINE_GETTER
			constexpr bool empty() const noexcept
			{
				return true;
			}
		};

		// span storage base - dynamic extent
		template <typename T, size_t Alignment>
		class MUU_TRIVIAL_ABI span_storage<T, dynamic_extent, Alignment>
		{
		  public:
			using element_type = T;
			using pointer	   = std::add_pointer_t<T>;

		  protected:
			pointer ptr_   = {};
			size_t extent_ = {};

		  public:
			constexpr span_storage() noexcept								= default;
			constexpr span_storage(const span_storage&) noexcept			= default;
			constexpr span_storage& operator=(const span_storage&) noexcept = default;

			constexpr span_storage(pointer p, size_t e) noexcept //
				: ptr_{ p },
				  extent_{ e }
			{
				MUU_CONSTEXPR_SAFE_ASSERT((extent_ == 0u || ptr_) && "a nullptr span is undefined behaviour");

				if constexpr (Alignment > alignof(T))
					MUU_CONSTEXPR_SAFE_ASSERT(reinterpret_cast<uintptr_t>(ptr_) % Alignment == 0u
											  && "pointer alignment must be >= span's Alignment parameter");
			}

			MUU_PURE_INLINE_GETTER
			constexpr size_t size() const noexcept
			{
				return extent_;
			}

			MUU_PURE_INLINE_GETTER
			constexpr size_t size_bytes() const noexcept
			{
				return extent_ * sizeof(element_type);
			}

			MUU_PURE_INLINE_GETTER
			constexpr bool empty() const noexcept
			{
				return !extent_;
			}
		};

		// span constructor base - static extents
		template <typename T, size_t Extent, size_t Alignment>
		class MUU_TRIVIAL_ABI span_constructors : public span_storage<T, Extent, Alignment>
		{
		  private:
			using base = span_storage<T, Extent, Alignment>;

		  public:
			using element_type = T;
			using pointer	   = std::add_pointer_t<T>;

			using span_storage<T, Extent, Alignment>::span_storage; // bring base class constructors into scope

			MUU_NODISCARD_CTOR
			constexpr span_constructors() noexcept = default;
			MUU_NODISCARD_CTOR
			constexpr span_constructors(const span_constructors&) noexcept			  = default;
			constexpr span_constructors& operator=(const span_constructors&) noexcept = default;

			// `explicit` when #extent != #dynamic_extent
			MUU_CONSTRAINED_TEMPLATE((is_qualifier_conversion_only<iter_reference_t<It>, element_type> //
									  && MUU_STD_CONCEPT(std::contiguous_iterator<It>)),			   //
									 typename It)
			MUU_NODISCARD_CTOR
			explicit constexpr span_constructors(It first, size_t count) noexcept //
				: base{ pointer_cast<pointer>(muu::to_address(first)), count }
			{
				MUU_CONSTEXPR_SAFE_ASSERT(count == Extent
										  && "count must be equal to span extent for statically-sized spans");
			}

			// `explicit` when #extent != #dynamic_extent
			MUU_CONSTRAINED_TEMPLATE((is_qualifier_conversion_only<iter_reference_t<It>, element_type> //
									  && !is_implicitly_convertible<End, size_t>					   //
									  && MUU_STD_CONCEPT(std::contiguous_iterator<It>)				   //
									  && MUU_STD_CONCEPT(std::sized_sentinel_for<End, It>)),
									 typename It,
									 typename End)
			MUU_NODISCARD_CTOR
			explicit constexpr span_constructors(It first, End last) noexcept(noexcept(last - first))
				: base{ pointer_cast<pointer>(muu::to_address(first)), static_cast<size_t>(last - first) }
			{
				MUU_CONSTEXPR_SAFE_ASSERT(static_cast<size_t>(last - first) == base::size()
										  && "(last - first) must be equal to span extent for statically-sized spans");
			}

			// `explicit` when #extent != #dynamic_extent && E == #dynamic_extent.
			MUU_CONSTRAINED_TEMPLATE((E == dynamic_extent									   //
									  && impl::is_qualifier_conversion_only<U, element_type>), //
									 typename U,
									 size_t E,
									 size_t A,
									 size_t Extent_ = Extent)
			MUU_NODISCARD_CTOR
			explicit constexpr span_constructors(const span<U, E, A>& s) noexcept
				: base{ pointer_cast<pointer>(s.data()), s.size() }
			{}

			// `explicit` when #extent != #dynamic_extent && E == #dynamic_extent.
			MUU_CONSTRAINED_TEMPLATE((E != dynamic_extent									   //
									  && impl::is_qualifier_conversion_only<U, element_type>), //
									 typename U,
									 size_t E,
									 size_t A,
									 size_t Extent_ = Extent)
			MUU_NODISCARD_CTOR
			/* implicit */ constexpr span_constructors(const span<U, E, A>& s) noexcept
				: base{ pointer_cast<pointer>(s.data()), s.size() }
			{}
		};

		// span constructor base - dynamic extents
		template <typename T, size_t Alignment>
		class MUU_TRIVIAL_ABI span_constructors<T, dynamic_extent, Alignment>
			: public span_storage<T, dynamic_extent, Alignment>
		{
		  private:
			using base = span_storage<T, dynamic_extent, Alignment>;

		  public:
			using element_type = T;
			using pointer	   = std::add_pointer_t<T>;

			using span_storage<T, dynamic_extent, Alignment>::span_storage; // bring base class constructors into scope

			MUU_NODISCARD_CTOR
			constexpr span_constructors() noexcept = default;
			MUU_NODISCARD_CTOR
			constexpr span_constructors(const span_constructors&) noexcept			  = default;
			constexpr span_constructors& operator=(const span_constructors&) noexcept = default;

			// `explicit` when #extent != #dynamic_extent
			MUU_CONSTRAINED_TEMPLATE((is_qualifier_conversion_only<iter_reference_t<It>, element_type> //
									  && MUU_STD_CONCEPT(std::contiguous_iterator<It>)),			   //
									 typename It)
			MUU_NODISCARD_CTOR
			/* implicit */ constexpr span_constructors(It first, size_t count) noexcept //
				: base{ pointer_cast<pointer>(muu::to_address(first)), count }
			{}

			// `explicit` when #extent != #dynamic_extent
			MUU_CONSTRAINED_TEMPLATE((is_qualifier_conversion_only<iter_reference_t<It>, element_type> //
									  && !is_implicitly_convertible<End, size_t>					   //
									  && MUU_STD_CONCEPT(std::contiguous_iterator<It>)				   //
									  && MUU_STD_CONCEPT(std::sized_sentinel_for<End, It>)),
									 typename It,
									 typename End)
			MUU_NODISCARD_CTOR
			/* implicit */ constexpr span_constructors(It first, End last) noexcept(noexcept(last - first))
				: base{ pointer_cast<pointer>(muu::to_address(first)), static_cast<size_t>(last - first) }
			{}

			// `explicit` when #extent != #dynamic_extent && E == #dynamic_extent.
			MUU_CONSTRAINED_TEMPLATE((impl::is_qualifier_conversion_only<U, element_type>), //
									 typename U,
									 size_t E,
									 size_t A)
			MUU_NODISCARD_CTOR
			/* implicit */ constexpr span_constructors(const span<U, E, A>& s) noexcept
				: base{ pointer_cast<pointer>(s.data()), s.size() }
			{}
		};
	}
	/// \endcond

	/// \brief	A non-owning view of contiguous elements.
	/// \ingroup	core mem
	///
	/// \remarks This is equivalent to C++20's std::span.
	///
	/// \tparam	T	  		The span's element type.
	/// \tparam	Extent		The number of elements represented by the span.
	/// 					Use muu::dynamic_extent for dynamically-sized spans.
	/// \tparam	Alignment	The minimum byte alignment of the base data pointer held by the span.
	///						Must be a power-of-two and >= `alignof(T)`.
	///						A value of `0` is accepted as `alignof(T)`.
	template <typename T, //
			  size_t Extent MUU_DOXYGEN_ONLY(= dynamic_extent),
			  size_t Alignment MUU_DOXYGEN_ONLY(= 0)>
	class MUU_TRIVIAL_ABI MUU_EMPTY_BASES
		span MUU_HIDDEN_BASE(public impl::span_constructors<T, Extent, Alignment ? Alignment : alignof(T)>)
	{
		static_assert(!std::is_reference_v<T>, "Spans cannot store references");
		static_assert(!std::is_function_v<T>, "Spans cannot store functions");
		static_assert(!Alignment || (Alignment & (Alignment - 1u)) == 0u, "Alignment must be a power of two");
		static_assert(!Alignment || Alignment >= alignof(T), "Cannot under-align types.");

	  public:
		/// \brief The span's element type.
		using element_type = T;

		/// \brief The span's base value type.
		using value_type = std::remove_cv_t<T>;

		/// \brief std::size_t
		using size_type = size_t;

		/// \brief std::ptrdiff_t
		using difference_type = ptrdiff_t;

		/// \brief A pointer to the span's element type.
		using pointer = std::add_pointer_t<T>;

		/// \brief A const pointer to the span's element type.
		using const_pointer = std::add_pointer_t<std::add_const_t<T>>;

		/// \brief A reference to the span's element type.
		using reference = std::add_lvalue_reference_t<T>;

		/// \brief A const reference to the span's element type.
		using const_reference = std::add_lvalue_reference_t<std::add_const_t<T>>;

		/// \brief A LegacyRandomAccessIterator for the elements in the span.
		using iterator = pointer;

		/// \brief std::reverse_iterator<iterator>
		using reverse_iterator = std::reverse_iterator<iterator>;

		/// \brief The number of elements in the span, or #muu::dynamic_extent.
		static constexpr size_t extent = Extent;

		/// \brief The minimum alignment of the base data pointer held by this span.
		static constexpr size_t alignment = Alignment ? Alignment : alignof(T);

	  private:
		using base = impl::span_constructors<T, Extent, alignment>;

	  public:
		/// \cond

		using impl::span_constructors<T, Extent, alignment>::span_constructors; // bring base constructors into scope

		/// \endcond

#if MUU_DOXYGEN

		/// \brief Default constructor.
		/// \availability This constructor is not available for statically-sized spans.
		constexpr span() noexcept;

		/// \brief Constructs a span from a contiguous iterator and an element count.
		/// \note This constructor is `explicit` when #extent != #dynamic_extent.
		template <typename It>
		explicit constexpr span(It first, size_t count) noexcept;

		/// \brief Constructs a span from a pair of contiguous iterators.
		/// \note This constructor is `explicit` when #extent != #dynamic_extent.
		template <typename It, typename End>
		explicit constexpr span(It first, End last) noexcept;

		/// \brief Constructs a span from another span.
		/// \note This constructor is `explicit` when #extent != #dynamic_extent && E == #dynamic_extent.
		template <typename U, size_t E, size_t A>
		explicit constexpr span(const span<U, E, A>& s) noexcept;

		/// \brief Returns the number of elements covered by the span.
		constexpr size_t size() const noexcept;

		/// \brief Returns the total size of the elements covered by the span in bytes.
		constexpr size_t size_bytes() const noexcept;

		/// \brief Returns true if the span is empty (i.e. covers zero elements).
		constexpr bool empty() const noexcept;

#endif

		/// \brief Constructs a span from an array.
		MUU_CONSTRAINED_TEMPLATE((Extent == dynamic_extent || N == Extent), size_t N)
		MUU_NODISCARD_CTOR
		constexpr span(MUU_TYPE_IDENTITY(element_type) (&arr)[N]) noexcept //
			: base{ arr, N }
		{}

		/// \brief Constructs a span from an array.
		MUU_CONSTRAINED_TEMPLATE((impl::is_qualifier_conversion_only<U, element_type> //
								  && (Extent == dynamic_extent || N == Extent)),	  //
								 typename U,
								 size_t N)
		MUU_NODISCARD_CTOR
		constexpr span(std::array<U, N>& arr) noexcept //
			: base{ pointer_cast<pointer>(arr.data()), N }
		{}

		/// \brief Constructs a span from an array.
		MUU_CONSTRAINED_TEMPLATE((impl::is_qualifier_conversion_only<std::add_const_t<U>, element_type> //
								  && (Extent == dynamic_extent || N == Extent)),						//
								 typename U,
								 size_t N)
		MUU_NODISCARD_CTOR
		constexpr span(const std::array<U, N>& arr) noexcept //
			: base{ pointer_cast<pointer>(arr.data()), N }
		{}

		/// \brief Copy constructor.
		constexpr span(const span&) noexcept = default;

		/// \brief Copy-assignment operator.
		constexpr span& operator=(const span&) noexcept = default;

		/// \name Elements
		/// @{

		/// \brief Returns a pointer to the first element in the span.
		MUU_PURE_INLINE_GETTER
		MUU_ATTR(assume_aligned(alignment))
		constexpr MUU_IF_DOXYGEN(pointer, T*) data() const noexcept
		{
			return muu::assume_aligned<alignment>(base::ptr_);
		}

		/// \brief Returns a reference to an arbitrary element in the span.
		MUU_PURE_INLINE_GETTER
		constexpr reference operator[](size_t idx) const noexcept
		{
			return data()[idx];
		}

		/// \brief Returns a reference to the first element in the span.
		MUU_PURE_INLINE_GETTER
		constexpr reference front() const noexcept
		{
			return *data();
		}

		/// \brief Returns a reference to the last element in the span.
		MUU_PURE_INLINE_GETTER
		constexpr reference back() const noexcept
		{
			return data()[base::size() - 1u];
		}

		/// @}

		/// \name Iterators
		/// @{

		/// \brief Returns an iterator to the beginning of the span.
		MUU_PURE_INLINE_GETTER
		MUU_ATTR(assume_aligned(alignment))
		constexpr MUU_IF_DOXYGEN(iterator, T*) begin() const noexcept
		{
			return data();
		}

		/// \brief Returns an iterator to end of the span.
		MUU_PURE_INLINE_GETTER
		constexpr iterator end() const noexcept
		{
			return begin() + base::size();
		}

		/// \brief Returns a reverse iterator to the beginning of the span.
		MUU_PURE_INLINE_GETTER
		constexpr reverse_iterator rbegin() const noexcept
		{
			return reverse_iterator{ end() };
		}

		/// \brief Returns a reverse iterator to the end of the span.
		MUU_PURE_INLINE_GETTER
		constexpr reverse_iterator rend() const noexcept
		{
			return reverse_iterator{ begin() };
		}

		/// @}

		/// \name Iterators (ADL)
		/// @{

		/// \brief Returns an iterator to the beginning of a span.
		MUU_PURE_INLINE_GETTER
		MUU_ATTR(assume_aligned(alignment))
		friend constexpr MUU_IF_DOXYGEN(iterator, T*) begin(const span& s) noexcept
		{
			return s.begin();
		}

		/// \brief Returns an iterator to end of a span.
		MUU_PURE_INLINE_GETTER
		friend constexpr iterator end(const span& s) noexcept
		{
			return s.end();
		}

		/// \brief Returns a reverse iterator to the beginning of a span.
		MUU_PURE_INLINE_GETTER
		friend constexpr reverse_iterator rbegin(const span& s) noexcept
		{
			return s.rbegin();
		}

		/// \brief Returns a reverse iterator to the end of a span.
		MUU_PURE_INLINE_GETTER
		friend constexpr reverse_iterator rend(const span& s) noexcept
		{
			return s.rend();
		}

		/// @}

		/// \name Subspans
		/// @{

		/// \brief Returns a subspan representing the first N elements of this span.
		template <size_t Count>
		MUU_PURE_GETTER
		constexpr span<element_type, Count, Alignment> first() const noexcept
		{
			static_assert(Count <= Extent, "span::first() count cannot exceed the size of the source span");
			MUU_CONSTEXPR_SAFE_ASSERT(Count <= base::size());

			return span<element_type, Count, Alignment>{ data(), Count };
		}

		/// \brief Returns a subspan representing the first N elements of this span.
		MUU_PURE_GETTER
		constexpr span<element_type, dynamic_extent, Alignment> first(size_t count) const noexcept
		{
			MUU_CONSTEXPR_SAFE_ASSERT(count <= base::size());

			return span<element_type, dynamic_extent, Alignment>{ data(), count };
		}

		/// \brief Returns a subspan representing the last N elements of this span.
		template <size_t Count>
		MUU_PURE_GETTER
		constexpr span<element_type, Count> last() const noexcept
		{
			static_assert(Count <= Extent, "span::last() count cannot exceed the size of the source span");
			MUU_CONSTEXPR_SAFE_ASSERT(Count <= base::size());

			return span<element_type, Count>{ data() + (base::size() - Count), Count };
		}

		/// \brief Returns a subspan representing the last N elements of this span.
		MUU_PURE_GETTER
		constexpr span<element_type> last(size_t count) const noexcept
		{
			MUU_CONSTEXPR_SAFE_ASSERT(count <= base::size());

			return span<element_type>{ data() + (base::size() - count), count };
		}

		/// \brief Returns an arbitrary subspan.
		template <size_t Offset, size_t Count = dynamic_extent>
		MUU_PURE_GETTER
		constexpr auto subspan() const noexcept
		{
			static_assert(Offset < Extent, "span::subspan() Offset cannot exceed the size of the source span");
			static_assert(Count == dynamic_extent || Count <= (Extent - Offset),
						  "span::subspan() Count cannot exceed the size of the source span");

			MUU_CONSTEXPR_SAFE_ASSERT(Offset < base::size());

			if constexpr (Count == dynamic_extent)
			{
				return span<element_type>{ data() + Offset, base::size() - Offset };
			}
			else
			{
				MUU_CONSTEXPR_SAFE_ASSERT(Count <= (base::size() - Offset));
				return span<element_type, Count>{ data() + Offset, Count };
			}
		}

		/// \brief Returns an arbitrary subspan.
		MUU_PURE_GETTER
		constexpr span<element_type> subspan(size_t offset, size_t count = dynamic_extent) const noexcept
		{
			MUU_CONSTEXPR_SAFE_ASSERT(offset < base::size());

			if (count == dynamic_extent)
			{
				return span<element_type>{ data() + offset, base::size() - offset };
			}
			else
			{
				MUU_CONSTEXPR_SAFE_ASSERT(count <= (base::size() - offset));
				return span<element_type>{ data() + offset, count };
			}
		}

		/// @}
	};

	/// \cond

	template <typename T>
	span(T*, size_t) -> span<T>;
	template <typename T, size_t N>
	span(T (&)[N]) -> span<T, N>;
	template <typename T, size_t N>
	span(T (&&)[N]) -> span<T, N>;
	template <typename T, size_t N>
	span(std::array<T, N>&) -> span<T, N>;
	template <typename T, size_t N>
	span(const std::array<T, N>&) -> span<const T, N>;

#if MUU_CONCEPTS
	template <std::contiguous_iterator It, typename E>
	span(It, E) -> span<impl::iter_value_t<It>>;
#else
	template <typename It, typename E>
	span(It, E) -> span<impl::iter_value_t<It>>;
	template <typename It>
	span(It, It) -> span<impl::iter_value_t<It>>;
#endif

	/// \endcond

	/// \brief	Convenience alias for `span<const T, Extent>`.
	/// \ingroup	memory
	template <typename T, //
			  size_t Extent MUU_DOXYGEN_ONLY(= dynamic_extent),
			  size_t Alignment MUU_DOXYGEN_ONLY(= 0)>
	using const_span = span<const T, Extent, Alignment>;

	/// \brief	Convenience alias for `span<std::byte>`.
	/// \ingroup	memory
	using byte_span = span<std::byte>;

	/// \brief	Convenience alias for `span<const std::byte>`.
	/// \ingroup	memory
	using const_byte_span = span<const std::byte>;

	/// \brief	Convenience alias for `span<std::byte, dynamic_extent, Alignment>`.
	/// \ingroup	memory
	template <size_t Alignment>
	using aligned_byte_span = span<std::byte, dynamic_extent, Alignment>;

	/// \brief	Reinterprets a span as an immutable view of the underlying bytes.
	/// \ingroup	memory
	/// \relatesalso	muu::span
	///
	/// \details Equivalent to C++20's std::as_bytes.
	template <typename T, size_t E, size_t A>
	MUU_NODISCARD
	span<const std::byte, impl::as_bytes_extent<T, E>> as_bytes(span<T, E, A> s) noexcept
	{
		if constexpr (E == dynamic_extent)
		{
			return span<const std::byte>{ pointer_cast<const std::byte*>(s.data()), s.size_bytes() };
		}
		else
		{
			return span<const std::byte, sizeof(T) * E>{ pointer_cast<const std::byte*>(s.data()), sizeof(T) * E };
		}
	}

	/// \brief	Reinterprets a span as a view of the underlying bytes.
	/// \ingroup	memory
	/// \relatesalso	muu::span
	///
	/// \details Equivalent to C++20's std::as_writable_bytes.
	MUU_CONSTRAINED_TEMPLATE(!std::is_const_v<T>, typename T, size_t E, size_t A)
	MUU_NODISCARD
	span<std::byte, impl::as_bytes_extent<T, E>> as_writable_bytes(span<T, E, A> s) noexcept
	{
		if constexpr (E == dynamic_extent)
		{
			return span<std::byte>{ pointer_cast<std::byte*>(s.data()), s.size_bytes() };
		}
		else
		{
			return span<std::byte, sizeof(T) * E>{ pointer_cast<std::byte*>(s.data()), sizeof(T) * E };
		}
	}
}

#include "impl/header_end.h"
