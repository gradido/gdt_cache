#ifndef __GDT_CACHE_GDT_ENTRIES_CACHE_H
#define __GDT_CACHE_GDT_ENTRIES_CACHE_H

#include <string>
#include <unordered_map>
#include <memory>
#include "model/GdtEntryList.h"

class GdtEntriesCache
{
public:
    ~GdtEntriesCache();
    static GdtEntriesCache* getInstance();

    // fill up cache, load every gdt entry request from gdt server, could need some time
    bool initialize();

    std::string listPerEmailApi(
        const std::string& email, 
        int page = 1, 
        int count = 25, 
        model::GdtEntryList::OrderDirections order = model::GdtEntryList::OrderDirections::ASC
    );

    std::string sumPerEmailApi(const std::string& email);

    

protected:
    GdtEntriesCache();
    //! email GdtEntryList used shared ptr because it could be pointed multiple times on by different emails
    std::unordered_map<std::string, std::shared_ptr<model::GdtEntryList>> mGdtEntriesByEmails; 

};

#endif //__GDT_CACHE_GDT_ENTRIES_CACHE_H