#!/usr/bin/env python3
# This file is a part of muu and is subject to the the terms of the MIT license.
# Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

import utils
import sys
import os
import re
import itertools
import shutil
import codecs
from uuid import UUID, uuid5
from pathlib import Path

def read_utf8_file_and_strip_bom(path):
	text = utils.read_all_text_from_file(path).encode()
	while text.startswith(codecs.BOM_UTF8):
		text = text[len(codecs.BOM_UTF8):]
	return text.decode('utf-8')

def main():

	uuid_namespace = UUID('{51C7001B-048C-4AF0-B598-D75E78FF31F0}')
	root = Path(utils.entry_script_dir(), '..').resolve()

	# read in muu.vcxproj and emit the static version
	lib_project_static = read_utf8_file_and_strip_bom(Path(root, 'muu.vcxproj'))
	lib_project_static = re.sub(r'<StaticDllExport>.+?</StaticDllExport>', '<StaticDllExport>false</StaticDllExport>', lib_project_static, flags=re.I)
	lib_project_static = re.sub(r'<ConfigurationType>.+?</ConfigurationType>', '<ConfigurationType>StaticLibrary</ConfigurationType>', lib_project_static, flags=re.I)
	lib_project_static_uuid = str(uuid5(uuid_namespace, 'muu_lib_static')).upper()
	lib_project_static = re.sub(r'<ProjectGuid>.+?</ProjectGuid>', f'<ProjectGuid>{{{lib_project_static_uuid}}}</ProjectGuid>', lib_project_static, flags=re.I)
	lib_project_static_temp = re.sub(
		r'<ItemDefinitionGroup\s+Label="Magic"\s*>.*?</ItemDefinitionGroup>',
		'<PropertyGroup>\n    <TargetName>muu</TargetName>\n  </PropertyGroup>',
		lib_project_static, flags=re.I | re.S
	)
	lib_project_static_path = Path(root, 'muu_static.vcxproj')
	print(f'Writing to {lib_project_static_path}')
	with open(lib_project_static_path, 'w', encoding='utf-8-sig', newline='\r\n') as file:
		print(lib_project_static_temp, file=file)
	shutil.copyfile(
		Path(root, 'muu.vcxproj.filters'),
		Path(root, 'muu_static.vcxproj.filters')
	)

	# read in test files to use as templates
	test_root = Path(root, 'tests', 'vs')
	test_project_template_path = Path(test_root, r'test_debug_x64.vcxproj').resolve()
	test_project = read_utf8_file_and_strip_bom(test_project_template_path)
	test_project_filters_template_path = Path(test_root, r'test_debug_x64.vcxproj.filters').resolve()
	test_project_filters = read_utf8_file_and_strip_bom(test_project_filters_template_path)

	# generate test permutations
	mode_keys = [ '!!debug', '!x86', 'cpplatest', 'noexcept' ]
	modes = [ [], [k for k in mode_keys] ]
	for n in range(1, len(mode_keys)):
		for combo in itertools.combinations(mode_keys, n):
			modes.append([i for i in combo])
	for mode in modes:
		if '!x86' not in mode:
			mode.insert(0, '!x64')
		if '!!debug' not in mode:
			mode.insert(0, '!!release')
		mode.sort()
		for i in range(0, len(mode)):
			while mode[i].startswith('!'):
				mode[i] = mode[i][1:]
	modes.sort()
	modes = [m for m in modes if not ('cpplatest' in m and 'noexcept' in m)]
	configuration_name = lambda x: 'Debug' if x.lower() == 'debug' else 'Release'
	platform_name = lambda x: 'Win32' if x == 'x86' else x
	for mode in modes:
		configuration = next(configuration_name(x) for x in mode if x in ('debug', 'release'))
		platform = next(platform_name(x) for x in mode if x in ('x64', 'x86'))
		exceptions ='false' if 'noexcept' in mode else 'Sync'
		standard ='cpplatest' if 'cpplatest' in mode else 'cpp17'
		mode_string = '_'.join(mode)

		# copy muu.vcxproj and modify configuration
		lib_project = lib_project_static
		lib_project_file_name = f'muu_{mode_string}.vcxproj'
		lib_project_path = Path(test_root, lib_project_file_name)
		lib_project_uuid = str(uuid5(uuid_namespace, 'lib_' + mode_string)).upper()
		lib_project = re.sub(r'<PropertyGroup\s+Condition="[^"]+?[|]Win32.+?</PropertyGroup>', '', lib_project, flags=re.I | re.S)
		lib_project = re.sub(r'<ProjectConfiguration\s+Include="[^"]+?[|]Win32.+?</ProjectConfiguration>', '', lib_project, flags=re.I | re.S)
		lib_project = re.sub(r'<ProjectGuid>.+?</ProjectGuid>', f'<ProjectGuid>{{{lib_project_uuid}}}</ProjectGuid>', lib_project, flags=re.I)
		lib_project = re.sub(r'<WholeProgramOptimization>.+?</WholeProgramOptimization>', '<WholeProgramOptimization>false</WholeProgramOptimization>', lib_project, flags=re.I)
		lib_project = re.sub(r'<PreferredToolArchitecture>.+?</PreferredToolArchitecture>', '', lib_project, flags=re.I)
		lib_project = re.sub(
			r'<ItemDefinitionGroup\s+Label="Magic"\s*>.*?</ItemDefinitionGroup>',
			rf'''<PropertyGroup>
		<IntDir>$(SolutionDir)build\\muu-{lib_project_uuid}\\</IntDir>
		<OutDir>$(SolutionDir)build\\</OutDir>
	</PropertyGroup>
	<ItemDefinitionGroup>
		<ClCompile>
			<ExceptionHandling>{exceptions}</ExceptionHandling>
			<PreprocessorDefinitions Condition="'%(ExceptionHandling)'=='false'">_HAS_EXCEPTIONS=0;%(PreprocessorDefinitions)</PreprocessorDefinitions>
			<PreprocessorDefinitions Condition="'%(ExceptionHandling)'=='false'">SHOULD_HAVE_EXCEPTIONS=0;%(PreprocessorDefinitions)</PreprocessorDefinitions>
			<PreprocessorDefinitions Condition="'%(ExceptionHandling)'!='false'">SHOULD_HAVE_EXCEPTIONS=1;%(PreprocessorDefinitions)</PreprocessorDefinitions>
			<LanguageStandard>std{standard}</LanguageStandard>
		</ClCompile>
	</ItemDefinitionGroup>'''.replace('\t', '  '),
			lib_project, flags=re.I | re.S
		)
		lib_project = re.sub(r'<\s*(?:None|Text)\s+.+?/>', '', lib_project)
		lib_project = lib_project.replace(r'muu.props', '$(SolutionDir)muu.props')
		lib_project = lib_project.replace(r'<IncludePath>$(ProjectDir)', '<IncludePath>$(SolutionDir)')
		lib_project = lib_project.replace(r'<ClInclude Include="', '<ClInclude Include="$(SolutionDir)')
		lib_project = lib_project.replace(r'<ClCompile Include="', '<ClCompile Include="$(SolutionDir)')
		lib_project = lib_project.replace(r'<Natvis Include="', '<Natvis Include="$(SolutionDir)')
		lib_project = lib_project.replace(r'|x64', fr'|{platform}')
		lib_project = lib_project.replace(r'<Platform>x64</Platform>', fr'<Platform>{platform}</Platform>')
		if configuration == 'Debug':
			lib_project = re.sub(r'<ProjectConfiguration\s+Include="Release.*?/ProjectConfiguration>', '', lib_project, flags=re.S | re.I)
		if configuration == 'Release':
			lib_project = re.sub(r'<ProjectConfiguration\s+Include="Debug.*?/ProjectConfiguration>', '', lib_project, flags=re.S | re.I)
		lib_project = re.sub('<ItemGroup>\s*</ItemGroup>', '', lib_project, flags=re.S)
		lib_project = re.sub('\n(\s*\n)+', '\n', lib_project)
		print(f'Writing to {lib_project_path}')
		with open(lib_project_path, 'w', encoding='utf-8-sig', newline='\r\n') as file:
			print(lib_project.strip(), file=file)

		# modify and emit corresponding test project
		test_project_path = Path(test_root, f'test_{mode_string}.vcxproj').resolve()
		test_project_uuid = str(uuid5(uuid_namespace, mode_string)).upper()
		if test_project_path != test_project_template_path:
			test_project = re.sub(r'<ProjectConfiguration\s+Include=".*?">', rf'<ProjectConfiguration Include="{configuration}|{platform}">', test_project, flags=re.I | re.S)
			test_project = re.sub(r'<Configuration>.*?</Configuration>', rf'<Configuration>{configuration}</Configuration>', test_project, flags=re.I | re.S)
			test_project = re.sub(r'<Platform>.*?</Platform>', rf'<Platform>{platform}</Platform>', test_project, flags=re.I | re.S)
			test_project = re.sub(r'<ProjectGuid>.*?</ProjectGuid>', rf'<ProjectGuid>{{{test_project_uuid}}}</ProjectGuid>', test_project, flags=re.I | re.S)
			test_project = re.sub(r'<IntDir>.*?</IntDir>', rf'<IntDir>$(SolutionDir)build\\tests-{test_project_uuid}\\</IntDir>', test_project, flags=re.I | re.S)
			test_project = re.sub(r'<LanguageStandard>.*?</LanguageStandard>', rf'<LanguageStandard>std{standard}</LanguageStandard>', test_project, flags=re.I | re.S)
			test_project = re.sub(r'<ExceptionHandling>.*?</ExceptionHandling>', rf'<ExceptionHandling>{exceptions}</ExceptionHandling>', test_project, flags=re.I | re.S)
			test_project = re.sub(
				r'''<PropertyGroup\s+Condition="'\$\(Configuration\)\|\$\(Platform\)'=='Debug\|.*?'"\s+Label="Configuration">''',
				rf'''<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|{platform}'" Label="Configuration">''', test_project,
				flags=re.I | re.S
			)
			test_project = re.sub(
				r'''<PropertyGroup\s+Condition="'\$\(Configuration\)\|\$\(Platform\)'=='Release\|.*?'"\s+Label="Configuration">''',
				rf'''<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|{platform}'" Label="Configuration">''', test_project,
				flags=re.I | re.S
			)
			test_project = re.sub(
				r'''<ProjectReference Include=".*?">.*?</ProjectReference>''',
				rf'''<ProjectReference Include="{lib_project_file_name}">
			<Project>{{{lib_project_uuid}}}</Project>
		</ProjectReference>''',
				test_project, flags=re.I | re.S
			)
			print(f'Writing to {test_project_path}')
			with open(test_project_path, 'w', encoding='utf-8-sig', newline='\r\n') as file:
				file.write(test_project)

		# write out corresponding test project filters file
		test_project_filters_path = Path(test_root, f'test_{mode_string}.vcxproj.filters').resolve()
		if test_project_filters_path != test_project_filters_template_path:
			print(f'Writing to {test_project_filters_path}')
			with open(test_project_filters_path, 'w', encoding='utf-8-sig', newline='\r\n') as file:
				file.write(test_project_filters)

if __name__ == '__main__':
	utils.run(main)
