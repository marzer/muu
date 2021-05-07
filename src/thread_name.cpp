// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "muu/strings.h"
#include "os.h"

#include "source_start.h"
MUU_DISABLE_SUGGEST_WARNINGS;

//======================================================================================================================
#if MUU_WINDOWS
//======================================================================================================================

	#if MUU_WIN10_SDK >= 1607
MUU_DISABLE_WARNINGS;
		#include <processthreadsapi.h> // SetThreadDescription()
MUU_ENABLE_WARNINGS;
	#endif

namespace
{
	// https://docs.microsoft.com/en-us/visualstudio/debugger/how-to-set-a-thread-name-in-native-code?view=vs-2019

	constexpr DWORD MS_VC_EXCEPTION = 0x406D1388;

	#pragma pack(push, 8)
	struct THREADNAME_INFO
	{
		DWORD dwType;
		LPCSTR szName;
		DWORD dwThreadID;
		DWORD dwFlags;
	};
	#pragma pack(pop)

	static void set_thread_name_legacy(const std::string& name) noexcept
	{
		THREADNAME_INFO info;
		info.dwType		= 0x1000u;
		info.szName		= name.c_str();
		info.dwThreadID = GetCurrentThreadId();
		info.dwFlags	= {};
		__try
		{
			RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), pointer_cast<ULONG_PTR*>(&info));
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{}
	}

	static void set_thread_name_os_specific(string_param&& name_) noexcept
	{
		std::string name(std::move(name_)); // ensure zero-termination
		set_thread_name_legacy(name);

	// 'modern'
	#if MUU_WIN10_SDK >= 1607
		SetThreadDescription(GetCurrentThread(), transcode<wchar_t>(name).c_str());
	#endif
	}
}

//======================================================================================================================
#else // other
//======================================================================================================================

namespace
{
	// MUU_ATTR(const)
	static void set_thread_name_os_specific(string_param&& /*name_*/) noexcept
	{}
}

#endif // other

//======================================================================================================================
// PUBLIC INTERFACE
//======================================================================================================================

namespace muu
{
	void set_thread_name(string_param name) noexcept
	{
		set_thread_name_os_specific(std::move(name));
	}

	//
	// std::string_view thread_name() noexcept
	//{
	//	return {};
	//}

}
