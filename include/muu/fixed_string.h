// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::fixed_string.

#include "iterators.h"
#include "meta.h"
#include "impl/core_utils.h"
#include "impl/core_constants.h"
#include "impl/std_string_view.h"
#include "impl/std_iosfwd.h"
#include "impl/std_compare.h"
#include "impl/header_start.h"

#if MUU_HAS_THREE_WAY_COMPARISON && defined(__cpp_nontype_template_args) && __cpp_nontype_template_args >= 201911      \
	&& (!MUU_MSVC || MUU_MSVC >= 1933)
	#define MUU_HAS_FIXED_STRING_LITERALS 1
#else
	#define MUU_HAS_FIXED_STRING_LITERALS 0
#endif

/// \def MUU_HAS_FIXED_STRING_LITERALS
/// \ingroup	preprocessor
/// \brief Indicates whether #muu::fixed_string string literals (e.g. `"hello"_fs`) are supported by the compiler.
/// \see muu::fixed_string

namespace muu
{
	/// \cond
	inline namespace literals
	{
		using namespace std::string_view_literals;
	}
	/// \endcond

	/// \brief	A null-terminated string for compile-time string manipulation.
	/// \ingroup strings
	///
	/// \tparam	Character	The character type of the string.
	/// \tparam	Length 	The length of the string (not including the null terminator).
	template <typename Character, size_t Length>
	class fixed_string
	{
		static_assert(!is_cvref<Character>, "fixed_string Character type cannot be const, volatile, or ref-qualified");
		static_assert(std::is_trivially_constructible_v<Character>	 //
						  && std::is_trivially_copyable_v<Character> //
						  && std::is_trivially_destructible_v<Character>,
					  "fixed_string Character type must be trivially constructible, copyable and destructible");

	  public:
		/// \brief The length of the string (not including the null-terminator).
		static constexpr size_t string_length = Length;

		/// \brief Character type used in the string.
		using value_type = Character;

		/// \brief Traits type describing the string's character type.
		using traits_type = std::char_traits<Character>;

		/// \brief Pointer to one of the string's characters.
		using pointer = Character*;

		/// \brief Constant pointer to one of the string's characters.
		using const_pointer = const Character*;

		/// \brief Reference to one of the string's characters.
		using reference = Character&;

		/// \brief Constant reference to one of the string's characters.
		using const_reference = const Character&;

		/// \brief	Iterator type used when iterating over the string.
		using iterator = pointer;

		/// \brief	Iterator type used when iterating over the string.
		using const_iterator = const_pointer;

		/// \brief	Iterator type used when reverse-iterating over the string.
		using reverse_iterator = std::reverse_iterator<iterator>;

		/// \brief	Iterator type used when reverse-iterating over the string.
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		/// \brief	Size/index type.
		using size_type = size_t;

		/// \brief	Difference type used by iterators.
		using difference_type = ptrdiff_t;

		/// \brief	The string view type corresponding to this string's character type.
		using view_type = std::basic_string_view<Character>;

		/// \cond

		// note that this is public only so this class qualifies as a 'structural type' per C++20's NTTP class rules.
		// don't mess with this directly!
		value_type _chars_[Length + 1_sz];

	  private:
		template <typename, size_t>
		friend class fixed_string;

		using empty_string_type = fixed_string<Character, 0>;

		template <typename Array, size_t... Indices>
		MUU_NODISCARD_CTOR
		explicit constexpr fixed_string(const Array& arr, std::index_sequence<Indices...>) noexcept //
			: _chars_{ arr[Indices]... }
		{
			static_assert(sizeof...(Indices) <= Length);
		}

		template <size_t... Indices>
		MUU_NODISCARD_CTOR
		explicit constexpr fixed_string(value_type fill, std::index_sequence<Indices...>) noexcept //
			: _chars_{ (static_cast<void>(Indices), fill)... }
		{
			static_assert(sizeof...(Indices) <= Length);
		}

