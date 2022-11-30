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

#ifndef MUU_TAGGED_PTR_BITS
	#define MUU_TAGGED_PTR_BITS 0 // 0 == "all the bits"
#endif
#if MUU_ARCH_AMD64 && MUU_TAGGED_PTR_BITS                                                                              \
	&& !(MUU_TAGGED_PTR_BITS == 48 || MUU_TAGGED_PTR_BITS == 57 || MUU_TAGGED_PTR_BITS == 64)
	#error MUU_TAGGED_PTR_BITS must be 48, 57 or 64 on AMD64. See: https://en.wikipedia.org/wiki/X86-64
#endif

#define MUU_TAGGED_PTR_TAG_TYPE_CHECKS(Tag)                                                                            \
	static_assert(!is_cvref<Tag>, "Tag type may not be explicitly cvref-qualified");                                   \
	static_assert(std::is_trivially_default_constructible_v<Tag>, "Tag type must be trivially default-constructible"); \
	static_assert(std::is_trivially_copyable_v<Tag>, "Tag type must be trivially copyable")

#define MUU_TAGGED_PTR_TAG_OBJECT_CHECKS(Tag)                                                                          \
	MUU_TAGGED_PTR_TAG_TYPE_CHECKS(Tag);                                                                               \
	static_assert((sizeof(Tag) * CHAR_BIT) <= tag_bits, "Tag type must fit in the available tag bits")

#if MUU_MSVC
	#pragma warning(push)
	#pragma warning(disable : 4296) // condition is always false/true
#endif

namespace muu::impl
{
	static_assert(MUU_TAGGED_PTR_BITS < sizeof(uintptr_t) * CHAR_BIT);

	inline constexpr size_t tptr_used_bits = MUU_TAGGED_PTR_BITS ? MUU_TAGGED_PTR_BITS : sizeof(uintptr_t) * CHAR_BIT;
	inline constexpr size_t tptr_free_bits = sizeof(uintptr_t) * CHAR_BIT - tptr_used_bits;

	// clang-format off

	template <size_t Bits>
	using tptr_uint_for_bits =
		std::conditional_t<(Bits <= sizeof(unsigned char) * CHAR_BIT), unsigned char,
		std::conditional_t<(Bits <= sizeof(unsigned short) * CHAR_BIT), unsigned short,
		std::conditional_t<(Bits <= sizeof(unsigned int) * CHAR_BIT), unsigned int,
		std::conditional_t<(Bits <= sizeof(unsigned long) * CHAR_BIT), unsigned long,
		std::conditional_t<(Bits <= sizeof(unsigned long long) * CHAR_BIT), unsigned long long,
		void
	>>>>>;

	template <typename T>
	using tptr_is_enum_or_integer_ = std::disjunction<
		std::is_enum<T>,
		std::is_same<T, signed char>,
		std::is_same<T, signed short>,
		std::is_same<T, signed int>,
		std::is_same<T, signed long>,
		std::is_same<T, signed long long>,
		std::is_same<T, unsigned char>,
		std::is_same<T, unsigned short>,
		std::is_same<T, unsigned int>,
		std::is_same<T, unsigned long>,
		std::is_same<T, unsigned long long>
	>;

	// clang-format on

	template <typename T>
	inline constexpr bool tptr_is_enum_or_integer = tptr_is_enum_or_integer_<remove_cvref<T>>::value;

	template <typename T, bool = std::is_enum_v<T>>
	struct tptr_make_unsigned_
	{
		using type = std::make_unsigned_t<T>;
	};
	template <typename T>
	struct tptr_make_unsigned_<T, true>
	{
		static_assert(tptr_is_enum_or_integer<T>);

		using type = std::make_unsigned_t<std::underlying_type_t<T>>;
	};
	template <typename T>
	using tptr_make_unsigned = typename tptr_make_unsigned_<T>::type;

	class MUU_TRIVIAL_ABI tptr_base
	{
	  protected:
		uintptr_t bits_ = {};

		MUU_NODISCARD_CTOR
		constexpr tptr_base(uintptr_t bits) noexcept //
			: bits_{ bits }
		{}

		MUU_CONST_INLINE_GETTER
		static constexpr uintptr_t pack_ptr_unchecked(uintptr_t ptr) noexcept
		{
			if constexpr (tptr_free_bits > 0)
				return (ptr << tptr_free_bits);
			else
				return ptr;
		}

	  public:
		MUU_NODISCARD_CTOR
		constexpr tptr_base() noexcept = default;
	};

