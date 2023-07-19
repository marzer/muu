// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

#include "../vector.h"
#include "../matrix.h"
#include "std_optional.h"
#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(float_control(except, off))

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
		inline constexpr bool is_hva<storage_base<T<Scalar>>> = can_be_hva_of<Scalar, storage_base<T<Scalar>>>;        \
                                                                                                                       \
		template <typename Scalar>                                                                                     \
		inline constexpr bool is_hva<T<Scalar>> = is_hva<storage_base<T<Scalar>>>;                                     \
                                                                                                                       \
		template <typename Scalar>                                                                                     \
		struct vector_param_<T<Scalar>>                                                                                \
		{                                                                                                              \
			using type = copy_cvref<T<Scalar>, vector_param<storage_base<T<Scalar>>>>;                                 \
		};                                                                                                             \
	}                                                                                                                  \
                                                                                                                       \
	template <typename From, typename Scalar>                                                                          \
	inline constexpr bool allow_implicit_bit_cast<From, impl::storage_base<T<Scalar>>> =                               \
		allow_implicit_bit_cast<From, T<Scalar>>;                                                                      \
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
		struct MUU_TRIVIAL_ABI storage_base<plane<Scalar>>
		{
			vector<Scalar, 3> normal;
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
		using vector_param = muu::vector_param<vector_type>;

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
		using vector_param = muu::vector_param<vector_type>;

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
		struct MUU_TRIVIAL_ABI storage_base<line_segment<Scalar>>
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
		using vector_param = muu::vector_param<vector_type>;
		using lines		   = lines_common<Scalar>;

		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL closest_point(vector_param seg0, // aka clamped_project()
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
			return vector_type::distance_squared(point, closest_point(seg0, seg1, point));
		}

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains_point(vector_param seg0,
															vector_param seg1,
															vector_param point,
															scalar_type epsilon = default_epsilon<scalar_type>) noexcept
		{
			return distance_squared(seg0, seg1, point) < epsilon;
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
		struct MUU_TRIVIAL_ABI storage_base<triangle<Scalar>>
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
		using scalar_type  = Scalar;
		using vector_type  = vector<scalar_type, 3>;
		using vector_param = muu::vector_param<vector_type>;

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
													  vector_param point,
													  scalar_type epsilon) noexcept
		{
			return muu::approx_zero(vector_type::dot(point - p0, normal(p0, p1, p2)), epsilon);
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
															vector_param point,
															scalar_type epsilon) noexcept
		{
			return coplanar(p0, p1, p2, point, epsilon) && contains_coplanar_point(p0, p1, p2, point);
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
		struct MUU_TRIVIAL_ABI storage_base<bounding_sphere<Scalar>>
		{
			vector<Scalar, 3> center;
			Scalar radius;
		};
	}

	MUU_GEOMETRY_BASE_BOILERPLATE(bounding_sphere);
}

namespace muu::impl
{
	template <typename Scalar>
	struct spheres_common
	{
		using scalar_type  = Scalar;
		using vector_type  = vector<scalar_type, 3>;
		using vector_param = muu::vector_param<vector_type>;
		using triangles	   = triangles_common<scalar_type>;
		using sat_tester   = muu::sat_tester<scalar_type, 3>;

