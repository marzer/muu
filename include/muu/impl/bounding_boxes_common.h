// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "geometric_types_common.h"
#include "header_start.h"

/// \cond

MUU_PUSH_WARNINGS;
MUU_DISABLE_SPAM_WARNINGS;

MUU_PRAGMA_MSVC(inline_recursion(on))
MUU_PRAGMA_MSVC(float_control(push))
MUU_PRAGMA_MSVC(float_control(except, off))
MUU_PRAGMA_MSVC(float_control(precise, off))

namespace muu::impl
{
	template <typename Scalar>
	struct bounding_boxes_common
	{
		using scalar_type = Scalar;
		using vector_type = vector<scalar_type, 3>;
		using vector_param = readonly_param<vector_type>;
		using delta_type = std::conditional_t<is_integral<scalar_type>, double, scalar_type>;
		using intermediate_float = impl::promote_if_small_float<delta_type>;

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr delta_type MUU_VECTORCALL width(vector_param extents) noexcept
		{
			return static_cast<delta_type>(extents.x * intermediate_float{ 2 });
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr delta_type MUU_VECTORCALL height(vector_param extents) noexcept
		{
			return static_cast<delta_type>(extents.y * intermediate_float{ 2 });
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr delta_type MUU_VECTORCALL depth(vector_param extents) noexcept
		{
			return static_cast<delta_type>(extents.z * intermediate_float{ 2 });
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr delta_type MUU_VECTORCALL diagonal(vector_param extents) noexcept
		{
			return static_cast<delta_type>(vector_type::raw_length(extents) * intermediate_float{ 2 });
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr const scalar_type& shortest_extent(const vector_type& extents) noexcept
		{
			return min(extents.x, min(extents.y, extents.z));
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr const scalar_type& longest_extent(const vector_type& extents) noexcept
		{
			return max(extents.x, max(extents.y, extents.z));
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr scalar_type& shortest_extent(vector_type& extents) noexcept
		{
			return min(extents.x, min(extents.y, extents.z));
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr scalar_type& longest_extent(vector_type& extents) noexcept
		{
			return max(extents.x, max(extents.y, extents.z));
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr delta_type shortest_side(const vector_type& extents) noexcept
		{
			return static_cast<delta_type>(shortest_extent(extents) * intermediate_float{ 2 });
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr delta_type longest_side(const vector_type& extents) noexcept
		{
			return static_cast<delta_type>(longest_extent(extents) * intermediate_float{ 2 });
		}

		template <typename T = intermediate_float>
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr T MUU_VECTORCALL raw_volume(vector_param extents) noexcept
		{
			return static_cast<T>(extents.x)
				* static_cast<T>(extents.y)
				* static_cast<T>(extents.z)
				* T{ 8 };
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr delta_type MUU_VECTORCALL volume(vector_param extents) noexcept
		{
			return static_cast<delta_type>(raw_volume(extents));
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr delta_type MUU_VECTORCALL mass(vector_param extents, delta_type density) noexcept
		{
			return static_cast<delta_type>(density * raw_volume(extents));
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr delta_type MUU_VECTORCALL density(vector_param extents, delta_type mass) noexcept
		{
			return static_cast<delta_type>(mass / raw_volume(extents));
		}

		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr bool MUU_VECTORCALL degenerate(vector_param extents) noexcept
		{
			return extents.x <= scalar_type{}
				|| extents.y <= scalar_type{}
				|| extents.z <= scalar_type{};
		}

		template <box_corners Corner>
		[[nodiscard]]
		MUU_ATTR(pure)
		static constexpr vector_type MUU_VECTORCALL corner(vector_param center, vector_param extents) noexcept
		{
			static_assert(Corner <= box_corners::max);

			if constexpr (Corner == box_corners::min) // 0
				return vector_type{
					static_cast<scalar_type>(center.x - extents.x),
					static_cast<scalar_type>(center.y - extents.y),
					static_cast<scalar_type>(center.z - extents.z)
				};
			if constexpr (Corner == box_corners::x)
				return vector_type{
					static_cast<scalar_type>(center.x + extents.x),
					static_cast<scalar_type>(center.y - extents.y),
					static_cast<scalar_type>(center.z - extents.z)
				};
			if constexpr (Corner == box_corners::y)
				return vector_type{
					static_cast<scalar_type>(center.x - extents.x),
					static_cast<scalar_type>(center.y + extents.y),
					static_cast<scalar_type>(center.z - extents.z)
				};
			if constexpr (Corner == box_corners::z)
				return vector_type{
					static_cast<scalar_type>(center.x - extents.x),
					static_cast<scalar_type>(center.y - extents.y),
					static_cast<scalar_type>(center.z + extents.z)
				};
			if constexpr (Corner == box_corners::xy)
				return vector_type{
					static_cast<scalar_type>(center.x + extents.x),
					static_cast<scalar_type>(center.y + extents.y),
					static_cast<scalar_type>(center.z - extents.z)
				};
			if constexpr (Corner == box_corners::xz)
				return vector_type{
					static_cast<scalar_type>(center.x + extents.x),
					static_cast<scalar_type>(center.y - extents.y),
					static_cast<scalar_type>(center.z + extents.z)
				};
			if constexpr (Corner == box_corners::yz)
				return vector_type{
					static_cast<scalar_type>(center.x - extents.x),
					static_cast<scalar_type>(center.y + extents.y),
					static_cast<scalar_type>(center.z + extents.z)
			};
			if constexpr (Corner == box_corners::max)
				return vector_type{
					static_cast<scalar_type>(center.x + extents.x),
					static_cast<scalar_type>(center.y + extents.y),
					static_cast<scalar_type>(center.z + extents.z)
			};
		}

		/// \brief	Returns a specific corner of a bounding box.
		[[nodiscard]]
		MUU_ATTR_NDEBUG(pure)
		static constexpr vector_type corner(vector_param center, vector_param extents, box_corners which) noexcept
		{
			MUU_CONSTEXPR_SAFE_ASSERT(
				which <= box_corners::max
				&& "'which' cannot exceed box_corners::max"
			);
			MUU_ASSUME(which <= box_corners::max);

			switch (which)
			{
				case box_corners::min: return corner<box_corners::min>(center, extents);
				case box_corners::x: return corner<box_corners::x>(center, extents);
				case box_corners::y: return corner<box_corners::y>(center, extents);
				case box_corners::xy: return corner<box_corners::xy>(center, extents);
				case box_corners::z: return corner<box_corners::z>(center, extents);
				case box_corners::xz: return corner<box_corners::xz>(center, extents);
				case box_corners::yz: return corner<box_corners::yz>(center, extents);
				case box_corners::max: return corner<box_corners::max>(center, extents);
				MUU_NO_DEFAULT_CASE;
			}
			MUU_UNREACHABLE;
		}
	};
}

MUU_PRAGMA_MSVC(float_control(pop))
MUU_PRAGMA_MSVC(inline_recursion(off))

MUU_POP_WARNINGS; // MUU_DISABLE_SPAM_WARNINGS

/// \endcond

#include "header_end.h"
