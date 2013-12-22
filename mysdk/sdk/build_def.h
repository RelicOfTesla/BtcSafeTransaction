#pragma once

#define _MACRO_STR_ADD(a,b)	a#b

#if _DEBUG
#define DEBUG_EXT_STR(x)	_MACRO_STR_ADD(x, "D")
#else
#define DEBUG_EXT_STR(x)	x
#endif

#if _DLL
#define CRT_NAME(x)		DEBUG_EXT_STR(_MACRO_STR_ADD(x, "md"))
#else
#define CRT_NAME(x)		DEBUG_EXT_STR(_MACRO_STR_ADD(x, "mt"))
#endif

#define LIB_NAME_TH(x)		_MACRO_STR_ADD(CRT_NAME(x), ".lib")
#define LIB_NAME(x)			_MACRO_STR_ADD(DEBUG_EXT_STR(x), ".lib")


#if _DLL
#define NODEFAULT_LIB_CRT	_MACRO_STR_ADD("/nodefaultlib:","libcmt")
#else
#define NODEFAULT_LIB_CRT	_MACRO_STR_ADD("/nodefaultlib:","msvcrt")
#endif

#ifndef AUTO_LIB_CRT
#define AUTO_LIB_CRT 0
#endif

#if AUTO_LIB_CRT
#pragma comment(linker, NODEFAULT_LIB_CRT)
#endif
