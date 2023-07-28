#include "stringUtils.h"
#include <algorithm>

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    if(first == std::string::npos && last == std::string::npos) {
        return str;
    }
    if (first == std::string::npos) {
        first = 0;
    } 
    // add 1 because last point on last character which isn't a whitespace character,
    // but we like to take this character also
    return str.substr(first, last - first + 1);
}

// Function to convert the string to lowercase
std::string toLowercase(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}