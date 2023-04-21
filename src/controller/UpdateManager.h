#ifndef __GDT_CACHE_CONTROLLER_UPDATE_MANAGER_H
#define __GDT_CACHE_CONTROLLER_UPDATE_MANAGER_H

#include <mutex>
#include <shared_mutex>
#include <vector>

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

    protected:
        UpdateManager();
        bool updateAllowedIp(const std::string& url);

        //! ip from allowed hosts from config
        std::vector<std::string> mAllowedIps;
        std::shared_mutex mAllowedIpsMutex;
        std::mutex mUpdateAllowedIpsMutex;

        // buffer 
        char mbIp[INET_ADDRSTRLEN];
    };
}

#endif //__GDT_CACHE_CONTROLLER_UPDATE_MANAGER_H