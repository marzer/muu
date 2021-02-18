// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "muu/string_param.h"
#include "muu/strings.h"

MUU_DISABLE_SUGGEST_WARNINGS;
MUU_DISABLE_SPAM_WARNINGS;
#if MUU_MSVC
	#undef min
	#undef max
#endif

MUU_FORCE_NDEBUG_OPTIMIZATIONS;

using namespace muu;

namespace
{
	enum class mode : uint8_t
	{
		none,

		narrow,
		wide,
		u8,
		u16,
		u32,

		view = 128,
		narrow_view = narrow | view,
		wide_view = wide | view,
		u8_view = u8 | view,
		u16_view = u16 | view,
		u32_view = u32 | view
	};

	template <mode> struct mode_to_type						{ using type = void; };
	template <> struct mode_to_type<mode::narrow>			{ using type = std::string; };
	template <> struct mode_to_type<mode::wide>				{ using type = std::wstring; };
	template <> struct mode_to_type<mode::u16>				{ using type = std::u16string; };
	template <> struct mode_to_type<mode::u32>				{ using type = std::u32string; };
	template <> struct mode_to_type<mode::narrow_view>		{ using type = std::string_view; };
	template <> struct mode_to_type<mode::wide_view>		{ using type = std::wstring_view; };
	template <> struct mode_to_type<mode::u16_view>			{ using type = std::u16string_view; };
	template <> struct mode_to_type<mode::u32_view>			{ using type = std::u32string_view; };
	#ifdef __cpp_lib_char8_t
	template <> struct mode_to_type<mode::u8>				{ using type = std::u8string; };
	template <> struct mode_to_type<mode::u8_view>			{ using type = std::u8string_view; };
	#endif

	template <mode Mode>
	using type_of = typename mode_to_type<Mode>::type;

	template <typename> struct type_to_mode					{ static constexpr auto value = mode::none; };
	template <> struct type_to_mode<std::string>			{ static constexpr auto value = mode::narrow; };
	template <> struct type_to_mode<std::wstring>			{ static constexpr auto value = mode::wide; };
	template <> struct type_to_mode<std::u16string>			{ static constexpr auto value = mode::u16; };
	template <> struct type_to_mode<std::u32string>			{ static constexpr auto value = mode::u32; };
	template <> struct type_to_mode<std::string_view>		{ static constexpr auto value = mode::narrow_view; };
	template <> struct type_to_mode<std::wstring_view>		{ static constexpr auto value = mode::wide_view; };
	template <> struct type_to_mode<std::u16string_view>	{ static constexpr auto value = mode::u16_view; };
	template <> struct type_to_mode<std::u32string_view>	{ static constexpr auto value = mode::u32_view; };
	#ifdef __cpp_lib_char8_t
	template <> struct type_to_mode<std::u8string>			{ static constexpr auto value = mode::u8; };
	template <> struct type_to_mode<std::u8string_view>		{ static constexpr auto value = mode::u8_view; };
	#endif

	template <typename Type>
	inline constexpr mode mode_of = type_to_mode<Type>::value;

	template <typename T, typename Char>
	static void initialize(T& storage, uint8_t& mode_, const std::basic_string_view<Char>& str) noexcept
	{
		if (!str.data() || str.empty())
			return;

		using type = std::basic_string_view<Char>;
		mode_ = unwrap(mode_of<type>);
		::new (static_cast<void*>(&storage)) type{ str };
	}

	template <typename T, typename Char>
	static void initialize(T& storage, uint8_t& mode_, const std::basic_string<Char>& str) noexcept
	{
		if (str.empty())
			return;

		using type = std::basic_string_view<Char>;
		mode_ = unwrap(mode_of<type>);
		::new (static_cast<void*>(&storage)) type{ str };
	}

	template <typename T, typename Char>
	static void initialize(T& storage, uint8_t& mode_, const Char* str, size_t len) noexcept
	{
		if (!str || !len)
			return;

		using type = std::basic_string_view<Char>;
		mode_ = unwrap(mode_of<type>);
		::new (static_cast<void*>(&storage)) type{ str, len };
	}

	template <typename T, typename Char>
	static void initialize(T& storage, uint8_t& mode_, const Char* str) noexcept
	{
		if (!str)
			return;

		const auto len = std::char_traits<Char>::length(str);
		if (!len)
			return;

		using type = std::basic_string_view<Char>;
		mode_ = unwrap(mode_of<type>);
		::new (static_cast<void*>(&storage)) type{ str, len };
	}

