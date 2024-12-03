#include "ContainerSingleton.h"
#include <iostream>
#include <sstream>

namespace logging {

    ContainerSingleton::ContainerSingleton()
    : mRemovedErrors(0)
    {

    }

    ContainerSingleton::~ContainerSingleton()
    {

    }

    ContainerSingleton* ContainerSingleton::getInstance()
    {
        static ContainerSingleton one;
        return &one;
    }

    void ContainerSingleton::addLog(const Entry& log) noexcept
    {
        try {
            std::lock_guard _lock(mWorkMutex);
            mLogEntries.push_back(log);
            while(mLogEntries.size() > 100) {
                mLogEntries.pop_front();
                mRemovedErrors++;
            }
        } catch(std::bad_alloc& ex) {
            std::clog << "[" << __PRETTY_FUNCTION__ << "] no memory left for adding log to log::ContainerSingleton: " << std::endl;
        } catch(std::system_error& ex) {
            std::clog << "[" << __PRETTY_FUNCTION__ << "] unable to lock work mutex, cannot add log to log::ContainerSingleton: " << std::endl;
        } catch(...) {
            std::clog << "[" << __PRETTY_FUNCTION__ << "] unknown exception occurred, cannot add log to log::ContainerSingleton: " << std::endl;
        }
    }

    std::string ContainerSingleton::getErrorsHtml() noexcept
    {
        try {
            std::lock_guard _lock(mWorkMutex);
            // lithium hardcoded buffer size - 100 for html page
            int maxHtmlSize = 50 * 1024 - 100;
            std::string resultHtml = "<ul class='success'>";
            resultHtml += "<li>" + mDBUpdateLastTimes.load() + "</li>";
            resultHtml += "</ul><ul class='errors'>";
            for(auto log: mLogEntries){
                std::stringstream logMessageHtml;
                logMessageHtml << "<li>";
                if(log.type == Type::ERROR) {
                    logMessageHtml << "<span style='color:red;'>";
                } else if(log.type == Type::INFORMATION) {
                    logMessageHtml << "<span style='color:grey;'>";
                } else {
                    logMessageHtml << "<span>";
                }
                logMessageHtml << typeToString(log.type) << "</span>&nbsp;"
                    << "[" << log.function << "] " << log.message 
                    << " (file: " << log.file << ", line: " << std::to_string(log.line) << ")</li>";
                
                if(resultHtml.size() + logMessageHtml.str().size() > maxHtmlSize) {
                    resultHtml += "<li>...</li>";
                    break;
                }
                resultHtml += logMessageHtml.str();
            }
            resultHtml +="</ul>";
            if(mRemovedErrors) {
                resultHtml += std::to_string(mRemovedErrors) + " additional errors already removed from memory";
            }
            return resultHtml;
        } catch(std::exception& ex) {
            std::clog << "[" << __PRETTY_FUNCTION__ << "] unable to lock work mutex, cannot render error messages, " 
                << ex.what() << std::endl;
            return "Cannot lock work mutex!";
        } catch(...) {
            std::clog << "[" << __PRETTY_FUNCTION__ << "] unknown exception occurred, cannot render error messages!"
                << std::endl;
            return "unknown exception!";
        }
    }

    bool ContainerSingleton::hasErrors() noexcept
    {
        try {
            std::lock_guard _lock(mWorkMutex);
            return mLogEntries.size() > 0;
        } catch(std::system_error& ex) {
            std::clog << "[" << __PRETTY_FUNCTION__ << "] unable to lock work mutex, " 
                << ex.what() << std::endl;
        } catch(...) {
            std::clog << "[" << __PRETTY_FUNCTION__ << "] unknown exception occurred"
                << std::endl;
        }   
        return true;
    }

}