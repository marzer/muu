import sys
import os.path
import utils
import zipfile
import re
import subprocess
import shutil
from argparse import ArgumentParser
from pathlib import Path

__verbose = False
def vprint(*args):
	global __verbose
	if __verbose:
		print(*args)



def main():

	global __verbose
	args = ArgumentParser(description='Bundles the current build of the library into a zip file for release.')
	args.add_argument('--verbose', '-v', action='store_true')
	args.add_argument(
		'--stale',
		action='store_true',
		help='Does not recompile binaries, regenerate documentation etc., using only what is already present.'
	)
	args.add_argument(
		'--nocleanup',
		action='store_true',
		help='Does not delete the temp directory afterward.'
	)
	args.add_argument(
		'--nozip',
		action='store_true',
		help='Does not create the zip file.'
	)
	args.add_argument(
		'--debug',
		action='store_true',
		help='Builds binaries with debug symbols and includes symbol files with the release.'
	)
	args.add_argument(
		'--iset',
		choices=['sse', 'sse2', 'avx', 'avx2', 'avx512'],
		default='avx',
		help='Sets the min instruction set to target. (default: %(default)s)'
	)
	args = args.parse_args()
	__verbose = args.verbose
	vprint(" ".join(sys.argv))
	vprint(args)

	# establish local directories
	root_dir = utils.entry_script_dir().parent
	include_dir = Path(root_dir, 'include')
	docs_dir = Path(root_dir, 'docs')
	tools_dir = Path(root_dir, 'tools')
	external_dir = Path(root_dir, 'external')
	utils.assert_existing_directory(include_dir)
	utils.assert_existing_directory(docs_dir)

	# helpers
	open_text_file_for_writing = lambda *f: open(str(Path(*f)), 'w', encoding='utf-8', newline='\n')

	# read version number
	pp_h = utils.read_all_text_from_file(Path(include_dir, 'muu/preprocessor.h'))
	match = re.search(
			r'#\s*define\s+MUU_VERSION_MAJOR\s+([0-9]+)[^0-9].*'
			+ r'#\s*define\s+MUU_VERSION_MINOR\s+([0-9]+)[^0-9].*'
			+ r'#\s*define\s+MUU_VERSION_PATCH\s+([0-9]+)[^0-9]',
			pp_h, re.I | re.S)
	if match is None:
		raise Exception("could not find MUU_VERSION macros in preprocessor.h")
	version = rf'{int(match[1])}.{int(match[2])}.{int(match[3])}'
	print(version)

	# create temp output directory
	out_dir = Path(root_dir, rf'muu_v{version}_temp')
	utils.delete_directory(out_dir)
	out_dir.mkdir()

	# write version file
	with open_text_file_for_writing(out_dir, 'VERSION') as f:
		print(version, file=f, end='')

	# get git commit hash and write commit file
	proc = subprocess.run(
		r'git rev-parse HEAD'.split(),
		capture_output=True,
		cwd=root_dir,
		encoding='utf-8'
	)
	git_commit_hash = proc.stdout.strip()
	with open_text_file_for_writing(out_dir, 'COMMIT') as f:
		if git_commit_hash:
			print(git_commit_hash, file=f, end='')
		elif proc.stderr.strip():
			print(proc.stderr.strip(), file=f)

	# get git commit description
	proc = subprocess.run(
		r'git describe --always --long --dirty --abbrev=10 --tags'.split(),
		capture_output=True,
		cwd=root_dir,
		encoding='utf-8'
	)
	git_commit_description = proc.stdout.strip()

	# pull submodules
	if not external_dir.exists():
		external_dir.mkdir()
	submodules = (
		r'dox',
		r'Catch2'
	)
	if False in [Path(external_dir, mod).exists() for mod in submodules]:
		subprocess.run(
			r'git submodule update --init --recursive'.split(),
			cwd=root_dir
		)
		for mod in submodules:
			utils.assert_existing_directory(Path(external_dir, mod))

	# regenerate unicode functions
	if not args.stale:
		utils.run_python_script(Path(tools_dir, 'generate_unicode_functions.py'))

	# build libs
	lib_dir = Path(root_dir, 'lib')
	if not args.stale:
		utils.delete_directory(lib_dir)
		cmd = [str(Path(tools_dir, 'build_libs_msvc.bat')),
				f'-p:MuuOptimizedDebug={not args.debug}',
				f'-p:MuuStripSymbols={not args.debug}',
				f'-p:MuuInstructionSet={args.iset}',
				f'-p:BuildInfoGitCommitHash={git_commit_hash}',
				f'-p:BuildInfoGitCommitDescription={git_commit_description}',
				'-p:MuuDeleteIntDir=True'
				]
		if args.verbose:
			cmd.append('-verbosity:detailed')
		subprocess.run(
			cmd,
			check=True,
			shell=True
		)

	# copy libs
	utils.assert_existing_directory(lib_dir)
	shutil.copytree(lib_dir, Path(out_dir, 'lib'))

	# generate documentation
	docs_html_dir = Path(docs_dir, 'html')
	if not args.stale:
		utils.run_python_script(
			Path(external_dir, 'dox', 'dox.py'),
			'dox.toml',
			cwd=docs_html_dir.parent
		)

	# copy documentation
	utils.assert_existing_directory(docs_html_dir)
	shutil.copytree(docs_html_dir, Path(out_dir, 'docs'))

	# create documentation helper redirect page at root
	with open_text_file_for_writing(out_dir, 'documentation.html') as f:
		print('''<!DOCTYPE html>
<html lang="en">
	<head>
		<meta charset="utf-8"/>
		<meta http-equiv="refresh" content="0; url=docs/index.html" />
	</head>
	<body></body>
</html>''',
		file =f)

	# copy includes
	shutil.copytree(include_dir, Path(out_dir, 'include'))

	# copy individual top-level files
	shutil.copy(Path(root_dir, 'LICENSE'), out_dir)
	shutil.copy(Path(root_dir, 'cpp.hint'), out_dir)
	shutil.copy(Path(root_dir, 'muu.natvis'), out_dir)

	# delete garbage from output
	garbage = ['*.exp', '*.ilk', '*.build', '*.dox', '*.log']
	if not args.debug:
		garbage.append('*.pdb')
	for f in utils.get_all_files(out_dir, any=garbage, recursive=True):
		print(f'Deleting {f}')
		f.unlink()

	# zip file
	if not args.nozip:
		out_file = Path(root_dir, rf'muu_v{version}.zip')
		if out_file.exists():
			print(f'Deleting {out_file}')
			out_file.unlink()
		with zipfile.ZipFile(out_file, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as zip:
			file_prefix_len = len(str(out_dir))
			for file in utils.get_all_files(out_dir, recursive=True):
				archive_file = 'muu/' + str(file)[file_prefix_len:].strip(r'\/')
				print(f'Zipping {file}')
				zip.write(file, arcname=archive_file)
		print(rf'Release packaged as {out_file}.')

	# delete temp directory
	if not args.nocleanup:
		utils.delete_directory(out_dir)

	print(r'All tasks complete!')

if __name__ == '__main__':
	utils.run(main)
