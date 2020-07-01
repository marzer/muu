#pragma once
#if !defined(_MSC_VER) || !defined(_M_IX86)
	#define MUU_ALL_INLINE 0
#endif
#include "../include/muu/preprocessor.h"

MUU_PUSH_WARNINGS
MUU_DISABLE_ALL_WARNINGS
#include "catch2.h"
#include <sstream>
#include <string_view>
namespace muu
{
	struct float16;
	std::ostream& operator << (std::ostream& os, const float16& value);
}
using namespace Catch::literals;
using namespace muu;
using namespace std::string_view_literals;
MUU_POP_WARNINGS