	template <size_t Align>
	class MUU_TRIVIAL_ABI tptr_aligned_base : public tptr_base
	{
	  protected:
		static constexpr size_t tag_bits	= (muu::max<size_t>(muu::bit_width(Align), 1u) - 1u) + impl::tptr_free_bits;
		static constexpr uintptr_t tag_mask = bit_fill_right<uintptr_t>(tag_bits);
		static constexpr uintptr_t ptr_mask = ~tag_mask;

		using tag_type =
			tptr_uint_for_bits<muu::clamp<size_t>(bit_ceil(tag_bits), CHAR_BIT, sizeof(uintptr_t) * CHAR_BIT)>;
		static_assert(sizeof(tag_type) <= sizeof(uintptr_t));

		using base = tptr_base;
		using base::bits_;
		using base::base;

		MUU_CONST_GETTER
		static constexpr bool can_store_ptr(uintptr_t ptr) noexcept
		{
			if constexpr (!tag_bits)
			{
				return true;
			}
			else
			{
				return !(base::pack_ptr_unchecked(ptr) & tag_mask);
			}
		}

		template <typename Tag>
		MUU_PURE_GETTER
		static constexpr bool can_store_tag([[maybe_unused]] const Tag& tag) noexcept
		{
			if constexpr (!tag_bits)
			{
				return false;
			}
			else if constexpr ((sizeof(Tag) * CHAR_BIT) <= tag_bits				 //
							   && std::is_trivially_default_constructible_v<Tag> //
							   && std::is_trivially_copyable_v<Tag>)
			{
				return true; // this branch works for both ints and pod types
			}
			else if constexpr (tptr_is_enum_or_integer<Tag>)
			{
				return !(static_cast<tptr_make_unsigned<Tag>>(tag) & ptr_mask);
			}
			else
			{
				return false; // oversized/non-POD
			}
		}

		MUU_CONST_GETTER
		static uintptr_t pack_ptr(uintptr_t ptr) noexcept
		{
			MUU_ASSERT((!ptr || muu::bit_floor(ptr) >= Align)
					   && "The pointer's address is aligned too strictly aligned");

			return base::pack_ptr_unchecked(ptr);
		}

		template <typename Tag>
		MUU_PURE_GETTER
		static uintptr_t pack_both(uintptr_t ptr, const Tag& tag) noexcept
		{
			MUU_TAGGED_PTR_TAG_TYPE_CHECKS(Tag);

			if constexpr (std::is_enum_v<Tag>)
			{
				return pack_both(ptr, static_cast<tptr_make_unsigned<Tag>>(tag));
			}
			else
			{
				if constexpr (std::is_integral_v<Tag>)
				{
					if constexpr ((sizeof(Tag) * CHAR_BIT) > tag_bits)
					{
						MUU_ASSERT(can_store_tag(tag) && "Tag value cannot be used without truncation");

						return pack_ptr(ptr) | (static_cast<uintptr_t>(tag) & tag_mask);
					}
					else
					{
						return pack_ptr(ptr) | static_cast<uintptr_t>(tag);
					}
				}
				else // some pod type
				{
					MUU_TAGGED_PTR_TAG_OBJECT_CHECKS(Tag);

					uintptr_t bits;
					if constexpr ((sizeof(Tag) * CHAR_BIT) < tag_bits)
					{
						bits = pack_ptr(ptr) & ptr_mask;
					}
					else
					{
						bits = pack_ptr(ptr);
					}
					MUU_MEMCPY(&bits, &tag, sizeof(tag));
					return bits;
				}
			}
		}

		MUU_CONST_GETTER
		static uintptr_t set_ptr(uintptr_t bits, uintptr_t ptr) noexcept
		{
			if constexpr (tag_bits)
			{
				return pack_ptr(ptr) | (bits & tag_mask);
			}
			else
			{
				return pack_ptr(ptr);
			}
		}

		MUU_CONSTRAINED_TEMPLATE(tptr_is_enum_or_integer<Tag>, typename Tag)
		MUU_CONST_GETTER
		static uintptr_t set_tag(uintptr_t bits, Tag tag) noexcept
		{
			MUU_TAGGED_PTR_TAG_TYPE_CHECKS(Tag);

			if constexpr (std::is_enum_v<Tag>)
			{
				return set_tag(bits, static_cast<tptr_make_unsigned<Tag>>(tag));
			}
			else if constexpr ((sizeof(Tag) * CHAR_BIT) > tag_bits)
			{
				MUU_ASSERT(can_store_tag(tag) && "Tag value cannot be used without truncation");

				return (bits & ptr_mask) | (static_cast<uintptr_t>(tag) & tag_mask);
			}
			else
			{
				return (bits & ptr_mask) | static_cast<uintptr_t>(tag);
			}
		}

