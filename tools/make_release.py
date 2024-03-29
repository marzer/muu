#!/usr/bin/env python3
# This file is a part of muu and is subject to the the terms of the MIT license.
# Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

import utils
import sys
import os
import zipfile
import re
import subprocess
import shutil
from argparse import ArgumentParser, RawTextHelpFormatter
from pathlib import Path



def open_text_file_for_writing(*path):
	p = str(Path(*path))
	print(rf'Writing {p}')
	return open(p, 'w', encoding='utf-8', newline='\n')



def run(args):

	# establish local directories
	root_dir = utils.entry_script_dir().resolve().parent
	include_dir = Path(root_dir, 'include')
	docs_dir = Path(root_dir, 'docs')
	tools_dir = Path(root_dir, 'tools')
	external_dir = Path(root_dir, 'external')
	utils.assert_existing_directory(include_dir)
	utils.assert_existing_directory(docs_dir)

	# ignore list for copytree
	garbage = ['*.ilk', '*.build', '*.dox', '*.log']
	if args.nosymbols:
		garbage.append('*.pdb')
	garbage = shutil.ignore_patterns(*garbage)

	# read version number
	pp_h = utils.read_all_text_from_file(Path(include_dir, 'muu/preprocessor.h'), logger=True)
	match = re.search(
		r'#\s*define\s+MUU_VERSION_MAJOR\s+([0-9]+)[^0-9].*' + r'#\s*define\s+MUU_VERSION_MINOR\s+([0-9]+)[^0-9].*'
		+ r'#\s*define\s+MUU_VERSION_PATCH\s+([0-9]+)[^0-9]', pp_h, re.I | re.S
	)
	if match is None:
		raise Exception("could not find MUU_VERSION macros in preprocessor.h")
	version_value = int(match[1]) * 10000 + int(match[2]) * 100 + int(match[3])
	version = rf'{int(match[1])}.{int(match[2])}.{int(match[3])}'
	print(rf'Local version: {version}')

	# set a few local git config options
	utils.git_query(r'config http.postBuffer 33554432', cwd=root_dir)
	utils.git_query(r'config protocol.version 2', cwd=root_dir)
	utils.git_query(r'config diff.ignoreSubmodules dirty', cwd=root_dir)

	# git fetch
	if not args.nofetch:
		with utils.ScopeTimer(r'Running git fetch', print_start=True) as timer:
			proc = subprocess.run(
				rf'git fetch --all --tags --force --jobs={os.cpu_count()} {"" if args.nosubmodules else "--recurse-submodules"}'
				.split(),
				cwd=root_dir,
				check=True
			)

	# get git commit hash
	print(rf'Getting git commit hash... ', end='')
	git_commit_hash = utils.git_query('rev-parse HEAD', cwd=root_dir)
	print(git_commit_hash)

	# get git commit description
	print(rf'Getting git commit description... ', end='')
	git_commit_description = utils.git_query('describe --always --long --dirty --abbrev=10 --tags', cwd=root_dir)
	print(git_commit_description)

	# get git branch
	print(rf'Getting git branch... ', end='')
	git_branch = utils.git_query('branch --show-current', cwd=root_dir)
	print(git_branch)

	# figure out what to call this release
	match = re.search(
		r'^(?:v?([0-9]+)[.]([0-9]+)[.]([0-9]+)-)?(?:([0-9]+)-)?(?:[a-z0-9]+)(-dirty)?$', git_commit_description, re.S
	)
	if match is None:
		raise Exception("could not parse git commit description")
	tag_version = ((int(match[1]) if match[1] else 0), (int(match[2]) if match[2] else 0),
		(int(match[3]) if match[3] else 0))
	tag_version_value = tag_version[0] * 10000 + tag_version[1] * 100 + tag_version[2]
	tag_version = rf'{tag_version[0]}.{tag_version[1]}.{tag_version[2]}'
	commits_ahead = int(match[4]) if match[4] else 0
	dirty = match[5] is not None and len(match[5]) > 0
	print(
		rf'Most recent tagged version: {tag_version}{ f" ({commits_ahead} commits ahead)" if commits_ahead else ""  }'
	)
	package_name = rf'muu_v{version}'
	if tag_version_value < version_value:
		package_name = rf'{package_name}-pre'
		if commits_ahead > 0:
			package_name = rf'{package_name}{commits_ahead}'
	elif commits_ahead:
		package_name = rf'{package_name}-patch{commits_ahead}'
	if git_branch and git_branch != 'master':
		package_name = rf'{package_name}-{git_branch}'
	if dirty:
		package_name = rf'{package_name}-dirty'

	# create temp output directory
	out_dir = Path(root_dir, args.tempdir) if args.tempdir else Path(root_dir, rf'{package_name}_temp')
	utils.delete_directory(out_dir, logger=True)
	print(rf'Creating {out_dir}')
	out_dir.mkdir(parents=True)

	# write VERSION
	with open_text_file_for_writing(out_dir, 'VERSION') as f:
		print(version, file=f, end='')

	# write COMMIT
	with open_text_file_for_writing(out_dir, 'COMMIT') as f:
		print(git_commit_hash, file=f, end='')

	# write BREADCRUMB
	with open_text_file_for_writing(out_dir, 'BREADCRUMB') as f:
		s = ''
		for arg in sys.argv:
			if re.search(r'\s', arg) is not None:
				s += rf'"{arg}" '
			else:
				s += rf'{arg} '
		print(s.strip(), file=f, end='')

	# pull submodules
	if not args.nosubmodules:
		if not external_dir.exists():
			external_dir.mkdir(parents=True)
		fetch = '' if args.nofetch else '--no-fetch'  # update includes fetch if we didn't fetch earlier
		with utils.ScopeTimer(r'Pulling submodules', print_start=True) as timer:
			subprocess.run((rf'git submodule update --init --depth 1 --jobs {os.cpu_count()} {fetch}').split(),
				cwd=root_dir,
				check=True)
			subprocess.run(
				r'git submodule foreach git reset --hard --recurse-submodules'.split(), cwd=root_dir, check=True
			)

	# libs
	if not args.nolibs:

		# regenerate the per-compiler version selection dispatch macros
		if not args.stale:
			with utils.ScopeTimer(r'Regenerating compiler version dispatch macros', print_start=True) as timer:
				utils.run_python_script(Path(tools_dir, 'generate_compiler_dispatch_macros.py'))

		# regenerate the MUU_FOR_EACH_XXXX macros
		if not args.stale:
			with utils.ScopeTimer(r'Regenerating MUU_FOR_EACH macros', print_start=True) as timer:
				utils.run_python_script(Path(tools_dir, 'generate_for_each_attribute_macros.py'))

		# regenerate unicode functions
		if not args.nounicode:
			with utils.ScopeTimer(r'Regenerating Unicode functions', print_start=True) as timer:
				utils.run_python_script(Path(tools_dir, 'generate_unicode_functions.py'))

		# build libs
		print(rf'Platforms: {", ".join(args.platforms)}')
		print(rf'Toolsets: {", ".join(args.toolsets)}')
		lib_dir = Path(root_dir, 'lib')
		if not args.stale:
			with utils.ScopeTimer(r'Building libs', print_start=True) as timer:
				utils.delete_directory(lib_dir, logger=True)
				for pf in args.platforms:
					for ts in args.toolsets:
						cmd = [
							str(Path(tools_dir,
							'build_libs_msvc.bat')), rf'-p:Platform={pf}', rf'-p:PlatformToolset=v{ts}',
							rf'-p:MuuOptimizedDebug={not args.nofastdebug}', rf'-p:MuuStripSymbols={args.nosymbols}',
							rf'-p:MuuInstructionSet={args.iset}', r'-p:MuuDeleteIntDir=True'
						]
						if args.verbose:
							cmd.append(r'-verbosity:detailed')
						subprocess.run(cmd, check=True)

		# copy libs
		utils.assert_existing_directory(lib_dir)
		print(rf'Copying {lib_dir}')
		shutil.copytree(lib_dir, Path(out_dir, 'lib'), ignore=garbage)

	# documentation
	if not args.nodocs:

		# generate documentation
		docs_html_dir = Path(docs_dir, 'html')
		if not args.stale:
			with utils.ScopeTimer(r'Generating documentation', print_start=True) as timer:
				subprocess.run([r'poxy'], check=True, cwd=docs_dir)
		utils.assert_existing_directory(docs_html_dir)

		# copy documentation
		print(rf'Copying {docs_html_dir}')
		shutil.copytree(docs_html_dir, Path(out_dir, 'docs'), ignore=garbage)

		# create documentation helper redirect page at root
		with open_text_file_for_writing(out_dir, 'documentation.html') as f:
			print(
				'''<!DOCTYPE html>
	<html lang="en">
		<head>
			<meta charset="utf-8"/>
			<meta http-equiv="refresh" content="0; url=docs/index.html" />
		</head>
		<body></body>
	</html>''',
				file=f
			)

	# copy includes
	print(rf'Copying {include_dir}')
	shutil.copytree(include_dir, Path(out_dir, 'include'), ignore=garbage)

	# copy individual top-level files
	top_level_files = ['LICENSE', 'cpp.hint', 'muu.natvis']
	for f in top_level_files:
		utils.copy_file(Path(root_dir, f), out_dir, logger=True)

	# zip file
	if not args.nozip:
		out_file = Path(root_dir, rf'{package_name}.zip')
		with utils.ScopeTimer(r'Zipping release', print_start=True) as timer:
			if out_file.exists():
				print(f'Deleting {out_file}')
				out_file.unlink()
			with zipfile.ZipFile(out_file, 'w', compression=zipfile.ZIP_LZMA, compresslevel=9) as zip:
				assert out_dir.is_absolute()
				file_prefix_len = len(str(out_dir))
				for file in utils.get_all_files(out_dir, recursive=True):
					relative_file = str(file)[file_prefix_len:].replace('\\', '/').strip('/')
					print(f'Zipping {relative_file}')
					zip.write(file, arcname=rf'muu/{relative_file}')
		print(rf'Release packaged as {out_file}.')

	# delete temp directory
	if not args.nocleanup:
		utils.delete_directory(out_dir, logger=True)



