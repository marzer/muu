// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief  Contains the definition of muu::matrix.

#include "vector.h"
#include "quaternion.h"
#include "impl/matrix_base.h"
#include "impl/header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_DISABLE_SHADOW_WARNINGS;
MUU_DISABLE_ARITHMETIC_WARNINGS;
MUU_PRAGMA_MSVC(float_control(except, off))

//======================================================================================================================
// IMPLEMENTATION DETAILS
//======================================================================================================================

namespace muu
{
	/// \brief Flags relating to math APIs w.r.t some graphics + rendering environments (e.g. DirectX).
	/// \ingroup math
	enum class math_apis
	{
		/// \brief DirectX (default)
		directx = 0,

		/// \brief OpenGL
		opengl = 1,

		/// \brief Right-handed coordinate system (default)
		right_handed = 0,

		/// \brief Left-handed coordinate system
		left_handed = 2
	};
	MUU_MAKE_FLAGS(math_apis);
}

/// \cond

namespace muu::impl
{
	//--- x + y column getters -----------------------------------------------------------------------------------------

	template <typename Derived, bool = is_matrix_<Derived, 1, 2>>
	struct matrix_get_xy_column
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct matrix_get_xy_column<matrix<Scalar, Rows, Columns>, true>
	{
		using column_type = vector<Scalar, Rows>;

		MUU_PURE_INLINE_GETTER
		constexpr column_type& x_column() noexcept
		{
			return static_cast<matrix<Scalar, Rows, Columns>&>(*this).template column<0>();
		}

		MUU_PURE_INLINE_GETTER
		constexpr const column_type& x_column() const noexcept
		{
			return static_cast<const matrix<Scalar, Rows, Columns>&>(*this).template column<0>();
		}

		MUU_PURE_INLINE_GETTER
		constexpr column_type& y_column() noexcept
		{
			return static_cast<matrix<Scalar, Rows, Columns>&>(*this).template column<1>();
		}

		MUU_PURE_INLINE_GETTER
		constexpr const column_type& y_column() const noexcept
		{
			return static_cast<const matrix<Scalar, Rows, Columns>&>(*this).template column<1>();
		}
	};

	//--- z column getter ----------------------------------------------------------------------------------------------

	template <typename Derived, bool = is_matrix_<Derived, 1, 3>>
	struct matrix_get_z_column
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct matrix_get_z_column<matrix<Scalar, Rows, Columns>, true>
	{
		using column_type = vector<Scalar, Rows>;

		MUU_PURE_INLINE_GETTER
		constexpr column_type& z_column() noexcept
		{
			return static_cast<matrix<Scalar, Rows, Columns>&>(*this).template column<2>();
		}

		MUU_PURE_INLINE_GETTER
		constexpr const column_type& z_column() const noexcept
		{
			return static_cast<const matrix<Scalar, Rows, Columns>&>(*this).template column<2>();
		}
	};

	//--- w column getter ----------------------------------------------------------------------------------------------

	template <typename Derived, bool = is_matrix_<Derived, 1, 4>>
	struct matrix_get_w_column
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct matrix_get_w_column<matrix<Scalar, Rows, Columns>, true>
	{
		using column_type = vector<Scalar, Rows>;

		MUU_PURE_INLINE_GETTER
		constexpr column_type& w_column() noexcept
		{
			return static_cast<matrix<Scalar, Rows, Columns>&>(*this).template column<3>();
		}

		MUU_PURE_INLINE_GETTER
		constexpr const column_type& w_column() const noexcept
		{
			return static_cast<const matrix<Scalar, Rows, Columns>&>(*this).template column<3>();
		}
	};

	//--- translation column getter ------------------------------------------------------------------------------------

	template <typename Derived, bool = is_translation_matrix_<Derived>>
	struct matrix_get_translation_column
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct matrix_get_translation_column<matrix<Scalar, Rows, Columns>, true>
	{
		using column_type = vector<Scalar, Rows>;

		MUU_PURE_INLINE_GETTER
		constexpr column_type& translation_column() noexcept
		{
			return static_cast<matrix<Scalar, Rows, Columns>&>(*this).template column<Columns - 1u>();
		}

		MUU_PURE_INLINE_GETTER
		constexpr const column_type& translation_column() const noexcept
		{
			return static_cast<const matrix<Scalar, Rows, Columns>&>(*this).template column<Columns - 1u>();
		}
	};

	//--- from_2d_scale() -----------------------------------------------------------------------------------------

	template <typename Derived, bool = is_scale_matrix_<Derived>>
	struct matrix_from_2d_scale
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct matrix_from_2d_scale<matrix<Scalar, Rows, Columns>, true>
	{
		MUU_PURE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_2d_scale(Scalar x, Scalar y) noexcept
		{
			using out_type = matrix<Scalar, Rows, Columns>;

			auto out				 = out_type::constants::identity;
			out.template get<0, 0>() = x;
			out.template get<1, 1>() = y;
			return out;
		}

		template <size_t D>
		MUU_PURE_INLINE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_2d_scale(
			const vector<Scalar, D>& scale) noexcept
		{
			if constexpr (D == 1)
				return from_2d_scale(scale.x, Scalar{ 1 });
			else
				return from_2d_scale(scale.template get<0>(), scale.template get<1>());
		}
	};

	//--- from_3d_scale() -----------------------------------------------------------------------------------------

	template <typename Derived, bool = is_3d_scale_matrix_<Derived>>
	struct matrix_from_3d_scale
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct matrix_from_3d_scale<matrix<Scalar, Rows, Columns>, true>
	{
		MUU_PURE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_3d_scale(Scalar x,
																					Scalar y,
																					Scalar z = Scalar{ 1 }) noexcept
		{
			using out_type = matrix<Scalar, Rows, Columns>;

			auto out				 = out_type::constants::identity;
			out.template get<0, 0>() = x;
			out.template get<1, 1>() = y;
			out.template get<2, 2>() = z;
			return out;
		}

		template <size_t D>
		MUU_PURE_INLINE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_3d_scale(
			const vector<Scalar, D>& scale) noexcept
		{
			if constexpr (D == 1)
				return from_3d_scale(scale.x, Scalar{ 1 });
			else if constexpr (D == 2)
				return from_3d_scale(scale.x, scale.y);
			else
				return from_3d_scale(scale.template get<0>(), scale.template get<1>(), scale.template get<2>());
		}
	};

	//--- from_scale() -----------------------------------------------------------------------------------------

	template <typename Derived, bool = is_scale_matrix_<Derived>>
	struct matrix_from_scale
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct matrix_from_scale<matrix<Scalar, Rows, Columns>, true>
	{
		MUU_PURE_INLINE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_scale(Scalar x, Scalar y) noexcept
		{
			return matrix<Scalar, Rows, Columns>::from_2d_scale(x, y);
		}

		MUU_PURE_INLINE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_scale(MUU_VPARAM(vector<Scalar, 2>)
																					 scale) noexcept
		{
			return matrix<Scalar, Rows, Columns>::from_2d_scale(scale);
		}

		MUU_HIDDEN_CONSTRAINT(sfinae, bool sfinae = is_3d_scale_matrix_<matrix<Scalar, Rows, Columns>>)
		MUU_PURE_INLINE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_scale(Scalar x, Scalar y, Scalar z) noexcept
		{
			return matrix<Scalar, Rows, Columns>::from_3d_scale(x, y, z);
		}

		MUU_HIDDEN_CONSTRAINT(sfinae, bool sfinae = is_3d_scale_matrix_<matrix<Scalar, Rows, Columns>>)
		MUU_PURE_INLINE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_scale(MUU_VPARAM(vector<Scalar, 3>)
																					 scale) noexcept
		{
			return matrix<Scalar, Rows, Columns>::from_3d_scale(scale);
		}
	};

	//--- from_2d_translation() ----------------------------------------------------------------------------------------

	template <typename Derived, bool = is_translation_matrix_<Derived>>
	struct matrix_from_2d_translation
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct matrix_from_2d_translation<matrix<Scalar, Rows, Columns>, true>
	{
		MUU_PURE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_2d_translation(Scalar x, Scalar y) noexcept
		{
			using out_type = matrix<Scalar, Rows, Columns>;

			auto m									 = out_type::constants::identity;
			m.translation_column().template get<0>() = x;
			m.translation_column().template get<1>() = y;
			return m;
		}

		template <size_t D>
		MUU_PURE_INLINE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_2d_translation(
			const vector<Scalar, D>& xlat) noexcept
		{
			if constexpr (D == 1)
				return from_2d_translation(xlat.x, Scalar{});
			else
				return from_2d_translation(xlat.template get<0>(), xlat.template get<1>());
		}
	};

	//--- from_3d_translation() ----------------------------------------------------------------------------------------

	template <typename Derived, bool = is_3d_translation_matrix_<Derived>>
	struct matrix_from_3d_translation
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct matrix_from_3d_translation<matrix<Scalar, Rows, Columns>, true>
	{
		MUU_PURE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_3d_translation(Scalar x,
																						  Scalar y,
																						  Scalar z = Scalar{}) noexcept
		{
			using out_type = matrix<Scalar, Rows, Columns>;

			auto m									 = out_type::constants::identity;
			m.translation_column().template get<0>() = x;
			m.translation_column().template get<1>() = y;
			m.translation_column().template get<2>() = z;
			return m;
		}

		template <size_t D>
		MUU_PURE_INLINE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_3d_translation(
			const vector<Scalar, D>& xlat) noexcept
		{
			if constexpr (D == 1)
				return from_3d_translation(xlat.x, Scalar{});
			else if constexpr (D == 2)
				return from_3d_translation(xlat.x, xlat.y);
			else
				return from_3d_translation(xlat.template get<0>(), xlat.template get<1>(), xlat.template get<2>());
		}
	};

	//--- from_translation() ----------------------------------------------------------------------------------------

	template <typename Derived, bool = is_translation_matrix_<Derived>>
	struct matrix_from_translation
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct matrix_from_translation<matrix<Scalar, Rows, Columns>, true>
	{
		MUU_PURE_INLINE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_translation(Scalar x, Scalar y) noexcept
		{
			if constexpr (is_2d_translation_matrix_<matrix<Scalar, Rows, Columns>>)
				return matrix<Scalar, Rows, Columns>::from_2d_translation(x, y);
			else if constexpr (is_3d_translation_matrix_<matrix<Scalar, Rows, Columns>>)
				return matrix<Scalar, Rows, Columns>::from_3d_translation(x, y);
			else
				static_assert(always_false<Scalar>, "evaluated unreachable branch!");
		}

		MUU_PURE_INLINE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_translation(MUU_VPARAM(vector<Scalar, 2>)
																						   xlat) noexcept
		{
			return matrix<Scalar, Rows, Columns>::from_2d_translation(xlat);
		}

		MUU_HIDDEN_CONSTRAINT(sfinae, bool sfinae = is_3d_translation_matrix_<matrix<Scalar, Rows, Columns>>)
		MUU_PURE_INLINE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_translation(Scalar x,
																					   Scalar y,
																					   Scalar z) noexcept
		{
			return matrix<Scalar, Rows, Columns>::from_3d_translation(x, y, z);
		}

		MUU_HIDDEN_CONSTRAINT(sfinae, bool sfinae = is_3d_translation_matrix_<matrix<Scalar, Rows, Columns>>)
		MUU_PURE_INLINE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_translation(MUU_VPARAM(vector<Scalar, 3>)
																						   xlat) noexcept
		{
			return matrix<Scalar, Rows, Columns>::from_3d_translation(xlat);
		}
	};

	//--- 2d rotation matrices -----------------------------------------------------------------------------------------

	template <typename Derived, bool = is_2d_rotation_matrix_<Derived>>
	struct matrix_rotations_2d
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct matrix_rotations_2d<matrix<Scalar, Rows, Columns>, true>
	{
		static_assert(Rows >= 2 && Columns >= 2);
		static_assert(Rows <= 3 && Columns <= 3);
		static_assert(is_floating_point<Scalar>);

		MUU_CONSTRAINED_TEMPLATE(R >= 2 && C >= 2, size_t R, size_t C)
		MUU_PURE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_2d_rotation(
			const matrix<Scalar, R, C>& rot) noexcept
		{
			if constexpr (R == Rows && C == Columns && R == 2u && C == 2u)
				return rot;
			else
			{
				using out_type = matrix<Scalar, Rows, Columns>;

				out_type out{ rot };

				if constexpr (C >= 3u && Columns == 3u)
					out.translation_column() = vector<Scalar, Rows>{};
				if constexpr (R >= 3u && Rows == 3u)
				{
					out.template get<2, 0>() = Scalar{};
					out.template get<2, 1>() = Scalar{};
				}
				if constexpr (Rows == 3u && Columns == 3u)
					out.template get<2, 2>() = Scalar{ 1 };

				return out;
			}
		}

		MUU_PURE_GETTER
		constexpr bool has_2d_shear() const noexcept
		{
			using matrix_type = matrix<Scalar, Rows, Columns>;
			using dir_type	  = vector<Scalar, 2>;

			const auto x = dir_type::normalize(dir_type{ static_cast<const matrix_type&>(*this).x_column() });
			const auto y = dir_type::normalize(dir_type{ static_cast<const matrix_type&>(*this).y_column() });

			constexpr auto eps = constants<Scalar>::one_over_one_thousand;

			return !approx_equal(dir_type::dot(x, y), Scalar{}, eps);
		}
	};

	//--- 3d rotation matrices -----------------------------------------------------------------------------------------

	template <typename Derived, bool = is_3d_rotation_matrix_<Derived>>
	struct matrix_rotations_3d
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct matrix_rotations_3d<matrix<Scalar, Rows, Columns>, true>
	{
		static_assert(Rows >= 3 && Columns >= 3);
		static_assert(Rows <= 4 && Columns <= 4);
		static_assert(is_floating_point<Scalar>);

		MUU_CONSTRAINED_TEMPLATE(R >= 3 && C >= 3, size_t R, size_t C)
		MUU_PURE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_3d_rotation(
			const matrix<Scalar, R, C>& rot) noexcept
		{
			if constexpr (R == Rows && C == Columns && R == 3u && C == 3u)
				return rot;
			else
			{
				using out_type = matrix<Scalar, Rows, Columns>;

				out_type out{ rot };

				if constexpr (C >= 4u && Columns == 4u)
					out.translation_column() = vector<Scalar, Rows>{};
				if constexpr (R >= 4u && Rows == 4u)
				{
					out.template get<3, 0>() = Scalar{};
					out.template get<3, 1>() = Scalar{};
					out.template get<3, 2>() = Scalar{};
				}
				if constexpr (Rows == 4u && Columns == 4u)
					out.template get<3, 3>() = Scalar{ 1 };

				return out;
			}
		}

		MUU_PURE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_3d_direction(MUU_VPARAM(vector<Scalar, 3>)
																							dir)
		{
			using out_type	  = matrix<Scalar, Rows, Columns>;
			using vector_type = vector<Scalar, 3>;

			if constexpr (is_small_float_<Scalar>)
			{
				return out_type{ matrix<float, Rows, Columns>::from_3d_direction(vector<float, 3>{ dir }) };
			}
			else
			{
				MUU_FMA_BLOCK;

				// z axis
				const auto z = -dir;

				// pick a tentative y axis for the purposes of establishing our x axis
				auto y = vector_type{};
				if MUU_UNLIKELY(muu::approx_equal(dir.y, Scalar{ -1 }))
					y.z = Scalar{ -1 }; // looking directly down, our 'up' axis is the forward direction
				else if MUU_UNLIKELY(muu::approx_equal(dir.y, Scalar{ 1 }))
					y.z = Scalar{ 1 };	// looking directly up, our 'up' axis is the backward direction
				else
					y.y = Scalar{ 1 };	// otherwise use the real up axis

				// x axis
				const auto x = vector_type::normalize(vector_type::cross(y, z));

				// get the (actual) y axis
				y = vector_type::normalize(vector_type::cross(z, x));

				return out_type::from_axes(x, y, z);
			}
		}

		MUU_PURE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_quaternion(MUU_VPARAM(quaternion<Scalar>)
																						  quat) noexcept
		{
			using out_type = matrix<Scalar, Rows, Columns>;

			if constexpr (is_small_float_<Scalar>)
			{
				return out_type{ matrix<float, Rows, Columns>::from_quaternion(quaternion<float>{ quat }) };
			}
			else
			{
				MUU_FMA_BLOCK;

				const Scalar x2 = quat.v.x * quat.v.x;
				const Scalar y2 = quat.v.y * quat.v.y;
				const Scalar z2 = quat.v.z * quat.v.z;
				const Scalar sx = quat.s * quat.v.x;
				const Scalar sy = quat.s * quat.v.y;
				const Scalar sz = quat.s * quat.v.z;
				const Scalar xz = quat.v.x * quat.v.z;
				const Scalar yz = quat.v.y * quat.v.z;
				const Scalar xy = quat.v.x * quat.v.y;

				using col_type = vector<Scalar, Rows>;
				out_type out{ col_type{ /* 0, 0 */ Scalar{ 1 } - Scalar{ 2 } * (y2 + z2),
										/* 1, 0 */ Scalar{ 2 } * (xy + sz),
										/* 2, 0 */ Scalar{ 2 } * (xz - sy) },
							  col_type{ /* 0, 1 */ Scalar{ 2 } * (xy - sz),
										/* 1, 1 */ Scalar{ 1 } - Scalar{ 2 } * (x2 + z2),
										/* 2, 1 */ Scalar{ 2 } * (sx + yz) },
							  col_type{ /* 0, 2 */ Scalar{ 2 } * (sy + xz),
										/* 1, 2 */ Scalar{ 2 } * (yz - sx),
										/* 2, 2 */ Scalar{ 1 } - Scalar{ 2 } * (x2 + y2) } };

				if constexpr (Rows == 4u && Columns == 4u)
					out.template get<3, 3>() = Scalar{ 1 };

				return out;
			}
		}

		MUU_PURE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_axis_angle(MUU_VPARAM(vector<Scalar, 3>)
																						  axis,
																					  Scalar angle) noexcept
		{
			if constexpr (is_small_float_<Scalar>)
			{
				using out_type = matrix<Scalar, Rows, Columns>;

				return out_type{ matrix<float, Rows, Columns>::from_axis_angle(vector<float, 3>{ axis },
																			   static_cast<float>(angle)) };
			}
			else
			{
				return from_quaternion(quaternion<Scalar>::from_axis_angle(axis, angle));
			}
		}

		MUU_PURE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_axis_angle(MUU_VPARAM(axis_angle<Scalar>)
																						  aa) noexcept
		{
			return from_axis_angle(aa.axis, aa.angle);
		}

		MUU_PURE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_euler(Scalar yaw,
																				 Scalar pitch,
																				 Scalar roll) noexcept
		{
			if constexpr (is_small_float_<Scalar>)
			{
				using out_type = matrix<Scalar, Rows, Columns>;

				return out_type{ matrix<float, Rows, Columns>::from_euler(static_cast<float>(yaw),
																		  static_cast<float>(pitch),
																		  static_cast<float>(roll)) };
			}
			else
			{
				return from_quaternion(quaternion<Scalar>::from_euler(yaw, pitch, roll));
			}
		}

		MUU_PURE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_euler(MUU_VPARAM(euler_angles<Scalar>)
																					 angles) noexcept
		{
			return from_euler(angles.yaw, angles.pitch, angles.roll);
		}

		MUU_PURE_GETTER
		constexpr bool has_3d_shear() const noexcept
		{
			using matrix_type = matrix<Scalar, Rows, Columns>;
			using dir_type	  = vector<Scalar, 3>;

			const auto x = dir_type::normalize(dir_type{ static_cast<const matrix_type&>(*this).x_column() });
			const auto y = dir_type::normalize(dir_type{ static_cast<const matrix_type&>(*this).y_column() });
			const auto z = dir_type::normalize(dir_type{ static_cast<const matrix_type&>(*this).z_column() });

			constexpr auto eps = constants<Scalar>::one_over_one_thousand;

			return !approx_equal(dir_type::dot(x, y), Scalar{}, eps) //
				|| !approx_equal(dir_type::dot(x, z), Scalar{}, eps) //
				|| !approx_equal(dir_type::dot(y, z), Scalar{}, eps);
		}
	};

