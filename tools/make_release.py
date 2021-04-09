import sys
import os
import utils
import zipfile
import re
import subprocess
import shutil
from argparse import ArgumentParser, RawTextHelpFormatter
from pathlib import Path

__verbose = False
def vprint(*args):
	global __verbose
	if __verbose:
		print(*args)



def open_text_file_for_writing(*path):
	p = str(Path(*path))
	print(rf'Writing {p}')
	return open(p, 'w', encoding='utf-8', newline='\n')



def main():

	global __verbose
	args = ArgumentParser(
		description='Rebuilds the library and documentation and bundles them into a zip file for release.',
		formatter_class=RawTextHelpFormatter
	)
	args.add_argument('--verbose', '-v', action='store_true')
	args.add_argument(
		'--stale',
		action='store_true',
		help='Does not recompile binaries, pull submodules, regenerate documentation etc., using only what is already present.'
	)
	args.add_argument(
		'--nosubmodules',
		action='store_true',
		help='Does not pull submodules.'
	)
	args.add_argument(
		'--nodocs',
		action='store_true',
		help='Does not include documentation.'
	)
	args.add_argument(
		'--nofetch',
		action='store_true',
		help='Does not run git fetch.'
	)
	args.add_argument(
		'--nolibs',
		action='store_true',
		help='Does not include library binaries.'
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

	# read version number
	pp_h = utils.read_all_text_from_file(Path(include_dir, 'muu/preprocessor.h'))
	match = re.search(
			r'#\s*define\s+MUU_VERSION_MAJOR\s+([0-9]+)[^0-9].*'
			+ r'#\s*define\s+MUU_VERSION_MINOR\s+([0-9]+)[^0-9].*'
			+ r'#\s*define\s+MUU_VERSION_PATCH\s+([0-9]+)[^0-9]',
			pp_h, re.I | re.S)
	if match is None:
		raise Exception("could not find MUU_VERSION macros in preprocessor.h")
	version_value = int(match[1]) * 10000 + int(match[2]) * 100 + int(match[3])
	version = rf'{int(match[1])}.{int(match[2])}.{int(match[3])}'
	print(rf'Local version: {version}')

	# git fetch
	if not args.nofetch:
		print(rf'Running git fetch')
		fetch_includes_submodules = not args.stale and not args.nosubmodules and not (args.nolibs and args.nodocs)
		proc = subprocess.run(
			rf'git fetch --jobs={os.cpu_count()} {"--recurse-submodules" if fetch_includes_submodules else ""}'.split(),
			cwd=root_dir,
			check=True
		)
		
	# get git commit hash
	print(rf'Getting git commit hash... ', end='')
	proc = subprocess.run(
		r'git rev-parse HEAD'.split(),
		capture_output=True,
		cwd=root_dir,
		encoding='utf-8',
		check=True
	)
	git_commit_hash = proc.stdout.strip()
	print(git_commit_hash)
	if not git_commit_hash and proc.stderr.strip():
		raise Exception(rf'git exited with error: {proc.stderr.strip()}')

	# get git commit description
	print(rf'Getting git commit description... ', end='')
	proc = subprocess.run(
		r'git describe --always --long --dirty --abbrev=10 --tags'.split(),
		capture_output=True,
		cwd=root_dir,
		encoding='utf-8',
		check=True
	)
	git_commit_description = proc.stdout.strip()
	print(git_commit_description)
	if not git_commit_hash and proc.stderr.strip():
		raise Exception(rf'git exited with error: {proc.stderr.strip()}')

	# figure out what to call this release
	match = re.search(r'^(?:v?([0-9]+)[.]([0-9]+)[.]([0-9]+)-)?(?:([0-9]+)-)?(?:[a-z0-9]+)(-dirty)?$', git_commit_description, re.S)
	if match is None:
		raise Exception("could not parse git commit description")
	tag_version = ((int(match[1]) if match[1] else 0), (int(match[2]) if match[2] else 0), (int(match[3]) if match[3] else 0))
	tag_version_value = tag_version[0] * 10000 + tag_version[1] * 100 + tag_version[2]
	tag_version = rf'{tag_version[0]}.{tag_version[1]}.{tag_version[2]}'
	commits_ahead = int(match[4]) if match[4] else 0
	dirty = match[5] is not None and len(match[5]) > 0
	print(rf'Most recent tagged version: {tag_version}{ f" ({commits_ahead} commits ahead)" if commits_ahead else ""  }')
	package_name = rf'muu_v{version}'
	if tag_version_value < version_value:
		package_name = rf'{package_name}-pre'
		if commits_ahead > 0:
			package_name = rf'{package_name}{commits_ahead}'
	elif commits_ahead:
			package_name = rf'{package_name}-patch{commits_ahead}'
	if dirty:
		package_name = rf'{package_name}-dev'

	# create temp output directory
	out_dir = Path(root_dir, rf'{package_name}_temp')
	utils.delete_directory(out_dir)
	print(rf'Creating {out_dir}')
	out_dir.mkdir()

	# write VERSION
	with open_text_file_for_writing(out_dir, 'VERSION') as f:
		print(version, file=f, end='')

	# write COMMIT
	with open_text_file_for_writing(out_dir, 'COMMIT') as f:
		print(git_commit_hash, file=f, end='')

	# pull submodules
	if not args.stale and not args.nosubmodules and not (args.nolibs and args.nodocs):
		if not external_dir.exists():
			external_dir.mkdir()
		fetch = '' if args.nofetch else '--no-fetch' # update includes fetch if we didn't fetch earlier
		#if not args.nolibs:
		#	subprocess.run(
		#		(rf'git submodule update --init --depth 1 --jobs {os.cpu_count()} {fetch}'
		#			+ r' external/eigen'
		#			+ r' external/GeometricTools'
		#			+ r' external/muu'
		#			+ r' external/xxHash').split(),
		#		cwd=root_dir,
		#		check=True
		#	)
		if not args.nodocs:
			subprocess.run(
				rf'git submodule update --init --depth 1 --jobs {os.cpu_count()} {fetch} --recursive external/dox'.split(),
				cwd=root_dir,
				check=True
			)
		subprocess.run(
			[ 'git', 'submodule', 'foreach', 'git reset --hard --recurse-submodules'],
			cwd=root_dir,
			check=True
		)
		
	# ignore list for copytree
	garbage = ['*.exp', '*.ilk', '*.build', '*.dox', '*.log']
	if not args.debug:
		garbage.append('*.pdb')
	garbage = shutil.ignore_patterns(*garbage)

	# libs
	if not args.nolibs:

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
		print(rf'Copying {lib_dir}')
		shutil.copytree(lib_dir, Path(out_dir, 'lib'), ignore=garbage)

	# documentation
	if not args.nodocs:

		# generate documentation
		docs_html_dir = Path(docs_dir, 'html')
		if not args.stale and not args.nodocs:
			utils.run_python_script(
				Path(external_dir, 'dox', 'dox.py'),
				'dox.toml',
				cwd=docs_html_dir.parent
			)

		# copy documentation
		utils.assert_existing_directory(docs_html_dir)
		print(rf'Copying {docs_html_dir}')
		shutil.copytree(docs_html_dir, Path(out_dir, 'docs'), ignore=garbage)

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
	print(rf'Copying {include_dir}')
	shutil.copytree(include_dir, Path(out_dir, 'include'), ignore=garbage)

	# copy individual top-level files
	top_level_files = [
		'LICENSE',
		'cpp.hint',
		'muu.natvis'
	]
	for f in top_level_files:
		p = Path(root_dir, f)
		print(rf'Copying {p}')
		shutil.copy(p, out_dir)

	# zip file
	if not args.nozip:
		out_file = Path(root_dir, rf'{package_name}.zip')
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
