// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::static_string.

#include "iterators.h"
#include "meta.h"
#include "impl/core_utils.h"
#include "impl/core_constants.h"
#include "impl/std_string_view.h"
#include "impl/std_iosfwd.h"
#include "impl/header_start.h"

#if !defined(DOXYGEN) && defined(__cpp_nontype_template_args) && __cpp_nontype_template_args >= 201911                 \
	&& (!MUU_MSVC || MUU_MSVC > 1931)
	#define MUU_STATIC_STRING_CHAR_UDL 1
#else
	#define MUU_STATIC_STRING_CHAR_UDL 0
#endif

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
	class static_string
	{
		static_assert(!is_cvref<Character>, "static_string Character type cannot be const, volatile, or ref-qualified");
		static_assert(std::is_trivially_constructible_v<Character>	 //
						  && std::is_trivially_copyable_v<Character> //
						  && std::is_trivially_destructible_v<Character>,
					  "static_string Character type must be trivially constructible, copyable and destructible");

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

	  private:
		template <typename, size_t>
		friend class static_string;

		using empty_string_type = static_string<Character, 0>;

		value_type chars_[Length + 1_sz];

		template <typename Array, size_t... Indices>
		MUU_NODISCARD_CTOR
		explicit constexpr static_string(const Array& arr, std::index_sequence<Indices...>) noexcept //
			: chars_{ arr[Indices]... }
		{
			static_assert(sizeof...(Indices) <= Length);
		}

		template <size_t... Indices>
		MUU_NODISCARD_CTOR
		explicit constexpr static_string(value_type fill, std::index_sequence<Indices...>) noexcept //
			: chars_{ (static_cast<void>(Indices), fill)... }
		{
			static_assert(sizeof...(Indices) <= Length);
		}

		template <size_t... Indices>
		MUU_NODISCARD_CTOR
		explicit constexpr static_string(std::string_view str, std::index_sequence<Indices...>) noexcept //
			: chars_{ (Indices < str.length() ? str[Indices] : '\0')... }
		{
			static_assert(sizeof...(Indices) <= Length);
		}

	  public:
#if 1 // constructors --------------------------------------------------------------------------------------------------

		/// \brief Default constructor. Characters are not initialized.
		MUU_NODISCARD_CTOR
		static_string() noexcept = default;

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr static_string(const static_string&) noexcept = default;

		/// \brief Copy-assigment operator.
		constexpr static_string& operator=(const static_string&) noexcept = default;

		/// \brief Constructs a string from a character array.
		/// \details	Any extra characters not covered by the input argument are zero-initialized.
		MUU_CONSTRAINED_TEMPLATE(N != 0 && Length != 0, size_t N)
		MUU_NODISCARD_CTOR
		explicit constexpr static_string(const value_type (&arr)[N]) noexcept //
			: static_string{ arr, std::make_index_sequence<min(Length, N)>{} }
		{}

		/// \cond
		MUU_CONSTRAINED_TEMPLATE(N == 0 || Length == 0, size_t N)
		MUU_NODISCARD_CTOR
		explicit constexpr static_string(const value_type (&)[N]) noexcept //
			: chars_{}
		{}
		/// \endcond

		/// \brief Constructs a string from a std::string_view.
		/// \details	Any extra characters not covered by the input argument are zero-initialized.
		MUU_LEGACY_REQUIRES(!!Len, size_t Len = Length)
		MUU_NODISCARD_CTOR
		explicit constexpr static_string(std::string_view str) noexcept //
			: static_string{ str, std::make_index_sequence<Length>{} }
		{}

		/// \cond
		MUU_LEGACY_REQUIRES(!Len, size_t Len = Length)
		MUU_NODISCARD_CTOR
		explicit constexpr static_string(std::string_view) noexcept //
			: chars_{}
		{}
		/// \endcond

		/// \brief		Lengthening/truncating constructor.
		/// \details	Any extra characters not covered by the input argument are zero-initialized.
		MUU_CONSTRAINED_TEMPLATE(Len != 0 && Length != 0, size_t Len)
		MUU_NODISCARD_CTOR
		explicit constexpr static_string(const static_string<value_type, Len>& str) noexcept //
			: static_string{ str, std::make_index_sequence<min(Length, Len)>{} }
		{}

		/// \cond
		MUU_CONSTRAINED_TEMPLATE(Len == 0 || Length == 0, size_t Len)
		MUU_NODISCARD_CTOR
		explicit constexpr static_string(const static_string<value_type, Len>&) noexcept //
			: chars_{}
		{}
		/// \endcond

		/// \brief Constructs a string with each character equal to the given value.
		MUU_NODISCARD_CTOR
		explicit constexpr static_string(value_type fill) noexcept //
			: static_string{ fill, std::make_index_sequence<Length>{} }
		{}

		/// \brief Constructs a string from a raw c-string pointer and integral length constant.
		MUU_CONSTRAINED_TEMPLATE(Len != 0 && Length != 0, size_t Len)
		MUU_NODISCARD_CTOR
		MUU_ATTR(nonnull)
		explicit constexpr static_string(const value_type* str, index_tag<Len>) noexcept
			: static_string{ str, std::make_index_sequence<min(Length, Len)>{} }
		{}

		/// \cond
		MUU_CONSTRAINED_TEMPLATE(Len == 0 || Length == 0, size_t Len)
		MUU_NODISCARD_CTOR
		explicit constexpr static_string(const value_type*, index_tag<Len>) noexcept //
			: chars_{}
		{}
		/// \endcond

#endif // constructors

#if 1 // characters ----------------------------------------------------------------------------------------------------
		/// \name Characters
		/// @{

		/// \brief Returns a reference to the first character in the string.
		///
		/// \availability		This function is not available when #string_length == 0.
		MUU_LEGACY_REQUIRES(!!Len, size_t Len = Length)
		MUU_PURE_INLINE_GETTER
		constexpr reference front() noexcept
		{
			return chars_[0];
		}

		/// \brief Returns a reference to the last character in the string.
		///
		/// \availability		This function is not available when #string_length == 0.
		MUU_LEGACY_REQUIRES(!!Len, size_t Len = Length)
		MUU_PURE_INLINE_GETTER
		constexpr reference back() noexcept
		{
			return chars_[Length - 1_sz];
		}

		/// \brief Returns a const reference to the first character in the string.
		///
		/// \availability		This function is not available when #string_length == 0.
		MUU_LEGACY_REQUIRES(!!Len, size_t Len = Length)
		MUU_PURE_INLINE_GETTER
		constexpr const_reference front() const noexcept
		{
			return chars_[0];
		}

		/// \brief Returns a const reference to the last character in the string.
		///
		/// \availability		This function is not available when #string_length == 0.
		MUU_LEGACY_REQUIRES(!!Len, size_t Len = Length)
		MUU_PURE_INLINE_GETTER
		constexpr const_reference back() const noexcept
		{
			return chars_[Length - 1_sz];
		}

		/// \brief	Returns a reference to the character at the given index.
		template <size_t Index>
		MUU_PURE_INLINE_GETTER
		constexpr reference get() noexcept
		{
			static_assert(Index < Length, "Character index out of range");

			return chars_[Index];
		}

		/// \brief	Returns a const reference to the character at the given index.
		template <size_t Index>
		MUU_PURE_INLINE_GETTER
		constexpr const_reference get() const noexcept
		{
			static_assert(Index < Length, "Character index out of range");

			return chars_[Index];
		}

		/// \brief	Returns a reference to the character at the given index.
		MUU_PURE_INLINE_GETTER
		constexpr reference operator[](size_type idx) noexcept
		{
			MUU_ASSUME(idx < Length);

			return chars_[idx];
		}

		/// \brief	Returns a const reference to the character at the given index.
		MUU_PURE_INLINE_GETTER
		constexpr const_reference operator[](size_type idx) const noexcept
		{
			MUU_ASSUME(idx < Length);

			return chars_[idx];
		}

		/// \brief	Returns a pointer to the first character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr pointer data() noexcept
		{
			return chars_;
		}

		/// \brief	Returns a const pointer to the first character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr const_pointer data() const noexcept
		{
			return chars_;
		}

		/// \brief	Returns a const pointer to the first character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr const_pointer c_str() const noexcept
		{
			return chars_;
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
			return chars_;
		}

		/// \brief Returns an iterator to the one-past-the-last character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr iterator end() noexcept
		{
			return chars_ + Length;
		}

		/// \brief Returns a const iterator to the first character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr const_iterator begin() const noexcept
		{
			return chars_;
		}

		/// \brief Returns a const iterator to the one-past-the-last character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr const_iterator end() const noexcept
		{
			return chars_ + Length;
		}

		/// \brief Returns a const iterator to the first character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr const_iterator cbegin() const noexcept
		{
			return chars_;
		}

		/// \brief Returns a const iterator to the one-past-the-last character in the string.
		MUU_PURE_INLINE_GETTER
		constexpr const_iterator cend() const noexcept
		{
			return chars_ + Length;
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
		friend constexpr iterator begin(static_string& s) noexcept
		{
			return s.begin();
		}

		/// \brief Returns an iterator to the one-past-the-last character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr iterator end(static_string& s) noexcept
		{
			return s.end();
		}

		/// \brief Returns a const iterator to the first character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_iterator begin(const static_string& s) noexcept
		{
			return s.begin();
		}

		/// \brief Returns a const iterator to the one-past-the-last character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_iterator end(const static_string& s) noexcept
		{
			return s.end();
		}

		/// \brief Returns a const iterator to the first character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_iterator cbegin(const static_string& s) noexcept
		{
			return s.begin();
		}

		/// \brief Returns a const iterator to the one-past-the-last character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_iterator cend(const static_string& s) noexcept
		{
			return s.end();
		}

		/// \brief Returns a reverse iterator to the last character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr reverse_iterator rbegin(static_string& s) noexcept
		{
			return s.rbegin();
		}

		/// \brief Returns a reverse iterator to one-before-the-first character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr reverse_iterator rend(static_string& s) noexcept
		{
			return s.rend();
		}

		/// \brief Returns a const reverse iterator to the last character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_reverse_iterator rbegin(const static_string& s) noexcept
		{
			return s.rbegin();
		}

		/// \brief Returns a const reverse iterator to one-before-the-first character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_reverse_iterator rend(const static_string& s) noexcept
		{
			return s.rend();
		}

		/// \brief Returns a const reverse iterator to the last character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_reverse_iterator crbegin(const static_string& s) noexcept
		{
			return s.rbegin();
		}

		/// \brief Returns a const reverse iterator to one-before-the-first character in a string.
		MUU_PURE_INLINE_GETTER
		friend constexpr const_reverse_iterator crend(const static_string& s) noexcept
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
		friend constexpr static_string<value_type, Length + Len> operator+(
			const static_string& lhs,
			const static_string<value_type, Len>& rhs) noexcept
		{
			if constexpr (!Length && !Len)
				return empty_string_type{};
			else if constexpr (!Len)
				return static_string<value_type, Length>{ lhs };
			else if constexpr (!Length)
				return static_string<value_type, Len>{ rhs };
			else
			{
				auto result = static_string<value_type, Length + Len>{ lhs };
				for (size_t i = 0; i < Len; i++)
					result[Length + i] = rhs[i];
				return result;
			}
		}

		/// \brief Concatenates a static string and a single character.
		MUU_PURE_GETTER
		friend constexpr static_string<value_type, Length + 1> operator+(const static_string& lhs,
																		 value_type rhs) noexcept
		{
			auto result	   = static_string<value_type, Length + 1>{ lhs };
			result[Length] = rhs;
			return result;
		}

		/// \brief Concatenates a static string and a single character.
		MUU_PURE_GETTER
		friend constexpr static_string<value_type, Length + 1> operator+(value_type lhs,
																		 const static_string& rhs) noexcept
		{
			auto result = static_string<value_type, Length + 1>{ lhs };
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
			return view_type{ chars_, Length };
		}

		/// \brief	Returns a view of the string (not including the null terminator).
		MUU_PURE_INLINE_GETTER
		constexpr operator view_type() const noexcept
		{
			return view_type{ chars_, Length };
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
					return static_string<value_type, substring_length>{ *this };
				else
					return static_string<value_type, substring_length>{ chars_ + Start, index_tag<substring_length>{} };
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
		friend constexpr bool operator==(const static_string& lhs, const static_string<value_type, Len>& rhs) noexcept
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
		friend constexpr bool operator!=(const static_string& lhs, const static_string<value_type, Len>& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		/// \brief	Returns the lexicographical ordering of two strings.
		template <size_t Len>
		MUU_PURE_GETTER
		static constexpr int compare(const static_string& lhs, const static_string<value_type, Len>& rhs) noexcept
		{
			// neither empty
			if constexpr (!!Length && !!Len)
			{
				// same length
				if constexpr (Length == Len)
				{
					if (&lhs == &rhs)
						return true;

					return traits_type::compare(lhs.chars_, rhs.c_str(), Length);
				}

				// different lengths
				else
				{
					constexpr size_t compare_length = min(Length, Len);
					constexpr int eq_result			= (Length < Len ? -1 : 1);
					const auto result				= traits_type::compare(lhs.chars_, rhs.c_str(), compare_length);
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
		constexpr int compare(const static_string<value_type, Len>& rhs) noexcept
		{
			return compare(*this, rhs);
		}

		/// \brief	Returns true if the LHS is lexicographically ordered before the RHS.
		template <size_t Len>
		MUU_PURE_GETTER
		friend constexpr bool operator<(const static_string& lhs, const static_string<value_type, Len>& rhs) noexcept
		{
			return compare(lhs, rhs) < 0;
		}

		/// \brief	Returns true if the LHS is lexicographically ordered before or equal to the RHS.
		template <size_t Len>
		MUU_PURE_GETTER
		friend constexpr bool operator<=(const static_string& lhs, const static_string<value_type, Len>& rhs) noexcept
		{
			return compare(lhs, rhs) <= 0;
		}

		/// \brief	Returns true if the LHS is lexicographically ordered after than the RHS.
		template <size_t Len>
		MUU_PURE_GETTER
		friend constexpr bool operator>(const static_string& lhs, const static_string<value_type, Len>& rhs) noexcept
		{
			return compare(lhs, rhs) > 0;
		}

		/// \brief	Returns true if the LHS is lexicographically ordered after or equal to the RHS.
		template <size_t Len>
		MUU_PURE_GETTER
		friend constexpr bool operator>=(const static_string& lhs, const static_string<value_type, Len>& rhs) noexcept
		{
			return compare(lhs, rhs) >= 0;
		}

		/// @}
#endif // equality

#if 1 // misc -----------------------------------------------------------------------------------------------------

		/// \brief Writes the string out to a text stream.
		template <typename Char, typename Traits>
		friend std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os,
															const static_string& str)
		{
			if constexpr (has_bitwise_lsh_operator<std::basic_ostream<Char, Traits>&, view_type>)
				return os << str.view();
			else if constexpr (has_bitwise_lsh_operator<std::basic_ostream<Char, Traits>&, const value_type*> //
							   || has_bitwise_lsh_operator<std::basic_ostream<Char, Traits>&,
														   const value_type(&)[Length + 1_sz]>)
				return os << str.chars_;
			else
				static_assert(always_false<Char>, "stream operator not supported with this stream and character type");
		}

#endif // misc
	};

	/// \cond

	MUU_CONSTRAINED_TEMPLATE(is_character<T>, typename T, size_t N)
	static_string(const T (&)[N])->static_string<T, N - 1>;

	MUU_CONSTRAINED_TEMPLATE(is_character<T>, typename T)
	static_string(T)->static_string<T, 1>;

#if MUU_STATIC_STRING_CHAR_UDL
	namespace impl
	{
		template <size_t Length>
		struct static_string_udl_impl
		{
			static_string<char, Length> value;

			constexpr static_string_udl_impl(const char (&str)[Length + 1]) //
				: value{ str }
			{}
		};
	}
#endif // MUU_STATIC_STRING_CHAR_UDL

	/// \endcond

	inline namespace literals
	{
		/// \brief	Constructs a static_string exactly representing a numeric literal.
		/// \detail \cpp
		/// constexpr auto str = 3.141_ss;
		/// static_assert(str == static_string{ "3.141" })
		/// \ecpp
		///
		/// \relatesalso muu::static_string
		template <char... Str>
		MUU_CONSTEVAL
		static_string<char, sizeof...(Str)> operator"" _ss()
		{
			const char str[] = { Str... };
			return static_string<char, sizeof...(Str)>{ str };
		}

#if MUU_STATIC_STRING_CHAR_UDL

		template <impl::static_string_udl_impl Str>
		MUU_CONSTEVAL
		decltype(Str) operator"" _ss()
		{
			return Str;
		}

#endif
	}
}

#include "impl/header_end.h"
