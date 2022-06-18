// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

#include "../vector.h"
#include "../matrix.h"
#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(float_control(except, off))
MUU_PRAGMA_MSVC(inline_recursion(on))

namespace muu
{
	/// \brief Corner masks/indices for three-dimensional bounding-boxes.
	/// \ingroup math
	///
	/// \see
	/// 	- muu::bounding_box
	/// 	- muu::oriented_bounding_box
	enum class box_corner : uint8_t
	{
		min = 0,		 ///< The 'min' corner (negative offset on all axes).
		x	= 1,		 ///< The corner with a positive offset on the X axis, negative on Y and Z.
		y	= 2,		 ///< The corner with a positive offset on the Y axis, negative on X and Z.
		xy	= x | y,	 ///< The corner with a positive offset on the X and Y axes, negative on Z.
		z	= 4,		 ///< The corner with a positive offset on the Z axis, negative on X and Y.
		xz	= x | z,	 ///< The corner with a positive offset on the X and Z axes, negative on Y.
		yz	= y | z,	 ///< The corner with a positive offset on the Y and Z axes, negative on X.
		xyz = x | y | z, ///< The 'max' corner (positive offset on all axes).
		max = xyz		 ///< The 'max' corner (positive offset on all axes).
	};
}

/// \cond

#define MUU_GEOMETRY_BASE_BOILERPLATE(T)                                                                               \
	namespace impl                                                                                                     \
	{                                                                                                                  \
		template <typename Scalar>                                                                                     \
		inline constexpr bool is_hva<T##_<Scalar>> = can_be_hva_of<Scalar, T##_<Scalar>>;                              \
                                                                                                                       \
		template <typename Scalar>                                                                                     \
		inline constexpr bool is_hva<muu::T<Scalar>> = is_hva<T##_<Scalar>>;                                           \
                                                                                                                       \
		template <typename Scalar>                                                                                     \
		struct vectorcall_param_<muu::T<Scalar>>                                                                       \
		{                                                                                                              \
			using type =                                                                                               \
				std::conditional_t<pass_vectorcall_by_value<T##_<Scalar>>, muu::T<Scalar>, const muu::T<Scalar>&>;     \
		};                                                                                                             \
	}                                                                                                                  \
                                                                                                                       \
	template <typename From, typename Scalar>                                                                          \
	inline constexpr bool allow_implicit_bit_cast<From, impl::T##_<Scalar>> =                                          \
		allow_implicit_bit_cast<From, muu::T<Scalar>>;                                                                 \
                                                                                                                       \
	static_assert(true)

//======================================================================================================================
// planes
//======================================================================================================================

namespace muu
{
	namespace impl
	{
		template <typename Scalar>
		struct MUU_TRIVIAL_ABI plane_
		{
			vector<Scalar, 3> n;
			Scalar d;
		};
	}

	MUU_GEOMETRY_BASE_BOILERPLATE(plane);
}

namespace muu::impl
{
	template <typename Scalar>
	struct planes_common
	{
		using scalar_type  = Scalar;
		using vector_type  = vector<scalar_type, 3>;
		using vector_param = vectorcall_param<vector_type>;

		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL origin(vector_param normal, scalar_type d) noexcept
		{
			return normal * -d;
		}

		MUU_PURE_INLINE_GETTER
		static constexpr scalar_type MUU_VECTORCALL d_term(vector_param pos, vector_param dir) noexcept
		{
			return -vector_type::dot(pos, dir);
		}

		MUU_PURE_INLINE_GETTER
		static constexpr scalar_type MUU_VECTORCALL signed_distance(vector_param normal,
																	scalar_type d,
																	vector_param point) noexcept
		{
			return vector_type::dot(normal, point) + d;
		}

		MUU_PURE_GETTER
		static constexpr scalar_type MUU_VECTORCALL unsigned_distance(vector_param normal,
																	  scalar_type d,
																	  vector_param point) noexcept
		{
			return muu::abs(signed_distance(normal, d, point));
		}

		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL project_with_signed_distance(vector_param normal,
																				 vector_param point,
																				 scalar_type signed_dist) noexcept
		{
			MUU_FMA_BLOCK;

			return point - normal * signed_dist;
		}

		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL project(vector_param normal,
															scalar_type d,
															vector_param point) noexcept
		{
			return project_with_signed_distance(normal, point, signed_distance(normal, d, point));
		}

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains_point(vector_param normal,
															scalar_type d,
															vector_param point,
															scalar_type epsilon = default_epsilon<scalar_type>) noexcept
		{
			return muu::approx_zero(signed_distance(normal, d, point), epsilon);
		}

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects_plane(vector_param normal1, vector_param normal2) noexcept
		{
			return muu::approx_zero(vector_type::length_squared(vector_type::cross(normal1, normal2)));
		}

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects_line_segment(vector_param normal,
																	 scalar_type d,
																	 vector_param start,
																	 vector_param end) noexcept
		{
			return signed_distance(normal, d, start) * signed_distance(normal, d, end) >= scalar_type{};
		}
	};
}

