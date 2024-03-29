#!/usr/bin/env python3
# This file is a part of muu and is subject to the the terms of the MIT license.
# Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

import utils
from pathlib import Path
from io import StringIO



def main():
	root_dir = Path(utils.entry_script_dir(), '..').resolve()
	impl_dir = Path(root_dir, 'include', 'muu', 'impl')
	utils.assert_existing_directory(root_dir)
	utils.assert_existing_directory(impl_dir)

	compilers = [
		("clang", "__clang_major__", 1, 50),
		("gcc", "__GNUC__", 1, 50),
		("msvc", "_MSC_VER", 1920, 1970),
	]

	for name, version_macro, min_version, max_version in compilers:
		file_path = Path(impl_dir, rf'preprocessor_macro_dispatch_{name}.h')
		name_upper = name.upper()
		guard_name = rf'MUU_PREPROCESSOR_MACRO_DISPATCH_{name_upper}_H'
		with StringIO() as buf:
			print(rf'''// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
//-----
// this file was generated by a script - do not modify it directly
//-----
#ifndef {guard_name}
#define {guard_name}
/// \cond

#ifndef {version_macro}
	#error {version_macro} must be defined to use this header!
#endif
''', file=buf)

			for version in range(min_version, max_version+1):
				print(rf'''
#if {version_macro} >= {version}
	#define MUU_MACRO_DISPATCH_{name_upper}_GE_{version}(...) __VA_ARGS__
	#define MUU_MACRO_DISPATCH_{name_upper}_LT_{version}(...)
#else
	#define MUU_MACRO_DISPATCH_{name_upper}_GE_{version}(...)
	#define MUU_MACRO_DISPATCH_{name_upper}_LT_{version}(...) __VA_ARGS__
#endif''', file=buf)

			print(rf'''
/// \endcond
#endif // {guard_name}''', file=buf)

			print(rf'Writing {file_path}')
			with open(file_path, 'w', encoding='utf-8', newline='\n') as file:
				file.write(utils.clang_format(buf.getvalue()))

if __name__ == '__main__':
	utils.run(main, verbose=True)
