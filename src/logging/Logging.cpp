#include <ctime>
#include <cstdio>
#include <iostream>

#include "Logging.h"
#include "Entry.h"
#include "ContainerSingleton.h"

namespace logging {
    std::string writeToFile(const std::string& content, const char* fileNamePart, const char* ending = "html")
    {
        std::string fileName = std::to_string(std::time(nullptr));
        fileName += "_";
        fileName += fileNamePart;
        fileName += ".";
        fileName += ending;
        FILE *f = fopen(fileName.data(), "wt");
        fwrite(content.data(), sizeof(char), content.size(), f);
        fclose(f);
        return fileName;
    }

    void printToConsole(const Entry& entry)
    {
        static std::mutex printMutex;
        std::lock_guard _lock(printMutex);

        if(entry.message.size() > 200) {
            auto fileName = writeToFile(entry.message, entry.function.data());
            getOutputStreamForType(entry.type) << typeToString(entry.type) 
                << " message bigger than 200 written to file: " << fileName << std::endl;
        } else {
            getOutputStreamForType(entry.type) 
                << entry.completeFunction << ": " << typeToString(entry.type) 
                << " " << entry.message 
                << " (file: " << entry.file << ", line: " << std::to_string(entry.line) << ")" << std::endl; 
        }
    }

    void information(const std::string& message, const char* complete_function, const char* function, const char* file, int line)
    {
        printToConsole({Type::INFORMATION, message, complete_function, function, file, line});
    }
    
    void error(const std::string& message, const char* complete_function, const char* function, const char* file, int line)
    {
        Entry log(Type::ERROR, message, complete_function, function, file, line);
        printToConsole(log);
        ContainerSingleton::getInstance()->addLog(log);
    }
    
    void debug(const std::string& message, const char* complete_function, const char* function, const char* file, int line)
    {
#ifdef _DEBUG
        printToConsole({Type::DEBUGGING, message, complete_function, function, file, line});
#endif // _DEBUG
    }
}