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

	test_root = path.join(utils.get_script_folder(), '..', 'vs', 'tests')
	uuid_namespace = UUID('{51C7001B-048C-4AF0-B598-D75E78FF31F0}')
	configuration_name = lambda x: 'Debug' if x.lower() == 'debug' else 'Release'
	platform_name = lambda x: 'Win32' if x == 'x86' else x
	for mode in modes:
		file_path = path.join(test_root, 'test_{}.vcxproj'.format('_'.join(mode)))
		print("Writing to {}".format(file_path))
		with open(file_path, 'w', encoding='utf-8-sig', newline='\r\n') as file:
			write = lambda txt: print(txt, file=file)
			write(r'''
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
		<ProjectGuid>{{{uuid}}}</ProjectGuid>
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
            <MultiProcessorCompilation>true</MultiProcessorCompilation>
		</ClCompile>
	</ItemDefinitionGroup>
	<PropertyGroup>
		<LocalDebuggerWorkingDirectory>..\..\tests\</LocalDebuggerWorkingDirectory>
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
		<ClCompile Include="..\..\tests\strings.cpp" />
		<ClCompile Include="..\..\tests\tagged_ptr.cpp" />
		<ClCompile Include="..\..\tests\tests.cpp">
			<PrecompiledHeader>Create</PrecompiledHeader>
		</ClCompile>
		<ClCompile Include="..\..\tests\unicode_char.cpp" />
		<ClCompile Include="..\..\tests\unicode_char16_t.cpp" />
		<ClCompile Include="..\..\tests\unicode_char32_t.cpp" />
		<ClCompile Include="..\..\tests\unicode_char8_t.cpp" />
		<ClCompile Include="..\..\tests\unicode_unsigned_char.cpp" />
		<ClCompile Include="..\..\tests\unicode_wchar_t.cpp" />
		<ClCompile Include="..\..\tests\uuid.cpp" />
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
	</ItemGroup>
	<ItemGroup>
		<None Include="..\..\tests\meson.build" />
		<None Include="..\.runsettings" />
		<None Include="..\muu.props" />
	</ItemGroup>
	<Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
</Project>
			'''.strip().format(
				configuration=next(configuration_name(x) for x in mode if x in ('debug', 'release')),
				platform=next(platform_name(x) for x in mode if x in ('x64', 'x86')),
				uuid=str(uuid5(uuid_namespace, '_'.join(mode))).upper(),
				exceptions='false' if 'noexcept' in mode else 'Sync',
				standard='cpplatest' if 'cpplatest' in mode else 'cpp17'
			))

if __name__ == '__main__':
	utils.run(main)