	//--- rotation matrices --------------------------------------------------------------------------------------------

	template <typename Derived, bool = is_rotation_matrix_<Derived>>
	struct matrix_rotations
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct matrix_rotations<matrix<Scalar, Rows, Columns>, true>
	{
		static_assert(Rows >= 2 && Columns >= 2);
		static_assert(Rows <= 4 && Columns <= 4);
		static_assert(is_floating_point<Scalar>);

		MUU_HIDDEN_CONSTRAINT(sfinae, bool sfinae = is_2d_rotation_matrix_<matrix<Scalar, Rows, Columns>>)
		MUU_PURE_INLINE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_rotation(
			const matrix<Scalar, 2, 2>& rot) noexcept
		{
			return matrix<Scalar, Rows, Columns>::from_2d_rotation(rot);
		}

		MUU_HIDDEN_CONSTRAINT(sfinae, bool sfinae = is_2d_rotation_matrix_<matrix<Scalar, Rows, Columns>>)
		MUU_PURE_INLINE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_rotation(
			const matrix<Scalar, 2, 3>& rot) noexcept
		{
			return matrix<Scalar, Rows, Columns>::from_2d_rotation(rot);
		}

		MUU_HIDDEN_CONSTRAINT(sfinae, bool sfinae = is_2d_rotation_matrix_<matrix<Scalar, Rows, Columns>>)
		MUU_PURE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_axes(MUU_VPARAM(vector<Scalar, 2>) x,
																				MUU_VPARAM(vector<Scalar, 2>)
																					y) noexcept
		{
			using out_type = matrix<Scalar, Rows, Columns>;

			out_type out{ x, y };

			if constexpr (Rows >= 3u && Columns >= 3u)
				out.template get<2, 2>() = Scalar{ 1 };
			if constexpr (Rows >= 4u && Columns >= 4u)
				out.template get<3, 3>() = Scalar{ 1 };

			return out;
		}

		MUU_HIDDEN_CONSTRAINT(sfinae, bool sfinae = is_3d_rotation_matrix_<matrix<Scalar, Rows, Columns>>)
		MUU_PURE_INLINE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_rotation(
			const matrix<Scalar, 3, 3>& rot) noexcept
		{
			return matrix<Scalar, Rows, Columns>::from_3d_rotation(rot);
		}

		MUU_HIDDEN_CONSTRAINT(sfinae, bool sfinae = is_3d_rotation_matrix_<matrix<Scalar, Rows, Columns>>)
		MUU_PURE_INLINE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_rotation(
			const matrix<Scalar, 3, 4>& rot) noexcept
		{
			return matrix<Scalar, Rows, Columns>::from_3d_rotation(rot);
		}

		MUU_HIDDEN_CONSTRAINT(sfinae, bool sfinae = is_3d_rotation_matrix_<matrix<Scalar, Rows, Columns>>)
		MUU_PURE_INLINE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_rotation(
			const matrix<Scalar, 4, 4>& rot) noexcept
		{
			return matrix<Scalar, Rows, Columns>::from_3d_rotation(rot);
		}

		MUU_HIDDEN_CONSTRAINT(sfinae, bool sfinae = is_3d_rotation_matrix_<matrix<Scalar, Rows, Columns>>)
		MUU_PURE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL from_axes(MUU_VPARAM(vector<Scalar, 3>) x,
																				MUU_VPARAM(vector<Scalar, 3>) y,
																				MUU_VPARAM(vector<Scalar, 3>)
																					z) noexcept
		{
			using out_type = matrix<Scalar, Rows, Columns>;

			out_type out{ x, y, z };

			if constexpr (Rows >= 4u && Columns >= 4u)
				out.template get<3, 3>() = Scalar{ 1 };

			return out;
		}
	};

	//--- transform_position() ----------------------------------------------------------------------------------------

	template <typename Derived, bool = is_3d_transform_matrix_<Derived>>
	struct matrix_transform_position
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct matrix_transform_position<matrix<Scalar, Rows, Columns>, true>
	{
		static_assert(Rows == 3 || Rows == 4);
		static_assert(Columns == 3 || Columns == 4);

		MUU_PURE_GETTER
		static constexpr vector<Scalar, 3> MUU_VECTORCALL transform_position(MUU_VPARAM(matrix<Scalar, Rows, Columns>)
																				 xform,
																			 MUU_VPARAM(vector<Scalar, 3>) pos) noexcept
		{
			MUU_FMA_BLOCK;

			if constexpr (Columns == 3)
			{
				static_assert(Rows == 3); // a plain ol' 3x3 rotation matrix.

				return xform * pos;
			}
			else
			{
				auto inv_w = Scalar{ 1 };
				if constexpr (Rows == 4)
				{
					inv_w /= (xform.template get<3, 0>() * pos.x   //
							  + xform.template get<3, 1>() * pos.y //
							  + xform.template get<3, 2>() * pos.z //
							  + xform.template get<3, 3>());
				}

				return { inv_w
							 * (xform.template get<0, 0>() * pos.x	 //
								+ xform.template get<0, 1>() * pos.y //
								+ xform.template get<0, 2>() * pos.z //
								+ xform.template get<0, 3>()),

						 inv_w
							 * (xform.template get<1, 0>() * pos.x	 //
								+ xform.template get<1, 1>() * pos.y //
								+ xform.template get<1, 2>() * pos.z //
								+ xform.template get<1, 3>()),

						 inv_w
							 * (xform.template get<2, 0>() * pos.x	 //
								+ xform.template get<2, 1>() * pos.y //
								+ xform.template get<2, 2>() * pos.z //
								+ xform.template get<2, 3>()) };
			}
		}

		MUU_PURE_INLINE_GETTER
		constexpr vector<Scalar, 3> MUU_VECTORCALL transform_position(MUU_VPARAM(vector<Scalar, 3>) pos) const noexcept
		{
			return transform_position(static_cast<const matrix<Scalar, Rows, Columns>&>(*this), pos);
		}
	};

	template <typename Derived, bool = (is_3d_transform_matrix_<Derived> && !is_matrix_<Derived, 3, 3, 3, 3>)>
	struct matrix_transform_position_operator
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct matrix_transform_position_operator<matrix<Scalar, Rows, Columns>, true>
	{
		static_assert(Rows == 3 || Rows == 4);
		static_assert(Columns == 3 || Columns == 4);
		static_assert(Rows != Columns || Rows != 3); // should not be instantiated for 3x3 matrices since that's already
													 // handled by the generalized mat * vec operator

		MUU_PURE_INLINE_GETTER
		friend constexpr vector<Scalar, 3> MUU_VECTORCALL operator*(MUU_VPARAM(matrix<Scalar, Rows, Columns>) xform,
																	MUU_VPARAM(vector<Scalar, 3>) pos) noexcept
		{
			return matrix<Scalar, Rows, Columns>::transform_position(xform, pos);
		}
	};

	//--- transform_without_translating() -----------------------------------------------------------------------------

	template <typename Derived, bool = is_3d_transform_matrix_<Derived>>
	struct matrix_transform_without_translating
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct matrix_transform_without_translating<matrix<Scalar, Rows, Columns>, true>
	{
		static_assert(Rows == 3 || Rows == 4);
		static_assert(Columns == 3 || Columns == 4);

		MUU_PURE_GETTER
		static constexpr vector<Scalar, 3> MUU_VECTORCALL transform_without_translating(
			MUU_VPARAM(matrix<Scalar, Rows, Columns>) xform,
			MUU_VPARAM(vector<Scalar, 3>) dir) noexcept
		{
			MUU_FMA_BLOCK;

			return vector<Scalar, 3>{ xform.template get<0, 0>() * dir.x	   //
										  + xform.template get<0, 1>() * dir.y //
										  + xform.template get<0, 2>() * dir.z,

									  xform.template get<1, 0>() * dir.x	   //
										  + xform.template get<1, 1>() * dir.y //
										  + xform.template get<1, 2>() * dir.z,

									  xform.template get<2, 0>() * dir.x	   //
										  + xform.template get<2, 1>() * dir.y //
										  + xform.template get<2, 2>() * dir.z };
		}

		MUU_PURE_INLINE_GETTER
		constexpr vector<Scalar, 3> MUU_VECTORCALL transform_without_translating(MUU_VPARAM(vector<Scalar, 3>)
																					 dir) const noexcept
		{
			return transform_without_translating(static_cast<const matrix<Scalar, Rows, Columns>&>(*this), dir);
		}
	};

	//--- transform_direction() ----------------------------------------------------------------------------------------

	template <typename Derived, bool = is_3d_transform_matrix_<Derived>>
	struct matrix_transform_direction
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct matrix_transform_direction<matrix<Scalar, Rows, Columns>, true>
	{
		static_assert(Rows == 3 || Rows == 4);
		static_assert(Columns == 3 || Columns == 4);

		MUU_PURE_GETTER
		static constexpr vector<Scalar, 3> MUU_VECTORCALL transform_direction(MUU_VPARAM(matrix<Scalar, Rows, Columns>)
																				  xform,
																			  MUU_VPARAM(vector<Scalar, 3>)
																				  dir) noexcept
		{
			MUU_FMA_BLOCK;

			return vector<Scalar, 3>::length(dir)
				 * vector<Scalar, 3>::normalize(
					   matrix<Scalar, Rows, Columns>::transform_without_translating(xform, dir));
		}

		MUU_PURE_INLINE_GETTER
		constexpr vector<Scalar, 3> MUU_VECTORCALL transform_direction(MUU_VPARAM(vector<Scalar, 3>) dir) const noexcept
		{
			return transform_direction(static_cast<const matrix<Scalar, Rows, Columns>&>(*this), dir);
		}
	};

	//--- extract_2d_scale() -----------------------------------------------------------------------------------------

	template <typename Derived, bool = is_2d_transform_matrix_<Derived>>
	struct matrix_extract_2d_scale
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct matrix_extract_2d_scale<matrix<Scalar, Rows, Columns>, true>
	{
		MUU_PURE_GETTER
		static constexpr vector<Scalar, 2> MUU_VECTORCALL extract_2d_scale(MUU_VPARAM(matrix<Scalar, Rows, Columns>)
																			   mat) noexcept
		{
			return { vector<Scalar, 2>{ mat.template get<0, 0>(), mat.template get<1, 0>() }.length(),
					 vector<Scalar, 2>{ mat.template get<0, 1>(), mat.template get<1, 1>() }.length() };
		}

		MUU_PURE_GETTER
		constexpr vector<Scalar, 2> extract_2d_scale() const noexcept
		{
			return extract_2d_scale(static_cast<const matrix<Scalar, Rows, Columns>&>(*this));
		}
	};

	//--- extract_3d_scale() -----------------------------------------------------------------------------------------

	template <typename Derived, bool = is_3d_transform_matrix_<Derived>>
	struct matrix_extract_3d_scale
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct matrix_extract_3d_scale<matrix<Scalar, Rows, Columns>, true>
	{
		MUU_PURE_GETTER
		static constexpr vector<Scalar, 3> MUU_VECTORCALL extract_3d_scale(MUU_VPARAM(matrix<Scalar, Rows, Columns>)
																			   mat) noexcept
		{
			return { vector<Scalar, 3>{ mat.template get<0, 0>(), mat.template get<1, 0>(), mat.template get<2, 0>() }
						 .length(),
					 vector<Scalar, 3>{ mat.template get<0, 1>(), mat.template get<1, 1>(), mat.template get<2, 1>() }
						 .length(),
					 vector<Scalar, 3>{ mat.template get<0, 2>(), mat.template get<1, 2>(), mat.template get<2, 2>() }
						 .length() };
		}

		MUU_PURE_GETTER
		constexpr vector<Scalar, 3> extract_3d_scale() const noexcept
		{
			return extract_3d_scale(static_cast<const matrix<Scalar, Rows, Columns>&>(*this));
		}
	};

	//--- perspective_projection() -------------------------------------------------------------------------------------

	template <typename Derived, bool = is_floating_point_matrix_<Derived>>
	struct matrix_perspective_projection
	{};

	template <typename T>
	struct matrix_perspective_projection<matrix<T, 4, 4>, true>
	{
		template <math_apis Mode = (math_apis::directx | math_apis::right_handed)>
		MUU_PURE_GETTER
		static constexpr matrix<T, 4, 4> MUU_VECTORCALL perspective_projection(T vertical_fov,
																			   T aspect_ratio,
																			   T near_clip,
																			   T far_clip) noexcept
		{
			// https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovrh
			// https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
			// http://perry.cz/articles/ProjectionMatrix.xhtml
			// http://www.songho.ca/opengl/gl_projectionmatrix.html

			MUU_FMA_BLOCK;

			const auto b = T{ 1 } / muu::tan(vertical_fov / T{ 2 });
			const auto a = b / aspect_ratio;

			if constexpr (!!(Mode & math_apis::opengl))
			{
				const auto c = -(far_clip + near_clip) / (far_clip - near_clip);

				if constexpr (!!(Mode & math_apis::left_handed))
				{
					const auto e = (T{ 2 } * far_clip * near_clip) / (far_clip - near_clip);

					// clang-format off
					return
					{
						a,		T{},	T{},	T{},
						T{},	b,		T{},	T{},
						T{},	T{},	c,		e,
						T{},	T{},	T{1},	T{}
					};
					// clang-format on
				}
				else // right-handed
				{
					const auto e = (T{ -2 } * far_clip * near_clip) / (far_clip - near_clip);

					// clang-format off
					return
					{
						a,		T{},	T{},	T{},
						T{},	b,		T{},	T{},
						T{},	T{},	c,		e,
						T{},	T{},	T{-1},	T{}
					};
					// clang-format on
				}
			}
			else // directx
			{
				const auto c = far_clip / (near_clip - far_clip);

				if constexpr (!!(Mode & math_apis::left_handed))
				{
					const auto e = -near_clip * far_clip / (far_clip - near_clip);

					// clang-format off
					return
					{
						a,		T{},	T{},	T{},
						T{},	b,		T{},	T{},
						T{},	T{},	c,		e,
						T{},	T{},	T{1},	T{}
					};
					// clang-format on
				}
				else // right-handed
				{
					const auto e = near_clip * far_clip / (near_clip - far_clip);

					// clang-format off
					return
					{
						a,		T{},	T{},	T{},
						T{},	b,		T{},	T{},
						T{},	T{},	c,		e,
						T{},	T{},	T{-1},	T{}
					};
					// clang-format on
				}
			}
		}

		template <math_apis Mode = (math_apis::directx | math_apis::right_handed)>
		MUU_PURE_INLINE_GETTER
		static constexpr matrix<T, 4, 4> MUU_VECTORCALL perspective_projection(T vertical_fov,
																			   MUU_VPARAM(vector<T, 2>) screen_size,
																			   T near_clip,
																			   T far_clip) noexcept
		{
			return perspective_projection(vertical_fov, screen_size.x / screen_size.y, near_clip, far_clip);
		}
	};

	//--- determinants -------------------------------------------------------------------------------------------------

#define MAT_GET(r, c) static_cast<type>(m.m[c].template get<r>())

	template <size_t Row0 = 0, size_t Row1 = 1, size_t Col0 = 0, size_t Col1 = 1, typename T>
	MUU_PURE_GETTER
	static constexpr promote_if_small_float<typename T::determinant_type> MUU_VECTORCALL raw_determinant_2x2(
		const T& m) noexcept
	{
		MUU_FMA_BLOCK;
		using type = promote_if_small_float<typename T::determinant_type>;

		return MAT_GET(Row0, Col0) * MAT_GET(Row1, Col1) - MAT_GET(Row0, Col1) * MAT_GET(Row1, Col0);
	}

