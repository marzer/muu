#!/usr/bin/env python3
# This file is a part of muu and is subject to the the terms of the MIT license.
# Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

import utils
import re
import textwrap
import sys
from argparse import ArgumentParser, BooleanOptionalAction
from pathlib import Path

SNIPPET_DECL  = re.compile(r'^\s*\/\/%\s*([a-zA-Z0-9::_+-]+)(?:\s+(guarded))?\s*$', flags=re.MULTILINE)
SNIPPET_START = re.compile(r'^\s*\/\/%\s*([a-zA-Z0-9::_+-]+)\s+start\s*$', flags=re.MULTILINE)
SNIPPET_END   = re.compile(r'^\s*\/\/%\s*([a-zA-Z0-9::_+-]+)\s+end\s*$', flags=re.MULTILINE)

STRIP_BLOCK = re.compile(r'(?:\n[ \t]*)?//[#!][ \t]*[{][{].*?//[#!][ \t]*[}][}].*?\n', flags=re.S)
MUU_DELETE_MOVE = re.compile(r'MUU_DELETE_MOVE\(\s*([a-zA-Z0-9_:]+)\s*\)\s*;')
MUU_DELETE_COPY = re.compile(r'MUU_DELETE_COPY\(\s*([a-zA-Z0-9_:]+)\s*\)\s*;')
DOXYGEN_INGROUP = re.compile(r'\n[ \t]*///[ \t]*[\\@]ingroup[ \t].*?\n', flags=re.MULTILINE)
TRAILING_WHITESPACE = re.compile(r'([^ \t])[ \t]+\n')

def cleanup_text(text: str, remove_doxygen_triple_slashes = True, remove_doxygen_ingroup = True) -> str:

	# 'strip this' blocks "//# {{" and "//# }}"
	text = STRIP_BLOCK.sub('\n', text)

	# /// @ingroup
	if remove_doxygen_ingroup:
		text = DOXYGEN_INGROUP.sub('\n', text)

	# MUU class macros
	text = MUU_DELETE_MOVE.sub(
		r'''\1(\1&&) = delete;
		\1& operator=(\1&&) = delete;''',
		text
	)
	text = MUU_DELETE_COPY.sub(
		r'''\1(const \1&) = delete;
		\1& operator=(const \1&) = delete;''',
		text
	)

	doxy = r'/' if remove_doxygen_triple_slashes else ''
	magic_comment = rf'(?://(?:[#!<{doxy}]|[ \t]?(?:\^\^\^|vvv)))'
	while True:
		prev_text = text
		# trailing whitespace
		text = TRAILING_WHITESPACE.sub(r'\1\n', text)
		# double blank lines
		text = re.sub('\n\n\n+', '\n\n', text)
		# magic comments
		text = re.sub(rf'{magic_comment}.*?($)', r'\1', text, flags=re.MULTILINE)
		# blank lines following a comma, opening bracket, or colon:
		text = re.sub(r'([{(\[:,])\n\n', r'\1\n', text)
		if text == prev_text:
			break

	return text


