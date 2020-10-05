// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::vector.

/*
	VECTOR CLASS DESIGN - KEY POINTS

	a.k.a. pre-emptive answers to "why is _____ implemented like that?"

	-	Vectors of <= 4 dimensions have named member variables x,y,z and w, whereas those with dimensions > 4
		have a single values[] array. This is accomplished by the use of specialized base class templates.
		While complicating the main class implementation somewhat, it is a great usability boon since in general being
		able to refer to component 0 like "vec.x" is much more user-friendly. (implementation complexity
		is largely addressed by the use of some generic function body implementation macros)

	-	"Why not just make them a union?"
		Because unions in C++ are a complicated mess, and almost impossible to use if you value any sort of constexpr.

	-	Some functions use SFINAE to select overloads that take things either by reference or value, with the value
		overloads being 'hidden' from doxygen and intellisense. These 'hidden' overloads are optimizations to take
		advantage of __vectorcall on windows, and only apply to float, double and long double vectors of <= dimensions
		since the're considered "Homogeneous Vector Aggreggates" and must be passed by value to be properly vectorized.
		- __vectorcall: https://docs.microsoft.com/en-us/cpp/cpp/vectorcall?view=vs-2019
		- vectorizer sandbox: https://godbolt.org/z/7Wq45T

	-	You'll see intermediate_type used instead of scalar_type in a few places. It's a 'better' type used for
		intermediate floating-point values where precision loss or unnecessary cast round-tripping is to be avoided.
		Scalar_type is a float: intermediate_type == scalar_type, except for 16-bit floats which are promoted to float.
		Scalar_type is integral: intermediate_type == double.

	-	Some code is statically switched/branched according to whether a static_cast<> is necessary; this is to avoid
		unnecessary codegen and improve debug build performance for non-trivial scalar_types (e.g. muu::half).
*/

#pragma once
#include "../muu/core.h"
MUU_DISABLE_WARNINGS
#include <iosfwd>
MUU_ENABLE_WARNINGS

MUU_PUSH_WARNINGS
MUU_DISABLE_SHADOW_WARNINGS

MUU_PRAGMA_MSVC(inline_recursion(on))
MUU_PRAGMA_MSVC(float_control(push))
MUU_PRAGMA_MSVC(float_control(except, off))
MUU_PRAGMA_MSVC(float_control(precise, off))

//=====================================================================================================================
// VECTOR CLASS IMPLEMENTATION
#if 1

#ifndef DOXYGEN // Template Specialization cast Confusion on Doxygen! It's super effective!

#if 1 // helper macros ------------------------------------------------------------------------------------------------

#if MUU_CLANG
	#define FMA_BLOCK	MUU_PRAGMA_CLANG("clang fp contract(fast)")
#else
	#define FMA_BLOCK
#endif

#define COMPONENTWISE_AND(func)																						\
	if constexpr (Dimensions == 1) return func(x);																	\
	if constexpr (Dimensions == 2) return (func(x)) && (func(y));													\
	if constexpr (Dimensions == 3) return (func(x)) && (func(y)) && (func(z));										\
	if constexpr (Dimensions == 4) return (func(x)) && (func(y)) && (func(z)) && (func(w));							\
	if constexpr (Dimensions > 4)																					\
	{																												\
		MUU_PRAGMA_MSVC(omp simd)																					\
		for (size_t i = 0; i < Dimensions; i++)																		\
			if (!(func(values[i])))																					\
				return false;																						\
		return true;																								\
	}																												\
	(void)0

#define COMPONENTWISE_OR(func)																						\
	if constexpr (Dimensions == 1) return func(x);																	\
	if constexpr (Dimensions == 2) return (func(x)) || (func(y));													\
	if constexpr (Dimensions == 3) return (func(x)) || (func(y)) || (func(z));										\
	if constexpr (Dimensions == 4) return (func(x)) || (func(y)) || (func(z)) || (func(w));							\
	if constexpr (Dimensions > 4)																					\
	{																												\
		MUU_PRAGMA_MSVC(omp simd)																					\
		for (size_t i = 0; i < Dimensions; i++)																		\
			if (func(values[i]))																					\
				return true;																						\
		return false;																								\
	}																												\
	(void)0

#define COMPONENTWISE_ARITHMETIC_OPERATOR(func, op)																	\
	if constexpr (Dimensions == 1) { FMA_BLOCK return func(x);											}			\
	if constexpr (Dimensions == 2) { FMA_BLOCK return (func(x)) op (func(y));							}			\
	if constexpr (Dimensions == 3) { FMA_BLOCK return (func(x)) op (func(y)) op (func(z));				}			\
	if constexpr (Dimensions == 4) { FMA_BLOCK return (func(x)) op (func(y)) op (func(z)) op (func(w));	}			\
	if constexpr (Dimensions > 4)																					\
	{																												\
		FMA_BLOCK																									\
		auto val = func(values[0]);																					\
		MUU_PRAGMA_MSVC(omp simd)																					\
		for (size_t i = 1; i < Dimensions; i++)																		\
		{																											\
			FMA_BLOCK																								\
			val op##= func(values[i]);																				\
		}																											\
		return val;																									\
	}																												\
	(void)0

#define COMPONENTWISE_ACCUMULATE(func)		COMPONENTWISE_ARITHMETIC_OPERATOR(func, +)

#define NULL_TRANSFORM(x) x

#define COMPONENTWISE_CASTING_OP_BRANCH(func, transformer, x_selector)												\
	using func_type = decltype(func(x_selector));																	\
	if constexpr (!std::is_same_v<func_type, scalar_type>)															\
	{																												\
		transformer(func, static_cast<scalar_type>);																\
	}																												\
	else																											\
	{																												\
		transformer(func, NULL_TRANSFORM);																			\
	}																												\
	(void)0


#define COMPONENTWISE_CASTING_OP(func, transformer)																	\
	if constexpr (Dimensions <= 4)																					\
	{																												\
		COMPONENTWISE_CASTING_OP_BRANCH(func, transformer, x);														\
	}																												\
	else																											\
	{																												\
		COMPONENTWISE_CASTING_OP_BRANCH(func, transformer, values[0]);												\
	}																												\
	(void)0

#define COMPONENTWISE_CONSTRUCT_WITH_TRANSFORM(func, xform)															\
	if constexpr (Dimensions == 1) { FMA_BLOCK return vector{ xform(func(x)) };													} \
	if constexpr (Dimensions == 2) { FMA_BLOCK return vector{ xform(func(x)), xform(func(y)) };									} \
	if constexpr (Dimensions == 3) { FMA_BLOCK return vector{ xform(func(x)), xform(func(y)), xform(func(z)) };					} \
	if constexpr (Dimensions == 4) { FMA_BLOCK return vector{ xform(func(x)), xform(func(y)), xform(func(z)), xform(func(w)) };	} \
	if constexpr (Dimensions > 4)																					\
	{																												\
		FMA_BLOCK																									\
		return vector{																								\
			impl::elementwise_func_tag{},																			\
			[&](size_t i) noexcept																					\
			{																										\
				FMA_BLOCK																							\
				return xform(func(values[i]));																		\
			}																										\
		};																											\
	}																												\
	(void)0

#define COMPONENTWISE_CONSTRUCT(func)	COMPONENTWISE_CASTING_OP(func, COMPONENTWISE_CONSTRUCT_WITH_TRANSFORM)

#define COMPONENTWISE_ASSIGN_WITH_TRANSFORM(func, xform)															\
	if constexpr (Dimensions <= 4)																					\
	{																												\
		FMA_BLOCK																									\
										base::x = xform(func(x));													\
		if constexpr (Dimensions >= 2)	base::y = xform(func(y));													\
		if constexpr (Dimensions >= 3)	base::z = xform(func(z));													\
		if constexpr (Dimensions == 4)	base::w = xform(func(w));													\
	}																												\
	else																											\
	{																												\
		FMA_BLOCK																									\
		MUU_PRAGMA_MSVC(omp simd)																					\
		for (size_t i = 0; i < Dimensions; i++)																		\
			base::values[i] = xform(func(values[i]));																\
	}																												\
	return *this

#define COMPONENTWISE_ASSIGN(func)		COMPONENTWISE_CASTING_OP(func, COMPONENTWISE_ASSIGN_WITH_TRANSFORM)

#define	ENABLE_IF_AT_LEAST_DIMENSIONS(dim)	\
		, size_t SFINAE = Dimensions MUU_SFINAE(SFINAE >= (dim) && SFINAE == Dimensions)

#define	ENABLE_IF_AT_LEAST_DIMENSIONS_AND(dim,...)	\
	, size_t SFINAE = Dimensions MUU_SFINAE_2(SFINAE >= (dim) && SFINAE == Dimensions && (__VA_ARGS__))

#define	REQUIRES_AT_LEAST_DIMENSIONS(dim) \
	template <size_t SFINAE = Dimensions MUU_SFINAE(SFINAE >= (dim) && SFINAE == Dimensions)>

#define	REQUIRES_EXACTLY_DIMENSIONS(dim) \
	template <size_t SFINAE = Dimensions MUU_SFINAE_2(SFINAE == (dim) && SFINAE == Dimensions)>