		MUU_CONSTRAINED_TEMPLATE(!tptr_is_enum_or_integer<Tag>, typename Tag)
		MUU_PURE_GETTER
		static uintptr_t set_tag(uintptr_t bits, const Tag& tag) noexcept
		{
			MUU_TAGGED_PTR_TAG_OBJECT_CHECKS(Tag);

			if constexpr ((sizeof(Tag) * CHAR_BIT) < tag_bits)
			{
				bits &= ptr_mask;
			}
			MUU_MEMCPY(&bits, &tag, sizeof(tag));
			return bits;
		}

		MUU_CONST_INLINE_GETTER
		static uintptr_t get_tag([[maybe_unused]] uintptr_t bits) noexcept
		{
			if constexpr (tag_bits)
			{
				return bits & tag_mask;
			}
			else
			{
				return {};
			}
		}

		MUU_CONST_GETTER
		static bool get_tag_bit(uintptr_t bits, size_t index) noexcept
		{
			MUU_ASSERT(index < tag_bits && "Tag bit index out-of-bounds");

			return bits & (uintptr_t{ 1 } << index);
		}

		MUU_CONST_GETTER
		static uintptr_t set_tag_bit(uintptr_t bits, size_t index, bool state) noexcept
		{
			MUU_ASSERT(index < tag_bits && "Tag bit index out-of-bounds");

			if (state)
				return bits | (uintptr_t{ 1 } << index);
			else
				return bits & (~(uintptr_t{ 1 } << index));
		}

		template <typename Tag>
		MUU_CONST_GETTER
		static Tag get_tag_as_object(uintptr_t bits) noexcept
		{
			MUU_TAGGED_PTR_TAG_OBJECT_CHECKS(Tag);

			Tag tag;
			MUU_MEMCPY(&tag, &bits, sizeof(tag));
			return tag;
		}

	  private:
		// Q: why does get_ptr_impl exist?
		//
		// A: compilers that evaluate the if constexpr branches over-eagerly will issue warnings about
		//    right-shifting >= sizeof(uintptr_t) when tptr_free_bits == 0; sticking the logic in a
		//    separate template forces the branch evaluation to be delayed so the dead ones get pruned
		//    properly without the warning.
		template <size_t FreeBits, size_t UsedBits>
		MUU_CONST_GETTER
		static uintptr_t get_ptr_impl(uintptr_t bits) noexcept
		{
			static_assert((FreeBits + UsedBits) == sizeof(uintptr_t) * CHAR_BIT);

			bits &= ptr_mask;
			if constexpr (FreeBits > 0)
			{
				bits >>= FreeBits;

#if MUU_ARCH_AMD64
				static constexpr uintptr_t canon_test = uintptr_t{ 1 } << (UsedBits - 1u);
				if (bits & canon_test)
				{
					static constexpr uintptr_t canon_mask = bit_fill_right<uintptr_t>(FreeBits) << UsedBits;
					bits |= canon_mask;
				}
#endif
			}
			return bits;
		}

	  public:
		MUU_CONST_INLINE_GETTER
		static uintptr_t get_ptr(uintptr_t bits) noexcept
		{
			return get_ptr_impl<tptr_free_bits, tptr_used_bits>(bits);
		}

	  public:
		MUU_NODISCARD_CTOR
		constexpr tptr_aligned_base() noexcept = default;
	};

	// primary template; T is a function
	template <typename T, size_t Align, bool = std::is_function_v<T>>
	class MUU_TRIVIAL_ABI tptr_to_function : public tptr_aligned_base<Align>
	{
	  protected:
		using base = tptr_aligned_base<Align>;
		using base::bits_;
		using base::base;

	  public:
		MUU_NODISCARD_CTOR
		constexpr tptr_to_function() noexcept = default;

		MUU_PURE_GETTER
		T* ptr() const noexcept
		{
			return reinterpret_cast<T*>(base::get_ptr(bits_));
		}

		MUU_PURE_INLINE_GETTER
		T* get() const noexcept
		{
			return ptr();
		}

		MUU_PURE_INLINE_GETTER
		explicit operator T*() const noexcept
		{
			return ptr();
		}

