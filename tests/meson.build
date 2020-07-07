test_sources = [
	'accumulator.cpp',
	'aligned_alloc.cpp',
	'blob.cpp',
	'core.cpp',
	'float16.cpp',
	'hashing.cpp',
	'main.cpp',
	'meta.cpp',
	'scope_guard.cpp',
	'tagged_ptr.cpp',
	'tests.cpp',
	'uuid.cpp',
]

tests = executable(
	'tests',
	test_sources,
	include_directories : inc
)
test('tests', tests)
