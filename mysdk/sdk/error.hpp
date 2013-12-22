#pragma once

#include <comdef.h>

template<typename R>
static R ComError2Str(const _com_error& e)
{
	R result;
	if (const TCHAR* em = e.ErrorMessage())
	{
		result = em;
	}
	bstr_t desc = e.Description();
	if (!!desc)
	{
		result += desc;
	}
	return result;
}


template<typename R>
static R WinError2Str(UINT ec = GetLastError())
{
	R result;
	HLOCAL hMem = 0;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM, 0, ec, 0, (TCHAR*)&hMem, sizeof(hMem), 0);
	if (hMem)
	{
		result = (const TCHAR*)LocalLock(hMem);
		LocalUnlock(hMem);
		LocalFree(hMem);
	}
	else
	{
		ec = GetLastError();
		assert(ec);
	}
	return result;
}
//////////////////////////////////////////////////////////////////////////
#include <stdexcept>
#include <string>

struct error_code_exception : std::runtime_error 
{
	int m_error_code;

	error_code_exception(const std::string& s, int ec) : std::runtime_error(s), m_error_code(ec)
	{}

	int error_code()const
	{
		return m_error_code;
	}
};

struct com_exception : error_code_exception 
{
	com_exception(const _com_error& e) : error_code_exception(ComError2Str<std::string>(e), e.Error())
	{}
};

struct winerror_exception : error_code_exception 
{
	winerror_exception(UINT ec = GetLastError()) : error_code_exception(WinError2Str<std::string>(ec), ec)
	{}
};
