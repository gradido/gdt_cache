#ifndef __GDT_CACHE_LOG_ENTRY
#define __GDT_CACHE_LOG_ENTRY

#include <string>
#include <source_location>
#include <iostream>

namespace logging {

    enum class Type 
    {
        INFORMATION,
        ERROR,
        DEBUGGING
    };

    const char* typeToString(Type type);
    std::ostream& getOutputStreamForType(Type type);

    struct Entry
    {
        Entry(Type _type, const std::string& _message, const char* _completeFunction, const char* _function, const char* _file, int _line) noexcept:
        type(_type), message(_message), completeFunction(_completeFunction), function(_function), file(_file), line(_line) {} ;

        Type type;
        std::string message;
        std::string completeFunction;
        std::string function;
        std::string file;
        int         line;
    };
}


#endif //__GDT_CACHE_LOG_ENTRY