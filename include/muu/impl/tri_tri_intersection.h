// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \cond
#include "../triangle.h"
#include "header_start.h"
MUU_FORCE_NDEBUG_OPTIMIZATIONS;
MUU_PRAGMA_MSVC(float_control(except, off))
MUU_DISABLE_ARITHMETIC_WARNINGS;

// the following is an adaptation of work by Tomas Akenine-Möller
// https://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/
//
// Copyright 2020 Tomas Akenine-Möller
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
// to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial
// portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
// OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
// OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#define MUU_TRITRI_SORT(a, b)                                                                                          \
	do                                                                                                                 \
	{                                                                                                                  \
		if (a > b)                                                                                                     \
		{                                                                                                              \
			const auto sort_temp_var = a;                                                                              \
			a						 = b;                                                                              \
			b						 = sort_temp_var;                                                                  \
		}                                                                                                              \
	}                                                                                                                  \
	while (false)

#define MUU_TRITRI_SORT2(a, b, smallest)                                                                               \
	do                                                                                                                 \
	{                                                                                                                  \
		if (a > b)                                                                                                     \
		{                                                                                                              \
			const auto sort_temp_var = a;                                                                              \
			a						 = b;                                                                              \
			b						 = sort_temp_var;                                                                  \
			smallest				 = 1;                                                                              \
		}                                                                                                              \
		else                                                                                                           \
			smallest = 0;                                                                                              \
	}                                                                                                                  \
	while (false)

#define MUU_TRITRI_EDGE_EDGE_TEST(V0, U0, U1)                                                                          \
	do                                                                                                                 \
	{                                                                                                                  \
		const auto Bx = U0[i0] - U1[i0];                                                                               \
		const auto By = U0[i1] - U1[i1];                                                                               \
		const auto Cx = V0[i0] - U0[i0];                                                                               \
		const auto Cy = V0[i1] - U0[i1];                                                                               \
		const auto f  = Ay * Bx - Ax * By;                                                                             \
		const auto d  = By * Cx - Bx * Cy;                                                                             \
                                                                                                                       \
		if ((f > scalar_type{} && d >= scalar_type{} && d <= f)                                                        \
			|| (f < scalar_type{} && d <= scalar_type{} && d >= f))                                                    \
		{                                                                                                              \
			const auto e = Ax * Cy - Ay * Cx;                                                                          \
			if (f > scalar_type{})                                                                                     \
			{                                                                                                          \
				if (e >= scalar_type{} && e <= f)                                                                      \
					return true;                                                                                       \
			}                                                                                                          \
			else                                                                                                       \
			{                                                                                                          \
				if (e <= scalar_type{} && e >= f)                                                                      \
					return true;                                                                                       \
			}                                                                                                          \
		}                                                                                                              \
	}                                                                                                                  \
	while (false)

#define MUU_TRITRI_EDGE_AGAINST_TRI_EDGES(V0, V1, U0, U1, U2)                                                          \
	do                                                                                                                 \
	{                                                                                                                  \
		const auto Ax = V1[i0] - V0[i0];                                                                               \
		const auto Ay = V1[i1] - V0[i1];                                                                               \
		MUU_TRITRI_EDGE_EDGE_TEST(V0, U0, U1);                                                                         \
		MUU_TRITRI_EDGE_EDGE_TEST(V0, U1, U2);                                                                         \
		MUU_TRITRI_EDGE_EDGE_TEST(V0, U2, U0);                                                                         \
	}                                                                                                                  \
	while (false)

#define MUU_TRITRI_POINT_IN_TRI(V0, U0, U1, U2)                                                                        \
	do                                                                                                                 \
	{                                                                                                                  \
		auto a		  = U1[i1] - U0[i1];                                                                               \
		auto b		  = -(U1[i0] - U0[i0]);                                                                            \
		auto c		  = -a * U0[i0] - b * U0[i1];                                                                      \
		const auto d0 = a * V0[i0] + b * V0[i1] + c;                                                                   \
                                                                                                                       \
		a			  = U2[i1] - U1[i1];                                                                               \
		b			  = -(U2[i0] - U1[i0]);                                                                            \
		c			  = -a * U1[i0] - b * U1[i1];                                                                      \
		const auto d1 = a * V0[i0] + b * V0[i1] + c;                                                                   \
                                                                                                                       \
		a			  = U0[i1] - U2[i1];                                                                               \
		b			  = -(U0[i0] - U2[i0]);                                                                            \
		c			  = -a * U2[i0] - b * U2[i1];                                                                      \
		const auto d2 = a * V0[i0] + b * V0[i1] + c;                                                                   \
                                                                                                                       \
		if (d0 * d1 > scalar_type{} && d0 * d2 > scalar_type{})                                                        \
			return true;                                                                                               \
	}                                                                                                                  \
	while (false)

