// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::packed_unit_vector.

#include "vector.h"
#include "bit.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(float_control(except, off))

//======================================================================================================================
// PACKED UNIT VECTOR
//======================================================================================================================

namespace muu
{
	/// \brief A utility type for lossy packing of 2D or 3D unit vectors into a single integer.
	template <typename Integer, size_t Dimensions>
	struct MUU_TRIVIAL_ABI packed_unit_vector
	{
		static_assert(!is_cvref<Integer>, "Packed unit vector integer type cannot be const, volatile, or a reference");
		static_assert(is_integer<Integer> && is_unsigned<Integer>,
					  "Packed unit vector backing type must be unsigned integer");
		static_assert(std::is_trivially_constructible_v<Integer>   //
						  && std::is_trivially_copyable_v<Integer> //
						  && std::is_trivially_destructible_v<Integer>,
					  "Packed unit vector integer type must be trivially constructible, copyable and destructible");
		static_assert(sizeof(Integer) * build::bits_per_byte >= 8_sz,
					  "Packed unit vector integer type must have at least 8 bits");
		static_assert(Dimensions == 2 || Dimensions == 3, "Packed unit vectors may only have two or three dimensions");

		/// \brief The number of scalar components stored in this vector.
		static constexpr size_t dimensions = Dimensions;

		/// \brief Compile-time constants for this vector type.
		using constants = muu::constants<packed_unit_vector>;

		/// \brief The integer type used as the backing storage for this unit vector.
		using integer_type = Integer;

		/// \brief The integer used to store the packed unit vector data.
		integer_type bits;

	  private:
		/// \cond

		using arith_type						= impl::highest_ranked<integer_type, uint32_t>;
		static constexpr arith_type total_bits	= static_cast<arith_type>(sizeof(Integer) * build::bits_per_byte);
		static constexpr arith_type sign_bits	= static_cast<arith_type>(Dimensions);
		static constexpr arith_type value_bits	= static_cast<arith_type>(total_bits - sign_bits);
		static constexpr arith_type stored_dims = static_cast<arith_type>(Dimensions - 1u);
		static constexpr arith_type x_bits		= evenly_distribute(value_bits, stored_dims, arith_type{});
		static constexpr arith_type y_bits		= Dimensions > 2u
													? evenly_distribute(value_bits, stored_dims, arith_type{ 1 })
													: arith_type{};
		static_assert(x_bits + y_bits == value_bits);

		static constexpr arith_type x_range = bit_fill_right<arith_type>(x_bits);
		static constexpr arith_type y_range = bit_fill_right<arith_type>(y_bits);

		template <typename T>
		MUU_PURE_GETTER
		static constexpr arith_type MUU_VECTORCALL pack_vector(vector<T, dimensions> v) noexcept
		{
			if constexpr (any_integer<T> || impl::any_small_float_<T>)
			{
				using cast_type = vector<promote_if_small_float<impl::std_math_common_type<T>>, dimensions>;
				return pack_vector(cast_type{ v });
			}
			else
			{
				static_assert(!is_integer<T> && !impl::is_small_float_<T>);

				arith_type b = {};

				// First handle sign bits.
				// Sign bit will be 1 if the component is >= 0. Also make the components positive.
				b |= v.x >= T{} ? (arith_type{ 1 } << (total_bits - arith_type{ 1 })) : (v.x = -v.x, arith_type{});
				b |= v.y >= T{} ? (arith_type{ 1 } << (total_bits - arith_type{ 2 })) : (v.y = -v.y, arith_type{});
				if constexpr (Dimensions >= 3)
					b |= v.z >= T{} ? (arith_type{ 1 } << (total_bits - arith_type{ 3 })) : (v.z = -v.z, arith_type{});

				// Project the vector to the line/plane that passes through standard basis vectors
				if constexpr (Dimensions >= 3)
				{
					const T proj = T{ 1 } / (v.x + v.y + v.z);
					v.x *= proj;
					v.y *= proj;
				}
				else
				{
					v.x /= (v.x + v.y);
				}

				arith_type ix = static_cast<arith_type>(v.x * static_cast<T>(x_range - arith_type{ 1 }) + T{ 0.5 });
				MUU_CONSTEXPR_SAFE_ASSERT(ix <= x_range);

				// 3d path does the fancy inversion trick to get an extra bit of precision
				if constexpr (Dimensions >= 3)
				{
					arith_type iy = static_cast<arith_type>(v.y * static_cast<T>(x_range - arith_type{ 1 }) + T{ 0.5 });
					MUU_CONSTEXPR_SAFE_ASSERT(iy <= x_range);

					if (iy >= (arith_type{ 1 } << x_bits) / arith_type{ 2 })
					{
						ix = x_range - ix;
						iy = x_range - iy;
					}

					MUU_CONSTEXPR_SAFE_ASSERT(ix <= x_range);
					MUU_CONSTEXPR_SAFE_ASSERT(iy <= y_range);

					b |= (ix << y_bits) | iy;
				}

				// 2d path can just use all the value bits for X directly
				else
					b |= ix;

				MUU_CONSTEXPR_SAFE_ASSERT(b <= muu::constants<integer_type>::highest);

				return b;
			}
		}

