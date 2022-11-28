// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::tagged_ptr.

#include "core.h"
#include "bit.h"
#include "assume_aligned.h"
#include "impl/std_memcpy.h"
#include "impl/std_memory.h" // pointer_traits
#include "impl/header_start.h"
MUU_DISABLE_SUGGEST_WARNINGS;
MUU_FORCE_NDEBUG_OPTIMIZATIONS;

//#=====================================================================================================================
//# IMPLEMENTATION DETAILS
//#=====================================================================================================================
//% tagged_ptr::impl start
/// \cond

namespace muu::impl
{
	template <typename To = uintptr_t, typename From>
	MUU_CONST_INLINE_GETTER
	constexpr To uintptr_cast(From ptr) noexcept
	{
		static_assert(!is_cvref<From> && !is_cvref<To>);
		static_assert(std::is_same_v<From, uintptr_t> || std::is_same_v<To, uintptr_t>);
		static_assert(std::is_pointer_v<From> || std::is_pointer_v<To>);

		MUU_IF_CONSTEVAL
		{
			return muu::bit_cast<To>(ptr);
		}
		else
		{
			return reinterpret_cast<To>(ptr);
		}
	}

	inline constexpr size_t tptr_addr_highest_used_bit = MUU_ARCH_AMD64 ? 47 : (sizeof(uintptr_t) * CHAR_BIT - 1);
	inline constexpr size_t tptr_addr_used_bits		   = tptr_addr_highest_used_bit + 1;
	inline constexpr size_t tptr_addr_free_bits		   = sizeof(uintptr_t) * CHAR_BIT - tptr_addr_used_bits;
	inline constexpr size_t tptr_addr_free_bits_mask   = bit_fill_right<uintptr_t>(tptr_addr_free_bits);

	template <typename T>
	struct tptr_uint
	{
		using type = T;
	};

	template <size_t Bits>
	constexpr auto tptr_uint_for_bits_impl() noexcept
	{
		static_assert(Bits <= sizeof(uintptr_t) * CHAR_BIT);

		if constexpr (Bits <= sizeof(unsigned char) * CHAR_BIT)
			return tptr_uint<unsigned char>{};
		else if constexpr (Bits <= sizeof(unsigned short) * CHAR_BIT)
			return tptr_uint<unsigned short>{};
		else if constexpr (Bits <= sizeof(unsigned int) * CHAR_BIT)
			return tptr_uint<unsigned int>{};
		else if constexpr (Bits <= sizeof(unsigned long) * CHAR_BIT)
			return tptr_uint<unsigned long>{};
		else if constexpr (Bits <= sizeof(unsigned long long) * CHAR_BIT)
			return tptr_uint<unsigned long long>{};
	}

	template <size_t Bits>
	using tptr_uint_for_bits = typename decltype(tptr_uint_for_bits_impl<Bits>())::type;

	// Q: "what is tptr?"
	//
	// A: it's essentially a just templated namespace. everything is static and dependent on Align.
	//    the reasoning being that the pointer type doesn't actually matter for all the different instantiations, only
	//    the alignment + architecture, so setting it out this way greatly reduces instantiation work for the compiler.
	//
	//    as for why it's a separate class and not a base of the tagged_ptr hierarchy... eh. I don't have a good answer
	//    for that other than "it is what it is" :P

	template <size_t Align>
	struct tptr
	{
		static constexpr size_t tag_bits	= (muu::max<size_t>(bit_width(Align), 1u) - 1u) + impl::tptr_addr_free_bits;
		static constexpr uintptr_t tag_mask = bit_fill_right<uintptr_t>(tag_bits);
		static constexpr uintptr_t ptr_mask = ~tag_mask;

		using tag_type =
			tptr_uint_for_bits<muu::clamp<size_t>(bit_ceil(tag_bits), CHAR_BIT, sizeof(uintptr_t) * CHAR_BIT)>;
		static_assert(sizeof(tag_type) <= sizeof(uintptr_t));

