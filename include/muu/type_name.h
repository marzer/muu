// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::type_name.

#include "static_string.h"

namespace muu
{
	/// \cond
	namespace impl
	{
		template <typename T>
		struct type_name_;
	}
	/// \endcond

	/// \brief		A #muu::static_string containing the name of a type.
	/// \ingroup	meta
	///
	/// \detail \cpp
	/// std::cout << muu::type_name<int> << "\n";
	/// \ecpp
	/// \out
	/// int
	/// \eout
	///
	/// Specialize the variable template if you wish to override the default name for a type: \cpp
	/// struct foo {};
	///
	/// namespace muu
	/// {
	/// 	template <>
	/// 	inline constexpr auto type_name<foo> = static_string{ "bar" };
	/// }
	///
	/// std::cout << muu::type_name<foo> << "\n";
	/// std::cout << muu::type_name<foo*> << "\n";
	/// \ecpp
	/// \out
	/// bar
	/// bar*
	/// \eout
	///
	/// \availability	Older compilers have limitations preventing the generation of (unspecialized) type names.
	///					Check for support by examining #build::supports_type_name.
	template <typename T>
	inline constexpr auto type_name = POXY_IMPLEMENTATION_DETAIL(impl::type_name_<T>::value);

	namespace build
	{
		/// \brief			True if the use of (unspecialized) #muu::type_name is supported on this compiler.
		inline constexpr bool supports_type_name = (!MUU_GCC || MUU_GCC > 7);
	}

	/// \cond
	namespace impl
	{
		MUU_CONSTEVAL
		size_t type_name_find_first_of(std::string_view src, char needle) noexcept
		{
			auto left = std::string_view::npos;
			for (size_t i = 0; i < src.length() && left == std::string_view::npos; i++)
				if (src[i] == needle)
					left = i;
			return left;
		}

		MUU_CONSTEVAL
		size_t type_name_find_first_of(std::string_view src, std::string_view needle) noexcept
		{
			if (src.length() >= needle.length())
			{
				for (size_t start = 0_sz, end = src.length() - needle.length(); start < end; start++)
				{
					size_t i = 0;
					for (; i < needle.length(); i++)
						if (src[start + i] != needle[i])
							break;
					if (i == needle.length())
						return start;
				}
			}
			return std::string_view::npos;
		}

		MUU_CONSTEVAL
		bool type_name_starts_with(std::string_view src, std::string_view prefix) noexcept
		{
			if (src.length() < prefix.length())
				return false;

			size_t i = 0;
			for (; i < prefix.length(); i++)
				if (src[i] != prefix[i])
					break;

			return i == prefix.length();
		}

		MUU_CONSTEVAL
		size_t type_name_find_last_of(std::string_view src, char needle) noexcept
		{
			auto right = std::string_view::npos;
			for (size_t i = src.length(); i-- > 0_sz && right == std::string_view::npos;)
				if (src[i] == needle)
					right = i;
			return right;
		}

		MUU_CONSTEVAL
		std::string_view type_name_left_trim(std::string_view src, char needle) noexcept
		{
			auto left = type_name_find_first_of(src, needle);
			if (left != std::string_view::npos)
				return src.substr(left + 1_sz);
			return src;
		}

		MUU_CONSTEVAL
		std::string_view type_name_right_trim(std::string_view src, char needle) noexcept
		{
			auto right = type_name_find_last_of(src, needle);
			if (right != std::string_view::npos)
				return src.substr(0_sz, right);
			return src;
		}

		MUU_CONSTEVAL
		std::string_view type_name_remove_prefixes(std::string_view src) noexcept
		{
			bool found_this_pass = true;
			while (found_this_pass)
			{
				found_this_pass = false;
				for (const auto& prefix : { "`anonymous-namespace'::"sv,
											"(anonymous namespace)::"sv,
											"{anonymous}::"sv,
											"struct "sv,
											"class "sv,
											"union "sv,
											"enum "sv,
											"T = "sv,
											"with "sv,
											"::"sv,
											" "sv })
				{
					if (type_name_starts_with(src, prefix))
					{
						found_this_pass = true;
						src				= src.substr(prefix.length());
					}
				}
			}

			return src;
		};

		template <typename T>
		MUU_CONSTEVAL
		auto type_name_source_string() noexcept
		{
#if MUU_MSVC
			auto str = std::string_view{ __FUNCSIG__ };
#elif MUU_CLANG || MUU_GCC
			auto str = std::string_view{ __PRETTY_FUNCTION__ };
#endif

#if MUU_MSVC
			str = type_name_left_trim(str, '<');
#elif MUU_GCC || MUU_CLANG
			str		 = type_name_left_trim(str, '[');
#endif

#if MUU_MSVC
			str = type_name_right_trim(str, '>');
#elif MUU_GCC || MUU_CLANG
			str		 = type_name_right_trim(str, ']');
#endif

			str = type_name_remove_prefixes(str);

			// todo: substitute a standard representation of anonymous namespaces

			return str;
		}

