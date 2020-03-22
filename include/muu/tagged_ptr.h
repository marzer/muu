//# This file is a part of muu and is subject to the the terms of the MIT license.
//# Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
//# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.

#pragma once
#include "../muu/common.h"

namespace muu::impl
{
	inline constexpr size_t tptr_addr_highest_used_bit = MUU_ARCH_AMD64 ? 47 : build::pointer_bits - 1;
	inline constexpr size_t tptr_addr_used_bits = tptr_addr_highest_used_bit + 1;
	inline constexpr size_t tptr_addr_free_bits = build::pointer_bits - tptr_addr_used_bits;

	template <typename T>
	inline constexpr size_t tptr_default_align = alignof(T);
	template <>
	inline constexpr size_t tptr_default_align<void> = 1;
	template <typename T>
	inline constexpr size_t tptr_default_align<const T> = tptr_default_align<T>;
	template <typename T>
	inline constexpr size_t tptr_default_align<volatile T> = tptr_default_align<T>;
	template <typename T>
	inline constexpr size_t tptr_default_align<const volatile T> = tptr_default_align<T>;
	template <typename R, typename ...P>
	inline constexpr size_t tptr_default_align<R(P...)> = 1_sz;
	template <typename R, typename ...P>
	inline constexpr size_t tptr_default_align<R(P...) noexcept> = tptr_default_align<R(P...)>;

	template <size_t min_align>
	struct tptr final
	{
		static constexpr size_t tag_bits = (muu::max(bit_width(min_align), 1_sz) - 1_sz) + impl::tptr_addr_free_bits;
		static constexpr uintptr_t tag_mask = bit_fill_right<uintptr_t>(tag_bits);
		static constexpr uintptr_t ptr_mask = ~tag_mask;

		using tag_type =
			std::conditional_t<(tag_bits <= 8_sz), uint8_t,
			std::conditional_t<(tag_bits <= 16_sz), uint16_t,
			std::conditional_t<(tag_bits <= 32_sz), uint32_t,
			std::conditional_t<(tag_bits <= 64_sz), uint64_t,
			uintptr_t // catch-all
		>>>>;
		static_assert(sizeof(tag_type) <= sizeof(uintptr_t));

		[[nodiscard]] MUU_ALWAYS_INLINE
		static constexpr uintptr_t pack_ptr(const volatile void* ptr) noexcept
		{
			MUU_ASSERT(
				(!ptr || bit_floor(pointer_cast<uintptr_t>(ptr)) >= min_align)
				&& "The pointer's address is more strictly aligned than min_align"
			);

			if constexpr (tptr_addr_free_bits > 0)
				return (pointer_cast<uintptr_t>(ptr) << tptr_addr_free_bits);
			else
				return pointer_cast<uintptr_t>(ptr);
		}

		template <typename T>
		[[nodiscard]] MUU_ALWAYS_INLINE
		static constexpr uintptr_t pack_both(const volatile void* ptr, const T& tag) noexcept
		{
			static_assert(
				std::is_trivially_copyable_v<T>,
				"The tag type must be trivially copyable"
			);

			MUU_ASSERT(
				(!ptr || bit_floor(pointer_cast<uintptr_t>(ptr)) >= min_align)
				&& "The pointer's address is more strictly aligned than min_align"
			);

			if constexpr (is_unsigned<T>)
			{
				if constexpr ((sizeof(T) * build::bits_per_byte) > tag_bits)
					return pack_ptr(ptr) | (static_cast<uintptr_t>(unbox(tag)) & tag_mask);
				else
					return pack_ptr(ptr) | static_cast<uintptr_t>(unbox(tag));
			}
			else //some pod type
			{
				static_assert(
					(sizeof(T) * build::bits_per_byte) <= tag_bits,
					"The tag type must fit in the available tag bits"
				);
				auto bits = pack_ptr(ptr);
				memcpy(&bits, &tag, sizeof(T));
				return bits;
			}
		}

		[[nodiscard]] MUU_ALWAYS_INLINE
		static constexpr uintptr_t set_ptr(uintptr_t bits, const volatile void* ptr) noexcept
		{
			return pack_ptr(ptr) | (bits & tag_mask);
		}