		template <typename T>
		MUU_PURE_INLINE_GETTER
		static constexpr vector<T, Dimensions> MUU_VECTORCALL unpack_vector_raw(arith_type b) noexcept
		{
			static_assert(!is_integer<T> && !impl::is_small_float_<T>);

			if constexpr (Dimensions >= 3)
			{
				return { !!(b & (arith_type{ 1 } << (total_bits - arith_type{ 1 }))) ? T{ 1 } : T{ -1 },
						 !!(b & (arith_type{ 1 } << (total_bits - arith_type{ 2 }))) ? T{ 1 } : T{ -1 },
						 !!(b & (arith_type{ 1 } << (total_bits - arith_type{ 3 }))) ? T{ 1 } : T{ -1 } };
			}
			else
			{
				return { !!(b & (arith_type{ 1 } << (total_bits - arith_type{ 1 }))) ? T{ 1 } : T{ -1 },
						 !!(b & (arith_type{ 1 } << (total_bits - arith_type{ 2 }))) ? T{ 1 } : T{ -1 } };
			}
		}

		template <typename T>
		MUU_PURE_GETTER
		static constexpr vector<T, Dimensions> MUU_VECTORCALL unpack_vector(arith_type b) noexcept
		{
			using out_type = vector<T, Dimensions>;

			if constexpr (is_integer<T> || impl::is_small_float_<T>)
			{
				using cast_type = promote_if_small_float<impl::std_math_common_type<T>>;
				return out_type{ unpack_vector<cast_type>(b) };
			}
			else
			{
				static_assert(is_floating_point<T>);

				out_type v = unpack_vector_raw<T>(b);

				if constexpr (Dimensions >= 3)
				{
					arith_type ix = (b & (x_range << y_bits)) >> y_bits;
					arith_type iy = b & y_range;

					if (ix + iy > x_range)
					{
						ix = x_range - ix;
						iy = x_range - iy;
					}

					v.x *= static_cast<T>(ix) / static_cast<T>(x_range);
					v.y *= static_cast<T>(iy) / static_cast<T>(x_range);
					v.z *= T{ 1 } - muu::abs(v.x) - muu::abs(v.y);
				}
				else
				{
					v.x *= static_cast<T>(b & x_range) / static_cast<T>(x_range);
					v.y *= T{ 1 } - muu::abs(v.x);
				}

				v.normalize();

				return v;
			}
		}

		/// \endcond
	  public:
		/// \brief Default constructor. Storage bits are not initialized.
		MUU_NODISCARD_CTOR
		packed_unit_vector() noexcept = default;

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr packed_unit_vector(const packed_unit_vector&) noexcept = default;

		/// \brief Copy-assigment operator.
		constexpr packed_unit_vector& operator=(const packed_unit_vector&) noexcept = default;

		/// \brief Constructs from a regular unit vector.
		template <typename T>
		MUU_NODISCARD_CTOR
		explicit constexpr packed_unit_vector(const vector<T, dimensions>& vec) noexcept
			: bits{ static_cast<integer_type>(pack_vector(vec)) }
		{
			MUU_CONSTEXPR_SAFE_ASSERT(vec.normalized());
		}

		/// \brief Unpacks the packed unit vector into a regular multi-component floating-point vector.
		MUU_CONSTRAINED_TEMPLATE(is_floating_point<T>, typename T)
		MUU_PURE_INLINE_GETTER
		constexpr operator vector<T, dimensions>() const noexcept
		{
			return unpack_vector<T>(static_cast<arith_type>(bits));
		}
	};

