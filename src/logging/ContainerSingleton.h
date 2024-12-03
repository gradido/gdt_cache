#ifndef __GDT_CACHE_LOG_CONTAINER_SINGLETON_H
#define __GDT_CACHE_LOG_CONTAINER_SINGLETON_H

#include "Entry.h"
#include <mutex>
#include <shared_mutex>
#include <list>
#include <atomic>

namespace logging {
    class ContainerSingleton
    {
    public:
        ~ContainerSingleton();
        static ContainerSingleton* getInstance();
        void addLog(const Entry& log) noexcept;

        std::string getErrorsHtml() noexcept;
        bool hasErrors() noexcept;

        inline void setDbUpdateTime(const std::string& str);

    protected:
        ContainerSingleton();
        std::list<Entry> mLogEntries;
        std::recursive_mutex mWorkMutex;
        int mRemovedErrors;
        std::string mDBUpdateLastTimes;
        std::shared_mutex mDBUpdateLastTimeMutex;
    };

    // inline functions
    void ContainerSingleton::setDbUpdateTime(const std::string& str) 
    {
        std::unique_lock _lock(mDBUpdateLastTimeMutex);
        mDBUpdateLastTimes = str;
    }
}

#endif //__GDT_CACHE_LOG_CONTAINER_SINGLETON_H