	template <size_t Row0 = 0,
			  size_t Row1 = 1,
			  size_t Row2 = 2,
			  size_t Col0 = 0,
			  size_t Col1 = 1,
			  size_t Col2 = 2,
			  typename T>
	MUU_PURE_GETTER
	static constexpr promote_if_small_float<typename T::determinant_type> MUU_VECTORCALL raw_determinant_3x3(
		const T& m) noexcept
	{
		MUU_FMA_BLOCK;
		using type = promote_if_small_float<typename T::determinant_type>;

		return MAT_GET(Row0, Col0) * raw_determinant_2x2<Row1, Row2, Col1, Col2>(m)
			 - MAT_GET(Row0, Col1) * raw_determinant_2x2<Row1, Row2, Col0, Col2>(m)
			 + MAT_GET(Row0, Col2) * raw_determinant_2x2<Row1, Row2, Col0, Col1>(m);
	}

	template <size_t Row0 = 0,
			  size_t Row1 = 1,
			  size_t Row2 = 2,
			  size_t Row3 = 3,
			  size_t Col0 = 0,
			  size_t Col1 = 1,
			  size_t Col2 = 2,
			  size_t Col3 = 3,
			  typename T>
	MUU_PURE_GETTER
	static constexpr promote_if_small_float<typename T::determinant_type> MUU_VECTORCALL raw_determinant_4x4(
		const T& m) noexcept
	{
		MUU_FMA_BLOCK;
		using type = promote_if_small_float<typename T::determinant_type>;

		return MAT_GET(Row0, Col0) * raw_determinant_3x3<Row1, Row2, Row3, Col1, Col2, Col3>(m)
			 - MAT_GET(Row0, Col1) * raw_determinant_3x3<Row1, Row2, Row3, Col0, Col2, Col3>(m)
			 + MAT_GET(Row0, Col2) * raw_determinant_3x3<Row1, Row2, Row3, Col0, Col1, Col3>(m)
			 - MAT_GET(Row0, Col3) * raw_determinant_3x3<Row1, Row2, Row3, Col0, Col1, Col2>(m);
	}

#undef MAT_GET

	//--- orthonormalize() -----------------------------------------------------------------------------------------

	template <typename Derived, bool = is_3d_rotation_matrix_<Derived>>
	struct matrix_orthonormalize
	{};

	template <typename Scalar, size_t Rows, size_t Columns>
	struct matrix_orthonormalize<matrix<Scalar, Rows, Columns>, true>
	{
		using scalar_type = Scalar;
		using column_type = vector<Scalar, Rows>;

	  private:
		template <size_t Depth = Rows>
		MUU_PURE_GETTER
		static constexpr scalar_type MUU_VECTORCALL column_dot(MUU_VPARAM(column_type) c1,
															   MUU_VPARAM(column_type) c2) noexcept
		{
			static_assert(Depth > 0);
			static_assert(Depth <= Rows);
			static_assert(is_floating_point<scalar_type>);
			static_assert(!is_small_float_<scalar_type>);

			if constexpr (Depth == Rows)
				return column_type::dot(c1, c2);
			else
			{
				MUU_FMA_BLOCK;

				// avoid operator[] for vectors <= 4 elems (potentially slower and/or not-constexpr safe)
				scalar_type dot = c1.template get<0>() * c2.template get<0>();
				if constexpr (Depth > 1)
					dot += c1.template get<1>() * c2.template get<1>();
				if constexpr (Depth > 2)
					dot += c1.template get<2>() * c2.template get<2>();
				if constexpr (Depth > 3)
					dot += c1.template get<3>() * c2.template get<3>();
				if constexpr (Depth > 4)
				{
					for (size_t i = 4; i < Depth; i++)
						dot += c1[i] * c2[i];
				}
				return dot;
			}
		}

		template <size_t Depth = Rows>
		static constexpr void column_normalize(column_type& c) noexcept
		{
			static_assert(Depth > 0);
			static_assert(Depth <= Rows);
			static_assert(is_floating_point<scalar_type>);
			static_assert(!is_small_float_<scalar_type>);

			if constexpr (Depth == Rows)
				c.normalize();
			else
			{
				MUU_FMA_BLOCK;

				const auto inv_len = scalar_type{ 1 } / muu::sqrt(column_dot<Depth>(c, c));

				// avoid operator[] for vectors <= 4 elems (potentially slower and/or not-constexpr safe)
				c.template get<0>() = c.template get<0>() * inv_len;
				if constexpr (Depth > 1)
					c.template get<1>() = c.template get<1>() * inv_len;
				if constexpr (Depth > 2)
					c.template get<2>() = c.template get<2>() * inv_len;
				if constexpr (Depth > 3)
					c.template get<3>() = c.template get<3>() * inv_len;
				if constexpr (Depth > 4)
				{
					for (size_t i = 4; i < Depth; i++)
						c[i] = c[i] * inv_len;
				}
			}
		}

	  public:
		MUU_PURE_GETTER
		static constexpr matrix<Scalar, Rows, Columns> MUU_VECTORCALL orthonormalize(
			MUU_VPARAM(matrix<Scalar, Rows, Columns>) m) noexcept
		{
			if constexpr (is_small_float_<scalar_type>)
			{
				using promoted_mat = matrix<promote_if_small_float<scalar_type>, Rows, Columns>;

				return matrix<Scalar, Rows, Columns>{ promoted_mat::orthonormalize(promoted_mat{ m }) };
			}
			else
			{
				MUU_FMA_BLOCK;

				// 'modified' gram-schmidt:
				// https://fgiesen.wordpress.com/2013/06/02/modified-gram-schmidt-orthogonalization/

				matrix<Scalar, Rows, Columns> out{ m };

				// x-axis
				column_normalize<3>(out.m[0]);

				constexpr auto subtract_dot_mult = [](auto& out_, auto& c1, auto& c2) noexcept
				{
					MUU_FMA_BLOCK;

					const auto dot = column_dot<3>(c1, c2);
					out_.template get<0>() -= dot * c2.template get<0>();
					out_.template get<1>() -= dot * c2.template get<1>();
					out_.template get<2>() -= dot * c2.template get<2>();
				};

				// y-axis
				subtract_dot_mult(out.m[1], m.m[1], out.m[0]);
				column_normalize<3>(out.m[1]);

				// z-axis
				subtract_dot_mult(out.m[2], m.m[2], out.m[0]);
				subtract_dot_mult(out.m[2], out.m[2], out.m[1]);
				column_normalize<3>(out.m[2]);

				return out;
			}
		}

		constexpr matrix<Scalar, Rows, Columns>& orthonormalize() noexcept
		{
			return static_cast<matrix<Scalar, Rows, Columns>&>(*this) =
					   orthonormalize(static_cast<matrix<Scalar, Rows, Columns>&>(*this));
		}

	  private:
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL orthonormalized(MUU_VPARAM(vector<Scalar, 3>) x,
															 MUU_VPARAM(vector<Scalar, 3>) y,
															 MUU_VPARAM(vector<Scalar, 3>) z) noexcept
		{
			constexpr auto eps = is_small_float_<Scalar> ? constants<Scalar>::one_over_one_hundred
														 : constants<Scalar>::one_over_one_thousand;

			using vec = vector<Scalar, 3>;
			return vec::normalized(x)					 //
				&& vec::normalized(y)					 //
				&& vec::normalized(z)					 //
				&& muu::approx_zero(vec::dot(x, y), eps) //
				&& muu::approx_zero(vec::dot(x, z), eps) //
				&& muu::approx_zero(vec::dot(y, z), eps);
		}

	  public:
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL orthonormalized(MUU_VPARAM(matrix<Scalar, Rows, Columns>) m) noexcept
		{
			if constexpr (Rows == 3)
			{
				return orthonormalized(m.x_column(), m.y_column(), m.z_column());
			}
			else
			{
				using vec = vector<Scalar, 3>;
				return orthonormalized(vec{ m.x_column() }, vec{ m.y_column() }, vec{ m.z_column() });
			}
		}

		MUU_PURE_INLINE_GETTER
		constexpr bool MUU_VECTORCALL orthonormalized() const noexcept
		{
			return orthonormalized(static_cast<const matrix<Scalar, Rows, Columns>&>(*this));
		}
	};

} // impl

#define SPECIALIZED_IF(cond) , bool = (cond)

/// \endcond

#ifndef SPECIALIZED_IF
	#define SPECIALIZED_IF(cond)
#endif

//======================================================================================================================
// MATRIX CLASS
//======================================================================================================================

