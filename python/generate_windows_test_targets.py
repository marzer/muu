#!/usr/bin/env python3
# This file is a part of muu and is subject to the the terms of the MIT license.
# Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

import sys
import os.path as path
import utils
import re
import itertools
from uuid import UUID, uuid5


def main():

	mode_keys = [ '!!debug', '!x86', 'cpplatest', 'noexcept' ]
	modes = [ [] ]
	for n in range(1, len(mode_keys)):
		for combo in itertools.combinations(mode_keys, n):
			modes.append([i for i in combo])
	modes.append(mode_keys)
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

	uuid_namespace = UUID('{51C7001B-048C-4AF0-B598-D75E78FF31F0}')
	configuration_name = lambda x: 'Debug' if x.lower() == 'debug' else 'Release'
	platform_name = lambda x: 'Win32' if x == 'x86' else x
	vs_root = path.join(utils.get_script_folder(), '..', 'vs')
	test_root = path.join(vs_root, 'tests')

	for mode in modes:
		configuration = next(configuration_name(x) for x in mode if x in ('debug', 'release'))
		platform = next(platform_name(x) for x in mode if x in ('x64', 'x86'))
		exceptions ='false' if 'noexcept' in mode else 'Sync'
		standard ='cpplatest' if 'cpplatest' in mode else 'cpp17'
		mode_string = '_'.join(mode)

		# read in muu.vcxproj and modify configuration
		lib_project = utils.read_all_text_from_file(path.join(vs_root, 'muu.vcxproj'))
		lib_project_file_name = f'muu_{mode_string}.vcxproj'
		lib_project_path = path.join(test_root, lib_project_file_name)
		lib_project_uuid = str(uuid5(uuid_namespace, 'lib_' + mode_string)).upper()
		lib_project = re.sub(r'<PropertyGroup\s+Condition="[^"]+?[|]Win32.+?</PropertyGroup>', '', lib_project, flags=re.I | re.S)
		lib_project = re.sub(r'<ProjectConfiguration\s+Include="[^"]+?[|]Win32.+?</ProjectConfiguration>', '', lib_project, flags=re.I | re.S)
		lib_project = re.sub(r'<ProjectGuid>.+?</ProjectGuid>', f'<ProjectGuid>{{{lib_project_uuid}}}</ProjectGuid>', lib_project, flags=re.I)
		lib_project = re.sub(
			r'<ItemDefinitionGroup\s+Label="Magic"\s*>.*?</ItemDefinitionGroup>',
			fr'''<PropertyGroup>
		<IntDir>$(SolutionDir)..\\build\\muu-{lib_project_uuid}\\</IntDir>
		<OutDir>$(SolutionDir)..\\build\\</OutDir>
	</PropertyGroup>
	<ItemDefinitionGroup>
		<ClCompile>
			<ExceptionHandling>{exceptions}</ExceptionHandling>
			<PreprocessorDefinitions Condition="'%(ExceptionHandling)'=='false'">_HAS_EXCEPTIONS=0;%(PreprocessorDefinitions)</PreprocessorDefinitions>
			<PreprocessorDefinitions Condition="'%(ExceptionHandling)'=='false'">SHOULD_HAVE_EXCEPTIONS=0;%(PreprocessorDefinitions)</PreprocessorDefinitions>
			<PreprocessorDefinitions Condition="'%(ExceptionHandling)'!='false'">SHOULD_HAVE_EXCEPTIONS=1;%(PreprocessorDefinitions)</PreprocessorDefinitions>
			<LanguageStandard>std{standard}</LanguageStandard>
		</ClCompile>
	</ItemDefinitionGroup>''',
			lib_project, flags=re.I | re.S
		)
		lib_project = re.sub(r'<\s*(?:None|Text)\s+.+?/>', '', lib_project)
		lib_project = lib_project.replace(r'Include="..', r'Include="..\..')
		lib_project = lib_project.replace('muu.natvis', '..\\muu.natvis')
		lib_project = lib_project.replace(r'|x64', fr'|{platform}')
		lib_project = lib_project.replace('$(ProjectDir)\\', '..\\')
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

		# write out corresponding test project
		test_project_path = path.join(test_root, f'test_{mode_string}.vcxproj')
		test_project_uuid = str(uuid5(uuid_namespace, mode_string)).upper()
		print(f'Writing to {test_project_path}')
		with open(test_project_path, 'w', encoding='utf-8-sig', newline='\r\n') as file:
			write = lambda txt: print(txt, file=file)
			write(fr'''
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
	<ItemGroup Label="ProjectConfigurations">
		<ProjectConfiguration Include="{configuration}|{platform}">
			<Configuration>{configuration}</Configuration>
			<Platform>{platform}</Platform>
		</ProjectConfiguration>
	</ItemGroup>
	<PropertyGroup Label="Globals">
		<VCProjectVersion>16.0</VCProjectVersion>
		<ProjectGuid>{{{test_project_uuid}}}</ProjectGuid>
		<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
	</PropertyGroup>
	<Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|{platform}'" Label="Configuration">
		<ConfigurationType>Application</ConfigurationType>
		<UseDebugLibraries>true</UseDebugLibraries>
		<PlatformToolset>v142</PlatformToolset>
		<CharacterSet>MultiByte</CharacterSet>
	</PropertyGroup>
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|{platform}'" Label="Configuration">
		<ConfigurationType>Application</ConfigurationType>
		<UseDebugLibraries>false</UseDebugLibraries>
		<PlatformToolset>v142</PlatformToolset>
		<WholeProgramOptimization>true</WholeProgramOptimization>
		<CharacterSet>MultiByte</CharacterSet>
	</PropertyGroup>
	<Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
	<ImportGroup Label="PropertySheets">
		<Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props"
			Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
	</ImportGroup>
	<Import Project="../muu.props" />
	<ItemDefinitionGroup>
		<ClCompile>
			<AdditionalIncludeDirectories>..\..\tests;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
			<ExceptionHandling>{exceptions}</ExceptionHandling>
			<PrecompiledHeader>Use</PrecompiledHeader>
			<PrecompiledHeaderFile>tests.h</PrecompiledHeaderFile>
			<PreprocessorDefinitions Condition="'%(ExceptionHandling)'=='false'">_HAS_EXCEPTIONS=0;%(PreprocessorDefinitions)</PreprocessorDefinitions>
			<PreprocessorDefinitions Condition="'%(ExceptionHandling)'=='false'">SHOULD_HAVE_EXCEPTIONS=0;%(PreprocessorDefinitions)</PreprocessorDefinitions>
			<PreprocessorDefinitions Condition="'%(ExceptionHandling)'!='false'">SHOULD_HAVE_EXCEPTIONS=1;%(PreprocessorDefinitions)</PreprocessorDefinitions>
			<LanguageStandard>std{standard}</LanguageStandard>
		</ClCompile>
	</ItemDefinitionGroup>
	<PropertyGroup>
		<LocalDebuggerWorkingDirectory>..\..\tests\</LocalDebuggerWorkingDirectory>
		<IntDir>$(SolutionDir)..\build\tests-{test_project_uuid}\</IntDir>
		<OutDir>$(SolutionDir)..\\build\\</OutDir>
	</PropertyGroup>
	<ItemGroup>
		<ClCompile Include="..\..\tests\accumulator.cpp" />
		<ClCompile Include="..\..\tests\aligned_alloc.cpp" />
		<ClCompile Include="..\..\tests\blob.cpp" />
		<ClCompile Include="..\..\tests\compressed_pair.cpp" />
		<ClCompile Include="..\..\tests\core_intrinsics_bit.cpp" />
		<ClCompile Include="..\..\tests\core_intrinsics_other.cpp" />
		<ClCompile Include="..\..\tests\core_meta.cpp" />
		<ClCompile Include="..\..\tests\half.cpp" />
		<ClCompile Include="..\..\tests\hashing.cpp" />
		<ClCompile Include="..\..\tests\main.cpp">
			<PrecompiledHeader>NotUsing</PrecompiledHeader>
		</ClCompile>
		<ClCompile Include="..\..\tests\scope_guard.cpp" />
		<ClCompile Include="..\..\tests\span.cpp" />
		<ClCompile Include="..\..\tests\string_param.cpp" />
		<ClCompile Include="..\..\tests\strings.cpp" />
		<ClCompile Include="..\..\tests\tagged_ptr.cpp" />
		<ClCompile Include="..\..\tests\tests.cpp">
			<PrecompiledHeader>Create</PrecompiledHeader>
		</ClCompile>
		<ClCompile Include="..\..\tests\thread_pool.cpp" />
		<ClCompile Include="..\..\tests\unicode_char.cpp" />
		<ClCompile Include="..\..\tests\unicode_char16_t.cpp" />
		<ClCompile Include="..\..\tests\unicode_char32_t.cpp" />
		<ClCompile Include="..\..\tests\unicode_char8_t.cpp" />
		<ClCompile Include="..\..\tests\unicode_unsigned_char.cpp" />
		<ClCompile Include="..\..\tests\unicode_wchar_t.cpp" />
		<ClCompile Include="..\..\tests\uuid.cpp" />
		<ClCompile Include="..\..\tests\vector_char.cpp" />
		<ClCompile Include="..\..\tests\vector_double.cpp" />
		<ClCompile Include="..\..\tests\vector_float.cpp" />
		<ClCompile Include="..\..\tests\vector_half.cpp" />
		<ClCompile Include="..\..\tests\vector_int.cpp" />
		<ClCompile Include="..\..\tests\vector_long.cpp" />
		<ClCompile Include="..\..\tests\vector_long_double.cpp" />
		<ClCompile Include="..\..\tests\vector_long_long.cpp" />
		<ClCompile Include="..\..\tests\vector_short.cpp" />
	</ItemGroup>
	<ItemGroup>
		<Natvis Include="..\muu.natvis" />
	</ItemGroup>
	<ItemGroup>
		<ClInclude Include="..\..\tests\catch2.h" />
		<ClInclude Include="..\..\tests\float_test_data.h" />
		<ClInclude Include="..\..\tests\settings.h" />
		<ClInclude Include="..\..\tests\tests.h" />
		<ClInclude Include="..\..\tests\unicode.h" />
		<ClInclude Include="..\..\tests\vector_template.h" />
	</ItemGroup>
	<ItemGroup>
		<None Include="..\..\tests\meson.build" />
		<None Include="..\.runsettings" />
		<None Include="..\muu.props" />
	</ItemGroup>
	<ItemGroup>
		<ProjectReference Include="{lib_project_file_name}">
			<Project>{{{lib_project_uuid}}}</Project>
		</ProjectReference>
	</ItemGroup>
	<Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
</Project>
			'''.strip()
		)

if __name__ == '__main__':
	utils.run(main)
