// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#ifndef MUU_PREPROCESSOR_MACRO_DISPATCH_GCC_H
#define MUU_PREPROCESSOR_MACRO_DISPATCH_GCC_H
/// \cond

#ifndef __GNUC__
	#error __GNUC__ must be defined to use this header!
#endif

#if __GNUC__ >= 1
	#define MUU_MACRO_DISPATCH_GCC_GE_1(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_1(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_1(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_1(...) __VA_ARGS__
#endif
#if __GNUC__ >= 2
	#define MUU_MACRO_DISPATCH_GCC_GE_2(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_2(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_2(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_2(...) __VA_ARGS__
#endif
#if __GNUC__ >= 3
	#define MUU_MACRO_DISPATCH_GCC_GE_3(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_3(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_3(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_3(...) __VA_ARGS__
#endif
#if __GNUC__ >= 4
	#define MUU_MACRO_DISPATCH_GCC_GE_4(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_4(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_4(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_4(...) __VA_ARGS__
#endif
#if __GNUC__ >= 5
	#define MUU_MACRO_DISPATCH_GCC_GE_5(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_5(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_5(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_5(...) __VA_ARGS__
#endif
#if __GNUC__ >= 6
	#define MUU_MACRO_DISPATCH_GCC_GE_6(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_6(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_6(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_6(...) __VA_ARGS__
#endif
#if __GNUC__ >= 7
	#define MUU_MACRO_DISPATCH_GCC_GE_7(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_7(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_7(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_7(...) __VA_ARGS__
#endif
#if __GNUC__ >= 8
	#define MUU_MACRO_DISPATCH_GCC_GE_8(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_8(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_8(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_8(...) __VA_ARGS__
#endif
#if __GNUC__ >= 9
	#define MUU_MACRO_DISPATCH_GCC_GE_9(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_9(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_9(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_9(...) __VA_ARGS__
#endif
#if __GNUC__ >= 10
	#define MUU_MACRO_DISPATCH_GCC_GE_10(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_10(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_10(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_10(...) __VA_ARGS__
#endif
#if __GNUC__ >= 11
	#define MUU_MACRO_DISPATCH_GCC_GE_11(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_11(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_11(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_11(...) __VA_ARGS__
#endif
#if __GNUC__ >= 12
	#define MUU_MACRO_DISPATCH_GCC_GE_12(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_12(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_12(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_12(...) __VA_ARGS__
#endif
#if __GNUC__ >= 13
	#define MUU_MACRO_DISPATCH_GCC_GE_13(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_13(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_13(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_13(...) __VA_ARGS__
#endif
#if __GNUC__ >= 14
	#define MUU_MACRO_DISPATCH_GCC_GE_14(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_14(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_14(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_14(...) __VA_ARGS__
#endif
#if __GNUC__ >= 15
	#define MUU_MACRO_DISPATCH_GCC_GE_15(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_15(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_15(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_15(...) __VA_ARGS__
#endif
#if __GNUC__ >= 16
	#define MUU_MACRO_DISPATCH_GCC_GE_16(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_16(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_16(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_16(...) __VA_ARGS__
#endif
#if __GNUC__ >= 17
	#define MUU_MACRO_DISPATCH_GCC_GE_17(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_17(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_17(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_17(...) __VA_ARGS__
#endif
#if __GNUC__ >= 18
	#define MUU_MACRO_DISPATCH_GCC_GE_18(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_18(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_18(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_18(...) __VA_ARGS__
#endif
#if __GNUC__ >= 19
	#define MUU_MACRO_DISPATCH_GCC_GE_19(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_19(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_19(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_19(...) __VA_ARGS__
#endif
#if __GNUC__ >= 20
	#define MUU_MACRO_DISPATCH_GCC_GE_20(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_20(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_20(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_20(...) __VA_ARGS__
#endif
#if __GNUC__ >= 21
	#define MUU_MACRO_DISPATCH_GCC_GE_21(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_21(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_21(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_21(...) __VA_ARGS__
#endif
#if __GNUC__ >= 22
	#define MUU_MACRO_DISPATCH_GCC_GE_22(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_22(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_22(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_22(...) __VA_ARGS__
#endif
#if __GNUC__ >= 23
	#define MUU_MACRO_DISPATCH_GCC_GE_23(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_23(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_23(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_23(...) __VA_ARGS__
#endif
#if __GNUC__ >= 24
	#define MUU_MACRO_DISPATCH_GCC_GE_24(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_24(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_24(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_24(...) __VA_ARGS__
#endif
#if __GNUC__ >= 25
	#define MUU_MACRO_DISPATCH_GCC_GE_25(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_25(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_25(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_25(...) __VA_ARGS__
