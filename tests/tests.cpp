#include "tests.h"
#include "../include/muu/float16.h"

namespace muu
{
	std::ostream& operator << (std::ostream& os, const float16& value)
	{
		return os << static_cast<float>(value);
	}
}


#if TESTS_MANUAL_INSTANTIATIONS

template class std::unique_ptr<const Catch::IExceptionTranslator>;
namespace Catch
{
	//template struct StringMaker<node_view<node>>;
	//template struct StringMaker<node_view<const node>>;
	//template ReusableStringStream& ReusableStringStream::operator << (toml::node_view<toml::node> const&);
	//template ReusableStringStream& ReusableStringStream::operator << (toml::node_view<const toml::node> const&);
	//namespace Detail
	//{
	//	template std::string stringify(const node_view<node>&);
	//	template std::string stringify(const node_view<const node>&);
	//}
}

#endif // TESTS_MANUAL_INSTANTIATIONS