		template <size_t... Indices>
		MUU_NODISCARD_CTOR
		explicit constexpr fixed_string(std::string_view str, std::index_sequence<Indices...>) noexcept //
			: _chars_{ (Indices < str.length() ? str[Indices] : '\0')... }
		{
			static_assert(sizeof...(Indices) <= Length);
		}
		/// \endcond

	  public:
#if 1 // constructors --------------------------------------------------------------------------------------------------

		/// \brief Default constructor. Characters are not initialized.
		MUU_NODISCARD_CTOR
		fixed_string() noexcept = default;

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr fixed_string(const fixed_string&) noexcept = default;

		/// \brief Copy-assigment operator.
		constexpr fixed_string& operator=(const fixed_string&) noexcept = default;

		/// \brief Constructs a string from a character array.
		/// \details	Any extra characters not covered by the input argument are zero-initialized.
		MUU_CONSTRAINED_TEMPLATE(N != 0 && Length != 0, size_t N)
		MUU_NODISCARD_CTOR
		explicit constexpr fixed_string(const value_type (&arr)[N]) noexcept //
			: fixed_string{ arr, std::make_index_sequence<min(Length, N)>{} }
		{}

		/// \cond
		MUU_CONSTRAINED_TEMPLATE(N == 0 || Length == 0, size_t N)
		MUU_NODISCARD_CTOR
		explicit constexpr fixed_string(const value_type (&)[N]) noexcept //
			: _chars_{}
		{}
		/// \endcond

		/// \brief Constructs a string from a std::string_view.
		/// \details	Any extra characters not covered by the input argument are zero-initialized.
		MUU_HIDDEN_CONSTRAINT(!!Len, size_t Len = Length)
		MUU_NODISCARD_CTOR
		explicit constexpr fixed_string(std::string_view str) noexcept //
			: fixed_string{ str, std::make_index_sequence<Length>{} }
		{}

		/// \cond
		MUU_HIDDEN_CONSTRAINT(!Len, size_t Len = Length)
		MUU_NODISCARD_CTOR
		explicit constexpr fixed_string(std::string_view) noexcept //
			: _chars_{}
		{}
		/// \endcond

		/// \brief		Lengthening/truncating constructor.
		/// \details	Any extra characters not covered by the input argument are zero-initialized.
		MUU_CONSTRAINED_TEMPLATE(Len != 0 && Length != 0, size_t Len)
		MUU_NODISCARD_CTOR
		explicit constexpr fixed_string(const fixed_string<value_type, Len>& str) noexcept //
			: fixed_string{ str, std::make_index_sequence<min(Length, Len)>{} }
		{}

		/// \cond
		MUU_CONSTRAINED_TEMPLATE(Len == 0 || Length == 0, size_t Len)
		MUU_NODISCARD_CTOR
		explicit constexpr fixed_string(const fixed_string<value_type, Len>&) noexcept //
			: _chars_{}
		{}
		/// \endcond

		/// \brief Constructs a string with each character equal to the given value.
		MUU_NODISCARD_CTOR
		explicit constexpr fixed_string(value_type fill) noexcept //
			: fixed_string{ fill, std::make_index_sequence<Length>{} }
		{}

		/// \brief Constructs a string from a raw c-string pointer and integral length constant.
		MUU_CONSTRAINED_TEMPLATE(Len != 0 && Length != 0, size_t Len)
		MUU_NODISCARD_CTOR
		MUU_ATTR(nonnull)
		explicit constexpr fixed_string(const value_type* str, index_tag<Len>) noexcept
			: fixed_string{ str, std::make_index_sequence<min(Length, Len)>{} }
		{}

		/// \cond
		MUU_CONSTRAINED_TEMPLATE(Len == 0 || Length == 0, size_t Len)
		MUU_NODISCARD_CTOR
		explicit constexpr fixed_string(const value_type*, index_tag<Len>) noexcept //
			: _chars_{}
		{}
		/// \endcond

#endif // constructors

#if 1 // characters ----------------------------------------------------------------------------------------------------
		/// \name Characters
		/// @{

