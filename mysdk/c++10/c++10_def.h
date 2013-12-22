#pragma once

#include "user_config.h"
#include <cstddef>


#if _MSC_VER < 1600
	#define nullptr NULL

	#if HAS_BOOST
		#include <boost/static_assert.hpp>
		#define static_assert	BOOST_STATIC_ASSERT_MSG
	#else
		#error "not support static_assert"
	#endif

#endif