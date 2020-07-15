test_sources = [
	'accumulator.cpp',
	'aligned_alloc.cpp',
	'blob.cpp',
	'core.cpp',
	'float16.cpp',
	'hashing.cpp',
	'main.cpp',
	'meta.cpp',
	'scope_guard.cpp',
	'tagged_ptr.cpp',
	'tests.cpp',
	'uuid.cpp',
]

fast_math_modes = [ true, false ]
executables = []
counter = 0
foreach fast_math : fast_math_modes
	
	name = 'tests'
	overrides = []
	args = []

	args += '-DTEST_BUILD_ID=@0@'.format(counter)

	if fast_math
		name = name + '_fast-math'
		if compiler.get_id() == 'gcc' or compiler.get_id() == 'clang'
			args += '-ffast-math'
			args += '-ffp-contract=fast'
		endif
	endif

	executables += [[
		name,
		executable(
			name,
			test_sources,
			include_directories : inc,
			cpp_args : args,
			override_options : overrides
		)
	]]

	counter = counter + 1

endforeach # fast_math

foreach executable : executables
	test(executable[0], executable[1])
endforeach