	template <typename T, typename Char>
	static std::basic_string<Char>* initialize(T& storage, uint8_t& mode_, std::basic_string<Char>&& str) noexcept
	{
		using type = std::basic_string<Char>;
		if (str.empty())
			return nullptr;

		mode_ = unwrap(mode_of<type>);
		return ::new (static_cast<void*>(&storage)) type( std::move(str) );
	}

	template <mode Mode, typename T>
	[[nodiscard]]
	static auto& value(T& storage) noexcept
	{
		return *muu::launder(reinterpret_cast<type_of<Mode>*>(&storage));
	}

	template <typename T, typename Func>
	static void visit(T& storage, uint8_t mode_, Func&& fn) noexcept
	{
		switch (mode_)
		{
			case unwrap(mode::none):		break;
			case unwrap(mode::narrow):		fn(value<mode::narrow>(storage)); break;
			case unwrap(mode::wide):		fn(value<mode::wide>(storage)); break;
			case unwrap(mode::u16):			fn(value<mode::u16>(storage)); break;
			case unwrap(mode::u32):			fn(value<mode::u32>(storage)); break;
			case unwrap(mode::narrow_view):	fn(value<mode::narrow_view>(storage)); break;
			case unwrap(mode::wide_view):	fn(value<mode::wide_view>(storage)); break;
			case unwrap(mode::u16_view):	fn(value<mode::u16_view>(storage)); break;
			case unwrap(mode::u32_view):	fn(value<mode::u32_view>(storage)); break;
			#ifdef __cpp_lib_char8_t
			case unwrap(mode::u8):			fn(value<mode::u8>(storage)); break;
			case unwrap(mode::u8_view):		fn(value<mode::u8_view>(storage)); break;
			#endif
			MUU_NO_DEFAULT_CASE;
		}
	}

	template <typename T, typename Func>
	static void visit(T& storage1, T& storage2, uint8_t mode_, Func&& fn) noexcept
	{
		switch (mode_)
		{
			case unwrap(mode::none):		break;
			case unwrap(mode::narrow):		fn(value<mode::narrow>(storage1), value<mode::narrow>(storage2)); break;
			case unwrap(mode::wide):		fn(value<mode::wide>(storage1), value<mode::wide>(storage2)); break;
			case unwrap(mode::u16):			fn(value<mode::u16>(storage1), value<mode::u16>(storage2)); break;
			case unwrap(mode::u32):			fn(value<mode::u32>(storage1), value<mode::u32>(storage2)); break;
			case unwrap(mode::narrow_view):	fn(value<mode::narrow_view>(storage1), value<mode::narrow_view>(storage2)); break;
			case unwrap(mode::wide_view):	fn(value<mode::wide_view>(storage1), value<mode::wide_view>(storage2)); break;
			case unwrap(mode::u16_view):	fn(value<mode::u16_view>(storage1), value<mode::u16_view>(storage2)); break;
			case unwrap(mode::u32_view):	fn(value<mode::u32_view>(storage1), value<mode::u32_view>(storage2)); break;
			#ifdef __cpp_lib_char8_t
			case unwrap(mode::u8):			fn(value<mode::u8>(storage1), value<mode::u8>(storage2)); break;
			case unwrap(mode::u8_view):		fn(value<mode::u8_view>(storage1), value<mode::u8_view>(storage2)); break;
			#endif
			MUU_NO_DEFAULT_CASE;
		}
	}

	template <typename T>
	MUU_ALWAYS_INLINE
	static void call_destructor([[maybe_unused]] T& str) noexcept
	{
		static_assert(mode_of<T> != mode::none);

		if constexpr (!std::is_trivially_destructible_v<T>)
			str.T::~T();
	}

	template <typename T>
	static void destroy(T& storage, uint8_t& mode_) noexcept
	{
		visit(storage, mode_, [](auto& str) noexcept { call_destructor(str); });
		mode_ = {};
	}
}

MUU_ATTR(const)
bool string_param::built_with_char8_support() noexcept
{
	#ifdef __cpp_lib_char8_t
	return true;
	#else
	return false;
	#endif
}

string_param::string_param() noexcept = default;

string_param::string_param(std::string_view str) noexcept
{
	initialize(storage_, mode_, str);
}

