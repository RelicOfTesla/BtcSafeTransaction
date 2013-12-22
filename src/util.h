#pragma once

#include <sdk/json.h>

USHORT Random(USHORT min_,USHORT max_);
std::string MakeRandomString(USHORT len);


std::string JsonOptGet_Str(Json::Value& jv, const char* name, const std::string& def);
UINT JsonOptGet_UINT(Json::Value& jv, const char* name, const UINT& def);

std::string GetWindowStlText(class CWnd*);


std::string amount2str(double v);