		/// \brief Returns a reference to the first character in the string.
		///
		/// \availability		This function is not available when #string_length == 0.
		MUU_HIDDEN_CONSTRAINT(!!Len, size_t Len = Length)
		MUU_PURE_INLINE_GETTER
		constexpr reference front() noexcept
		{
			return _chars_[0];
		}

		/// \brief Returns a reference to the last character in the string.
		///
		/// \availability		This function is not available when #string_length == 0.
		MUU_HIDDEN_CONSTRAINT(!!Len, size_t Len = Length)
		MUU_PURE_INLINE_GETTER
		constexpr reference back() noexcept
		{
			return _chars_[Length - 1_sz];
		}

		/// \brief Returns a const reference to the first character in the string.
		///
		/// \availability		This function is not available when #string_length == 0.
		MUU_HIDDEN_CONSTRAINT(!!Len, size_t Len = Length)
		MUU_PURE_INLINE_GETTER
		constexpr const_reference front() const noexcept
		{
			return _chars_[0];
		}

		/// \brief Returns a const reference to the last character in the string.
		///
		/// \availability		This function is not available when #string_length == 0.
		MUU_HIDDEN_CONSTRAINT(!!Len, size_t Len = Length)
		MUU_PURE_INLINE_GETTER
		constexpr const_reference back() const noexcept
		{
			return _chars_[Length - 1_sz];
		}

		/// \brief	Returns a reference to the character at the given index.
		template <size_t Index>
		MUU_PURE_INLINE_GETTER
		constexpr reference get() noexcept
		{
			static_assert(Index < Length, "Character index out of range");

			return _chars_[Index];
		}

		/// \brief	Returns a const reference to the character at the given index.
		template <size_t Index>
		MUU_PURE_INLINE_GETTER
		constexpr const_reference get() const noexcept
		{
			static_assert(Index < Length, "Character index out of range");

			return _chars_[Index];
		}

		/// \brief	Returns a reference to the character at the given index.
		MUU_PURE_INLINE_GETTER
		constexpr reference operator[](size_type idx) noexcept
		{
			MUU_ASSUME(idx < Length);

			return _chars_[idx];
		}

		/// \brief	Returns a const reference to the character at the given index.
		MUU_PURE_INLINE_GETTER
		constexpr const_reference operator[](size_type idx) const noexcept
		{
			MUU_ASSUME(idx < Length);

			return _chars_[idx];
		}

		/// \brief	Returns a pointer to the first character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr pointer data() noexcept
		{
			return _chars_;
		}

		/// \brief	Returns a const pointer to the first character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr const_pointer data() const noexcept
		{
			return _chars_;
		}

		/// \brief	Returns a const pointer to the first character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr const_pointer c_str() const noexcept
		{
			return _chars_;
		}

		/// @}
#endif // characters

#if 1 // length --------------------------------------------------------------------------------------------------------
		/// \name Length
		/// @{

		/// \brief Returns true if the string is empty.
		MUU_PURE_INLINE_GETTER
		constexpr bool empty() const noexcept
		{
			return Length == 0_sz;
		}

		/// \brief Returns true if the string is not empty.
		MUU_PURE_INLINE_GETTER
		explicit constexpr operator bool() const noexcept
		{
			return Length > 0_sz;
		}

		/// \brief Returns the number of characters in the string (not including the null-terminator).
		MUU_PURE_INLINE_GETTER
		constexpr size_type size() const noexcept
		{
			return Length;
		}

		/// \brief Returns the number of characters in the string (not including the null-terminator).
		MUU_PURE_INLINE_GETTER
		constexpr size_type length() const noexcept
		{
			return Length;
		}

		/// @}
#endif // length

#if 1 // iterators -----------------------------------------------------------------------------------------------------
		/// \name Iterators
		/// @{

		/// \brief Returns an iterator to the first character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr iterator begin() noexcept
		{
			return _chars_;
		}

		/// \brief Returns an iterator to the one-past-the-last character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr iterator end() noexcept
		{
			return _chars_ + Length;
		}

