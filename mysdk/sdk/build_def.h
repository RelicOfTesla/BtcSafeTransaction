#pragma once

#if _DLL
#define _THREAD_TYPE_NAME "MD"
#define _STATIC_TYPE_NAME ""
#else
#define _THREAD_TYPE_NAME "MT"
#define _STATIC_TYPE_NAME "s"
#endif

#if _DEBUG
#define _DEBUG_TYPE_EXT "d"
#else
#define _DEBUG_TYPE_EXT ""
#endif

#define LIB_NAME_M(x)		x _THREAD_TYPE_NAME _DEBUG_TYPE_EXT ".lib"
#define LIB_NAME_S(x)		x _STATIC_TYPE_NAME _DEBUG_TYPE_EXT ".lib"
#define LIB_NAME_D(x)		x _DEBUG_TYPE_EXT ".lib"


#ifndef AUTO_LIB_CRT
#define AUTO_LIB_CRT 0
#endif

#if AUTO_LIB_CRT

#if _DLL
#pragma comment(linker, "/nodefaultlib:libcmt.lib")
#pragma comment(linker, "/nodefaultlib:libcpmt.lib")

#pragma comment(linker, "/nodefaultlib:libcmtd.lib")
#pragma comment(linker, "/nodefaultlib:libcpmtd.lib")
#else
#pragma comment(linker, "/nodefaultlib:msvcrt.lib")
#pragma comment(linker, "/nodefaultlib:msvcrtd.lib")
#endif


#endif
