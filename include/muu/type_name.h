// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#ifndef MUU_TYPE_NAME_INCLUDED
#define MUU_TYPE_NAME_INCLUDED

/// \file
/// \brief  Contains the definition of muu::type_name.

#include "static_string.h"
#include "type_list.h"

namespace muu
{
	/// \cond
	namespace impl
	{
		template <typename T>
		struct type_name_;

		template <typename... T>
		struct type_name_list_;
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

	/// \brief		A #muu::static_string containing a comma-delimited list of type names.
	/// \ingroup	meta
	///
	/// \detail \cpp
	/// std::cout << muu::type_name_list<int, float, char> << "\n";
	/// \ecpp
	/// \out
	/// int, float, char
	/// \eout
	///
	/// \availability	Older compilers have limitations preventing the generation of (unspecialized) type names.
	///					Check for support by examining #build::supports_type_name.
	template <typename... T>
	inline constexpr auto type_name_list = POXY_IMPLEMENTATION_DETAIL(impl::type_name_list_<T...>::value);

	namespace build
	{
		/// \brief			True if the use of (unspecialized) #muu::type_name is supported on this compiler.
		inline constexpr bool supports_type_name = (!MUU_GCC || MUU_GCC > 7);
	}

	/// \cond
	namespace impl
	{
		MUU_CONSTEVAL
		size_t type_name_find(std::string_view src, char needle) noexcept
		{
			auto left = std::string_view::npos;
			for (size_t i = 0; i < src.length() && left == std::string_view::npos; i++)
				if (src[i] == needle)
					left = i;
			return left;
		}

		MUU_CONSTEVAL
		size_t type_name_find(std::string_view src, std::string_view needle) noexcept
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
		size_t type_name_rfind(std::string_view src, char needle) noexcept
		{
			auto right = std::string_view::npos;
			for (size_t i = src.length(); i-- > 0_sz && right == std::string_view::npos;)
				if (src[i] == needle)
					right = i;
			return right;
		}

		MUU_CONSTEVAL
		std::string_view type_name_trim_left_from_first(std::string_view src, char needle) noexcept
		{
			auto left = type_name_find(src, needle);
			if (left != std::string_view::npos)
				return src.substr(left + 1_sz);
			return src;
		}

		MUU_CONSTEVAL
		std::string_view type_name_trim_right_from_last(std::string_view src, char needle) noexcept
		{
			auto right = type_name_rfind(src, needle);
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
#if defined(__GNUC__)

			auto str = std::string_view{ __PRETTY_FUNCTION__ };
			str		 = type_name_trim_left_from_first(str, '[');
			str		 = type_name_trim_right_from_last(str, ']');

#elif defined(_MSC_VER)

			auto str = std::string_view{ __FUNCSIG__ };
			str		 = type_name_trim_left_from_first(str, '<');
			str		 = type_name_trim_right_from_last(str, '>');
#endif

			str = type_name_trim_right_from_last(str, ';');
			str = type_name_remove_prefixes(str);

			// todo: substitute a standard representation of anonymous namespaces

			return str;
		}

		// base

		template <typename T>
		struct type_name_
		{
			static constexpr auto value =
				static_string<char, type_name_source_string<T>().length()>{ type_name_source_string<T>() };
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

		template <typename T>
		constexpr size_t type_name_int_to_str_length(T n) noexcept
		{
			if (!n)
				return 1_sz;
			else
			{
				size_t len{};
				T val = n;
				if constexpr (is_signed<T>)
				{
					if (val < T{})
					{
						val = -val;
						len++;
					}
				}
				while (val)
				{
					val /= T{ 10 };
					len++;
				}
				return len;
			}
		}

		template <auto N>
		inline constexpr auto type_name_int_to_str = []() noexcept
		{
			using T = decltype(N);
			static_string<char, type_name_int_to_str_length(N)> str{};

			T val	 = N;
			size_t i = type_name_int_to_str_length(N) - 1_sz;
			if constexpr (is_signed<T>)
			{
				if constexpr (N < T{})
					val = -val;
			}
			while (val)
			{
				str[i--] = static_cast<char>('0' + (val % T{ 10 }));
				val /= T{ 10 };
			}
			if constexpr (is_signed<T>)
			{
				if constexpr (N < T{})
					str[i] = '-';
			}
			return str;
		}();

