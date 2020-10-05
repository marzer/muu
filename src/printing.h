// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "muu/core.h"
MUU_DISABLE_WARNINGS
#include <ostream>
#include <iomanip>
MUU_ENABLE_WARNINGS

MUU_PUSH_WARNINGS
MUU_DISABLE_SPAM_WARNINGS

MUU_NAMESPACE_START
{
	template <typename Char>
	struct stream_saver final
	{
		std::basic_ios<Char>& stream;
		std::ios_base::fmtflags flags;
		std::streamsize precision;
		std::streamsize width;
		Char fill;

		stream_saver(std::basic_ios<Char>& os) noexcept
			: stream{ os },
			flags{ stream.flags() },
			precision{ stream.precision() },
			width{ stream.width() },
			fill{ stream.fill() }
		{}

		stream_saver(const stream_saver&) = delete;
		stream_saver(stream_saver&&) = delete;
		stream_saver& operator = (const stream_saver&) = delete;
		stream_saver& operator = (stream_saver&&) = delete;

		~stream_saver() noexcept
		{
			stream.flags(flags);
			stream.precision(precision);
			stream.width(width);
			stream.fill(fill);
		}
	};

	template <typename Char>
	stream_saver(std::basic_ios<Char>&) -> stream_saver<Char>;
}
MUU_NAMESPACE_END

MUU_POP_WARNINGS // MUU_DISABLE_SPAM_WARNINGS
