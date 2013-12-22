#pragma once

#include <vector>
#include "shared_ptr.h"

typedef std::vector<BYTE> binary;

typedef shared_ptr<binary> binptr;

inline void binary_append( binary& bin, const void* data, size_t len )
{
    const BYTE* pd = ( const BYTE* )data;
    bin.insert( bin.end(), pd, &pd[len] );
}

inline void binptr_append( binptr& pbin, const void* data, size_t len )
{
	const BYTE* pd = ( const BYTE* )data;
	pbin->insert( pbin->end(), pd, &pd[len] );
}