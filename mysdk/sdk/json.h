#pragma once

#include <json/json.h>
#include <fstream>
#include "shared_ptr.h"

static shared_ptr<Json::Value> pjson_from_string(const std::string& s, std::string* error_str = nullptr)
{
	shared_ptr<Json::Value> pVal(new Json::Value);
	Json::Reader rd;
	if (s.size())
	{
		rd.parse(s, *pVal);
		if (error_str)
		{
			*error_str = rd.getFormatedErrorMessages();
		}
#if _DEBUG
		if (pVal->empty())
		{
			std::string s = rd.getFormatedErrorMessages();
			printf("%s\n", s.c_str());
		}
#endif
	}
	if (pVal->empty())
	{
		rd.parse("{}", *pVal);
	}
	return pVal;
}
static shared_ptr<Json::Value> pjson_from_file(const char* filepath, std::string* error_str = nullptr)
{
	shared_ptr<Json::Value> pVal(new Json::Value);
	Json::Reader rd;
	std::ifstream file(filepath);
	if (!file.eof())
	{
		rd.parse(file, *pVal);

		if (error_str)
		{
			*error_str = rd.getFormatedErrorMessages();
		}
#if _DEBUG
		if (pVal->empty())
		{
			std::string s = rd.getFormatedErrorMessages();
			printf("%s\n", s.c_str());
		}
#endif
	}
	return pVal;
}

static void json_to_file(const Json::Value& v, const char* filepath, std::string* error_str = nullptr)
{
	Json::StyledWriter writer;
	std::string data = writer.write(v);
	std::ofstream file(filepath);
	file << data;
}


inline Json::Value json_from_string(const std::string& s, std::string* error_str = nullptr)
{
	return *pjson_from_string(s, error_str);
}
inline Json::Value json_from_file(const char* filepath, std::string* error_str = nullptr)
{
	return *pjson_from_file(filepath, error_str);
}