namespace muu
{
	/// \brief A matrix.
	/// \ingroup math
	///
	/// \tparam	Scalar      The matrix's scalar component type.
	/// \tparam Rows		The number of rows in the matrix.
	/// \tparam Columns		The number of columns in the matrix.
	template <typename Scalar, size_t Rows, size_t Columns>
	struct MUU_EMPTY_BASES MUU_TRIVIAL_ABI matrix //
		MUU_HIDDEN_BASE(
			impl::storage_base<matrix<Scalar, Rows, Columns>>,
			impl::matrix_get_xy_column<matrix<Scalar, Rows, Columns>>,
			impl::matrix_get_z_column<matrix<Scalar, Rows, Columns>>,
			impl::matrix_get_w_column<matrix<Scalar, Rows, Columns>>,
			impl::matrix_get_translation_column<matrix<Scalar, Rows, Columns>>,
			impl::matrix_from_2d_scale<matrix<Scalar, Rows, Columns>>,
			impl::matrix_from_3d_scale<matrix<Scalar, Rows, Columns>>,
			impl::matrix_from_scale<matrix<Scalar, Rows, Columns>>,
			impl::matrix_from_2d_translation<matrix<Scalar, Rows, Columns>>,
			impl::matrix_from_3d_translation<matrix<Scalar, Rows, Columns>>,
			impl::matrix_from_translation<matrix<Scalar, Rows, Columns>>,
			impl::matrix_rotations_2d<matrix<Scalar, Rows, Columns>>,
			impl::matrix_rotations_3d<matrix<Scalar, Rows, Columns>>,
			impl::matrix_rotations<matrix<Scalar, Rows, Columns>>,
			impl::matrix_transform_position<matrix<Scalar, Rows, Columns>>,
			impl::matrix_transform_position_operator<matrix<Scalar, Rows, Columns>>,
			impl::matrix_transform_without_translating<matrix<Scalar, Rows, Columns>>,
			impl::matrix_transform_direction<matrix<Scalar, Rows, Columns>>,
			impl::matrix_extract_2d_scale<matrix<Scalar, Rows, Columns>>,
			impl::matrix_extract_3d_scale<matrix<Scalar, Rows, Columns>>,
			impl::matrix_orthonormalize<matrix<Scalar, Rows, Columns>>,
			impl::matrix_perspective_projection<matrix<Scalar, Rows, Columns>>
		)
	{
		static_assert(!std::is_reference_v<Scalar>, "Matrix scalar type cannot be a reference");
		static_assert(!is_cv<Scalar>, "Matrix scalar type cannot be const- or volatile-qualified");
		static_assert(std::is_trivially_constructible_v<Scalar>	  //
						  && std::is_trivially_copyable_v<Scalar> //
						  && std::is_trivially_destructible_v<Scalar>,
					  "Matrix scalar type must be trivially constructible, copyable and destructible");
		static_assert(Rows >= 1, "Matrices must have at least one row");
		static_assert(Columns >= 1, "Matrices must have at least one column");

		/// \brief The number of rows in the matrix.
		static constexpr size_t rows = Rows;

		/// \brief The number of columns in the matrix.
		static constexpr size_t columns = Columns;

		/// \brief The type of each scalar component stored in this matrix.
		using scalar_type = Scalar;

		/// \brief The type of one row of this matrix.
		using row_type = vector<scalar_type, columns>;

		/// \brief The type of one column of this matrix.
		using column_type = vector<scalar_type, rows>;

		/// \brief The #muu::quaternion with the same #scalar_type as this matrix.
		using quaternion_type = quaternion<scalar_type>;

		/// \brief The #muu::axis_angle with the same #scalar_type as this matrix.
		using axis_angle_type = axis_angle<scalar_type>;

		/// \brief The #muu::euler_angles with the same #scalar_type as this matrix.
		using euler_type = euler_angles<scalar_type>;

		/// \brief Compile-time constants for this matrix.
		using constants = muu::constants<matrix>;

		/// \brief The scalar type used for determinants. Always signed.
		using determinant_type = std::
			conditional_t<is_integral<scalar_type>, impl::highest_ranked<make_signed<scalar_type>, int>, scalar_type>;

		/// \brief The scalar type returned by inversion operations. Always floating-point.
		using inverse_scalar_type = std::conditional_t<is_integral<scalar_type>, double, scalar_type>;

		/// \brief The matrix type returned by inversion operations. Always floating-point.
		using inverse_type = matrix<inverse_scalar_type, Rows, Columns>;

	  private:
		/// \cond

		template <typename, size_t, size_t>
		friend struct matrix;

		using base = impl::storage_base<matrix<Scalar, Rows, Columns>>;
		static_assert(sizeof(base) == (sizeof(scalar_type) * Rows * Columns), "Matrices should not have padding");

		using promoted_scalar				 = promote_if_small_float<scalar_type>;
		using promoted_mat					 = matrix<promoted_scalar, Rows, Columns>;
		static constexpr bool is_small_float = impl::is_small_float_<scalar_type>;

		using product_scalar_type  = typename column_type::product_scalar_type;
		using promoted_product	   = promote_if_small_float<product_scalar_type>;
		using promoted_product_mat = matrix<promoted_product, Rows, Columns>;
		static constexpr bool product_requires_promotion =
			!all_same<product_scalar_type, scalar_type, promoted_product>;

		static_assert(is_floating_point<inverse_scalar_type>);
		using promoted_inverse	   = promote_if_small_float<inverse_scalar_type>;
		using promoted_inverse_mat = matrix<promoted_inverse, Rows, Columns>;
		static constexpr bool inverse_requires_promotion =
			!all_same<inverse_scalar_type, scalar_type, promoted_inverse>;

		using scalar_constants = muu::constants<scalar_type>;

		/// \endcond

	  public:
#if MUU_DOXYGEN

		/// \brief The values in the matrix (stored column-major).
		column_type m[columns];

#endif // DOXYGEN

#if 1  // constructors ---------------------------------------------------------------------------------------------

		/// \brief Default constructor. Values are not initialized.
		MUU_NODISCARD_CTOR
		matrix() noexcept = default;

		/// \brief Copy constructor.
		MUU_NODISCARD_CTOR
		constexpr matrix(const matrix&) noexcept = default;

		/// \brief Copy-assigment operator.
		constexpr matrix& operator=(const matrix&) noexcept = default;

		/// \brief	Constructs a matrix by broadcasting a scalar value to all components.
		///
		/// \details \cpp
		/// std::cout << matrix<int, 3, 3>{ 1 } << "\n";
		/// \ecpp
		///
		/// \out
		/// {    1,    1,    1,
		///	     1,    1,    1,
		///	     1,    1,    1 }
		/// \eout
		///
		/// \param	broadcast	The value used to initialize each of the matrix's scalar components.
		MUU_NODISCARD_CTOR
		explicit constexpr matrix(scalar_type broadcast) noexcept
			: base{ impl::broadcast_tag{}, std::make_index_sequence<Columns>{}, broadcast }
		{}

		/// \brief	Constructs a matrix from (row-major-ordered) scalars.
		///
		/// \details \cpp
		/// // explicitly-sized matrices:
		/// std::cout << matrix<int, 2, 3>{ 1, 2, 3, 4, 5, 6 } << "\n\n";
		///
		/// // 2x2, 2x3, 3x3, 3x4 and 4x4 matrices can be deduced automatically
		/// // from 4, 6, 9, 12 and 16 inputs (respectively):
		/// std::cout << matrix{ 1, 2, 3, 4 } << "\n\n";
		/// std::cout << matrix{ 1, 2, 3, 4, 5, 6 } << "\n\n";
		/// std::cout << matrix{ 1, 2, 3, 4, 5, 6, 7, 8, 9 } << "\n\n";
		/// std::cout << matrix{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 } << "\n\n";
		/// std::cout << matrix{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } << "\n\n";
		/// \ecpp
		///
		/// \out
		/// {    1,    2,
		///      3,    4 }
		///
		/// {    1,    2,    3,
		///      4,    5,    6 }
		///
		/// {    1,    2,    3,
		///      4,    5,    6,
		///      7,    8,    9 }
		///
		/// {    1,    2,    3,    4,
		///      5,    6,    7,    8,
		///      9,   10,   11,   12 }
		///
		/// {    1,    2,    3,    4,
		///      5,    6,    7,    8,
		///      9,   10,   11,   12,
		///     13,   14,   15,   16 }
		/// \eout
		///
		/// Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \tparam T		Types convertible to #scalar_type.
		/// \param	v0		Initial value for the matrix's first scalar component.
		/// \param	v1		Initial value for the matrix's second scalar component.
		/// \param	vals	Initial values for the matrix's remaining scalar components.
		MUU_CONSTRAINED_TEMPLATE((!impl::is_common_matrix_<matrix>			 //
								  && (2u + sizeof...(T)) <= (Rows * Columns) //
								  && all_convertible_to<scalar_type, scalar_type, const T&...>),
								 typename... T)
		MUU_NODISCARD_CTOR
		constexpr matrix(scalar_type v0, scalar_type v1, const T&... vals) noexcept
			: base{ impl::row_major_tuple_tag{}, std::tuple<scalar_type, scalar_type, const T&...>{ v0, v1, vals... } }
		{}

		/// \cond

		// row-major scalar constructor optimizations for some common cases

		// 2x2
		MUU_HIDDEN_CONSTRAINT((R == 2 && C == 2), size_t R = Rows, size_t C = Columns)
		MUU_NODISCARD_CTOR
		constexpr matrix(scalar_type v00,
						 scalar_type v01,
						 scalar_type v10 = scalar_type{},
						 scalar_type v11 = scalar_type{}) noexcept //
			: base{ v00, v01, v10, v11 }
		{}

		// 2x3
		MUU_HIDDEN_CONSTRAINT((R == 2 && C == 3), size_t R = Rows, size_t C = Columns)
		MUU_NODISCARD_CTOR
		constexpr matrix(scalar_type v00,
						 scalar_type v01,
						 scalar_type v02 = scalar_type{},
						 scalar_type v10 = scalar_type{},
						 scalar_type v11 = scalar_type{},
						 scalar_type v12 = scalar_type{}) noexcept //
			: base{ v00, v01, v02, v10, v11, v12 }
		{}

		// 3x3
		MUU_HIDDEN_CONSTRAINT((R == 3 && C == 3), size_t R = Rows, size_t C = Columns)
		MUU_NODISCARD_CTOR
		constexpr matrix(scalar_type v00,
						 scalar_type v01,
						 scalar_type v02 = scalar_type{},
						 scalar_type v10 = scalar_type{},
						 scalar_type v11 = scalar_type{},
						 scalar_type v12 = scalar_type{},
						 scalar_type v20 = scalar_type{},
						 scalar_type v21 = scalar_type{},
						 scalar_type v22 = scalar_type{}) noexcept //
			: base{ v00, v01, v02, v10, v11, v12, v20, v21, v22 }
		{}

		// 3x4
		MUU_HIDDEN_CONSTRAINT((R == 3 && C == 4), size_t R = Rows, size_t C = Columns)
		MUU_NODISCARD_CTOR
		constexpr matrix(scalar_type v00,
						 scalar_type v01,
						 scalar_type v02 = scalar_type{},
						 scalar_type v03 = scalar_type{},
						 scalar_type v10 = scalar_type{},
						 scalar_type v11 = scalar_type{},
						 scalar_type v12 = scalar_type{},
						 scalar_type v13 = scalar_type{},
						 scalar_type v20 = scalar_type{},
						 scalar_type v21 = scalar_type{},
						 scalar_type v22 = scalar_type{},
						 scalar_type v23 = scalar_type{}) noexcept //
			: base{ v00, v01, v02, v03, v10, v11, v12, v13, v20, v21, v22, v23 }
		{}

		// 4x4
		MUU_HIDDEN_CONSTRAINT((R == 4 && C == 4), size_t R = Rows, size_t C = Columns)
		MUU_NODISCARD_CTOR
		constexpr matrix(scalar_type v00,
						 scalar_type v01,
						 scalar_type v02 = scalar_type{},
						 scalar_type v03 = scalar_type{},
						 scalar_type v10 = scalar_type{},
						 scalar_type v11 = scalar_type{},
						 scalar_type v12 = scalar_type{},
						 scalar_type v13 = scalar_type{},
						 scalar_type v20 = scalar_type{},
						 scalar_type v21 = scalar_type{},
						 scalar_type v22 = scalar_type{},
						 scalar_type v23 = scalar_type{},
						 scalar_type v30 = scalar_type{},
						 scalar_type v31 = scalar_type{},
						 scalar_type v32 = scalar_type{},
						 scalar_type v33 = scalar_type{}) noexcept //
			: base{ v00, v01, v02, v03, v10, v11, v12, v13, v20, v21, v22, v23, v30, v31, v32, v33 }
		{}

		/// \endcond

		/// \brief Enlarging/truncating/converting constructor.
		/// \details Copies source matrix's scalar components, casting if necessary:
		/// \cpp
		/// matrix<int, 3, 3> m33 = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		/// std::cout << m33 << "\n\n";
		///
		/// matrix<int, 2, 2> m22 = { m33 };
		/// std::cout << m22 << "\n\n";
		///
		/// matrix<int, 4, 4> m44 = { m33 };
		/// std::cout << m44 << "\n\n";
		/// \ecpp
		///
		/// \out
		/// {    1,    2,    3,
		///      4,    5,    6,
		///      7,    8,    9 }
		///
		/// {    1,    2,
		///      4,    5 }
		///
		/// {    1,    2,    3,    0,
		///      4,    5,    6,    0,
		///      7,    8,    9,    0,
		///      0,    0,    0,    0 }
		/// \eout
		///
		/// Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \tparam	S		Source matrix's #scalar_type.
		/// \tparam	R		Source matrix's row count.
		/// \tparam	C		Source matrix's column count.
		/// \param 	mat		Source matrix.
		template <typename S, size_t R, size_t C>
		MUU_NODISCARD_CTOR
		explicit constexpr matrix(const matrix<S, R, C>& mat) noexcept
			: base{ impl::columnwise_copy_tag{}, std::make_index_sequence<(C < Columns ? C : Columns)>{}, mat.m }
		{}

		/// \brief Constructs a matrix from an implicitly bit-castable type.
		///
		/// \tparam T	A bit-castable type.
		///
		/// \see muu::allow_implicit_bit_cast
		MUU_CONSTRAINED_TEMPLATE((allow_implicit_bit_cast<T, matrix> && !impl::is_matrix_<T>), typename T)
		MUU_NODISCARD_CTOR
		/*implicit*/
		constexpr matrix(const T& obj) noexcept //
			: base{ muu::bit_cast<base>(obj) }
		{
			static_assert(sizeof(T) == sizeof(base), "Bit-castable types must be the same size");
			static_assert(std::is_trivially_copyable_v<T>, "Bit-castable types must be trivially-copyable");
		}

		/// \brief Constructs a matrix from a series of column vectors.
		/// \details Copies from the input columns, enlarging/truncating/casting as necessary:
		/// \cpp
		/// matrix<int, 3, 3> m33{ vector{ 1, 2, 3 }, vector{ 4, 5, 6 }, vector{ 7, 8, 9 } };
		/// std::cout << m33 << "\n\n";
		///
		/// matrix<int, 2, 2> m22{ vector{ 1, 2, 3 }, vector{ 4, 5, 6 } };
		/// std::cout << m22 << "\n\n";
		/// \ecpp
		///
		/// \out
		/// {    1,    4,    7,
		///      2,    5,    8,
		///      3,    6,    9 }
		///
		/// {    1,    4,
		///      2,    5 }
		/// \eout
		///
		/// Any scalar components not covered by the constructor's parameters are initialized to zero.
		///
		/// \tparam	S0		#scalar_type of the first column's initializer.
		/// \tparam	D0		Number of dimensions in the first column's initializer.
		/// \tparam	S1		#scalar_type of the second column's initializer.
		/// \tparam	D1		Number of dimensions in the second column's initializer.
		/// \tparam	Cols	Any additional column initializers.
		///
		/// \availability		This constructor is only available when #columns &gt;= 2.
		MUU_CONSTRAINED_TEMPLATE(((2u + sizeof...(Cols)) <= Columns												   //
								  && !all_same<column_type, vector<S0, D0>, vector<S1, D1>, remove_cvref<Cols>...> //
								  && all_convertible_to<column_type, column_type, const Cols&...>),
								 typename S0,
								 size_t D0,
								 typename S1,
								 size_t D1,
								 typename... Cols)
		MUU_NODISCARD_CTOR
		explicit constexpr matrix(const vector<S0, D0>& col0, const vector<S1, D1>& col1, const Cols&... cols) noexcept
			: base{ impl::columnwise_init_tag{}, column_type{ col0 }, column_type{ col1 }, column_type{ cols }... }
		{}

		/// \cond

		// non-converting case

		MUU_CONSTRAINED_TEMPLATE(((2u + sizeof...(Cols)) <= Columns //
								  && all_same<column_type, vector<S0, D0>, vector<S1, D1>, remove_cvref<Cols>...>),
								 typename S0,
								 size_t D0,
								 typename S1,
								 size_t D1,
								 typename... Cols)
		MUU_NODISCARD_CTOR
		explicit constexpr matrix(const vector<S0, D0>& col0, const vector<S1, D1>& col1, const Cols&... cols) noexcept
			: base{ impl::columnwise_init_tag{}, col0, col1, cols... }
		{}

		/// \endcond

#endif	// constructors

#if 1	// scaling ----------------------------------------------------------------------------------------------
		/// \name Scaling
		/// \availability	These functions are only available when the matrix is a typical 2D or 3D scale matrix
		///					(i.e. one of 2x2, 2x3 or 3x3, 3x4 or 4x4).
		/// @{

	#if MUU_DOXYGEN

		/// \brief Creates a scale matrix from x and y scalar components.
		///
		/// \availability	This function is only available when the matrix is 2x2, 2x3, 3x3, 3x4 or 4x4.
		static constexpr matrix from_2d_scale(scalar_type x, scalar_type y) noexcept;

		/// \brief Creates a scale matrix from the x and y components of a vector of arbitrary size.
		///
		/// \availability	This function is only available when the matrix is 2x2, 2x3, 3x3, 3x4 or 4x4.
		template <size_t D>
		static constexpr matrix from_2d_scale(const vector<scalar_type, D>& scale) noexcept;

		/// \brief Creates a scale matrix from x, y and z scalar components.
		///
		/// \availability	This function is only available when the matrix is 3x3, 3x4 or 4x4.
		static constexpr matrix from_3d_scale(scalar_type x, scalar_type y, scalar_type z = Scalar{ 1 }) noexcept;

		/// \brief Creates a scale matrix from the x, y and z components of a vector of arbitrary size.
		///
		/// \availability	This function is only available when the matrix is 3x3, 3x4 or 4x4.
		template <size_t D>
		static constexpr matrix from_3d_scale(const vector<scalar_type, D>& scale) noexcept;

		/// \brief Creates a scale matrix from x and y scalar components.
		///
		/// \availability	This function is only available when the matrix is 2x2, 2x3, 3x3, 3x4 or 4x4.
		static constexpr matrix from_scale(scalar_type x, scalar_type y) noexcept;

		/// \brief Creates a scale matrix from x, y and z scalar components.
		///
		/// \availability	This function is only available when the matrix is 3x3, 3x4 or 4x4.
		static constexpr matrix from_scale(scalar_type x, scalar_type y, scalar_type z) noexcept;

		/// \brief Creates a scale matrix from a 2D vector.
		///
		/// \availability	This overload is only available when the matrix is 2x2, 2x3, 3x3, 3x4 or 4x4.
		static constexpr matrix from_scale(const vector<scalar_type, 2>& scale) noexcept;

		/// \brief Creates a scale matrix from a 3D vector.
		///
		/// \availability	This overload is only available when the matrix is 3x3, 3x4 or 4x4.
		static constexpr matrix from_scale(const vector<scalar_type, 3>& scale) noexcept;

	#endif // DOXYGEN

		   /// @}
#endif	// scaling

#if 1	// translation ----------------------------------------------------------------------------------------------
		/// \name Translation
		/// \availability	These functions are only available when the matrix is a typical 2D or 3D translation
		///					matrix (i.e. one of 2x3, 3x3, 3x4 or 4x4).
		/// @{

	#if MUU_DOXYGEN

		/// \brief Creates a translation matrix from x and y scalar components.
		///
		/// \availability	This function is only available when the matrix is 2x3, 3x3, 3x4 or 4x4.
		static constexpr matrix from_2d_translation(scalar_type x, scalar_type y) noexcept;

		/// \brief Creates a translation matrix from the x and y components of a vector of arbitrary size.
		///
		/// \availability	This function is only available when the matrix is 2x3, 3x3, 3x4 or 4x4.
		template <size_t D>
		static constexpr matrix from_2d_translation(const vector<scalar_type, D>& xlat) noexcept;

		/// \brief Creates a translation matrix from x, y and z scalar components.
		///
		/// \availability	This function is only available when the matrix is 3x4 or 4x4.
		static constexpr matrix from_3d_translation(scalar_type x, scalar_type y, scalar_type z = Scalar{}) noexcept;

		/// \brief Creates a translation matrix from the x, y and z components of a vector of arbitrary size.
		///
		/// \availability	This function is only available when the matrix is 3x4 or 4x4.
		template <size_t D>
		static constexpr matrix from_3d_translation(const vector<scalar_type, D>& xlat) noexcept;

		/// \brief Creates a translation matrix from x and y scalar components.
		///
		/// \availability	This function is only available when the matrix is 2x3, 3x3, 3x4 or 4x4.
		static constexpr matrix from_translation(scalar_type x, scalar_type y) noexcept;

		/// \brief Creates a translation matrix from x, y and z scalar components.
		///
		/// \availability	This function is only available when the matrix is 3x4 or 4x4.
		static constexpr matrix from_translation(scalar_type x, scalar_type y, scalar_type z) noexcept;

		/// \brief Creates a translation matrix from a 2D vector.
		///
		/// \availability	This overload is only available when the matrix is 2x3, 3x3, 3x4 or 4x4.
		static constexpr matrix from_translation(const vector<scalar_type, 2>& xlat) noexcept;

		/// \brief Creates a translation matrix from a 3D vector.
		///
		/// \availability	This overload is only available when the matrix is 3x4 or 4x4.
		static constexpr matrix from_translation(const vector<scalar_type, 3>& xlat) noexcept;

	#endif // DOXYGEN

		   /// @}
#endif	// translation

#if 1	// rotation -------------------------------------------------------------------------------------------------
		/// \name Rotation
		/// \availability	These functions are only available when the matrix is a typical 2D or 3D rotation matrix
		///					(i.e. one of 2x2, 2x3, 3x3, 3x4 or 4x4) and has a floating-point #scalar_type.
		/// @{

	#if MUU_DOXYGEN

		/// \brief Creates a 2D rotation matrix from the lower 2x2 part of a matrix.
		///
		/// \availability	This function is only available #scalar_type is a floating-point type
		///					and the destination matrix is 2x2, 2x3 or 3x3.
		template <size_t R, size_t C>
		static constexpr matrix from_2d_rotation(const matrix<scalar_type, R, C>& rot) noexcept;

		/// \brief Creates a 2D rotation matrix from a 2x2 matrix.
		///
		/// \availability	This function is only available #scalar_type is a floating-point type
		///					and the destination matrix is 2x2 or 2x3.
		static constexpr matrix from_rotation(const matrix<scalar_type, 2, 2>& rot) noexcept;

		/// \brief Creates a 2D rotation matrix from a 2x3 matrix.
		///
		/// \availability	This function is only available #scalar_type is a floating-point type
		///					and the destination matrix is 2x2 or 2x3.
		static constexpr matrix from_rotation(const matrix<scalar_type, 2, 3>& rot) noexcept;

		/// \brief Creates a 3D rotation matrix from the lower 3x3 part of a matrix.
		///
		/// \availability	This function is only available #scalar_type is a floating-point type
		///					and the destination matrix is 3x4 or 4x4.
		template <size_t R, size_t C>
		static constexpr matrix from_3d_rotation(const matrix<scalar_type, R, C>& rot) noexcept;

		/// \brief Creates a 3D rotation matrix from a forward direction vector.
		///
		/// \availability	This function is only available #scalar_type is a floating-point type
		///					and the destination matrix is 3x4 or 4x4.
		static constexpr matrix from_3d_direction(const vector<scalar_type, 3>& dir) noexcept;

		/// \brief Creates a 3D rotation matrix from a 3x3 matrix.
		///
		/// \availability	This function is only available #scalar_type is a floating-point type
		///					and the destination matrix is 3x3, 3x4 or 4x4.
		static constexpr matrix from_rotation(const matrix<scalar_type, 3, 3>& rot) noexcept;

		/// \brief Creates a 3D rotation matrix from a 3x4 matrix.
		///
		/// \availability	This function is only available #scalar_type is a floating-point type
		///					and the destination matrix is 3x3, 3x4 or 4x4.
		static constexpr matrix from_rotation(const matrix<scalar_type, 3, 4>& rot) noexcept;

		/// \brief Creates a 3D rotation matrix from a 4x4 matrix.
		///
		/// \availability	This function is only available #scalar_type is a floating-point type
		///					and the destination matrix is 3x3, 3x4 or 4x4.
		static constexpr matrix from_rotation(const matrix<scalar_type, 4, 4>& rot) noexcept;

		/// \brief Creates a 3D rotation matrix from a quaternion
		///
		/// \availability	This function is only available #scalar_type is a floating-point type
		///					and the destination matrix is 3x4 or 4x4.
		static constexpr matrix from_quaternion(const quaternion_type& quat) noexcept;

		/// \brief Creates a 3D rotation matrix from an axis and an angle.
		///
		/// \availability	This function is only available #scalar_type is a floating-point type
		///					and the destination matrix is 3x4 or 4x4.
		static constexpr matrix from_axis_angle(const vector<scalar_type, 3>& axis, scalar_type angle) noexcept;

		/// \brief Creates a 3D rotation matrix from an axis and an angle.
		///
		/// \availability	This function is only available #scalar_type is a floating-point type
		///					and the destination matrix is 3x4 or 4x4.
		static constexpr matrix from_axis_angle(const axis_angle_type& aa) noexcept;

		/// \brief Creates a 3D rotation matrix from a set of euler angles.
		///
		/// \availability	This function is only available #scalar_type is a floating-point type
		///					and the destination matrix is 3x4 or 4x4.
		static constexpr matrix from_euler(scalar_type yaw, scalar_type pitch, scalar_type roll) noexcept;

		/// \brief Creates a 3D rotation matrix from a set of euler angles.
		///
		/// \availability	This function is only available #scalar_type is a floating-point type
		///					and the destination matrix is 3x4 or 4x4.
		static constexpr matrix from_euler(const euler_type& angles) noexcept;

		/// \brief Creates a 2D rotation matrix from x and y axis vectors.
		///
		/// \availability	This function is only available #scalar_type is a floating-point type
		///					and the destination matrix is 2x2 or 2x3.
		static constexpr matrix from_axes(const vector<Scalar, 2>& x, const vector<Scalar, 2>& y) noexcept;

		/// \brief Creates a 3D rotation matrix from x, y and z axis vectors.
		///
		/// \availability	This function is only available #scalar_type is a floating-point type
		///					and the destination matrix is 3x3, 3x4 or 4x4.
		static constexpr matrix from_axes(const vector<Scalar, 3>& x,
										  const vector<Scalar, 3>& y,
										  const vector<Scalar, 3>& z) noexcept;

		/// \brief Returns true if a 2D rotation matrix contains shear.
		///
		/// \availability	This function is only available #scalar_type is a floating-point type
		///					and the destination matrix is 2x2, 2x3 or 3x3.
		constexpr bool has_2d_shear() const noexcept;

		/// \brief Returns true if a 3D rotation matrix contains shear.
		///
		/// \availability	This function is only available #scalar_type is a floating-point type
		///					and the destination matrix is 3x3, 3x4 or 4x4.
		constexpr bool has_3d_shear() const noexcept;

	#endif // DOXYGEN

		   /// @}
#endif	// rotation

#if 1	// projection -------------------------------------------------------------------------------------------------
		/// \name Projection
		/// \availability	These functions are only available when the matrix is 4x4 and has a floating-point #scalar_type.
		/// @{

	#if MUU_DOXYGEN

		/// \brief Creates a perspective projection matrix.
		///
		/// \tparam Mode	Select the combination of flags for your math/graphics API.
		///
		/// \param vertical_fov		Vertical field-of-view in radians.
		/// \param aspect_ratio		Screen aspect ratio (`width / height`).
		/// \param near_clip		The camera's near-clip plane distance.
		/// \param far_clip			The camera's far-clip plane distance.
		///
		/// \availability	This function is only available when the matrix is 4x4 and has a floating-point #scalar_type.
		template <math_apis Mode = (math_apis::directx | math_apis::right_handed)>
		static constexpr matrix perspective_projection(scalar_type vertical_fov,
													   scalar_type aspect_ratio,
													   scalar_type near_clip,
													   scalar_type far_clip) noexcept;

		/// \brief Creates a perspective projection matrix.
		/// \tparam Mode	Select the combination of flags for your math/graphics API.
		///
		/// \param vertical_fov		Vertical field-of-view in radians.
		/// \param screen_size		The screen's size (width + height).
		/// \param near_clip		The camera's near-clip plane distance.
		/// \param far_clip			The camera's far-clip plane distance.
		///
		/// \availability	This function is only available when the matrix is 4x4 and has a floating-point #scalar_type.
		template <math_apis Mode = (math_apis::directx | math_apis::right_handed)>
		static constexpr matrix perspective_projection(scalar_type vertical_fov,
													   const vector<scalar_type, 2>& screen_size,
													   scalar_type near_clip,
													   scalar_type far_clip) noexcept;

	#endif // DOXYGEN

		   /// @}
#endif // projection

#if 1  // column accessors ------------------------------------------------------------------------------------------
		/// \name Column accessors
		/// @{

		/// \brief Gets a reference to a specific column.
		template <size_t C>
		MUU_PURE_INLINE_GETTER
		constexpr column_type& column() noexcept
		{
			static_assert(C < Columns, "Column index out of range");

			return base::m[C];
		}

		/// \brief Gets a const reference to a specific column.
		template <size_t C>
		MUU_PURE_INLINE_GETTER
		constexpr const column_type& column() const noexcept
		{
			static_assert(C < Columns, "Column index out of range");

			return base::m[C];
		}

	#if MUU_DOXYGEN

		/// \brief Returns a reference to the X column (column 0).
		///
		/// \availability	This function is only available when the matrix has at least two columns.
		constexpr column_type& x_column() noexcept;

		/// \brief Returns a const reference to the X axis column (column 0).
		///
		/// \availability	This function is only available when the matrix has at least two columns.
		constexpr const column_type& x_column() const noexcept;

		/// \brief Returns a reference to the Y column (column 1).
		///
		/// \availability	This function is only available when the matrix has at least two columns.
		constexpr column_type& y_column() noexcept;

		/// \brief Returns a const reference to the Y column (column 1).
		///
		/// \availability	This function is only available when the matrix has at least two columns.
		constexpr const column_type& y_column() const noexcept;

		/// \brief Returns a reference to the Z column (column 2).
		///
		/// \availability	This function is only available when the matrix has at least three columns.
		constexpr column_type& z_column() noexcept;

		/// \brief Returns a const reference to the Z column (column 2).
		///
		/// \availability	This function is only available when the matrix has at least three columns.
		constexpr const column_type& z_column() const noexcept;

		/// \brief Returns a reference to the W column (column 3).
		///
		/// \availability	This function is only available when the matrix has at least four columns.
		constexpr column_type& w_column() noexcept;

		/// \brief Returns a const reference to the W column (column 3).
		///
		/// \availability	This function is only available when the matrix has at least four columns.
		constexpr const column_type& w_column() const noexcept;

		/// \brief Returns a reference to the Translation column.
		///
		/// \availability	This function is only available when the matrix is 2x3, 3x3, 3x4 or 4x4.
		constexpr column_type& translation_column() noexcept;

		/// \brief Returns a const reference to the Translation column.
		///
		/// \availability	This function is only available when the matrix is 2x3, 3x3, 3x4 or 4x4.
		constexpr const column_type& translation_column() const noexcept;

	#endif

		/// @}
#endif	// column accessors

#if 1	// scalar component accessors --------------------------------------------------------------------------------
		/// \name Scalar accessors
		/// @{

	  private:
		/// \cond

		template <size_t R, size_t C, typename T>
		MUU_PURE_INLINE_GETTER
		static constexpr auto& do_get_scalar(T& mat) noexcept
		{
			static_assert(R < Rows, "Row index out of range");
			static_assert(C < Columns, "Column index out of range");

			return mat.m[C].template get<R>();
		}

		template <typename T>
		MUU_PURE_GETTER
		static constexpr auto& do_lookup_operator(T& mat, size_t r, size_t c) noexcept
		{
			MUU_ASSUME(r < Rows);
			MUU_ASSUME(c < Columns);

			return mat.m[c][r];
		}

		/// \endcond

	  public:
		/// \brief Gets a reference to the scalar component at a specific row and column.
		///
		/// \tparam R	The row of the scalar component to retrieve.
		/// \tparam C	The column of the scalar component to retrieve.
		///
		/// \return  A reference to the selected scalar component.
		template <size_t R, size_t C>
		MUU_PURE_INLINE_GETTER
		constexpr const scalar_type& get() const noexcept
		{
			return do_get_scalar<R, C>(*this);
		}

		/// \brief Gets a reference to the scalar component at a specific row and column.
		///
		/// \tparam R	The row of the scalar component to retrieve.
		/// \tparam C	The column of the scalar component to retrieve.
		///
		/// \return  A reference to the selected scalar component.
		template <size_t R, size_t C>
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type& get() noexcept
		{
			return do_get_scalar<R, C>(*this);
		}

		/// \brief Gets a reference to the scalar component at a specific row and column.
		///
		/// \param r	The row of the scalar component to retrieve.
		/// \param c	The column of the scalar component to retrieve.
		///
		/// \return  A reference to the selected scalar component.
		MUU_PURE_GETTER
		constexpr const scalar_type& operator()(size_t r, size_t c) const noexcept
		{
			return do_lookup_operator(*this, r, c);
		}

		/// \brief Gets a reference to the scalar component at a specific row and column.
		///
		/// \param r	The row of the scalar component to retrieve.
		/// \param c	The column of the scalar component to retrieve.
		///
		/// \return  A reference to the selected scalar component.
		MUU_PURE_GETTER
		constexpr scalar_type& operator()(size_t r, size_t c) noexcept
		{
			return do_lookup_operator(*this, r, c);
		}

		/// \brief Returns a pointer to the first scalar component in the matrix.
		MUU_PURE_INLINE_GETTER
		constexpr const scalar_type* data() const noexcept
		{
			return &do_get_scalar<0, 0>(*this);
		}

		/// \brief Returns a pointer to the first scalar component in the matrix.
		MUU_PURE_INLINE_GETTER
		constexpr scalar_type* data() noexcept
		{
			return &do_get_scalar<0, 0>(*this);
		}

		/// @}
#endif // scalar component accessors

#if 1 // equality (exact) ----------------------------------------------------------------------------------------------
		/// \name Equality (exact)
		/// @{

		/// \brief		Returns true if two matrices are exactly equal.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		MUU_CONSTRAINED_TEMPLATE((impl::pass_vector_by_reference<matrix, matrix<T, Rows, Columns>>), typename T)
		MUU_PURE_GETTER
		friend constexpr bool operator==(const matrix& lhs, const matrix<T, rows, columns>& rhs) noexcept
		{
			for (size_t i = 0; i < columns; i++)
				if (lhs.m[i] != rhs.m[i])
					return false;
			return true;
		}

	#if !MUU_DOXYGEN

		MUU_CONSTRAINED_TEMPLATE((impl::pass_vector_by_value<matrix, matrix<T, Rows, Columns>>), typename T)
		MUU_CONST_GETTER
		friend constexpr bool MUU_VECTORCALL operator==(matrix lhs, matrix<T, rows, columns> rhs) noexcept
		{
			for (size_t i = 0; i < columns; i++)
				if (lhs.m[i] != rhs.m[i])
					return false;
			return true;
		}

	#endif // !MUU_DOXYGEN

		/// \brief	Returns true if two matrices are not exactly equal.
		///
		/// \remarks	This is an exact check;
		///				use #approx_equal() if you want an epsilon-based "near-enough" check.
		MUU_CONSTRAINED_TEMPLATE((impl::pass_vector_by_reference<matrix, matrix<T, Rows, Columns>>), typename T)
		MUU_PURE_GETTER
		friend constexpr bool operator!=(const matrix& lhs, const matrix<T, rows, columns>& rhs) noexcept
		{
			return !(lhs == rhs);
		}

	#if !MUU_DOXYGEN

		MUU_CONSTRAINED_TEMPLATE((impl::pass_vector_by_value<matrix, matrix<T, Rows, Columns>>), typename T)
		MUU_CONST_INLINE_GETTER
		friend constexpr bool operator!=(matrix lhs, matrix<T, rows, columns> rhs) noexcept
		{
			return !(lhs == rhs);
		}

	#endif // !MUU_DOXYGEN

		/// \brief	Returns true if all the scalar components of a matrix are exactly zero.
		///
		/// \remarks	This is an exact check;
		///				use #approx_zero() if you want an epsilon-based "near-enough" check.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL zero(MUU_VPARAM(matrix) m) noexcept
		{
			for (size_t i = 0; i < columns; i++)
				if (!column_type::zero(m.m[i]))
					return false;
			return true;
		}

		/// \brief	Returns true if all the scalar components of the matrix are exactly zero.
		///
		/// \remarks	This is an exact check;
		///				use #approx_zero() if you want an epsilon-based "near-enough" check.
		MUU_PURE_GETTER
		constexpr bool zero() const noexcept
		{
			return zero(*this);
		}

		/// \brief	Returns true if any of the scalar components of a matrix are infinity or NaN.
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL infinity_or_nan(MUU_VPARAM(matrix) m) noexcept
		{
			if constexpr (is_floating_point<scalar_type>)
			{
				for (size_t i = 0; i < columns; i++)
					if (column_type::infinity_or_nan(m.m[i]))
						return true;
				return false;
			}
			else
			{
				MUU_UNUSED(m);
				return false;
			}
		}

		/// \brief	Returns true if any of the scalar components of the matrix are infinity or NaN.
		MUU_PURE_GETTER
		constexpr bool infinity_or_nan() const noexcept
		{
			if constexpr (is_floating_point<scalar_type>)
				return infinity_or_nan(*this);
			else
				return false;
		}

		/// @}
#endif // equality (exact)

#if 1  // equality (approx) -------------------------------------------------------------------------------------
		/// \name Equality (approximate)
		/// @{

		/// \brief	Returns true if two matrices are approximately equal.
		///
		/// \availability	This function is only available when at least one of #scalar_type
		///					and `T` is a floating-point type.
		MUU_CONSTRAINED_TEMPLATE((any_floating_point<Scalar, T> //
								  && impl::pass_vector_by_reference<matrix, matrix<T, Rows, Columns>>),
								 typename T)
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_equal(
			const matrix& m1,
			const matrix<T, rows, columns>& m2,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) noexcept
		{
			for (size_t i = 0; i < columns; i++)
				if (!column_type::approx_equal(m1.m[i], m2.m[i], epsilon))
					return false;
			return true;
		}

	#if !MUU_DOXYGEN

		MUU_CONSTRAINED_TEMPLATE((any_floating_point<Scalar, T> //
								  && impl::pass_vector_by_value<matrix, matrix<T, Rows, Columns>>),
								 typename T)
		MUU_CONST_GETTER
		static constexpr bool MUU_VECTORCALL approx_equal(
			matrix m1,
			matrix<T, rows, columns> m2,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) noexcept
		{
			for (size_t i = 0; i < columns; i++)
				if (!column_type::approx_equal(m1.m[i], m2.m[i], epsilon))
					return false;
			return true;
		}

	#endif // !MUU_DOXYGEN

		/// \brief	Returns true if the matrix is approximately equal to another.
		///
		/// \availability	This function is only available when at least one of #scalar_type
		///					and `T` is a floating-point type.
		MUU_CONSTRAINED_TEMPLATE((any_floating_point<Scalar, T> //
								  && impl::pass_vector_by_reference<matrix<T, Rows, Columns>>),
								 typename T)
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL approx_equal(
			const matrix<T, rows, columns>& m,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) const noexcept
		{
			return approx_equal(*this, m, epsilon);
		}

	#if !MUU_DOXYGEN

		MUU_CONSTRAINED_TEMPLATE((any_floating_point<Scalar, T> //
								  && impl::pass_vector_by_value<matrix<T, Rows, Columns>>),
								 typename T)
		MUU_CONST_GETTER
		constexpr bool MUU_VECTORCALL approx_equal(
			matrix<T, rows, columns> m,
			epsilon_type<scalar_type, T> epsilon = default_epsilon<scalar_type, T>) const noexcept
		{
			return approx_equal(*this, m, epsilon);
		}

	#endif // !MUU_DOXYGEN

		/// \brief	Returns true if all the scalar components in a matrix are approximately equal to zero.
		///
		/// \availability	This function is only available when #scalar_type is a floating-point type.
		MUU_HIDDEN_CONSTRAINT(is_floating_point<T>, typename T = Scalar)
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL approx_zero(MUU_VPARAM(matrix) m,
														 scalar_type epsilon = default_epsilon<scalar_type>) noexcept
		{
			for (size_t i = 0; i < columns; i++)
				if (!column_type::approx_zero(m.m[i], epsilon))
					return false;
			return true;
		}

		/// \brief	Returns true if all the scalar components in the matrix are approximately equal to zero.
		///
		/// \availability	This function is only available when #scalar_type is a floating-point type.
		MUU_HIDDEN_CONSTRAINT(is_floating_point<T>, typename T = Scalar)
		MUU_PURE_GETTER
		constexpr bool MUU_VECTORCALL approx_zero(scalar_type epsilon = default_epsilon<scalar_type>) const noexcept
		{
			return approx_zero(*this, epsilon);
		}

		/// @}
#endif // equality (approx)

#if 1  // addition --------------------------------------------------------------------------------------------------
		/// \name Addition
		/// @{

		/// \brief Returns the componentwise addition of two matrices.
		MUU_PURE_GETTER
		friend constexpr matrix MUU_VECTORCALL operator+(MUU_VPARAM(matrix) lhs, MUU_VPARAM(matrix) rhs) noexcept
		{
			matrix out{ lhs };
			for (size_t i = 0; i < columns; i++)
				out.m[i] += rhs.m[i];
			return out;
		}

		/// \brief Componentwise adds another matrix to this one.
		constexpr matrix& MUU_VECTORCALL operator+=(MUU_VPARAM(matrix) rhs) noexcept
		{
			for (size_t i = 0; i < columns; i++)
				base::m[i] += rhs.m[i];
			return *this;
		}

		/// \brief Returns a componentwise copy of a matrix.
		MUU_PURE_GETTER
		constexpr matrix operator+() const noexcept
		{
			return *this;
		}

		/// @}
#endif // addition

#if 1  // subtraction -----------------------------------------------------------------------------------------------
		/// \name Subtraction
		/// @{

		/// \brief Returns the componentwise subtraction of two matrices.
		MUU_PURE_GETTER
		friend constexpr matrix MUU_VECTORCALL operator-(MUU_VPARAM(matrix) lhs, MUU_VPARAM(matrix) rhs) noexcept
		{
			matrix out{ lhs };
			for (size_t i = 0; i < columns; i++)
				out.m[i] -= rhs.m[i];
			return out;
		}

		/// \brief Componentwise subtracts another matrix from this one.
		constexpr matrix& MUU_VECTORCALL operator-=(MUU_VPARAM(matrix) rhs) noexcept
		{
			for (size_t i = 0; i < columns; i++)
				base::m[i] -= rhs.m[i];
			return *this;
		}

		/// \brief Returns a componentwise negation of a matrix.
		MUU_HIDDEN_CONSTRAINT(is_signed<T>, typename T = Scalar)
		MUU_PURE_GETTER
		constexpr matrix operator-() const noexcept
		{
			matrix out{ *this };
			for (size_t i = 0; i < columns; i++)
				out.m[i] = -out.m[i];
			return out;
		}

		/// @}
#endif // subtraction

#if 1  // multiplication --------------------------------------------------------------------------------------------
		/// \name Multiplication
		/// @{

		/// \brief Multiplies two matrices.
		///
		/// \tparam C The number of columns in the RHS matrix.
		/// \param lhs  The LHS matrix.
		/// \param rhs  The RHS matrix.
		///
		/// \return  The result of `lhs * rhs`.
		template <size_t C>
		MUU_PURE_GETTER
		friend constexpr matrix<scalar_type, rows, C> MUU_VECTORCALL operator*(
			MUU_VPARAM(matrix) lhs,
			const matrix<scalar_type, columns, C>& rhs) noexcept
		{
			using result_type = matrix<scalar_type, Rows, C>;

			if constexpr (product_requires_promotion)
			{
				return result_type{ promoted_product_mat{ lhs } * matrix<promoted_product, columns, C>{ rhs } };
			}
			else
			{
				MUU_FMA_BLOCK;

	#define MULT_DOT(row, col, idx) lhs.m[idx].template get<row>() * rhs.m[col].template get<idx>()

				// common square cases are manually unrolled
				if constexpr (Rows == 2 && Columns == 2 && C == 2)
				{
					return result_type{
						column_type{ MULT_DOT(0, 0, 0) + MULT_DOT(0, 0, 1), MULT_DOT(1, 0, 0) + MULT_DOT(1, 0, 1) },
						column_type{ MULT_DOT(0, 1, 0) + MULT_DOT(0, 1, 1), MULT_DOT(1, 1, 0) + MULT_DOT(1, 1, 1) }
					};
				}
				else if constexpr (Rows == 3 && Columns == 3 && C == 3)
				{
					return result_type{ column_type{ MULT_DOT(0, 0, 0) + MULT_DOT(0, 0, 1) + MULT_DOT(0, 0, 2),
													 MULT_DOT(1, 0, 0) + MULT_DOT(1, 0, 1) + MULT_DOT(1, 0, 2),
													 MULT_DOT(2, 0, 0) + MULT_DOT(2, 0, 1) + MULT_DOT(2, 0, 2) },
										column_type{ MULT_DOT(0, 1, 0) + MULT_DOT(0, 1, 1) + MULT_DOT(0, 1, 2),
													 MULT_DOT(1, 1, 0) + MULT_DOT(1, 1, 1) + MULT_DOT(1, 1, 2),
													 MULT_DOT(2, 1, 0) + MULT_DOT(2, 1, 1) + MULT_DOT(2, 1, 2) },
										column_type{ MULT_DOT(0, 2, 0) + MULT_DOT(0, 2, 1) + MULT_DOT(0, 2, 2),
													 MULT_DOT(1, 2, 0) + MULT_DOT(1, 2, 1) + MULT_DOT(1, 2, 2),
													 MULT_DOT(2, 2, 0) + MULT_DOT(2, 2, 1) + MULT_DOT(2, 2, 2) }

					};
				}
				else if constexpr (Rows == 4 && Columns == 4 && C == 4)
				{
					return result_type{
						column_type{ MULT_DOT(0, 0, 0) + MULT_DOT(0, 0, 1) + MULT_DOT(0, 0, 2) + MULT_DOT(0, 0, 3),
									 MULT_DOT(1, 0, 0) + MULT_DOT(1, 0, 1) + MULT_DOT(1, 0, 2) + MULT_DOT(1, 0, 3),
									 MULT_DOT(2, 0, 0) + MULT_DOT(2, 0, 1) + MULT_DOT(2, 0, 2) + MULT_DOT(2, 0, 3),
									 MULT_DOT(3, 0, 0) + MULT_DOT(3, 0, 1) + MULT_DOT(3, 0, 2) + MULT_DOT(3, 0, 3) },
						column_type{ MULT_DOT(0, 1, 0) + MULT_DOT(0, 1, 1) + MULT_DOT(0, 1, 2) + MULT_DOT(0, 1, 3),
									 MULT_DOT(1, 1, 0) + MULT_DOT(1, 1, 1) + MULT_DOT(1, 1, 2) + MULT_DOT(1, 1, 3),
									 MULT_DOT(2, 1, 0) + MULT_DOT(2, 1, 1) + MULT_DOT(2, 1, 2) + MULT_DOT(2, 1, 3),
									 MULT_DOT(3, 1, 0) + MULT_DOT(3, 1, 1) + MULT_DOT(3, 1, 2) + MULT_DOT(3, 1, 3) },
						column_type{ MULT_DOT(0, 2, 0) + MULT_DOT(0, 2, 1) + MULT_DOT(0, 2, 2) + MULT_DOT(0, 2, 3),
									 MULT_DOT(1, 2, 0) + MULT_DOT(1, 2, 1) + MULT_DOT(1, 2, 2) + MULT_DOT(1, 2, 3),
									 MULT_DOT(2, 2, 0) + MULT_DOT(2, 2, 1) + MULT_DOT(2, 2, 2) + MULT_DOT(2, 2, 3),
									 MULT_DOT(3, 2, 0) + MULT_DOT(3, 2, 1) + MULT_DOT(3, 2, 2) + MULT_DOT(3, 2, 3) },
						column_type{ MULT_DOT(0, 3, 0) + MULT_DOT(0, 3, 1) + MULT_DOT(0, 3, 2) + MULT_DOT(0, 3, 3),
									 MULT_DOT(1, 3, 0) + MULT_DOT(1, 3, 1) + MULT_DOT(1, 3, 2) + MULT_DOT(1, 3, 3),
									 MULT_DOT(2, 3, 0) + MULT_DOT(2, 3, 1) + MULT_DOT(2, 3, 2) + MULT_DOT(2, 3, 3),
									 MULT_DOT(3, 3, 0) + MULT_DOT(3, 3, 1) + MULT_DOT(3, 3, 2) + MULT_DOT(3, 3, 3) }
					};
				}
				else
				{
					result_type out;
					for (size_t out_r = 0; out_r < Rows; out_r++)
					{
						for (size_t out_c = 0; out_c < C; out_c++)
						{
							auto val = lhs(out_r, 0) * rhs(0, out_c);
							for (size_t r = 1; r < Columns; r++)
								val += lhs(out_r, r) * rhs(r, out_c);

							out(out_r, out_c) = val;
						}
					}
					return out;
				}

	#undef MULT_DOT
			}
		}

		/// \brief Multiplies this matrix with another and assigns the result.
		///
		/// \availability	This function is only available when the matrix is square.
		MUU_HIDDEN_CONSTRAINT(R == C, size_t R = Rows, size_t C = Columns)
		constexpr matrix& MUU_VECTORCALL operator*=(MUU_VPARAM(matrix) rhs) noexcept
		{
			return *this = *this * rhs;
		}

		/// \brief Multiplies a matrix and a column vector.
		///
		/// \param lhs  The LHS matrix.
		/// \param rhs  The RHS column vector.
		///
		/// \return  The result of `lhs * rhs`.
		MUU_PURE_GETTER
		friend constexpr column_type MUU_VECTORCALL operator*(MUU_VPARAM(matrix) lhs,
															  MUU_VPARAM(vector<scalar_type, columns>) rhs) noexcept
		{
			if constexpr (product_requires_promotion)
			{
				return column_type{ promoted_product_mat{ lhs } * vector<promoted_product, columns>{ rhs } };
			}
			else
			{
				MUU_FMA_BLOCK;

	#define MULT_COL(row, col, vec_elem) lhs.m[col].template get<row>() * rhs.vec_elem

				// common square cases are manually unrolled
				if constexpr (Rows == 2 && Columns == 2)
				{
					return column_type{ MULT_COL(0, 0, x) + MULT_COL(0, 1, y), MULT_COL(1, 0, x) + MULT_COL(1, 1, y) };
				}
				else if constexpr (Rows == 3 && Columns == 3)
				{
					return column_type{ MULT_COL(0, 0, x) + MULT_COL(0, 1, y) + MULT_COL(0, 2, z),
										MULT_COL(1, 0, x) + MULT_COL(1, 1, y) + MULT_COL(1, 2, z),
										MULT_COL(2, 0, x) + MULT_COL(2, 1, y) + MULT_COL(2, 2, z) };
				}
				else if constexpr (Rows == 4 && Columns == 4)
				{
					return column_type{ MULT_COL(0, 0, x) + MULT_COL(0, 1, y) + MULT_COL(0, 2, z) + MULT_COL(0, 3, w),
										MULT_COL(1, 0, x) + MULT_COL(1, 1, y) + MULT_COL(1, 2, z) + MULT_COL(1, 3, w),
										MULT_COL(2, 0, x) + MULT_COL(2, 1, y) + MULT_COL(2, 2, z) + MULT_COL(2, 3, w),
										MULT_COL(3, 0, x) + MULT_COL(3, 1, y) + MULT_COL(3, 2, z) + MULT_COL(3, 3, w) };
				}
				else
				{
					column_type out;
					for (size_t out_r = 0; out_r < Rows; out_r++)
					{
						auto val = lhs(out_r, 0) * rhs.template get<0>();
						for (size_t c = 1; c < Columns; c++)
							val += lhs(out_r, c) * rhs[c];

						out[out_r] = val;
					}
					return out;
				}

	#undef MULT_COL
			}
		}

		/// \brief Multiplies a row vector and a matrix.
		///
		/// \param lhs  The LHS row vector.
		/// \param rhs  The RHS matrix.
		///
		/// \return  The result of `lhs * rhs`.
		MUU_PURE_GETTER
		friend constexpr row_type MUU_VECTORCALL operator*(MUU_VPARAM(vector<scalar_type, rows>) lhs,
														   MUU_VPARAM(matrix) rhs) noexcept
		{
			if constexpr (product_requires_promotion)
			{
				return row_type{ vector<promoted_product, rows>{ lhs } * promoted_product_mat{ rhs } };
			}
			else
			{
				MUU_FMA_BLOCK;

	#define MULT_ROW(row, col, vec_elem) lhs.vec_elem* rhs.template get<row, col>()

				// unroll the common square cases
				if constexpr (Rows == 2 && Columns == 2)
				{
					return row_type{ MULT_ROW(0, 0, x) + MULT_ROW(1, 0, y), MULT_ROW(0, 1, x) + MULT_ROW(1, 1, y) };
				}
				else if constexpr (Rows == 3 && Columns == 3)
				{
					return row_type{ MULT_ROW(0, 0, x) + MULT_ROW(1, 0, y) + MULT_ROW(2, 0, z),
									 MULT_ROW(0, 1, x) + MULT_ROW(1, 1, y) + MULT_ROW(2, 1, z),
									 MULT_ROW(0, 2, x) + MULT_ROW(1, 2, y) + MULT_ROW(2, 2, z) };
				}
				else if constexpr (Rows == 4 && Columns == 4)
				{
					return row_type{ MULT_ROW(0, 0, x) + MULT_ROW(1, 0, y) + MULT_ROW(2, 0, z) + MULT_ROW(3, 0, w),
									 MULT_ROW(0, 1, x) + MULT_ROW(1, 1, y) + MULT_ROW(2, 1, z) + MULT_ROW(3, 1, w),
									 MULT_ROW(0, 2, x) + MULT_ROW(1, 2, y) + MULT_ROW(2, 2, z) + MULT_ROW(3, 2, w),
									 MULT_ROW(0, 3, x) + MULT_ROW(1, 3, y) + MULT_ROW(2, 3, z) + MULT_ROW(3, 3, w) };
				}
				else
				{
					row_type out;
					for (size_t out_col = 0; out_col < Columns; out_col++)
					{
						auto val = lhs.template get<0>() * rhs(0, out_col);
						for (size_t r = 1; r < Rows; r++)
							val += lhs[r] * rhs(r, out_col);

						out[out_col] = val;
					}
					return out;
				}

	#undef MULT_ROW
			}
		}

		/// \brief Returns the componentwise multiplication of a matrix and a scalar.
		MUU_PURE_GETTER
		friend constexpr matrix MUU_VECTORCALL operator*(MUU_VPARAM(matrix) lhs, scalar_type rhs) noexcept
		{
			matrix out{ lhs };
			out *= rhs;
			return out;
		}

		/// \brief Returns the componentwise multiplication of a matrix and a scalar.
		MUU_PURE_GETTER
		friend constexpr matrix MUU_VECTORCALL operator*(scalar_type lhs, MUU_VPARAM(matrix) rhs) noexcept
		{
			return rhs * lhs;
		}

		/// \brief Componentwise multiplies this matrix by a scalar.
		constexpr matrix& MUU_VECTORCALL operator*=(scalar_type rhs) noexcept
		{
			for (auto& col : base::m)
				col *= rhs;
			return *this;
		}

		/// @}
#endif // multiplication

#if 1  // division --------------------------------------------------------------------------------------------------
		/// \name Division
		/// @{

		/// \brief Returns the componentwise multiplication of a matrix by a scalar.
		MUU_PURE_GETTER
		friend constexpr matrix MUU_VECTORCALL operator/(MUU_VPARAM(matrix) lhs, scalar_type rhs) noexcept
		{
			matrix out{ lhs };
			out /= rhs;
			return out;
		}

		/// \brief Componentwise multiplies this matrix by a scalar.
		constexpr matrix& MUU_VECTORCALL operator/=(scalar_type rhs) noexcept
		{
			if constexpr (is_small_float)
			{
				*this = matrix{ promoted_mat{ *this } * (promoted_scalar{ 1 } / static_cast<promoted_scalar>(rhs)) };
			}
			else if constexpr (is_floating_point<scalar_type>)
			{
				const auto div = scalar_type{ 1 } / rhs;
				for (auto& col : base::m)
					col *= div;
			}
			else
			{
				for (auto& col : base::m)
					col /= rhs;
			}
			return *this;
		}

		/// @}
#endif // division

#if 1  // transposition ---------------------------------------------------------------------------------------------
		/// \name Transposition
		/// @{

		/// \brief	Returns a transposed copy of a matrix.
		MUU_PURE_GETTER
		static constexpr matrix<scalar_type, columns, rows> MUU_VECTORCALL transpose(MUU_VPARAM(matrix) m) noexcept
		{
			using result_type	= matrix<scalar_type, columns, rows>;
			using result_column = vector<scalar_type, columns>;

	#define MAT_GET(r, c) m.m[c].template get<r>()

			// common square cases are manually unrolled
			if constexpr (Rows == 2 && Columns == 2)
			{
				return result_type{
					result_column{ MAT_GET(0, 0), MAT_GET(0, 1) },
					result_column{ MAT_GET(1, 0), MAT_GET(1, 1) },
				};
			}
			else if constexpr (Rows == 3 && Columns == 3)
			{
				return result_type{
					result_column{ MAT_GET(0, 0), MAT_GET(0, 1), MAT_GET(0, 2) },
					result_column{ MAT_GET(1, 0), MAT_GET(1, 1), MAT_GET(1, 2) },
					result_column{ MAT_GET(2, 0), MAT_GET(2, 1), MAT_GET(2, 2) },
				};
			}
			else if constexpr (Rows == 4 && Columns == 4)
			{
				return result_type{
					result_column{ MAT_GET(0, 0), MAT_GET(0, 1), MAT_GET(0, 2), MAT_GET(0, 3) },
					result_column{ MAT_GET(1, 0), MAT_GET(1, 1), MAT_GET(1, 2), MAT_GET(1, 3) },
					result_column{ MAT_GET(2, 0), MAT_GET(2, 1), MAT_GET(2, 2), MAT_GET(2, 3) },
					result_column{ MAT_GET(3, 0), MAT_GET(3, 1), MAT_GET(3, 2), MAT_GET(3, 3) },
				};
			}
			else
			{
				result_type out;
				for (size_t c = 0; c < Columns; c++)
				{
					auto& col = m.m[c];
					for (size_t r = 0; r < Rows; r++)
						out.m[r][c] = col[r];
				}
				return out;
			}

	#undef MAT_GET
		}

		/// \brief	Transposes the matrix (in-place).
		///
		/// \availability	This function is only available when the matrix is square.
		MUU_HIDDEN_CONSTRAINT(R == C, size_t R = Rows, size_t C = Columns)
		constexpr matrix& transpose() noexcept
		{
			return *this = transpose(*this);
		}

		/// @}
#endif // transposition

#if 1  // inverse & determinant -------------------------------------------------------------------------------------
		/// \name Inverse & Determinant
		/// \availability	These functions are only available when the matrix is square.
		/// @{

		/// \brief	Calculates the determinant of a matrix.
		///
		/// \availability	This function is only available when the matrix is square
		///					and has at most 4 rows and columns.
		MUU_HIDDEN_CONSTRAINT(R == C && C <= 4, size_t R = Rows, size_t C = Columns)
		MUU_PURE_GETTER
		static constexpr determinant_type MUU_VECTORCALL determinant(MUU_VPARAM(matrix) m) noexcept
		{
			if constexpr (Columns == 1)
				return static_cast<determinant_type>(m.m[0].x);
			if constexpr (Columns == 2)
				return static_cast<determinant_type>(impl::raw_determinant_2x2(m));
			if constexpr (Columns == 3)
				return static_cast<determinant_type>(impl::raw_determinant_3x3(m));
			if constexpr (Columns == 4)
				return static_cast<determinant_type>(impl::raw_determinant_4x4(m));
		}

		/// \brief	Calculates the determinant of a matrix.
		///
		/// \availability	This function is only available when the matrix is square
		///					and has at most 4 rows and columns.
		MUU_HIDDEN_CONSTRAINT(R == C && C <= 4, size_t R = Rows, size_t C = Columns)
		MUU_PURE_GETTER
		constexpr determinant_type determinant() noexcept
		{
			return determinant(*this);
		}

		/// \brief	Returns the inverse of a matrix.
		///
		/// \availability	This function is only available when the matrix is square
		///					and has at most 4 rows and columns.
		MUU_HIDDEN_CONSTRAINT(R == C && C <= 4, size_t R = Rows, size_t C = Columns)
		MUU_PURE_GETTER
		static constexpr inverse_type MUU_VECTORCALL invert(MUU_VPARAM(matrix) m) noexcept
		{
			if constexpr (inverse_requires_promotion)
			{
				return inverse_type{ promoted_inverse_mat::invert(promoted_inverse_mat{ m }) };
			}
			else
			{
				MUU_FMA_BLOCK;

	#define MAT_GET(r, c) m.m[c].template get<r>()

				if constexpr (Columns == 1)
				{
					return inverse_type{ column_type{ scalar_type{ 1 } / m.m[0].x } };
				}
				if constexpr (Columns == 2)
				{
					const auto det = scalar_type{ 1 } / impl::raw_determinant_2x2(m);
					return inverse_type{ column_type{ det * MAT_GET(1, 1), det * -MAT_GET(1, 0) },
										 column_type{ det * -MAT_GET(0, 1), det * MAT_GET(0, 0) } };
				}
				if constexpr (Columns == 3)
				{
					const auto det = scalar_type{ 1 } / impl::raw_determinant_3x3(m);
					return inverse_type{
						column_type{ det * (MAT_GET(1, 1) * MAT_GET(2, 2) - MAT_GET(1, 2) * MAT_GET(2, 1)),
									 det * -(MAT_GET(1, 0) * MAT_GET(2, 2) - MAT_GET(1, 2) * MAT_GET(2, 0)),
									 det * (MAT_GET(1, 0) * MAT_GET(2, 1) - MAT_GET(1, 1) * MAT_GET(2, 0)) },
						column_type{ det * -(MAT_GET(0, 1) * MAT_GET(2, 2) - MAT_GET(0, 2) * MAT_GET(2, 1)),
									 det * (MAT_GET(0, 0) * MAT_GET(2, 2) - MAT_GET(0, 2) * MAT_GET(2, 0)),
									 det * -(MAT_GET(0, 0) * MAT_GET(2, 1) - MAT_GET(0, 1) * MAT_GET(2, 0)) },
						column_type{ det * (MAT_GET(0, 1) * MAT_GET(1, 2) - MAT_GET(0, 2) * MAT_GET(1, 1)),
									 det * -(MAT_GET(0, 0) * MAT_GET(1, 2) - MAT_GET(0, 2) * MAT_GET(1, 0)),
									 det * (MAT_GET(0, 0) * MAT_GET(1, 1) - MAT_GET(0, 1) * MAT_GET(1, 0)) }
					};
				}
				if constexpr (Columns == 4)
				{
					// generated using https://github.com/willnode/N-Matrix-Programmer

					const auto A2323 = MAT_GET(2, 2) * MAT_GET(3, 3) - MAT_GET(2, 3) * MAT_GET(3, 2);
					const auto A1323 = MAT_GET(2, 1) * MAT_GET(3, 3) - MAT_GET(2, 3) * MAT_GET(3, 1);
					const auto A1223 = MAT_GET(2, 1) * MAT_GET(3, 2) - MAT_GET(2, 2) * MAT_GET(3, 1);
					const auto A0323 = MAT_GET(2, 0) * MAT_GET(3, 3) - MAT_GET(2, 3) * MAT_GET(3, 0);
					const auto A0223 = MAT_GET(2, 0) * MAT_GET(3, 2) - MAT_GET(2, 2) * MAT_GET(3, 0);
					const auto A0123 = MAT_GET(2, 0) * MAT_GET(3, 1) - MAT_GET(2, 1) * MAT_GET(3, 0);
					const auto A2313 = MAT_GET(1, 2) * MAT_GET(3, 3) - MAT_GET(1, 3) * MAT_GET(3, 2);
					const auto A1313 = MAT_GET(1, 1) * MAT_GET(3, 3) - MAT_GET(1, 3) * MAT_GET(3, 1);
					const auto A1213 = MAT_GET(1, 1) * MAT_GET(3, 2) - MAT_GET(1, 2) * MAT_GET(3, 1);
					const auto A2312 = MAT_GET(1, 2) * MAT_GET(2, 3) - MAT_GET(1, 3) * MAT_GET(2, 2);
					const auto A1312 = MAT_GET(1, 1) * MAT_GET(2, 3) - MAT_GET(1, 3) * MAT_GET(2, 1);
					const auto A1212 = MAT_GET(1, 1) * MAT_GET(2, 2) - MAT_GET(1, 2) * MAT_GET(2, 1);
					const auto A0313 = MAT_GET(1, 0) * MAT_GET(3, 3) - MAT_GET(1, 3) * MAT_GET(3, 0);
					const auto A0213 = MAT_GET(1, 0) * MAT_GET(3, 2) - MAT_GET(1, 2) * MAT_GET(3, 0);
					const auto A0312 = MAT_GET(1, 0) * MAT_GET(2, 3) - MAT_GET(1, 3) * MAT_GET(2, 0);
					const auto A0212 = MAT_GET(1, 0) * MAT_GET(2, 2) - MAT_GET(1, 2) * MAT_GET(2, 0);
					const auto A0113 = MAT_GET(1, 0) * MAT_GET(3, 1) - MAT_GET(1, 1) * MAT_GET(3, 0);
					const auto A0112 = MAT_GET(1, 0) * MAT_GET(2, 1) - MAT_GET(1, 1) * MAT_GET(2, 0);

					const auto det =
						scalar_type{ 1 }
						/ (MAT_GET(0, 0) * (MAT_GET(1, 1) * A2323 - MAT_GET(1, 2) * A1323 + MAT_GET(1, 3) * A1223)
						   - MAT_GET(0, 1) * (MAT_GET(1, 0) * A2323 - MAT_GET(1, 2) * A0323 + MAT_GET(1, 3) * A0223)
						   + MAT_GET(0, 2) * (MAT_GET(1, 0) * A1323 - MAT_GET(1, 1) * A0323 + MAT_GET(1, 3) * A0123)
						   - MAT_GET(0, 3) * (MAT_GET(1, 0) * A1223 - MAT_GET(1, 1) * A0223 + MAT_GET(1, 2) * A0123));

					return inverse_type{
						column_type{ //
									 det * (MAT_GET(1, 1) * A2323 - MAT_GET(1, 2) * A1323 + MAT_GET(1, 3) * A1223),
									 det * -(MAT_GET(1, 0) * A2323 - MAT_GET(1, 2) * A0323 + MAT_GET(1, 3) * A0223),
									 det * (MAT_GET(1, 0) * A1323 - MAT_GET(1, 1) * A0323 + MAT_GET(1, 3) * A0123),
									 det * -(MAT_GET(1, 0) * A1223 - MAT_GET(1, 1) * A0223 + MAT_GET(1, 2) * A0123) },
						column_type{ //
									 det * -(MAT_GET(0, 1) * A2323 - MAT_GET(0, 2) * A1323 + MAT_GET(0, 3) * A1223),
									 det * (MAT_GET(0, 0) * A2323 - MAT_GET(0, 2) * A0323 + MAT_GET(0, 3) * A0223),
									 det * -(MAT_GET(0, 0) * A1323 - MAT_GET(0, 1) * A0323 + MAT_GET(0, 3) * A0123),
									 det * (MAT_GET(0, 0) * A1223 - MAT_GET(0, 1) * A0223 + MAT_GET(0, 2) * A0123) },
						column_type{ //
									 det * (MAT_GET(0, 1) * A2313 - MAT_GET(0, 2) * A1313 + MAT_GET(0, 3) * A1213),
									 det * -(MAT_GET(0, 0) * A2313 - MAT_GET(0, 2) * A0313 + MAT_GET(0, 3) * A0213),
									 det * (MAT_GET(0, 0) * A1313 - MAT_GET(0, 1) * A0313 + MAT_GET(0, 3) * A0113),
									 det * -(MAT_GET(0, 0) * A1213 - MAT_GET(0, 1) * A0213 + MAT_GET(0, 2) * A0113) },
						column_type{ //
									 det * -(MAT_GET(0, 1) * A2312 - MAT_GET(0, 2) * A1312 + MAT_GET(0, 3) * A1212),
									 det * (MAT_GET(0, 0) * A2312 - MAT_GET(0, 2) * A0312 + MAT_GET(0, 3) * A0212),
									 det * -(MAT_GET(0, 0) * A1312 - MAT_GET(0, 1) * A0312 + MAT_GET(0, 3) * A0112),
									 det * (MAT_GET(0, 0) * A1212 - MAT_GET(0, 1) * A0212 + MAT_GET(0, 2) * A0112) }
					};
				}

	#undef MAT_GET
			}
		}

		/// \brief	Inverts the matrix (in-place).
		///
		/// \availability	This function is only available when the matrix is square,
		///					has at most 4 rows and columns, and has a floating-point #scalar_type.
		MUU_HIDDEN_CONSTRAINT((R == C && C <= 4 && is_floating_point<Scalar>), size_t R = Rows, size_t C = Columns)
		constexpr matrix& invert() noexcept
		{
			return *this = invert(*this);
		}

		/// @}
#endif	// inverse & determinant

#if 1	// orthonormalize --------------------------------------------------------------------------------------------
		/// \name Orthonormalization
		/// \availability	These functions are only available when the matrix is a typical 3D rotation matrix
		///					(i.e. one of 3x3, 3x4 or 4x4) and has a floating-point #scalar_type.
		/// @{

	#if MUU_DOXYGEN

		/// \brief	Orthonormalizes the 3x3 part of a rotation or transformation matrix.
		///
		/// \availability	This function is only available when the matrix is a typical 3D rotation matrix
		///					(i.e. one of 3x3, 3x4 or 4x4) and has a floating-point #scalar_type.
		///
		/// \see [Orthonormal basis](https://en.wikipedia.org/wiki/Orthonormal_basis)
		static constexpr matrix orthonormalize(const matrix&) noexcept;

		/// \brief	Orthonormalizes the 3x3 part of the matrix.
		///
		/// \availability	This function is only available when the matrix is a typical 3D rotation matrix
		///					(i.e. one of 3x3, 3x4 or 4x4) and has a floating-point #scalar_type.
		///
		/// \see [Orthonormal basis](https://en.wikipedia.org/wiki/Orthonormal_basis)
		constexpr matrix& orthonormalize() noexcept;

		/// \brief	Returns true if the 3x3 part of a matrix is orthonormalized.
		///
		/// \availability	This function is only available when the matrix is a typical 3D rotation matrix
		///					(i.e. one of 3x3, 3x4 or 4x4) and has a floating-point #scalar_type.
		///
		/// \see [Orthonormal basis](https://en.wikipedia.org/wiki/Orthonormal_basis)
		static constexpr bool orthonormalized(const matrix&) noexcept;

		/// \brief	Returns true if the 3x3 part of the matrix is orthonormalized.
		///
		/// \availability	This function is only available when the matrix is a typical 3D rotation matrix
		///					(i.e. one of 3x3, 3x4 or 4x4) and has a floating-point #scalar_type.
		///
		/// \see [Orthonormal basis](https://en.wikipedia.org/wiki/Orthonormal_basis)
		constexpr bool orthonormalized() const noexcept;

	#endif // DOXYGEN

		   /// @}
#endif	// orthonormalize

#if 1	// transformation ----------------------------------------------------------------------------------------
		/// \name Transformation
		/// \availability	These functions are only available when the matrix is a typical 2D or 3D transformation matrix
		///					(i.e. one of 2x2, 2x3, 3x3, 3x4 or 4x4) and has a floating-point #scalar_type.
		/// @{

	#if MUU_DOXYGEN

		/// \brief Applies a matrix's 3d transformation to the given vector.
		///
		/// \availability This operator is only available for 3x4 and 4x4 matrices with a floating-point #scalar_type.
		///
		/// \return The result of transforming the 3D position by the matrix.
		static constexpr vector<scalar_type, 3> transform_position(const matrix& xform,
																   const vector<scalar_type, 3>& pos) noexcept;

		/// \brief Applies the matrix's 3d transformation to the given vector.
		///
		/// \availability This operator is only available for 3x4 and 4x4 matrices with a floating-point #scalar_type.
		///
		/// \return The result of transforming the 3D position by the matrix.
		constexpr vector<scalar_type, 3> transform_position(const vector<scalar_type, 3>& pos) const noexcept;

		/// \brief Applies the matrix's 3d transformation to the vector.
		///
		/// \availability This operator is only available for 3x4 and 4x4 matrices with a floating-point #scalar_type.
		///
		/// \return The result of transforming the 3D position by the matrix.
		///
		/// \see #transform_position()
		friend constexpr vector<scalar_type, 3> operator*(const matrix& xform,
														  const vector<scalar_type, 3>& pos) noexcept;

		/// \brief Applies a matrix's 3d transformation to the given position or direction vector, ignorning translation.
		///
		/// \availability This operator is only available for 3x4 and 4x4 matrices with a floating-point #scalar_type.
		///
		/// \return The result of transforming the 3D vector by the matrix.
		static constexpr vector<scalar_type, 3> transform_without_translating(const matrix& xform,
																			  const vector<scalar_type, 3>& v) noexcept;

		/// \brief Applies a matrix's 3d transformation to the position or direction vector, ignorning translation.
		///
		/// \availability This operator is only available for 3x4 and 4x4 matrices with a floating-point #scalar_type.
		///
		/// \return The result of transforming the 3D vector by the matrix.
		constexpr vector<scalar_type, 3> transform_without_translating(const vector<scalar_type, 3>& v) const noexcept;

		/// \brief Applies a matrix's 3d transformation to the given direction vector, ignorning translation and scale.
		///
		/// \availability This operator is only available for 3x4 and 4x4 matrices with a floating-point #scalar_type.
		///
		/// \return The result of transforming the 3D direction by the matrix.
		static constexpr vector<scalar_type, 3> transform_direction(const matrix& xform,
																	const vector<scalar_type, 3>& dir) noexcept;

		/// \brief Applies a matrix's 3d transformation to the direction vector, ignorning translation and scale.
		///
		/// \availability This operator is only available for 3x4 and 4x4 matrices with a floating-point #scalar_type.
		///
		/// \return The result of transforming the 3D direction by the matrix.
		constexpr vector<scalar_type, 3> transform_direction(const vector<scalar_type, 3>& dir) const noexcept;

		/// \brief Extracts the scale from a 2D transform matrix.
		///
		/// \availability	This overload is only available when the matrix is 2x2, 2x3 or 3x3 with a floating-point #scalar_type.
		static constexpr vector<scalar_type, 2> extract_2d_scale(const matrix& mat) noexcept;

		/// \brief Extracts the scale from a 2D transform matrix.
		///
		/// \availability	This overload is only available when the matrix is 2x2, 2x3 or 3x3 with a floating-point #scalar_type.
		constexpr vector<scalar_type, 2> extract_2d_scale() const noexcept;

		/// \brief Extracts the scale from a 3D transform matrix.
		///
		/// \availability	This overload is only available when the matrix is 3x3, 3x4 or 4x4 with a floating-point #scalar_type.
		static constexpr vector<scalar_type, 3> extract_3d_scale(const matrix& mat) noexcept;

		/// \brief Extracts the scale from a 3D transform matrix.
		///
		/// \availability	This overload is only available when the matrix is 3x3, 3x4 or 4x4 with a floating-point #scalar_type.
		constexpr vector<scalar_type, 3> extract_3d_scale() const noexcept;

	#endif

		/// @}
#endif // transformation

#if 1  // misc ------------------------------------------------------------------------------------------------------

		/// \brief Writes a matrix out to a text stream.
		template <typename Char, typename Traits>
		friend std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os, const matrix& m)
		{
			impl::print_matrix(os, &m.get<0, 0>(), Rows, Columns);
			return os;
		}

#endif // misc
	};

	/// \cond

	MUU_CONSTRAINED_TEMPLATE(is_arithmetic<T>, typename T)
	matrix(T) -> matrix<std::remove_cv_t<T>, 1, 1>;

	MUU_CONSTRAINED_TEMPLATE((all_arithmetic<T1, T2, T3, T4>), typename T1, typename T2, typename T3, typename T4)
	matrix(T1, T2, T3, T4) -> matrix<impl::highest_ranked<T1, T2, T3, T4>, 2, 2>;

	MUU_CONSTRAINED_TEMPLATE((all_arithmetic<T1, T2, T3, T4, T5, T6>),
							 typename T1,
							 typename T2,
							 typename T3,
							 typename T4,
							 typename T5,
							 typename T6)
	matrix(T1, T2, T3, T4, T5, T6) -> matrix<impl::highest_ranked<T1, T2, T3, T4, T5, T6>, 2, 3>;

	MUU_CONSTRAINED_TEMPLATE((all_arithmetic<T1, T2, T3, T4, T5, T6, T7, T8, T9>),
							 typename T1,
							 typename T2,
							 typename T3,
							 typename T4,
							 typename T5,
							 typename T6,
							 typename T7,
							 typename T8,
							 typename T9)
	matrix(T1, T2, T3, T4, T5, T6, T7, T8, T9)
		-> matrix<impl::highest_ranked<T1, T2, T3, T4, T5, T6, T7, T8, T9>, 3, 3>;

	MUU_CONSTRAINED_TEMPLATE((all_arithmetic<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12>),
							 typename T1,
							 typename T2,
							 typename T3,
							 typename T4,
							 typename T5,
							 typename T6,
							 typename T7,
							 typename T8,
							 typename T9,
							 typename T10,
							 typename T11,
							 typename T12)
	matrix(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12)
		-> matrix<impl::highest_ranked<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12>, 3, 4>;

	MUU_CONSTRAINED_TEMPLATE((all_arithmetic<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16>),
							 typename T1,
							 typename T2,
							 typename T3,
							 typename T4,
							 typename T5,
							 typename T6,
							 typename T7,
							 typename T8,
							 typename T9,
							 typename T10,
							 typename T11,
							 typename T12,
							 typename T13,
							 typename T14,
							 typename T15,
							 typename T16)
	matrix(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16)
		-> matrix<impl::highest_ranked<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16>, 4, 4>;

	template <typename S0, size_t D0, typename S1, size_t D1>
	matrix(vector<S0, D0>, vector<S1, D1>) -> matrix<impl::highest_ranked<S0, S1>, muu::max(D0, D1), 2>;

	template <typename S0, size_t D0, typename S1, size_t D1, typename S2, size_t D2>
	matrix(vector<S0, D0>, vector<S1, D1>, vector<S2, D2>)
		-> matrix<impl::highest_ranked<S0, S1, S2>, muu::max(D0, D1, D2), 3>;

	template <typename S0, size_t D0, typename S1, size_t D1, typename S2, size_t D2, typename S3, size_t D3>
	matrix(vector<S0, D0>, vector<S1, D1>, vector<S2, D2>, vector<S3, D3>)
		-> matrix<impl::highest_ranked<S0, S1, S2, S3>, muu::max(D0, D1, D2, D3), 4>;

	/// \endcond
}

