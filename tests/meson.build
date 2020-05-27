test_sources = [
	'main.cpp',
	'tests.cpp',
	'common.cpp',
	'blob.cpp',
	'aligned_alloc.cpp',
	'tagged_ptr.cpp',
	'float16.cpp',
	'scope_guard.cpp',
	'accumulator.cpp',
	'uuid.cpp',
]

tests = executable(
	'tests',
	test_sources,
	include_directories : inc
)
test('tests', tests)
