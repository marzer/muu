// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#pragma once
#include "muu/preprocessor.h"

MUU_DISABLE_WARNINGS;

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define OEMRESOURCE
#define NOATOM //            - Atom Manager routines
#define NOAPISET
#define NOBITMAP
#define NOCLIPBOARD //       - Clipboard routines
#define NOCOLOR //           - Screen colors
#define NOCOMM //            - COMM driver routines
#define NOCTLMGR //          - Control and Dialog routines
#define NODEFERWINDOWPOS //  - DeferWindowPos routines
#define NODRAWTEXT //        - DrawText() and DT_*
#define NOGDI //             - All GDI defines and routines
#define NOGDICAPMASKS //     - CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOHELP //            - Help engine interface.
#define NOICONS //           - IDI_*
#define NOKANJI //           - Kanji support stuff.
#define NOKERNEL //          - All KERNEL defines and routines
#define NOKEYSTATES //       - MK_*
#define NOMB //              - MB_* and MessageBox()
#define NOMCX //             - Modem Configuration Extensions
#define NOMEMMGR //          - GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMENUS //           - MF_*
#define NOMETAFILE //        - typedef METAFILEPICT
#define NOMINMAX //          - Macros min(a,b) and max(a,b)
//#define NOMSG //             - typedef MSG and associated routines
#define NONLS //             - All NLS defines and routines
#define NOOPENFILE //        - OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOPROFILER //        - Profiler interface.
#define NORASTEROPS //       - Binary and Tertiary raster ops
#define NOSCROLL //          - SB_* and scrolling routines
#define NOSERVICE //         - All Service Controller routines, SERVICE_ equates, etc.
#define NOSHOWWINDOW //      - SW_*
#define NOSOUND //           - Sound driver routines
#define NOSYSCOMMANDS //     - SC_*
#define NOSYSMETRICS //      - SM_*
#define NOTEXTMETRIC //      - typedef TEXTMETRIC and associated routines
//#define NOUSER //            - All USER defines and routines
#define NOVIRTUALKEYCODES // - VK_*
#define NOWH //              - SetWindowsHook and WH_*
#define NOWINMESSAGES //     - WM_*, EM_*, LB_*, CB_*
#define NOWINOFFSETS //      - GWL_*, GCL_*, associated routines
#define NOWINSTYLES //       - WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*

#include <Windows.h>
#include <objbase.h> // CoInitializeEx, CoUninitialize

MUU_ENABLE_WARNINGS;

// these mappings assembled from:
// https://en.wikipedia.org/wiki/Microsoft_Windows_SDK
// https://en.wikipedia.org/wiki/Windows_10_version_history
// sdkddkver.h
#if WDK_NTDDI_VERSION >=  0x0A000008	// NTDDI_WIN10_VB	/ 2004	/ 20H1			/ 10.0.19041.0
	#define MUU_WIN10_SDK 2004
#elif WDK_NTDDI_VERSION >=  0x0A000007	// NTDDI_WIN10_19H1	/ 1903	/ 19H1			/ 10.0.18362.0
	#define MUU_WIN10_SDK 1903
#elif WDK_NTDDI_VERSION >=  0x0A000006	// NTDDI_WIN10_RS5	/ 1809	/ Redstone 5	/ 10.0.17763.0
	#define MUU_WIN10_SDK 1809
#elif WDK_NTDDI_VERSION >=  0x0A000005	// NTDDI_WIN10_RS4	/ 1803	/ Redstone 4	/ 10.0.17134.0
	#define MUU_WIN10_SDK 1803
#elif WDK_NTDDI_VERSION >=  0x0A000004	// NTDDI_WIN10_RS3	/ 1709	/ Redstone 3	/ 10.0.16299.0
	#define MUU_WIN10_SDK 1709
#elif WDK_NTDDI_VERSION >=  0x0A000003	// NTDDI_WIN10_RS2	/ 1703	/ Redstone 2	/ 10.0.15063.0
	#define MUU_WIN10_SDK 1703
#elif WDK_NTDDI_VERSION >=  0x0A000002	// NTDDI_WIN10_RS1	/ 1607	/ Redstone 1	/ 10.0.14393.0
	#define MUU_WIN10_SDK 1607
#elif WDK_NTDDI_VERSION >=  0x0A000001	// NTDDI_WIN10_TH2	/ 1511	/ Threshold 2	/ 10.0.10586.0
	#define MUU_WIN10_SDK 1511
#elif WDK_NTDDI_VERSION >=  0x0A000000	// NTDDI_WIN10		/ 1507	/ Threshold 1	/ 10.0.10240.0
	#define MUU_WIN10_SDK 1507
#else
	#define MUU_WIN10_SDK 0
#endif
