// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::vector.

#pragma once
#include "../muu/core.h"
MUU_DISABLE_WARNINGS
// ...
MUU_ENABLE_WARNINGS

MUU_PUSH_WARNINGS
MUU_DISABLE_ARITHMETIC_WARNINGS
MUU_DISABLE_LIFETIME_WARNINGS
MUU_DISABLE_SHADOW_WARNINGS
MUU_PRAGMA_MSVC(inline_recursion(on))


//=====================================================================================================================
// VECTOR CLASS IMPLEMENTATION
//=====================================================================================================================
#if 1

#ifndef DOXYGEN // Template Specialization cast Confusion on Doxygen! It's super effective!

	#define	ENABLE_IF_AT_LEAST_DIMENSIONS(dim) , size_t DIM = Dimensions, typename = std::enable_if_t<(DIM >= dim)>
	#define	ENABLE_IF_AT_LEAST_DIMENSIONS_AND(dim,...) , size_t DIM = Dimensions, typename = std::enable_if_t<(DIM >= dim) && (__VA_ARGS__)>
	#define	REQUIRES_AT_LEAST_DIMENSIONS(dim) template <size_t DIM = Dimensions, typename = std::enable_if_t<(DIM >= dim)>>

	MUU_IMPL_NAMESPACE_START
	{
		template <typename Scalar, size_t Dimensions>
		struct vector_base
		{
			Scalar values[Dimensions];

			constexpr vector_base() noexcept = default;

			explicit constexpr vector_base(Scalar fill) noexcept
				: values{} // todo: eliminate this somehow
			{
				for (auto& val : values)
					val = fill;
			}

			template <typename... T>
			explicit constexpr vector_base(Scalar x_, Scalar y_, T... vals) noexcept
				: values{ x_, y_, static_cast<Scalar>(vals)... }
			{
				static_assert(sizeof...(T) <= Dimensions - 2);
			}
		};

		template <typename Scalar>
		struct vector_base<Scalar, 1>
		{
			Scalar x;

			constexpr vector_base() noexcept = default;

			explicit constexpr vector_base(Scalar x_) noexcept
				: x{ x_ }
			{}
		};

		template <typename Scalar>
		struct vector_base<Scalar, 2>
		{
			Scalar x;
			Scalar y;

			constexpr vector_base() noexcept = default;

			explicit constexpr vector_base(Scalar fill) noexcept
				: x{ fill },
				y{ fill }
			{}

			constexpr vector_base(Scalar x_, Scalar y_) noexcept
				: x{ x_ },
				y{ y_ }
			{}
		};

		template <typename Scalar>
		struct vector_base<Scalar, 3>
		{
			Scalar x;
			Scalar y;
			Scalar z;

			constexpr vector_base() noexcept = default;

			explicit constexpr vector_base(Scalar fill) noexcept
				: x{ fill },
				y{ fill },
				z{ fill }
			{}

			constexpr vector_base(Scalar x_, Scalar y_, Scalar z_ = Scalar{}) noexcept
				: x{ x_ },
				y{ y_ },
				z{ z_ }
			{}
		};

		template <typename Scalar>
		struct vector_base<Scalar, 4>
		{
			Scalar x;
			Scalar y;
			Scalar z;
			Scalar w;

			constexpr vector_base() noexcept = default;

			explicit constexpr vector_base(Scalar fill) noexcept
				: x{ fill },
				y{ fill },
				z{ fill },
				w{ fill }
			{}

			constexpr vector_base(Scalar x_, Scalar y_, Scalar z_ = Scalar{}, Scalar w_ = Scalar{}) noexcept
				: x{ x_ },
				y{ y_ },
				z{ z_ },
				w{ w_ }
			{}
		};
	}
	MUU_IMPL_NAMESPACE_END
#else
	#define ENABLE_IF_AT_LEAST_DIMENSIONS(...)
	#define ENABLE_IF_AT_LEAST_DIMENSIONS_AND(...)
	#define	REQUIRES_AT_LEAST_DIMENSIONS(...)
#endif // !DOXYGEN

