#pragma once


#include "c++10_def.h"


#if _CPPLIB_VER >= 540 // stdlib>=vs2012
#include <mutex>
#elif HAS_BOOST
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
namespace std
{
	using boost::mutex;
	using boost::recursive_mutex;
	using boost::timed_mutex;
	using boost::recursive_timed_mutex;	
	using boost::lock_guard;
	using boost::unique_lock;
};
#else
#error "not c++ 10 lib"
#endif