//======================================================================================================================
// lines
//======================================================================================================================

namespace muu::impl
{
	template <typename Scalar>
	struct lines_common
	{
		using scalar_type  = Scalar;
		using vector_type  = vector<scalar_type, 3>;
		using vector_param = vectorcall_param<vector_type>;

		MUU_PURE_INLINE_GETTER
		static constexpr scalar_type MUU_VECTORCALL project_distance(vector_param line_origin,
																	 vector_param line_dir,
																	 vector_param point) noexcept
		{
			return vector_type::dot(point - line_origin, line_dir);
		}

		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL project(vector_param line_origin,
															vector_param line_dir,
															vector_param point) noexcept
		{
			return line_origin + line_dir * project_distance(line_origin, line_dir, point);
		}

		MUU_PURE_GETTER
		static constexpr scalar_type MUU_VECTORCALL distance_squared(vector_param line_origin,
																	 vector_param line_dir,
																	 vector_param point) noexcept
		{
			return vector_type::distance_squared(point, project(line_origin, line_dir, point));
		}

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains_point(vector_param line_origin,
															vector_param line_dir,
															vector_param point,
															scalar_type epsilon = default_epsilon<scalar_type>) noexcept
		{
			return muu::approx_zero(distance_squared(line_origin, line_dir, point), epsilon);
		}
	};
}

//======================================================================================================================
// line segments
//======================================================================================================================

namespace muu
{
	namespace impl
	{
		template <typename Scalar>
		struct MUU_TRIVIAL_ABI line_segment_
		{
			vector<Scalar, 3> points[2];
		};
	}

	MUU_GEOMETRY_BASE_BOILERPLATE(line_segment);
}

namespace muu::impl
{
	template <typename Scalar>
	struct line_segments_common
	{
		using scalar_type  = Scalar;
		using vector_type  = vector<scalar_type, 3>;
		using vector_param = vectorcall_param<vector_type>;
		using lines		   = lines_common<Scalar>;

		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL nearest_point(vector_param seg0, // aka clamped_project()
																  vector_param seg1,
																  vector_param point) noexcept
		{
			scalar_type seg_len{};
			const auto seg_dir = vector_type::direction(seg0, seg1, seg_len);
			return seg0 + seg_dir * muu::clamp(lines::project_distance(seg0, seg_dir, point), scalar_type{}, seg_len);
		}

		MUU_PURE_GETTER
		static constexpr scalar_type MUU_VECTORCALL distance_squared(vector_param seg0,
																	 vector_param seg1,
																	 vector_param point) noexcept
		{
			return vector_type::distance_squared(point, nearest_point(seg0, seg1, point));
		}

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains_point(vector_param seg0,
															vector_param seg1,
															vector_param point,
															scalar_type epsilon = default_epsilon<scalar_type>) noexcept
		{
			return muu::approx_zero(distance_squared(seg0, seg1, point), epsilon);
		}
	};
}

//======================================================================================================================
// triangles
//======================================================================================================================

namespace muu
{
	namespace impl
	{
		template <typename Scalar>
		struct MUU_TRIVIAL_ABI triangle_
		{
			vector<Scalar, 3> points[3];
		};
	}

	MUU_GEOMETRY_BASE_BOILERPLATE(triangle);
}

namespace muu::impl
{
	template <typename Scalar>
	struct triangles_common
	{
		using scalar_type	 = Scalar;
		using vector_type	 = vector<scalar_type, 3>;
		using vector_param	 = vectorcall_param<vector_type>;
		using triangle_param = vectorcall_param<triangle_<scalar_type>>;

		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL normal(vector_param p0, vector_param p1, vector_param p2) noexcept
		{
			return vector_type::normalize(vector_type::cross(p1 - p0, p2 - p0));
		}

		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL centroid(vector_param p0, vector_param p1, vector_param p2) noexcept
		{
			MUU_FMA_BLOCK;

			return (p0 + p1 + p2) / scalar_type{ 3 };
		}

