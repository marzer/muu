// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::string_param.

#pragma once
#include "../muu/core.h"

MUU_DISABLE_WARNINGS
#include <string>
#include <string_view>
#include <iosfwd>
MUU_ENABLE_WARNINGS

MUU_PUSH_WARNINGS
MUU_DISABLE_SPAM_WARNINGS

MUU_NAMESPACE_START
{
	namespace impl
	{
		template <typename>
		struct is_string_or_string_view_ { static constexpr bool value = false; };
		template <typename Char>
		struct is_string_or_string_view_<std::basic_string_view<Char>> { static constexpr bool value = true; };
		template <typename Char>
		struct is_string_or_string_view_<std::basic_string<Char>> { static constexpr bool value = true; };
		template <typename T>
		inline constexpr bool is_string_or_string_view = is_string_or_string_view_<remove_cvref<T>>::value;

		template <typename T>
		inline constexpr bool is_string_view_ish = !is_string_or_string_view<T>
			&& !std::is_pointer_v<T>
			&& is_convertible_to_any<T,
				std::string_view,
				std::wstring_view,
				std::u16string_view,
				std::u32string_view
				#ifdef __cpp_lib_char8_t
				, std::u8string_view
				#endif
			>;
	}

	/// \brief		A move-only string type-eraser capable of representing any UTF string.
	/// \ingroup strings
	class string_param
	{
		private:

			mutable variant_storage<
				std::string, std::string_view,
				std::wstring, std::wstring_view,
				std::u16string, std::u16string_view,
				std::u32string, std::u32string_view
			> storage;
			mutable uint8_t mode_ = {};

			struct char8_tag {};
			MUU_ATTR(const) MUU_API static bool built_with_char8_support() noexcept;
			MUU_API string_param(const void*, size_t, char8_tag) noexcept; // non-owning (const char8_t*, len)
			MUU_API string_param(void*, const void*, size_t, char8_tag) noexcept; // owning (std::u8string*, const char8_t*, len)
			MUU_API void get_char8_view(void*) const noexcept; // (std::u8string_view*)
			MUU_API void move_into_char8_string(void*) noexcept; // (std::u8string*)

		public:

			/// \brief	Constructs an empty string_param.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param() noexcept;



			/// \brief	Constructs a non-owning string_param from a UTF-8 string view.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(std::string_view str) noexcept;

			/// \brief	Constructs a non-owning string_param from a UTF-8 string.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(const std::string& str) noexcept;

			/// \brief	Constructs a non-owning string_param from a pointer to a UTF-8 string and a length.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(const char* str, size_t len) noexcept;

			/// \brief	Constructs a non-owning string_param from a pointer to a UTF-8 string.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(const char* str) noexcept;

			/// \brief	Constructs an owning string_param from a UTF-8 string.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(std::string&& str) noexcept;



			/// \brief	Constructs a non-owning string_param from a UTF wide string view.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(std::wstring_view str) noexcept;

			/// \brief	Constructs a non-owning string_param from a UTF wide string.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(const std::wstring& str) noexcept;

			/// \brief	Constructs a non-owning string_param from a pointer to a UTF wide string and a length.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(const wchar_t* str, size_t len) noexcept;

			/// \brief	Constructs a non-owning string_param from a pointer to a UTF wide string.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(const wchar_t* str) noexcept;

			/// \brief	Constructs an owning string_param from a UTF wide string.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(std::wstring&& str) noexcept;



			/// \brief	Constructs a non-owning string_param from a UTF-16 string view.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(std::u16string_view str) noexcept;

			/// \brief	Constructs a non-owning string_param from a UTF-16 string.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(const std::u16string& str) noexcept;

			/// \brief	Constructs a non-owning string_param from a pointer to a UTF-16 string and a length.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(const char16_t* str, size_t len) noexcept;

			/// \brief	Constructs a non-owning string_param from a pointer to a UTF-16 string.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(const char16_t* str) noexcept;

			/// \brief	Constructs an owning string_param from a UTF-16 string.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(std::u16string&& str) noexcept;



			/// \brief	Constructs a non-owning string_param from a UTF-32 string view.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(std::u32string_view str) noexcept;

			/// \brief	Constructs a non-owning string_param from a UTF-32 string.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(const std::u32string& str) noexcept;

			/// \brief	Constructs a non-owning string_param from a pointer to a UTF-32 string and a length.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(const char32_t* str, size_t len) noexcept;

			/// \brief	Constructs a non-owning string_param from a pointer to a UTF-32 string.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(const char32_t* str) noexcept;

			/// \brief	Constructs an owning string_param from a UTF-32 string.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(std::u32string&& str) noexcept;



			#ifdef __cpp_lib_char8_t

			/// \brief	Constructs a non-owning string_param from a UTF-8 string view.
			MUU_NODISCARD_CTOR
			string_param(std::u8string_view str) noexcept
				: string_param{ str.data(), str.length(), char8_tag{} }
			{}

			/// \brief	Constructs a non-owning string_param from a UTF-8 string.
			MUU_NODISCARD_CTOR
			string_param(const std::u8string& str) noexcept
				: string_param{ str.c_str(), str.length(), char8_tag{} }
			{}

			/// \brief	Constructs a non-owning string_param from a pointer to a UTF-8 string and a length.
			MUU_NODISCARD_CTOR
			string_param(const char8_t* str, size_t len) noexcept
				: string_param{ str, len, char8_tag{} }
			{}

			/// \brief	Constructs a non-owning string_param from a pointer to a UTF-8 string.
			MUU_NODISCARD_CTOR
			string_param(const char8_t* str) noexcept
				: string_param{ str, str ? std::char_traits<char8_t>::length(str) : 0u, char8_tag{} }
			{}

			/// \brief	Constructs an owning string_param from a UTF-8 string.
			MUU_NODISCARD_CTOR
			string_param(std::u8string&& str) noexcept
				: string_param{ &str, str.c_str(), str.length(), char8_tag{} }
			{}

			#endif // __cpp_lib_char8_t



		private:

			template <typename T>
			[[nodiscard]]
			static auto make_string_view(T&& string_viewable) noexcept
			{
				#ifdef __cpp_lib_char8_t
				if constexpr (std::is_convertible_v<T&&, std::u8string_view>)
					return std::u8string_view{ static_cast<T&&>(string_viewable) };
				else
				#endif

				if constexpr (std::is_convertible_v<T&&, std::u32string_view>)
					return std::u32string_view{ static_cast<T&&>(string_viewable) };
				else if constexpr (std::is_convertible_v<T&&, std::u16string_view>)
					return std::u16string_view{ static_cast<T&&>(string_viewable) };
				else if constexpr (std::is_convertible_v<T&&, std::wstring_view>)
					return std::wstring_view{ static_cast<T&&>(string_viewable) };
				else if constexpr (std::is_convertible_v<T&&, std::string_view>)
					return std::string_view{ static_cast<T&&>(string_viewable) };
				else
					static_assert(always_false<T>, "Evaluated unreachable branch");
			}

		public:

			/// \brief	Constructs a non-owning string_param from a string-viewable object.
			template <typename T MUU_SFINAE(impl::is_string_view_ish<T&&>)>
			MUU_NODISCARD_CTOR
			string_param(T&& string_viewable) noexcept
				: string_param{ make_string_view(static_cast<T&&>(string_viewable)) }
			{}



			/// \brief	Move-assignment operator.
			MUU_API
			string_param& operator= (string_param&& rhs) noexcept;

			/// \brief	Move constructor.
			MUU_NODISCARD_CTOR
			MUU_API
			string_param(string_param&& other) noexcept;

			MUU_DELETE_COPY(string_param);

			/// \brief	Destructor.
			MUU_API
			~string_param() noexcept;



			/// \brief	Returns true if the string_param does not contain a value, or it has a length of zero.
			[[nodiscard]]
			MUU_API
			bool empty() const noexcept;

			/// \brief	Returns true if the string_param contains a value of non-zero length.
			[[nodiscard]]
			MUU_API
			explicit operator bool() const noexcept;

			/// \brief	Returns true if the string_param is not empty and owns the payload string (rather than simply being a view).
			[[nodiscard]]
			MUU_API
			bool owning() const noexcept;

			/// \brief	Trims leading and trailing whitespace from the payload string.
			MUU_API
			string_param& trim() & noexcept;

			/// \brief	Trims leading and trailing whitespace from the payload string (rvalue overload).
			MUU_API
			string_param&& trim() && noexcept;




			/// \brief	Returns a std::string_view of the string_param's payload.
			[[nodiscard]]
			MUU_API
			operator std::string_view() const & noexcept;
			
			/// \brief	Returns a std::wstring_view of the string_param's payload.
			[[nodiscard]]
			MUU_API
			operator std::wstring_view() const & noexcept;

			/// \brief	Returns a std::u16string_view of the string_param's payload.
			[[nodiscard]]
			MUU_API
			operator std::u16string_view() const & noexcept;

			/// \brief	Returns a std::u32string_view of the string_param's payload.
			[[nodiscard]]
			MUU_API
			operator std::u32string_view() const & noexcept;

			#ifdef __cpp_lib_char8_t

			/// \brief	Returns a std::u8string_view of the string_param's payload.
			[[nodiscard]]
			operator std::u8string_view() const & noexcept
			{
				if (empty())
					return {};

				if (built_with_char8_support())
				{
					std::u8string_view out;
					get_char8_view(&out);
					return out;
				}
				else
				{
					std::string_view str{ *this };
					return std::u8string_view{ reinterpret_cast<const char8_t*>(str.data()), str.length() };
				}
			}

			#endif // __cpp_lib_char8_t



			/// \brief	Returns a std::string_view of the string_param's payload (rvalue overload).
			[[nodiscard]]
			MUU_API
			explicit operator std::string_view() const&& noexcept;
			
			/// \brief	Returns a std::wstring_view of the string_param's payload (rvalue overload).
			[[nodiscard]]
			MUU_API
			explicit operator std::wstring_view() const&& noexcept;

			/// \brief	Returns a std::u16string_view of the string_param's payload (rvalue overload).
			[[nodiscard]]
			MUU_API
			explicit operator std::u16string_view() const&& noexcept;

			/// \brief	Returns a std::u32string_view of the string_param's payload (rvalue overload).
			[[nodiscard]]
			MUU_API
			explicit operator std::u32string_view() const&& noexcept;

			#ifdef __cpp_lib_char8_t

			/// \brief	Returns a std::u8string_view of the string_param's payload (rvalue overload).
			[[nodiscard]]
			explicit operator std::u8string_view() const && noexcept
			{
				return std::u8string_view{ *this }; // explicitly invoke lvalue overload
			}

			#endif // __cpp_lib_char8_t



			/// \brief	Moves the string_param's payload into a std::string.
			[[nodiscard]]
			MUU_API
			operator std::string() && noexcept;

			/// \brief	Moves the string_param's payload into a std::wstring.
			[[nodiscard]]
			MUU_API
			operator std::wstring() && noexcept;

			/// \brief	Moves the string_param's payload into a std::u16string.
			[[nodiscard]]
			MUU_API
			operator std::u16string() && noexcept;

			/// \brief	Moves the string_param's payload into a std::u32string.
			[[nodiscard]]
			MUU_API
			operator std::u32string() && noexcept;

			#ifdef __cpp_lib_char8_t

			/// \brief	Moves the string_param's payload into a std::u8string.
			[[nodiscard]]
			operator std::u8string() && noexcept
			{
				if (empty())
					return {};

				std::u8string out;
				if (built_with_char8_support())
					move_into_char8_string(&out);
				else
				{
					std::string_view str{ *this };
					out.resize(str.length());
					memcpy(out.data(), str.data(), str.length());
				}
				return out;
			}

			#endif // __cpp_lib_char8_t



			/// \brief	Writes the string_param to a text stream.
			template <typename Char, typename Traits>
			friend std::basic_ostream<Char, Traits>& operator << (std::basic_ostream<Char, Traits>& lhs, const string_param& rhs)
			{
				if (rhs)
					lhs << std::basic_string_view<Char>{ rhs };
				return lhs;
			}
	};

}
MUU_NAMESPACE_END

MUU_POP_WARNINGS // MUU_DISABLE_SPAM_WARNINGS
