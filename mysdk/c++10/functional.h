#pragma once


#if _CPPLIB_VER >= 520 // vs2010
#include <functional>
#elif HAS_BOOST
#include <boost/tr1/functional.hpp>
#else
#error "not c++ tr1 lib"
#endif
