#pragma once

#include "c++10_def.h"

#if _CPPLIB_VER >= 540 // stdlib>=vs2012
#include <condition_variable>
#elif HAS_BOOST
#include <boost/thread/condition_variable.hpp>
namespace std
{	
	using boost::condition_variable_any;
	using boost::condition_variable;
};
#else
#error "not c++ 10 lib"
#endif
