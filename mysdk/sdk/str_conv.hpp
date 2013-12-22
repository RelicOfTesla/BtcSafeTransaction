#pragma once

#include <string>

static
std::wstring ToWString(const void* raw, size_t len, int SrcFormat)
{
	int wlen = MultiByteToWideChar(SrcFormat, 0, (LPCSTR)raw, len, nullptr, 0);
	std::wstring wstr;
	wstr.resize(wlen);
	int ret = MultiByteToWideChar(SrcFormat, 0, (LPCSTR)raw, len, &wstr.front(), wlen);
	return wstr;
}

static
std::string ToAnsi(const wchar_t* wstr, size_t wlen, int DestFormat = CP_ACP)
{
	size_t alen = WideCharToMultiByte(DestFormat, 0, wstr, wlen, nullptr, 0, nullptr, 0);
	std::string astr;
	astr.resize(alen);
	int ret = WideCharToMultiByte(DestFormat, 0, wstr, wlen, &astr.front(), alen, nullptr, 0);
	return astr;
}

static
std::string UTF8_TO_ANSI(const char* val)
{
	size_t rlen = strlen(val);

	std::wstring wstr = ToWString(val, rlen, CP_UTF8);
	std::string astr = ToAnsi(wstr.c_str(), wstr.size());
	return astr;
}

static
std::string ANSI_TO_UTF8(const char* val)
{
	size_t rlen = strlen(val);

	std::wstring wstr = ToWString(val, rlen, CP_ACP);
	std::string astr = ToAnsi(wstr.c_str(), wstr.size());
	return astr;
}

inline std::wstring _A2W(const char* src)
{
	return ToWString(src, -1, CP_ACP);
}

inline std::string _W2A(const wchar_t* src)
{
	return ToAnsi(src, -1);
}

#ifndef A2W
#define A2W _A2W
#endif

#ifndef W2A
#define W2A _W2A
#endif