		MUU_PURE_GETTER
		static constexpr scalar_type MUU_VECTORCALL perimeter(vector_param p0,
															  vector_param p1,
															  vector_param p2) noexcept
		{
			return vector_type::distance(p0, p1) //
				 + vector_type::distance(p1, p2) //
				 + vector_type::distance(p2, p0);
		}

		MUU_PURE_GETTER
		static constexpr scalar_type MUU_VECTORCALL area(vector_param p0, vector_param p1, vector_param p2) noexcept
		{
			MUU_FMA_BLOCK;

			const auto a = vector_type::distance(p0, p1);
			const auto b = vector_type::distance(p1, p2);
			const auto c = vector_type::distance(p2, p0);
			const auto s = (a + b + c) / scalar_type{ 2 };
			return muu::sqrt(s * (s - a) * (s - b) * (s - c)); // heron's formula
		}

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL degenerate(vector_param p0, vector_param p1, vector_param p2) noexcept
		{
			return p0 == p1 || p0 == p2 || p1 == p2;
		}

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL coplanar(vector_param p0,
													  vector_param p1,
													  vector_param p2,
													  vector_param point) noexcept
		{
			return muu::approx_zero(vector_type::dot(point - p0, normal(p0, p1, p2)));
		}

		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL barycentric(vector_param p0,
																vector_param p1,
																vector_param p2,
																vector_param point) noexcept
		{
			MUU_FMA_BLOCK;

			const auto v0	 = p1 - p0;
			const auto v1	 = p2 - p0;
			const auto d00	 = vector_type::dot(v0, v0);
			const auto d01	 = vector_type::dot(v0, v1);
			const auto d11	 = vector_type::dot(v1, v1);
			const auto denom = scalar_type{ 1 } / (d00 * d11 - d01 * d01);

			const auto v2  = point - p0;
			const auto d20 = vector_type::dot(v2, v0);
			const auto d21 = vector_type::dot(v2, v1);

			vector_type out{ scalar_type{ 1 }, //
							 (d11 * d20 - d01 * d21) * denom,
							 (d00 * d21 - d01 * d20) * denom };
			out.x -= out.y + out.z;
			return out;
		}

		class MUU_TRIVIAL_ABI barycentric_generator
		{
		  private:
			vector_type p;
			vector_type v0;
			vector_type v1;
			scalar_type d00;
			scalar_type d01;
			scalar_type d11;
			scalar_type denom;

		  public:
			MUU_NODISCARD_CTOR
			constexpr barycentric_generator(const vector_type& p0,
											const vector_type& p1,
											const vector_type& p2) noexcept
				: p{ p0 },
				  v0{ p1 - p0 },
				  v1{ p2 - p0 },
				  d00{ vector_type::dot(v0, v0) },
				  d01{ vector_type::dot(v0, v1) },
				  d11{ vector_type::dot(v1, v1) },
				  denom{ scalar_type{ 1 } / (d00 * d11 - d01 * d01) }
			{}

			MUU_PURE_GETTER
			constexpr vector_type MUU_VECTORCALL operator()(vector_param point) const noexcept
			{
				MUU_FMA_BLOCK;

				const auto v2  = point - p;
				const auto d20 = vector_type::dot(v2, v0);
				const auto d21 = vector_type::dot(v2, v1);

				vector_type out{ scalar_type{ 1 }, //
								 (d11 * d20 - d01 * d21) * denom,
								 (d00 * d21 - d01 * d20) * denom };
				out.x -= out.y + out.z;
				return out;
			}
		};

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains_coplanar_point(vector_param p0,
																	 vector_param p1,
																	 vector_param p2,
																	 vector_param point) noexcept
		{
			const auto a = p0 - point;
			const auto b = p1 - point;
			const auto c = p2 - point;

			const auto u = vector_type::cross(b, c);
			const auto v = vector_type::cross(c, a);
			const auto w = vector_type::cross(a, b);

			return vector_type::dot(u, v) >= scalar_type{} && vector_type::dot(u, w) >= scalar_type{};
		}

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains_point(vector_param p0,
															vector_param p1,
															vector_param p2,
															vector_param point) noexcept
		{
			return coplanar(p0, p1, p2, point) && contains_coplanar_point(p0, p1, p2, point);
		}
	};
}

//======================================================================================================================
// bounding spheres
//======================================================================================================================

namespace muu
{
	namespace impl
	{
		template <typename Scalar>
		struct MUU_TRIVIAL_ABI bounding_sphere_
		{
			vector<Scalar, 3> center;
			Scalar radius;
		};
	}