		/// \brief Returns a const iterator to the first character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr const_iterator begin() const noexcept
		{
			return _chars_;
		}

		/// \brief Returns a const iterator to the one-past-the-last character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr const_iterator end() const noexcept
		{
			return _chars_ + Length;
		}

		/// \brief Returns a const iterator to the first character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr const_iterator cbegin() const noexcept
		{
			return _chars_;
		}

		/// \brief Returns a const iterator to the one-past-the-last character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr const_iterator cend() const noexcept
		{
			return _chars_ + Length;
		}

		/// \brief Returns a reverse iterator to the last character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr reverse_iterator rbegin() noexcept
		{
			return std::make_reverse_iterator(end());
		}

		/// \brief Returns a reverse iterator to one-before-the-first character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr reverse_iterator rend() noexcept
		{
			return std::make_reverse_iterator(begin());
		}

		/// \brief Returns a const reverse iterator to the last character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr const_reverse_iterator rbegin() const noexcept
		{
			return std::make_reverse_iterator(end());
		}

		/// \brief Returns a const reverse iterator to one-before-the-first character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr const_reverse_iterator rend() const noexcept
		{
			return std::make_reverse_iterator(begin());
		}

		/// \brief Returns a const reverse iterator to the last character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr const_reverse_iterator crbegin() const noexcept
		{
			return std::make_reverse_iterator(end());
		}

		/// \brief Returns a const reverse iterator to one-before-the-first character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr const_reverse_iterator crend() const noexcept
		{
			return std::make_reverse_iterator(begin());
		}

		/// @}

		/// \name Iterators (ADL)
		/// @{

		/// \brief Returns an iterator to the first character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr iterator begin(fixed_string& s) noexcept
		{
			return s.begin();
		}

		/// \brief Returns an iterator to the one-past-the-last character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr iterator end(fixed_string& s) noexcept
		{
			return s.end();
		}

		/// \brief Returns a const iterator to the first character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_iterator begin(const fixed_string& s) noexcept
		{
			return s.begin();
		}

		/// \brief Returns a const iterator to the one-past-the-last character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_iterator end(const fixed_string& s) noexcept
		{
			return s.end();
		}

		/// \brief Returns a const iterator to the first character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_iterator cbegin(const fixed_string& s) noexcept
		{
			return s.begin();
		}

		/// \brief Returns a const iterator to the one-past-the-last character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_iterator cend(const fixed_string& s) noexcept
		{
			return s.end();
		}

		/// \brief Returns a reverse iterator to the last character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr reverse_iterator rbegin(fixed_string& s) noexcept
		{
			return s.rbegin();
		}

		/// \brief Returns a reverse iterator to one-before-the-first character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr reverse_iterator rend(fixed_string& s) noexcept
		{
			return s.rend();
		}

		/// \brief Returns a const reverse iterator to the last character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_reverse_iterator rbegin(const fixed_string& s) noexcept
		{
			return s.rbegin();
		}

		/// \brief Returns a const reverse iterator to one-before-the-first character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_reverse_iterator rend(const fixed_string& s) noexcept
		{
			return s.rend();
		}

		/// \brief Returns a const reverse iterator to the last character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_reverse_iterator crbegin(const fixed_string& s) noexcept
		{
			return s.rbegin();
		}

		/// \brief Returns a const reverse iterator to one-before-the-first character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_reverse_iterator crend(const fixed_string& s) noexcept
		{
			return s.rend();
		}

		/// @}
#endif // iterators

#if 1 // concatenation -------------------------------------------------------------------------------------------------
		/// \name Concatenation
		/// @{

		/// \brief Concatenates two static strings.
		template <size_t Len>
		MUU_PURE_GETTER
		friend constexpr fixed_string<value_type, Length + Len> operator+(
			const fixed_string& lhs,
			const fixed_string<value_type, Len>& rhs) noexcept
		{
			if constexpr (!Length && !Len)
				return empty_string_type{};
			else if constexpr (!Len)
				return fixed_string<value_type, Length>{ lhs };
			else if constexpr (!Length)
				return fixed_string<value_type, Len>{ rhs };
			else
			{
				auto result = fixed_string<value_type, Length + Len>{ lhs };
				for (size_t i = 0; i < Len; i++)
					result[Length + i] = rhs[i];
				return result;
			}
		}

