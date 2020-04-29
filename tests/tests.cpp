#include "tests.h"
#include "../include/muu/float16.h"

namespace muu
{
	std::ostream& operator << (std::ostream& os, const float16& value)
	{
		return os << static_cast<float>(value);
	}
}