		template <typename T, typename = std::enable_if_t<is_unsigned<T>>>
		[[nodiscard]] MUU_ALWAYS_INLINE
		static constexpr uintptr_t set_tag(uintptr_t bits, T tag) noexcept
		{
			if constexpr ((sizeof(T) * build::bits_per_byte) > tag_bits)
				return (bits & ptr_mask) | (static_cast<uintptr_t>(tag) & tag_mask);
			else
				return (bits & ptr_mask) | static_cast<uintptr_t>(tag);
		}

		template <typename T, std::enable_if_t<!is_unsigned<T>>* = nullptr>
		[[nodiscard]]
		static uintptr_t set_tag(uintptr_t bits, const T& tag) noexcept
		{
			static_assert(
				std::is_trivially_copyable_v<T>,
				"The tag type must be trivially copyable"
			);
			static_assert(
				(sizeof(T) * build::bits_per_byte) <= tag_bits,
				"The tag type must fit in the available tag bits"
			);
			if constexpr ((sizeof(T) * build::bits_per_byte) < tag_bits)
				bits &= ptr_mask;
			memcpy(&bits, &tag, sizeof(T));
			return bits;
		}

		[[nodiscard]] MUU_ALWAYS_INLINE
		static constexpr uintptr_t get_tag(uintptr_t bits) noexcept
		{
			return bits & tag_mask;
		}

		template <typename T>
		[[nodiscard]] MUU_ALWAYS_INLINE
		static constexpr T get_tag_as(uintptr_t bits) noexcept
		{
			static_assert(
				std::is_trivially_copyable_v<T>,
				"The tag type must be trivially copyable"
			);
			static_assert(
				(sizeof(T) * build::bits_per_byte) <= tag_bits,
				"The tag type must fit in the available tag bits"
			);

			using intermediate_type =
				std::conditional_t<(sizeof(T) > 4), uint64_t,
				std::conditional_t<(sizeof(T) > 2), uint32_t,
				std::conditional_t<(sizeof(T) > 1), uint16_t,
				uint8_t
			>>>;
			static_assert(sizeof(intermediate_type) >= sizeof(T));
			static_assert(sizeof(intermediate_type) <= sizeof(tag_type));

			if constexpr (sizeof(T) == sizeof(intermediate_type))
				return bit_cast<T>(static_cast<intermediate_type>(get_tag(bits)));
			else
			{
				uint8_t bytes[sizeof(T)];
				auto intermediate_bits = static_cast<intermediate_type>(get_tag(bits));
				for (size_t i = 0; i < sizeof(T); i++)
				{
					bytes[i] = static_cast<uint8_t>(intermediate_bits & bit_fill_right<intermediate_type>(8));
					intermediate_bits >>= 8;
				}
				return bit_cast<T>(bytes);
			}
		}

		[[nodiscard]]
		static constexpr uintptr_t get_ptr(uintptr_t bits) noexcept
		{
			bits &= ptr_mask;
			if constexpr (tptr_addr_free_bits > 0)
			{
				bits >>= tptr_addr_free_bits;

				#if MUU_ARCH_AMD64
				{
					constexpr uintptr_t canon_test = uintptr_t{ 1u } << tptr_addr_highest_used_bit;
					if (bits & canon_test)
					{
						constexpr uintptr_t canon_mask = bit_fill_right<uintptr_t>(tptr_addr_free_bits) << tptr_addr_used_bits;
						bits |= canon_mask;
					}
				}
				#endif
			}
			return bits;
		}
	};
}

namespace muu
{
	/// \brief	A specialized pointer capable of packing 'tag' data into the unused bits of a pointer's value.
	///
	/// \tparam	T			The type being pointed to.
	/// \tparam	min_align	Minimum alignment of values stored in the tagged_ptr.
	/// 					Default is `alignof(T)`, but you may override it if you know
	/// 					you will only be storing values with larger alignments.
	template <typename T = void, size_t min_align = impl::tptr_default_align<T>>
	class MUU_TRIVIAL_ABI tagged_ptr final
	{
		static_assert(
			!std::is_reference_v<T>,
			"Tagged pointers cannot store references"
		);
		static_assert(
			min_align > 0 && has_single_bit(min_align),
			"Minimum alignment must be a power of two"
		);
		static_assert(
			std::is_function_v<T> //the default is not strictly the minimum for function pointers
			|| min_align >= impl::tptr_default_align<T>,
			"Minimum alignment cannot be smaller than the type's actual minimum alignment"
		);
		static_assert(
			min_align > 1 || impl::tptr_addr_free_bits > 0,
			"Types aligned on a single byte cannot be pointed to by a tagged pointer on this platform"
		);