		template <typename T, size_t N>
		struct type_name_<T[N]>
		{
			static constexpr auto value = type_name<T> + '[' + type_name_int_to_str<N> + ']';
		};

		template <typename T, size_t N>
		struct type_name_<const T[N]>
		{
			static constexpr auto value = type_name<const T> + '[' + type_name_int_to_str<N> + ']';
		};

		template <typename T, size_t N>
		struct type_name_<volatile T[N]>
		{
			static constexpr auto value = type_name<volatile T> + '[' + type_name_int_to_str<N> + ']';
		};

		template <typename T, size_t N>
		struct type_name_<const volatile T[N]>
		{
			static constexpr auto value = type_name<const volatile T> + '[' + type_name_int_to_str<N> + ']';
		};

		template <typename T, size_t N>
		struct type_name_<T (&)[N]>
		{
			static constexpr auto value = type_name<T> + static_string{ "(&)[" } + type_name_int_to_str<N> + ']';
		};

		template <typename T, size_t N>
		struct type_name_<T(&&)[N]>
		{
			static constexpr auto value = type_name<T> + static_string{ "(&&)[" } + type_name_int_to_str<N> + ']';
		};

		// basic template unpacking

		template <typename... T>
		struct type_name_list_;

		template <>
		struct type_name_list_<>
		{
			static constexpr auto value = static_string{ "" };
		};

		template <typename T>
		struct type_name_list_<T>
		{
			static constexpr auto& value = type_name<T>;
		};

		template <typename T, typename U>
		struct type_name_list_<T, U>
		{
			static constexpr auto value = type_name<T> + static_string{ ", " } + type_name<U>;
		};

		template <typename T, typename U, typename... V>
		struct type_name_list_<T, U, V...>
		{
			static constexpr auto value = type_name_list<T, U> + static_string{ ", " } + type_name_list<V...>;
		};

		template <auto... N>
		struct type_name_nttp_list_;

		template <>
		struct type_name_nttp_list_<>
		{
			static constexpr auto value = static_string{ "" };
		};

		template <auto N>
		struct type_name_nttp_list_<N>
		{
			static constexpr auto& value = type_name_int_to_str<N>;
		};

		template <auto N, auto O>
		struct type_name_nttp_list_<N, O>
		{
			static constexpr auto value = type_name_int_to_str<N> + static_string{ ", " } + type_name_int_to_str<O>;
		};

		template <auto N, auto O, auto... P>
		struct type_name_nttp_list_<N, O, P...>
		{
			static constexpr auto value =
				type_name_nttp_list_<N, O>::value + static_string{ ", " } + type_name_nttp_list_<P...>::value;
		};

		// only types

		template <template <typename...> typename T, typename... U>
		struct type_name_<T<U...>>
		{
			static constexpr std::string_view base =
				type_name_trim_right_from_last(type_name_source_string<T<U...>>(), '<');

			static constexpr auto value = static_string<char, base.length()>{ base } + '<' + type_name_list<U...> + '>';
		};

		// only NTTPs

		template <template <auto...> typename T, auto... N>
		struct type_name_<T<N...>>
		{
			static constexpr std::string_view base =
				type_name_trim_right_from_last(type_name_source_string<T<N...>>(), '<');

			static constexpr auto value =
				static_string<char, base.length()>{ base } + '<' + type_name_nttp_list_<N...>::value + '>';
		};

		// one type then one or more NTTPs

		template <template <typename, auto...> typename T, typename U, auto... N>
		struct type_name_<T<U, N...>>
		{
			static constexpr std::string_view base =
				type_name_trim_right_from_last(type_name_source_string<T<U, N...>>(), '<');

			static constexpr auto value = static_string<char, base.length()>{ base } + '<'
										+ type_name<U> + static_string{ ", " } + type_name_nttp_list_<N...>::value
										+ '>';
		};

		// two types then one or more NTTPs

		template <template <typename, typename, auto...> typename T, typename U0, typename U1, auto... N>
		struct type_name_<T<U0, U1, N...>>
		{
			static constexpr std::string_view base =
				type_name_trim_right_from_last(type_name_source_string<T<U0, U1, N...>>(), '<');