		MUU_CONST_INLINE_GETTER
		static constexpr uintptr_t pack_ptr_unchecked(uintptr_t ptr) noexcept
		{
			if constexpr (tptr_addr_free_bits > 0)
				return (ptr << tptr_addr_free_bits);
			else
				return ptr;
		}

		MUU_CONST_GETTER
		static constexpr bool can_store_ptr(uintptr_t ptr) noexcept
		{
			return !(pack_ptr_unchecked(ptr) & tag_mask);
		}

		template <typename T>
		MUU_PURE_GETTER
		static constexpr bool can_store_tag([[maybe_unused]] const T& tag) noexcept
		{
			if constexpr ((sizeof(T) * CHAR_BIT) <= tag_bits && std::is_trivially_copyable_v<T>)
			{
				return true; // this branch works for both uints and pod types
			}
			else if constexpr (is_unsigned<T>)
			{
				if constexpr (std::is_enum_v<T>)
				{
					return !(static_cast<std::underlying_type_t<T>>(tag) & ptr_mask);
				}
				else
				{
					return !(tag & ptr_mask);
				}
			}
			else
			{
				return false; // oversized/non-POD
			}
		}

		MUU_CONST_GETTER
		static constexpr uintptr_t pack_ptr(uintptr_t ptr) noexcept
		{
			MUU_CONSTEXPR_SAFE_ASSERT((!ptr || bit_floor(ptr) >= Align)
									  && "The pointer's address is aligned too strictly");

			return pack_ptr_unchecked(ptr);
		}

		template <typename T>
		MUU_PURE_GETTER
		static constexpr uintptr_t pack_both(uintptr_t ptr, const T& tag) noexcept
		{
			static_assert(std::is_trivially_copyable_v<T>, "The tag type must be trivially copyable");

			if constexpr (std::is_enum_v<T>)
			{
				return pack_both(ptr, static_cast<std::underlying_type_t<T>>(tag));
			}
			else
			{
				if constexpr (is_unsigned<T>)
				{
					if constexpr ((sizeof(T) * CHAR_BIT) > tag_bits)
					{
						MUU_CONSTEXPR_SAFE_ASSERT(can_store_tag(tag)
												  && "The tag value cannot be used without truncation");

						return pack_ptr(ptr) | (static_cast<uintptr_t>(tag) & tag_mask);
					}
					else
					{
						return pack_ptr(ptr) | static_cast<uintptr_t>(tag);
					}
				}
				else // some pod type
				{
					static_assert((sizeof(T) * CHAR_BIT) <= tag_bits,
								  "The tag type must fit in the available tag bits");

					struct source_t
					{
						unsigned char bytes[sizeof(uintptr_t)];
					};
					struct dest_t
					{
						unsigned char bytes[sizeof(T)];
					};
					union proxy_t
					{
						source_t source;
						dest_t dest;
					};

					proxy_t proxy{ muu::bit_cast<source_t>(pack_ptr(ptr)) };
					proxy.dest = muu::bit_cast<dest_t>(tag);
					return muu::bit_cast<uintptr_t>(proxy);
				}
			}
		}

		MUU_CONST_GETTER
		static constexpr uintptr_t set_ptr(uintptr_t bits, uintptr_t ptr) noexcept
		{
			return pack_ptr(ptr) | (bits & tag_mask);
		}

		MUU_CONSTRAINED_TEMPLATE(is_unsigned<T>, typename T)
		MUU_CONST_GETTER
		static constexpr uintptr_t set_tag(uintptr_t bits, T tag) noexcept
		{
			if constexpr (std::is_enum_v<T>)
			{
				return set_tag(bits, static_cast<std::underlying_type_t<T>>(tag));
			}
			else if constexpr ((sizeof(T) * CHAR_BIT) > tag_bits)
			{
				MUU_CONSTEXPR_SAFE_ASSERT(can_store_tag(tag) && "The tag value cannot be used without truncation");

				return (bits & ptr_mask) | (static_cast<uintptr_t>(tag) & tag_mask);
			}
			else
			{
				return (bits & ptr_mask) | static_cast<uintptr_t>(tag);
			}
		}