		// base

		template <typename T>
		struct type_name_
		{
			static constexpr std::string_view source = type_name_source_string<T>();
			static constexpr auto value				 = static_string<char, source.length()>{ source };
		};

		// bare qualifiers

		template <typename T>
		struct type_name_<const T>
		{
			static constexpr auto value = static_string{ "const " } + type_name<T>;
		};

		template <typename T>
		struct type_name_<volatile T>
		{
			static constexpr auto value = static_string{ "volatile " } + type_name<T>;
		};

		template <typename T>
		struct type_name_<const volatile T>
		{
			static constexpr auto value = static_string{ "const volatile " } + type_name<T>;
		};

		// pointers

		template <typename T>
		struct type_name_<T*>
		{
			static constexpr auto value = type_name<T> + '*';
		};

		template <typename T>
		struct type_name_<T* const>
		{
			static constexpr auto value = type_name<T> + static_string{ "* const" };
		};

		template <typename T>
		struct type_name_<T* volatile>
		{
			static constexpr auto value = type_name<T> + static_string{ "* volatile" };
		};

		template <typename T>
		struct type_name_<T* const volatile>
		{
			static constexpr auto value = type_name<T> + static_string{ "* const volatile" };
		};

		// references

		template <typename T>
		struct type_name_<T&>
		{
			static constexpr auto value = type_name<T> + '&';
		};

		template <typename T>
		struct type_name_<T&&>
		{
			static constexpr auto value = type_name<T> + static_string{ "&&" };
		};

		// unbounded arrays

		template <typename T>
		struct type_name_<T[]>
		{
			static constexpr auto value = type_name<T> + static_string{ "[]" };
		};

		template <typename T>
		struct type_name_<const T[]>
		{
			static constexpr auto value = type_name<const T> + static_string{ "[]" };
		};

		template <typename T>
		struct type_name_<volatile T[]>
		{
			static constexpr auto value = type_name<volatile T> + static_string{ "[]" };
		};

		template <typename T>
		struct type_name_<const volatile T[]>
		{
			static constexpr auto value = type_name<const volatile T> + static_string{ "[]" };
		};

		template <typename T>
		struct type_name_<T (&)[]>
		{
			static constexpr auto value = type_name<T> + static_string{ "(&)[]" };
		};

		template <typename T>
		struct type_name_<T(&&)[]>
		{
			static constexpr auto value = type_name<T> + static_string{ "(&&)[]" };
		};

		// bounded arrays

		template <size_t N>
		inline constexpr size_t type_name_array_size_length = [](auto n) noexcept
		{
			if constexpr (!decltype(n)::value)
				return 1_sz;
			else
			{
				size_t len{};
				size_t val = decltype(n)::value;
				while (val)
				{
					val /= 10_sz;
					len++;
				}
				return len;
			}
		}(index_tag<N>{});

		template <size_t N>
		inline constexpr auto type_name_array_size = [](auto n) noexcept
		{
			static_string<char, type_name_array_size_length<decltype(n)::value>> str{};

			size_t val = decltype(n)::value;
			size_t i   = type_name_array_size_length<decltype(n)::value> - 1_sz;
			while (val)
			{
				str[i--] = static_cast<char>('0' + (val % 10_sz));
				val /= 10_sz;
			}
			return str;
		}(index_tag<N>{});

		template <typename T, size_t N>
		struct type_name_<T[N]>
		{
			static constexpr auto value = type_name<T> + '[' + type_name_array_size<N> + ']';
		};

		template <typename T, size_t N>
		struct type_name_<const T[N]>
		{
			static constexpr auto value = type_name<const T> + '[' + type_name_array_size<N> + ']';
		};

		template <typename T, size_t N>
		struct type_name_<volatile T[N]>
		{
			static constexpr auto value = type_name<volatile T> + '[' + type_name_array_size<N> + ']';
		};

		template <typename T, size_t N>
		struct type_name_<const volatile T[N]>
		{
			static constexpr auto value = type_name<const volatile T> + '[' + type_name_array_size<N> + ']';
		};

		template <typename T, size_t N>
		struct type_name_<T (&)[N]>
		{
			static constexpr auto value = type_name<T> + static_string{ "(&)[" } + type_name_array_size<N> + ']';
		};

		template <typename T, size_t N>
		struct type_name_<T(&&)[N]>
		{
			static constexpr auto value = type_name<T> + static_string{ "(&&)[" } + type_name_array_size<N> + ']';
		};

		// todo: functions
		// todo: function pointers
		// todo: function references
		// todo: member function pointers
		// todo: member object pointers
		// todo: template unpacking + propagation
	}
	/// \endcond
}
