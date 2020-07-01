args = []

if compiler.get_id() == 'gcc' or compiler.get_id() == 'clang'
	args += [ '-Wno-switch', '-Wno-switch-default', '-Wno-switch-enum' ]
endif

simple_parser = executable(
	'example',
	[ 'example.cpp' ],
	include_directories : inc,
	cpp_args : args
)