		private:
			mutable uintptr_t bits = {};

			using tptr = impl::tptr<min_align>;

		public:

			/// \brief	The type being pointed to.
			using element_type = T;
			/// \brief	A pointer to element_type.
			using pointer = std::add_pointer_t<T>;
			/// \brief	A pointer to element_type (const-qualified).
			using const_pointer = constify_pointer<pointer>;
			/// \brief	An integer just large enough to store all the bits in the tag area.
			using tag_type = typename tptr::tag_type;

			/// \brief The minimum alignment of values stored in this pointer.
			static constexpr size_t minimum_alignment = min_align;
			/// \brief	The number of tag bits available.
			static constexpr size_t tag_bit_count = tptr::tag_bits;
			/// \brief	The largest integral value that can be stored in the available tag bits.
			static constexpr tag_type max_tag = bit_fill_right<tag_type>(tag_bit_count);

			/// \brief	Constructs a tagged pointer.
			///
			/// \param	value	The inital address of the pointer's target.
			/// \remarks Tag bits are initialized to zero.
			explicit constexpr tagged_ptr(pointer value) noexcept
				: bits{ tptr::pack_ptr(value) }
			{}

			/// \brief	Constructs a tagged pointer.
			///
			/// \tparam	U			An unsigned integral type, or a trivially-copyable type small enough to fit.
			/// \param	value		The inital address of the pointer's target.
			/// \param	tag			The initial value of the pointer's tag bits.
			///
			/// \warning If the tag parameter is an integer larger than the available tag bits,
			/// 		 any overflow will be masked out and ignored.
			template <typename U>
			constexpr tagged_ptr(pointer value, const U& tag) noexcept
				: bits{ tptr::pack_both(value, tag) }
			{
				static_assert(
					is_unsigned<U> || (std::is_trivially_copyable_v<U> && sizeof(U) * build::bits_per_byte <= tag_bit_count),
					"Tag types must be unsigned integrals or trivially-copyable and small enough to fit in the available tag bits"
				);
			}

			/// \brief	Constructs zero-initialized tagged pointer.
			constexpr tagged_ptr(nullptr_t) noexcept
			{}

			/// \brief	Constructs zero-initialized tagged pointer.
			tagged_ptr() noexcept = default;

			/// \brief	Copy constructor.
			tagged_ptr(const tagged_ptr&) noexcept = default;

			/// \brief	Copy-assignment operator.
			tagged_ptr& operator= (const tagged_ptr&) noexcept = default;

			/// \brief	Destructor.
			~tagged_ptr() noexcept = default;

			/// \brief	Sets the target pointer value and all tag bits to zero.
			///
			/// \returns	A reference to the tagged_ptr.
			MUU_ALWAYS_INLINE
			constexpr tagged_ptr& reset() noexcept
			{
				bits = {};
				return *this;
			}

			/// \brief	Resets the target pointer value and sets the tag bits to zero.
			///
			/// \param	value	The new target pointer value.
			/// 
			/// \returns	A reference to the tagged_ptr.
			constexpr tagged_ptr& reset(pointer value) noexcept
			{
				bits = tptr::pack_ptr(value);
				return *this;
			}

			/// \brief	Resets the target pointer value and tag.
			///
			/// \tparam	U		An unsigned integral type, or a trivially-copyable type small enough to fit.
			/// \param	value	The new target pointer value.
			/// \param	tag		The new tag.
			///
			/// \warning If the tag parameter is an integer larger than the available tag bits,
			/// 		 any overflow will be masked out and ignored.
			/// 
			/// \returns	A reference to the tagged_ptr.
			template <typename U>
			constexpr tagged_ptr& reset(pointer value, const U& tag) noexcept
			{
				static_assert(
					is_unsigned<U> || (std::is_trivially_copyable_v<U> && sizeof(U) * build::bits_per_byte <= tag_bit_count),
					"Tag types must be unsigned integrals or trivially-copyable and small enough to fit in the available tag bits"
				);

				bits = tptr::pack_both(value, tag);
				return *this;
			}

