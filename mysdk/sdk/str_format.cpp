#include "stdafx.h"
#include "str_format.h"
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

std::string str_vformat(const char* format, va_list ap)
{
	std::string s;

	for (int len = 64; ; len *= 2)
	{
		s.resize(len);
		int n = wvnsprintfA(&s[0], len, format, ap);
		if (n >= 0 && n < len-1)
		{
			s.resize(n);
			break;
		}
	}
	return s;
}
std::string str_format(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	return str_vformat(format, ap);
}
