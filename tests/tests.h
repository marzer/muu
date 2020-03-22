#pragma once
#include "catch2.h"
#include <sstream>
#include <string_view>
#if !defined(_MSC_VER) || !defined(_M_IX86)
	#define MUU_ALL_INLINE 0
#endif
#include "../include/muu/common.h"

using namespace muu;
using namespace std::string_view_literals;
using namespace Catch::literals;

// manually instantiate some templates to reduce test compilation time (chosen using ClangBuildAnalyzer)
#define TESTS_MANUAL_INSTANTIATIONS 1
#if TESTS_MANUAL_INSTANTIATIONS

extern template class std::unique_ptr<const Catch::IExceptionTranslator>;
namespace Catch
{
	//extern template struct StringMaker<node_view<node>>;
	//extern template struct StringMaker<node_view<const node>>;
	//extern template ReusableStringStream& ReusableStringStream::operator << (node_view<node> const&);
	//extern template ReusableStringStream& ReusableStringStream::operator << (node_view<const node> const&);
	//namespace Detail
	//{
	//	extern template std::string stringify(const node_view<node>&);
	//	extern template std::string stringify(const node_view<const node>&);
	//}
}

#endif // TESTS_MANUAL_INSTANTIATIONS