//======================================================================================================================
// CONSTANTS
//======================================================================================================================

namespace muu
{
	MUU_PUSH_PRECISE_MATH;

	namespace impl
	{
		/// \cond

		template <template <typename, size_t, size_t> typename Matrix, typename Scalar, size_t Rows, size_t Columns>
		MUU_NODISCARD
		MUU_CONSTEVAL
		Matrix<Scalar, Rows, Columns> make_identity_matrix() noexcept
		{
			using scalars = constants<Scalar>;
			Matrix<Scalar, Rows, Columns> m{ scalars::zero };
			m.template get<0, 0>() = scalars::one;
			if constexpr (Rows > 1 && Columns > 1)
				m.template get<1, 1>() = scalars::one;
			if constexpr (Rows > 2 && Columns > 2)
				m.template get<2, 2>() = scalars::one;
			if constexpr (Rows > 3 && Columns > 3)
				m.template get<3, 3>() = scalars::one;
			if constexpr (Rows > 4 && Columns > 4)
			{
				for (size_t i = 4; i < muu::min(Rows, Columns); i++)
					m(i, i) = scalars::one;
			}
			return m;
		}

		template <template <typename, size_t, size_t> typename Matrix, typename Scalar, size_t Rows, size_t Columns>
		MUU_NODISCARD
		MUU_CONSTEVAL
		Matrix<Scalar, Rows, Columns> make_rotation_matrix(Scalar xx,
														   Scalar yx,
														   Scalar zx,
														   Scalar xy,
														   Scalar yy,
														   Scalar zy,
														   Scalar xz,
														   Scalar yz,
														   Scalar zz) noexcept
		{
			auto m				   = make_identity_matrix<Matrix, Scalar, Rows, Columns>();
			m.template get<0, 0>() = xx;
			m.template get<1, 0>() = xy;
			m.template get<2, 0>() = xz;
			m.template get<0, 1>() = yx;
			m.template get<1, 1>() = yy;
			m.template get<2, 1>() = yz;
			m.template get<0, 2>() = zx;
			m.template get<1, 2>() = zy;
			m.template get<2, 2>() = zz;
			return m;
		}