		MUU_CONSTRAINED_TEMPLATE(!is_unsigned<T>, typename T)
		MUU_PURE_GETTER
		static constexpr uintptr_t set_tag(uintptr_t bits, const T& tag) noexcept
		{
			static_assert(std::is_trivially_copyable_v<T>, "The tag type must be trivially copyable");
			static_assert((sizeof(T) * CHAR_BIT) <= tag_bits, "The tag type must fit in the available tag bits");

			struct source_t
			{
				unsigned char bytes[sizeof(uintptr_t)];
			};
			struct dest_t
			{
				unsigned char bytes[sizeof(T)];
			};
			union proxy_t
			{
				source_t source;
				dest_t dest;
			};

			proxy_t proxy{ muu::bit_cast<source_t>(bits & ptr_mask) };
			proxy.dest = muu::bit_cast<dest_t>(tag);
			return muu::bit_cast<uintptr_t>(proxy);
		}

		MUU_CONST_INLINE_GETTER
		static constexpr uintptr_t get_tag(uintptr_t bits) noexcept
		{
			return bits & tag_mask;
		}

		MUU_CONST_GETTER
		static constexpr bool get_tag_bit(uintptr_t bits, size_t index) noexcept
		{
			MUU_CONSTEXPR_SAFE_ASSERT(index < tag_bits && "Tag bit index out-of-bounds");

			return bits & (uintptr_t{ 1 } << index);
		}

		MUU_CONST_GETTER
		static constexpr uintptr_t set_tag_bit(uintptr_t bits, size_t index, bool state) noexcept
		{
			MUU_CONSTEXPR_SAFE_ASSERT(index < tag_bits && "Tag bit index out-of-bounds");

			if (state)
				return bits | (uintptr_t{ 1 } << index);
			else
				return bits & (~(uintptr_t{ 1 } << index));
		}

		template <typename T>
		MUU_CONST_GETTER
		static constexpr T get_tag_as(uintptr_t bits) noexcept
		{
			static_assert(std::is_trivially_copyable_v<T>, "The tag type must be trivially copyable");
			static_assert((sizeof(T) * CHAR_BIT) <= tag_bits, "The tag type must fit in the available tag bits");

			using intermediate_type = tptr_uint_for_bits<muu::bit_ceil(sizeof(T) * CHAR_BIT)>;
			static_assert(sizeof(intermediate_type) >= sizeof(T));
			static_assert(sizeof(intermediate_type) <= sizeof(tag_type));

			if constexpr (sizeof(T) == sizeof(intermediate_type))
			{
				return muu::bit_cast<T>(static_cast<intermediate_type>(get_tag(bits)));
			}
			else
			{
				struct source_t
				{
					unsigned char bytes[sizeof(intermediate_type)];
				};
				struct dest_t
				{
					unsigned char bytes[sizeof(T)];
				};
				union proxy_t
				{
					source_t source;
					dest_t dest;
				};
				proxy_t proxy{ muu::bit_cast<source_t>(static_cast<intermediate_type>(get_tag(bits))) };
				return muu::bit_cast<T>(proxy.dest);
			}
		}

		MUU_CONST_GETTER
		static constexpr uintptr_t get_ptr(uintptr_t bits) noexcept
		{
			bits &= ptr_mask;
			if constexpr (tptr_addr_free_bits > 0)
			{
				bits >>= tptr_addr_free_bits;

#if MUU_ARCH_AMD64
				constexpr uintptr_t canon_test = uintptr_t{ 1u } << tptr_addr_highest_used_bit;
				if (bits & canon_test)
				{
					constexpr uintptr_t canon_mask = tptr_addr_free_bits_mask << tptr_addr_used_bits;
					bits |= canon_mask;
				}
#endif
			}
			return bits;
		}
	};

	struct MUU_TRIVIAL_ABI tagged_ptr_storage
	{
		uintptr_t bits_;

		constexpr tagged_ptr_storage(uintptr_t bits) noexcept //
			: bits_{ bits }
		{}
	};

