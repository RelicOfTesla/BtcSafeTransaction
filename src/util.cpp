#include "stdafx.h"
#include "util.h"

USHORT Random(USHORT min_,USHORT max_)
{
	return ( rand() % (max_ - min_ + 1) + min_);
};


std::string MakeRandomString(USHORT len)
{
	const char* temp_str = "0123456789abcdefghijkllmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const size_t temp_len = strlen(temp_str);
	std::string result;
	for (size_t i = 0; i < len; ++i)
	{
		result += temp_str[Random(0, temp_len-1)];
	}
	return result;
}



std::string JsonOptGet_Str(Json::Value& jv, const char* name, const std::string& def)
{
	try
	{
		Json::Value& cv = jv[name];
		if ( !cv.empty() )
		{
			return cv.asString();
		}
	}
	catch(std::exception&)
	{}
	return def;
}

UINT JsonOptGet_UINT(Json::Value& jv, const char* name, const UINT& def)
{
	try
	{
		Json::Value& cv = jv[name];
		if ( !cv.empty() )
		{
			return cv.asUInt();
		}
	}
	catch(std::exception&)
	{}
	return def;
}


std::string GetWindowStlText(class CWnd* pWnd)
{
	std::string result;
	if (pWnd)
	{
		CString sz;
		pWnd->GetWindowText(sz);
		result = sz;
	}
	return result;
}


std::string amount2str(double v)
{
	char sbuf[256];
	sprintf(sbuf, "%.8f", v);
	return std::string(sbuf);
}
