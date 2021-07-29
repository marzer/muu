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


def main():

	uuid_namespace = UUID('{51C7001B-048C-4AF0-B598-D75E78FF31F0}')
	root = Path(utils.entry_script_dir(), '..').resolve()

	# read in muu.vcxproj and emit the static version
	lib_project_static = utils.read_all_text_from_file(Path(root, 'muu.vcxproj')).encode()
	while lib_project_static.startswith(codecs.BOM_UTF8):
		lib_project_static = lib_project_static[len(codecs.BOM_UTF8):]
	lib_project_static = lib_project_static.decode('utf-8')
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
	test_root = Path(root, 'tests', 'vs')
	os.makedirs(test_root, exist_ok = True)
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
			fr'''<PropertyGroup>
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

		# write out corresponding test project
		test_project_path = Path(test_root, f'test_{mode_string}.vcxproj')
		test_project_uuid = str(uuid5(uuid_namespace, mode_string)).upper()
		print(f'Writing to {test_project_path}')
		with open(test_project_path, 'w', encoding='utf-8-sig', newline='\r\n') as file:
			print(fr'''<?xml version="1.0" encoding="utf-8"?>
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
		<!-- <PreferredToolArchitecture>x64</PreferredToolArchitecture> -->
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
		<WholeProgramOptimization>false</WholeProgramOptimization>
		<CharacterSet>MultiByte</CharacterSet>
	</PropertyGroup>
	<Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
	<ImportGroup Label="PropertySheets">
		<Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props"
			Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
	</ImportGroup>
	<Import Project="$(SolutionDir)muu.props" />
	<PropertyGroup>
		<IncludePath>$(SolutionDir)external;$(IncludePath)</IncludePath>
		<IncludePath>$(SolutionDir)include;$(IncludePath)</IncludePath>
		<IncludePath>$(SolutionDir)tests;$(IncludePath)</IncludePath>
	</PropertyGroup>
	<ItemDefinitionGroup>
		<ClCompile>
			<ExceptionHandling>{exceptions}</ExceptionHandling>
			<PrecompiledHeader>Use</PrecompiledHeader>
			<PrecompiledHeaderFile>tests.h</PrecompiledHeaderFile>
			<PreprocessorDefinitions>MUU_DLL=0;%(PreprocessorDefinitions)</PreprocessorDefinitions>
			<PreprocessorDefinitions Condition="'%(ExceptionHandling)'=='false'">_HAS_EXCEPTIONS=0;%(PreprocessorDefinitions)</PreprocessorDefinitions>
			<PreprocessorDefinitions Condition="'%(ExceptionHandling)'=='false'">SHOULD_HAVE_EXCEPTIONS=0;%(PreprocessorDefinitions)</PreprocessorDefinitions>
			<PreprocessorDefinitions Condition="'%(ExceptionHandling)'!='false'">SHOULD_HAVE_EXCEPTIONS=1;%(PreprocessorDefinitions)</PreprocessorDefinitions>
			<LanguageStandard>std{standard}</LanguageStandard>
			<DebugInformationFormat Condition="!$(Configuration.ToLower().Contains('debug'))">None</DebugInformationFormat>
			<WarningLevel>EnableAllWarnings</WarningLevel>
			<DisableSpecificWarnings>%(DisableSpecificWarnings);4127</DisableSpecificWarnings> <!-- conditional expr is constant -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);4324</DisableSpecificWarnings> <!-- structure was padded due to alignment specifier -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);4464</DisableSpecificWarnings> <!-- relative include path contains '..' -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);4505</DisableSpecificWarnings> <!-- unreferenced local function removed -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);4514</DisableSpecificWarnings> <!-- unreferenced inline function has been removed -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);4577</DisableSpecificWarnings> <!-- 'noexcept' used with no exception handling mode specified -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);4582</DisableSpecificWarnings> <!-- constructor is not implicitly called -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);4623</DisableSpecificWarnings> <!-- default constructor was implicitly defined as deleted -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);4625</DisableSpecificWarnings> <!-- copy constructor was implicitly defined as deleted -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);4626</DisableSpecificWarnings> <!-- assignment operator was implicitly defined as deleted -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);4710</DisableSpecificWarnings> <!-- function not inlined -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);4711</DisableSpecificWarnings> <!-- function selected for automatic expansion -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);4738</DisableSpecificWarnings> <!-- storing 32-bit float result in memory -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);4820</DisableSpecificWarnings> <!-- N bytes padding added -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);4866</DisableSpecificWarnings> <!-- compiler may not enforce ltr eval in operator[] -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);4868</DisableSpecificWarnings> <!-- compiler may not enforce ltr eval in initializer list -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);4883</DisableSpecificWarnings> <!-- function size suppresses optimizations -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);4946</DisableSpecificWarnings> <!-- reinterpret_cast used between related classes -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);5026</DisableSpecificWarnings> <!-- move constructor was implicitly defined as deleted -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);5027</DisableSpecificWarnings> <!-- move assignment operator was implicitly defined as deleted -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);5039</DisableSpecificWarnings> <!-- potentially throwing function passed to 'extern "C"' -->
			<DisableSpecificWarnings>%(DisableSpecificWarnings);5045</DisableSpecificWarnings> <!-- Compiler will insert Spectre mitigation -->
		</ClCompile>
		<Link>
			<GenerateDebugInformation Condition="!$(Configuration.ToLower().Contains('debug'))">false</GenerateDebugInformation>
			<LinkTimeCodeGeneration>Default</LinkTimeCodeGeneration>
			<SubSystem>Console</SubSystem>
		</Link>
	</ItemDefinitionGroup>
	<PropertyGroup>
		<LocalDebuggerWorkingDirectory>$(SolutionDir)tests</LocalDebuggerWorkingDirectory>
		<IntDir>$(SolutionDir)build\tests-{test_project_uuid}\</IntDir>
		<OutDir>$(SolutionDir)build\</OutDir>
		<LinkIncremental>false</LinkIncremental>
	</PropertyGroup>
	<ItemGroup>
		<ClCompile Include="..\accumulator.cpp" />
		<ClCompile Include="..\blob.cpp" />
		<ClCompile Include="..\bounding_box_0.cpp" />
		<ClCompile Include="..\bounding_box_1.cpp" />
		<ClCompile Include="..\bounding_box_2.cpp" />
		<ClCompile Include="..\bounding_box_3.cpp" />
		<ClCompile Include="..\compressed_pair.cpp" />
		<ClCompile Include="..\core_bit.cpp" />
		<ClCompile Include="..\core_other.cpp" />
		<ClCompile Include="..\core_meta.cpp" />
		<ClCompile Include="..\function_view.cpp" />
		<ClCompile Include="..\half.cpp" />
		<ClCompile Include="..\hashing.cpp" />
		<ClCompile Include="..\infinity_or_nan_0.cpp" />
		<ClCompile Include="..\infinity_or_nan_1.cpp" />
		<ClCompile Include="..\infinity_or_nan_2.cpp" />
		<ClCompile Include="..\infinity_or_nan_3.cpp" />
		<ClCompile Include="..\main.cpp">
			<PrecompiledHeader>NotUsing</PrecompiledHeader>
		</ClCompile>
		<ClCompile Include="..\math.cpp" />
		<ClCompile Include="..\matrix_0.cpp" />
		<ClCompile Include="..\matrix_1.cpp" />
		<ClCompile Include="..\matrix_2.cpp" />
		<ClCompile Include="..\matrix_3.cpp" />
		<ClCompile Include="..\matrix_4.cpp" />
		<ClCompile Include="..\matrix_5.cpp" />
		<ClCompile Include="..\matrix_6.cpp" />
		<ClCompile Include="..\matrix_7.cpp" />
		<ClCompile Include="..\oriented_bounding_box_0.cpp" />
		<ClCompile Include="..\oriented_bounding_box_1.cpp" />
		<ClCompile Include="..\oriented_bounding_box_2.cpp" />
		<ClCompile Include="..\oriented_bounding_box_3.cpp" />
		<ClCompile Include="..\plane_0.cpp" />
		<ClCompile Include="..\plane_1.cpp" />
		<ClCompile Include="..\plane_2.cpp" />
		<ClCompile Include="..\plane_3.cpp" />
		<ClCompile Include="..\quaternion_0.cpp" />
		<ClCompile Include="..\quaternion_1.cpp" />
		<ClCompile Include="..\quaternion_2.cpp" />
		<ClCompile Include="..\quaternion_3.cpp" />
		<ClCompile Include="..\scope_guard.cpp" />
		<ClCompile Include="..\span.cpp" />
		<ClCompile Include="..\string_param.cpp" />
		<ClCompile Include="..\strings.cpp" />
		<ClCompile Include="..\tagged_ptr.cpp" />
		<ClCompile Include="..\tests.cpp">
			<PrecompiledHeader>Create</PrecompiledHeader>
		</ClCompile>
		<ClCompile Include="..\thread_pool.cpp" />
		<ClCompile Include="..\unicode_char.cpp" />
		<ClCompile Include="..\unicode_char16_t.cpp" />
		<ClCompile Include="..\unicode_char32_t.cpp" />
		<ClCompile Include="..\unicode_char8_t.cpp" />
		<ClCompile Include="..\unicode_unsigned_char.cpp" />
		<ClCompile Include="..\uuid.cpp" />
		<ClCompile Include="..\vector_0.cpp" />
		<ClCompile Include="..\vector_1.cpp" />
		<ClCompile Include="..\vector_2.cpp" />
		<ClCompile Include="..\vector_3.cpp" />
		<ClCompile Include="..\vector_4.cpp" />
		<ClCompile Include="..\vector_5.cpp" />
		<ClCompile Include="..\vector_6.cpp" />
		<ClCompile Include="..\vector_7.cpp" />
	</ItemGroup>
	<ItemGroup>
		<Natvis Include="$(SolutionDir)muu.natvis" />
	</ItemGroup>
	<ItemGroup>
		<ClInclude Include="..\batching.h" />
		<ClInclude Include="..\bounding_box_tests.h" />
		<ClInclude Include="..\catch2.h" />
		<ClInclude Include="..\float_test_data.h" />
		<ClInclude Include="..\infinity_or_nan_tests.h" />
		<ClInclude Include="..\matrix_tests.h" />
		<ClInclude Include="..\oriented_bounding_box_tests.h" />
		<ClInclude Include="..\plane_tests.h" />
		<ClInclude Include="..\quaternion_tests.h" />
		<ClInclude Include="..\settings.h" />
		<ClInclude Include="..\tests.h" />
		<ClInclude Include="..\unicode.h" />
		<ClInclude Include="..\vector_tests.h" />
	</ItemGroup>
	<ItemGroup>
		<None Include="..\cpp.hint" />
		<None Include="..\meson.build" />
		<None Include="$(SolutionDir).runsettings" />
		<None Include="$(SolutionDir)muu.props" />
	</ItemGroup>
	<ItemGroup>
		<ProjectReference Include="{lib_project_file_name}">
			<Project>{{{lib_project_uuid}}}</Project>
		</ProjectReference>
	</ItemGroup>
	<Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