	template <typename T, size_t Align, bool = std::is_function_v<T>>
	struct MUU_TRIVIAL_ABI tagged_ptr_to_function : protected tagged_ptr_storage
	{
	  protected:
		using base = tagged_ptr_storage;
		using base::bits_;
		using base::base;

	  public:
		MUU_PURE_GETTER
		constexpr T* ptr() const noexcept
		{
			using tptr = impl::tptr<Align>;

			return uintptr_cast<T*>(tptr::get_ptr(bits_));
		}

		MUU_PURE_INLINE_GETTER
		constexpr T* get() const noexcept
		{
			return ptr();
		}

		MUU_PURE_INLINE_GETTER
		explicit constexpr operator T*() const noexcept
		{
			return ptr();
		}

		template <typename... U>
		constexpr decltype(auto) operator()(U&&... args) const noexcept(std::is_nothrow_invocable_v<T, U&&...>)
		{
			static_assert(std::is_invocable_v<T, U&&...>);

			return ptr()(static_cast<U&&>(args)...);
		}
	};

	template <typename T, size_t Align>
	struct MUU_TRIVIAL_ABI tagged_ptr_to_function<T, Align, false> : protected tagged_ptr_storage
	{
	  protected:
		using base = tagged_ptr_storage;
		using base::bits_;
		using base::base;

	  public:
		MUU_PURE_GETTER
		MUU_ATTR(assume_aligned(Align))
		constexpr T* ptr() const noexcept
		{
			using tptr = impl::tptr<Align>;

			return muu::assume_aligned<Align>(uintptr_cast<T*>(tptr::get_ptr(bits_)));
		}

		MUU_PURE_INLINE_GETTER
		MUU_ATTR(assume_aligned(Align))
		constexpr T* get() const noexcept
		{
			return ptr();
		}

		MUU_PURE_INLINE_GETTER
		MUU_ATTR(assume_aligned(Align))
		explicit constexpr operator T*() const noexcept
		{
			return ptr();
		}
	};

	template <typename T, size_t Align, bool = (!std::is_function_v<T> && !std::is_void_v<T>)>
	struct MUU_TRIVIAL_ABI tagged_ptr_to_object : public tagged_ptr_to_function<T, Align>
	{
	  protected:
		using base = tagged_ptr_to_function<T, Align>;
		using base::bits_;

		constexpr tagged_ptr_to_object(uintptr_t bits = {}) noexcept //
			: base{ bits }
		{}

	  public:
		using base::ptr;

		MUU_PURE_GETTER
		constexpr T& operator*() const noexcept
		{
			return *ptr();
		}

		MUU_PURE_GETTER
		constexpr T* operator->() const noexcept
		{
			return ptr();
		}
	};

	template <typename T, size_t Align>
	struct MUU_TRIVIAL_ABI tagged_ptr_to_object<T, Align, false> : public tagged_ptr_to_function<T, Align>
	{
	  protected:
		using base = tagged_ptr_to_function<T, Align>;
		using base::bits_;

		constexpr tagged_ptr_to_object(uintptr_t bits = {}) noexcept //
			: base{ bits }
		{}

	  public:
		using base::ptr;
	};

	struct tptr_nullptr_deduced_tag
	{};

	template <typename T, bool = (std::is_void_v<T> || std::is_function_v<T>)>
	inline constexpr size_t tptr_min_align = alignof(T);
	template <typename T>
	inline constexpr size_t tptr_min_align<T, true> = 1;
}

/// \endcond
//% tagged_ptr::impl end

//#=====================================================================================================================
//# TAGGED POINTER
//#=====================================================================================================================
//% tagged_ptr start