			static constexpr auto value = static_string<char, base.length()>{ base } + '<'
										+ type_name_list<U0, U1> + static_string{ ", " }
										+ type_name_nttp_list_<N...>::value + '>';
		};

		// three types then one or more NTTPs

		template <template <typename, typename, typename, auto...> typename T,
				  typename U0,
				  typename U1,
				  typename U2,
				  auto... N>
		struct type_name_<T<U0, U1, U2, N...>>
		{
			static constexpr std::string_view base =
				type_name_trim_right_from_last(type_name_source_string<T<U0, U1, U2, N...>>(), '<');

			static constexpr auto value = static_string<char, base.length()>{ base } + '<'
										+ type_name_list<U0, U1, U2> + static_string{ ", " }
										+ type_name_nttp_list_<N...>::value + '>';
		};

		// free/static functions

		template <typename R, typename... P>
		struct type_name_<R(P...)>
		{
			static constexpr auto value = type_name<R> + '(' + type_name_list<P...> + ')';
		};

		template <typename R, typename... P>
		struct type_name_<R(P...) noexcept>
		{
			static constexpr auto value = type_name<R(P...)> + static_string{ " noexcept" };
		};

		template <typename R>
		struct type_name_<R(...)>
		{
			static constexpr auto value = type_name<R> + static_string{ "(...)" };
		};

		template <typename R>
		struct type_name_<R(...) noexcept>
		{
			static constexpr auto value = type_name<R(...)> + static_string{ " noexcept" };
		};

		template <typename R, typename... P>
		struct type_name_<R(P..., ...)>
		{
			static constexpr auto value = type_name<R> + '(' + type_name_list<P...> + static_string{ ", ...)" };
		};

		template <typename R, typename... P>
		struct type_name_<R(P..., ...) noexcept>
		{
			static constexpr auto value = type_name<R(P..., ...)> + static_string{ " noexcept" };
		};

		// function pointers

		template <typename R, typename... P>
		struct type_name_<R (*)(P...)>
		{
			static constexpr auto value = type_name<R> + static_string{ "(*)(" } + type_name_list<P...> + ')';
		};

		template <typename R, typename... P>
		struct type_name_<R (*)(P...) noexcept>
		{
			static constexpr auto value = type_name<R (*)(P...)> + static_string{ " noexcept" };
		};

		template <typename R>
		struct type_name_<R (*)(...)>
		{
			static constexpr auto value = type_name<R> + static_string{ "(*)(...)" };
		};

		template <typename R>
		struct type_name_<R (*)(...) noexcept>
		{
			static constexpr auto value = type_name<R (*)(...)> + static_string{ " noexcept" };
		};

		template <typename R, typename... P>
		struct type_name_<R (*)(P..., ...)>
		{
			static constexpr auto value =
				type_name<R> + static_string{ "(*)(" } + '(' + type_name_list<P...> + static_string{ ", ...)" };
		};

		template <typename R, typename... P>
		struct type_name_<R (*)(P..., ...) noexcept>
		{
			static constexpr auto value = type_name<R (*)(P..., ...)> + static_string{ " noexcept" };
		};

		// const function pointers

		template <typename R, typename... P>
		struct type_name_<R (*const)(P...)>
		{
			static constexpr auto value = type_name<R> + static_string{ "(*const)(" } + type_name_list<P...> + ')';
		};

		template <typename R, typename... P>
		struct type_name_<R (*const)(P...) noexcept>
		{
			static constexpr auto value = type_name<R (*const)(P...)> + static_string{ " noexcept" };
		};

		template <typename R>
		struct type_name_<R (*const)(...)>
		{
			static constexpr auto value = type_name<R> + static_string{ "(*const)(...)" };
		};

		template <typename R>
		struct type_name_<R (*const)(...) noexcept>
		{
			static constexpr auto value = type_name<R (*const)(...)> + static_string{ " noexcept" };
		};

		template <typename R, typename... P>
		struct type_name_<R (*const)(P..., ...)>
		{
			static constexpr auto value =
				type_name<R> + static_string{ "(*const)(" } + '(' + type_name_list<P...> + static_string{ ", ...)" };
		};

