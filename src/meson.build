# This file is a part of muu and is subject to the the terms of the MIT license.
# Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

lib_sources = [
	'hashing.cpp',
	'vector_types_common.cpp',
	'string_param.cpp',
	'thread_name.cpp',
	'thread_pool.cpp',
	'uuid.cpp',
]

lib_public_args = []
if compiler_supports_fp16 or compiler_supports_float16 or compiler_supports_float128 or compiler_supports_int128
	lib_public_args += compiler.get_supported_arguments('-fext-numeric-literals')
endif
if not get_option('float16')
	lib_public_args += '-DMUU_HAS_FP16=0'
	lib_public_args += '-DMUU_HAS_FLOAT16=0'
endif
lib_static_public_args = lib_public_args + [ '-DMUU_DLL=0' ]
lib_shared_public_args = lib_public_args + [ '-DMUU_DLL=1' ]

lib_internal_args = [ '-DMUU_BUILDING=1' ]
lib_internal_args += internal_args
if not is_msvc
	if compiler_supports_cpp20
		lib_internal_args += compiler_supports_cpp20_args
	endif
	if compiler_supports_char8
		lib_internal_args += compiler_supports_char8_args
	endif
endif
if is_windows
	lib_internal_args += has_exceptions ? '-D_HAS_EXCEPTIONS=1' : '-D_HAS_EXCEPTIONS=0'
endif

lib_dependencies = []
lib_dependencies += internal_dependencies

lib_overrides = []
lib_overrides += internal_overrides
if is_msvc
	lib_overrides += 'cpp_std=vc++latest'
elif compiler_supports_cpp20
	lib_overrides += 'cpp_std=none' # switch is set above
endif

lib_extra_files = []
lib_extra_files += extra_files

# static lib
lib_static = false
if get_option('default_library') in [ 'static', 'both' ]
	lib_static = static_library(
		meson.project_name(),
		lib_sources,
		cpp_args: [ lib_internal_args, lib_static_public_args ],
		gnu_symbol_visibility: 'hidden',
		include_directories: include_dir,
		dependencies: lib_dependencies,
		override_options: lib_overrides,
		install: false,
		extra_files: lib_extra_files
	)
endif

# shared lib
lib_shared = false
if get_option('default_library') in [ 'shared', 'both' ]
	lib_shared = shared_library(
		meson.project_name(),
		lib_sources,
		cpp_args: [ lib_internal_args, lib_shared_public_args ],
		gnu_symbol_visibility: 'hidden',
		include_directories: include_dir,
		version: meson.project_version(),
		dependencies: lib_dependencies,
		override_options: lib_overrides,
		install: true,
		extra_files: lib_extra_files
	)
endif

muu_dep = declare_dependency(
	compile_args: get_option('default_library') == 'static' ? lib_static_public_args : lib_shared_public_args,
	link_with: get_option('default_library') == 'static' ? lib_static : lib_shared,
	include_directories: include_dir
)

meson.override_dependency(meson.project_name(), muu_dep)