string_param::string_param(const std::string& str) noexcept
{
	initialize(storage_, mode_, str);
}

string_param::string_param(const char* str, size_t len) noexcept
{
	initialize(storage_, mode_, str, len);
}

string_param::string_param(const char* str) noexcept
{
	initialize(storage_, mode_, str);
}

string_param::string_param(std::string&& str) noexcept
{
	initialize(storage_, mode_, std::move(str));
}

string_param::string_param(std::wstring_view str) noexcept
{
	initialize(storage_, mode_, str);
}

string_param::string_param(const std::wstring& str) noexcept
{
	initialize(storage_, mode_, str);
}

string_param::string_param(const wchar_t* str, size_t len) noexcept
{
	initialize(storage_, mode_, str, len);
}

string_param::string_param(const wchar_t* str) noexcept
{
	initialize(storage_, mode_, str);
}

string_param::string_param(std::wstring&& str) noexcept
{
	initialize(storage_, mode_, std::move(str));
}

string_param::string_param(std::u16string_view str) noexcept
{
	initialize(storage_, mode_, str);
}

string_param::string_param(const std::u16string& str) noexcept
{
	initialize(storage_, mode_, str);
}

string_param::string_param(const char16_t* str, size_t len) noexcept
{
	initialize(storage_, mode_, str, len);
}

string_param::string_param(const char16_t* str) noexcept
{
	initialize(storage_, mode_, str);
}

string_param::string_param(std::u16string&& str) noexcept
{
	initialize(storage_, mode_, std::move(str));
}

string_param::string_param(std::u32string_view str) noexcept
{
	initialize(storage_, mode_, str);
}

string_param::string_param(const std::u32string& str) noexcept
{
	initialize(storage_, mode_, str);
}

string_param::string_param(const char32_t* str, size_t len) noexcept
{
	initialize(storage_, mode_, str, len);
}

string_param::string_param(const char32_t* str) noexcept
{
	initialize(storage_, mode_, str);
}

string_param::string_param(std::u32string&& str) noexcept
{
	initialize(storage_, mode_, std::move(str));
}

string_param::string_param(const void* str, size_t len, char8_tag) noexcept
{
	#ifdef __cpp_lib_char8_t
	initialize(storage_, mode_, static_cast<const char8_t*>(str), len);
	#else
	initialize(storage_, mode_, static_cast<const char*>(str), len);
	#endif
}

string_param::string_param(void* str_obj, const void* str, size_t len, char8_tag) noexcept
{
	#ifdef __cpp_lib_char8_t

	MUU_UNUSED(str);
	MUU_UNUSED(len);
	initialize(storage_, mode_, std::move(*static_cast<std::u8string*>(str_obj)));

	#else

	MUU_UNUSED(str_obj);
	if (str && len)
		initialize(storage_, mode_, std::string(static_cast<const char*>(str), len));

	#endif
}

string_param& string_param::operator= (string_param&& rhs_) noexcept
{
	if (&rhs_ == this)
		return *this;

	// if they're the same we can just move (don't need to destroy our value)
	if (mode_ == rhs_.mode_)
	{
		visit(storage_, rhs_.storage_, mode_, [](auto& lhs, auto& rhs) noexcept
		{
			lhs = std::move(rhs);
			call_destructor(rhs);
		});
	}
	else // otherwise we're changing mode
	{
		if (mode_)
			destroy(storage_, mode_);

		visit(rhs_.storage_, rhs_.mode_, [this](auto& rhs) noexcept
		{
			initialize(storage_, mode_, std::move(rhs));
		});
	}
	rhs_.mode_ = {};
	return *this;
}

string_param::string_param(string_param&& other) noexcept
{
	*this = std::move(other);
}

string_param::~string_param() noexcept
{
	if (mode_)
		destroy(storage_, mode_);
}

bool string_param::empty() const noexcept
{
	if (!mode_)
		return true;

	bool empty_;
	visit(storage_, mode_, [&](auto& str) noexcept { empty_ = str.empty(); });
	return empty_;
}

string_param::operator bool() const noexcept
{
	return !empty();
}

bool string_param::owning() const noexcept
{
	return mode_ && mode_ < unwrap(mode::view);
}