	/// \cond

	template <typename S, size_t D>
	packed_unit_vector(vector<S, D>) -> packed_unit_vector<uint32_t, D>;

	/// \endcond
}

//======================================================================================================================
// PACKED UNIT VECTOR CONSTANTS
//======================================================================================================================

namespace muu
{
	/// \cond
	namespace impl
	{
		template <typename Integer, size_t Dimensions>
		struct packed_unit_vector_constants_2d
		{};

		template <typename Integer>
		struct packed_unit_vector_constants_2d<Integer, 2>
		{
			using type	  = packed_unit_vector<Integer, 2>;
			using vectors = typename vector<float, 2>::constants;

			static constexpr auto screen_right = type{ vectors::screen_right };
			static constexpr auto screen_down  = type{ vectors::screen_down };
			static constexpr auto screen_left  = type{ vectors::screen_left };
			static constexpr auto screen_up	   = type{ vectors::screen_up };
		};

		template <typename Integer, size_t Dimensions>
		struct packed_unit_vector_constants_3d
		{};

		template <typename Integer>
		struct packed_unit_vector_constants_3d<Integer, 3>
		{
			using type	  = packed_unit_vector<Integer, 3>;
			using vectors = typename vector<float, 3>::constants;

			static constexpr auto z_axis   = type{ vectors::z_axis };
			static constexpr auto backward = type{ vectors::backward };
			static constexpr auto forward  = type{ vectors::forward };
		};
	}
	/// \endcond

	/// \ingroup	constants
	/// \see		muu::packed_unit_vector
	///
	/// \brief		Packed unit vector constants.
	template <typename Integer, size_t Dimensions>
	struct MUU_EMPTY_BASES constants<packed_unit_vector<Integer, Dimensions>> //
		MUU_HIDDEN_BASE(
			impl::packed_unit_vector_constants_2d<Integer, Dimensions>,
			impl::packed_unit_vector_constants_3d<Integer, Dimensions>)
	{
		using type	  = packed_unit_vector<Integer, Dimensions>;
		using vectors = typename vector<float, Dimensions>::constants;

		/// \name Axes
		/// @{

		/// \brief	The X axis.
		static constexpr packed_unit_vector<Integer, Dimensions> x_axis{ vectors::x_axis };

		/// \brief	The Y axis.
		static constexpr packed_unit_vector<Integer, Dimensions> y_axis{ vectors::y_axis };

#ifdef DOXYGEN

		/// \brief	The Z axis.
		static constexpr packed_unit_vector<Integer, Dimensions> z_axis;

#endif

		/// @}

		/// \name Directions (screen space)
		/// @{

#ifdef DOXYGEN

		/// \brief	Right direction (in a top-down screen coordinate system).
		static constexpr packed_unit_vector<Integer, Dimensions> screen_right;

		/// \brief	Down direction (in a top-down screen coordinate system).
		static constexpr packed_unit_vector<Integer, Dimensions> screen_down;

		/// \brief	Left direction (in a top-down screen coordinate system).
		static constexpr packed_unit_vector<Integer, Dimensions> screen_left;

		/// \brief	Up direction (in a top-down screen coordinate system).
		static constexpr packed_unit_vector<Integer, Dimensions> screen_up;

#endif

		/// @}

		/// \name Directions (world space)
		/// @{

#ifdef DOXYGEN

		/// \brief	Backward direction (in a right-handed coordinate system).
		static constexpr packed_unit_vector<Integer, Dimensions> backward;

		/// \brief	Forward direction (in a right-handed coordinate system).
		static constexpr packed_unit_vector<Integer, Dimensions> forward;

#endif

		/// \brief	Right direction (in a right-handed coordinate system).
		static constexpr packed_unit_vector<Integer, Dimensions> right{ vectors::right };

		/// \brief	Up direction (in a right-handed coordinate system).
		static constexpr packed_unit_vector<Integer, Dimensions> up{ vectors::up };

		/// \brief	Left direction (in a right-handed coordinate system).
		static constexpr packed_unit_vector<Integer, Dimensions> left{ vectors::left };

		/// \brief	Down direction (in a right-handed coordinate system).
		static constexpr packed_unit_vector<Integer, Dimensions> down{ vectors::down };

		/// @}
	};
}

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"