		template <typename R, typename... P>
		struct type_name_<R (*const)(P..., ...) noexcept>
		{
			static constexpr auto value = type_name<R (*const)(P..., ...)> + static_string{ " noexcept" };
		};

		// volatile function pointers

		template <typename R, typename... P>
		struct type_name_<R (*volatile)(P...)>
		{
			static constexpr auto value = type_name<R> + static_string{ "(*volatile)(" } + type_name_list<P...> + ')';
		};

		template <typename R, typename... P>
		struct type_name_<R (*volatile)(P...) noexcept>
		{
			static constexpr auto value = type_name<R (*volatile)(P...)> + static_string{ " noexcept" };
		};

		template <typename R>
		struct type_name_<R (*volatile)(...)>
		{
			static constexpr auto value = type_name<R> + static_string{ "(*volatile)(...)" };
		};

		template <typename R>
		struct type_name_<R (*volatile)(...) noexcept>
		{
			static constexpr auto value = type_name<R (*volatile)(...)> + static_string{ " noexcept" };
		};

		template <typename R, typename... P>
		struct type_name_<R (*volatile)(P..., ...)>
		{
			static constexpr auto value =
				type_name<R> + static_string{ "(*volatile)(" } + '(' + type_name_list<P...> + static_string{ ", ...)" };
		};

		template <typename R, typename... P>
		struct type_name_<R (*volatile)(P..., ...) noexcept>
		{
			static constexpr auto value = type_name<R (*volatile)(P..., ...)> + static_string{ " noexcept" };
		};

		// const volatile function pointers

		template <typename R, typename... P>
		struct type_name_<R (*const volatile)(P...)>
		{
			static constexpr auto value =
				type_name<R> + static_string{ "(*const volatile)(" } + type_name_list<P...> + ')';
		};

		template <typename R, typename... P>
		struct type_name_<R (*const volatile)(P...) noexcept>
		{
			static constexpr auto value = type_name<R (*const volatile)(P...)> + static_string{ " noexcept" };
		};

		template <typename R>
		struct type_name_<R (*const volatile)(...)>
		{
			static constexpr auto value = type_name<R> + static_string{ "(*const volatile)(...)" };
		};

		template <typename R>
		struct type_name_<R (*const volatile)(...) noexcept>
		{
			static constexpr auto value = type_name<R (*const volatile)(...)> + static_string{ " noexcept" };
		};

		template <typename R, typename... P>
		struct type_name_<R (*const volatile)(P..., ...)>
		{
			static constexpr auto value = type_name<R> + static_string{ "(*const volatile)(" } + '('
										+ type_name_list<P...> + static_string{ ", ...)" };
		};

		template <typename R, typename... P>
		struct type_name_<R (*const volatile)(P..., ...) noexcept>
		{
			static constexpr auto value = type_name<R (*const volatile)(P..., ...)> + static_string{ " noexcept" };
		};

		// function references

		template <typename R, typename... P>
		struct type_name_<R (&)(P...)>
		{
			static constexpr auto value = type_name<R> + static_string{ "(&)(" } + type_name_list<P...> + ')';
		};

		template <typename R, typename... P>
		struct type_name_<R (&)(P...) noexcept>
		{
			static constexpr auto value = type_name<R (&)(P...)> + static_string{ " noexcept" };
		};

		template <typename R>
		struct type_name_<R (&)(...)>
		{
			static constexpr auto value = type_name<R> + static_string{ "(&)(...)" };
		};

		template <typename R>
		struct type_name_<R (&)(...) noexcept>
		{
			static constexpr auto value = type_name<R (&)(...)> + static_string{ " noexcept" };
		};

		template <typename R, typename... P>
		struct type_name_<R (&)(P..., ...)>
		{
			static constexpr auto value =
				type_name<R> + static_string{ "(&)(" } + '(' + type_name_list<P...> + static_string{ ", ...)" };
		};

		template <typename R, typename... P>
		struct type_name_<R (&)(P..., ...) noexcept>
		{
			static constexpr auto value = type_name<R (&)(P..., ...)> + static_string{ " noexcept" };
		};