namespace muu
{
	/// \brief	Specialized pointer capable of storing data in the unused bits of a pointer's value.
	/// \ingroup core
	///
	/// \tparam	T		The type being pointed to.
	/// \tparam	Align	Minimum alignment of values stored in the tagged_ptr.
	/// 				Default is alignof(T), but you may override it if you know
	/// 				you will only be storing values with larger alignments.
	///
	/// \see [Tagged pointer](https://en.wikipedia.org/wiki/Tagged_pointer)
	template <typename T, size_t Align = impl::tptr_min_align<T>>
	class MUU_TRIVIAL_ABI tagged_ptr //
		MUU_HIDDEN_BASE(public impl::tagged_ptr_to_object<T, Align>)
	{
		static_assert(!std::is_same_v<T, impl::tptr_nullptr_deduced_tag>,
					  "Tagged pointers cannot have their type deduced from a nullptr"
					  " (a nullptr is meaningless in this context)");

		static_assert(!std::is_reference_v<T>, "Tagged pointers cannot store references");

		static_assert(!std::is_function_v<T> || !is_cvref<T>, "Tagged pointers to functions cannot be cv-qualified");

		static_assert(has_single_bit(Align), "Alignment must be a power of two");

		static_assert(Align >= impl::tptr_min_align<T>, "Alignment cannot be smaller than the type's actual alignment");

		static_assert(Align > 1 || impl::tptr_addr_free_bits > 0,
					  "Types aligned on a single byte cannot be pointed to by a tagged pointer on this platform");

	  private:
		using tptr = impl::tptr<Align>;
		using base = impl::tagged_ptr_to_object<T, Align>;
		using base::bits_;

	  public:
		/// \brief	The type being pointed to.
		using element_type = T;

		/// \brief	A pointer to element_type.
		using pointer = std::add_pointer_t<T>;
		static_assert(sizeof(pointer) == sizeof(uintptr_t), "unexpected pointer size");

		/// \brief	A pointer to element_type (const-qualified).
		using const_pointer = std::add_pointer_t<std::add_const_t<T>>;

		/// \brief	An unsigned integer just large enough to store all the bits in the tag area.
		using tag_type = typename tptr::tag_type;

		/// \brief The minimum alignment of values stored in this pointer.
		static constexpr size_t alignment = Align;

		/// \brief	The number of tag bits available.
		static constexpr size_t tag_bit_count = tptr::tag_bits;

		/// \brief	The largest integral value that can be stored in the available tag bits.
		static constexpr tag_type max_tag = bit_fill_right<tag_type>(tag_bit_count);

		/// \brief	Constructs a tagged pointer.
		/// \details Tag bits are initialized to zero.
		///
		/// \param	value	The initial address of the pointer's target.
		MUU_NODISCARD_CTOR
		explicit constexpr tagged_ptr(pointer value) noexcept //
			: base{ tptr::pack_ptr(impl::uintptr_cast(value)) }
		{}

		/// \brief	Constructs a tagged pointer.
		///
		/// \tparam	U			An unsigned integral type, or a trivially-copyable type small enough to fit.
		/// \param	value		The initial address of the pointer's target.
		/// \param	tag_value	The initial value of the pointer's tag bits.
		///
		/// \warning If the tag parameter is an integer larger than the available tag bits,
		/// 		 any overflow will be masked out and ignored.
		template <typename U>
		MUU_NODISCARD_CTOR
		constexpr tagged_ptr(pointer value, const U& tag_value) noexcept //
			: base{ tptr::pack_both(impl::uintptr_cast(value), tag_value) }
		{
			static_assert(is_unsigned<U> //
							  || (std::is_trivially_copyable_v<U> && sizeof(U) * CHAR_BIT <= tag_bit_count),
						  "Tag types must be unsigned integrals or trivially-copyable"
						  " and small enough to fit in the available tag bits");
		}

		/// \brief	Constructs zero-initialized tagged pointer.
		MUU_NODISCARD_CTOR
		constexpr tagged_ptr(std::nullptr_t) noexcept
		{}

		/// \brief	Constructs zero-initialized tagged pointer.
		MUU_NODISCARD_CTOR
		constexpr tagged_ptr() noexcept = default;

		/// \brief	Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr tagged_ptr(const tagged_ptr&) noexcept = default;

		/// \brief	Copy-assignment operator.
		constexpr tagged_ptr& operator=(const tagged_ptr&) noexcept = default;

		/// \brief	Destructor.
		~tagged_ptr() noexcept = default;

		/// \brief	Sets the target pointer value and all tag bits to zero.
		///
		/// \returns	A reference to the tagged_ptr.
		constexpr tagged_ptr& reset() noexcept
		{
			bits_ = {};
			return *this;
		}

		/// \brief	Resets the target pointer value and sets the tag bits to zero.
		///
		/// \param	value	The new target pointer value.
		///
		/// \returns	A reference to the tagged_ptr.
		constexpr tagged_ptr& reset(pointer value) noexcept
		{
			bits_ = tptr::pack_ptr(impl::uintptr_cast(value));
			return *this;
		}

		/// \brief	Resets the target pointer value and tag.
		///
		/// \tparam	U			An unsigned integral type, or a trivially-copyable type small enough to fit.
		/// \param	value		The new target pointer value.
		/// \param	tag_value	The new tag.
		///
		/// \warning If the tag parameter is an integer larger than the available tag bits,
		/// 		 any overflow will be masked out and ignored.
		///
		/// \returns	A reference to the tagged_ptr.
		template <typename U>
		constexpr tagged_ptr& reset(pointer value, const U& tag_value) noexcept
		{
			static_assert(is_unsigned<U> //
							  || (std::is_trivially_copyable_v<U> && sizeof(U) * CHAR_BIT <= tag_bit_count),
						  "Tag types must be unsigned integrals or trivially-copyable"
						  " and small enough to fit in the available tag bits");

			bits_ = tptr::pack_both(impl::uintptr_cast(value), tag_value);
			return *this;
		}

#if !MUU_DOXYGEN

		using base::ptr; // prevent the setter from hiding the getter in the base

#else
		//# {{

		/// \brief	Returns the target pointer value.
		MUU_NODISCARD
		constexpr pointer ptr() const noexcept;

		/// \brief	Returns the target pointer value.
		///
		/// \remarks This is an alias for ptr(); it exists to keep the interface consistent with std::unique_ptr.
		MUU_NODISCARD
		constexpr pointer get() const noexcept;

		/// \brief	Returns the target pointer value.
		MUU_NODISCARD
		explicit constexpr operator pointer() const noexcept;

		/// \brief	Invokes the function call operator on the pointed function.
		///
		/// \tparam U		Argument types.
		/// \param	args	The arguments to pass to the pointed function.
		///
		/// \returns	The return value of the function call.
		///
		/// \availability This operator is only available when the pointed type is a function.
		template <typename... U>
		constexpr decltype(auto) operator()(U&&... args) const
			noexcept(std::is_nothrow_invocable_v<element_type, U&&...>);

		/// \brief	Returns a reference to the pointed object.
		///
		/// \availability This operator is not available for pointers to `void` or functions.
		MUU_NODISCARD
		constexpr element_type& operator*() const noexcept;

		/// \brief	Returns the target pointer value.
		///
		/// \availability This operator is not available for pointers to `void` or functions.
		MUU_NODISCARD
		constexpr pointer operator->() const noexcept;

		//# }}
#endif
		/// \brief	Checks if a raw pointer can be safely stored without clipping into the tag bits.
		///
		/// \returns	True if the given raw pointer is sufficiently aligned and does not already contain any tag bits.
		MUU_CONST_INLINE_GETTER
		static constexpr bool can_store_ptr(pointer value) noexcept
		{
			return tptr::can_store_ptr(impl::uintptr_cast(value));
		}

		/// \brief	Sets the target pointer value, leaving the tag bits unchanged.
		///
		/// \param 	value	The new target pointer value.
		///
		/// \returns	A reference to a tagged_ptr.
		constexpr tagged_ptr& ptr(pointer value) noexcept
		{
			bits_ = tptr::set_ptr(bits_, impl::uintptr_cast(value));
			return *this;
		}

		/// \brief	Sets the target pointer value, leaving the tag bits unchanged.
		///
		/// \param	rhs	The new target pointer value.
		///
		/// \returns	A reference to the tagged_ptr.
		MUU_ALWAYS_INLINE
		constexpr tagged_ptr& operator=(pointer rhs) noexcept
		{
			return ptr(rhs);
		}

		/// \brief	Sets the target pointer value to nullptr, leaving the tag bits unchanged.
		///
		/// \returns	A reference to the tagged_ptr.
		constexpr tagged_ptr& clear_ptr() noexcept
		{
			bits_ &= tptr::tag_mask;
			return *this;
		}

		/// \brief	Returns the tag bits as an unsigned integer or trivially-copyable type.
		template <typename U = tag_type>
		MUU_PURE_GETTER
		constexpr U tag() const noexcept
		{
			static_assert(!std::is_reference_v<U>, "Reference types are not allowed");
			static_assert(std::is_trivially_copyable_v<U>, "Tag types must be trivially-copyable");

			if constexpr (is_unsigned<U>)
			{
				static_assert(
					sizeof(U) >= sizeof(tag_type),
					"The destination integer type is not large enough to store the tag without a loss of data");
				return static_cast<U>(tptr::get_tag(bits_));
			}
			else
			{
				static_assert((sizeof(U) * CHAR_BIT) <= tag_bit_count, "Tag types must fit in the available tag bits");

				return tptr::template get_tag_as<U>(bits_);
			}
		}

		/// \brief	Checks if a tag value can be safely stored without clipping into the pointer bits.
		///
		/// \returns	True if the given tag value is of a compatible type (trivially-copyable, unsigned/enum or POD, small enough, etc.)
		///				and would not collide with any bits in the pointer region.
		template <typename U>
		MUU_PURE_INLINE_GETTER
		static constexpr bool can_store_tag(const U& tag_value) noexcept
		{
			return tptr::can_store_tag(tag_value);
		}

		/// \brief	Sets the tag bits, leaving the target pointer value unchanged.
		///
		/// \tparam	U			An unsigned integral type, or a trivially-copyable type small enough to fit.
		/// \param	tag_value	The new value to set for the pointer's tag bits.
		///
		/// \warning If the tag parameter is an integer larger than the available tag bits,
		/// 		 any overflow will be masked out and ignored.
		///
		/// \returns	A reference to the tagged_ptr.
		template <typename U>
		constexpr tagged_ptr& tag(const U& tag_value) noexcept
		{
			static_assert(is_unsigned<U> //
							  || (std::is_trivially_copyable_v<U> && sizeof(U) * CHAR_BIT <= tag_bit_count),
						  "Tag types must be unsigned integrals or trivially-copyable"
						  " and small enough to fit in the available tag bits");

			bits_ = tptr::set_tag(bits_, tag_value);
			return *this;
		}

		/// \brief	Returns the value of one of the tag bits.
		MUU_PURE_GETTER
		constexpr bool tag_bit(size_t tag_bit_index) const noexcept
		{
			return tptr::get_tag_bit(bits_, tag_bit_index);
		}

		/// \brief	Sets the value of one of the tag bits.
		///
		/// \param	tag_bit_index	Zero-based index of the tag bit.
		/// \param	val		   		The bit state to set.
		///
		/// \returns	A reference to the tagged_ptr.
		///
		/// \warning Using the pointer's tag to store a small POD type is generally incompatible with setting
		/// 		 individual tag bits, as one is likely to render the other meaningless. Mix methodologies with
		/// 		 caution!
		constexpr tagged_ptr& tag_bit(size_t tag_bit_index, bool val) noexcept
		{
			bits_ = tptr::set_tag_bit(bits_, tag_bit_index, val);
			return *this;
		}

		/// \brief	Sets the tag bits to zero, leaving the target pointer value unchanged.
		///
		/// \returns	A reference to the tagged_ptr.
		constexpr tagged_ptr& clear_tag() noexcept
		{
			bits_ &= tptr::ptr_mask;
			return *this;
		}

		/// \brief	Returns true if the target pointer value is not nullptr.
		MUU_PURE_INLINE_GETTER
		explicit constexpr operator bool() const noexcept
		{
			return bits_ & tptr::ptr_mask;
		}

		/// \brief	Returns true if a tagged_ptr and raw pointer refer to the same address.
		MUU_CONST_INLINE_GETTER
		friend constexpr bool operator==(tagged_ptr lhs, const_pointer rhs) noexcept
		{
			return lhs.ptr() == rhs;
		}

		/// \brief	Returns true if a tagged_ptr and raw pointer do not refer to the same address.
		MUU_CONST_INLINE_GETTER
		friend constexpr bool operator!=(tagged_ptr lhs, const_pointer rhs) noexcept
		{
			return lhs.ptr() != rhs;
		}

		/// \brief	Returns true if a tagged_ptr and raw pointer refer to the same address.
		MUU_CONST_INLINE_GETTER
		friend constexpr bool operator==(const_pointer lhs, tagged_ptr rhs) noexcept
		{
			return lhs == rhs.ptr();
		}

		/// \brief	Returns true if a tagged_ptr and raw pointer do not refer to the same address.
		MUU_CONST_INLINE_GETTER
		friend constexpr bool operator!=(const_pointer lhs, tagged_ptr rhs) noexcept
		{
			return lhs != rhs.ptr();
		}

		/// \brief	Returns true if two tagged_ptrs are equal (including their tag bits).
		MUU_CONST_INLINE_GETTER
		friend constexpr bool operator==(tagged_ptr lhs, tagged_ptr rhs) noexcept
		{
			return lhs.bits_ == rhs.bits_;
		}

		/// \brief	Returns true if two tagged_ptrs are not equal (including their tag bits).
		MUU_CONST_INLINE_GETTER
		friend constexpr bool operator!=(tagged_ptr lhs, tagged_ptr rhs) noexcept
		{
			return lhs.bits_ != rhs.bits_;
		}
	};