#define	REQUIRES_FLOATING_POINT	\
	template <typename SFINAE = Scalar MUU_SFINAE(muu::is_floating_point<SFINAE> && std::is_same_v<SFINAE, Scalar>)>

#define	REQUIRES_SIGNED	\
	template <typename SFINAE = Scalar MUU_SFINAE(muu::is_signed<SFINAE> && std::is_same_v<SFINAE, Scalar>)>

#endif // helper macros

MUU_IMPL_NAMESPACE_START
{
	struct value_fill_tag {};
	struct zero_fill_tag {};
	struct array_copy_tag {};
	struct array_cast_tag {};
	struct tuple_copy_tag {};
	struct tuple_cast_tag {};
	struct elementwise_func_tag {};
	struct tuple_concat_tag{};

	template <typename Scalar, size_t Dimensions>
	struct MUU_TRIVIAL_ABI vector_base
	{
		static_assert(Dimensions > 4);

		Scalar values[Dimensions];

		vector_base() noexcept = default;

		explicit constexpr vector_base(zero_fill_tag) noexcept
			: values{}
		{}

		template <size_t... Indices>
		explicit constexpr vector_base(Scalar fill, std::index_sequence<Indices...>) noexcept
			: values{ ((void)Indices, fill)... }
		{
			static_assert(sizeof...(Indices) <= Dimensions);
		}

		explicit constexpr vector_base(value_fill_tag, Scalar fill) noexcept
			: vector_base{ fill, std::make_index_sequence<Dimensions>{} }
		{}

		explicit constexpr vector_base(Scalar x_) noexcept
			: values{ x_ }
		{}

		template <typename... T>
		explicit constexpr vector_base(Scalar x_, Scalar y_, T... vals) noexcept
			: values{ x_, y_, static_cast<Scalar>(vals)... }
		{
			static_assert(sizeof...(T) <= Dimensions - 2);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_base(array_copy_tag, std::index_sequence<Indices...>, const T& arr) noexcept
			: values{ arr[Indices]... }
		{
			static_assert(sizeof...(Indices) <= Dimensions);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_base(array_cast_tag, std::index_sequence<Indices...>, const T& arr) noexcept
			: values{ static_cast<Scalar>(arr[Indices])... }
		{
			static_assert(sizeof...(Indices) <= Dimensions);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_base(tuple_copy_tag, std::index_sequence<Indices...>, const T& tpl) noexcept
			: values{ get_from_tuple_like<Indices>(tpl)... }
		{
			static_assert(sizeof...(Indices) <= Dimensions);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_base(tuple_cast_tag, std::index_sequence<Indices...>, const T& tpl) noexcept
			: values{ static_cast<Scalar>(get_from_tuple_like<Indices>(tpl))... }
		{
			static_assert(sizeof...(Indices) <= Dimensions);
		}

		template <typename Func, size_t... Indices>
		explicit constexpr vector_base(elementwise_func_tag, std::index_sequence<Indices...>, Func&& func) noexcept
			: values{ func(Indices)... }
		{
			static_assert(sizeof...(Indices) <= Dimensions);
		}

		template <typename T1, typename T2, size_t... Indices1, size_t... Indices2>
		explicit constexpr vector_base(tuple_concat_tag,
			std::index_sequence<Indices1...>, const T1& tpl1,
			std::index_sequence<Indices2...>, const T2& tpl2
		) noexcept
			: values{
				static_cast<Scalar>(get_from_tuple_like<Indices1>(tpl1))...,
				static_cast<Scalar>(get_from_tuple_like<Indices2>(tpl2))...
			}
		{
			static_assert((sizeof...(Indices1) + sizeof...(Indices2)) <= Dimensions);
		}

		template <typename T, size_t... Indices, typename... V>
		explicit constexpr vector_base(tuple_concat_tag,
			std::index_sequence<Indices...>, const T& tpl,
			V... vals
		) noexcept
			: values{
				static_cast<Scalar>(get_from_tuple_like<Indices>(tpl))...,
				static_cast<Scalar>(vals)...
			}
		{
			static_assert((sizeof...(Indices) + sizeof...(V)) <= Dimensions);
		}
	};

	template <typename Scalar>
	struct MUU_TRIVIAL_ABI vector_base<Scalar, 1>
	{
		Scalar x;

		vector_base() noexcept = default;

		explicit constexpr vector_base(zero_fill_tag) noexcept
			: x{}
		{}

		explicit constexpr vector_base(value_fill_tag, Scalar x_) noexcept
			: x{ x_ }
		{}

		explicit constexpr vector_base(Scalar x_) noexcept
			: x{ x_ }
		{}

		template <typename T, size_t... Indices>
		explicit constexpr vector_base(array_copy_tag, std::index_sequence<Indices...>, const T& arr) noexcept
			: vector_base{ arr[Indices]... }
		{
			static_assert(sizeof...(Indices) == 1);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_base(array_cast_tag, std::index_sequence<Indices...>, const T& arr) noexcept
			: vector_base{ static_cast<Scalar>(arr[Indices])... }
		{
			static_assert(sizeof...(Indices) == 1);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_base(tuple_copy_tag, std::index_sequence<Indices...>, const T& tpl) noexcept
			: vector_base{ get_from_tuple_like<Indices>(tpl)... }
		{
			static_assert(sizeof...(Indices) == 1);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_base(tuple_cast_tag, std::index_sequence<Indices...>, const T& tpl) noexcept
			: vector_base{ static_cast<Scalar>(get_from_tuple_like<Indices>(tpl))... }
		{
			static_assert(sizeof...(Indices) == 1);
		}
	};

	template <typename Scalar>
	struct MUU_TRIVIAL_ABI vector_base<Scalar, 2>
	{
		Scalar x;
		Scalar y;

		vector_base() noexcept = default;

		explicit constexpr vector_base(zero_fill_tag) noexcept
			: x{},
			y{}
		{}

		explicit constexpr vector_base(value_fill_tag, Scalar fill) noexcept
			: x{ fill },
			y{ fill }
		{}

		explicit constexpr vector_base(Scalar x_, Scalar y_ = Scalar{}) noexcept
			: x{ x_ },
			y{ y_ }
		{}

		template <typename T, size_t... Indices>
		explicit constexpr vector_base(array_copy_tag, std::index_sequence<Indices...>, const T& arr) noexcept
			: vector_base{ arr[Indices]... }
		{
			static_assert(sizeof...(Indices) <= 2);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_base(array_cast_tag, std::index_sequence<Indices...>, const T& arr) noexcept
			: vector_base{ static_cast<Scalar>(arr[Indices])... }
		{
			static_assert(sizeof...(Indices) <= 2);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_base(tuple_copy_tag, std::index_sequence<Indices...>, const T& tpl) noexcept
			: vector_base{ get_from_tuple_like<Indices>(tpl)... }
		{
			static_assert(sizeof...(Indices) <= 2);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_base(tuple_cast_tag, std::index_sequence<Indices...>, const T& tpl) noexcept
			: vector_base{ static_cast<Scalar>(get_from_tuple_like<Indices>(tpl))... }
		{
			static_assert(sizeof...(Indices) <= 2);
		}

		template <typename T1, typename T2, size_t... Indices1, size_t... Indices2>
		explicit constexpr vector_base(tuple_concat_tag,
			std::index_sequence<Indices1...>, const T1& tpl1,
			std::index_sequence<Indices2...>, const T2& tpl2
		) noexcept
			: vector_base{
				static_cast<Scalar>(get_from_tuple_like<Indices1>(tpl1))...,
				static_cast<Scalar>(get_from_tuple_like<Indices2>(tpl2))...
			}
		{
			static_assert((sizeof...(Indices1) + sizeof...(Indices2)) <= 2);
		}

		template <typename T, size_t... Indices, typename... V>
		explicit constexpr vector_base(tuple_concat_tag,
			std::index_sequence<Indices...>, const T& tpl,
			V... vals
		) noexcept
			: vector_base{
				static_cast<Scalar>(get_from_tuple_like<Indices>(tpl))...,
				static_cast<Scalar>(vals)...
			}
		{
			static_assert((sizeof...(Indices) + sizeof...(V)) <= 2);
		}
	};

	template <typename Scalar>
	struct MUU_TRIVIAL_ABI vector_base<Scalar, 3>
	{
		Scalar x;
		Scalar y;
		Scalar z;

		vector_base() noexcept = default;

		explicit constexpr vector_base(zero_fill_tag) noexcept
			: x{},
			y{},
			z{}
		{}

		explicit constexpr vector_base(value_fill_tag, Scalar fill) noexcept
			: x{ fill },
			y{ fill },
			z{ fill }
		{}

		explicit constexpr vector_base(Scalar x_, Scalar y_ = Scalar{}, Scalar z_ = Scalar{}) noexcept
			: x{ x_ },
			y{ y_ },
			z{ z_ }
		{}

		template <typename T, size_t... Indices>
		explicit constexpr vector_base(array_copy_tag, std::index_sequence<Indices...>, const T& arr) noexcept
			: vector_base{ arr[Indices]... }
		{
			static_assert(sizeof...(Indices) <= 3);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_base(array_cast_tag, std::index_sequence<Indices...>, const T& arr) noexcept
			: vector_base{ static_cast<Scalar>(arr[Indices])... }
		{
			static_assert(sizeof...(Indices) <= 3);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_base(tuple_copy_tag, std::index_sequence<Indices...>, const T& tpl) noexcept
			: vector_base{ get_from_tuple_like<Indices>(tpl)... }
		{
			static_assert(sizeof...(Indices) <= 3);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_base(tuple_cast_tag, std::index_sequence<Indices...>, const T& tpl) noexcept
			: vector_base{ static_cast<Scalar>(get_from_tuple_like<Indices>(tpl))... }
		{
			static_assert(sizeof...(Indices) <= 3);
		}

		template <typename T1, typename T2, size_t... Indices1, size_t... Indices2>
		explicit constexpr vector_base(tuple_concat_tag,
			std::index_sequence<Indices1...>, const T1& tpl1,
			std::index_sequence<Indices2...>, const T2& tpl2
		) noexcept
			: vector_base{
				static_cast<Scalar>(get_from_tuple_like<Indices1>(tpl1))...,
				static_cast<Scalar>(get_from_tuple_like<Indices2>(tpl2))...
			}
		{
			static_assert((sizeof...(Indices1) + sizeof...(Indices2)) <= 3);
		}

		template <typename T, size_t... Indices, typename... V>
		explicit constexpr vector_base(tuple_concat_tag,
			std::index_sequence<Indices...>, const T& tpl,
			V... vals
		) noexcept
			: vector_base{
				static_cast<Scalar>(get_from_tuple_like<Indices>(tpl))...,
				static_cast<Scalar>(vals)...
			}
		{
			static_assert((sizeof...(Indices) + sizeof...(V)) <= 3);
		}
	};

	template <typename Scalar>
	struct MUU_TRIVIAL_ABI vector_base<Scalar, 4>
	{
		Scalar x;
		Scalar y;
		Scalar z;
		Scalar w;

		vector_base() noexcept = default;

		explicit constexpr vector_base(zero_fill_tag) noexcept
			: x{},
			y{},
			z{},
			w{}
		{}

		explicit constexpr vector_base(value_fill_tag, Scalar fill) noexcept
			: x{ fill },
			y{ fill },
			z{ fill },
			w{ fill }
		{}

		explicit constexpr vector_base(Scalar x_, Scalar y_ = Scalar{}, Scalar z_ = Scalar{}, Scalar w_ = Scalar{}) noexcept
			: x{ x_ },
			y{ y_ },
			z{ z_ },
			w{ w_ }
		{}

		template <typename T, size_t... Indices>
		explicit constexpr vector_base(array_copy_tag, std::index_sequence<Indices...>, const T& arr) noexcept
			: vector_base{ arr[Indices]... }
		{
			static_assert(sizeof...(Indices) <= 4);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_base(array_cast_tag, std::index_sequence<Indices...>, const T& arr) noexcept
			: vector_base{ static_cast<Scalar>(arr[Indices])... }
		{
			static_assert(sizeof...(Indices) <= 4);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_base(tuple_copy_tag, std::index_sequence<Indices...>, const T& tpl) noexcept
			: vector_base{ get_from_tuple_like<Indices>(tpl)... }
		{
			static_assert(sizeof...(Indices) <= 4);
		}

		template <typename T, size_t... Indices>
		explicit constexpr vector_base(tuple_cast_tag, std::index_sequence<Indices...>, const T& tpl) noexcept
			: vector_base{ static_cast<Scalar>(get_from_tuple_like<Indices>(tpl))... }
		{
			static_assert(sizeof...(Indices) <= 4);
		}

		template <typename T1, typename T2, size_t... Indices1, size_t... Indices2>
		explicit constexpr vector_base(tuple_concat_tag,
			std::index_sequence<Indices1...>, const T1& tpl1,
			std::index_sequence<Indices2...>, const T2& tpl2
		) noexcept
			: vector_base{
				static_cast<Scalar>(get_from_tuple_like<Indices1>(tpl1))...,
				static_cast<Scalar>(get_from_tuple_like<Indices2>(tpl2))...
			}
		{
			static_assert((sizeof...(Indices1) + sizeof...(Indices2)) <= 4);
		}

		template <typename T, size_t... Indices, typename... V>
		explicit constexpr vector_base(tuple_concat_tag,
			std::index_sequence<Indices...>, const T& tpl,
			V... vals
		) noexcept
			: vector_base{
				static_cast<Scalar>(get_from_tuple_like<Indices>(tpl))...,
				static_cast<Scalar>(vals)...
			}
		{
			static_assert((sizeof...(Indices) + sizeof...(V)) <= 4);
		}
	};

	#if MUU_HAS_VECTORCALL

	template <typename Scalar, size_t Dimensions>
	inline constexpr bool is_hva<vector_base<Scalar, Dimensions>> =
		Dimensions <= 4
		&& (is_same_as_any<Scalar, float, double, long double> || is_simd_intrinsic<Scalar>)
		&& sizeof(vector_base<Scalar, Dimensions>) == sizeof(Scalar) * Dimensions
		&& alignof(vector_base<Scalar, Dimensions>) == alignof(Scalar)
	;

	template <typename Scalar, size_t Dimensions>
	inline constexpr bool is_hva<vector<Scalar, Dimensions>> = is_hva<vector_base<Scalar, Dimensions>>;

	#endif // MUU_HAS_VECTORCALL

	template <typename Scalar, size_t Dimensions>
	inline constexpr bool pass_vector_by_value
		= !std::is_reference_v<maybe_pass_readonly_by_value<vector_base<Scalar, Dimensions>>>;

	template <typename Scalar, size_t Dimensions>
	inline constexpr bool pass_vector_by_reference = !pass_vector_by_value<Scalar, Dimensions>;

	template <typename T>
	[[nodiscard]]
	MUU_ALWAYS_INLINE
	MUU_ATTR(const)
	static constexpr auto MUU_VECTORCALL modulo(T lhs, T rhs) noexcept
	{
		if constexpr (is_floating_point<T>)
		{
			using type = smallest<promote_if_small_float<T>, long double>;
			return std::fmod(static_cast<type>(lhs), static_cast<type>(rhs));
		}
		else
			return lhs % rhs;
	}

	MUU_API void print_vector_to_stream(std::ostream& stream, const half*, size_t);
	MUU_API void print_vector_to_stream(std::ostream& stream, const float*, size_t);
	MUU_API void print_vector_to_stream(std::ostream& stream, const double*, size_t);
	MUU_API void print_vector_to_stream(std::ostream& stream, const long double*, size_t);
	MUU_API void print_vector_to_stream(std::ostream& stream, const signed char*, size_t);
	MUU_API void print_vector_to_stream(std::ostream& stream, const signed short*, size_t);
	MUU_API void print_vector_to_stream(std::ostream& stream, const signed int*, size_t);
	MUU_API void print_vector_to_stream(std::ostream& stream, const signed long*, size_t);
	MUU_API void print_vector_to_stream(std::ostream& stream, const signed long long*, size_t);
	MUU_API void print_vector_to_stream(std::ostream& stream, const unsigned char*, size_t);
	MUU_API void print_vector_to_stream(std::ostream& stream, const unsigned short*, size_t);
	MUU_API void print_vector_to_stream(std::ostream& stream, const unsigned int*, size_t);
	MUU_API void print_vector_to_stream(std::ostream& stream, const unsigned long*, size_t);
	MUU_API void print_vector_to_stream(std::ostream& stream, const unsigned long long*, size_t);
	#if MUU_HAS_FLOAT16
	MUU_API void print_vector_to_stream(std::ostream& stream, const __Float16*, size_t);
	#endif
	#if MUU_HAS_FP16
	MUU_API void print_vector_to_stream(std::ostream& stream, const __fp16*, size_t);
	#endif
	#if MUU_HAS_FLOAT128
	MUU_API void print_vector_to_stream(std::ostream& stream, const quad*, size_t);
	#endif
	#if MUU_HAS_INT128
	MUU_API void print_vector_to_stream(std::ostream& stream, const int128_t*, size_t);
	MUU_API void print_vector_to_stream(std::ostream& stream, const uint128_t*, size_t);
	#endif

	MUU_API void print_vector_to_stream(std::wostream& stream, const half*, size_t);
	MUU_API void print_vector_to_stream(std::wostream& stream, const float*, size_t);
	MUU_API void print_vector_to_stream(std::wostream& stream, const double*, size_t);
	MUU_API void print_vector_to_stream(std::wostream& stream, const long double*, size_t);
	MUU_API void print_vector_to_stream(std::wostream& stream, const signed char*, size_t);
	MUU_API void print_vector_to_stream(std::wostream& stream, const signed short*, size_t);
	MUU_API void print_vector_to_stream(std::wostream& stream, const signed int*, size_t);
	MUU_API void print_vector_to_stream(std::wostream& stream, const signed long*, size_t);
	MUU_API void print_vector_to_stream(std::wostream& stream, const signed long long*, size_t);
	MUU_API void print_vector_to_stream(std::wostream& stream, const unsigned char*, size_t);
	MUU_API void print_vector_to_stream(std::wostream& stream, const unsigned short*, size_t);
	MUU_API void print_vector_to_stream(std::wostream& stream, const unsigned int*, size_t);
	MUU_API void print_vector_to_stream(std::wostream& stream, const unsigned long*, size_t);
	MUU_API void print_vector_to_stream(std::wostream& stream, const unsigned long long*, size_t);
	#if MUU_HAS_FLOAT16
	MUU_API void print_vector_to_stream(std::wostream& stream, const __Float16*, size_t);
	#endif
	#if MUU_HAS_FP16
	MUU_API void print_vector_to_stream(std::wostream& stream, const __fp16*, size_t);
	#endif
	#if MUU_HAS_FLOAT128
	MUU_API void print_vector_to_stream(std::wostream& stream, const quad*, size_t);
	#endif
	#if MUU_HAS_INT128
	MUU_API void print_vector_to_stream(std::wostream& stream, const int128_t*, size_t);
	MUU_API void print_vector_to_stream(std::wostream& stream, const uint128_t*, size_t);
	#endif
}
MUU_IMPL_NAMESPACE_END

#else // ^^^ !DOXYGEN / DOXYGEN vvv

#define ENABLE_IF_AT_LEAST_DIMENSIONS(...)
#define ENABLE_IF_AT_LEAST_DIMENSIONS_AND(...)
#define	REQUIRES_AT_LEAST_DIMENSIONS(...)
#define	REQUIRES_EXACTLY_DIMENSIONS(...)
#define	REQUIRES_FLOATING_POINT
#define REQUIRES_SIGNED

#endif // DOXYGEN

MUU_NAMESPACE_START
{
	/// \brief An N-dimensional vector.
	///
	/// \tparam	Scalar      The type of the vector's scalar components.
	/// \tparam Dimensions  The number of dimensions.
	template <typename Scalar, size_t Dimensions>
	struct MUU_TRIVIAL_ABI vector
		#ifndef DOXYGEN
		: impl::vector_base<Scalar, Dimensions>
		#endif
	{
		static_assert(
			!std::is_reference_v<Scalar>,
			"Scalar type cannot be a reference"
		);
		static_assert(
			!std::is_const_v<Scalar> && !std::is_volatile_v<Scalar>,
			"Scalar type cannot be const- or volatile-qualified"
		);
		static_assert(
			std::is_trivially_constructible_v<Scalar>
			&& std::is_trivially_copyable_v<Scalar>
			&& std::is_trivially_destructible_v<Scalar>,
			"Scalar type must be trivially constructible, copyable and destructible"
		);
		static_assert(
			Dimensions >= 1,
			"Vectors must have at least one dimension"
		);

		/// \brief The type of each scalar component stored in this vector.
		using scalar_type = Scalar;

		/// \brief The type used to return scalar products (length/distance dot, etc.)
		using product_type = std::conditional_t<is_integral<scalar_type>, double, scalar_type>;

		/// \brief The number of scalar components stored in this vector.
		static constexpr size_t dimensions = Dimensions;

		/// \brief Compile-time constants for this vector type.
		using constants = muu::constants<vector>;

		/// \brief Compile-time constants for this vector's scalar type.
		using scalar_constants = muu::constants<scalar_type>;

		/// \brief `vector` or `const vector&`, depending on depending on size, triviality, simd-friendliness, etc.
		using vector_param = std::conditional_t<
			impl::pass_vector_by_value<scalar_type, dimensions>,
			vector,
			const vector&
		>;

	private:

		template <typename T, size_t N>
		friend struct vector;
		using base = impl::vector_base<Scalar, Dimensions>;
		static_assert(
			sizeof(base) == (sizeof(Scalar) * Dimensions),
			"Vectors should not have padding"
		);
		using intermediate_type = impl::promote_if_small_float<product_type>;

	public:

		/// \brief A LegacyRandomAccessIterator for the scalar components in the vector.
		using iterator = scalar_type*;

		/// \brief A const LegacyRandomAccessIterator for the scalar components in the vector.
		using const_iterator = const scalar_type*;

		#ifdef DOXYGEN
		/// \brief The vector's X scalar component (when dimensions &lt;= 4).
		scalar_type x;
		/// \brief The vector's Y scalar component (when dimensions &lt;= 4).
		scalar_type y;
		/// \brief The vector's Z scalar component (when dimensions &lt;= 4).
		scalar_type z;
		/// \brief The vector's W scalar component (when dimensions &lt;= 4).
		scalar_type w;
		/// \brief The array of scalar components stored in the vector (when dimensions &gt; 4).
		scalar_type values[dimensions];
		#endif

	#if 1 // scalar component accessors -------------------------------------------------------------------------------

	private:

		template <size_t Index, typename T>
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(pure)
		static constexpr auto& do_get(T& vec) noexcept
		{
			static_assert(
				Index < Dimensions,
				"Element index out of range"
			);

			if constexpr (Dimensions <= 4)
			{
				if constexpr (Index == 0) return vec.x;
				if constexpr (Index == 1) return vec.y;
				if constexpr (Index == 2) return vec.z;
				if constexpr (Index == 3) return vec.w;
			}
			else
				return vec.values[Index];
		}

		template <typename T>
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(pure)
		static constexpr auto& do_array_operator(T& vec, size_t idx) noexcept
		{
			MUU_ASSUME(idx < Dimensions);

			if constexpr (Dimensions <= 4)
			{
				if (!build::supports_is_constant_evaluated || is_constant_evaluated())
				{
					switch (idx)
					{
						case 0:	return vec.x;
						case 1:	if constexpr (Dimensions > 1) { return vec.y; } else { MUU_UNREACHABLE; }
						case 2:	if constexpr (Dimensions > 2) { return vec.z; } else { MUU_UNREACHABLE; }
						case 3:	if constexpr (Dimensions > 3) { return vec.w; } else { MUU_UNREACHABLE; }
						MUU_NO_DEFAULT_CASE;
					}
				}
				else
					return *(&vec.x + idx);
			}
			else
				return vec.values[idx];
		}

	public:

		/// \brief Gets a reference to the scalar component at a specific index.
		/// 
		/// \tparam Index  The index of the scalar component to retrieve, where x == 0, y == 1, etc.
		///
		/// \return  A reference to the selected scalar component.
		template <size_t Index>
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(pure)
		MUU_ATTR(flatten)
		constexpr const scalar_type& get() const noexcept
		{
			return do_get<Index>(*this);
		}

		/// \brief Gets a reference to the scalar component at a specific index.
		/// 
		/// \tparam Index  The index of the scalar component to retrieve, where x == 0, y == 1, etc.
		///
		/// \return  A reference to the selected scalar component.
		template <size_t Index>
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(pure)
		MUU_ATTR(flatten)
		constexpr scalar_type& get() noexcept
		{
			return do_get<Index>(*this);
		}

		/// \brief Gets a reference to the Nth scalar component.
		///
		/// \param idx  The index of the scalar component to retrieve, where x == 0, y == 1, etc.
		///
		/// \return  A reference to the selected scalar component.
		/// 
		/// \warning No bounds-checking is done!
		[[nodiscard]]
		MUU_ATTR(pure)
		MUU_ATTR(flatten)
		constexpr const scalar_type& operator [](size_t idx) const noexcept
		{
			return do_array_operator(*this, idx);
		}

		/// \brief Gets a reference to the Nth scalar component.
		///
		/// \param idx  The index of the scalar component to retrieve, where x == 0, y == 1, etc. 
		///
		/// \return  A reference to the selected scalar component.
		/// 
		/// \warning No bounds-checking is done!
		[[nodiscard]]
		MUU_ATTR(pure)
		MUU_ATTR(flatten)
		constexpr scalar_type& operator [](size_t idx) noexcept
		{
			return do_array_operator(*this, idx);
		}

	#endif // scalar component accessors

	#if 1 // constructors ---------------------------------------------------------------------------------------------

	private:

		template <typename Func>
		explicit constexpr vector(impl::elementwise_func_tag, Func&& func) noexcept
			: base{
				impl::elementwise_func_tag{},
				std::make_index_sequence<Dimensions>{},
				std::forward<Func>(func)
			}
		{}

	public:

		/// \brief Default constructor. Scalar components are not initialized.
		vector() noexcept = default;

		/// \brief Copy constructor.
		constexpr vector(const vector&) noexcept = default;

		/// \brief Copy-assigment operator.
		constexpr vector& operator = (const vector&) noexcept = default;

		/// \brief	Constructs a vector with all scalar components set to the same value.
		///
		/// \param	fill	The value used to initialize each of the vector's scalar components.
		MUU_NODISCARD_CTOR
		explicit constexpr vector(scalar_type fill) noexcept
			: base{ impl::value_fill_tag{}, fill }
		{}

		/// \brief		Constructs a vector from two scalar values.
		/// \details	Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \param	x	Initial value for the vector's x scalar component.
		/// \param	y	Initial value for the vector's y scalar component.
		/// 
		/// \attention This constructor is only available when the vector has &gt;= 2 dimensions.
		REQUIRES_AT_LEAST_DIMENSIONS(2)
		MUU_NODISCARD_CTOR
		constexpr vector(scalar_type x, scalar_type y) noexcept
			: base{ x, y }
		{}

		/// \brief		Constructs a vector from three scalar values.
		/// \details	Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \param	x	Initial value for the vector's x scalar component.
		/// \param	y	Initial value for the vector's y scalar component.
		/// \param	z	Initial value for the vector's z scalar component.
		/// 
		/// \attention This constructor is only available when the vector has &gt;= 3 dimensions.
		REQUIRES_AT_LEAST_DIMENSIONS(3)
		MUU_NODISCARD_CTOR
		constexpr vector(scalar_type x, scalar_type y, scalar_type z) noexcept
			: base{ x, y, z }
		{}

		/// \brief		Constructs a vector from four scalar values.
		/// \details	Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \param	x		Initial value for the vector's x scalar component.
		/// \param	y		Initial value for the vector's y scalar component.
		/// \param	z		Initial value for the vector's z scalar component.
		/// \param	w		Initial value for the vector's w scalar component.
		/// 
		/// \attention This constructor is only available when the vector has &gt;= 4 dimensions.
		REQUIRES_AT_LEAST_DIMENSIONS(4)
		MUU_NODISCARD_CTOR
		constexpr vector(scalar_type x, scalar_type y, scalar_type z, scalar_type w) noexcept
			: base{ x, y, z, w }
		{}

		/// \brief		Constructs a vector from five or more scalar values.
		/// \details	Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \tparam T		Types convertible to scalar_type.
		/// \param	x		Initial value for the vector's x scalar component.
		/// \param	y		Initial value for the vector's y scalar component.
		/// \param	z		Initial value for the vector's z scalar component.
		/// \param	w		Initial value for the vector's w scalar component.
		/// \param	vals	Initial values for the vector's remaining scalar components.
		/// 
		/// \attention This constructor is only available when the vector has &gt;= 5 dimensions.
		template <typename... T ENABLE_IF_AT_LEAST_DIMENSIONS_AND(5, all_convertible_to<scalar_type, T...>)>
		MUU_NODISCARD_CTOR
		constexpr vector(scalar_type x, scalar_type y, scalar_type z, scalar_type w, T... vals) noexcept
			: base{ x, y, z, w, vals... }
		{}

		/// \brief Constructs a vector from a raw array.
		/// \details	Any scalar components not covered by the constructor's parameters are initialized to zero.
		/// 
		/// \tparam T			A type convertible to scalar_type.
		/// \tparam N			The number of elements in the array.
		/// \param	arr			Array of values used to initialize the vector's scalar components.
		template <typename T, size_t N ENABLE_IF_AT_LEAST_DIMENSIONS_AND(N, all_convertible_to<scalar_type, T>)>
		MUU_NODISCARD_CTOR
		explicit constexpr vector(const T(& arr)[N]) noexcept
			: base{ impl::array_cast_tag{}, std::make_index_sequence<N>{}, arr }
		{}

		/// \brief Constructs a vector from a std::array.
		/// \details	Any scalar components not covered by the constructor's parameters are initialized to zero.
		/// 
		/// \tparam T			A type convertible to scalar_type.
		/// \tparam N			The number of elements in the array.
		/// \param	arr			Array of values used to initialize the vector's scalar components.
		template <typename T, size_t N ENABLE_IF_AT_LEAST_DIMENSIONS_AND(N, all_convertible_to<scalar_type, T>)>
		MUU_NODISCARD_CTOR
		explicit constexpr vector(const std::array<T, N>& arr) noexcept
			: base{ impl::array_cast_tag{}, std::make_index_sequence<N>{}, arr }
		{}

		/// \brief Constructs a vector from any tuple-like type.
		/// 
		/// \tparam T			A tuple-like type.
		/// 
		/// \details	Any scalar components not covered by the constructor's parameters are initialized to zero.
		template <typename T ENABLE_IF_AT_LEAST_DIMENSIONS_AND(tuple_size<T>, is_tuple_like<T>)>
		MUU_NODISCARD_CTOR
		explicit constexpr vector(const T& tuple) noexcept
			: base{ impl::tuple_cast_tag{}, std::make_index_sequence<tuple_size<T>>{}, tuple }
		{}

		/// \brief Enlarging/truncating/converting constructor.
		/// \details Copies source vector's scalar components, casting if necessary:
		/// \cpp
		/// const vector<float, 3> xyz  = { 1, 2, 3 };
		/// const vector<float, 2> xy   = { xyz }; // { 1, 2 }
		/// const vector<float, 4> xyzw = { xyz }; // { 1, 2, 3, 0 }
		/// const auto rect   = vector{ point1, point2 }; // { 1, 2, 10, 15 }
		/// \ecpp
		/// Any scalar components not covered by the constructor's parameters are initialized to zero.
		/// 
		/// \tparam	S		Source vector's scalar_type.
		/// \tparam	D		Source vector's dimensions.
		/// \param 	vec		Source vector.
		template <typename S, size_t D>
		MUU_NODISCARD_CTOR
		explicit constexpr vector(const vector<S, D>& vec) noexcept
			: base{ impl::tuple_cast_tag{}, std::make_index_sequence<(D < Dimensions ? D : Dimensions)>{}, vec }
		{}

		/// \brief	Concatenating constructor.
		/// \details Copies the scalar components from vector 1 and then vector 2 contiguously into the new vector:
		/// \cpp
		/// const vector<float, 2> point1 = { 1, 2 };
		/// const vector<float, 2> point2 = { 10, 15 };
		/// const vector<float, 4> rect   = { point1, point2 }; // { 1, 2, 10, 15 }
		/// \ecpp
		/// Any scalar components not covered by the constructor's parameters are initialized to zero.
		///  
		/// \tparam	S1  	Vector 1's scalar_type.
		/// \tparam	D1		Vector 1's dimensions.
		/// \tparam	S2  	Vector 2's scalar_type.
		/// \tparam	D2		Vector 2's dimensions.
		/// \param 	vec1	Vector 1.
		/// \param 	vec2	Vector 2.
		template <typename S1, size_t D1, typename S2, size_t D2 ENABLE_IF_AT_LEAST_DIMENSIONS(D1 + D2)>
		MUU_NODISCARD_CTOR
			explicit constexpr vector(const vector<S1, D1>& vec1, const vector<S2, D2>& vec2) noexcept
			: base{
				impl::tuple_concat_tag{},
				std::make_index_sequence<D1>{}, vec1,
				std::make_index_sequence<D2>{}, vec2
			}
		{}

		/// \brief	Appending constructor.
		/// \details Copies the scalar components from the vector and then the list of scalars contiguously into the new vector:
		/// \cpp
		/// const vector<float, 2> point = { 1, 2 };
		/// const vector<float, 4> rect  = { point, 10, 15 }; // { 1, 2, 10, 15 }
		/// \ecpp
		/// Any scalar components not covered by the constructor's parameters are initialized to zero.
		///  
		/// \tparam	S	  	Vector's scalar_type.
		/// \tparam	D		Vector's dimensions.
		/// \tparam T		Types convertible to scalar_type.
		/// \param 	vec		A vector.
		/// \param 	vals	Scalar values.
		template <typename S, size_t D, typename... T ENABLE_IF_AT_LEAST_DIMENSIONS_AND(D + sizeof...(T), all_convertible_to<scalar_type, T...>)>
		MUU_NODISCARD_CTOR
			explicit constexpr vector(const vector<S, D>& vec, T... vals) noexcept
			: base{
				impl::tuple_concat_tag{},
				std::make_index_sequence<D>{}, vec,
				vals...
			}
		{}


		#if !defined(DOXYGEN) && !MUU_INTELLISENSE

		template <size_t N ENABLE_IF_AT_LEAST_DIMENSIONS(N)>
		MUU_NODISCARD_CTOR
		explicit constexpr vector(const scalar_type(&arr)[N]) noexcept
			: base{ impl::array_copy_tag{}, std::make_index_sequence<N>{}, arr }
		{}

		template <size_t N ENABLE_IF_AT_LEAST_DIMENSIONS(N)>
		MUU_NODISCARD_CTOR
		explicit constexpr vector(const std::array<scalar_type, N>& arr) noexcept
			: base{ impl::array_copy_tag{}, std::make_index_sequence<N>{}, arr }
		{}

		template <size_t N>
		MUU_NODISCARD_CTOR
		explicit constexpr vector(const vector<scalar_type, N>& vec) noexcept
			: base{ impl::tuple_copy_tag{}, std::make_index_sequence<(N < Dimensions ? N : Dimensions)>{}, vec }
		{}

		#endif // !DOXYGEN && !MUU_INTELLISENSE

	#endif // constructors

	#if 1 // equality -------------------------------------------------------------------------------------------------

		/// \brief	Returns true if two vectors are exactly equal.
		template <typename T MUU_SFINAE(MUU_INTELLISENSE || impl::pass_vector_by_reference<T, dimensions>)>
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool MUU_VECTORCALL equal(vector_param a, const vector<T, dimensions>& b) noexcept
		{
			#define VEC_FUNC(member)	a.member == b.member
			COMPONENTWISE_AND(VEC_FUNC);
			#undef VEC_FUNC
		}

		/// \brief	Returns true if the vector is exactly equal to another.
		template <typename T MUU_SFINAE(MUU_INTELLISENSE || impl::pass_vector_by_reference<T, dimensions>)>
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool MUU_VECTORCALL equal(const vector<T, dimensions>& other) const noexcept
		{
			return equal(*this, other);
		}

		/// \brief	Returns true if two vectors are exactly equal.
		template <typename T MUU_SFINAE(MUU_INTELLISENSE || impl::pass_vector_by_reference<T, dimensions>)>
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr bool MUU_VECTORCALL operator == (vector_param lhs, const vector<T, dimensions>& rhs) noexcept
		{
			return equal(lhs, rhs);
		}

		/// \brief	Returns true if two vectors are not exactly equal.
		template <typename T MUU_SFINAE(MUU_INTELLISENSE || impl::pass_vector_by_reference<T, dimensions>)>
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr bool MUU_VECTORCALL operator != (vector_param lhs, const vector<T, dimensions>& rhs) noexcept
		{
			return !equal(lhs, rhs);
		}

		#if !defined(DOXYEN) && !MUU_INTELLISENSE

		template <typename T MUU_SFINAE_2(impl::pass_vector_by_value<T, dimensions>)>
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool MUU_VECTORCALL equal(vector_param a, vector<T, dimensions> b) noexcept
		{
			#define VEC_FUNC(member)	a.member == b.member
			COMPONENTWISE_AND(VEC_FUNC);
			#undef VEC_FUNC
		}

		template <typename T MUU_SFINAE_2(impl::pass_vector_by_value<T, dimensions>)>
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool MUU_VECTORCALL equal(vector<T, dimensions> other) const noexcept
		{
			return equal(*this, other);
		}

		template <typename T MUU_SFINAE_2(impl::pass_vector_by_value<T, dimensions>)>
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr bool MUU_VECTORCALL operator == (vector_param lhs, vector<T, dimensions> rhs) noexcept
		{
			return equal(lhs, rhs);
		}

		template <typename T MUU_SFINAE_2(impl::pass_vector_by_value<T, dimensions>)>
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr bool MUU_VECTORCALL operator != (vector_param lhs, vector<T, dimensions> rhs) noexcept
		{
			return !equal(lhs, rhs);
		}

		#endif // !DOXYEN && !MUU_INTELLISENSE

		/// \brief	Returns true if all the scalar components of the vector are exactly zero.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool zero() const noexcept
		{
			#define VEC_FUNC(member)	base::member == scalar_constants::zero
			COMPONENTWISE_AND(VEC_FUNC);
			#undef VEC_FUNC
		}

		/// \brief	Returns true if any of the scalar components of the vector are infinity or NaN.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool infinity_or_nan() const noexcept
		{
			if constexpr (is_floating_point<Scalar>)
			{
				#define VEC_FUNC(member)	muu::infinity_or_nan(base::member)
				COMPONENTWISE_OR(VEC_FUNC);
				#undef VEC_FUNC
			}
			else
				return false;
		}

	#endif // equality

	#if 1 // approx_equal ---------------------------------------------------------------------------------------------

		/// \brief	Returns true if two vectors are approximately equal.
		template <typename T
			MUU_SFINAE(any_floating_point<Scalar, T> && (MUU_INTELLISENSE || impl::pass_vector_by_reference<T, dimensions>))
		>
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool MUU_VECTORCALL approx_equal(
			vector_param a,
			const vector<T, dimensions>& b,
			impl::highest_ranked<scalar_type, T> epsilon
				= muu::constants<impl::highest_ranked<scalar_type, T>>::approx_equal_epsilon
		) noexcept
		{
			using type = impl::highest_ranked<
				impl::promote_if_small_float<Scalar>,
				impl::promote_if_small_float<T>
			>;

			#define VEC_FUNC(member)															\
				static_cast<bool(MUU_VECTORCALL *)(type,type,type)noexcept>(muu::approx_equal)(	\
					a.member,																	\
					b.member,																	\
					epsilon																		\
				)
			COMPONENTWISE_AND(VEC_FUNC);
			#undef VEC_FUNC
		}

		/// \brief	Returns true if the vector is approximately equal to another.
		template <typename T
			MUU_SFINAE(any_floating_point<Scalar, T> && (MUU_INTELLISENSE || impl::pass_vector_by_reference<T, dimensions>))
		>
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool MUU_VECTORCALL approx_equal(
			const vector<T, dimensions>& other,
			impl::highest_ranked<scalar_type, T> epsilon
				= muu::constants<impl::highest_ranked<scalar_type, T>>::approx_equal_epsilon
		) const noexcept
		{
			return approx_equal(*this, other, epsilon);
		}

		#if !defined(DOXYEN) && !MUU_INTELLISENSE

		template <typename T MUU_SFINAE_2(any_floating_point<Scalar, T> && impl::pass_vector_by_value<T, dimensions>)>
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool MUU_VECTORCALL approx_equal(
			vector_param a,
			vector<T, dimensions> b,
			impl::highest_ranked<scalar_type, T> epsilon
				= muu::constants<impl::highest_ranked<scalar_type, T>>::approx_equal_epsilon
		) noexcept
		{
			using type = impl::highest_ranked<
				impl::promote_if_small_float<Scalar>,
				impl::promote_if_small_float<T>
			>;

			#define VEC_FUNC(member)															\
				static_cast<bool(MUU_VECTORCALL *)(type,type,type)noexcept>(muu::approx_equal)(	\
					a.member,																	\
					b.member,																	\
					epsilon																		\
				)
			COMPONENTWISE_AND(VEC_FUNC);
			#undef VEC_FUNC
		}

		template <typename T MUU_SFINAE_2(any_floating_point<Scalar, T> && impl::pass_vector_by_value<T, dimensions>)>
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool MUU_VECTORCALL approx_equal(
			vector<T, dimensions> other,
			impl::highest_ranked<scalar_type, T> epsilon
				= muu::constants<impl::highest_ranked<scalar_type, T>>::approx_equal_epsilon
		) const noexcept
		{
			return approx_equal(*this, other, epsilon);
		}

		#endif // !DOXYEN && !MUU_INTELLISENSE

	#endif // approx_equal

	#if 1 // length and distance --------------------------------------------------------------------------------------

	private:

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr intermediate_type MUU_VECTORCALL raw_length_squared(vector_param v) noexcept
		{
			#define VEC_FUNC(member)	static_cast<intermediate_type>(v.member) * static_cast<intermediate_type>(v.member)
			COMPONENTWISE_ACCUMULATE(VEC_FUNC);
			#undef VEC_FUNC
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr intermediate_type MUU_VECTORCALL raw_length(vector_param v) noexcept
		{
			if constexpr (Dimensions == 1)
				return static_cast<intermediate_type>(v.x);
			else
				return muu::sqrt(raw_length_squared(v));
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr intermediate_type MUU_VECTORCALL raw_distance_squared(vector_param p1, vector_param p2) noexcept
		{
			constexpr auto subtract_and_square = [](scalar_type lhs, scalar_type rhs) noexcept
			{
				FMA_BLOCK

				const auto temp = static_cast<intermediate_type>(lhs) - static_cast<intermediate_type>(rhs);
				return temp * temp;
			};

			#define VEC_FUNC(member)	subtract_and_square(p2.member, p1.member)
			COMPONENTWISE_ACCUMULATE(VEC_FUNC);
			#undef VEC_FUNC
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr intermediate_type MUU_VECTORCALL raw_distance(vector_param p1, vector_param p2) noexcept
		{
			if constexpr (Dimensions == 1)
				return static_cast<intermediate_type>(p2.x) - static_cast<intermediate_type>(p1.x);
			else
				return muu::sqrt(raw_distance_squared(p1, p2));
		}

	public:

		/// \brief	Returns the squared length (magnitude) of a vector.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr product_type MUU_VECTORCALL length_squared(vector_param v) noexcept
		{
			return static_cast<product_type>(raw_length_squared(v));
		}

		/// \brief	Returns the squared length (magnitude) of the vector.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr product_type MUU_VECTORCALL length_squared() const noexcept
		{
			return static_cast<product_type>(raw_length_squared(*this));
		}

		/// \brief	Returns the length (magnitude) of a vector.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr product_type MUU_VECTORCALL length(vector_param v) noexcept
		{
			return static_cast<product_type>(raw_length(v));
		}

		/// \brief	Returns the length (magnitude) of the vector.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr product_type length() const noexcept
		{
			return static_cast<product_type>(raw_length(*this));
		}

		/// \brief	Returns the squared distance between two point vectors.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr product_type MUU_VECTORCALL distance_squared(vector_param p1, vector_param p2) noexcept
		{
			return static_cast<product_type>(raw_distance_squared(p1, p2));
		}

		/// \brief	Returns the squared distance between this and another point vector.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr product_type MUU_VECTORCALL distance_squared(vector_param p) const noexcept
		{
			return static_cast<product_type>(raw_distance_squared(*this, p));
		}

		/// \brief	Returns the squared distance between two point vectors.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr product_type MUU_VECTORCALL distance(vector_param p1, vector_param p2) noexcept
		{
			return static_cast<product_type>(raw_distance(p1, p2));
		}

		/// \brief	Returns the squared distance between this and another point vector.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr product_type MUU_VECTORCALL distance(vector_param p) const noexcept
		{
			return static_cast<product_type>(raw_distance(*this, p));
		}

		/// \brief Returns true if the vector has a length of 1.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr bool unit_length() const noexcept
		{
			if constexpr (is_integral<scalar_type>)
			{
				if constexpr (Dimensions == 1)
					return base::x == scalar_type{ 1 };
				else
				{
					using sum_type = decltype(scalar_type{} + scalar_type{});
					sum_type sum{};
					for (size_t i = 0u; i < dimensions && sum > sum_type{ 1 }; i++)
						sum += (*this)[i];
					return sum == sum_type{ 1 };
				}
			}
			else
			{
				if constexpr (Dimensions == 1)
					return muu::approx_equal(static_cast<intermediate_type>(base::x), intermediate_type{ 1 });
				else 
				{
					constexpr auto epsilon = intermediate_type{ 1 } / (
						100ull
						* (sizeof(scalar_type) >= sizeof(float)  ? 10000ull : 1ull)
						* (sizeof(scalar_type) >= sizeof(double) ? 10000ull : 1ull)
					);

					return muu::approx_equal(
						raw_length_squared(*this),
						intermediate_type{ 1 },
						epsilon
					);
				}
			}
		}

	#endif // length and distance

	#if 1 // dot and cross products -----------------------------------------------------------------------------------

	private:

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr intermediate_type MUU_VECTORCALL raw_dot(vector_param v1, vector_param v2) noexcept
		{
			#define VEC_FUNC(member)	static_cast<intermediate_type>(v1.member) * static_cast<intermediate_type>(v2.member)
			COMPONENTWISE_ACCUMULATE(VEC_FUNC);
			#undef VEC_FUNC
		}

	public:

		/// \brief	Returns the dot product of two vectors.
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr product_type MUU_VECTORCALL dot(vector_param v1, vector_param v2) noexcept
		{
			return static_cast<product_type>(raw_dot(v1, v2));
		}

		/// \brief	Returns the dot product of this and another vector.
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr product_type MUU_VECTORCALL dot(vector_param v) const noexcept
		{
			return dot(*this, v);
		}

		/// \brief	Returns the cross product of two three-dimensional vectors.
		REQUIRES_EXACTLY_DIMENSIONS(3)
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr vector<product_type, 3> MUU_VECTORCALL cross(vector_param lhs, vector_param rhs) noexcept
		{
			FMA_BLOCK

			return
			{
				static_cast<product_type>(
					static_cast<intermediate_type>(lhs.y) * static_cast<intermediate_type>(rhs.z)
					- static_cast<intermediate_type>(lhs.z) * static_cast<intermediate_type>(rhs.y)
				),
				static_cast<product_type>(
					static_cast<intermediate_type>(lhs.z) * static_cast<intermediate_type>(rhs.x)
					- static_cast<intermediate_type>(lhs.x) * static_cast<intermediate_type>(rhs.z)
				),
				static_cast<product_type>(
					static_cast<intermediate_type>(lhs.x) * static_cast<intermediate_type>(rhs.y)
					- static_cast<intermediate_type>(lhs.y) * static_cast<intermediate_type>(rhs.x)
				)
			};
		}

		/// \brief	Returns the cross product of this and another three-dimensional vector.
		REQUIRES_EXACTLY_DIMENSIONS(3)
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr vector<product_type, 3> MUU_VECTORCALL cross(vector_param v) const noexcept
		{
			return cross(*this, v);
		}

	#endif // dot and cross products

	#if 1 // addition -------------------------------------------------------------------------------------------------

		/// \brief Returns the componentwise addition of two vectors.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr vector MUU_VECTORCALL operator + (vector_param lhs, vector_param rhs) noexcept
		{
			#define VEC_FUNC(member)	lhs.member + rhs.member
			COMPONENTWISE_CONSTRUCT(VEC_FUNC);
			#undef VEC_FUNC
		}

		/// \brief Componentwise adds another vector to this one.
		constexpr vector& MUU_VECTORCALL operator += (vector_param rhs) noexcept
		{
			#define VEC_FUNC(member)	base::member + rhs.member
			COMPONENTWISE_ASSIGN(VEC_FUNC);
			#undef VEC_FUNC
		}

	#endif // addition

	#if 1 // subtraction ----------------------------------------------------------------------------------------------

		/// \brief Returns the componentwise subtraction of one vector from another.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr vector MUU_VECTORCALL operator - (vector_param lhs, vector_param rhs) noexcept
		{
			#define VEC_FUNC(member)	lhs.member - rhs.member
			COMPONENTWISE_CONSTRUCT(VEC_FUNC);
			#undef VEC_FUNC
		}

		/// \brief Componentwise subtracts another vector from this one.
		constexpr vector& MUU_VECTORCALL operator -= (vector_param rhs) noexcept
		{
			#define VEC_FUNC(member)	base::member - rhs.member
			COMPONENTWISE_ASSIGN(VEC_FUNC);
			#undef VEC_FUNC
		}

		/// \brief Returns the componentwise negation of a vector.
		REQUIRES_SIGNED
		[[nodiscard]]
		MUU_ATTR(pure)
		constexpr vector operator - () const noexcept
		{
			#define VEC_FUNC(member)	-base::member
			COMPONENTWISE_CONSTRUCT(VEC_FUNC);
			#undef VEC_FUNC
		}

	#endif // subtraction

	#if 1 // multiplication -------------------------------------------------------------------------------------------

		/// \brief Returns the componentwise multiplication of two vectors.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr vector MUU_VECTORCALL operator * (vector_param lhs, vector_param rhs) noexcept
		{
			#define VEC_FUNC(member)	lhs.member * rhs.member
			COMPONENTWISE_CONSTRUCT(VEC_FUNC);
			#undef VEC_FUNC
		}

		/// \brief Componentwise multiplies this vector by another.
		constexpr vector& MUU_VECTORCALL operator *= (vector_param rhs) noexcept
		{
			#define VEC_FUNC(member)	base::member * rhs.member
			COMPONENTWISE_ASSIGN(VEC_FUNC);
			#undef VEC_FUNC
		}

	private:

		template <typename T>
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr vector MUU_VECTORCALL raw_multiply_scalar(vector_param lhs, T rhs) noexcept
		{
			static_assert(is_floating_point<scalar_type> == is_floating_point<T>);

			#define VEC_FUNC(member)	lhs.member * rhs
			COMPONENTWISE_CONSTRUCT(VEC_FUNC);
			#undef VEC_FUNC
		}

		template <typename T>
		[[nodiscard]]
		constexpr vector& MUU_VECTORCALL raw_multiply_assign_scalar(T rhs) noexcept
		{
			static_assert(is_floating_point<scalar_type> == is_floating_point<T>);

			#define VEC_FUNC(member)	base::member * rhs
			COMPONENTWISE_ASSIGN(VEC_FUNC);
			#undef VEC_FUNC
		}

	public:

		/// \brief Returns the componentwise multiplication of a vector and a scalar.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr vector MUU_VECTORCALL operator * (vector_param lhs, scalar_type rhs) noexcept
		{
			return raw_multiply_scalar(lhs, rhs);
		}

		/// \brief Returns the componentwise multiplication of a vector and a scalar.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr vector MUU_VECTORCALL operator * (scalar_type lhs, vector_param rhs) noexcept
		{
			return raw_multiply_scalar(rhs, lhs);
		}


		/// \brief Componentwise multiplies this vector by a scalar.
		constexpr vector& MUU_VECTORCALL operator *= (scalar_type rhs) noexcept
		{
			return raw_multiply_assign_scalar(rhs);
		}

	#endif // multiplication

	#if 1 // division -------------------------------------------------------------------------------------------------

		/// \brief Returns the componentwise division of one vector by another.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr vector MUU_VECTORCALL operator / (vector_param lhs, vector_param rhs) noexcept
		{
			#define VEC_FUNC(member)	lhs.member / rhs.member
			COMPONENTWISE_CONSTRUCT(VEC_FUNC);
			#undef VEC_FUNC
		}

		/// \brief Componentwise divides this vector by another.
		constexpr vector& MUU_VECTORCALL operator /= (vector_param rhs) noexcept
		{
			#define VEC_FUNC(member)	base::member / rhs.member
			COMPONENTWISE_ASSIGN(VEC_FUNC);
			#undef VEC_FUNC
		}

	private:

		template <typename T>
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr vector MUU_VECTORCALL raw_divide_scalar(vector_param lhs, T rhs) noexcept
		{
			static_assert(is_floating_point<scalar_type> == is_floating_point<T>);

			if constexpr (is_floating_point<T>)
			{
				if constexpr (sizeof(T) < sizeof(float)) // half, _Float16 etc
				{
					const float inv = 1.0f / rhs;
					#define VEC_FUNC(member)	lhs.member * inv
					COMPONENTWISE_CONSTRUCT(VEC_FUNC);
					#undef VEC_FUNC
				}
				else
					return raw_multiply_scalar(lhs, T{ 1 } / rhs);
			}
			else
			{
				#define VEC_FUNC(member)	lhs.member / rhs
				COMPONENTWISE_CONSTRUCT(VEC_FUNC);
				#undef VEC_FUNC
			}
		}

		template <typename T>
		[[nodiscard]]
		constexpr vector& MUU_VECTORCALL raw_divide_assign_scalar(T rhs) noexcept
		{
			static_assert(is_floating_point<scalar_type> == is_floating_point<T>);

			if constexpr (is_floating_point<T>)
			{
				if constexpr (sizeof(T) < sizeof(float)) // half, _Float16 etc
				{
					const float inv = 1.0f / rhs;
					#define VEC_FUNC(member)	base::member * inv
					COMPONENTWISE_ASSIGN(VEC_FUNC);
					#undef VEC_FUNC
				}
				else
					return raw_multiply_assign_scalar(T{ 1 } / rhs);
			}
			else
			{
				#define VEC_FUNC(member)	base::member / rhs
				COMPONENTWISE_ASSIGN(VEC_FUNC);
				#undef VEC_FUNC
			}
		}

	public:

		/// \brief Returns the componentwise division of a vector and a scalar.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr vector MUU_VECTORCALL operator / (vector_param lhs, scalar_type rhs) noexcept
		{
			return raw_divide_scalar(lhs, rhs);
		}

		/// \brief Componentwise divides this vector by a scalar.
		constexpr vector& MUU_VECTORCALL operator /= (scalar_type rhs) noexcept
		{
			return raw_divide_assign_scalar(rhs);
		}

	#endif // division

	#if 1 // modulo ---------------------------------------------------------------------------------------------------

		/// \brief Returns the remainder of componentwise dividing of one vector by another.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr vector MUU_VECTORCALL operator % (vector_param lhs, vector_param rhs) noexcept
		{
			#define VEC_FUNC(member)	impl::modulo(lhs.member, rhs.member)
			COMPONENTWISE_CONSTRUCT(VEC_FUNC);
			#undef VEC_FUNC
		}

		/// \brief Assigns the result of componentwise dividing this vector by another.
		constexpr vector& MUU_VECTORCALL operator %= (vector_param rhs) noexcept
		{
			#define VEC_FUNC(member)	impl::modulo(base::member, rhs.member)
			COMPONENTWISE_ASSIGN(VEC_FUNC);
			#undef VEC_FUNC
		}

		/// \brief Returns the remainder of componentwise dividing vector by a scalar.
		[[nodiscard]]
		MUU_ATTR(pure)
		friend constexpr vector MUU_VECTORCALL operator % (vector_param lhs, scalar_type rhs) noexcept
		{
			#define VEC_FUNC(member)	impl::modulo(lhs.member, rhs)
			COMPONENTWISE_CONSTRUCT(VEC_FUNC);
			#undef VEC_FUNC
		}

		/// \brief Assigns the result of componentwise dividing this vector by a scalar.
		constexpr vector& MUU_VECTORCALL operator %= (scalar_type rhs) noexcept
		{
			#define VEC_FUNC(member)	impl::modulo(base::member, rhs)
			COMPONENTWISE_ASSIGN(VEC_FUNC);
			#undef VEC_FUNC
		}

	#endif // modulo

	#if 1 // normalization --------------------------------------------------------------------------------------------

		/// \brief	Normalizes a vector.
		///
		/// \param v			The vector to normalize.
		/// \param length_out	An output param to receive the length of the vector pre-normalization.
		/// 
		/// \return		A normalized copy of the input vector.
		/// 
		/// \remarks This function is only available when the scalar type is a float type.
		REQUIRES_FLOATING_POINT
		[[nodiscard]]
		static constexpr vector MUU_VECTORCALL normalize(vector_param v, scalar_type& length_out) noexcept
		{
			const auto len = raw_length(v);
			length_out = static_cast<scalar_type>(len);
			return raw_divide_scalar(v, len);
		}

		/// \brief	Normalizes a vector.
		///
		/// \param v	The vector to normalize.
		/// 
		/// \return		A normalized copy of the input vector.
		/// 
		/// \remarks This function is only available when the scalar type is a float type.
		REQUIRES_FLOATING_POINT
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr vector MUU_VECTORCALL normalize(vector_param v) noexcept
		{
			return raw_divide_scalar(v, raw_length(v));
		}

		/// \brief	Normalizes the vector in-place.
		///
		/// \param length_out	An output param to receive the length of the vector pre-normalization.
		/// 
		/// \return	A reference to the vector.
		/// 
		/// \remarks This function is only available when the scalar type is a float type.
		REQUIRES_FLOATING_POINT
		constexpr vector& normalize(scalar_type& length_out) noexcept
		{
			const auto len = raw_length(*this);
			length_out = static_cast<scalar_type>(len);
			return raw_divide_assign_scalar(len);
		}

		/// \brief	Normalizes the vector in-place.
		///
		/// \return	A reference to the vector.
		/// 
		/// \remarks This function is only available when the scalar type is a float type.
		REQUIRES_FLOATING_POINT
		constexpr vector& normalize() noexcept
		{
			return raw_divide_assign_scalar(raw_length(*this));
		}

	#endif // normalization

	#if 1 // iterators ------------------------------------------------------------------------------------------------

		/// \brief Returns an iterator to the first scalar component in the vector.
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(pure)
		constexpr iterator begin() noexcept
		{
			return &get<0>();
		}

		/// \brief Returns an iterator to the one-past-the-last scalar component in the vector.
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(pure)
		constexpr iterator end() noexcept
		{
			return begin() + Dimensions;
		}

		/// \brief Returns a const iterator to the first scalar component in the vector.
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(pure)
		constexpr const_iterator begin() const noexcept
		{
			return &get<0>();
		}

		/// \brief Returns a const iterator to the one-past-the-last scalar component in the vector.
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		MUU_ATTR(pure)
		constexpr const_iterator end() const noexcept
		{
			return begin() + Dimensions;
		}

	#endif // iterators

	#if 1 // streams --------------------------------------------------------------------------------------------------

		/// \brief Writes a vector out to a text stream.
		template <typename Char, typename Traits>
		friend std::basic_ostream<Char, Traits>& operator << (std::basic_ostream<Char, Traits>& os, const vector& vec)
		{
			impl::print_vector_to_stream(os, &vec.get<0>(), Dimensions);
			return os;
		}

	#endif // streams

	#if 1 // ________________ -----------------------------------------------------------------------------------------
	#endif // ________________

	#if 1 // ________________ -----------------------------------------------------------------------------------------
	#endif // ________________

	};
	
	#ifndef DOXYGEN // deduction guides -------------------------------------------------------------------------------

	template <typename T, typename U, typename... V>
	vector(T, U, V...) -> vector<impl::highest_ranked<std::remove_cv_t<T>, std::remove_cv_t<U>, std::remove_cv_t<V>...>, 2 + sizeof...(V)>;

	template <typename T>
	vector(T) -> vector<std::remove_cv_t<T>, 1>;

	template <typename T, size_t N>
	vector(const T(&)[N]) -> vector<std::remove_cv_t<T>, N>;

	template <typename T, size_t N>
	vector(const std::array<T, N>&) -> vector<std::remove_cv_t<T>, N>;

	template <typename T, typename U>
	vector(const std::pair<T, U>&) -> vector<impl::highest_ranked<std::remove_cv_t<T>, std::remove_cv_t<U>>, 2>;

	template <typename... T>
	vector(const std::tuple<T...>&) -> vector<impl::highest_ranked<std::remove_cv_t<T>...>, sizeof...(T)>;

	template <typename S1, size_t D1, typename S2, size_t D2>
	vector(const vector<S1, D1>&, const vector<S2, D2>&) -> vector<impl::highest_ranked<S1, S2>, D1 + D2>;

	template <typename S, size_t D, typename... T>
	vector(const vector<S, D>&, T...) -> vector<impl::highest_ranked<S, std::remove_cv_t<T>...>, D + sizeof...(T)>;

	#endif // deduction guides
}
MUU_NAMESPACE_END

namespace std
{
	/// \brief Specialization of std::tuple_size for muu::vector.
	template <typename Scalar, size_t Dimensions>
	struct tuple_size<muu::vector<Scalar, Dimensions>>
	{
		static constexpr size_t value = Dimensions;
	};

	/// \brief Specialization of std::tuple_element for muu::vector.
	template <size_t I, typename Scalar, size_t Dimensions>
	struct tuple_element<I, muu::vector<Scalar, Dimensions>>
	{
		using type = Scalar;
	};
}

#endif // =============================================================================================================

#undef ENABLE_IF_AT_LEAST_DIMENSIONS
#undef ENABLE_IF_AT_LEAST_DIMENSIONS_AND
#undef REQUIRES_AT_LEAST_DIMENSIONS
#undef REQUIRES_EXACTLY_DIMENSIONS
#undef REQUIRES_FLOATING_POINT
#undef REQUIRES_SIGNED
#undef COMPONENTWISE_AND
#undef COMPONENTWISE_OR
#undef COMPONENTWISE_ARITHMETIC_OPERATOR
#undef COMPONENTWISE_ACCUMULATE
#undef COMPONENTWISE_CASTING_OP_BRANCH
#undef COMPONENTWISE_CASTING_OP
#undef COMPONENTWISE_CONSTRUCT_WITH_TRANSFORM
#undef COMPONENTWISE_CONSTRUCT
#undef COMPONENTWISE_ASSIGN_WITH_TRANSFORM
#undef COMPONENTWISE_ASSIGN
#undef NULL_TRANSFORM
#undef FMA_BLOCK

MUU_PRAGMA_MSVC(inline_recursion(off))
MUU_PRAGMA_MSVC(float_control(pop))

MUU_POP_WARNINGS	// MUU_DISABLE_SHADOW_WARNINGS