		// todo: member function pointers
		// todo: member object pointers
		// todo: template unpacking with NTTPs
	}

#define MUU_SPECIALIZE_TYPENAME_ALIAS(T)                                                                               \
	template <>                                                                                                        \
	inline constexpr auto type_name<T> = static_string(MUU_MAKE_STRING(T))

#define MUU_SPECIALIZE_TYPENAME_TEMPLATE_ALIAS(T)                                                                      \
	template <typename T0>                                                                                             \
	inline constexpr auto type_name<T<T0>> = static_string(MUU_MAKE_STRING(T)) + '<' + type_name<T0> + '>'

	// built-ins (to reduce instantiations and ensure consistent behaviour)
	MUU_SPECIALIZE_TYPENAME_ALIAS(void);
	MUU_SPECIALIZE_TYPENAME_ALIAS(bool);
	MUU_SPECIALIZE_TYPENAME_ALIAS(char);
	MUU_SPECIALIZE_TYPENAME_ALIAS(signed char);
	MUU_SPECIALIZE_TYPENAME_ALIAS(unsigned char);
	MUU_SPECIALIZE_TYPENAME_ALIAS(wchar_t);
	MUU_SPECIALIZE_TYPENAME_ALIAS(char32_t);
	MUU_SPECIALIZE_TYPENAME_ALIAS(char16_t);
	MUU_SPECIALIZE_TYPENAME_ALIAS(short);
	MUU_SPECIALIZE_TYPENAME_ALIAS(int);
	MUU_SPECIALIZE_TYPENAME_ALIAS(long);
	MUU_SPECIALIZE_TYPENAME_ALIAS(long long);
	MUU_SPECIALIZE_TYPENAME_ALIAS(unsigned short);
	MUU_SPECIALIZE_TYPENAME_ALIAS(unsigned int);
	MUU_SPECIALIZE_TYPENAME_ALIAS(unsigned long);
	MUU_SPECIALIZE_TYPENAME_ALIAS(unsigned long long);
#if MUU_HAS_INT128
	MUU_SPECIALIZE_TYPENAME_ALIAS(__int128_t);
	MUU_SPECIALIZE_TYPENAME_ALIAS(__uint128_t);
#endif
	MUU_SPECIALIZE_TYPENAME_ALIAS(float);
	MUU_SPECIALIZE_TYPENAME_ALIAS(double);
	MUU_SPECIALIZE_TYPENAME_ALIAS(long double);
#if MUU_HAS_CHAR8
	MUU_SPECIALIZE_TYPENAME_ALIAS(char8_t);
#endif
#if MUU_HAS_FP16
	MUU_SPECIALIZE_TYPENAME_ALIAS(__fp16);
#endif
#if MUU_HAS_FLOAT16
	MUU_SPECIALIZE_TYPENAME_ALIAS(_Float16);
#endif
#if MUU_HAS_FLOAT128
	MUU_SPECIALIZE_TYPENAME_ALIAS(__float128);
#endif

	// muu types forward-declared in fwd.h
	MUU_SPECIALIZE_TYPENAME_TEMPLATE_ALIAS(muu::span);
	MUU_SPECIALIZE_TYPENAME_TEMPLATE_ALIAS(muu::accumulator);

	// <string_view> (included in this header)
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::string_view);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::wstring_view);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::u16string_view);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::u32string_view);
#if MUU_HAS_CHAR8_STRINGS
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::u8string_view);
#endif

	// <iosfwd> (included in this header)
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::ios);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::wios);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::streambuf);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::istream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::ostream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::iostream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::stringbuf);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::istringstream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::ostringstream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::stringstream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::filebuf);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::ifstream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::ofstream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::fstream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::wstreambuf);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::wistream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::wostream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::wiostream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::wstringbuf);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::wistringstream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::wostringstream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::wstringstream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::wfilebuf);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::wifstream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::wofstream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::wfstream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::streampos);
#if MUU_HAS_SYNCSTREAMS
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::syncbuf);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::osyncstream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::wsyncbuf);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::wosyncstream);
#endif
#if MUU_HAS_SPANSTREAMS
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::spanbuf);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::ispanstream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::ospanstream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::spanstream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::wspanbuf);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::wispanstream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::wospanstream);
	MUU_SPECIALIZE_TYPENAME_ALIAS(std::wspanstream);
#endif
	/// \endcond
}

#include "impl/type_name_specializations.h"

#endif // MUU_TYPE_NAME_INCLUDED
