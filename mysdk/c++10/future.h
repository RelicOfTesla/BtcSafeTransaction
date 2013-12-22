#pragma once

#include "c++10_def.h"

#if _CPPLIB_VER >= 540 // stdlib>=vs2012
#include <future>
#elif HAS_BOOST
#include <boost/thread/future.hpp>
namespace std
{
	using boost::shared_future;
	using boost::unique_future;
	using boost::packaged_task;
	using boost::promise;

};
#else
#error "not c++ 10 lib"
#endif