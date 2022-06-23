test_sources = [
	'main.cpp',
	'tests.cpp'
]
test_sources += [
	'accumulator.cpp',
	'bit.cpp',
	'blob.cpp',
	'bounding_box_0.cpp',
	'bounding_box_1.cpp',
	'bounding_box_2.cpp',
	'bounding_box_3.cpp',
	'compressed_pair.cpp',
	'core_other.cpp',
	'function_view.cpp',
	'half.cpp',
	'hashing.cpp',
	'infinity_or_nan_0.cpp',
	'infinity_or_nan_1.cpp',
	'infinity_or_nan_2.cpp',
	'infinity_or_nan_3.cpp',
	'integral_range.cpp',
	'line_segment_0.cpp',
	'line_segment_1.cpp',
	'line_segment_2.cpp',
	'line_segment_3.cpp',
	'math.cpp',
	'matrix_0.cpp',
	'matrix_1.cpp',
	'matrix_2.cpp',
	'matrix_3.cpp',
	'matrix_4.cpp',
	'matrix_5.cpp',
	'matrix_6.cpp',
	'matrix_7.cpp',
	'meta.cpp',
	'oriented_bounding_box_0.cpp',
	'oriented_bounding_box_1.cpp',
	'oriented_bounding_box_2.cpp',
	'oriented_bounding_box_3.cpp',
	'packed_unit_vector_0.cpp',
	'packed_unit_vector_1.cpp',
	'packed_unit_vector_2.cpp',
	'packed_unit_vector_3.cpp',
	'plane_0.cpp',
	'plane_1.cpp',
	'plane_2.cpp',
	'plane_3.cpp',
	'quaternion_0.cpp',
	'quaternion_1.cpp',
	'quaternion_2.cpp',
	'quaternion_3.cpp',
	'scope_guard.cpp',
	'span.cpp',
	'static_string.cpp',
	'string_param.cpp',
	'strings.cpp',
	'tagged_ptr.cpp',
	'thread_pool.cpp',
	'triangle_0.cpp',
	'triangle_1.cpp',
	'triangle_2.cpp',
	'triangle_3.cpp',
	'type_list.cpp',
	'type_name.cpp',
	'unicode_char.cpp',
	'unicode_char16_t.cpp',
	'unicode_char32_t.cpp',
	'unicode_char8_t.cpp',
	'unicode_unsigned_char.cpp',
	'uuid.cpp',
	'vector_0.cpp',
	'vector_1.cpp',
	'vector_2.cpp',
	'vector_3.cpp',
	'vector_4.cpp',
	'vector_5.cpp',
	'vector_6.cpp',
	'vector_7.cpp',
	'vector_misc.cpp',
]

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

fast_math_modes = [ false, true  ]
exception_modes = [ true , false  ]
cpp20_modes     = [ true , false ]
test_executables = []
test_additional_arguments = []
if is_gcc
	# precompiled headers fuck up warning pragmas in GCC =/
	test_additional_arguments += [
		'-Wno-float-equal',
		'-Wno-sign-conversion',
		'-Wno-attributes',
		'-Wno-shadow',
		'-Wno-all',
		'-Wno-extra',
		'-Wno-pedantic',
	]
endif

foreach cpp20 : cpp20_modes
	if cpp20 and not compiler_supports_cpp20
		continue
	endif
	foreach fast_math : fast_math_modes
		if fast_math and not compiler_supports_fast_math
			continue
		endif
		foreach exceptions : exception_modes

			# fast_math + noexcept, or neither; not one or the other
			if fast_math == exceptions
				continue
			endif

			test_name = ''
			test_overrides = []
			test_overrides += overrides
			test_args = []
			test_args += additional_arguments
			test_args += test_additional_arguments
	
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

			# if is_release and (is_gcc or is_clang)
			# 	test_overrides += 'optimization=2'
			# endif

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
					cpp_pch : 'pch/gcc_pch.h'
				)
			]]

		endforeach # exceptions
	endforeach # fast_math
endforeach # cpp20

foreach executable : test_executables
	test(executable[0], executable[1], workdir : meson.project_source_root() / 'tests')
endforeach