		template <typename... U>
		decltype(auto) operator()(U&&... args) const noexcept(std::is_nothrow_invocable_v<T, U&&...>)
		{
			static_assert(std::is_invocable_v<T, U&&...>);

			return ptr()(static_cast<U&&>(args)...);
		}
	};

	// specialization; T is an object or void
	template <typename T, size_t Align>
	struct MUU_TRIVIAL_ABI tptr_to_function<T, Align, false> : public tptr_aligned_base<Align>
	{
	  protected:
		using base = tptr_aligned_base<Align>;
		using base::bits_;
		using base::base;

	  public:
		MUU_NODISCARD_CTOR
		constexpr tptr_to_function() noexcept = default;

		MUU_PURE_GETTER
		MUU_ATTR(assume_aligned(Align))
		T* ptr() const noexcept
		{
			return muu::assume_aligned<Align>(reinterpret_cast<T*>(base::get_ptr(bits_)));
		}

		MUU_PURE_INLINE_GETTER
		MUU_ATTR(assume_aligned(Align))
		T* get() const noexcept
		{
			return ptr();
		}

		MUU_PURE_INLINE_GETTER
		MUU_ATTR(assume_aligned(Align))
		explicit operator T*() const noexcept
		{
			return ptr();
		}
	};

	// primary template; T is an object
	template <typename T, size_t Align, bool = (!std::is_function_v<T> && !std::is_void_v<T>)>
	struct MUU_TRIVIAL_ABI tptr_to_object : public tptr_to_function<T, Align>
	{
	  protected:
		using base = tptr_to_function<T, Align>;
		using base::bits_;
		using base::base;

	  public:
		MUU_NODISCARD_CTOR
		constexpr tptr_to_object() noexcept = default;

		MUU_PURE_INLINE_GETTER
		T& operator*() const noexcept
		{
			return *base::ptr();
		}

		MUU_PURE_INLINE_GETTER
		T* operator->() const noexcept
		{
			return base::ptr();
		}
	};

	// specialization; T is a function or void
	template <typename T, size_t Align>
	struct MUU_TRIVIAL_ABI tptr_to_object<T, Align, false> : public tptr_to_function<T, Align>
	{
	  protected:
		using base = tptr_to_function<T, Align>;
		using base::bits_;
		using base::base;

	  public:
		MUU_NODISCARD_CTOR
		constexpr tptr_to_object() noexcept = default;
	};

	struct tptr_nullptr_deduced_tag
	{};

	template <typename T, bool = (std::is_void_v<T> || std::is_function_v<T>)>
	inline constexpr size_t tptr_min_align = alignof(T);
	template <typename T>
	inline constexpr size_t tptr_min_align<T, true> = 1;
}

#if MUU_MSVC
	#pragma warning(pop)
