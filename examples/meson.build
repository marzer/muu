# This file is a part of muu and is subject to the the terms of the MIT license.
# Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

example_args = []
example_args += internal_args
if is_windows
	example_args += has_exceptions ? '-D_HAS_EXCEPTIONS=1' : '-D_HAS_EXCEPTIONS=0'
endif

example_dependencies = [ muu_dep ]
example_dependencies += internal_dependencies

example_overrides = []
example_overrides += internal_overrides

executable(
	'example',
	[ 'example.cpp' ],
	cpp_args: example_args,
	dependencies: example_dependencies,
	override_options: example_overrides,
	install: false
)
