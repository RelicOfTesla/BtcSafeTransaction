#pragma once

#include <sdk/json.h>

USHORT Random(USHORT min_,USHORT max_);
std::string MakeRandomString(USHORT len);


std::string JsonOptGet_Str(Json::Value& jv, const char* name, const std::string& def);
UINT JsonOptGet_UINT(Json::Value& jv, const char* name, const UINT& def);
double JsonOptGet_double(Json::Value& jv, const char* name, const double& def);

std::string GetWindowStlText(class CWnd*);


std::string amount2str(double v);

inline BOOL IsMulSigAddr(const std::string& addr)
{
	if (addr.size())
	{
		return addr[0] == '3' || addr[0] =='2';
	}
	return FALSE;
}