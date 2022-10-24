#include "muu/preprocessor.h"
#include "muu/launder.h"
#include "muu/impl/std_memcpy.h"
#include "muu/impl/std_new.h"

MUU_PUSH_WARNINGS;
MUU_DISABLE_SPAM_WARNINGS;

#if MUU_MSVC_LIKE
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
