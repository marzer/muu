muu_lib_sources = [
	'blob.cpp',
	'generic_allocator.cpp',
	'hashing.cpp',
	'vector_types_common.cpp',
	'string_param.cpp',
	'thread_name.cpp',
	'thread_pool.cpp',
	'uuid.cpp',
]

muu_lib_args = [ '-DMUU_BUILDING=1' ]
muu_lib_args += additional_arguments
muu_lib_suffix = []
if is_windows
	if get_option('default_library') == 'shared'
		muu_lib_suffix = 'dll'
	else
		muu_lib_suffix = 'lib'
		if get_option('static_dllexport')
			muu_lib_args += '-DMUU_API=__declspec(dllexport)'
		endif
	endif
endif

muu_lib_overrides = []
muu_lib_overrides +=  overrides
if compiler_supports_cpp20
	muu_lib_overrides += 'cpp_std=none'
	muu_lib_args += compiler_supports_cpp20_args
	if compiler_supports_char8
		muu_lib_args += compiler_supports_char8_args
	endif
endif

muu_dependencies = []
if not is_windows
	muu_dependencies += dependency('threads')
endif
if compiler_supports_float128
	muu_dependencies += compiler.find_library('quadmath')
endif

muu_lib = library(
	'muu',
	muu_lib_sources,
	include_directories: include_dirs,
	override_options: muu_lib_overrides,
	cpp_args: muu_lib_args,
	name_suffix: muu_lib_suffix,
	dependencies: muu_dependencies,
	install: true
)