</Project>
''',
			file=file
		)

		# write out corresponding test project filters file
		test_project_filters_path = Path(test_root, f'test_{mode_string}.vcxproj.filters')
		print(f'Writing to {test_project_filters_path}')
		with open(test_project_filters_path, 'w', encoding='utf-8-sig', newline='\r\n') as file:
			print(r'''
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
    <Natvis Include="$(SolutionDir)muu.natvis" />
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="..\accumulator.cpp" />
    <ClCompile Include="..\blob.cpp" />
    <ClCompile Include="..\compressed_pair.cpp" />
	<ClCompile Include="..\function_view.cpp" />
    <ClCompile Include="..\half.cpp" />
    <ClCompile Include="..\hashing.cpp" />
    <ClCompile Include="..\main.cpp" />
    <ClCompile Include="..\math.cpp" />
    <ClCompile Include="..\scope_guard.cpp" />
    <ClCompile Include="..\span.cpp" />
    <ClCompile Include="..\string_param.cpp" />
    <ClCompile Include="..\strings.cpp" />
    <ClCompile Include="..\tagged_ptr.cpp" />
    <ClCompile Include="..\tests.cpp" />
    <ClCompile Include="..\thread_pool.cpp" />
    <ClCompile Include="..\uuid.cpp" />
    <ClCompile Include="..\matrix_0.cpp">
      <Filter>matrix</Filter>
    </ClCompile>
    <ClCompile Include="..\matrix_1.cpp">
      <Filter>matrix</Filter>
    </ClCompile>
    <ClCompile Include="..\matrix_2.cpp">
      <Filter>matrix</Filter>
    </ClCompile>
    <ClCompile Include="..\matrix_3.cpp">
      <Filter>matrix</Filter>
    </ClCompile>
    <ClCompile Include="..\matrix_4.cpp">
      <Filter>matrix</Filter>
    </ClCompile>
    <ClCompile Include="..\matrix_5.cpp">
      <Filter>matrix</Filter>
    </ClCompile>
    <ClCompile Include="..\matrix_6.cpp">
      <Filter>matrix</Filter>
    </ClCompile>
    <ClCompile Include="..\matrix_7.cpp">
      <Filter>matrix</Filter>
    </ClCompile>
    <ClCompile Include="..\vector_0.cpp">
      <Filter>vector</Filter>
    </ClCompile>
    <ClCompile Include="..\vector_1.cpp">
      <Filter>vector</Filter>
    </ClCompile>
    <ClCompile Include="..\vector_2.cpp">
      <Filter>vector</Filter>
    </ClCompile>
    <ClCompile Include="..\vector_3.cpp">
      <Filter>vector</Filter>
    </ClCompile>
    <ClCompile Include="..\vector_4.cpp">
      <Filter>vector</Filter>
    </ClCompile>
    <ClCompile Include="..\vector_5.cpp">
      <Filter>vector</Filter>
    </ClCompile>
    <ClCompile Include="..\vector_6.cpp">
      <Filter>vector</Filter>
    </ClCompile>
    <ClCompile Include="..\vector_7.cpp">
      <Filter>vector</Filter>
    </ClCompile>
    <ClCompile Include="..\quaternion_0.cpp">
      <Filter>quaternion</Filter>
    </ClCompile>
    <ClCompile Include="..\quaternion_1.cpp">
      <Filter>quaternion</Filter>
    </ClCompile>
    <ClCompile Include="..\quaternion_2.cpp">
      <Filter>quaternion</Filter>
    </ClCompile>
    <ClCompile Include="..\quaternion_3.cpp">
      <Filter>quaternion</Filter>
    </ClCompile>
    <ClCompile Include="..\bounding_box_0.cpp">
      <Filter>bounding_box</Filter>
    </ClCompile>
    <ClCompile Include="..\bounding_box_1.cpp">
      <Filter>bounding_box</Filter>
    </ClCompile>
    <ClCompile Include="..\bounding_box_2.cpp">
      <Filter>bounding_box</Filter>
    </ClCompile>
    <ClCompile Include="..\bounding_box_3.cpp">
      <Filter>bounding_box</Filter>
    </ClCompile>
    <ClCompile Include="..\oriented_bounding_box_0.cpp">
      <Filter>oriented_bounding_box</Filter>
    </ClCompile>
    <ClCompile Include="..\oriented_bounding_box_1.cpp">
      <Filter>oriented_bounding_box</Filter>
    </ClCompile>
    <ClCompile Include="..\oriented_bounding_box_2.cpp">
      <Filter>oriented_bounding_box</Filter>
    </ClCompile>
    <ClCompile Include="..\oriented_bounding_box_3.cpp">
      <Filter>oriented_bounding_box</Filter>
    </ClCompile>
    <ClCompile Include="..\plane_0.cpp">
      <Filter>plane</Filter>
    </ClCompile>
    <ClCompile Include="..\plane_1.cpp">
      <Filter>plane</Filter>
    </ClCompile>
    <ClCompile Include="..\plane_2.cpp">
      <Filter>plane</Filter>
    </ClCompile>
    <ClCompile Include="..\plane_3.cpp">
      <Filter>plane</Filter>
    </ClCompile>
    <ClCompile Include="..\core_bit.cpp">
      <Filter>core</Filter>
    </ClCompile>
    <ClCompile Include="..\core_meta.cpp">
      <Filter>core</Filter>
    </ClCompile>
    <ClCompile Include="..\core_other.cpp">
      <Filter>core</Filter>
    </ClCompile>
    <ClCompile Include="..\unicode_unsigned_char.cpp">
      <Filter>unicode</Filter>
    </ClCompile>
    <ClCompile Include="..\unicode_char.cpp">
      <Filter>unicode</Filter>
    </ClCompile>
    <ClCompile Include="..\unicode_char8_t.cpp">
      <Filter>unicode</Filter>
    </ClCompile>
    <ClCompile Include="..\unicode_char16_t.cpp">
      <Filter>unicode</Filter>
    </ClCompile>
    <ClCompile Include="..\unicode_char32_t.cpp">
      <Filter>unicode</Filter>
    </ClCompile>
    <ClCompile Include="..\infinity_or_nan_0.cpp">
      <Filter>infinity_or_nan</Filter>
    </ClCompile>
    <ClCompile Include="..\infinity_or_nan_1.cpp">
      <Filter>infinity_or_nan</Filter>
    </ClCompile>
    <ClCompile Include="..\infinity_or_nan_2.cpp">
      <Filter>infinity_or_nan</Filter>
    </ClCompile>
    <ClCompile Include="..\infinity_or_nan_3.cpp">
      <Filter>infinity_or_nan</Filter>
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\batching.h" />
    <ClInclude Include="..\catch2.h" />
    <ClInclude Include="..\float_test_data.h" />
    <ClInclude Include="..\settings.h" />
    <ClInclude Include="..\tests.h" />
    <ClInclude Include="..\matrix_tests.h">
      <Filter>matrix</Filter>
    </ClInclude>
    <ClInclude Include="..\vector_tests.h">
      <Filter>vector</Filter>
    </ClInclude>
    <ClInclude Include="..\quaternion_tests.h">
      <Filter>quaternion</Filter>
    </ClInclude>
    <ClInclude Include="..\bounding_box_tests.h">
      <Filter>bounding_box</Filter>
    </ClInclude>
    <ClInclude Include="..\oriented_bounding_box_tests.h">
      <Filter>oriented_bounding_box</Filter>
    </ClInclude>
    <ClInclude Include="..\plane_tests.h">
      <Filter>plane</Filter>
    </ClInclude>
    <ClInclude Include="..\unicode.h">
      <Filter>unicode</Filter>
    </ClInclude>
    <ClInclude Include="..\infinity_or_nan_tests.h">
      <Filter>infinity_or_nan</Filter>
    </ClInclude>
  </ItemGroup>
  <ItemGroup>
    <None Include="..\cpp.hint" />
    <None Include="..\meson.build" />
    <None Include="$(SolutionDir).runsettings" />
    <None Include="$(SolutionDir)muu.props" />
  </ItemGroup>
  <ItemGroup>
    <Filter Include="bounding_box">
      <UniqueIdentifier>{4390118b-b943-4315-836b-7f6303087e64}</UniqueIdentifier>
    </Filter>
    <Filter Include="matrix">
      <UniqueIdentifier>{b8679be1-2be2-46e3-bd50-a34abc938907}</UniqueIdentifier>
    </Filter>
    <Filter Include="oriented_bounding_box">
      <UniqueIdentifier>{a3dabe9b-1c17-4a45-bf49-862484ef5e41}</UniqueIdentifier>
    </Filter>
    <Filter Include="plane">
      <UniqueIdentifier>{dd8a9e50-48e8-48eb-8d36-9336d79a9f0d}</UniqueIdentifier>
    </Filter>
    <Filter Include="quaternion">
      <UniqueIdentifier>{34d85895-ae82-4486-b6c1-b681079e8b46}</UniqueIdentifier>
    </Filter>
    <Filter Include="vector">
      <UniqueIdentifier>{43aab29d-8fac-43d5-b858-80b691a6b0a2}</UniqueIdentifier>
    </Filter>
    <Filter Include="core">
      <UniqueIdentifier>{23c11922-0b9e-4b0a-933c-f64905fcde71}</UniqueIdentifier>
    </Filter>
    <Filter Include="unicode">
      <UniqueIdentifier>{202ce89f-5555-4e74-8b3b-3c56c20d20ab}</UniqueIdentifier>
    </Filter>
    <Filter Include="infinity_or_nan">
      <UniqueIdentifier>{b74fb7e2-dedb-4cd0-9a91-1f963c7c7f23}</UniqueIdentifier>
    </Filter>
  </ItemGroup>
</Project>
'''.strip(),
			file=file
		)

if __name__ == '__main__':
	utils.run(main)
