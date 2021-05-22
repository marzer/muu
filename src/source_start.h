#include "muu/preprocessor.h"

MUU_DISABLE_WARNINGS;
#include <new>
#include <cstring>
MUU_ENABLE_WARNINGS;

MUU_PUSH_WARNINGS;
MUU_DISABLE_SPAM_WARNINGS;

#ifdef _MSC_VER
	#pragma push_macro("min")
	#pragma push_macro("max")
	#undef min
	#undef max
#endif

namespace muu
{
}

MUU_DISABLE_WARNINGS;
using namespace muu;
MUU_ENABLE_WARNINGS;
