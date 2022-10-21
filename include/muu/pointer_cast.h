// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the definition of #muu::pointer_cast().

#include "meta.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS; // these should be considered "intrinsics"

namespace muu
{
	/// \brief	Casts between pointers, choosing the most appropriate conversion path.
	/// \ingroup core
	///
	/// \details Doing low-level work with pointers often requires a lot of tedious boilerplate,
	/// 		particularly when moving to/from raw byte representations or dealing with `const`.
	/// 		By using `pointer_cast` instead you can eliminate a lot of that boilerplate,
	/// 		since it will do 'the right thing' via some combination of:
	///	| From                    | To                      | Cast              | Note                  |
	///	|-------------------------|-------------------------|-------------------|-----------------------|
	///	| T\*                     | void\*                  | static_cast       |                       |
	///	| void\*                  | T\*                     | static_cast       |                       |
	///	| T\*                     | const T\*               | static_cast       |                       |
	///	| T\*                     | volatile T\*            | static_cast       |                       |
	///	| T\*                     | const volatile T\*      | static_cast       |                       |
	///	| const T\*               | T\*                     | const_cast        |                       |
	///	| volatile T\*            | T\*                     | const_cast        |                       |
	///	| const volatile T\*      | T\*                     | const_cast        |                       |
	///	| Derived\*               | Base\*                  | static_cast       |                       |
	///	| Base\*                  | Derived\*               | dynamic_cast      | Polymorphic bases     |
	///	| Base\*                  | Derived\*               | reinterpret_cast  | Non-polymorphic bases |
	///	| T\*                     | (u)intptr_t             | reinterpret_cast  |                       |
	///	| (u)intptr_t             | T\*                     | reinterpret_cast  |                       |
	///	| void\*                  | T(\*)()                 | reinterpret_cast  | Where supported       |
	///	| T(\*)()                 | void\*                  | reinterpret_cast  | Where supported       |
	///	| T(\*)()                 | T(\*)()noexcept         | reinterpret_cast  |                       |
	///	| T(\*)()noexcept         | T(\*)()                 | static_cast       |                       |
	///	| IUnknown\*              | IUnknown\*              | QueryInterface    | Windows only          |
	///
	///  \warning There are lots of static checks to stop you from doing something overtly dangerous,
	/// 		 but ultimately the fallback behaviour for casting between unrelated types is to use a
	/// 		 `reinterpret_cast`, and there's nothing stopping you from using multiple `pointer_casts`
	/// 		 through `void*` to make a conversion 'work'. Footguns aplenty!
	///
	/// \tparam To	A pointer or integral type large enough to store a pointer
	/// \tparam From A pointer, array, nullptr_t, or an integral type large enough to store a pointer.
	/// \param from The value being cast.
	///
	/// \return The input casted to the desired type.
	///
	template <typename To, typename From>
	MUU_PURE_INLINE_GETTER
	constexpr To pointer_cast(From from) noexcept
	{
		static_assert(!std::is_reference_v<To> && !std::is_reference_v<From>, // will never be deduced as a reference
																			  // but it might be specified explicitly
					  "From and To types cannot be references");
		static_assert(!(is_integral<From> && is_integral<To>),
					  "From and To types cannot both be integral types (did you mean static_cast?)");
		static_assert(std::is_pointer_v<To> || (is_integral<To> && sizeof(To) >= sizeof(void*)),
					  "To type must be a pointer or an integral type large enough to store a pointer");
		static_assert(!(!std::is_same_v<From, remove_cv<nullptr_t>> && std::is_same_v<remove_cv<nullptr_t>, To>),
					  "To type cannot be nullptr_t (such a conversion is nonsensical)");
		static_assert(
			std::is_pointer_v<From>										//
				|| (is_integral<From> && sizeof(From) >= sizeof(void*)) //
				|| std::is_same_v<From, remove_cv<nullptr_t>>,
			"From type must be a pointer, array, nullptr_t, or an integral type large enough to store a pointer");
		static_assert(!std::is_member_pointer_v<To> && !std::is_member_pointer_v<From>,
					  "From and To types cannot be pointers to members");

		using from_base = std::remove_pointer_t<From>;
		using to_base	= std::remove_pointer_t<To>;

		static_assert((!std::is_function_v<from_base> || sizeof(From) == sizeof(void*))
						  && (!std::is_function_v<to_base> || sizeof(To) == sizeof(void*)),
					  "Cannot pointer_cast with function pointers on the target platform");

		// same input and output types (no-op)
		if constexpr (std::is_same_v<From, To>)
			return from;

		// nullptr_t -> *
		else if constexpr (std::is_same_v<From, nullptr_t>)
		{
			MUU_UNUSED(from);
			return {};
		}

		// pointer -> integral
		// integral -> pointer
		else if constexpr (is_integral<From> || is_integral<To>)
		{
			static_assert(std::is_pointer_v<To> || std::is_pointer_v<From>);

			// pointer -> integral
			if constexpr (is_integral<To>)
			{
				// widening conversion and enums
				if constexpr (is_enum<To> || sizeof(To) > sizeof(From))
				{
					using cast_t = std::conditional_t<is_signed<To>, intptr_t, uintptr_t>;
					return static_cast<To>(reinterpret_cast<cast_t>(from));
				}
				// integers of the same size
				else
				{
					static_assert(sizeof(To) == sizeof(From));
					return reinterpret_cast<To>(from);
				}
			}

			// integral -> pointer
			else
			{
				// enum -> pointer
				if constexpr (is_enum<From>)
					return pointer_cast<To>(static_cast<std::underlying_type_t<From>>(from));

				// (uintptr_t, intptr_t) -> pointer
				else if constexpr (any_same<From, uintptr_t, intptr_t>)
					return reinterpret_cast<To>(from);

				// other integers -> pointer
				else
				{
					using cast_t = std::conditional_t<is_signed<From>, intptr_t, uintptr_t>;
					return reinterpret_cast<To>(static_cast<cast_t>(from));
				}
			}
		}

		// pointer -> pointer
		else
		{
			static_assert(std::is_pointer_v<To> && std::is_pointer_v<From>);

			// Foo -> Foo (different cv)
			if constexpr (std::is_same_v<remove_cv<from_base>, remove_cv<to_base>>)
			{
				static_assert(!std::is_same_v<from_base, to_base>);

				// remove const/volatile
				if constexpr (is_cv<from_base>)
					return static_cast<To>(const_cast<remove_cv<from_base>*>(from));

				// add const/volatile
				else
				{
					static_assert(std::is_same_v<From, remove_cv<from_base>*>);
					static_assert(any_same<To, const to_base*, const volatile to_base*, volatile to_base*>);
					return static_cast<To>(from);
				}
			}

			// function -> non-function
			// non-function -> function
			// function -> function (different exception specifier)
			else if constexpr (std::is_function_v<from_base> || std::is_function_v<to_base>)
			{
				// function -> function (different exception specifier)
				if constexpr (std::is_function_v<from_base> && std::is_function_v<to_base>)
				{
					static_assert(std::is_same_v<remove_noexcept<from_base>, remove_noexcept<to_base>>,
								  "Cannot cast between pointers to two different function types");

					// function -> function (noexcept)
					if constexpr (std::is_same_v<from_base, remove_noexcept<from_base>>)
						return reinterpret_cast<To>(from);

					// function (noexcept) -> function
					else
						return static_cast<To>(from);
				}

				// function -> non-function
				else if constexpr (std::is_function_v<from_base>)
				{
					static_assert(std::is_void_v<to_base>,
								  "Cannot cast from a function pointer to a type other than void");

					// function -> void
					return static_cast<To>(reinterpret_cast<void*>(from));
				}

				// non-function -> function
				else
				{
					static_assert(std::is_void_v<from_base>,
								  "Cannot cast to a function pointer from a type other than void");

					// void -> function
					return reinterpret_cast<To>(pointer_cast<void*>(from));
				}
			}

			// void -> non-void
			// non-void -> void
			// derived -> base
			else if constexpr (std::is_void_v<from_base>  //
							   || std::is_void_v<to_base> //
							   || inherits_from<from_base, to_base>)
				return pointer_cast<To>(static_cast<rebase_pointer<From, remove_cv<to_base>>>(from));

#if MUU_WINDOWS

			// IUnknown -> IUnknown (windows only)
			else if constexpr (std::is_class_v<from_base>				 //
							   && std::is_class_v<to_base>				 //
							   && std::is_base_of_v<IUnknown, from_base> //
							   && std::is_base_of_v<IUnknown, to_base>)
			{
				if (!from)
					return nullptr;

				// remove const/volatile from source type
				if constexpr (is_cv<from_base>)
					return pointer_cast<To>(const_cast<remove_cv<from_base>*>(from));

				// remove const/volatile from destination type
				else if constexpr (is_cv<to_base>)
					return const_cast<To>(pointer_cast<remove_cv<to_base>*>(from));

				else
				{
					static_assert(!is_cv<from_base>);
					static_assert(!is_cv<to_base>);

					to_base* to = {};
					if (from->QueryInterface(__uuidof(to_base), reinterpret_cast<void**>(&to)) == 0)
						to->Release(); // QueryInterface adds 1 to ref count on success
					return to;
				}
			}

#endif // MUU_WINDOWS

			// base -> derived
			else if constexpr (inherits_from<to_base, from_base>)
			{
				if constexpr (std::is_polymorphic_v<from_base>)
					return pointer_cast<To>(dynamic_cast<rebase_pointer<From, remove_cv<to_base>>>(from));
				else
					return pointer_cast<To>(reinterpret_cast<rebase_pointer<From, remove_cv<to_base>>>(from));
			}

			// rank 2+ pointer -> *
			else if constexpr (std::is_pointer_v<from_base>)
			{
				return pointer_cast<To>(static_cast<copy_cv<void, from_base>*>(from));
			}

			// * -> rank 2+ pointer
			else if constexpr (std::is_pointer_v<to_base>)
			{
				static_assert(!std::is_pointer_v<from_base>);
				return static_cast<To>(pointer_cast<copy_cv<void, to_base>*>(from));
			}

			// Foo -> Bar (unrelated types)
			else
			{
				static_assert(!std::is_same_v<remove_cv<from_base>, remove_cv<to_base>>);
				static_assert(!std::is_pointer_v<from_base>);
				static_assert(!std::is_pointer_v<to_base>);
				return pointer_cast<To>(reinterpret_cast<rebase_pointer<From, remove_cv<to_base>>>(from));
			}
		}
	}

	/// \cond

	template <typename To, typename From, size_t N>
	MUU_PURE_INLINE_GETTER
	constexpr To pointer_cast(From (&arr)[N]) noexcept
	{
		return pointer_cast<To, From*>(arr);
	}

	template <typename To, typename From, size_t N>
	MUU_PURE_INLINE_GETTER
	constexpr To pointer_cast(From (&&arr)[N]) noexcept
	{
		return pointer_cast<To, From*>(arr);
	}

	/// \endcond
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
