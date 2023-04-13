#ifndef __GDT_CACHE_GDT_ENTRIES_CACHE_H
#define __GDT_CACHE_GDT_ENTRIES_CACHE_H

#include <string>

class GdtEntriesCache
{
public:
    enum class OrderDirections 
    {
        ASC,
        DESC
    };

    ~GdtEntriesCache();
    static GdtEntriesCache* getInstance();

    // fill up cache, load every gdt entry request from gdt server, could need some time
    bool initialize();

    std::string listPerEmailApi(
        const std::string& email, 
        int page = 1, 
        int count = 25, 
        OrderDirections order = OrderDirections::ASC
    );

    std::string sumPerEmailApi(const std::string& email);

    static OrderDirections orderDirectionFromString(const std::string& orderDirection);

protected:
    GdtEntriesCache();

    // store complete answer string in memory
    // 
};

#endif //__GDT_CACHE_GDT_ENTRIES_CACHE_H