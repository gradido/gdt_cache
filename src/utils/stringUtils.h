#ifndef __GRADIDO_GDT_CACHE_UTILS_STRING_UTILS_H
#define __GRADIDO_GDT_CACHE_UTILS_STRING_UTILS_H

#include <string>

// remove whitespaces, \r\t\n from begin and end of string, just like php trim function
// if at least one character to remove was found, return new string, else return original string
std::string trim(const std::string& str);
// operate directly on original string, return original string
std::string toLowercase(std::string& str);

#endif //__GRADIDO_GDT_CACHE_UTILS_STRING_UTILS_H