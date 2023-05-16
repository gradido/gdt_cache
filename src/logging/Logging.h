#ifndef __GDT_CACHE_LOG_LOG_H
#define __GDT_CACHE_LOG_LOG_H

#include <string>

namespace logging {
    //! printed only to console to stdout stream
    void information(const std::string& message, const char* complete_function, const char* function, const char* file, int line);
    //! printed to console to stderr stream and stored for web status request
    void error(const std::string& message, const char* complete_function, const char* function, const char* file, int line);
    //! only in debug printed to console to stdout stream, in release nothing will happen
    void debug(const std::string& message, const char* complete_function, const char* function, const char* file, int line);    
}

#define LOG_ERROR(message) logging::error(message, __PRETTY_FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define LOG_INFORMATION(message) logging::information(message, __PRETTY_FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define LOG_DEBUG(message) logging::debug(message, __PRETTY_FUNCTION__, __FUNCTION__, __FILE__, __LINE__)

#endif //__GDT_CACHE_LOG_LOG_H