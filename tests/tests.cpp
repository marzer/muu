// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"
#include "../include/muu/half.h"

MUU_NAMESPACE_START
{
	std::ostream& operator << (std::ostream& os, const half& value)
	{
		return os << static_cast<float>(value);
	}
}
MUU_NAMESPACE_END
