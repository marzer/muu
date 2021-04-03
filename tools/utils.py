#!/usr/bin/env python3
# This file is a part of muu and is subject to the the terms of the MIT license.
# Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

import sys
import os
import subprocess
import traceback
import shutil
import fnmatch
import requests
import hashlib
from pathlib import Path



def is_tool(name):
	return shutil.which(name) is not None



__entry_script_dir = None
def entry_script_dir():
	global __entry_script_dir
	if __entry_script_dir is None:
		__entry_script_dir = Path(sys.argv[0]).resolve().parent
	return __entry_script_dir



def assert_existing_file(path):
	assert path is not None
	if not isinstance(path, Path):
		path = Path(path)
	if not (path.exists() and path.is_file()):
		raise Exception(f'{path} did not exist or was not a file')



def assert_existing_directory(path):
	assert path is not None
	if not isinstance(path, Path):
		path = Path(path)
	if not (path.exists() and path.is_dir()):
		raise Exception(f'{path} did not exist or was not a directory')



def read_all_text_from_file(path, fallback_url=None, encoding='utf-8'):
	assert path is not None
	if not isinstance(path, Path):
		path = Path(path)
	if fallback_url is None:
		assert_existing_file(path)
	try:
		print(f'Reading {path}')
		with open(str(path), 'r', encoding=encoding) as f:
			text = f.read()
		return text
	except:
		if fallback_url is not None:
			print(f"Couldn't read file locally, downloading from {fallback_url}")
			response = requests.get(
				fallback_url,
				timeout=1
			)
			text = response.text
			with open(str(path), 'w', encoding='utf-8', newline='\n') as f:
				print(text, end='', file=f)
			return text
		else:
			raise



def run_python_script(path, *args, cwd=None):
	assert path is not None
	if cwd is not None:
		cwd = str(cwd)
	assert_existing_file(path)
	subprocess.run(
		['py' if shutil.which('py') is not None else 'python3', str(path)] + [arg for arg in args],
		check=True,
		cwd=cwd
	)



def delete_directory(path):
	assert path is not None
	if not isinstance(path, Path):
		path = Path(path)
	if path.exists():
		if not path.is_dir():
			raise Exception(f'{path} was not a directory')
		print(f'Deleting {path}')
		shutil.rmtree(str(path.resolve()))



def get_all_files(path, all=None, any=None, recursive=False, sort=True):
	assert path is not None
	if not isinstance(path, Path):
		path = Path(path)
	if not path.exists():
		return []
	if not path.is_dir():
		raise Exception(f'{path} was not a directory')
	path = path.resolve()
	
	child_files = []
	files = []
	for p in path.iterdir():
		if p.is_dir():
			if recursive:
				child_files = child_files + get_all_files(p, all=all, any=any, recursive=True, sort=False)
		elif p.is_file():
			files.append(str(p))

	if files and all is not None:
		if (not is_collection(all)):
			all = (all,)
		all = [f for f in all if f is not None]
		for fil in all:
			files = fnmatch.filter(files, fil)

	if files and any is not None:
		if (not is_collection(any)):
			any = (any,)
		any = [f for f in any if f is not None]
		if any:
			results = set()
			for fil in any:
				results.update(fnmatch.filter(files, fil))
			files = [f for f in results]

	files = [Path(f) for f in files] + child_files
	if sort:
		files.sort()
	return files



def is_collection(val):
	if isinstance(val, (list, tuple, dict, set, range)):
		return True
	return False



def is_pow2(v):
	return v & (v-1) == 0



def next_power_of_2(n):
	if n == 0:
		return 1
	if n & (n - 1) == 0:
		return n
	while n & (n - 1) > 0:
		n &= (n - 1)
	return n << 1



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



def multi_sha256(*objs):
	assert objs
	h = hashlib.sha256()
	for o in objs:
		assert o is not None
		h.update(str(o).encode('utf-8'))
	return h.hexdigest()



def run(main_func):
	try:
		result = main_func()
		if result is None:
			sys.exit(0)
		else:
			sys.exit(int(result))
	except Exception as err:
		print(
			rf'Fatal error: [{type(err).__name__}] {str(err)}',
			file=sys.stderr
		)
		traceback.print_exc(file=sys.stderr)
		sys.exit(-1)