def main():
	args = ArgumentParser(
		description='Rebuilds the library and documentation and bundles them into a zip file for release.',
		formatter_class=RawTextHelpFormatter
	)
	args.add_argument('--verbose', '-v', action='store_true')
	args.add_argument(
		'--stale',
		action='store_true',
		help='do not recompile binaries, pull submodules, regenerate documentation etc.; use only what is already present'
	)
	args.add_argument('--nosubmodules', action='store_true', help='do not pull submodules (implied by --stale)')
	args.add_argument(
		'--nounicode',
		action='store_true',
		help='do not regenerate the Unicode functions before building (implied by --stale)'
	)
	args.add_argument('--nodocs', action='store_true', help='do not include documentation')
	args.add_argument('--nofetch', action='store_true', help='do not run git fetch (implied by --stale)')
	args.add_argument('--nolibs', action='store_true', help='do not include library binaries')
	args.add_argument('--nocleanup', action='store_true', help='do not delete the temp directory afterward')
	args.add_argument('--nozip', action='store_true', help='do not create the zip file')
	args.add_argument(
		'--nofastdebug',
		action='store_true',
		help='do not build debug-mode binaries with any optimization (default is "optimized debug").'
	)
	args.add_argument(
		'--nosymbols', action='store_true', help='do not generate debug information or emit symbol database files.'
	)
	args.add_argument(
		'--iset',
		choices=['sse2', 'avx', 'avx2', 'avx512'],
		default='avx',
		help='minimum instruction set to target (default: %(default)s)'
	)
	default_platforms = ['x64']
	supported_platforms = ['x64', 'Win32']
	args.add_argument(
		'--platforms',
		type=str,
		action='extend',
		nargs='+',
		metavar='PF',
		help=rf"the platforms to target (default: {' '.join(default_platforms)})"
	)
	default_toolsets = ['143']
	supported_toolsets = ['143', '142']
	args.add_argument(
		'--toolsets',
		type=str,
		action='extend',
		nargs='+',
		metavar='TS',
		help=rf"the VS toolsets to target (default: {' '.join(default_toolsets)})"
	)
	args.add_argument(
		'--tempdir',
		type=str,
		default='',
		metavar='DIR',
		help=rf"temp directory used for collecting the outputs prior to zipping (default: automatic)."
		+ "\nRelative paths are resolved from the repository root."
		+ "\nWARNING: any existing contents will be deleted!"
	)
	args = args.parse_args()

	# process platforms arg
	if not args.platforms:
		args.platforms = default_platforms
	args.platforms = [pf.strip(' ,') for pf in args.platforms]
	args.platforms = [pf for pf in args.platforms if pf]
	for pf in args.platforms:
		if pf not in supported_platforms:
			print(
				rf"unknown or unsupported target platform '{pf}' - must be one of: {' '.join(supported_platforms)}",
				file=sys.stderr
			)
			return 1

	# process toolsets arg
	if not args.toolsets:
		args.toolsets = default_toolsets
	args.toolsets = [ts.strip(' ,v') for ts in args.toolsets]
	args.toolsets = [ts for ts in args.toolsets if ts]
	for ts in args.toolsets:
		if ts not in supported_toolsets:
			print(
				rf"unknown or unsupported VS toolset version '{ts}' - must be one of: {' '.join(supported_toolsets)}",
				file=sys.stderr
			)
			return 1

	# process tempdir arg
	args.tempdir = args.tempdir.strip()
	args.tempdir = args.tempdir.replace('/', os.path.sep).replace('\\', os.path.sep)
	if re.search(r'[<>"|?*]', args.tempdir) is not None:
		print(r'tempdir argument may not contain any of these characters: < > " | ? *', file=sys.stderr)
		return 1

	# collapse some invariants here to make the run routine easier to reason about
	if args.stale:
		args.nofetch = True
		args.nosubmodules = True
		args.nounicode = True
	if args.nolibs:
		args.nosubmodules = True
		args.nounicode = True

	with utils.ScopeTimer(r'All tasks') as timer:
		run(args)



if __name__ == '__main__':
	utils.run(main, verbose=True)
