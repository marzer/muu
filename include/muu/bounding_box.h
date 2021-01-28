// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

/// \file
/// \brief  Contains the definition of muu::bounding_box.

#pragma once
#include "../muu/vector.h"

MUU_PUSH_WARNINGS;
MUU_PRAGMA_MSVC(inline_recursion(on))
MUU_PRAGMA_MSVC(float_control(push))
MUU_PRAGMA_MSVC(float_control(except, off))
MUU_PRAGMA_MSVC(float_control(precise, off))
MUU_PRAGMA_MSVC(push_macro("min"))
MUU_PRAGMA_MSVC(push_macro("max"))
#if MUU_MSVC
	#undef min
	#undef max
#endif

namespace muu
{
	MUU_ABI_VERSION_START(0);

	/// \brief An axis-aligned bounding box.
	/// \ingroup math
	/// 
	/// \tparam	Scalar      The type of the bounding box's scalar components.
	/// 
	/// \see [Axis-aligned bounding box](https://en.wikipedia.org/w/index.php?title=Axis-aligned_bounding_box)
	template <typename Scalar>
	struct MUU_TRIVIAL_ABI bounding_box
	{
	};

	MUU_ABI_VERSION_END;
}

MUU_PRAGMA_MSVC(pop_macro("min"))
MUU_PRAGMA_MSVC(pop_macro("max"))
MUU_PRAGMA_MSVC(float_control(pop))
MUU_PRAGMA_MSVC(inline_recursion(off))
MUU_POP_WARNINGS;
