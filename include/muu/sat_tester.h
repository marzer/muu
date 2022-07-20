// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::sat_tester.

#include "vector.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(float_control(except, off))

namespace muu
{
	/// \brief A helper utility for performing Separating-Axis Theorem tests.
	/// \ingroup math
	///
	/// \tparam	Scalar      The vector scalar component type.
	/// \tparam Dimensions  The number of dimensions.
	///
	/// \see [Separating-Axis Theorem](https://en.wikipedia.org/wiki/Hyperplane_separation_theorem)
	template <typename Scalar, size_t Dimensions>
	struct MUU_TRIVIAL_ABI sat_tester
	{
		static_assert(!is_cvref<Scalar>, "sat_tester scalar type cannot be const, volatile, or a reference");
		static_assert(std::is_trivially_constructible_v<Scalar>	  //
						  && std::is_trivially_copyable_v<Scalar> //
						  && std::is_trivially_destructible_v<Scalar>,
					  "sat_tester scalar type must be trivially constructible, copyable and destructible");
		static_assert(is_signed<Scalar>, "sat_tester scalar type must be signed");
		static_assert(is_floating_point<Scalar>, "sat_tester scalar type must be a floating-point type");
		static_assert(Dimensions >= 2 && Dimensions <= 3, "sat_tester must be for 2 or 3 dimensions");

		/// \brief The sat_tester's scalar type.
		using scalar_type = Scalar;

		/// \brief The sat_tester's vector type.
		using vector_type = vector<scalar_type, Dimensions>;

		/// \brief The number of dimensions represented by this sat_tester's vector_type.
		static constexpr size_t dimensions = Dimensions;

		/// \brief The minimum projection seen so far.
		scalar_type min;

		/// \brief The maximum projection seen so far.
		scalar_type max;

		/// \brief Resets the SAT tester to the default-constructed state.
		MUU_ALWAYS_INLINE
		constexpr sat_tester& reset() noexcept
		{
			min = constants<scalar_type>::highest;
			max = constants<scalar_type>::lowest;
			return *this;
		}

		/// \brief Adds one or more points to the SAT test projection range.
		MUU_CONSTRAINED_TEMPLATE((sizeof...(T) == 0 || all_convertible_to<vector_type, const T&...>), typename... T)
		constexpr sat_tester& MUU_VECTORCALL add(MUU_VPARAM(vector_type) axis,
												 MUU_VPARAM(vector_type) point,
												 const T&... points) noexcept
		{
			const auto proj = vector_type::dot(axis, point);
			min				= muu::min(proj, min);
			max				= muu::max(proj, max);
			if constexpr (sizeof...(T) > 0)
				return add(axis, points...);
			else
				return *this;
		}

		/// \brief Adds one or more points to the SAT test projection range.
		MUU_CONSTRAINED_TEMPLATE((sizeof...(T) == 0 || all_convertible_to<vector_type, const T&...>),
								 size_t Dimension,
								 typename... T)
		constexpr sat_tester& MUU_VECTORCALL add(index_tag<Dimension> /*axis*/,
												 MUU_VPARAM(vector_type) point,
												 const T&... points) noexcept
		{
			static_assert(Dimension < Dimensions, "Dimension index out of range");

			const auto proj = point.template get<Dimension>() * scalar_type{ 1 };
			min				= muu::min(proj, min);
			max				= muu::max(proj, max);
			if constexpr (sizeof...(T) > 0)
				return add(index_tag<Dimension>{}, points...);
			else
				return *this;
		}

		/// \brief Adds an array of points to the SAT test projection range.
		template <size_t N>
		constexpr sat_tester& MUU_VECTORCALL add(MUU_VPARAM(vector_type) axis, const vector_type (&points)[N]) noexcept
		{
			for (const auto& p : points)
				add(axis, p);
			return *this;
		}

		/// \brief Adds a range of points to the SAT test projection range.
		constexpr sat_tester& MUU_VECTORCALL add(MUU_VPARAM(vector_type) axis,
												 const vector_type* begin,
												 const vector_type* end) noexcept
		{
			if (begin == end)
				return *this;

			MUU_ASSUME(begin != nullptr);
			MUU_ASSUME(end != nullptr);
			MUU_ASSUME(begin < end);

			for (; begin != end; begin++)
				add(axis, *begin);
			return *this;
		}