#define MUU_TRITRI_COMPUTE_INTERVALS(VV0, VV1, VV2, D0, D1, D2, D0D1, D0D2, A, B, C, X0, X1)                           \
	do                                                                                                                 \
	{                                                                                                                  \
		if (D0D1 > scalar_type{})                                                                                      \
		{                                                                                                              \
			A  = VV2;                                                                                                  \
			B  = (VV0 - VV2) * D2;                                                                                     \
			C  = (VV1 - VV2) * D2;                                                                                     \
			X0 = D2 - D0;                                                                                              \
			X1 = D2 - D1;                                                                                              \
		}                                                                                                              \
		else if (D0D2 > scalar_type{})                                                                                 \
		{                                                                                                              \
			A  = VV1;                                                                                                  \
			B  = (VV0 - VV1) * D1;                                                                                     \
			C  = (VV2 - VV1) * D1;                                                                                     \
			X0 = D1 - D0;                                                                                              \
			X1 = D1 - D2;                                                                                              \
		}                                                                                                              \
		else if (D1 * D2 > scalar_type{} || D0 != scalar_type{})                                                       \
		{                                                                                                              \
			A  = VV0;                                                                                                  \
			B  = (VV1 - VV0) * D0;                                                                                     \
			C  = (VV2 - VV0) * D0;                                                                                     \
			X0 = D0 - D1;                                                                                              \
			X1 = D0 - D2;                                                                                              \
		}                                                                                                              \
		else if (D1 != scalar_type{})                                                                                  \
		{                                                                                                              \
			A  = VV1;                                                                                                  \
			B  = (VV0 - VV1) * D1;                                                                                     \
			C  = (VV2 - VV1) * D1;                                                                                     \
			X0 = D1 - D0;                                                                                              \
			X1 = D1 - D2;                                                                                              \
		}                                                                                                              \
		else if (D2 != scalar_type{})                                                                                  \
		{                                                                                                              \
			A  = VV2;                                                                                                  \
			B  = (VV0 - VV2) * D2;                                                                                     \
			C  = (VV1 - VV2) * D2;                                                                                     \
			X0 = D2 - D0;                                                                                              \
			X1 = D2 - D1;                                                                                              \
		}                                                                                                              \
		else                                                                                                           \
		{                                                                                                              \
			return coplanar_tri_tri(N1, V0, V1, V2, U0, U1, U2);                                                       \
		}                                                                                                              \
	}                                                                                                                  \
	while (false)

