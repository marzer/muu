test_sources = [
	'accumulator.cpp',
	'aligned_alloc.cpp',
	'blob.cpp',
	'compressed_pair.cpp',
	'core_bit.cpp',
	'core_math.cpp',
	'core_meta.cpp',
	'core_other.cpp',
	'half.cpp',
	'hashing.cpp',
	'main.cpp',
	'quaternion_double.cpp',
	'quaternion_float.cpp',
	'quaternion_half.cpp',
	'quaternion_long_double.cpp',
	'scope_guard.cpp',
	'span.cpp',
	'string_param.cpp',
	'strings.cpp',
	'tagged_ptr.cpp',
	'tests.cpp',
	'thread_pool.cpp',
	'unicode_char.cpp',
	'unicode_char8_t.cpp',
	'unicode_char16_t.cpp',
	'unicode_char32_t.cpp',
	'unicode_unsigned_char.cpp',
	'uuid.cpp',
	'vector_char.cpp',
	'vector_double.cpp',
	'vector_float.cpp',
	'vector_half.cpp',
	'vector_int.cpp',
	'vector_long.cpp',
	'vector_long_double.cpp',
	'vector_long_long.cpp',
	'vector_short.cpp',
]
test_dependencies = []
if compiler_supports_fp16
	test_sources += 'quaternion_fp16.cpp'
	test_sources += 'vector_fp16.cpp'
endif
if compiler_supports_float16
	test_sources += 'quaternion_Float16.cpp'
	test_sources += 'vector_Float16.cpp'
endif
if compiler_supports_int128
	test_sources += 'vector_int128.cpp'
endif
if compiler_supports_float128
	test_sources += 'quaternion_float128.cpp'
	test_sources += 'vector_float128.cpp'
	test_dependencies += compiler.find_library('quadmath')
endif

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
					link_with: [ muu_lib ],
					dependencies: test_dependencies,
				)
			]]

		endforeach # exceptions
	endforeach # fast_math
endforeach # cpp20

foreach executable : test_executables
	test(executable[0], executable[1], workdir : meson.source_root() / 'tests')
endforeach
