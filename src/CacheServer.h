#ifndef __GDT_CACHE_CACHE_SINGLETON_H
#define __GDT_CACHE_CACHE_SINGLETON_H

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include "model/GdtEntryList.h"
#include "model/Customer.h"
#include "background_tasks/UpdateCacheWorker.h"
#include "background_tasks/KlicktippApiRequestsWorker.h"
#include "view/View.h"


#ifdef _WIN32 // Prüfen, ob unter Windows kompiliert wird
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else // Für Linux
    #include <netdb.h>
    #include <arpa/inet.h>
#endif

/*!
 * @author einhornimmond
 * @date 10.05.2023
 * @brief store data over the whole program lifetime and serves the api requests
*/

class CacheServer
{
public:
    enum class UpdateStatus {
        OK = 0,
        ERROR,
        SKIPPED,
        RUNNING
    };

    ~CacheServer();
    static CacheServer* getInstance();

    // fill up cache, load every gdt entry request from gdt server, could need some time (30 min on test system)
    bool initializeFromPhp();
    // fill up cache, directly access gdt server db and request gdt entries only if global modificators are not up to date
    void initializeFromDb();

    //! return json string from cache and schedule update for this email
    std::string listPerEmailApi(
        const std::string& email, 
        int page = 1, 
        int count = 25, 
        view::OrderDirections order = view::OrderDirections::ASC
    );

    std::string sumPerEmailApi(const std::string& email);    

    //! reload gdt entries and customers freshly from db, if last update is older then maxCacheTimeout
    UpdateStatus reloadCacheAfterTimeout(li::mysql_connection<li::mysql_functions_blocking> connection, bool ignoreTimeout = false) noexcept;
    UpdateStatus reloadGdtEntry(li::mysql_connection<li::mysql_functions_blocking> connection, const std::string& email) noexcept;

    //! access allowed ips cache
    inline size_t getAllowedIpsCount() {std::shared_lock _lock(mAllowedIpsMutex); return mAllowedIps.capacity();}
    std::string getAllowedIp(int i) {
        std::shared_lock _lock(mAllowedIpsMutex); 
        if(i >= mAllowedIps.size()) return "error"; else return mAllowedIps[i];
    }

    UpdateStatus updateAllowedIps(bool ignoreTimeout = false) noexcept;

protected:
    CacheServer();
    // reload contacts and gdt entries from db complete
    void loadFromDb(li::mysql_connection<li::mysql_functions_blocking> connection);

    // api calls
    rapidjson::Document _listPerEmailApi(
        const std::string& email, 
        int page = 1, 
        int count = 25, 
        view::OrderDirections order = view::OrderDirections::ASC
    );
    bool updateAllowedIp(const std::string& url);

    //! email GdtEntryList used shared ptr because it could be pointed multiple times on by different emails
    std::unordered_map<std::string, std::shared_ptr<model::GdtEntryList>> mGdtEntriesByEmails; 
    mutable std::recursive_mutex    mGdtEntriesAccessMutex;
    mutable std::mutex              mGdtEntriesUpdateMutex;
    std::time_t                     mLastUpdateGdtEntries;

    //! ip from allowed hosts from config
    std::vector<std::string>        mAllowedIps;
    mutable std::shared_mutex       mAllowedIpsMutex;
    mutable std::mutex              mUpdateAllowedIpsMutex;
    std::time_t                     mLastUpdateAllowedIps;

    // buffer 
    char                            mbIp[INET_ADDRSTRLEN];
    char                            mbIpv6[INET6_ADDRSTRLEN];

    //! Cache Reload Worker
    UpdateCacheWorker               mUpdateCacheWorker;
};

#endif //__GDT_CACHE_CACHE_SINGLETON_H