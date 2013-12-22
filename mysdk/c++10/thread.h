#pragma once


#include "c++10_def.h"


#if _CPPLIB_VER >= 540 // stdlib>=vs2012
#include <thread>
#elif HAS_BOOST
#include <boost/thread/thread.hpp>
namespace std
{
	using boost::thread;
};
#else
#error "not c++ 10 lib"
#endif