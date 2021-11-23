// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::packed_unit_vector.

#include "vector.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(float_control(except, off))

//======================================================================================================================
// IMPLEMENTATION DETAILS
//======================================================================================================================
/// \cond

namespace muu::impl
{
	template <typename Integer, size_t Dimensions>
	struct packed_unit_vector_traits
	{
		using integer_type = Integer;
		static_assert(is_integer<integer_type> && is_unsigned<integer_type>);

		using arith_type = impl::highest_ranked<integer_type, unsigned>;
		static_assert(is_integer<arith_type> && is_unsigned<arith_type>);

		static constexpr arith_type total_bits	= static_cast<arith_type>(sizeof(integer_type) * CHAR_BIT);
		static constexpr arith_type sign_bits	= static_cast<arith_type>(Dimensions);
		static constexpr arith_type value_bits	= static_cast<arith_type>(total_bits - sign_bits);
		static constexpr arith_type stored_dims = static_cast<arith_type>(Dimensions - 1u);
		static constexpr arith_type x_bits		= evenly_distribute(value_bits, stored_dims, arith_type{});
		static constexpr arith_type y_bits		= Dimensions >= 3 //
													? evenly_distribute(value_bits, stored_dims, arith_type{ 1 })
													: arith_type{};

		static_assert(x_bits >= y_bits);
		static_assert(x_bits + y_bits == value_bits);
		static_assert(x_bits + y_bits + sign_bits == total_bits);

		static constexpr arith_type x_range = (arith_type{ 1 } << x_bits) - arith_type{ 1 };
		static constexpr arith_type y_range = (arith_type{ 1 } << y_bits) - arith_type{ 1 };

		static constexpr arith_type sign_mask = ((arith_type{ 1 } << sign_bits) - arith_type{ 1 }) << (x_bits + y_bits);
		static constexpr arith_type x_mask	  = x_range << y_bits;
		static constexpr arith_type y_mask	  = y_range;

		static_assert((sign_mask ^ x_mask ^ y_mask) == static_cast<integer_type>(-1));
		static_assert((sign_mask | x_mask | y_mask) == static_cast<integer_type>(-1));

		static constexpr arith_type x_sign_bit = arith_type{ 1 } << (total_bits - arith_type{ 1 });
		static constexpr arith_type y_sign_bit = arith_type{ 1 } << (total_bits - arith_type{ 2 });
		static constexpr arith_type z_sign_bit = arith_type{ 1 } << (total_bits - arith_type{ 3 });

		template <typename T>
		MUU_PURE_GETTER
		static constexpr arith_type MUU_VECTORCALL pack_vector(vector<T, Dimensions> v) noexcept
		{
			if constexpr (any_integer<T> || impl::any_small_float_<T>)
			{
				using cast_type = vector<promote_if_small_float<impl::std_math_common_type<T>>, Dimensions>;
				return pack_vector(cast_type{ v });
			}
			else
			{
				static_assert(!is_integer<T> && !impl::is_small_float_<T>);

				arith_type b = {};

				// First handle sign bits.
				// Sign bit will be 1 if the component is < 0. Also make the components positive.
				b |= v.x >= T{} ? arith_type{} : (v.x = -v.x, x_sign_bit);
				b |= v.y >= T{} ? arith_type{} : (v.y = -v.y, y_sign_bit);
				if constexpr (Dimensions >= 3)
					b |= v.z >= T{} ? arith_type{} : (v.z = -v.z, z_sign_bit);

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
				MUU_ASSUME(ix <= x_range);

				// 3d path does the fancy inversion trick to get an extra bit of precision
				if constexpr (Dimensions >= 3)
				{
					arith_type iy = static_cast<arith_type>(v.y * static_cast<T>(x_range - arith_type{ 1 }) + T{ 0.5 });
					MUU_ASSUME(iy <= x_range);

					constexpr auto iy_half_threshold = (arith_type{ 1 } << x_bits) / arith_type{ 2 };
					if (iy >= iy_half_threshold)
					{
						ix = x_range - ix;
						iy = x_range - iy;
					}

					MUU_ASSUME(ix <= x_range);
					MUU_ASSUME(iy <= y_range);

					b |= (ix << y_bits) | iy;
				}

				// 2d path can just use all the value bits for X directly
				else
					b |= ix;

				MUU_ASSUME(b <= muu::constants<integer_type>::highest);

				return b;
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

				constexpr auto x_range_div = T{ 1 } / static_cast<T>(x_range);

				if constexpr (Dimensions >= 3)
				{
					arith_type ix = (b & x_mask) >> y_bits;
					arith_type iy = b & y_mask;

					if (ix + iy > x_range)
					{
						ix = x_range - ix;
						iy = x_range - iy;
					}

					const auto x = static_cast<T>(ix) * x_range_div;
					const auto y = static_cast<T>(iy) * x_range_div;

					return out_type::normalize(
						out_type{ static_cast<T>(1 - (static_cast<int>(!!(b & x_sign_bit)) * 2)) * x,
								  static_cast<T>(1 - (static_cast<int>(!!(b & y_sign_bit)) * 2)) * y,
								  static_cast<T>(1 - (static_cast<int>(!!(b & z_sign_bit)) * 2)) * (T{ 1 } - x - y) });
				}
				else
				{
					const auto x = static_cast<T>(b & x_range) * x_range_div;
					return out_type::normalize(
						out_type{ static_cast<T>(1 - (static_cast<int>(!!(b & x_sign_bit)) * 2)) * x,
								  static_cast<T>(1 - (static_cast<int>(!!(b & y_sign_bit)) * 2)) * (T{ 1 } - x) });
				}
			}
		}
	};
}

