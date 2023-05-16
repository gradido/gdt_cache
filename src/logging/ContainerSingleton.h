#ifndef __GDT_CACHE_LOG_CONTAINER_SINGLETON_H
#define __GDT_CACHE_LOG_CONTAINER_SINGLETON_H

#include "Entry.h"
#include <mutex>
#include <list>

namespace logging {
    class ContainerSingleton
    {
    public:
        ~ContainerSingleton();
        static ContainerSingleton* getInstance();
        void addLog(const Entry& log) noexcept;

        std::string getErrorsHtml() noexcept;
        bool hasErrors() noexcept;

    protected:
        ContainerSingleton();
        std::list<Entry> mLogEntries;
        std::recursive_mutex mWorkMutex;
        int mRemovedErrors;
    };
}

#endif //__GDT_CACHE_LOG_CONTAINER_SINGLETON_H