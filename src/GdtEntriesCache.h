#ifndef __GDT_CACHE_GDT_ENTRIES_CACHE_H
#define __GDT_CACHE_GDT_ENTRIES_CACHE_H

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include "model/GdtEntryList.h"
#include "model/Customer.h"

class GdtEntriesCache
{
public:
    ~GdtEntriesCache();
    static GdtEntriesCache* getInstance();

    // fill up cache, load every gdt entry request from gdt server, could need some time
    bool initializeFromPhp();
    // fill up cache, directly access gdt server db and request gdt entries only if global modificators are not up to date
    void initializeFromDb();

    //! return json string from cache and schedule update for this email
    std::string listPerEmailApi(
        const std::string& email, 
        int page = 1, 
        int count = 25, 
        model::GdtEntryList::OrderDirections order = model::GdtEntryList::OrderDirections::ASC
    );

    std::string sumPerEmailApi(const std::string& email);    

    // go through gdt entries and create update task for every gdt entry list older than 4h
    void reloadCacheAfterTimeout(li::mysql_connection<li::mysql_functions_blocking> connection, bool ignoreTimeout = false);

    bool canUpdateGdtEntryList(const std::string& email) const noexcept;
    bool shouldUpdateGdtEntryList(const std::string& email) const noexcept;
    void swapGdtEntryList(
        std::shared_ptr<model::Customer> customer, 
        std::shared_ptr<model::GdtEntryList> updatedGdtEntryList
    ) noexcept;

protected:
    GdtEntriesCache();
    // reload contacts and gdt entries from db complete
    void loadFromDb(li::mysql_connection<li::mysql_functions_blocking> connection);

    // api calls
    rapidjson::Document _listPerEmailApi(
        const std::string& email, 
        int page = 1, 
        int count = 25, 
        model::GdtEntryList::OrderDirections order = model::GdtEntryList::OrderDirections::ASC
    );
    //! email GdtEntryList used shared ptr because it could be pointed multiple times on by different emails
    std::unordered_map<std::string, std::shared_ptr<model::GdtEntryList>> mGdtEntriesByEmails; 
    mutable std::recursive_mutex mGdtEntriesByEmailMutex;
    std::time_t     mLastCacheReload;

};

#endif //__GDT_CACHE_GDT_ENTRIES_CACHE_H