namespace
{
	template <typename Char, typename T>
	static std::basic_string_view<Char> get_view(T& storage_, uint8_t& mode_) noexcept
	{
		MUU_ASSUME(mode_ > uint8_t{});

		std::basic_string_view<Char> out;
		visit(storage_, mode_, [&](auto& str) noexcept
		{
			using type = remove_cvref<decltype(str)>;
			if constexpr (is_same_as_any<type, std::basic_string<Char>, std::basic_string_view<Char>>)
			{
				out = str;
			}
			else
			{
				auto new_str = transcode<Char>(str);
				call_destructor(str);
				out = *initialize(storage_, mode_, std::move(new_str));
			}
		});
		return out;
	}
}

string_param::operator std::string_view() const & noexcept
{
	if (empty())
		return {};

	return get_view<char>(storage_, mode_);
}

string_param::operator std::wstring_view() const & noexcept
{
	if (empty())
		return {};

	return get_view<wchar_t>(storage_, mode_);
}

string_param::operator std::u16string_view() const & noexcept
{
	if (empty())
		return {};

	return get_view<char16_t>(storage_, mode_);
}

string_param::operator std::u32string_view() const & noexcept
{
	if (empty())
		return {};

	return get_view<char32_t>(storage_, mode_);
}

void string_param::get_char8_view(void* str) const noexcept
{
	#ifdef __cpp_lib_char8_t
	*static_cast<std::u8string_view*>(str) = get_view<char8_t>(storage_, mode_);
	#else
	MUU_UNUSED(str);
	#endif
}

namespace
{
	template <typename Char, typename T>
	static std::basic_string<Char> move_into_string(T& storage_, uint8_t& mode_) noexcept
	{
		MUU_ASSUME(mode_ > uint8_t{});

		std::basic_string<Char> out;
		visit(storage_, mode_, [&](auto& str) noexcept
		{
			using type = remove_cvref<decltype(str)>;
			if constexpr (std::is_same_v<type, std::basic_string<Char>>)
			{
				out = std::move(str);
				call_destructor(str);
				mode_ = {};
			}
			else if constexpr (std::is_same_v<type, std::basic_string_view<Char>>)
			{
				out = str;
			}
			else
			{
				out =  transcode<Char>(str);
			}
		});
		return out;
	}
}

string_param::operator std::string_view() const&& noexcept
{
	return std::string_view{ *this }; // explicitly invoke lvalue overload
}

string_param::operator std::wstring_view() const&& noexcept
{
	return std::wstring_view{ *this }; // explicitly invoke lvalue overload
}

string_param::operator std::u16string_view() const&& noexcept
{
	return std::u16string_view{ *this }; // explicitly invoke lvalue overload
}

string_param::operator std::u32string_view() const&& noexcept
{
	return std::u32string_view{ *this }; // explicitly invoke lvalue overload
}

string_param::operator std::string() && noexcept
{
	if (empty())
		return {};

	return move_into_string<char>(storage_, mode_);
}

string_param::operator std::wstring() && noexcept
{
	if (empty())
		return {};

	return move_into_string<wchar_t>(storage_, mode_);
}

string_param::operator std::u16string() && noexcept
{
	if (empty())
		return {};

	return move_into_string<char16_t>(storage_, mode_);
}

string_param::operator std::u32string() && noexcept
{
	if (empty())
		return {};

	return move_into_string<char32_t>(storage_, mode_);
}

void string_param::move_into_char8_string(void* str) noexcept
{
	#ifdef __cpp_lib_char8_t
	*static_cast<std::u8string*>(str) = move_into_string<char8_t>(storage_, mode_);
	#else
	MUU_UNUSED(str);
	#endif
}

string_param& string_param::trim() & noexcept
{
	visit(storage_, mode_, [&](auto& str) noexcept
	{
		using type = remove_cvref<decltype(str)>;

		const auto trimmed = muu::trim(str); // returns a string view
		if (trimmed.empty())
		{
			call_destructor(str);
			mode_ = {};
		}
		else
		{
			if constexpr (mode_of<type> < mode::view) // strings
			{
				// snip beginning
				if (trimmed.data() != str.data())
				{
					MUU_ASSERT((trimmed.data() - str.data()) > 0);
					str.erase(str.begin(), str.begin() + (trimmed.data() - str.data()));
				}

				// snip end
				if (trimmed.length() != str.length())
					str.resize(trimmed.length());
			}
			else // views
			{
				str = trimmed;
			}
		}
	});
	return *this;
}

string_param&& string_param::trim() && noexcept
{
	return std::move(trim());
}
