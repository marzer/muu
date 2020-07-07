#if !defined(_MSC_VER) || !defined(_M_IX86)
	#define MUU_ALL_INLINE 0
	#define MUU_DEV 1
	#define MUU_IMPLEMENTATION
	#include "../include/muu/all.h"
#else
	#include "../include/muu/preprocessor.h"
#endif

MUU_PUSH_WARNINGS
MUU_DISABLE_ALL_WARNINGS
#define CATCH_CONFIG_RUNNER
#include "catch2.h"
#include <clocale>
MUU_POP_WARNINGS

int main(int argc, char* argv[])
{
	#ifdef _WIN32
		SetConsoleOutputCP(65001);
	#endif
	std::setlocale(LC_ALL, "");
	std::locale::global(std::locale(""));
	return Catch::Session().run(argc, argv);
}
