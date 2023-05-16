#include "Entry.h"

namespace logging {
    const char* typeToString(Type type) {
        switch(type){
            case Type::INFORMATION: return "info";
            case Type::ERROR: return "error";
            case Type::DEBUGGING: return "debugging";
            default: return "unknown";
        }
    }

    std::ostream& getOutputStreamForType(Type type) {
        switch(type) {
            case Type::INFORMATION: return std::cout;
            case Type::ERROR: return std::clog;
            case Type::DEBUGGING: return std::cout;
            default: return std::cout;
        }
    }
}