			/// \brief	Returns the target pointer value.
			[[nodiscard]]
			constexpr pointer ptr() const noexcept
			{
				return pointer_cast<pointer>(tptr::get_ptr(bits));
			}

			/// \brief	Sets the target pointer value, leaving the tag bits unchanged.
			///
			/// \param 	value	The new target pointer value.
			///
			/// \returns	A reference to a tagged_ptr.
			constexpr tagged_ptr& ptr(pointer value) noexcept
			{
				bits = tptr::set_ptr(bits, value);
				return *this;
			}

			/// \brief	Sets the target pointer value, leaving the tag bits unchanged.
			///
			/// \param	rhs	The new target pointer value.
			///
			/// \returns	A reference to the tagged_ptr.
			constexpr tagged_ptr& operator = (pointer rhs) noexcept
			{
				return ptr(rhs);
			}

			/// \brief	Sets the target pointer value to nullptr, leaving the tag bits unchanged.
			///
			/// \returns	A reference to the tagged_ptr.
			MUU_ALWAYS_INLINE
			constexpr tagged_ptr& clear_ptr() noexcept
			{
				bits &= tptr::tag_mask;
				return *this;
			}

			/// \brief	Returns the target pointer value.
			/// 
			/// \remarks This is an alias for ptr(); it exists to keep the interface consistent with std::unique_ptr.
			[[nodiscard]] MUU_ALWAYS_INLINE
			constexpr pointer get() const noexcept
			{
				return ptr();
			}

			/// \brief	Returns the tag bits as an unsigned integer or trivially-copyable type.
			template <typename U = tag_type>
			[[nodiscard]]
			constexpr U tag() const noexcept
			{
				static_assert(
					!std::is_reference_v<U>,
					"Reference types are not allowed"
				);
				static_assert(
					std::is_trivially_copyable_v<U>,
					"Tag types must be trivially-copyable"
				);

				if constexpr (is_unsigned<U>)
				{
					static_assert(
						sizeof(U) >= sizeof(tag_type),
						"The destination integer type is not large enough to store the tag without a loss of data"
					);
					return static_cast<U>(tptr::get_tag(bits));
				}
				else
				{
					static_assert(
						(sizeof(U) * build::bits_per_byte) <= tag_bit_count,
						"Tag types must fit in the available tag bits"
					);

					return tptr::template get_tag_as<U>(bits);
				}
			}

			/// \brief	Sets the tag bits, leaving the target pointer value unchanged.
			///
			/// \tparam	U	An unsigned integral type, or a trivially-copyable type small enough to fit.
			/// \param	tag	The new value to set for the pointer's tag bits.
			///
			/// \warning If the tag parameter is an integer larger than the available tag bits,
			/// 		 any overflow will be masked out and ignored.
			/// 
			/// \returns	A reference to the tagged_ptr.
			template <typename U>
			constexpr tagged_ptr& tag(const U& tag) noexcept
			{
				static_assert(
					is_unsigned<U> || (std::is_trivially_copyable_v<U> && sizeof(U) * build::bits_per_byte <= tag_bit_count),
					"Tag types must be unsigned integrals or trivially-copyable and small enough to fit in the available tag bits"
				);

				bits = tptr::set_tag(bits, tag);
				return *this;
			}

			/// \brief	Sets the tag bits to zero, leaving the target pointer value unchanged.
			///
			/// \returns	A reference to the tagged_ptr.
			MUU_ALWAYS_INLINE
			constexpr tagged_ptr& clear_tag() noexcept
			{
				bits &= tptr::ptr_mask;
				return *this;
			}

			/// \brief	Returns the target pointer value.
			[[nodiscard]] MUU_ALWAYS_INLINE
			explicit constexpr operator pointer () const noexcept
			{
				return ptr();
			}