namespace muu::impl
{
	template <typename Scalar>
	struct tri_tri_intersection_tester
	{
		using scalar_type			  = Scalar;
		using vector_type			  = vector<scalar_type, 3>;
		using vector_param			  = muu::vector_param<vector_type>;
		static constexpr auto EPSILON = default_epsilon<scalar_type>;

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL intersects(vector_param V0,
														vector_param V1,
														vector_param V2,
														vector_param U0,
														vector_param U1,
														vector_param U2) noexcept
		{
			const auto E1 = V1 - V0;
			const auto E2 = V2 - V0;
			const auto N1 = vector_type::cross(E1, E2);
			const auto d1 = -vector_type::dot(N1, V0);

			const auto du0 = vector_type::dot(N1, U0) + d1;
			const auto du1 = vector_type::dot(N1, U1) + d1;
			const auto du2 = vector_type::dot(N1, U2) + d1;
			if (muu::abs(du0) < EPSILON)
				du0 = scalar_type{};
			if (muu::abs(du1) < EPSILON)
				du1 = scalar_type{};
			if (muu::abs(du2) < EPSILON)
				du2 = scalar_type{};
			const auto du0du1 = du0 * du1;
			const auto du0du2 = du0 * du2;
			if (du0du1 > scalar_type{} && du0du2 > scalar_type{})
				return false;

			E1			  = U1 - U0;
			E2			  = U2 - U0;
			const auto N2 = vector_type::cross(E1, E2);
			const auto d2 = -vector_type::dot(N2, U0);

			const auto dv0 = vector_type::dot(N2, V0) + d2;
			const auto dv1 = vector_type::dot(N2, V1) + d2;
			const auto dv2 = vector_type::dot(N2, V2) + d2;
			if (muu::abs(dv0) < EPSILON)
				dv0 = scalar_type{};
			if (muu::abs(dv1) < EPSILON)
				dv1 = scalar_type{};
			if (muu::abs(dv2) < EPSILON)
				dv2 = scalar_type{};
			const auto dv0dv1 = dv0 * dv1;
			const auto dv0dv2 = dv0 * dv2;
			if (dv0dv1 > scalar_type{} && dv0dv2 > scalar_type{})
				return false;

			const auto D = vector_type::cross(N1, N2);

			unsigned index = {};
			auto max	   = muu::abs(D[0]);
			if (const auto new_max = muu::abs(D[1]); new_max > max)
			{
				max	  = new_max;
				index = 1;
			}
			if (const auto new_max = muu::abs(D[2]); new_max > max)
			{
				max	  = new_max;
				index = 2;
			}

			const auto vp0 = V0[index];
			const auto vp1 = V1[index];
			const auto vp2 = V2[index];

			const auto up0 = U0[index];
			const auto up1 = U1[index];
			const auto up2 = U2[index];

			scalar_type a, b, c, x0, x1;
			MUU_TRITRI_COMPUTE_INTERVALS(vp0, vp1, vp2, dv0, dv1, dv2, dv0dv1, dv0dv2, a, b, c, x0, x1);

			scalar_type d, e, f, y0, y1;
			MUU_TRITRI_COMPUTE_INTERVALS(up0, up1, up2, du0, du1, du2, du0du1, du0du2, d, e, f, y0, y1);

			const auto xx	= x0 * x1;
			const auto yy	= y0 * y1;
			const auto xxyy = xx * yy;

			auto tmp			  = a * xxyy;
			scalar_type isect1[2] = { tmp + b * x1 * yy, tmp + c * x0 * yy };

			tmp					  = d * xxyy;
			scalar_type isect2[2] = { tmp + e * xx * y1, tmp + f * xx * y0 };

			MUU_TRITRI_SORT(isect1[0], isect1[1]);
			MUU_TRITRI_SORT(isect2[0], isect2[1]);

			if (isect1[1] < isect2[0] || isect2[1] < isect1[0])
				return false;

			return true;
		}

#if 0

		MUU_PURE_GETTER
		static constexpr bool MUU_VECTORCALL coplanar_tri_tri(vector_param N,
															  vector_param V0,
															  vector_param V1,
															  vector_param V2,
															  vector_param U0,
															  vector_param U1,
															  vector_param U2) noexcept
		{
			unsigned i0, i1;
			const vector_type A = vector_type::abs(N);
			if (A[0] > A[1])
			{
				if (A[0] > A[2])
				{
					i0 = 1; /* A[0] is greatest */
					i1 = 2;
				}
				else
				{
					i0 = 0; /* A[2] is greatest */
					i1 = 1;
				}
			}
			else /* A[0]<=A[1] */
			{
				if (A[2] > A[1])
				{
					i0 = 0; /* A[2] is greatest */
					i1 = 1;
				}
				else
				{
					i0 = 0; /* A[1] is greatest */
					i1 = 2;
				}
			}

			/* test all edges of triangle 1 against the edges of triangle 2 */
			MUU_TRITRI_EDGE_AGAINST_TRI_EDGES(V0, V1, U0, U1, U2);
			MUU_TRITRI_EDGE_AGAINST_TRI_EDGES(V1, V2, U0, U1, U2);
			MUU_TRITRI_EDGE_AGAINST_TRI_EDGES(V2, V0, U0, U1, U2);

			/* finally, test if tri1 is totally contained in tri2 or vice versa */
			MUU_TRITRI_POINT_IN_TRI(V0, U0, U1, U2);
			MUU_TRITRI_POINT_IN_TRI(U0, V0, V1, V2);

			return false;
		}

		inline void isect2(float VTX0[3],
			float VTX1[3],
			float VTX2[3],
			float VV0,
			float VV1,
			float VV2,
			float D0,
			float D1,
			float D2,
			float* isect0,
			float* isect1,
			float isectpoint0[3],
			float isectpoint1[3])
		{
			float tmp = D0 / (D0 - D1);
			float diff[3];
			*isect0 = VV0 + (VV1 - VV0) * tmp;
			diff = VTX1 - VTX0;
			diff = tmp * diff;
			isectpoint0 = diff + VTX0;
			tmp = D0 / (D0 - D2);
			*isect1 = VV0 + (VV2 - VV0) * tmp;
			diff = VTX2 - VTX0;
			diff = tmp * diff;
			isectpoint1 = VTX0 + diff;
		}

