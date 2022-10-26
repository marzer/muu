# This file is a part of muu and is subject to the the terms of the MIT license.
# Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

project(
	'muu',
	'cpp',
	license : 'MIT',
	version : '0.2.0',
	meson_version : '>=0.63.0',
	default_options : [ # https://mesonbuild.com/Builtin-options.html
		# core options
		'buildtype=release',
		'default_library=shared',
		# base options
		'b_lto=true',
		'b_ndebug=if-release',
		# compiler options
		'cpp_std=c++17'
	]
)

is_devel = get_option('devel')
is_debug = get_option('debug')
is_release = not is_debug
is_pedantic = get_option('pedantic') or is_devel
is_windows = host_machine.system() == 'windows'
is_x64 = host_machine.cpu_family() == 'x86_64'
is_subproject = meson.is_subproject()

compiler = meson.get_compiler('cpp')
is_gcc = compiler.get_id() == 'gcc'
is_clang = compiler.get_id() == 'clang'
is_msvc = compiler.get_id() == 'msvc'
is_icc_cl = compiler.get_id() == 'intel-cl'
is_icc = is_icc_cl or compiler.get_id() == 'intel'
is_lld = compiler.get_linker_id() == 'ld.lld'

has_exceptions = get_option('cpp_eh') != 'none'

#=======================================================================================================================
# internal_args
#
# these are the arguments common internally when compiling the library and tests.
# *** they are not forwarded to dependents when using this as a submodule. ***
#=======================================================================================================================

internal_args = []

# internal args on clang
if is_clang
	if is_pedantic
		internal_args += '-Weverything'
	endif
	if get_option('time_trace')
		internal_args += '-ftime-trace'
	endif
	internal_args += [
		'-ferror-limit=5',
		'-Wno-unused-command-line-argument',
		'-Wno-c++98-compat',
		'-Wno-c++98-compat-pedantic',
		'-Wno-documentation',
		'-Wno-documentation-unknown-command',
		'-Wno-pch-date-time',
	]
endif

# internal args on GCC
if is_gcc
	if is_pedantic
		internal_args += [
			'-Wcast-align',
			'-Wcast-qual',
			'-Wctor-dtor-privacy',
			'-Wdisabled-optimization',
			'-Wfloat-equal',
			'-Wimport',
			'-Winit-self',
			'-Wlogical-op',
			'-Wmissing-declarations',
			'-Wmissing-field-initializers',
			'-Wmissing-format-attribute',
			'-Wmissing-include-dirs',
			'-Wmissing-noreturn',
			'-Wold-style-cast',
			'-Woverloaded-virtual',
			'-Wpacked',
			'-Wpadded',
			'-Wpointer-arith',
			'-Wredundant-decls',
			'-Wshadow',
			'-Wsign-conversion',
			'-Wsign-promo',
			'-Wstack-protector',
			'-Wstrict-null-sentinel',
			'-Wswitch-default',
			'-Wswitch-enum',
			'-Wundef',
			'-Wunreachable-code',
			'-Wunused',
			'-Wunused-parameter',
			'-Wuseless-cast',
			'-Wvariadic-macros',
			'-Wwrite-strings',
			'-Wmissing-noreturn',
		]
	endif
	if is_release and is_pedantic
		internal_args += [
			'-Wsuggest-attribute=const',
			'-Wsuggest-attribute=pure',
		]
	endif
	internal_args += [
		'-fmax-errors=5',
		'-Wno-init-list-lifetime',
	]
endif

# internal args on MSVC
if is_msvc
	internal_args += [
		'/bigobj',
		'/Gy', # function-level linking
		'/GF', # string pooling
		'/openmp-',
		'/permissive-',
		'/utf-8',
		'/volatile:iso',
		'/Zc:__cplusplus',
		'/Zc:inline',
		'/Zc:throwingNew',
		'/Zc:externConstexpr',
		'/Zc:preprocessor'
	]
	if is_pedantic
		internal_args += '/W4'
	endif
endif

# internal args when targeting windows
if is_windows
	internal_args += '-D_CRT_SECURE_NO_WARNINGS=1'
endif

#=======================================================================================================================
# internal_dependencies
#
# these are the dependencies common internally when compiling the library and tests.
# *** they are not forwarded to dependents when using this as a submodule. ***
#=======================================================================================================================

internal_dependencies = []

if not is_windows
	internal_dependencies += dependency('threads')
endif

#=======================================================================================================================
# internal_overrides
#
# these are the dependencies common internally when compiling the library and tests.
# *** they are not forwarded to dependents when using this as a submodule. ***
#=======================================================================================================================

internal_overrides = []

if is_lld or is_debug or (is_windows and is_clang)
	internal_overrides += 'b_lto=false'
endif
if is_pedantic
	internal_overrides += [ 'werror=true', 'warning_level=3' ]
endif

#=======================================================================================================================
# c++ 20 check
#=======================================================================================================================

compiler_supports_cpp20_args = compiler.first_supported_argument(
	'-std=c++20', '/Qstd=c++20', '/std:c++20',
	'-std=c++2a', '/Qstd=c++2a', '/std:c++latest')