		/// \brief Concatenates a static string and a single character.
		MUU_PURE_GETTER
		friend constexpr fixed_string<value_type, Length + 1> operator+(const fixed_string& lhs,
																		value_type rhs) noexcept
		{
			auto result	   = fixed_string<value_type, Length + 1>{ lhs };
			result[Length] = rhs;
			return result;
		}

		/// \brief Concatenates a static string and a single character.
		MUU_PURE_GETTER
		friend constexpr fixed_string<value_type, Length + 1> operator+(value_type lhs,
																		const fixed_string& rhs) noexcept
		{
			auto result = fixed_string<value_type, Length + 1>{ lhs };
			if constexpr (Length > 0_sz)
			{
				for (size_t i = 0; i < Length; i++)
					result[i + 1] = rhs[i];
			}
			return result;
		}

		/// @}
#endif // concatenation

#if 1 // views ----------------------------------------------------------------------------------------------------
		/// \name Views
		/// @{

		/// \brief	Returns a view of the string (not including the null terminator).
		MUU_PURE_INLINE_GETTER
		constexpr view_type view() const noexcept
		{
			return view_type{ _chars_, Length };
		}

		/// \brief	Returns a view of the string (not including the null terminator).
		MUU_PURE_INLINE_GETTER
		constexpr operator view_type() const noexcept
		{
			return view_type{ _chars_, Length };
		}

		/// @}
#endif // views

#if 1	// substrings -------------------------------------------------------------------------------------------------
		/// \name Substrings
		/// @{

		/// \cond
	  private:
		template <typename T>
		MUU_PURE_GETTER
		MUU_CONSTEVAL
		static size_t clamp_index(T idx) noexcept
		{
			if constexpr (!Length)
				return 0_sz;
			else
			{
				if constexpr (is_signed<T>)
				{
					if (idx < T{})
					{
						if (idx == constants<T>::lowest)
							return 0_sz;

						const auto abs_uidx = static_cast<size_t>(-idx);
						if (abs_uidx >= Length)
							return 0_sz;
						else
							return Length - abs_uidx;
					}
				}
				return clamp(static_cast<size_t>(idx), 0_sz, Length);
			}
		}

		// Q: "why are these impl functions necessary?"
		// A: A clang bug caused the auto NTTP arguments to never be deduced properly in constexpr

		template <size_t Start, size_t Len>
		MUU_PURE_GETTER
		constexpr auto substr_impl() const noexcept
		{
			if constexpr (!Length || !Len || Start >= Length)
				return empty_string_type{};
			else
			{
				constexpr size_t substring_end	  = min(Start + min(Length, Len), Length);
				constexpr size_t substring_length = substring_end - Start;
				if constexpr (!substring_length)
					return empty_string_type{};
				else if constexpr (!Start)
					return fixed_string<value_type, substring_length>{ *this };
				else
					return fixed_string<value_type, substring_length>{ _chars_ + Start, index_tag<substring_length>{} };
			}
		}

		template <size_t Start, size_t End>
		MUU_PURE_GETTER
		constexpr auto slice_impl() const noexcept
		{
			if constexpr (End <= Start)
				return empty_string_type{};
			else
				return substr_impl<Start, End - Start>();
		}

	  public:
		/// \endcond

		/// \brief	Returns a substring.
		///
		/// \tparam	Start	Starting index of the substring. Negative values mean "this many characters from the end".
		/// \tparam	Len		Length of the substring.
		///
		/// \return	A string containing the substring from the given range.
		/// 		If the source string literal was empty, the range was empty, or
		/// 		they do not intersect, an zero-length string literal is returned.
		template <auto Start, size_t Len = static_cast<size_t>(-1)>
		MUU_PURE_GETTER
		constexpr auto substr() const noexcept
		{
			using start_type = decltype(Start);
			static_assert(is_integral<start_type> && is_arithmetic<start_type>, "Start must be an integral type.");

			return substr_impl<clamp_index(Start), Len>();
		}