	/// \cond

	tagged_ptr(std::nullptr_t)->tagged_ptr<impl::tptr_nullptr_deduced_tag, 1>;
	template <typename T>
	tagged_ptr(std::nullptr_t, T) -> tagged_ptr<impl::tptr_nullptr_deduced_tag, 1>;
	template <typename T, typename U>
	tagged_ptr(T*, U) -> tagged_ptr<T>;
	template <typename T>
	tagged_ptr(T*) -> tagged_ptr<T>;

	/// \endcond
}

//% tagged_ptr end

//#=====================================================================================================================
//# POINTER TRAITS
//#=====================================================================================================================
//% tagged_ptr::traits start

/// \cond
namespace muu::impl
{
	template <typename T>
	struct tagged_pointer_traits_base
	{};

	template <typename T, bool IsVoid>
	struct tagged_pointer_traits
	{};

	template <template <typename, size_t> typename TaggedPointer, typename T, size_t Align>
	struct tagged_pointer_traits_base<TaggedPointer<T, Align>>
	{
		using pointer		  = TaggedPointer<T, Align>;
		using element_type	  = T;
		using difference_type = ptrdiff_t;
		template <typename U>
		using rebind = TaggedPointer<U, Align>;

		MUU_PURE_GETTER
		constexpr static element_type* to_address(const pointer& p) noexcept
		{
			return p.ptr();
		}
	};

	template <template <typename, size_t> typename TaggedPointer, typename T, size_t Align>
	struct tagged_pointer_traits<TaggedPointer<T, Align>, true> : tagged_pointer_traits_base<TaggedPointer<T, Align>>
	{};

	template <template <typename, size_t> typename TaggedPointer, typename T, size_t Align>
	struct tagged_pointer_traits<TaggedPointer<T, Align>, false> : tagged_pointer_traits_base<TaggedPointer<T, Align>>
	{
		using pointer	   = TaggedPointer<T, Align>;
		using element_type = T;

		MUU_PURE_GETTER
		constexpr static pointer pointer_to(element_type& r) noexcept
		{
			return pointer{ std::addressof(r) };
		}
	};
}
/// \endcond

namespace std
{
	/// \brief Specialization of std::pointer_traits for #muu::tagged_ptr.
	template <typename T, size_t Align>
	struct pointer_traits<muu::tagged_ptr<T, Align>>
		: muu::impl::tagged_pointer_traits<muu::tagged_ptr<T, Align>, std::is_void_v<T>>
	{};
}

//% tagged_ptr::traits end

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