		template <typename Scalar, size_t Rows, size_t Columns>
		struct integer_limits<matrix<Scalar, Rows, Columns>>
		{
			using type	  = matrix<Scalar, Rows, Columns>;
			using scalars = integer_limits<Scalar>;

			static constexpr type lowest  = type{ scalars::lowest };
			static constexpr type highest = type{ scalars::highest };
		};

		template <typename Scalar, size_t Rows, size_t Columns>
		struct integer_positive_constants<matrix<Scalar, Rows, Columns>>
		{
			using type	  = matrix<Scalar, Rows, Columns>;
			using scalars = integer_positive_constants<Scalar>;

			static constexpr type zero		  = type{ scalars::zero };
			static constexpr type one		  = type{ scalars::one };
			static constexpr type two		  = type{ scalars::two };
			static constexpr type three		  = type{ scalars::three };
			static constexpr type four		  = type{ scalars::four };
			static constexpr type five		  = type{ scalars::five };
			static constexpr type six		  = type{ scalars::six };
			static constexpr type seven		  = type{ scalars::seven };
			static constexpr type eight		  = type{ scalars::eight };
			static constexpr type nine		  = type{ scalars::nine };
			static constexpr type ten		  = type{ scalars::ten };
			static constexpr type one_hundred = type{ scalars::one_hundred };
		};

