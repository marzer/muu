#!/usr/bin/env python3
# This file is a part of muu and is subject to the the terms of the MIT license.
# Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

import utils
import argparse
from pathlib import Path
from io import StringIO



def run(args):
	batch_size = min(4 if args.size <= 0 else args.size, 16)

	# directories
	root_dir = Path(utils.entry_script_dir(), '..').resolve()
	tests_dir = Path(root_dir, 'tests')
	utils.assert_existing_directory(tests_dir)

	# input file
	header_path = Path(args.header)
	if not header_path.is_absolute():
		header_path = Path(tests_dir, header_path)
	if not header_path.suffix:
		tentative_path = Path(rf'{header_path}.h')
		if not tentative_path.exists():
			tentative_path = Path(rf'{header_path}_tests.h')
		if tentative_path.exists():
			header_path = tentative_path
	utils.assert_existing_file(header_path)

	# output files
	for i in range(0, 16):
		source_path = header_path.stem
		if source_path.endswith(r'_tests'):
			source_path = source_path[:-len(r'_tests')]
		infix = source_path.strip()
		source_path = Path(header_path.parent, source_path + rf'_{i}.cpp')
		
		if i < batch_size:
			with StringIO() as buf:
				print(rf'''
// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "tests.h"

#define TEST_BATCHES     {batch_size}
#define TEST_BATCH       {i}
#define TEST_BATCH_INFIX {infix}
#include "batching.h"

#include "{header_path.name}"

'''.strip(), file=buf)

				print(rf'Writing {source_path}')
				with open(source_path, 'w', encoding='utf-8', newline='\n') as file:
					file.write(utils.clang_format(buf.getvalue()))
		else:
			utils.delete_file(source_path, logger=True)


def main():
	args = argparse.ArgumentParser(
		description=r'''Generates source files for 'batched' template tests.'''
	)
	args.add_argument(
		r'header',
		type=Path,
		help=r'test header file'
	)
	args.add_argument(
		r'size',
		type=int,
		nargs='?',
		default=4,
		help=r"batch size (default: 4)"
	)
	args = args.parse_args()
	with utils.ScopeTimer(r'All tasks') as timer:
		run(args)



if __name__ == '__main__':
	utils.run(main, verbose=True)
