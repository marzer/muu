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
	name : 'supports C++20',
	args : [ '-std=c++2a' ]
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

compiler_supports_float16_args = []
if compiler.get_id() == 'gcc'
	compiler_supports_float16_args += '-mfp16-format=ieee'
endif
compiler_supports_float16 = compiler.links('''
	int main()
	{
		static_assert(sizeof(_Float16) == 2);
		_Float16 f = static_cast<_Float16>(1);
		return 0;
	}
	''',
	name : 'supports float16',
	args : compiler_supports_float16_args
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
			
			if compiler_supports_float16
				if compiler.get_id() == 'gcc'
					args += '-mfp16-format=ieee'
				endif
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
	endforeach # fast_math
endforeach # cpp20

foreach executable : executables
	test(executable[0], executable[1])
endforeach