	MUU_GEOMETRY_BASE_BOILERPLATE(bounding_sphere);
}

//======================================================================================================================
// bounding boxes - all
//======================================================================================================================

namespace muu::impl
{
	template <typename Scalar>
	struct boxes_common
	{
		using scalar_type  = Scalar;
		using vector_type  = vector<scalar_type, 3>;
		using vector_param = vectorcall_param<vector_type>;

		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL center(vector_param min, vector_param max) noexcept
		{
			return (min + max) * static_cast<scalar_type>(0.5);
		}

		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL extents(vector_param min, vector_param max) noexcept
		{
			return (max - min) * static_cast<scalar_type>(0.5);
		}

		MUU_PURE_INLINE_GETTER
		static constexpr scalar_type MUU_VECTORCALL width(vector_param extents) noexcept
		{
			return extents.x * scalar_type{ 2 };
		}

		MUU_PURE_INLINE_GETTER
		static constexpr scalar_type MUU_VECTORCALL height(vector_param extents) noexcept
		{
			return extents.y * scalar_type{ 2 };
		}

		MUU_PURE_INLINE_GETTER
		static constexpr scalar_type MUU_VECTORCALL depth(vector_param extents) noexcept
		{
			return extents.z * scalar_type{ 2 };
		}

		MUU_PURE_GETTER
		static constexpr scalar_type MUU_VECTORCALL diagonal(vector_param extents) noexcept
		{
			return vector_type::length(extents) * scalar_type{ 2 };
		}

		MUU_PURE_GETTER
		static constexpr const scalar_type& shortest_extent(const vector_type& extents) noexcept
		{
			return min(extents.x, min(extents.y, extents.z));
		}

		MUU_PURE_GETTER
		static constexpr const scalar_type& longest_extent(const vector_type& extents) noexcept
		{
			return max(extents.x, max(extents.y, extents.z));
		}

		MUU_PURE_GETTER
		static constexpr scalar_type& shortest_extent(vector_type& extents) noexcept
		{
			return const_cast<scalar_type&>(shortest_extent(static_cast<const vector_type&>(extents)));
		}

		MUU_PURE_GETTER
		static constexpr scalar_type& longest_extent(vector_type& extents) noexcept
		{
			return const_cast<scalar_type&>(longest_extent(static_cast<const vector_type&>(extents)));
		}

		MUU_PURE_GETTER
		static constexpr scalar_type shortest_side(const vector_type& extents) noexcept
		{
			return shortest_extent(extents) * scalar_type{ 2 };
		}

		MUU_PURE_GETTER
		static constexpr scalar_type longest_side(const vector_type& extents) noexcept
		{
			return longest_extent(extents) * scalar_type{ 2 };
		}

		MUU_PURE_GETTER
		static constexpr scalar_type MUU_VECTORCALL volume(vector_param extents) noexcept
		{
			return extents.x * extents.y * extents.z * scalar_type{ 8 };
		}

		MUU_PURE_GETTER
		static constexpr scalar_type MUU_VECTORCALL mass(vector_param extents, scalar_type density) noexcept
		{
			return density * volume(extents);
		}

		MUU_PURE_GETTER
		static constexpr scalar_type MUU_VECTORCALL density(vector_param extents, scalar_type mass) noexcept
		{
			return mass / volume(extents);
		}

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL degenerate(vector_param extents) noexcept
		{
			return extents.x <= scalar_type{} || extents.y <= scalar_type{} || extents.z <= scalar_type{};
		}

