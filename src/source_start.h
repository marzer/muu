#include "muu/preprocessor.h"
#include "muu/impl/std_memcpy.h"
#include "muu/impl/std_new.h"
#include "muu/impl/std_launder.h"

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