MUU_NAMESPACE_START
{
	namespace impl
	{
		template <size_t First = 0, size_t Count = constants<size_t>::highest, typename Vector, typename Func>
		MUU_ATTR(flatten)
		static constexpr void vector_for_each(Vector& vec, Func&& func) noexcept
		{
			static_assert(First < Vector::dimensions);
			static_assert(Count > 0);
			constexpr auto actual_count = (min)(Vector::dimensions - First, Count);
			constexpr auto last = (First + actual_count) - 1_sz;

			if constexpr (std::is_invocable_v<Func, decltype(vec[0]), size_t>)
			{
				if constexpr (Vector::dimensions > 4)
				{
					MUU_PRAGMA_MSVC(omp simd)
					for (size_t i = First; i <= last; i++)
						func(vec.values[i], i);
				}
				else
				{
					if constexpr (is_between(0_sz, First, last)) func(vec.template get<0>(), 0_sz);
					if constexpr (is_between(1_sz, First, last)) func(vec.template get<1>(), 1_sz);
					if constexpr (is_between(2_sz, First, last)) func(vec.template get<2>(), 2_sz);
					if constexpr (is_between(3_sz, First, last)) func(vec.template get<3>(), 3_sz);
				}
			}
			else
			{
				static_assert(std::is_invocable_v<Func, decltype(vec[0])>);

				if constexpr (Vector::dimensions > 4)
				{
					MUU_PRAGMA_MSVC(omp simd)
					for (size_t i = First; i <= last; i++)
						func(vec.values[i]);
				}
				else
				{
					if constexpr (is_between(0_sz, First, last)) func(vec.template get<0>());
					if constexpr (is_between(1_sz, First, last)) func(vec.template get<1>());
					if constexpr (is_between(2_sz, First, last)) func(vec.template get<2>());
					if constexpr (is_between(3_sz, First, last)) func(vec.template get<3>());
				}
			}
		}

		template <size_t First = 0, size_t Count = constants<size_t>::highest, typename Vector1, typename Vector2, typename Func>
		MUU_ATTR(flatten)
		static constexpr void vector_for_each(Vector1& vec1, Vector2& vec2, Func&& func) noexcept
		{
			static_assert(First < Vector1::dimensions);
			static_assert(First < Vector2::dimensions);
			static_assert(Count > 0);
			constexpr auto actual_count = (min)((min)(Vector1::dimensions, Vector2::dimensions) - First, Count);
			constexpr auto last = (First + actual_count) - 1_sz;

			if constexpr (std::is_invocable_v<Func, decltype(vec1[0]), decltype(vec2[0]), size_t>)
			{
				if constexpr (Vector1::dimensions > 4 && Vector2::dimensions > 4)
				{
					MUU_PRAGMA_MSVC(omp simd)
					for (size_t i = First; i <= last; i++)
						func(vec1.values[i], vec2.values[i], i);
				}
				else
				{
					if constexpr (is_between(0_sz, First, last)) func(vec1.template get<0>(), vec2.template get<0>(), 0_sz);
					if constexpr (is_between(1_sz, First, last)) func(vec1.template get<1>(), vec2.template get<1>(), 1_sz);
					if constexpr (is_between(2_sz, First, last)) func(vec1.template get<2>(), vec2.template get<2>(), 2_sz);
					if constexpr (is_between(3_sz, First, last)) func(vec1.template get<3>(), vec2.template get<3>(), 3_sz);
				}
			}
			else
			{
				static_assert(std::is_invocable_v<Func, decltype(vec1[0]), decltype(vec2[0])>);

				if constexpr (Vector1::dimensions > 4 && Vector2::dimensions > 4)
				{
					MUU_PRAGMA_MSVC(omp simd)
					for (size_t i = First; i <= last; i++)
						func(vec1.values[i], vec2.values[i]);
				}
				else
				{
					if constexpr (is_between(0_sz, First, last)) func(vec1.template get<0>(), vec2.template get<0>());
					if constexpr (is_between(1_sz, First, last)) func(vec1.template get<1>(), vec2.template get<1>());
					if constexpr (is_between(2_sz, First, last)) func(vec1.template get<2>(), vec2.template get<2>());
					if constexpr (is_between(3_sz, First, last)) func(vec1.template get<3>(), vec2.template get<3>());
				}
			}

		}
	}

	/// \brief An N-dimensional vector.
	///
	/// \tparam	Scalar      An arithmetic type.
	/// \tparam Dimensions  The number of dimensions.
	template <typename Scalar, size_t Dimensions>
	struct vector
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
			is_arithmetic<Scalar>,
			"Scalar type must be an arithmetic type"
		);

		/// \brief The type of each scalar component stored in this vector.
		using scalar_type = Scalar;

		/// \brief Alias for scalar_type.
		using value_type = Scalar;

		/// \brief The number of scalar components stored in this vector.
		static constexpr size_t dimensions = Dimensions;

		/// \brief Compile-time constants for this vector type.
		using constants = muu::constants<vector>;

		#ifdef DOXYGEN
		/// \brief The 0th scalar component stored in the vector.
		/// \attention This field only exists when the vector has &lt;= 4 dimensions.
		T x;
		/// \brief The 1st scalar component stored in the vector.
		/// \attention This field only exists when the vector has &lt;= 4 dimensions.
		T y;
		/// \brief The 2nd scalar component stored in the vector.
		/// \attention This field only exists when the vector has &lt;= 4 dimensions.
		T z;
		/// \brief The 3rd scalar component stored in the vector.
		/// \attention This field only exists when the vector has &lt;= 4 dimensions.
		T w;
		/// \brief The scalar components stored in the vector.
		/// \attention This field only exists when the vector has &gt; 4 dimensions.
		T values[dimensions];
		#endif

	private:

		template <typename T, size_t N>
		friend struct vector;
		using base_type = impl::vector_base<Scalar, dimensions>;
		static constexpr bool has_padding = sizeof(base_type) > (sizeof(scalar_type) * dimensions);
		static constexpr size_t padded_element_size = sizeof(base_type) / dimensions;

	public:

		#if 1 // element accessors

		/// \brief Gets a reference to the scalar component at a specific index.
		/// 
		/// \tparam Index  The index of the scalar component to retrieve, where X == 0, Y == 1, etc.
		///
		/// \return  A reference to the selected scalar component.
		template <size_t Index>
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		constexpr const scalar_type& get() const noexcept
		{
			static_assert(
				Index < dimensions,
				"Element index out of range"
			);
			
			if constexpr (dimensions <= 4)
			{
				if constexpr (Index == 0)		return base_type::x;
				else if constexpr (Index == 1)	return base_type::y;
				else if constexpr (Index == 2)	return base_type::z;
				else							return base_type::w;
			}
			else
				return base_type::values[Index];
		}

		/// \brief Gets a reference to the scalar component at a specific index.
		/// 
		/// \tparam Index  The index of the scalar component to retrieve, where X == 0, Y == 1, etc.
		///
		/// \return  A reference to the selected scalar component.
		template <size_t Index>
		[[nodiscard]]
		MUU_ALWAYS_INLINE
		constexpr scalar_type& get() noexcept
		{
			static_assert(
				Index < dimensions,
				"Element index out of range"
			);
			
			if constexpr (dimensions <= 4)
			{
				if constexpr (Index == 0)		return base_type::x;
				else if constexpr (Index == 1)	return base_type::y;
				else if constexpr (Index == 2)	return base_type::z;
				else							return base_type::w;
			}
			else
				return base_type::values[Index];
		}

		/// \brief Gets a reference to the Nth scalar component.
		///
		/// \param idx  The index of the scalar component to retrieve, where X == 0, Y == 1, etc.
		///
		/// \return  A reference to the selected scalar component.
		/// 
		/// \warning No bounds-checking is done!
		[[nodiscard]]
		constexpr const scalar_type& operator [](size_t idx) const noexcept
		{
			MUU_ASSUME(idx < dimensions);
			
			if constexpr (dimensions <= 4)
			{
				if (has_padding || is_constant_evaluated())
				{
					if (idx == 0)									return base_type::x;
					if constexpr (dimensions > 1) { if (idx == 1)	return base_type::y; }
					if constexpr (dimensions > 2) { if (idx == 2)	return base_type::z; }
					if constexpr (dimensions > 3) { if (idx == 3)	return base_type::w; }
					MUU_UNREACHABLE;
				}
				else
					return *(&get<0>() + idx);
			}
			else
				return base_type::values[idx];
		}

		/// \brief Gets a reference to the Nth scalar component.
		///
		/// \param idx  The index of the scalar component to retrieve, where X == 0, Y == 1, etc. 
		///
		/// \return  A reference to the selected scalar component.
		/// 
		/// \warning No bounds-checking is done!
		[[nodiscard]]
		constexpr scalar_type& operator [](size_t idx) noexcept
		{
			MUU_ASSUME(idx < dimensions);
			
			if constexpr (dimensions <= 4)
			{
				if (has_padding || is_constant_evaluated())
				{
					if (idx == 0)									return base_type::x;
					if constexpr (dimensions > 1) { if (idx == 1)	return base_type::y; }
					if constexpr (dimensions > 2) { if (idx == 2)	return base_type::z; }
					if constexpr (dimensions > 3) { if (idx == 3)	return base_type::w; }
					MUU_UNREACHABLE;
				}
				else
					return *(&get<0>() + idx);
			}
			else
				return base_type::values[idx];
		}

		#endif // scalar component accessors

		#if 1 // constructors

	private:

		template <size_t First = 0, size_t Count = dimensions - First>
		MUU_ATTR(flatten)
		static constexpr void fill_with_zero(vector& vec) noexcept
		{
			static_assert(First < dimensions);
			static_assert(Count > 0);
			static_assert(First + Count <= dimensions);

			if (is_constant_evaluated())
			{
				impl::vector_for_each<First, Count>(vec, [](scalar_type& el) noexcept
				{
					el = scalar_type{};
				});
			}
			else
			{
				if constexpr (First == 0 && Count == dimensions)
					memset(&vec, 0, sizeof(vector));
				else
					memset(&vec.template get<First>(), 0, padded_element_size * Count);
			}
		}

		template <size_t N, typename T>
		MUU_ATTR(flatten)
		static constexpr void fill_from_array(vector& vec, const T& arr) noexcept
		{
			constexpr bool can_memcpy = !has_padding
				&& std::is_same_v<remove_cvref<decltype(arr[0])>, scalar_type>;

			if constexpr (can_memcpy && build::supports_is_constant_evaluated)
			{
				if (is_constant_evaluated())
				{
					impl::vector_for_each<0, N>(vec, [&](scalar_type& el, size_t i) noexcept
					{
						el = arr[i];
					});
				}
				else
					memcpy(&vec, &arr, sizeof(scalar_type) * N);
			}
			else
			{
				impl::vector_for_each<0, N>(vec, [&](scalar_type& el, size_t i) noexcept
				{
					el = arr[i];
				});
			}

			if constexpr (N < dimensions)
				fill_with_zero<N>(vec);
		}

	public:

		/// \brief Default constructor. Scalar components are not initialized.
		constexpr vector() noexcept = default;

		/// \brief Copy constructor.
		constexpr vector(const vector&) noexcept = default;

		/// \brief	Constructs a vector with all scalar components set to the same value.
		///
		/// \param	fill	The value used to initialize each of the vector's scalar components.
		explicit constexpr vector(scalar_type fill) noexcept
			: base_type{ fill }
		{}

		/// \brief		Constructs a vector from two scalar values.
		/// \remarks	Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \param	x	Initial value for the vector's x scalar component.
		/// \param	y	Initial value for the vector's y scalar component.
		/// 
		/// \attention This constructor is only available when the vector has &gt;= 2 dimensions.
		REQUIRES_AT_LEAST_DIMENSIONS(2)
		constexpr vector(scalar_type x, scalar_type y) noexcept
			: base_type{ x, y }
		{}

		/// \brief		Constructs a vector from three scalar values.
		/// \remarks	Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \param	x	Initial value for the vector's x scalar component.
		/// \param	y	Initial value for the vector's y scalar component.
		/// \param	z	Initial value for the vector's z scalar component.
		/// 
		/// \attention This constructor is only available when the vector has &gt;= 3 dimensions.
		REQUIRES_AT_LEAST_DIMENSIONS(3)
		constexpr vector(scalar_type x, scalar_type y, scalar_type z) noexcept
			: base_type{ x, y, z }
		{}

		/// \brief		Constructs a vector from four scalar values.
		/// \remarks	Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \param	x		Initial value for the vector's x scalar component.
		/// \param	y		Initial value for the vector's y scalar component.
		/// \param	z		Initial value for the vector's z scalar component.
		/// \param	w		Initial value for the vector's w scalar component.
		/// \param	vals	Initial values for the vector's remaining scalar components.
		/// 
		/// \attention This constructor is only available when the vector has &gt;= 4 dimensions.
		REQUIRES_AT_LEAST_DIMENSIONS(4)
		constexpr vector(scalar_type x, scalar_type y, scalar_type z, scalar_type w) noexcept
			: base_type{ x, y, z, w }
		{}

		/// \brief		Constructs a vector from five or more scalar values.
		/// \remarks	Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \tparam T		Types compatible with scalar_type.
		/// \param	x		Initial value for the vector's x scalar component.
		/// \param	y		Initial value for the vector's y scalar component.
		/// \param	z		Initial value for the vector's z scalar component.
		/// \param	w		Initial value for the vector's w scalar component.
		/// \param	vals	Initial values for the vector's remaining scalar components.
		/// 
		/// \attention This constructor is only available when the vector has &gt;= 5 dimensions.
		template <typename... T ENABLE_IF_AT_LEAST_DIMENSIONS(5)>
		constexpr vector(scalar_type x, scalar_type y, scalar_type z, scalar_type w, T... vals) noexcept
			: base_type{ x, y, z, w, vals... }
		{}

		/// \brief Constructs a vector from a raw array.
		/// 
		/// \tparam N			The number of elements in the array.
		/// \param	vals		Array of values used to initialize the vector's scalar components.
		/// 
		/// \remarks	Any scalar components not covered by the constructor's parameters are initialized to zero.
		template <size_t N ENABLE_IF_AT_LEAST_DIMENSIONS(N)>
		explicit vector(const scalar_type(& vals)[N]) noexcept
		{
			fill_from_array<N>(*this, vals);
		}

		/// \brief Constructs a vector from a std::array.
		/// 
		/// \tparam N			The number of elements in the array.
		/// \param	vals		Array of values used to initialize the vector's scalar components.
		/// 
		/// \remarks	Any scalar components not covered by the constructor's parameters are initialized to zero.
		template <size_t N ENABLE_IF_AT_LEAST_DIMENSIONS(N)>
		explicit vector(const std::array<scalar_type, N>& vals) noexcept
		{
			fill_from_array<N>(*this, vals);
		}

		/// \brief Enlarging/truncating/converting constructor.
		/// 
		/// \remarks	Any scalar component not covered by the constructor's parameters are initialized to zero.
		template <typename T, size_t N>
		explicit vector(const vector<T, N>& vec) noexcept
		{
			constexpr auto can_memcpy = std::is_same_v<T, scalar_type> && has_padding == vector<T, N>::has_padding;

			if constexpr (can_memcpy && build::supports_is_constant_evaluated)
			{
				if (is_constant_evaluated())
				{
					impl::vector_for_each(*this, vec, [](scalar_type& lhs, auto& rhs) noexcept
					{
						lhs = static_cast<scalar_type>(rhs);
					});
				}
				else
					memcpy(this, &vec, (min)(sizeof(*this), sizeof(vector<T, N>)));
			}
			else
			{
				impl::vector_for_each(*this, vec, [](scalar_type& lhs, auto& rhs) noexcept
				{
					lhs = static_cast<scalar_type>(rhs);
				});
			}

			if constexpr (N < dimensions)
				fill_with_zero<N>(*this);
		}

		#endif // constructors


	};
	
	#ifndef DOXYGEN
	//template <typename T, typename U>
	//vector(Vector<T, 2>, U)->Vector<decltype(std::declval<T>() + std::declval<U>()), 3>;
	//template <typename T, typename U>
	//vector(Vector<T, 3>, U)->Vector<decltype(std::declval<T>() + std::declval<U>()), 4>;
	//template <typename T, size_t N>
	//Vector(const ArrayOf<T, N>&)->Vector<T, N>;
	//template <typename T, size_t N>
	//Vector(const std::array<T, N>&)->Vector<T, N>;
	//template <typename T, typename U>
	//Vector(const std::pair<T, U>&)->Vector<decltype(std::declval<T>() + std::declval<U>()), 2>;
	//template <typename T, typename U, typename... V, typename S = decltype(((T{} + U{}) + ... + V{})) >
	//Vector(const std::tuple<T, U, V...>&)->Vector<S, 2 + sizeof...(V)>;
	template <typename T, typename U, typename... V, typename = std::enable_if_t<all_arithmetic<T, U, V...>>>
	vector(T, U, V...) -> vector<decltype(((T{} + U{}) + ... + V{})), 2 + sizeof...(V)>;
	template <typename T, typename = std::enable_if_t<is_arithmetic<T>>>
	vector(T) -> vector<T, 1>;
	#endif // !DOXYGEN

	inline constexpr auto kek_test1 = vector{ 1 };
	inline constexpr auto kek_test2 = vector{ 1, 2.0f };
	inline constexpr auto kek_test3 = vector{ 1, 2.0f, 3.0 };
	inline constexpr auto kek_test4 = vector{ 1, 2.0f, 3.0, 4 };
	inline constexpr auto kek_test5 = vector{ 1, 2.0f, 3.0, 4, 5 };
}
MUU_NAMESPACE_END

#undef ENABLE_IF_AT_LEAST_DIMENSIONS
#undef ENABLE_IF_AT_LEAST_DIMENSIONS_AND
#undef REQUIRES_AT_LEAST_DIMENSIONS

#endif // vector class implementation



MUU_PRAGMA_MSVC(inline_recursion(off))
MUU_POP_WARNINGS // MUU_DISABLE_ARITHMETIC_WARNINGS, MUU_DISABLE_LIFETIME_WARNINGS, MUU_DISABLE_SHADOW_WARNINGS