compiler_supports_cpp20 = compiler_supports_cpp20_args.length() > 0 and compiler.links('''
	#include <version>
	#include <string>
	#include <iostream>
	#include <cstdint>
	#include <cstddef>
	#include <cstring>
	#include <cfloat>
	#include <climits>
	#include <cmath>
	#include <limits>
	#include <memory>
	#include <iosfwd>
	#include <type_traits>

	int main()
	{
		std::string s = "kek";
		std::cout << s << std::endl;
		return 0;
	}
	''',
	name: 'supports c++20',
	args: compiler_supports_cpp20_args
)

#=======================================================================================================================
# char8_t check
#=======================================================================================================================

compiler_supports_char8_args = compiler.first_supported_argument('-fchar8_t')
compiler_supports_char8 = compiler_supports_cpp20 and compiler.links('''
	#include <version>
	#include <string_view>
	#include <string>
	#include <type_traits>
	using namespace std::string_view_literals;

	#if !defined(__cpp_char8_t)	|| __cpp_char8_t < 201811 || !defined(__cpp_lib_char8_t) || __cpp_lib_char8_t < 201907
		#error oh noes
	#endif

	static_assert(!std::is_same_v<char, char8_t>);
	static_assert(!std::is_same_v<std::string, std::u8string>);

	std::u8string func()
	{
		return std::u8string{ u8"this is a test."sv };
	}

	int main()
	{
		return 0;
	}
	''',
	name: 'supports char8_t',
	args: [ compiler_supports_cpp20_args, compiler_supports_char8_args ]
)

#=======================================================================================================================
# __fp16 and _Float16
#=======================================================================================================================

compiler_supports_float16_args = []
if get_option('float16')
	compiler_supports_float16_args += compiler.get_supported_arguments('-mfp16-format=ieee')
else
	internal_args += '-DMUU_HAS_FP16=0'
	internal_args += '-DMUU_HAS_FLOAT16=0'
endif
compiler_supports_fp16 = get_option('float16') and compiler.links('''
	int main()
	{
		static_assert(sizeof(__fp16) == 2);
		__fp16 f = static_cast<__fp16>(1);
		const auto f2 = static_cast<float>(f);
		const auto f3 = static_cast<__fp16>(0.2L);
		return 0;
	}
	''',
	name : 'supports __fp16',
	args : compiler_supports_float16_args
)
compiler_supports_float16 = get_option('float16') and compiler.links('''
	int main()
	{
		static_assert(sizeof(_Float16) == 2);
		_Float16 f = static_cast<_Float16>(1);
		const auto f2 = static_cast<float>(f);
		const auto f3 = static_cast<_Float16>(0.2L);
		return 0;
	}
	''',
	name : 'supports _Float16',
	args : compiler_supports_float16_args
)
if compiler_supports_fp16 or compiler_supports_float16
	internal_args += compiler_supports_float16_args
endif

#=======================================================================================================================
# int128 check
#=======================================================================================================================

compiler_supports_int128 = compiler.links('''
	#ifndef __SIZEOF_INT128__
		#error __SIZEOF_INT128__ wasn't defined!
	#endif

	#include <cstdint>

	int main()
	{
		static_assert(__SIZEOF_INT128__ == 16);
		static_assert(sizeof(__int128_t) == 16);
		static_assert(sizeof(__uint128_t) == 16);
		__int128_t i = static_cast<__int128_t>(1);
		const auto i2 = static_cast<int64_t>(i);
		const auto i3 = static_cast<int32_t>(i);
		return 0;
	}
	''',
	name: 'supports __int128_t'
)

#=======================================================================================================================
# float128 check
#=======================================================================================================================

compiler_supports_float128 = compiler.links('''
	#ifndef __SIZEOF_FLOAT128__
		#error __SIZEOF_FLOAT128__ wasn't defined!
	#endif
	#ifndef __FLT128_MANT_DIG__
		#error __FLT128_MANT_DIG__ wasn't defined!
	#endif
	#ifndef __LDBL_MANT_DIG__
		#error __LDBL_MANT_DIG__ wasn't defined!
	#endif
	#if __FLT128_MANT_DIG__ <= __LDBL_MANT_DIG__
		#error __FLT128_MANT_DIG__ was <= __LDBL_MANT_DIG__
	#endif
	int main()
	{
		static_assert(__SIZEOF_FLOAT128__ == 16);
		static_assert(sizeof(__float128) == 16);
		__float128 f = static_cast<__float128>(1);
		const auto f2 = static_cast<long double>(f);
		const auto f3 = static_cast<double>(f);
		return 0;
	}
	''',
	name: 'supports __float128'
)
if compiler_supports_float128
	internal_dependencies += compiler.find_library('quadmath')
endif

#=======================================================================================================================
# fext-numeric-literals
#=======================================================================================================================

if compiler_supports_fp16 or compiler_supports_float16 or compiler_supports_float128 or compiler_supports_int128
	internal_args += compiler.get_supported_arguments('-fext-numeric-literals')
endif

#=======================================================================================================================
# subdirectories
#=======================================================================================================================

subdir('include')

subdir('src')

if (get_option('build_tests') or is_devel) and not is_subproject
	subdir('tests')
endif

if (get_option('build_examples') or is_devel) and not is_subproject
	subdir('examples')
endif