		template <typename Scalar, size_t Rows, size_t Columns>
		struct floating_point_traits<matrix<Scalar, Rows, Columns>> : floating_point_traits<Scalar>
		{};

		template <typename Scalar, size_t Rows, size_t Columns>
		struct floating_point_special_constants<matrix<Scalar, Rows, Columns>>
		{
			using type	  = matrix<Scalar, Rows, Columns>;
			using scalars = floating_point_special_constants<Scalar>;

			static constexpr type nan				= type{ scalars::nan };
			static constexpr type signaling_nan		= type{ scalars::signaling_nan };
			static constexpr type infinity			= type{ scalars::infinity };
			static constexpr type negative_infinity = type{ scalars::negative_infinity };
			static constexpr type negative_zero		= type{ scalars::negative_zero };
		};

		template <typename Scalar, size_t Rows, size_t Columns>
		struct floating_point_named_constants<matrix<Scalar, Rows, Columns>>
		{
			using type	  = matrix<Scalar, Rows, Columns>;
			using scalars = floating_point_named_constants<Scalar>;

			static constexpr type one_over_two		   = type{ scalars::one_over_two };
			static constexpr type two_over_three	   = type{ scalars::two_over_three };
			static constexpr type two_over_five		   = type{ scalars::two_over_five };
			static constexpr type sqrt_two			   = type{ scalars::sqrt_two };
			static constexpr type one_over_sqrt_two	   = type{ scalars::one_over_sqrt_two };
			static constexpr type one_over_three	   = type{ scalars::one_over_three };
			static constexpr type three_over_two	   = type{ scalars::three_over_two };
			static constexpr type three_over_four	   = type{ scalars::three_over_four };
			static constexpr type three_over_five	   = type{ scalars::three_over_five };
			static constexpr type sqrt_three		   = type{ scalars::sqrt_three };
			static constexpr type one_over_sqrt_three  = type{ scalars::one_over_sqrt_three };
			static constexpr type pi				   = type{ scalars::pi };
			static constexpr type one_over_pi		   = type{ scalars::one_over_pi };
			static constexpr type pi_over_two		   = type{ scalars::pi_over_two };
			static constexpr type pi_over_three		   = type{ scalars::pi_over_three };
			static constexpr type pi_over_four		   = type{ scalars::pi_over_four };
			static constexpr type pi_over_five		   = type{ scalars::pi_over_five };
			static constexpr type pi_over_six		   = type{ scalars::pi_over_six };
			static constexpr type pi_over_seven		   = type{ scalars::pi_over_seven };
			static constexpr type pi_over_eight		   = type{ scalars::pi_over_eight };
			static constexpr type sqrt_pi			   = type{ scalars::sqrt_pi };
			static constexpr type one_over_sqrt_pi	   = type{ scalars::one_over_sqrt_pi };
			static constexpr type two_pi			   = type{ scalars::two_pi };
			static constexpr type sqrt_two_pi		   = type{ scalars::sqrt_two_pi };
			static constexpr type one_over_sqrt_two_pi = type{ scalars::one_over_sqrt_two_pi };
			static constexpr type one_over_three_pi	   = type{ scalars::one_over_three_pi };
			static constexpr type three_pi_over_two	   = type{ scalars::three_pi_over_two };
			static constexpr type three_pi_over_four   = type{ scalars::three_pi_over_four };
			static constexpr type three_pi_over_five   = type{ scalars::three_pi_over_five };
			static constexpr type e					   = type{ scalars::e };
			static constexpr type one_over_e		   = type{ scalars::one_over_e };
			static constexpr type e_over_two		   = type{ scalars::e_over_two };
			static constexpr type e_over_three		   = type{ scalars::e_over_three };
			static constexpr type e_over_four		   = type{ scalars::e_over_four };
			static constexpr type e_over_five		   = type{ scalars::e_over_five };
			static constexpr type e_over_six		   = type{ scalars::e_over_six };
			static constexpr type sqrt_e			   = type{ scalars::sqrt_e };
			static constexpr type one_over_sqrt_e	   = type{ scalars::one_over_sqrt_e };
			static constexpr type phi				   = type{ scalars::phi };
			static constexpr type one_over_phi		   = type{ scalars::one_over_phi };
			static constexpr type phi_over_two		   = type{ scalars::phi_over_two };
			static constexpr type phi_over_three	   = type{ scalars::phi_over_three };
			static constexpr type phi_over_four		   = type{ scalars::phi_over_four };
			static constexpr type phi_over_five		   = type{ scalars::phi_over_five };
			static constexpr type phi_over_six		   = type{ scalars::phi_over_six };
			static constexpr type sqrt_phi			   = type{ scalars::sqrt_phi };
			static constexpr type one_over_sqrt_phi	   = type{ scalars::one_over_sqrt_phi };
			static constexpr type degrees_to_radians   = type{ scalars::degrees_to_radians };
			static constexpr type radians_to_degrees   = type{ scalars::radians_to_degrees };
		};