		/// \brief	Returns a substring.
		///
		/// \tparam	Start	 	Starting index of the substring, inclusive. Negative values mean "this many characters from the end".
		/// \tparam	End			Ending index of the substring, exclusive. Negative values mean "this many characters from the end".
		///
		/// \return	A string containing the substring from the given range.
		/// 		If the source string literal was empty, the range was empty, or
		/// 		they do not intersect, an zero-length string is returned.
		template <auto Start, auto End = static_cast<size_t>(-1)>
		MUU_PURE_GETTER
		constexpr auto slice() const noexcept
		{
			using start_type = decltype(Start);
			static_assert(is_integral<start_type> && is_arithmetic<start_type>, "Start must be an integral type.");

			using end_type = decltype(End);
			static_assert(is_integral<end_type> && is_arithmetic<end_type>, "End must be an integral type.");

			return slice_impl<clamp_index(Start), clamp_index(End)>();
		}

		/// @}
#endif // substrings

#if 1 // equality -------------------------------------------------------------------------------------------------
		/// \name Equality and Comparison
		/// @{

		/// \brief	Returns true if two strings contain the same character sequence.
		template <size_t Len>
		MUU_PURE_GETTER
		friend constexpr bool operator==(const fixed_string& lhs, const fixed_string<value_type, Len>& rhs) noexcept
		{
			if constexpr (Length == Len && Length > 0_sz)
			{
				if (&lhs == &rhs)
					return true;

				for (size_t i = 0; i < Length; i++)
					if (lhs[i] != rhs[i])
						return false;

				return true;
			}
			else
			{
				MUU_UNUSED(lhs);
				MUU_UNUSED(rhs);

				return Length == Len; // empty strings
			}
		}

		/// \brief	Returns true if two strings do not contain the same character sequence.
		template <size_t Len>
		MUU_PURE_GETTER
		friend constexpr bool operator!=(const fixed_string& lhs, const fixed_string<value_type, Len>& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		/// \brief	Returns the lexicographical ordering of two strings.
		template <size_t Len>
		MUU_PURE_GETTER
		static constexpr int compare(const fixed_string& lhs, const fixed_string<value_type, Len>& rhs) noexcept
		{
			// neither empty
			if constexpr (!!Length && !!Len)
			{
				// same length
				if constexpr (Length == Len)
				{
					if (&lhs == &rhs)
						return true;

					return traits_type::compare(lhs._chars_, rhs.c_str(), Length);
				}

				// different lengths
				else
				{
					constexpr size_t compare_length = min(Length, Len);
					constexpr int eq_result			= (Length < Len ? -1 : 1);
					const auto result				= traits_type::compare(lhs._chars_, rhs.c_str(), compare_length);
					return result == 0 ? eq_result : result;
				}
			}
			else
			{
				MUU_UNUSED(lhs);
				MUU_UNUSED(rhs);

				// both empty
				if constexpr (Length == Len)
					return 0;

				// lhs empty
				else if constexpr (!Length)
					return -1;

				// rhs empty
				else
					return 1;
			}
		}

		/// \brief	Returns the lexicographical ordering of this string with respect to another.
		template <size_t Len>
		MUU_PURE_GETTER
		constexpr int compare(const fixed_string<value_type, Len>& rhs) noexcept
		{
			return compare(*this, rhs);
		}

		/// \brief	Returns true if the LHS is lexicographically ordered before the RHS.
		template <size_t Len>
		MUU_PURE_GETTER
		friend constexpr bool operator<(const fixed_string& lhs, const fixed_string<value_type, Len>& rhs) noexcept
		{
			return compare(lhs, rhs) < 0;
		}

		/// \brief	Returns true if the LHS is lexicographically ordered before or equal to the RHS.
		template <size_t Len>
		MUU_PURE_GETTER
		friend constexpr bool operator<=(const fixed_string& lhs, const fixed_string<value_type, Len>& rhs) noexcept
		{
			return compare(lhs, rhs) <= 0;
		}

		/// \brief	Returns true if the LHS is lexicographically ordered after than the RHS.
		template <size_t Len>
		MUU_PURE_GETTER
		friend constexpr bool operator>(const fixed_string& lhs, const fixed_string<value_type, Len>& rhs) noexcept
		{
			return compare(lhs, rhs) > 0;
		}

		/// \brief	Returns true if the LHS is lexicographically ordered after or equal to the RHS.
		template <size_t Len>
		MUU_PURE_GETTER
		friend constexpr bool operator>=(const fixed_string& lhs, const fixed_string<value_type, Len>& rhs) noexcept
		{
			return compare(lhs, rhs) >= 0;
		}

	#if MUU_HAS_THREE_WAY_COMPARISON
		friend constexpr auto operator<=>(const fixed_string&, const fixed_string&) noexcept = default;
	#endif

		/// @}
#endif // equality

#if 1 // misc -----------------------------------------------------------------------------------------------------

		/// \brief Writes the string out to a text stream.
		template <typename Char, typename Traits>
		friend std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os,
															const fixed_string& str)
		{
			if constexpr (has_bitwise_lsh_operator<std::basic_ostream<Char, Traits>&, view_type>)
				return os << str.view();
			else if constexpr (has_bitwise_lsh_operator<std::basic_ostream<Char, Traits>&, const value_type*> //
							   || has_bitwise_lsh_operator<std::basic_ostream<Char, Traits>&,
														   const value_type(&)[Length + 1_sz]>)
				return os << str._chars_;
			else
				static_assert(always_false<Char>, "stream operator not supported with this stream and character type");
		}

#endif // misc
	};