#endif
#if __GNUC__ >= 26
	#define MUU_MACRO_DISPATCH_GCC_GE_26(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_26(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_26(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_26(...) __VA_ARGS__
#endif
#if __GNUC__ >= 27
	#define MUU_MACRO_DISPATCH_GCC_GE_27(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_27(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_27(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_27(...) __VA_ARGS__
#endif
#if __GNUC__ >= 28
	#define MUU_MACRO_DISPATCH_GCC_GE_28(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_28(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_28(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_28(...) __VA_ARGS__
#endif
#if __GNUC__ >= 29
	#define MUU_MACRO_DISPATCH_GCC_GE_29(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_29(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_29(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_29(...) __VA_ARGS__
#endif
#if __GNUC__ >= 30
	#define MUU_MACRO_DISPATCH_GCC_GE_30(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_30(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_30(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_30(...) __VA_ARGS__
#endif
#if __GNUC__ >= 31
	#define MUU_MACRO_DISPATCH_GCC_GE_31(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_31(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_31(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_31(...) __VA_ARGS__
#endif
#if __GNUC__ >= 32
	#define MUU_MACRO_DISPATCH_GCC_GE_32(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_32(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_32(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_32(...) __VA_ARGS__
#endif
#if __GNUC__ >= 33
	#define MUU_MACRO_DISPATCH_GCC_GE_33(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_33(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_33(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_33(...) __VA_ARGS__
#endif
#if __GNUC__ >= 34
	#define MUU_MACRO_DISPATCH_GCC_GE_34(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_34(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_34(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_34(...) __VA_ARGS__
#endif
#if __GNUC__ >= 35
	#define MUU_MACRO_DISPATCH_GCC_GE_35(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_35(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_35(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_35(...) __VA_ARGS__
#endif
#if __GNUC__ >= 36
	#define MUU_MACRO_DISPATCH_GCC_GE_36(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_36(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_36(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_36(...) __VA_ARGS__
#endif
#if __GNUC__ >= 37
	#define MUU_MACRO_DISPATCH_GCC_GE_37(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_37(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_37(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_37(...) __VA_ARGS__
#endif
#if __GNUC__ >= 38
	#define MUU_MACRO_DISPATCH_GCC_GE_38(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_38(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_38(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_38(...) __VA_ARGS__
#endif
#if __GNUC__ >= 39
	#define MUU_MACRO_DISPATCH_GCC_GE_39(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_39(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_39(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_39(...) __VA_ARGS__
#endif
#if __GNUC__ >= 40
	#define MUU_MACRO_DISPATCH_GCC_GE_40(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_40(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_40(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_40(...) __VA_ARGS__
#endif
#if __GNUC__ >= 41
	#define MUU_MACRO_DISPATCH_GCC_GE_41(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_41(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_41(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_41(...) __VA_ARGS__
#endif
#if __GNUC__ >= 42
	#define MUU_MACRO_DISPATCH_GCC_GE_42(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_42(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_42(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_42(...) __VA_ARGS__
#endif
#if __GNUC__ >= 43
	#define MUU_MACRO_DISPATCH_GCC_GE_43(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_43(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_43(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_43(...) __VA_ARGS__
#endif
#if __GNUC__ >= 44
	#define MUU_MACRO_DISPATCH_GCC_GE_44(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_44(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_44(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_44(...) __VA_ARGS__
#endif
#if __GNUC__ >= 45
	#define MUU_MACRO_DISPATCH_GCC_GE_45(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_45(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_45(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_45(...) __VA_ARGS__
#endif
#if __GNUC__ >= 46
	#define MUU_MACRO_DISPATCH_GCC_GE_46(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_46(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_46(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_46(...) __VA_ARGS__
#endif
#if __GNUC__ >= 47
	#define MUU_MACRO_DISPATCH_GCC_GE_47(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_47(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_47(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_47(...) __VA_ARGS__
#endif
#if __GNUC__ >= 48
	#define MUU_MACRO_DISPATCH_GCC_GE_48(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_48(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_48(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_48(...) __VA_ARGS__
#endif
#if __GNUC__ >= 49
	#define MUU_MACRO_DISPATCH_GCC_GE_49(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_49(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_49(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_49(...) __VA_ARGS__
#endif
#if __GNUC__ >= 50
	#define MUU_MACRO_DISPATCH_GCC_GE_50(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_GCC_LT_50(...)
#else
	#define MUU_MACRO_DISPATCH_GCC_GE_50(...)
	#define MUU_MACRO_DISPATCH_GCC_LT_50(...) __VA_ARGS__
#endif

/// \endcond
#endif // MUU_PREPROCESSOR_MACRO_DISPATCH_GCC_H