		template <typename Scalar,
				  size_t Rows,
				  size_t Columns,
				  int = (is_floating_point<Scalar> ? 2 : (is_signed<Scalar> ? 1 : 0))>
		struct matrix_constants_base : unsigned_integral_constants<matrix<Scalar, Rows, Columns>>
		{};
		template <typename Scalar, size_t Rows, size_t Columns>
		struct matrix_constants_base<Scalar, Rows, Columns, 1>
			: signed_integral_constants<matrix<Scalar, Rows, Columns>>
		{};
		template <typename Scalar, size_t Rows, size_t Columns>
		struct matrix_constants_base<Scalar, Rows, Columns, 2> : floating_point_constants<matrix<Scalar, Rows, Columns>>
		{};

		/// \endcond

		template <typename Scalar,
				  size_t Rows,
				  size_t Columns SPECIALIZED_IF(Rows >= 3 && Rows <= 4 && Columns >= 3 && Columns <= 4
												&& is_signed<Scalar>)>
		struct rotation_matrix_constants
		{
			using scalars = muu::constants<Scalar>;

			/// \name Rotations
			/// @{

			/// \brief A matrix encoding a rotation 90 degrees to the right.
			static constexpr matrix<Scalar, Rows, Columns> right =
				make_rotation_matrix<matrix, Scalar, Rows, Columns>(scalars::zero,
																	scalars::zero,
																	-scalars::one,
																	scalars::zero,
																	scalars::one,
																	scalars::zero,
																	scalars::one,
																	scalars::zero,
																	scalars::zero);

			/// \brief A matrix encoding a rotation 90 degrees upward.
			static constexpr matrix<Scalar, Rows, Columns> up =
				make_rotation_matrix<matrix, Scalar, Rows, Columns>(scalars::one,
																	scalars::zero,
																	scalars::zero,
																	scalars::zero,
																	scalars::zero,
																	-scalars::one,
																	scalars::zero,
																	scalars::one,
																	scalars::zero);

			/// \brief A matrix encoding a rotation 180 degrees backward.
			static constexpr matrix<Scalar, Rows, Columns> backward =
				make_rotation_matrix<matrix, Scalar, Rows, Columns>(-scalars::one,
																	scalars::zero,
																	scalars::zero,
																	scalars::zero,
																	scalars::one,
																	scalars::zero,
																	scalars::zero,
																	scalars::zero,
																	-scalars::one);

			/// \brief A matrix encoding a rotation 90 degrees to the left.
			static constexpr matrix<Scalar, Rows, Columns> left =
				make_rotation_matrix<matrix, Scalar, Rows, Columns>(scalars::zero,
																	scalars::zero,
																	scalars::one,
																	scalars::zero,
																	scalars::one,
																	scalars::zero,
																	-scalars::one,
																	scalars::zero,
																	scalars::zero);

			/// \brief A matrix encoding a rotation 90 degrees downward.
			static constexpr matrix<Scalar, Rows, Columns> down =
				make_rotation_matrix<matrix, Scalar, Rows, Columns>(scalars::one,
																	scalars::zero,
																	scalars::zero,
																	scalars::zero,
																	scalars::zero,
																	scalars::one,
																	scalars::zero,
																	-scalars::one,
																	scalars::zero);

			/// @}
		};

		/// \cond

		template <typename Scalar, size_t Rows, size_t Columns>
		struct rotation_matrix_constants<Scalar, Rows, Columns, false>
		{};

		/// \endcond
	}

#if MUU_DOXYGEN
	#define MATRIX_CONSTANTS_BASES                                                                                     \
		impl::rotation_matrix_constants<Scalar, Rows, Columns>, impl::integer_limits<matrix<Scalar, Rows, Columns>>,   \
			impl::integer_positive_constants<matrix<Scalar, Rows, Columns>>,                                           \
			impl::floating_point_traits<matrix<Scalar, Rows, Columns>>,                                                \
			impl::floating_point_special_constants<matrix<Scalar, Rows, Columns>>,                                     \
			impl::floating_point_named_constants<matrix<Scalar, Rows, Columns>>
#else
	#define MATRIX_CONSTANTS_BASES                                                                                     \
		impl::rotation_matrix_constants<Scalar, Rows, Columns>, impl::matrix_constants_base<Scalar, Rows, Columns>
#endif

	/// \brief		Matrix constants.
	///
	/// \ingroup	constants
	/// \see		muu::matrix
	template <typename Scalar, size_t Rows, size_t Columns>
	struct constants<matrix<Scalar, Rows, Columns>> : MATRIX_CONSTANTS_BASES
	{
		/// \brief The identity matrix.
		static constexpr matrix<Scalar, Rows, Columns> identity =
			impl::make_identity_matrix<matrix, Scalar, Rows, Columns>();
	};

#undef MATRIX_CONSTANTS_BASES

	MUU_POP_PRECISE_MATH;
}

//======================================================================================================================
// FREE FUNCTIONS
//======================================================================================================================

namespace muu
{
	/// \ingroup	infinity_or_nan
	/// \relatesalso	muu::matrix
	///
	/// \brief	Returns true if any of the scalar components of a matrix are infinity or NaN.
	template <typename S, size_t R, size_t C>
	MUU_PURE_INLINE_GETTER
	constexpr bool infinity_or_nan(const matrix<S, R, C>& m) noexcept
	{
		if constexpr (is_floating_point<S>)
			return matrix<S, R, C>::infinity_or_nan(m);
		else
		{
			MUU_UNUSED(m);
			return false;
		}
	}

	/// \brief		Returns true if two matrices are approximately equal.
	///
	/// \ingroup	approx_equal
	/// \relatesalso	muu::matrix
	///
	/// \availability	This function is only available when at least one of `S` and `T` is a floating-point type.
	MUU_CONSTRAINED_TEMPLATE((any_floating_point<S, T>), typename S, typename T, size_t R, size_t C)
	MUU_PURE_GETTER
	constexpr bool MUU_VECTORCALL approx_equal(const matrix<S, R, C>& m1,
											   const matrix<T, R, C>& m2,
											   epsilon_type<S, T> epsilon = default_epsilon<S, T>) noexcept
	{
		return matrix<S, R, C>::approx_equal(m1, m2, epsilon);
	}

	/// \brief		Returns true if all the scalar components of a matrix are approximately equal to zero.
	///
	/// \ingroup	approx_zero
	/// \relatesalso	muu::matrix
	///
	/// \availability	This function is only available when `S` is a floating-point type.
	MUU_CONSTRAINED_TEMPLATE(is_floating_point<S>, typename S, size_t R, size_t C)
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL approx_zero(const matrix<S, R, C>& m, S epsilon = default_epsilon<S>) noexcept
	{
		return matrix<S, R, C>::approx_zero(m, epsilon);
	}

	/// \brief	Returns a transposed copy of a matrix.
	///
	/// \relatesalso	muu::matrix
	template <typename S, size_t R, size_t C>
	MUU_PURE_INLINE_GETTER
	constexpr matrix<S, C, R> transpose(const matrix<S, R, C>& m) noexcept
	{
		return matrix<S, R, C>::transpose(m);
	}

	/// \brief	Calculates the determinant of a matrix.
	///
	/// \relatesalso	muu::matrix
	///
	/// \availability	This function is only available for square matrices with at most 4 rows and columns.
	MUU_CONSTRAINED_TEMPLATE(
		(R == C && R <= 4),
		typename S,
		size_t R,
		size_t C //
			MUU_HIDDEN_PARAM(typename determinant_type = typename matrix<S, R, C>::determinant_type))
	MUU_PURE_INLINE_GETTER
	constexpr determinant_type determinant(const matrix<S, R, C>& m) noexcept
	{
		return matrix<S, R, C>::determinant(m);
	}

	/// \brief	Returns the inverse of a matrix.
	///
	/// \relatesalso	muu::matrix
	///
	/// \availability	This function is only available for square matrices with at most 4 rows and columns.
	MUU_CONSTRAINED_TEMPLATE((R == C && R <= 4),
							 typename S,
							 size_t R,
							 size_t C //
								 MUU_HIDDEN_PARAM(typename inverse_type = typename matrix<S, R, C>::inverse_type))
	MUU_PURE_INLINE_GETTER
	constexpr inverse_type invert(const matrix<S, R, C>& m) noexcept
	{
		return matrix<S, R, C>::invert(m);
	}

	/// \brief	Returns a copy of a matrix with the 3x3 part orthonormalized.
	///
	/// \relatesalso	muu::matrix
	///
	/// \availability	This function is only available for matrices with 3 or 4 rows and columns
	/// 				and a floating-point scalar_type.
	///
	/// \see [Orthonormal basis](https://en.wikipedia.org/wiki/Orthonormal_basis)
	MUU_CONSTRAINED_TEMPLATE(is_floating_point<S>, typename S, size_t R, size_t C)
	MUU_PURE_INLINE_GETTER
	constexpr matrix<S, R, C> orthonormalize(const matrix<S, R, C>& m) noexcept
	{
		return matrix<S, R, C>::orthonormalize(m);
	}
}

#undef SPECIALIZED_IF

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "impl/header_end.h"

/// \cond
#include "impl/quaternion_x_matrix.h"
/// \endcond
