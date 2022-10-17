lib_sources = [
	'hashing.cpp',
	'vector_types_common.cpp',
	'string_param.cpp',
	'thread_name.cpp',
	'thread_pool.cpp',
	'uuid.cpp',
]

lib_public_args = []
if get_option('default_library') != 'static'
	lib_public_args += '-DMUU_DLL=1'
endif

lib_internal_args = [ '-DMUU_BUILDING=1']
lib_internal_args += universal_args
if compiler_supports_cpp20
	lib_internal_args += compiler_supports_cpp20_args
endif
if compiler_supports_char8
	lib_internal_args += compiler_supports_char8_args
endif
if is_windows
	lib_internal_args += '-D_CRT_SECURE_NO_WARNINGS=1'
endif

lib_suffix = []
if is_windows
	if get_option('default_library') == 'shared'
		lib_suffix = 'dll'
	else
		lib_suffix = 'lib'
	endif
endif

lib_overrides = []
lib_overrides += overrides
if compiler_supports_cpp20
	lib_overrides += 'cpp_std=none'
endif

lib_dependencies = []
if not is_windows
	lib_dependencies += dependency('threads')
endif
if compiler_supports_float128
	lib_dependencies += compiler.find_library('quadmath')
endif

lib = library(
	meson.project_name(),
	lib_sources,
	cpp_args: [ lib_internal_args, lib_public_args ],
	gnu_symbol_visibility: 'hidden',
	include_directories: include_dir,
	install: not is_subproject,
	version: meson.project_version(),
	override_options: lib_overrides,
	name_suffix: lib_suffix,
	dependencies: lib_dependencies
)

muu_dep = declare_dependency(
	compile_args: lib_public_args,
	include_directories: include_dir,
	link_with: lib
)
