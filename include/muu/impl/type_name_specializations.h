// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \cond
// clang-format off
#ifdef MUU_TYPE_NAME_INCLUDED

namespace muu
{
	// <string>
	#if !defined(MUU_TYPENAME_SPECIALIZED_STD_STRING)                                                                  \
		&& (defined(MUU_STD_STRING_INCLUDED) || (defined(_MSC_VER) && defined(_STRING_)) || defined(_GLIBCXX_STRING))

		MUU_SPECIALIZE_TYPENAME_ALIAS(std::string);
		MUU_SPECIALIZE_TYPENAME_ALIAS(std::wstring);
		MUU_SPECIALIZE_TYPENAME_ALIAS(std::u16string);
		MUU_SPECIALIZE_TYPENAME_ALIAS(std::u32string);
		#if MUU_HAS_CHAR8_STRINGS
			MUU_SPECIALIZE_TYPENAME_ALIAS(std::u8string);
		#endif

		#if defined(__cpp_lib_memory_resource) && __cpp_lib_memory_resource >= 201603
		MUU_SPECIALIZE_TYPENAME_ALIAS(std::pmr::string);
		MUU_SPECIALIZE_TYPENAME_ALIAS(std::pmr::wstring);
		MUU_SPECIALIZE_TYPENAME_ALIAS(std::pmr::u16string);
		MUU_SPECIALIZE_TYPENAME_ALIAS(std::pmr::u32string);
		#if MUU_HAS_CHAR8_STRINGS
			MUU_SPECIALIZE_TYPENAME_ALIAS(std::pmr::u8string);
		#endif
		#endif

		#define MUU_TYPENAME_SPECIALIZED_STD_STRING
	#endif // <string>

	// <vector>
	#if !defined(MUU_TYPENAME_SPECIALIZED_STD_VECTOR)                                                                  \
		&& (defined(MUU_STD_VECTOR_INCLUDED) || (defined(_MSC_VER) && defined(_VECTOR_)) || defined(_GLIBCXX_VECTOR))

		MUU_SPECIALIZE_TYPENAME_TEMPLATE_ALIAS(std::vector);

		#if defined(__cpp_lib_memory_resource) && __cpp_lib_memory_resource >= 201603
		MUU_SPECIALIZE_TYPENAME_TEMPLATE_ALIAS(std::pmr::vector);
		#endif

		#define MUU_TYPENAME_SPECIALIZED_STD_VECTOR
	#endif // <vector>

	// <exception>
	#if !defined(MUU_TYPENAME_SPECIALIZED_STD_EXCEPTION)                                                               \
		&& (defined(MUU_STD_EXCEPTION_INCLUDED) || (defined(_MSC_VER) && defined(_EXCEPTION_))                         \
			|| defined(_GLIBCXX_STDEXCEPT) || defined(__EXCEPTION__))

		MUU_SPECIALIZE_TYPENAME_ALIAS(std::exception);
		MUU_SPECIALIZE_TYPENAME_ALIAS(std::nested_exception);
		MUU_SPECIALIZE_TYPENAME_ALIAS(std::bad_exception);
		MUU_SPECIALIZE_TYPENAME_ALIAS(std::exception_ptr);

		#define MUU_TYPENAME_SPECIALIZED_STD_EXCEPTION
	#endif // <exception>

	// <memory>
	#if !defined(MUU_TYPENAME_SPECIALIZED_STD_MEMORY)                                                                  \
		&& (defined(MUU_STD_MEMORY_INCLUDED) || (defined(_MSC_VER) && defined(_MEMORY_)) || defined(_GLIBCXX_MEMORY))

		MUU_SPECIALIZE_TYPENAME_ALIAS(std::bad_weak_ptr);

		MUU_SPECIALIZE_TYPENAME_TEMPLATE_ALIAS(std::unique_ptr);
		MUU_SPECIALIZE_TYPENAME_TEMPLATE_ALIAS(std::shared_ptr);
		MUU_SPECIALIZE_TYPENAME_TEMPLATE_ALIAS(std::weak_ptr);
		MUU_SPECIALIZE_TYPENAME_TEMPLATE_ALIAS(std::default_delete);
		MUU_SPECIALIZE_TYPENAME_TEMPLATE_ALIAS(std::enable_shared_from_this);
		MUU_SPECIALIZE_TYPENAME_TEMPLATE_ALIAS(std::allocator);
		MUU_SPECIALIZE_TYPENAME_TEMPLATE_ALIAS(std::allocator_traits);
		MUU_SPECIALIZE_TYPENAME_TEMPLATE_ALIAS(std::pointer_traits);
		MUU_SPECIALIZE_TYPENAME_TEMPLATE_ALIAS(std::owner_less);
		MUU_SPECIALIZE_TYPENAME_TEMPLATE_ALIAS(std::hash);
		MUU_SPECIALIZE_TYPENAME_TEMPLATE_ALIAS(std::atomic);

		#define MUU_TYPENAME_SPECIALIZED_STD_MEMORY
	#endif // <memory>
}

#endif // MUU_TYPE_NAME_INCLUDED
/// \endcond
// clang-format on
