#if !defined(_MSC_VER) || !defined(_M_IX86)
	#define MUU_ALL_INLINE 0
	#define MUU_IMPLEMENTATION
#endif

// files that have a split implementation:
#include "../include/muu/aligned_alloc.h"
#include "../include/muu/blob.h"

#define CATCH_CONFIG_RUNNER
#include "catch2.h"

int main(int argc, char* argv[])
{
	#ifdef _WIN32
	SetConsoleOutputCP(65001);
	#endif

	return Catch::Session().run(argc, argv);
}
