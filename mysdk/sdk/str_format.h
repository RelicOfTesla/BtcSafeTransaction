#pragma once

#include <string>

std::string str_vformat(const char* format, va_list ap);
std::string str_format(const char* format, ...);