	/// \cond

	MUU_CONSTRAINED_TEMPLATE(is_character<T>, typename T, size_t N)
	fixed_string(const T (&)[N])->fixed_string<T, N - 1>;

	MUU_CONSTRAINED_TEMPLATE(is_character<T>, typename T)
	fixed_string(T)->fixed_string<T, 1>;

#if MUU_HAS_FIXED_STRING_LITERALS
	namespace impl
	{
		template <typename Char, size_t Length>
		struct fixed_string_udl_impl
		{
			fixed_string<Char, Length - 1> value;

			constexpr fixed_string_udl_impl(const Char (&str)[Length]) noexcept //
				: value{ str }
			{}

			friend constexpr auto operator<=>(const fixed_string_udl_impl&,
											  const fixed_string_udl_impl&) noexcept = default;
		};

		template <typename T, size_t N>
		fixed_string_udl_impl(const T (&)[N]) -> fixed_string_udl_impl<T, N>;
	}
#endif // MUU_HAS_FIXED_STRING_LITERALS

	/// \endcond

	inline namespace literals
	{
#if MUU_DOXYGEN || MUU_HAS_FIXED_STRING_LITERALS

		/// \brief	Constructs a fixed_string directly using a string literal.
		/// \detail \cpp
		/// constexpr auto str = "hello"_fs;
		/// static_assert(str == fixed_string{ "hello" })
		/// \ecpp
		///
		/// \availability This operator depends on C++20's class-type NTTPs and three-way comparison operator.
		/// Check for support using #MUU_HAS_FIXED_STRING_LITERALS and/or #build::supports_fixed_string_literals.
		///
		/// \relatesalso muu::fixed_string
		template <impl::fixed_string_udl_impl Str>
		MUU_CONSTEVAL
		auto operator"" _fs()
		{
			return Str.value;
		}
#endif
	}

	namespace build
	{
		/// \brief Indicates whether #muu::fixed_string string literals (e.g. `"hello"_fs`) are supported by the compiler.
		/// \see muu::fixed_string
		inline constexpr bool supports_fixed_string_literals = !!MUU_HAS_FIXED_STRING_LITERALS;
	}
}

#include "impl/header_end.h"
