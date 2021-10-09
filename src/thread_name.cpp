// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "muu/impl/thread_name.h"
#include "muu/strings.h"
#include "os.h"

#include "source_start.h"
MUU_DISABLE_SUGGEST_WARNINGS;

//======================================================================================================================
#if MUU_WINDOWS
//======================================================================================================================

MUU_DISABLE_WARNINGS;
	#if MUU_WIN10_SDK >= 1607
		#include <processthreadsapi.h> // SetThreadDescription()
	#endif
MUU_ENABLE_WARNINGS;

namespace
{
	// https://docs.microsoft.com/en-us/visualstudio/debugger/how-to-set-a-thread-name-in-native-code?view=vs-2019

	static constexpr DWORD MS_VC_EXCEPTION = 0x406D1388;

	#pragma pack(push, 8)
	struct THREADNAME_INFO
	{
		DWORD dwType;
		LPCSTR szName;
		DWORD dwThreadID;
		DWORD dwFlags;
	};
	#pragma pack(pop)

	static void set_thread_name_legacy(const std::string_view& name)
	{
		const THREADNAME_INFO info{ 0x1000u, name.data(), GetCurrentThreadId(), {} };
		__try
		{
			RaiseException(MS_VC_EXCEPTION,
						   0,
						   sizeof(info) / sizeof(ULONG_PTR),
						   reinterpret_cast<const ULONG_PTR*>(&info));
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{}
	}

	static void set_thread_name_os_specific(string_param&& name_)
	{
		// ensure zero-termination
		auto str = static_cast<std::string_view>(name_);
		char buf[256];
		std::strncpy(buf, str.data(), muu::min(str.length(), 256_sz));
		buf[255] = '\0';
		str		 = std::string_view{ buf, str.length() };

		set_thread_name_legacy(str);

	// 'modern'
	#if MUU_WIN10_SDK >= 1607
		SetThreadDescription(GetCurrentThread(), transcode<wchar_t>(str).c_str());
	#endif
	}
}

//======================================================================================================================
#else // other
//======================================================================================================================

namespace
{
	// MUU_ATTR(const)
	static void set_thread_name_os_specific(string_param&& /*name_*/)
	{}
}

#endif // other

//======================================================================================================================
// PUBLIC INTERFACE
//======================================================================================================================

extern "C" //
{
	void MUU_CALLCONV muu_impl_set_thread_name(muu::string_param* name) noexcept
	{
		MUU_ASSUME(name != nullptr);

#if MUU_HAS_EXCEPTIONS

		try
		{
			::set_thread_name_os_specific(MUU_MOVE(*name));
		}
		catch (...)
		{}

#else
		::set_thread_name_os_specific(MUU_MOVE(*name));
#endif
	}
}