		inline int compute_intervals_isectline(float VERT0[3],
			float VERT1[3],
			float VERT2[3],
			float VV0,
			float VV1,
			float VV2,
			float D0,
			float D1,
			float D2,
			float D0D1,
			float D0D2,
			float* isect0,
			float* isect1,
			float isectpoint0[3],
			float isectpoint1[3])
		{
			if (D0D1 > 0.0f)
			{
				/* here we know that D0D2<=0.0 */
				/* that is D0, D1 are on the same side, D2 on the other or on the plane */
				isect2(VERT2, VERT0, VERT1, VV2, VV0, VV1, D2, D0, D1, isect0, isect1, isectpoint0, isectpoint1);
			}
			else if (D0D2 > 0.0f)
			{
				/* here we know that d0d1<=0.0 */
				isect2(VERT1, VERT0, VERT2, VV1, VV0, VV2, D1, D0, D2, isect0, isect1, isectpoint0, isectpoint1);
			}
			else if (D1 * D2 > 0.0f || D0 != 0.0f)
			{
				/* here we know that d0d1<=0.0 or that D0!=0.0 */
				isect2(VERT0, VERT1, VERT2, VV0, VV1, VV2, D0, D1, D2, isect0, isect1, isectpoint0, isectpoint1);
			}
			else if (D1 != 0.0f)
			{
				isect2(VERT1, VERT0, VERT2, VV1, VV0, VV2, D1, D0, D2, isect0, isect1, isectpoint0, isectpoint1);
			}
			else if (D2 != 0.0f)
			{
				isect2(VERT2, VERT0, VERT1, VV2, VV0, VV1, D2, D0, D1, isect0, isect1, isectpoint0, isectpoint1);
			}
			else
			{
				/* triangles are coplanar */
				return 1;
			}
			return 0;
		}

