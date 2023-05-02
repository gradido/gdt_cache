#ifndef __GDT_CACHE_CONTROLLER_UPDATE_MANAGER_H
#define __GDT_CACHE_CONTROLLER_UPDATE_MANAGER_H

#include <mutex>
#include <shared_mutex>
#include <vector>
#include <queue>
#include <thread>
#include <condition_variable>

#include "../task/UpdateGdtEntryList.h"

#ifdef _WIN32 
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else // Für Linux
    #include <netdb.h>
    #include <arpa/inet.h>
#endif

namespace controller {
    enum class UpdateStatus:uint8_t {
        OK = 0,
        ERROR,
        RUNNING
    };
    class UpdateManager
    {
    public:
        ~UpdateManager();
        static UpdateManager* getInstance();

        UpdateStatus updateAllowedIps();

        inline size_t getAllowedIpsCount() {std::shared_lock _lock(mAllowedIpsMutex); return mAllowedIps.capacity();}
        std::string getAllowedIp(int i) {
            std::shared_lock _lock(mAllowedIpsMutex); 
            if(i >= mAllowedIps.size()) return "error"; else return mAllowedIps[i];
        }

        void pushTask(std::shared_ptr<task::UpdateGdtEntryList> task, bool priority = false);

        // will be started on construction and run the whole live time
        // not allowed to crash!!!
        void updateThread();

    protected:
        UpdateManager();
        bool updateAllowedIp(const std::string& url);

        // thread stuff
        std::thread mMainThread;
        std::condition_variable mMainThreadCondition;
        std::mutex mMainThreadMutex;
        bool mExitSignal;

        // tasks
        std::queue<std::shared_ptr<task::UpdateGdtEntryList>> mTasks;
        std::queue<std::shared_ptr<task::UpdateGdtEntryList>> mPriorityTasks;
        std::mutex mTasksMutex; 

        //! ip from allowed hosts from config
        std::vector<std::string> mAllowedIps;
        std::shared_mutex mAllowedIpsMutex;
        std::mutex mUpdateAllowedIpsMutex;
        std::time_t mLastUpdateAllowedIps;

        // buffer 
        char mbIp[INET_ADDRSTRLEN];
        char mbIpv6[INET6_ADDRSTRLEN];
    };
}

#endif //__GDT_CACHE_CONTROLLER_UPDATE_MANAGER_H