		MUU_PURE_GETTER
		static constexpr scalar_type MUU_VECTORCALL volume(scalar_type radius) noexcept
		{
			return constants<scalar_type>::four_over_three_pi * radius * radius * radius;
		}
	};
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
		using vector_param = muu::vector_param<vector_type>;

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
			return extents.x* scalar_type{ 2 };
		}

		MUU_PURE_INLINE_GETTER
		static constexpr scalar_type MUU_VECTORCALL height(vector_param extents) noexcept
		{
			return extents.y* scalar_type{ 2 };
		}

		MUU_PURE_INLINE_GETTER
		static constexpr scalar_type MUU_VECTORCALL depth(vector_param extents) noexcept
		{
			return extents.z* scalar_type{ 2 };
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
			return extents.x * extents.y * extents.z* scalar_type{ 8 };
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
		struct MUU_TRIVIAL_ABI storage_base<bounding_box<Scalar>>
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
	struct aabbs_common : boxes_common<Scalar>
	{
		using scalar_type  = Scalar;
		using vector_type  = vector<scalar_type, 3>;
		using vector_param = muu::vector_param<vector_type>;
		using boxes		   = boxes_common<scalar_type>;
		using triangles	   = triangles_common<scalar_type>;
		using sat_tester   = muu::sat_tester<scalar_type, 3>;

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

		template <box_corner Corner>
		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL corner_min_max(vector_param min, vector_param max) noexcept
		{
			static_assert(Corner <= box_corner::max);

			if constexpr (Corner == box_corner::min)
			{
				MUU_UNUSED(max);
				return min;
			}
			if constexpr (Corner == box_corner::x)
				return vector_type{ max.x, min.y, min.z };
			if constexpr (Corner == box_corner::y)
				return vector_type{ min.x, max.y, min.z };
			if constexpr (Corner == box_corner::z)
				return vector_type{ min.x, min.y, max.z };
			if constexpr (Corner == box_corner::xy)
				return vector_type{ max.x, max.y, min.z };
			if constexpr (Corner == box_corner::xz)
				return vector_type{ max.x, min.y, max.z };
			if constexpr (Corner == box_corner::yz)
				return vector_type{ min.x, max.y, max.z };
			if constexpr (Corner == box_corner::max)
			{
				MUU_UNUSED(min);
				return max;
			}
		}

		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL corner_min_max(vector_param min,
																   vector_param max,
																   box_corner which) noexcept
		{
			MUU_CONSTEXPR_SAFE_ASSERT(which <= box_corner::max && "'which' cannot exceed box_corner::max");
			MUU_ASSUME(which <= box_corner::max);

			switch (which)
			{
				case box_corner::min: return min;
				case box_corner::x: vector_type{ max.x, min.y, min.z };
				case box_corner::y: vector_type{ min.x, max.y, min.z };
				case box_corner::xy: vector_type{ max.x, max.y, min.z };
				case box_corner::z: vector_type{ min.x, min.y, max.z };
				case box_corner::xz: vector_type{ max.x, min.y, max.z };
				case box_corner::yz: vector_type{ min.x, max.y, max.z };
				case box_corner::max: return min;
				default: MUU_UNREACHABLE;
			}
			MUU_UNREACHABLE;
		}

		//--------------------------------
		// aabb x point
		//--------------------------------

		MUU_PURE_INLINE_GETTER
		static constexpr vector_type MUU_VECTORCALL closest_min_max(vector_param min,
																	vector_param max,
																	vector_param pt) noexcept
		{
			return vector_type::clamp(pt, min, max);
		}

		MUU_PURE_GETTER
		static constexpr vector_type MUU_VECTORCALL furthest_center_min_max(vector_param center,
																			vector_param min,
																			vector_param max,
																			vector_param pt) noexcept
		{
			return vector_type{
				pt.x >= center.x ? min.x : max.x,
				pt.y >= center.y ? min.y : max.y,
				pt.z >= center.z ? min.z : max.z,
			};
		}

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains_point(vector_param center,
															vector_param extents,
															vector_param point) noexcept
		{
			const auto adj = vector_type::abs(point - center);
			return adj.x <= extents.x //
				&& adj.y <= extents.y //
				&& adj.z <= extents.z;
		}

		//--------------------------------
		// aabb x triangle
		//--------------------------------

		// the following functions are collectively the the Akenine-Moller algorithm:
		// https://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/pubs/tribox.pdf

		// part one - test box normals against triangle points
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects_tri_akenine_moller_1(vector_param min,
																			 vector_param max,
																			 vector_param tri_p0,
																			 vector_param tri_p1,
																			 vector_param tri_p2) noexcept
		{
			const auto test_axis = [&](auto idx) noexcept -> bool
			{
				constexpr auto axis_index = remove_cvref<decltype(idx)>::value;
				sat_tester sat{ index_tag<axis_index>{}, tri_p0, tri_p1, tri_p2 };
				return sat(min.template get<axis_index>(), max.template get<axis_index>());
			};

			return test_axis(x_axis_tag{}) && test_axis(y_axis_tag{}) && test_axis(z_axis_tag{});
		}

		// part two - test triangle normal against box points
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects_tri_akenine_moller_2(const vector_type (&corners)[8],
																			 vector_param tri_p0,
																			 vector_param tri_normal) noexcept
		{
			sat_tester sat{ tri_normal, corners };
			return sat(vector_type::dot(tri_normal, tri_p0));
		}

		// part three - test edge cross products
		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects_tri_akenine_moller_3(const vector_type (&corners)[8],
																			 vector_param tri_p0,
																			 vector_param tri_p1,
																			 vector_param tri_p2,
																			 const vector_type (&tri_edges)[3]) noexcept
		{
			for (const auto& edge : tri_edges)
			{
				const auto test_edge = [&](auto idx) noexcept -> bool
				{
					constexpr auto axis_index = remove_cvref<decltype(idx)>::value;

					const auto axis = vector_type::cross(edge, index_tag<axis_index>{});
					sat_tester box_sat{ axis, corners };
					sat_tester tri_sat{ axis, tri_p0, tri_p1, tri_p2 };
					return box_sat(tri_sat);
				};
				if (!test_edge(x_axis_tag{}) || !test_edge(y_axis_tag{}) || !test_edge(z_axis_tag{}))
					return false;
			}
			return true;
		}

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects_tri_min_max(vector_param min,
																	vector_param max,
																	vector_param p0,
																	vector_param p1,
																	vector_param p2) noexcept
		{
			if (!intersects_tri_akenine_moller_1(min, max, p0, p1, p2))
				return false;

			const vector_type corners[] = { min,
											corner_min_max<box_corner::x>(min, max),
											corner_min_max<box_corner::xy>(min, max),
											corner_min_max<box_corner::xz>(min, max),
											corner_min_max<box_corner::y>(min, max),
											corner_min_max<box_corner::yz>(min, max),
											corner_min_max<box_corner::z>(min, max),
											max };
			if (!intersects_tri_akenine_moller_2(corners, p0, triangles::normal(p0, p1, p2)))
				return false;

			const vector_type tri_edges[] = { p1 - p0, p2 - p1, p0 - p2 };
			if (!intersects_tri_akenine_moller_3(corners, p0, p1, p2, tri_edges))
				return false;

			return true;
		}

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects_tri(vector_param center,
															vector_param extents,
															vector_param p0,
															vector_param p1,
															vector_param p2) noexcept
		{
			return intersects_tri_min_max(center - extents, center + extents, p0, p1, p2);
		}

		//--------------------------------
		// aabb x sphere
		//--------------------------------

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects_sphere_min_max_radsq(vector_param min,
																			 vector_param max,
																			 vector_param sphere_center,
																			 scalar_type sphere_radius_squared) noexcept
		{
			return vector_type::distance_squared(closest_min_max(min, max, sphere_center), sphere_center)
				<= sphere_radius_squared;
		}

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects_sphere(vector_param center,
															   vector_param extents,
															   vector_param sphere_center,
															   scalar_type sphere_radius) noexcept
		{
			return intersects_sphere_min_max_radsq(center - extents, //
												   center + extents, //
												   sphere_center,	 //
												   sphere_radius * sphere_radius);
		}

		//--------------------------------
		// aabb x aabb
		//--------------------------------

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects_aabb_min_max(vector_param min1,
																	 vector_param max1,
																	 vector_param min2,
																	 vector_param max2) noexcept
		{
			return max1.x >= min2.x //
				&& min1.x <= max2.x //
				&& max1.y >= min2.y //
				&& min1.y <= max2.y //
				&& max1.z >= min2.z //
				&& min1.z <= max2.z;
		}

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL contains_aabb_min_max(vector_param outer_min,
																   vector_param outer_max,
																   vector_param inner_min,
																   vector_param inner_max) noexcept
		{
			return outer_min.x <= inner_min.x //
				&& outer_max.x >= inner_max.x //
				&& outer_min.y <= inner_min.y //
				&& outer_max.y >= inner_max.y //
				&& outer_min.z <= inner_min.z //
				&& outer_max.z >= inner_max.z;
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
		struct MUU_TRIVIAL_ABI storage_base<oriented_bounding_box<Scalar>>
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
	struct obbs_common : boxes_common<Scalar>
	{
		using scalar_type  = Scalar;
		using vector_type  = vector<scalar_type, 3>;
		using vector_param = muu::vector_param<vector_type>;
		using axes_type	   = matrix<scalar_type, 3, 3>;
		using axes_param   = muu::vector_param<axes_type>;
		using boxes		   = boxes_common<scalar_type>;

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

//======================================================================================================================
// rays
//======================================================================================================================

namespace muu
{
	namespace impl
	{
		template <typename Scalar>
		struct MUU_TRIVIAL_ABI storage_base<ray<Scalar>>
		{
			vector<Scalar, 3> origin;
			vector<Scalar, 3> direction;
		};
	}

	MUU_GEOMETRY_BASE_BOILERPLATE(ray);
}

namespace muu::impl
{
	template <typename Scalar>
	struct rays_common
	{
		using scalar_type  = Scalar;
		using vector_type  = vector<scalar_type, 3>;
		using vector_param = muu::vector_param<vector_type>;
		using planes	   = planes_common<scalar_type>;
		using triangles	   = triangles_common<scalar_type>;
		using aabbs		   = aabbs_common<scalar_type>;
		using result_type  = std::optional<scalar_type>;

		MUU_PURE_GETTER
		static constexpr result_type hits_plane(vector_param ray_origin,
												vector_param ray_direction,
												vector_param p_normal,
												scalar_type p_d) noexcept
		{
			MUU_FMA_BLOCK;

			const auto nd		   = vector_type::dot(ray_direction, p_normal);
			const auto signed_dist = planes::signed_distance(p_normal, p_d, ray_origin);
			if ((signed_dist > scalar_type{} && nd >= scalar_type{})
				|| (signed_dist < scalar_type{} && nd <= scalar_type{}))
				return {};

			const auto pn			= vector_type::dot(ray_origin, p_normal);
			const auto hit_distance = (-p_d - pn) / nd;
			if (hit_distance >= scalar_type{})
				return hit_distance;

			return {};
		}

		MUU_PURE_GETTER
		static constexpr result_type hits_triangle(vector_param ray_origin,
												   vector_param ray_direction,
												   vector_param p0,
												   vector_param p1,
												   vector_param p2) noexcept
		{
			MUU_FMA_BLOCK;

			// this is an implementation of the Möller-Trumbore intersection algorithm:
			// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm

			vector_type v0v1 = p1 - p0;
			vector_type v0v2 = p2 - p0;
			vector_type pvec = vector_type::cross(ray_direction, v0v2);
			scalar_type det	 = vector_type::dot(v0v1, pvec);

			// ray and triangle are parallel if det is close to 0
			if (muu::abs(det) < default_epsilon<scalar_type>)
				return {};

			const auto invDet = scalar_type{ 1 } / det;

			vector_type tvec = ray_origin - p0;
			scalar_type u	 = vector_type::dot(tvec, pvec) * invDet;
			if (u < scalar_type{} || u > scalar_type{ 1 })
				return {};

			vector_type qvec = vector_type::cross(tvec, v0v1);
			scalar_type v	 = vector_type::dot(ray_direction, qvec) * invDet;
			if (v < scalar_type{} || u + v > scalar_type{ 1 })
				return {};

			return vector_type::dot(v0v2, qvec) * invDet;
		}

		MUU_PURE_GETTER
		static constexpr result_type hits_sphere(vector_param ray_origin,
												 vector_param ray_direction,
												 vector_param sphere_center,
												 scalar_type sphere_radius) noexcept
		{
			MUU_FMA_BLOCK;

			const vector_type e = sphere_center - ray_origin;
			const scalar_type a = vector_type::dot(e, ray_direction);

			// ray points in the wrong direction
			if (a < scalar_type{})
				return {};

			const scalar_type e2 = vector_type::length_squared(e);
			const scalar_type r2 = sphere_radius * sphere_radius;
			const scalar_type a2 = a * a;

			// no collision
			if (r2 - e2 + a2 < default_epsilon<scalar_type>)
				return {};

			const scalar_type b = muu::sqrt(e2 - a2);
			const scalar_type f = muu::sqrt(r2 - (b * b));

			// ray inside sphere
			if (e2 < r2)
				return a + f;

			// normal intersection
			return a - f;
		}

		MUU_PURE_GETTER
		static constexpr result_type hits_aabb_min_max(vector_param ray_origin,
													   vector_param ray_direction,
													   vector_param box_min,
													   vector_param box_max) noexcept
		{
			MUU_FMA_BLOCK;

			const auto t1	= (box_min.x - ray_origin.x) / ray_direction.x;
			const auto t2	= (box_max.x - ray_origin.x) / ray_direction.x;
			const auto t3	= (box_min.y - ray_origin.y) / ray_direction.y;
			const auto t4	= (box_max.y - ray_origin.y) / ray_direction.y;
			const auto t5	= (box_min.z - ray_origin.z) / ray_direction.z;
			const auto t6	= (box_max.z - ray_origin.z) / ray_direction.z;
			const auto tmin = muu::max(muu::min(t1, t2), muu::min(t3, t4), muu::min(t5, t6));
			const auto tmax = muu::min(muu::max(t1, t2), muu::max(t3, t4), muu::max(t5, t6));

			if (tmax < scalar_type{} || tmin > tmax)
				return {};

			return result_type{ tmin < scalar_type{} ? tmax : tmin };
		}

		MUU_PURE_GETTER
		static constexpr result_type hits_aabb(vector_param ray_origin,
											   vector_param ray_direction,
											   vector_param box_center,
											   vector_param box_extents) noexcept
		{
			return hits_aabb_min_max(ray_origin, ray_direction, box_center - box_extents, box_center + box_extents);
		}
	};
}

/// \endcond

#undef MUU_GEOMETRY_BASE_BOILERPLATE
MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
