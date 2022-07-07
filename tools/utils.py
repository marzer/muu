#!/usr/bin/env python3
# This file is a part of muu and is subject to the the terms of the MIT license.
# Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

import sys
import subprocess
from pathlib import Path
from misk import *



def repeat_pattern(pattern, count):
	if len(pattern) == 1:
		return pattern * count
	text = ''
	for i in range(0, count):
		text = text + pattern[i % len(pattern)]
	return text



def make_divider(text = None, text_col = 40, pattern = '-', line_length = 120):
	if (text is None):
		return "//" + repeat_pattern(pattern, line_length-2)
	else:
		text = "//{}  {}  ".format(repeat_pattern(pattern, text_col - 2), text);
		if (len(text) < line_length):
			return text + repeat_pattern(pattern, line_length - len(text))
		else:
			return text



def clang_format(s, cwd=None):
	result = subprocess.run(
		['clang-format', '--style=file'],
		capture_output=True,
		cwd=str(Path.cwd() if cwd is None else cwd),
		encoding='utf-8',
		check=True,
		input=s
	)
	return str(result.stdout)



def __hash_combine_32(current_hash, next_hash):
	current_hash = current_hash & 0xFFFFFFFF
	next_hash = next_hash & 0xFFFFFFFF
	current_hash = current_hash ^ (next_hash + 0x9E3779B9 + (current_hash << 6) + (current_hash >> 2))
	return current_hash & 0xFFFFFFFF
def __hash_combine_64(current_hash, next_hash):
	current_hash = current_hash & 0xFFFFFFFFFFFFFFFF
	next_hash = next_hash & 0xFFFFFFFFFFFFFFFF
	current_hash = current_hash ^ (next_hash + 0x9E3779B97F4A7C15 + (current_hash << 12) + (current_hash >> 4))
	return current_hash & 0xFFFFFFFFFFFFFFFF
__hash_combine_func = None
if sys.hash_info.width == 64:
	__hash_combine_func = __hash_combine_64
elif sys.hash_info.width == 32:
	__hash_combine_func = __hash_combine_32



def hash_combine(current_hash, next_hash, *nexts):
	global __hash_combine_func
	current_hash = __hash_combine_func(current_hash, next_hash)
	if nexts:
		for n in nexts:
			current_hash = __hash_combine_func(current_hash, n)
	return current_hash



def multi_hash(obj, *objs):
	assert obj is not None
	h = hash(obj)
	if objs:
		for o in objs:
			assert o is not None
			h = hash_combine(h, hash(o))
	return h



def run(main_func, verbose=False):
	try:
		result = main_func()
		if result is None:
			sys.exit(0)
		else:
			sys.exit(int(result))
	except Exception as err:
		print_exception(err, include_type=verbose, include_traceback=verbose, skip_frames=1)
		sys.exit(-1)