			/// \brief	Returns true if the target pointer value is not nullptr.
			[[nodiscard]] MUU_ALWAYS_INLINE
			explicit constexpr operator bool() const noexcept
			{
				return bits & tptr::ptr_mask;
			}

			/// \brief	Returns a reference the pointed object.
			/// 
			/// \remarks This operator is not available for pointers to void or functions.
			template <typename U = element_type, typename = std::enable_if_t<
				!std::is_void_v<U>
				&& !std::is_function_v<U>
			>>
			[[nodiscard]] MUU_ALWAYS_INLINE
			constexpr U& operator * () const noexcept
			{
				return *ptr();
			}

			/// \brief	Returns the target pointer value.
			/// 
			/// \remarks This operator is not available for pointers to void or functions.
			template <typename U = element_type, typename = std::enable_if_t<
				!std::is_void_v<U>
				&& !std::is_function_v<U>
			>>
			[[nodiscard]] MUU_ALWAYS_INLINE
			constexpr pointer operator -> () const noexcept
			{
				return ptr();
			}

			/// \brief	Invokes the function call operator on the pointed function.
			///
			/// \tparam U		Argument types.
			/// \param	args	The arguments to pass to the pointed function.
			///
			/// \returns	The return value of the function call.
			/// 
			/// \remarks This operator is only available when the pointed value is a function.
			template <typename... U, typename V = element_type, typename = std::enable_if_t<std::is_function_v<V>>>
			constexpr decltype(auto) operator () (U&&... args) const
				noexcept(std::is_nothrow_invocable_v<V, U&&...>)
			{
				static_assert(std::is_invocable_v<V, U&&...>);

				return ptr()(std::forward<U>(args)...);
			}

			/// \brief	Returns true if a tagged_ptr and raw pointer refer to the same address.
			[[nodiscard]]
			friend constexpr bool operator == (tagged_ptr lhs, const_pointer rhs) noexcept
			{
				return lhs.ptr() == rhs;
			}

			/// \brief	Returns true if a tagged_ptr and raw pointer do not refer to the same address.
			[[nodiscard]]
			friend constexpr bool operator != (tagged_ptr lhs, const_pointer rhs) noexcept
			{
				return lhs.ptr() != rhs;
			}

			/// \brief	Returns true if a tagged_ptr and raw pointer refer to the same address.
			[[nodiscard]]
			friend constexpr bool operator == (const_pointer lhs, tagged_ptr rhs) noexcept
			{
				return lhs == rhs.ptr();
			}

			/// \brief	Returns true if a tagged_ptr and raw pointer do not refer to the same address.
			[[nodiscard]]
			friend constexpr bool operator != (const_pointer lhs, tagged_ptr rhs) noexcept
			{
				return lhs != rhs.ptr();
			}

			/// \brief	Returns true if two tagged_ptrs are equal (including their tag bits).
			[[nodiscard]] MUU_ALWAYS_INLINE
			friend constexpr bool operator == (tagged_ptr lhs, tagged_ptr rhs) noexcept
			{
				return lhs.bits == rhs.bits;
			}

			/// \brief	Returns true if two tagged_ptrs are not equal (including their tag bits).
			[[nodiscard]] MUU_ALWAYS_INLINE
			friend constexpr bool operator != (tagged_ptr lhs, tagged_ptr rhs) noexcept
			{
				return lhs.bits != rhs.bits;
			}
	};

	template <typename T>
	tagged_ptr(T*&&) -> tagged_ptr<T>;
	template <typename T>
	tagged_ptr(T*&) -> tagged_ptr<T>;
	template <typename T>
	tagged_ptr(T*) -> tagged_ptr<T>;
}

namespace std
{
	template <typename T, size_t min_align>
	struct pointer_traits<muu::tagged_ptr<T, min_align>>
	{
		using pointer = muu::tagged_ptr<T, min_align>;
		using element_type = T;
		using difference_type = ptrdiff_t;
		template <typename U>
		using rebind = muu::tagged_ptr<U, min_align>;

		static pointer pointer_to(element_type& r) noexcept
		{
			return pointer{ &r };
		}

		static element_type* to_address(pointer p) noexcept
		{
			return p.ptr();
		}
	};
}



