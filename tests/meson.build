test_sources = [
	'main.cpp',
	'tests.cpp',
	'common.cpp',
	'blob.cpp',
	'aligned_alloc.cpp',
	'tagged_ptr.cpp',
	'scope_guard.cpp',
]

tests = executable(
	'tests',
	test_sources,
	include_directories : inc
)
test('tests', tests)
