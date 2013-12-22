#pragma once


#include "c++10_def.h"

#if _CPPLIB_VER >= 520 // stdlib>=vs2010
#include <type_traits>
#elif HAS_BOOST
#include <boost/tr1/type_traits.hpp>
namespace std
{
	using ::boost::conditional;
};
#else
#error "not c++ tr1 lib"
#endif
