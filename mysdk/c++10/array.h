#pragma once

#include "c++10_def.h"

#if _CPPLIB_VER >= 520 // vs2010
#include <array>
#elif HAS_BOOST
#include <boost/tr1/array.hpp>
#else
#error "not c++ tr1 lib"
#endif