		/// \brief Adds a range of points to the SAT test projection range.
		template <size_t Dimension>
		constexpr sat_tester& MUU_VECTORCALL add(index_tag<Dimension> /*axis*/,
												 const vector_type* begin,
												 const vector_type* end) noexcept
		{
			static_assert(Dimension < Dimensions, "Dimension index out of range");

			if (begin == end)
				return *this;

			MUU_ASSUME(begin != nullptr);
			MUU_ASSUME(end != nullptr);
			MUU_ASSUME(begin < end);

			for (; begin != end; begin++)
				add(index_tag<Dimension>{}, *begin);
			return *this;
		}

		/// \brief Default constructor.
		MUU_NODISCARD_CTOR
		constexpr sat_tester() noexcept //
			: min{ constants<scalar_type>::highest },
			  max{ constants<scalar_type>::lowest }
		{}

		/// \brief Initializes the SAT test projection range bounds to a specific value.
		MUU_NODISCARD_CTOR
		explicit constexpr sat_tester(scalar_type min_max) noexcept //
			: min{ min_max },
			  max{ min_max }
		{}

		/// \brief Initializes the SAT test projection range directly from one or more points.
		MUU_CONSTRAINED_TEMPLATE((sizeof...(T) == 0 || all_convertible_to<vector_type, const T&...>), typename... T)
		MUU_NODISCARD_CTOR
		constexpr sat_tester(MUU_VPARAM(vector_type) axis,
							 MUU_VPARAM(vector_type) point,
							 const T&... points) noexcept //
			: sat_tester{ vector_type::dot(axis, point) }
		{
			if constexpr (sizeof...(T) > 0)
				add(axis, points...);
		}

		/// \brief Initializes the SAT test projection range directly from one or more points.
		MUU_CONSTRAINED_TEMPLATE((sizeof...(T) == 0 || all_convertible_to<vector_type, const T&...>),
								 size_t Dimension,
								 typename... T)
		MUU_NODISCARD_CTOR
		constexpr sat_tester(index_tag<Dimension> /*axis*/,
							 MUU_VPARAM(vector_type) point,
							 const T&... points) noexcept //
			: sat_tester{ point.template get<Dimension>() * scalar_type{ 1 } }
		{
			if constexpr (sizeof...(T) > 0)
				add(index_tag<Dimension>{}, points...);
		}

		/// \brief Initializes the SAT test projection range directly from an array of points.
		template <size_t N>
		MUU_NODISCARD_CTOR
		constexpr sat_tester(MUU_VPARAM(vector_type) axis, const vector_type (&points)[N]) noexcept //
			: sat_tester{ axis, points[0] }
		{
			static_assert(N >= 1);

			if constexpr (N > 1)
				add(axis, points + 1, points + N);
		}

		/// \brief Initializes the SAT test projection range directly from an array of points.
		template <size_t Dimension, size_t N>
		MUU_NODISCARD_CTOR
		constexpr sat_tester(index_tag<Dimension> /*axis*/, const vector_type (&points)[N]) noexcept //
			: sat_tester{ index_tag<Dimension>{}, points[0] }
		{
			static_assert(N >= 1);

			if constexpr (N > 1)
				add(index_tag<Dimension>{}, points + 1, points + N);
		}

		/// \brief Returns true if the SAT test projection range seen so far contains the given value.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL operator()(scalar_type val) noexcept
		{
			return min <= val && max >= val;
		}

		/// \brief Returns true if the SAT test projection range seen so far overlaps the given range.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL operator()(scalar_type min_val, scalar_type max_val) noexcept
		{
			return min <= max_val && max >= min_val;
		}

		/// \brief Returns true if the SAT test projection range seen so far overlaps that of another sat_tester.
		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL operator()(const sat_tester& other) noexcept
		{
			return min <= other.max && max >= other.min;
		}
	};
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
