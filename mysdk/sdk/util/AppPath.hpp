#pragma once

#include <string>
#include <Shlwapi.h>

static inline std::string GetAppPath()
{
	std::string r;
	r.resize(MAX_PATH);
	size_t len = GetModuleFileNameA(0, &r[0], MAX_PATH);
	r.resize(len);
	return r;
}

static inline std::string GetDirectory(const std::string& filepath)
{
	size_t pos = filepath.rfind('\\');
	if (pos != std::string::npos)
	{
		return filepath.substr(0, pos+1);
	}
	return filepath;
}

static inline std::string GetAppDir()
{
	return GetDirectory( GetAppPath() );
}

static inline std::string GetAppFile(const char* file)
{
	std::string r = GetAppDir() + file;
	return r;
}

static 
std::string GetFullFilePath(const std::string& filepath)
{
	UINT len;
	
	std::string expand_path;
	std::string result;

	expand_path.resize(MAX_PATH);
	result.resize(MAX_PATH);

	len = ExpandEnvironmentStringsA(filepath.c_str(), &expand_path[0], MAX_PATH);

	PathCanonicalizeA(&result[0], expand_path.c_str());

	for (std::string::iterator it = result.begin(); it != result.end(); ++it)
	{
		if (*it == '/')
		{
			*it = '\\';
		}
	}
	expand_path = result;

	result.resize(MAX_PATH);
	len = SearchPathA(expand_path.c_str(), nullptr, nullptr, MAX_PATH, &result[0], nullptr);
	result.resize(strlen(result.c_str()));
	if (result.empty())
	{
		result = expand_path.c_str();
		if (result.empty())
		{
			result = filepath;
		}
	}
	return result;
}

#define GetExecutePath GetAppPath
#define GetExecuteDir GetAppDir