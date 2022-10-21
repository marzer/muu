// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

/// \file
/// \brief Contains the definition of #muu::set_thread_name().

#include "string_param.h"
#include "impl/header_start.h"

/// \cond
extern "C" //
{
	MUU_API
	MUU_ATTR(nonnull)
	void MUU_CALLCONV muu_impl_set_thread_name(muu::string_param*) noexcept;
}
/// \endcond

namespace muu
{
	/// \brief		Sets the name of the current thread for debuggers.
	/// \ingroup	strings
	MUU_ALWAYS_INLINE
	void set_thread_name(string_param name) noexcept
	{
		::muu_impl_set_thread_name(&name);
	}
}

#include "impl/header_end.h"