/// \endcond

//======================================================================================================================
// PACKED UNIT VECTOR
//======================================================================================================================

namespace muu
{
	/// \brief A utility type for lossy packing of 2D or 3D unit vectors into a single integer.
	///
	/// \detail \cpp
	/// muu::vector v{ 2.0f, 3.0f, 4.0f };
	/// v.normalize();
	/// muu::packed_unit_vector pv{ v }; // deduced as 3D with uint32_t storage
	/// muu::packed_unit_vector<uint16_t, 3> pv2{ v };
	///
	/// std::cout << v << " (" << sizeof(v) << " bytes)\n";
	/// std::cout << pv << " (" << sizeof(pv) << " bytes)\n";
	/// std::cout << pv2 << " (" << sizeof(pv2) << " bytes)";
	/// \ecpp
	/// \out
	/// { 0.371391, 0.557086, 0.742781 } (12 bytes)
	/// { 0.371357, 0.557061, 0.742817 } (4 bytes)
	/// { 0.367753, 0.55163, 0.74864 } (2 bytes)
	/// \eout
	///
	/// \see muu::vector
	template <typename Integer, size_t Dimensions>
	struct MUU_TRIVIAL_ABI packed_unit_vector
	{
		static_assert(!is_cvref<Integer>, "Packed unit vector integer type cannot be const, volatile, or a reference");
		static_assert(is_integer<Integer> && is_unsigned<Integer>,
					  "Packed unit vector backing type must be an unsigned integer");
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

		/// \brief The integer used as the backing storage for this unit vector.
		integer_type bits;

	  private:
		/// \cond

		using traits	 = impl::packed_unit_vector_traits<integer_type, Dimensions>;
		using arith_type = typename traits::arith_type;

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
		///
		/// \tparam T	Source vector scalar type.
		/// \param vec	Source vector. Should be normalized already.
		template <typename T>
		MUU_NODISCARD_CTOR
		explicit constexpr packed_unit_vector(const vector<T, dimensions>& vec) noexcept
			: bits{ static_cast<integer_type>(traits::pack_vector(vec)) }
		{
			MUU_CONSTEXPR_SAFE_ASSERT(vec.normalized());
		}

		/// \brief Unpacks the packed unit vector into a regular multi-component floating-point vector.
		MUU_CONSTRAINED_TEMPLATE(is_floating_point<T>, typename T)
		MUU_PURE_INLINE_GETTER
		constexpr vector<T, dimensions> unpack() const noexcept
		{
			return traits::template unpack_vector<T>(static_cast<arith_type>(bits));
		}

		/// \brief Unpacks the packed unit vector into a regular multi-component floating-point vector.
		MUU_CONSTRAINED_TEMPLATE(is_floating_point<T>, typename T)
		MUU_PURE_INLINE_GETTER
		explicit constexpr operator vector<T, dimensions>() const noexcept
		{
			return unpack<T>();
		}

		/// \brief Checks if the x-axis component is negative (without needing to unpack the vector).
		MUU_PURE_INLINE_GETTER
		constexpr bool x_negative() const noexcept
		{
			return !!(static_cast<arith_type>(bits) & traits::x_sign_bit);
		}

		/// \brief Checks if the y-axis component is negative (without needing to unpack the vector).
		MUU_PURE_INLINE_GETTER
		constexpr bool y_negative() const noexcept
		{
			return !!(static_cast<arith_type>(bits) & traits::y_sign_bit);
		}

		/// \brief Checks if the z-axis component is negative (without needing to unpack the vector).
		MUU_LEGACY_REQUIRES(Dims >= 3, size_t Dims = Dimensions)
		MUU_PURE_INLINE_GETTER
		constexpr bool z_negative() const noexcept
		{
			return !!(static_cast<arith_type>(bits) & traits::z_sign_bit);
		}

		/// \brief Writes a packed unit vector out to a text stream.
		template <typename Char, typename Traits>
		friend std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os,
															const packed_unit_vector& v)
		{
			return os << v.unpack<float>();
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
