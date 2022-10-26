#!/usr/bin/env python3
# This file is a part of muu and is subject to the the terms of the MIT license.
# Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

import sys
import utils
import re
from argparse import ArgumentParser
from pathlib import Path



def write_text_file(path, text):
	print(rf'Writing {path}')
	with open(path, r'w', encoding=r'utf-8', newline='\n') as f:
		f.write(text)



def main():

	args = ArgumentParser(r'version.py', description=r'Sets the project version in all the necessary places.')
	args.add_argument(r'version', type=str)
	args = args.parse_args()

	version = re.fullmatch(r'\s*([0-9]+)\s*[.,;]\s*([0-9]+)\s*[.,;]\s*([0-9]+)\s*', args.version)
	if not version:
		print(rf"Couldn't parse version triplet from '{args.version}'", file=sys.sys.stderr)
		return 1
	version = (int(version[1]), int(version[2]), int(version[3]))
	version_str = rf'{version[0]}.{version[1]}.{version[2]}'
	print(rf'version: {version_str}')

	root = Path(utils.entry_script_dir(), r'..').resolve()

	path = root / r'meson.build'
	text = utils.read_all_text_from_file(path)
	text = re.sub(r'''(\s|^)version\s*:\s*['"][.*?]['"]''', rf"\1version : '{version_str}'", text, count=1)
	write_text_file(path, text)

	path = root / r'include/muu/preprocessor.h'
	text = utils.read_all_text_from_file(path)
	text = re.sub(r'''(\s*#\s*define\s+MUU_VERSION_MAJOR)\s+[0-9]+''', rf"\1 {version[0]}", text)
	text = re.sub(r'''(\s*#\s*define\s+MUU_VERSION_MINOR)\s+[0-9]+''', rf"\1 {version[1]}", text)
	text = re.sub(r'''(\s*#\s*define\s+MUU_VERSION_PATCH)\s+[0-9]+''', rf"\1 {version[2]}", text)
	write_text_file(path, text)



if __name__ == '__main__':
	utils.run(main)