		int tri_tri_intersect_with_isectline(float V0[3],
			float V1[3],
			float V2[3],
			float U0[3],
			float U1[3],
			float U2[3],
			int* coplanar,
			float isectpt1[3],
			float isectpt2[3])
		{
			float E1[3], E2[3];
			float N1[3], N2[3], d1, d2;
			float du0, du1, du2, dv0, dv1, dv2;
			float D[3];
			float isect1[2], isect2[2];
			float isectpointA1[3], isectpointA2[3];
			float isectpointB1[3], isectpointB2[3];
			float du0du1, du0du2, dv0dv1, dv0dv2;
			short index;
			float vp0, vp1, vp2;
			float up0, up1, up2;
			float b, c, max;
			float tmp, diff[3];
			int smallest1, smallest2;

			/* compute plane equation of triangle(V0,V1,V2) */
			E1 = V1 - V0;
			E2 = V2 - V0;
			N1 = vector_type::cross(E1, E2);
			d1 = -vector_type::dot(N1, V0);
			/* plane equation 1: N1.X+d1=0 */

			/* put U0,U1,U2 into plane equation 1 to compute signed distances to the plane*/
			du0 = vector_type::dot(N1, U0) + d1;
			du1 = vector_type::dot(N1, U1) + d1;
			du2 = vector_type::dot(N1, U2) + d1;

			/* coplanarity robustness check */
			if (muu::abs(du0) < EPSILON)
				du0 = 0.0;
			if (muu::abs(du1) < EPSILON)
				du1 = 0.0;
			if (muu::abs(du2) < EPSILON)
				du2 = 0.0;
			du0du1 = du0 * du1;
			du0du2 = du0 * du2;

			if (du0du1 > 0.0f && du0du2 > 0.0f) /* same sign on all of them + not equal 0 ? */
				return 0;						/* no intersection occurs */

			/* compute plane of triangle (U0,U1,U2) */
			E1 = U1 - U0;
			E2 = U2 - U0;
			N2 = vector_type::cross(E1, E2);
			d2 = -vector_type::dot(N2, U0);
			/* plane equation 2: N2.X+d2=0 */

			/* put V0,V1,V2 into plane equation 2 */
			dv0 = vector_type::dot(N2, V0) + d2;
			dv1 = vector_type::dot(N2, V1) + d2;
			dv2 = vector_type::dot(N2, V2) + d2;
			if (muu::abs(dv0) < EPSILON)
				dv0 = 0.0;
			if (muu::abs(dv1) < EPSILON)
				dv1 = 0.0;
			if (muu::abs(dv2) < EPSILON)
				dv2 = 0.0;

			dv0dv1 = dv0 * dv1;
			dv0dv2 = dv0 * dv2;

			if (dv0dv1 > 0.0f && dv0dv2 > 0.0f) /* same sign on all of them + not equal 0 ? */
				return 0;						/* no intersection occurs */

			/* compute direction of intersection line */
			D = vector_type::cross(N1, N2);

			/* compute and index to the largest component of D */
			max = muu::abs(D[0]);
			index = 0;
			b = muu::abs(D[1]);
			c = muu::abs(D[2]);
			if (b > max)
				max = b, index = 1;
			if (c > max)
				max = c, index = 2;

			/* this is the simplified projection onto L*/
			vp0 = V0[index];
			vp1 = V1[index];
			vp2 = V2[index];

			up0 = U0[index];
			up1 = U1[index];
			up2 = U2[index];

			/* compute interval for triangle 1 */
			*coplanar = compute_intervals_isectline(V0,
				V1,
				V2,
				vp0,
				vp1,
				vp2,
				dv0,
				dv1,
				dv2,
				dv0dv1,
				dv0dv2,
				&isect1[0],
				&isect1[1],
				isectpointA1,
				isectpointA2);
			if (*coplanar)
				return coplanar_tri_tri(N1, V0, V1, V2, U0, U1, U2);

			/* compute interval for triangle 2 */
			compute_intervals_isectline(U0,
				U1,
				U2,
				up0,
				up1,
				up2,
				du0,
				du1,
				du2,
				du0du1,
				du0du2,
				&isect2[0],
				&isect2[1],
				isectpointB1,
				isectpointB2);

			MUU_TRITRI_SORT2(isect1[0], isect1[1], smallest1);
			MUU_TRITRI_SORT2(isect2[0], isect2[1], smallest2);

			if (isect1[1] < isect2[0] || isect2[1] < isect1[0])
				return 0;

			/* at this point, we know that the triangles intersect */

			if (isect2[0] < isect1[0])
			{
				if (smallest1 == 0)
				{
					isectpt1 = isectpointA1;
				}
				else
				{
					isectpt1 = isectpointA2;
				}

				if (isect2[1] < isect1[1])
				{
					if (smallest2 == 0)
					{
						isectpt2 = isectpointB2;
					}
					else
					{
						isectpt2 = isectpointB1;
					}
				}
				else
				{
					if (smallest1 == 0)
					{
						isectpt2 = isectpointA2;
					}
					else
					{
						isectpt2 = isectpointA1;
					}
				}
			}
			else
			{
				if (smallest2 == 0)
				{
					isectpt1 = isectpointB1;
				}
				else
				{
					isectpt1 = isectpointB2;
				}

				if (isect2[1] > isect1[1])
				{
					if (smallest1 == 0)
					{
						isectpt2 = isectpointA2;
					}
					else
					{
						isectpt2 = isectpointA1;
					}
				}
				else
				{
					if (smallest2 == 0)
					{
						isectpt2 = isectpointB2;
					}
					else
					{
						isectpt2 = isectpointB1;
					}
				}
			}
			return 1;
		}

#endif
	};
}

namespace muu
{
	template <typename Scalar>
	MUU_PURE_INLINE_GETTER
	constexpr bool MUU_VECTORCALL triangle<Scalar>::intersects(MUU_VPARAM(triangle) tri1,
															   MUU_VPARAM(triangle) tri2) noexcept
	{
		return impl::tri_tri_intersection_tester<Scalar>::intersects(tri1[0],
																	 tri1[1],
																	 tri1[2],
																	 tri2[0],
																	 tri2[1],
																	 tri2[2]);
	}
}

#undef MUU_TRITRI_SORT
#undef MUU_TRITRI_SORT2
#undef MUU_TRITRI_EDGE_EDGE_TEST
#undef MUU_TRITRI_EDGE_AGAINST_TRI_EDGES
#undef MUU_TRITRI_POINT_IN_TRI
#undef MUU_TRITRI_COMPUTE_INTERVALS

MUU_RESET_NDEBUG_OPTIMIZATIONS;
#include "header_end.h"
/// \endcond