#endif

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
		MUU_HIDDEN_BASE(public impl::tptr_to_object<T, Align>)
	{
	  private:
		using base = impl::tptr_to_object<T, Align>;
		using base::bits_;
		using base::base;

		static_assert(!std::is_same_v<T, impl::tptr_nullptr_deduced_tag>,
					  "Tagged pointers cannot have their type deduced from a nullptr"
					  " (a nullptr is meaningless in this context)");

		static_assert(!std::is_reference_v<T>, "Tagged pointers cannot store references");

		static_assert(!std::is_function_v<T> || !is_cvref<T>, "Tagged pointers to functions cannot be cv-qualified");

		static_assert(has_single_bit(Align), "Alignment must be a power of two");

		static_assert(Align >= impl::tptr_min_align<T>, "Alignment cannot be smaller than the type's actual alignment");

		static_assert(Align > 1 || base::tag_bits > 0,
					  "Types aligned on a single byte cannot be pointed to by a tagged pointer on this platform");

	  public:
		/// \brief	The type being pointed to.
		using element_type = T;

		/// \brief	A pointer to element_type.
		using pointer = std::add_pointer_t<T>;
		static_assert(sizeof(pointer) == sizeof(uintptr_t), "unexpected pointer size");

		/// \brief	A pointer to element_type (const-qualified).
		using const_pointer = std::add_pointer_t<std::add_const_t<T>>;

		/// \brief	An unsigned integer just large enough to store all the bits in the tag area.
		using tag_type = typename base::tag_type;

		/// \brief The minimum alignment of values stored in this pointer.
		static constexpr size_t alignment = Align;

		/// \brief	The number of tag bits available.
		static constexpr size_t tag_bit_count = base::tag_bits;

		/// \brief	The largest integral value that can be stored in the available tag bits.
		static constexpr tag_type max_tag = bit_fill_right<tag_type>(tag_bit_count);

		/// \brief	Constructs a null pointer with all tag bits initialized to zero.
		MUU_NODISCARD_CTOR
		constexpr tagged_ptr() noexcept = default;

		/// \brief	Constructs a null pointer with all tag bits initialized to zero.
		MUU_NODISCARD_CTOR
		constexpr tagged_ptr(std::nullptr_t) noexcept
		{}

		/// \brief	Constructs a tagged pointer.
		/// \details Tag bits are initialized to zero.
		///
		/// \param	value	The initial address of the pointer's target.
		MUU_NODISCARD_CTOR
		explicit tagged_ptr(pointer value) noexcept //
			: base{ base::pack_ptr(reinterpret_cast<uintptr_t>(value)) }
		{}

		/// \brief	Constructs a tagged pointer.
		///
		/// \tparam	Tag			An integer, enum, or a trivial object type small enough to fit.
		/// \param	value		The initial address of the pointer's target.
		/// \param	tag_value	The initial value of the pointer's tag bits.
		///
		/// \warning If the tag parameter is an integer larger than the available tag bits,
		/// 		 any overflow will be masked out and ignored.
		template <typename Tag>
		MUU_NODISCARD_CTOR
		tagged_ptr(pointer value, const Tag& tag_value) noexcept //
			: base{ base::pack_both(reinterpret_cast<uintptr_t>(value), tag_value) }
		{}

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
		tagged_ptr& reset(pointer value) noexcept
		{
			bits_ = base::pack_ptr(reinterpret_cast<uintptr_t>(value));
			return *this;
		}

		/// \brief	Resets the target pointer value and tag.
		///
		/// \tparam	Tag			An integer, enum, or a trivial object type small enough to fit.
		/// \param	value		The new target pointer value.
		/// \param	tag_value	The new tag.
		///
		/// \warning If the tag parameter is an integer larger than the available tag bits,
		/// 		 any overflow will be masked out and ignored.
		///
		/// \returns	A reference to the tagged_ptr.
		template <typename Tag>
		tagged_ptr& reset(pointer value, const Tag& tag_value) noexcept
		{
			bits_ = base::pack_both(reinterpret_cast<uintptr_t>(value), tag_value);
			return *this;
		}

#if !MUU_DOXYGEN

		using base::ptr; // prevent the setter from hiding the getter in the base

#else
		//# {{

		/// \brief	Returns the target pointer value.
		MUU_NODISCARD
		pointer ptr() const noexcept;

		/// \brief	Returns the target pointer value.
		///
		/// \remarks This is an alias for ptr(); it exists to keep the interface consistent with std::unique_ptr.
		MUU_NODISCARD
		pointer get() const noexcept;

		/// \brief	Returns the target pointer value.
		MUU_NODISCARD
		explicit operator pointer() const noexcept;

		/// \brief	Invokes the function call operator on the pointed function.
		///
		/// \tparam U		Argument types.
		/// \param	args	The arguments to pass to the pointed function.
		///
		/// \returns	The return value of the function call.
		///
		/// \availability This operator is only available when the pointed type is a function.
		template <typename... U>
		decltype(auto) operator()(U&&... args) const noexcept(std::is_nothrow_invocable_v<element_type, U&&...>);

		/// \brief	Returns a reference to the pointed object.
		///
		/// \availability This operator is not available for pointers to `void` or functions.
		MUU_NODISCARD
		element_type& operator*() const noexcept;

		/// \brief	Returns the target pointer value.
		///
		/// \availability This operator is not available for pointers to `void` or functions.
		MUU_NODISCARD
		pointer operator->() const noexcept;

		//# }}
#endif
		/// \brief	Checks if a raw pointer can be safely stored without clipping into the tag bits.
		///
		/// \returns	True if the given raw pointer is sufficiently aligned and does not already contain any tag bits.
		MUU_CONST_INLINE_GETTER
		static bool can_store_ptr(pointer value) noexcept
		{
			return base::can_store_ptr(reinterpret_cast<uintptr_t>(value));
		}

		/// \brief	Sets the target pointer value, leaving the tag bits unchanged.
		///
		/// \param 	value	The new target pointer value.
		///
		/// \returns	A reference to a tagged_ptr.
		tagged_ptr& ptr(pointer value) noexcept
		{
			bits_ = base::set_ptr(bits_, reinterpret_cast<uintptr_t>(value));
			return *this;
		}

		/// \brief	Sets the target pointer value, leaving the tag bits unchanged.
		///
		/// \param	rhs	The new target pointer value.
		///
		/// \returns	A reference to the tagged_ptr.
		MUU_ALWAYS_INLINE
		tagged_ptr& operator=(pointer rhs) noexcept
		{
			return ptr(rhs);
		}

		/// \brief	Sets the target pointer value to nullptr, leaving the tag bits unchanged.
		///
		/// \returns	A reference to the tagged_ptr.
		constexpr tagged_ptr& clear_ptr() noexcept
		{
			bits_ &= base::tag_mask;
			return *this;
		}

		/// \brief	Returns the tag bits as an integer, enum, or trivial object type.
		template <typename Tag = tag_type>
		MUU_PURE_GETTER
		Tag tag() const noexcept
		{
			MUU_TAGGED_PTR_TAG_TYPE_CHECKS(Tag);

			if constexpr (impl::tptr_is_enum_or_integer<Tag>)
			{
				static_assert(sizeof(Tag) * CHAR_BIT >= tag_bit_count,
							  "The destination type is not large enough to store the tag without a loss of data");

				return static_cast<Tag>(base::get_tag(bits_));
			}
			else
			{
				return base::template get_tag_as_object<Tag>(bits_);
			}
		}

		/// \brief	Checks if a tag value can be safely stored without clipping into the pointer bits.
		///
		/// \returns	True if the given tag value is of a compatible type (trivial, integer/enum/POD, small enough, etc.)
		///				and would not collide with any bits in the pointer region.
		template <typename Tag>
		MUU_PURE_INLINE_GETTER
		static constexpr bool can_store_tag(const Tag& tag_value) noexcept
		{
			return base::can_store_tag(tag_value);
		}

		/// \brief	Sets the tag bits, leaving the target pointer value unchanged.
		///
		/// \tparam	Tag			An integer, enum, or a trivial object type small enough to fit.
		/// \param	tag_value	The new value to set for the pointer's tag bits.
		///
		/// \warning If the tag parameter is an integer larger than the available tag bits,
		/// 		 any overflow will be masked out and ignored.
		///
		/// \returns	A reference to the tagged_ptr.
		template <typename Tag>
		tagged_ptr& tag(const Tag& tag_value) noexcept
		{
			bits_ = base::set_tag(bits_, tag_value);
			return *this;
		}

		/// \brief	Returns the value of one of the tag bits.
		MUU_PURE_GETTER
		bool tag_bit(size_t tag_bit_index) const noexcept
		{
			return base::get_tag_bit(bits_, tag_bit_index);
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
		tagged_ptr& tag_bit(size_t tag_bit_index, bool val) noexcept
		{
			bits_ = base::set_tag_bit(bits_, tag_bit_index, val);
			return *this;
		}

		/// \brief	Sets the tag bits to zero, leaving the target pointer value unchanged.
		///
		/// \returns	A reference to the tagged_ptr.
		constexpr tagged_ptr& clear_tag() noexcept
		{
			bits_ &= base::ptr_mask;
			return *this;
		}

		/// \brief	Returns true if the target pointer value is not nullptr.
		MUU_PURE_INLINE_GETTER
		explicit constexpr operator bool() const noexcept
		{
			return bits_ & base::ptr_mask;
		}

		/// \brief	Returns true if a tagged_ptr and raw pointer refer to the same address.
		MUU_CONST_INLINE_GETTER
		friend bool operator==(tagged_ptr lhs, const_pointer rhs) noexcept
		{
			return lhs.ptr() == rhs;
		}

		/// \brief	Returns true if a tagged_ptr and raw pointer do not refer to the same address.
		MUU_CONST_INLINE_GETTER
		friend bool operator!=(tagged_ptr lhs, const_pointer rhs) noexcept
		{
			return lhs.ptr() != rhs;
		}

		/// \brief	Returns true if a tagged_ptr and raw pointer refer to the same address.
		MUU_CONST_INLINE_GETTER
		friend bool operator==(const_pointer lhs, tagged_ptr rhs) noexcept
		{
			return lhs == rhs.ptr();
		}

		/// \brief	Returns true if a tagged_ptr and raw pointer do not refer to the same address.
		MUU_CONST_INLINE_GETTER
		friend bool operator!=(const_pointer lhs, tagged_ptr rhs) noexcept
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

#undef MUU_TAGGED_PTR_TAG_TYPE_CHECKS
#undef MUU_TAGGED_PTR_TAG_OBJECT_CHECKS

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
