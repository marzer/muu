#include "tests.h"

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