		template <box_corner Corner>
		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL corner_offset(vector_param extents) noexcept
		{
			static_assert(Corner <= box_corner::max);

			if constexpr (Corner == box_corner::min)
				return -extents;
			if constexpr (Corner == box_corner::x)
				return vector_type{ extents.x, -extents.y, -extents.z };
			if constexpr (Corner == box_corner::y)
				return vector_type{ -extents.x, extents.y, -extents.z };
			if constexpr (Corner == box_corner::z)
				return vector_type{ -extents.x, -extents.y, extents.z };
			if constexpr (Corner == box_corner::xy)
				return vector_type{ extents.x, extents.y, -extents.z };
			if constexpr (Corner == box_corner::xz)
				return vector_type{ extents.x, -extents.y, extents.z };
			if constexpr (Corner == box_corner::yz)
				return vector_type{ -extents.x, extents.y, extents.z };
			if constexpr (Corner == box_corner::max)
				return extents;
		}

		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL corner_offset(vector_param extents, box_corner which) noexcept
		{
			MUU_CONSTEXPR_SAFE_ASSERT(which <= box_corner::max && "'which' cannot exceed box_corner::max");
			MUU_ASSUME(which <= box_corner::max);

			switch (which)
			{
				case box_corner::min: return boxes_common::template corner_offset<box_corner::min>(extents);
				case box_corner::x: return boxes_common::template corner_offset<box_corner::x>(extents);
				case box_corner::y: return boxes_common::template corner_offset<box_corner::y>(extents);
				case box_corner::xy: return boxes_common::template corner_offset<box_corner::xy>(extents);
				case box_corner::z: return boxes_common::template corner_offset<box_corner::z>(extents);
				case box_corner::xz: return boxes_common::template corner_offset<box_corner::xz>(extents);
				case box_corner::yz: return boxes_common::template corner_offset<box_corner::yz>(extents);
				case box_corner::max: return boxes_common::template corner_offset<box_corner::max>(extents);
				default: MUU_UNREACHABLE;
			}
			MUU_UNREACHABLE;
		}
	};
}

//======================================================================================================================
// bounding boxes - axis-aligned
//======================================================================================================================

namespace muu
{
	namespace impl
	{
		template <typename Scalar>
		struct MUU_TRIVIAL_ABI bounding_box_
		{
			vector<Scalar, 3> center;
			vector<Scalar, 3> extents;
		};
	}

	MUU_GEOMETRY_BASE_BOILERPLATE(bounding_box);
}

namespace muu::impl
{
	template <typename Scalar>
	struct aabb_common : boxes_common<Scalar>
	{
		using scalar_type  = Scalar;
		using vector_type  = vector<scalar_type, 3>;
		using vector_param = vectorcall_param<vector_type>;
		using boxes		   = boxes_common<Scalar>;
		using triangles	   = triangles_common<Scalar>;

		template <box_corner Corner>
		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL corner(vector_param center, vector_param extents) noexcept
		{
			return center + boxes::template corner_offset<Corner>(extents);
		}

		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL corner(vector_param center,
														   vector_param extents,
														   box_corner which) noexcept
		{
			return center + boxes::corner_offset(extents, which);
		}

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects_sphere_minmax(vector_param min,
																	  vector_param max,
																	  vector_param sphere_center,
																	  scalar_type sphere_radius) noexcept
		{
			return vector_type::distance_squared(vector_type::clamp(sphere_center, min, max), sphere_center)
				<= sphere_radius;
		}

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects_sphere(vector_param center,
															   vector_param extents,
															   vector_param sphere_center,
															   scalar_type sphere_radius) noexcept
		{
			return intersects_sphere_minmax(center - extents, //
											center + extents, //
											sphere_center,	  //
											sphere_radius);
		}
	};
}

//======================================================================================================================
// bounding boxes - oriented
//======================================================================================================================

namespace muu
{
	namespace impl
	{
		template <typename Scalar>
		struct MUU_TRIVIAL_ABI oriented_bounding_box_
		{
			vector<Scalar, 3> center;
			vector<Scalar, 3> extents;
			matrix<Scalar, 3, 3> axes;
		};
	}

	MUU_GEOMETRY_BASE_BOILERPLATE(oriented_bounding_box);
}

namespace muu::impl
{
	template <typename Scalar>
	struct obb_common : boxes_common<Scalar>
	{
		using scalar_type  = Scalar;
		using vector_type  = vector<scalar_type, 3>;
		using vector_param = vectorcall_param<vector_type>;
		using axes_type	   = matrix<scalar_type, 3, 3>;
		using axes_param   = vectorcall_param<axes_type>;
		using boxes		   = boxes_common<Scalar>;

		template <box_corner Corner>
		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL corner(vector_param center,
														   vector_param extents,
														   axes_param axes) noexcept
		{
			const auto offset = boxes::template corner_offset<Corner>(extents);
			return center + offset.x * axes.m[0] + offset.y * axes.m[1] + offset.z * axes.m[2];
		}

		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL corner(vector_param center,
														   vector_param extents,
														   axes_param axes,
														   box_corner which) noexcept
		{
			const auto offset = boxes::corner_offset(extents, which);
			return center + offset.x * axes.m[0] + offset.y * axes.m[1] + offset.z * axes.m[2];
		}
	};
}

/// \endcond

#undef MUU_GEOMETRY_BASE_BOILERPLATE
MUU_PRAGMA_MSVC(inline_recursion(off))
MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
