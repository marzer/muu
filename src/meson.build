muu_lib_sources = [
	'aligned_alloc.cpp',
	'blob.cpp',
	'hashing.cpp',
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

muu_dependencies = []
if not is_windows
	muu_dependencies += dependency('threads')
endif

muu_lib = library(
	'muu',
	muu_lib_sources,
	include_directories: include_dirs,
	override_options: overrides,
	cpp_args: muu_lib_args,
	name_suffix: muu_lib_suffix,
	dependencies: muu_dependencies,
	install: true
)
