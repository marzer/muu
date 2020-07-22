test_sources = [
	'accumulator.cpp',
	'aligned_alloc.cpp',
	'blob.cpp',
	'core.cpp',
	'half.cpp',
	'hashing.cpp',
	'main.cpp',
	'meta.cpp',
	'scope_guard.cpp',
	'tagged_ptr.cpp',
	'tests.cpp',
	'uuid.cpp',
]

compiler_supports_cpp20 = compiler.links('''
	#include <version>
	#include <string>
	#include <iostream>

	int main()
	{
		std::string s = "kek";
		std::cout << s << std::endl;
		return 0;
	}
	''',
	name : 'supports c++20',
	args : [ '-std=c++2a' ]
)

compiler_supports_big_endian = compiler.compiles('''
	#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
		#define YAY
	#endif
	#ifndef YAY
		#error oh noes
	#endif
	int main()
	{
		return 0;
	}
	''',
	name : 'supports big-endian',
	args : [ '-mbig-endian' ]
)

compiler_supports_char8 = compiler_supports_cpp20 and compiler.links('''
	#include <version>
	#include <string_view>
	#include <string>
	#include <type_traits>
	using namespace std::string_view_literals;

	#ifndef __cpp_lib_char8_t
		#error oh noes
	#endif

	static_assert(!std::is_same_v<char, char8_t>);
	static_assert(!std::is_same_v<std::string, std::u8string>);

	std::u8string func()
	{
		return std::u8string{ u8"this is a test."sv };
	}

	int main()
	{
		return 0;
	}
	''',
	name : 'supports char8_t',
	args : [ '-std=c++2a', '-fchar8_t' ]
)

compiler_supports_consteval = compiler_supports_cpp20 and compiler.compiles('''

	consteval int kek() noexcept
	{
		return 42;
	}

	int main()
	{
		return kek();
	}
	''',
	name : 'supports consteval',
	args : [ '-std=c++2a' ]
)

compiler_supports_ieee_float16_args = [ '-O0' ]
if compiler.get_id() == 'gcc'
	compiler_supports_ieee_float16_args += '-mfp16-format=ieee'
endif
compiler_supports_ieee_float16 = compiler.links('''
	#if !defined(__FLT_RADIX__)					\
			|| !defined(__FLT16_MANT_DIG__)		\
			|| !defined(__FLT16_MAX_EXP__)		\
			|| __FLT_RADIX__ != 2				\
			|| __FLT16_MANT_DIG__ != 11			\
			|| __FLT16_DIG__ != 3				\
			|| __FLT16_MIN_EXP__ != -14			\
			|| __FLT16_MIN_10_EXP__ != -13		\
			|| __FLT16_MAX_EXP__ != 15			\
			|| __FLT16_MAX_10_EXP__ != 4
		#error oh noes
	#endif

	int main()
	{
		static_assert(sizeof(_Float16) == 2);
		_Float16 f = static_cast<_Float16>(1);
		const auto f2 = static_cast<float>(_Float16);
		const auto f3 = static_cast<_Float16>(0.2L);
		return 0;
	}
	''',
	name : 'supports iec559 float16',
	args : compiler_supports_ieee_float16_args
)

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
	args : [ '-ffast-math', '-ffp-contract=fast' ]
)

fast_math_modes = [ false, true ]
exception_modes = [ false, true ]
big_endian_modes = [ false, true ]
cpp20_modes = [ false, true ]
executables = []
counter = 0

foreach cpp20 : cpp20_modes
	if cpp20 and not compiler_supports_cpp20
		continue
	endif
	foreach fast_math : fast_math_modes
		if fast_math and not compiler_supports_fast_math
			continue
		endif
		foreach big_endian : big_endian_modes
			if big_endian and not compiler_supports_big_endian
				continue
			endif
			foreach exceptions : exception_modes
				name = ''
				overrides = []
				args = []
	
				if cpp20
					name = 'cpp20'
					overrides += 'cpp_std=none'
					args += '-std=c++2a'
	
					if compiler_supports_char8
						args += '-fchar8_t'
					endif
				else
					name = 'cpp17'
				endif
				
				if not exceptions
					name = name + '_noexcept'
					overrides += 'cpp_eh=none'
				endif
	
				if fast_math
					name = name + '_fastmath'
					if compiler.get_id() == 'gcc' or compiler.get_id() == 'clang'
						args += '-ffast-math'
						args += '-ffp-contract=fast'
					endif
				endif

				if big_endian
					name = name + '_be'
					args += '-mbig-endian'
					args += '-DSHOULD_BE_BIG_ENDIAN=1'
				endif
				
				if compiler_supports_ieee_float16
					if compiler.get_id() == 'gcc'
						args += '-mfp16-format=ieee'
					endif
					args += '-DSHOULD_HAVE_FLOAT16=1'
				endif
				
				if counter % 4 == 2
					args += '-DMUU_ALL_INLINE=1'
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

			endforeach # exceptions
		endforeach # big_endian
	endforeach # fast_math
endforeach # cpp20

foreach executable : executables
	test(executable[0], executable[1])
endforeach
