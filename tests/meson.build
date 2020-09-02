test_sources = [
	'accumulator.cpp',
	'aligned_alloc.cpp',
	'blob.cpp',
	'compressed_pair.cpp',
	'core_intrinsics_bit.cpp',
	'core_intrinsics_other.cpp',
	'core_meta.cpp',
	'half.cpp',
	'hashing.cpp',
	'main.cpp',
	'scope_guard.cpp',
	'span.cpp',
	'strings.cpp',
	'tagged_ptr.cpp',
	'tests.cpp',
	'thread_pool.cpp',
	'uuid.cpp',
]

#######################################################################################################################
# fast math check
#######################################################################################################################

compiler_supports_fast_math_args = []
if is_gcc or is_clang
	compiler_supports_fast_math_args += '-ffast-math'
	compiler_supports_fast_math_args += '-ffp-contract=fast'
elif is_msvc or is_icc_cl
	compiler_supports_fast_math_args += '/fp:fast'
endif
compiler_supports_fast_math = compiler.links('''
	#include <cmath>
	#include <iostream>
	int main()
	{
		std::cout << std::exp2(2.0) << std::pow(2.0, 3.0) << std::endl;
		return 0;
	}
	''',
	name : 'supports fast-math',
	args : compiler_supports_fast_math_args
)

#######################################################################################################################
# do the thing!
#######################################################################################################################

fast_math_modes = [ false, true ]
exception_modes = [ false, true ]
cpp20_modes = [ false, true ]
test_executables = []

foreach cpp20 : cpp20_modes
	if cpp20 and not compiler_supports_cpp20
		continue
	endif
	foreach fast_math : fast_math_modes
		if fast_math and not compiler_supports_fast_math
			continue
		endif
		foreach exceptions : exception_modes
			test_name = ''
			test_overrides = []
			test_overrides += overrides
			test_args = []
			test_args += additional_arguments
	
			if cpp20
				test_name = 'cpp20'
				test_overrides += 'cpp_std=none'
				test_args += compiler_supports_cpp20_args
				if compiler_supports_char8
					test_args += compiler_supports_char8_args
				endif
			else
				test_name = 'cpp17'
			endif
				
			if exceptions
				test_overrides += 'cpp_eh=default'
				test_args += '-DSHOULD_HAVE_EXCEPTIONS=1'
				if is_windows
					test_args += '-D_HAS_EXCEPTIONS=1'
				endif
			else
				test_name = test_name + '_noexcept'
				test_overrides += 'cpp_eh=none'
				test_args += '-DSHOULD_HAVE_EXCEPTIONS=0'
				if is_windows
					test_args += '-D_HAS_EXCEPTIONS=0'
				endif
			endif
	
			if fast_math
				test_name = test_name + '_fastmath'
				test_args += compiler_supports_fast_math_args
			endif

			if compiler_supports_float16 or compiler_supports_fp16
				if compiler_supports_fp16
					test_args += '-DSHOULD_HAVE_FP16=1'
				endif
				if compiler_supports_float16
					test_args += '-DSHOULD_HAVE_FLOAT16=1'
				endif
			endif
			if compiler_supports_int128
				test_args += '-DSHOULD_HAVE_INT128=1'
			endif
			if compiler_supports_float128
				test_args += '-DSHOULD_HAVE_FLOAT128=1'
			endif

			test_executables += [[
				test_name,
				executable(
					test_name,
					test_sources,
					include_directories: include_dirs,
					cpp_args: test_args,
					override_options: test_overrides,
					link_with: [ muu_lib ]
				)
			]]

		endforeach # exceptions
	endforeach # fast_math
endforeach # cpp20

foreach executable : test_executables
	test(executable[0], executable[1], workdir : meson.source_root() / 'tests')
endforeach