def main():
	args = ArgumentParser(description=r'Creates a single header from fragments of the codebase.')
	args.add_argument(r'template', type=Path, help=r'input template file')
	args.add_argument(r'--output', type=Path, metavar=r'<path>', help=r"output file", default=None)
	args.add_argument(
		r'--namespaces',
		type=str,
		nargs=2,
		metavar=(r'<main>', r'<impl>'),
		help=r'names to use when transforming namespaces (default: %(default)s)',
		default=[r'mz', r'detail']
	)
	args.add_argument(
		r'--macros',
		type=str,
		metavar=r'<prefix>',
		help=r'prefix to apply when transforming macros (default: %(default)s)',
		default=r'MZ'
	)
	args.add_argument(r'--strip-vectorcall', action=BooleanOptionalAction, default=True)
	args.add_argument(r'--strip-hidden-bases', action=BooleanOptionalAction, default=True)
	args.add_argument(r'--strip-doxygen-from-snippets', action=BooleanOptionalAction, default=True)
	args = args.parse_args()

	# check args
	if 1:
		args.template = args.template.resolve()
		print(rf'  template: {args.template}')

		if args.output is None:
			if args.template.name.endswith(r'.in'):
				args.output = args.template.parent / args.template.name[:-3]
			else:
				args.output = args.template.parent / rf'{args.template.stem}.single{args.template.suffix}'
		args.output = args.output.resolve()
		print(rf'    output: {args.output}')
		if args.template == args.output:
			raise Exception('template and output cannot be the same')

		assert args.namespaces is not None
		assert len(args.namespaces) == 2
		assert isinstance(args.namespaces, list)
		for ns in args.namespaces:
			if not ns:
				raise Exception(r'namespaces cannot be blank')
			if re.search(r'[^a-zA-Z0-9_:]', ns):
				raise Exception(r'namespaces may only contain a-z, A-Z, 0-9, _, :')
			if ns.startswith(r':') or ns.endswith(r':'):
				raise Exception(r'namespaces may not start or end with :')
			if ns.find(r'__') != -1:
				raise Exception(r'namespaces may not contain double-underscores; those are reserved identifiers in C++')
		print(rf'namespaces: {args.namespaces}')

		assert args.macros is not None
		assert isinstance(args.macros, str)
		if not args.macros:
			raise Exception(r'macros cannot be blank')
		if not args.macros.rstrip(r'_'):
			raise Exception(r'macros cannot be entirely underscores')
		if re.search(r'[^a-zA-Z0-9_]', args.macros):
			raise Exception(r'macros may only contain a-z, A-Z, 0-9, _')
		if args.macros.find(r'__') != -1:
			raise Exception(r'macros may not contain double-underscores; those are reserved identifiers in C++')
		if not args.macros.endswith(r'_'):
			args.macros += r'_'
		print(rf'    macros: {args.macros}')

	# figure out which snippets we need
	snippets = dict()
	text = utils.read_all_text_from_file(args.template)
	for m in SNIPPET_DECL.finditer(text):
		snippets[m[1]] = None
		#print(m[1])

	# read + pre-format the LICENSE snippet
	license = []
	for name in (r'LICENSE.md', r'LICENSE.txt', r'LICENSE'):
		dir = args.output.parent
		steps = 0
		path = dir / name
		while not (path.exists() and path.is_file()):
			steps += 1
			prev_dir = dir
			dir = dir.parent
			if prev_dir == dir:
				break
			path = dir / name
		if path.exists() and path.is_file():
			license.append((path, steps))
	license.sort(key=lambda val: val[1])
	if license:
		license = utils.read_all_text_from_file(license[0][0]).replace('\r\n', '\n').strip()
		license = license.replace('\n\n', '\b')
		license = license.replace('\n', ' ')
		license = license.replace('\b', '\n')
		license = license.split('\n')
		for i in range(len(license)):
			license[i] = '// ' + '\n// '.join(textwrap.wrap(license[i], width=115, tabsize=4))
		license = '\n//\n'.join(license)
		license = f'//{"-"*118}\n//\n{license}\n//'
	else:
		license = ''

	# stick in the special preamble about this process

	generated_script_warning = r"    !!!!! THIS FILE WAS ASSEMBLED FROM MULTIPLE HEADER FILES BY A SCRIPT - PLEASE DON'T EDIT IT DIRECTLY !!!!!"
	snippets[r'generated_header_preamble'] = rf'''
// {generated_script_warning}
{license}
	'''.strip()

	# extract the snippets from the codebase
	unresolved_snippets = len(snippets)
	if unresolved_snippets:
		includes = Path(__file__).parent.parent / 'include'
		for f in utils.enumerate_files(includes, any=(r'*.h', r'*.hpp'), recursive=True):
			file_text = utils.read_all_text_from_file(f)
			for m in SNIPPET_START.finditer(file_text):
				if m[1] not in snippets or snippets[m[1]] is not None:
					continue
				search_pos = m.end()
				m2 = None
				while True:
					m2 = SNIPPET_END.search(file_text, pos=search_pos)
					if not m2:
						raise Exception(rf'unterminated snippet {m[1]} in {f.name}')
					if m2[1] == m[1]:
						break
					search_pos = m2.end()
				snippet = file_text[m.end():m2.start()].strip('\r\n')
				snippet = re.sub(r'^\s*//%.+?$', '', snippet, flags=re.MULTILINE)
				if m[1].endswith(r'_naive'):
					snippet = snippet.replace(r'_naive', r'')
				snippet = f'\n{snippet}\n'
				snippet = cleanup_text(snippet, remove_doxygen_triple_slashes=args.strip_doxygen_from_snippets)
				snippet = f'\n{snippet}\n'
				snippets[m[1]] = snippet
				#print(f'############################################\n{snippets[m[1]]}')
				unresolved_snippets -= 1
				if not unresolved_snippets:
					break
			if not unresolved_snippets:
				break
	for k, v in snippets.items():
		if v is None:
			raise Exception(rf'unresolved snippet {k}')

	# sub the snippets into the template
	seen_preamble = False
	def sub_snippet(m) -> str:
		nonlocal seen_preamble
		nonlocal snippets
		out = snippets[m[1]]
		if m[1] == r'generated_header_preamble':
			seen_preamble = True
		if m[2] == 'guarded':
			guard = re.sub(r'[^a-zA-Z0-9]+', '_', str(m[1])).strip('_')
			guard = rf'MZ_HAS_SNIPPET_{guard.upper()}'
			out = f'\n#ifndef {guard}\n#define {guard}\n{snippets[m[1]]}\n#endif //{guard}\n'
		return out

	text = SNIPPET_DECL.sub(sub_snippet, text)

	# de-muuifiying
	text = re.sub(r'\bMUU_MEMCPY\b', r'std::memcpy', text)
	if args.strip_vectorcall:
		text = re.sub(r'(#\s*define\s+MUU_VECTORCALL)', r'\1_', text)
		text = re.sub(r'\bMUU_VECTORCALL\b', r'', text)
		text = re.sub(r'(#\s*define\s+MUU_VECTORCALL)_', r'\1', text)
	if args.strip_hidden_bases:
		text = re.sub(r'(#\s*define\s+MUU_HIDDEN_BASE)', r'\1_', text)
		text = re.sub(r'MUU_HIDDEN_BASE\((.+?)\)', r': \1', text)
		text = re.sub(r'(#\s*define\s+MUU_HIDDEN_BASE)_', r'\1', text)
	text = re.sub(r'(?:\b|::)?muu::impl::enable_if_t_\b', r'std::enable_if_t', text)
	text = re.sub(r'(?:\b|::)?muu::impl::enable_if_\b', r'std::enable_if', text)
	text = re.sub(r'\b(?:muu::)?remove_cvref<([a-zA-Z0-9:_]+?)>', r'std::remove_cv_t<std::remove_reference_t<\1>>', text)
	text = utils.replace_metavar(r'namespaces::main', args.namespaces[0], text)
	text = utils.replace_metavar(r'namespace', args.namespaces[0], text)
	text = utils.replace_metavar(r'namespaces::impl', args.namespaces[1], text)
	text = utils.replace_metavar(r'namespaces::detail', args.namespaces[1], text)
	text = utils.replace_metavar(r'macros::prefix', args.macros, text)
	text = re.sub(r'\bMUU_', args.macros, text)
	text = re.sub(r'^\s*#\s*include\s*"impl/std_(.+?)[.]h"', r'#include <\1>', text, flags=re.MULTILINE)
	namespace_substitutions = (
		(r'muu::impl', rf'{args.namespaces[0]}::{args.namespaces[1]}'),
		(r'muu', args.namespaces[0]),
		(r'impl', args.namespaces[1]),
	)
	for ns in namespace_substitutions:
		text = re.sub(rf'\bnamespace\s+{ns[0]}\b', rf'namespace {ns[1]}', text)
		text = re.sub(rf'\b{ns[0]}::', rf'{ns[1]}::', text)
		text = re.sub(rf'::{ns[0]}\b', rf'::{ns[1]}', text)

	# other cleanup
	text = cleanup_text(text, remove_doxygen_triple_slashes=False)

	# clang-format
	if 1:
		try:
			text = utils.clang_format(text, cwd=args.output.parent)
		except:
			pass

	# if we didn't use the preamble, add a reduced version of it without the license just to warn the file was automatically generated
	if not seen_preamble:
		text = f'''//#{"-"*117}
//# {generated_script_warning}
//#{"-"*117}
{text}'''

	# warn if there's unresolved headers
	# (this script is not currently a preprocessor)
	unresolved_include = re.search(r'^\s*#\s*include\s*"(.+?)"', text, flags=re.MULTILINE)
	if unresolved_include:
		print(rf'Warning: unresolved #include "{unresolved_include[1]}"', file=sys.stderr)

	print(rf'Writing {args.output}')
	with open(args.output, 'w', newline='\n', encoding=r'utf-8') as f:
		f.write(text)



if __name__ == '__main__':
	utils.run(main, verbose=True)
