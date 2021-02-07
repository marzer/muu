test_sources = [
	'main.cpp',
	'tests.cpp'
]
test_set = get_option('test_set')
if test_set == 0 or test_set == 1
	test_sources += 'accumulator.cpp'
	test_sources += 'aligned_alloc.cpp'
	test_sources += 'blob.cpp'
	test_sources += 'bounding_box_char.cpp'
	test_sources += 'bounding_box_double.cpp'
	test_sources += 'compressed_pair.cpp'
	test_sources += 'core_bit.cpp'
	test_sources += 'matrix_char.cpp'
	test_sources += 'matrix_double.cpp'
	test_sources += 'oriented_bounding_box_double.cpp'
	test_sources += 'quaternion_double.cpp'
	test_sources += 'unicode_char.cpp'
	test_sources += 'unicode_char8_t.cpp'
	test_sources += 'vector_char.cpp'
	test_sources += 'vector_double.cpp'
	if compiler_supports_fp16
		test_sources += 'bounding_box_fp16.cpp'
		test_sources += 'matrix_fp16.cpp'
		test_sources += 'oriented_bounding_box_fp16.cpp'
		test_sources += 'quaternion_fp16.cpp'
		test_sources += 'vector_fp16.cpp'
	endif
endif
if test_set == 0 or test_set == 2
	test_sources += 'bounding_box_float.cpp'
	test_sources += 'bounding_box_half.cpp'
	test_sources += 'core_meta.cpp'
	test_sources += 'core_other.cpp'
	test_sources += 'half.cpp'
	test_sources += 'hashing.cpp'
	test_sources += 'matrix_float.cpp'
	test_sources += 'matrix_half.cpp'
	test_sources += 'oriented_bounding_box_float.cpp'
	test_sources += 'quaternion_float.cpp'
	test_sources += 'unicode_char16_t.cpp'
	test_sources += 'vector_float.cpp'
	test_sources += 'vector_half.cpp'
	test_sources += 'vector_long_double.cpp'
	if compiler_supports_float16
		test_sources += 'bounding_box_Float16.cpp'
		test_sources += 'matrix_Float16.cpp'
		test_sources += 'oriented_bounding_box_Float16.cpp'
		test_sources += 'quaternion_Float16.cpp'
		test_sources += 'vector_Float16.cpp'
	endif
endif
if test_set == 0 or test_set == 3
	test_sources += 'bounding_box_int.cpp'
	test_sources += 'bounding_box_long.cpp'
	test_sources += 'math.cpp'
	test_sources += 'matrix_int.cpp'
	test_sources += 'matrix_long.cpp'
	test_sources += 'matrix_long_double.cpp'
	test_sources += 'oriented_bounding_box_half.cpp'
	test_sources += 'quaternion_half.cpp'
	test_sources += 'scope_guard.cpp'
	test_sources += 'span.cpp'
	test_sources += 'string_param.cpp'
	test_sources += 'unicode_char32_t.cpp'
	test_sources += 'vector_int.cpp'
	test_sources += 'vector_long.cpp'
	if compiler_supports_int128
		test_sources += 'bounding_box_int128.cpp'
		test_sources += 'matrix_int128.cpp'
		test_sources += 'vector_int128.cpp'
	endif
endif
if test_set == 0 or test_set == 4
	test_sources += 'bounding_box_long_double.cpp'
	test_sources += 'bounding_box_long_long.cpp'
	test_sources += 'bounding_box_short.cpp'
	test_sources += 'matrix_long_long.cpp'
	test_sources += 'matrix_short.cpp'
	test_sources += 'oriented_bounding_box_long_double.cpp'
	test_sources += 'quaternion_long_double.cpp'
	test_sources += 'strings.cpp'
	test_sources += 'tagged_ptr.cpp'
	test_sources += 'thread_pool.cpp'
	test_sources += 'unicode_unsigned_char.cpp'
	test_sources += 'uuid.cpp'
	test_sources += 'vector_long_long.cpp'
	test_sources += 'vector_short.cpp'
	if compiler_supports_float128
		test_sources += 'bounding_box_float128.cpp'
		test_sources += 'matrix_float128.cpp'
		test_sources += 'oriented_bounding_box_float128.cpp'
		test_sources += 'quaternion_float128.cpp'
		test_sources += 'vector_float128.cpp'
	endif
endif

test_dependencies = []
if compiler_supports_float128
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
