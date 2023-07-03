#ifndef __GDT_CACHE_VIEW_VIEW_H
#define __GDT_CACHE_VIEW_VIEW_H

#include "../model/GdtEntry.h"
#include "../model/GdtEntryList.h"
#include <ios>

namespace view {
    enum class OrderDirections 
    {
        ASC,
        DESC
    };

    OrderDirections orderDirectionFromString(const std::string& orderDirection);
    const char* orderDirectionsToString(OrderDirections dir);    

    void configNumberFormat(std::ios& stream);

    template<class T> std::string toJsonString(const T& data);
    template<> std::string toJsonString<model::GdtEntry>(const model::GdtEntry& data);
    template<> std::string toJsonString<model::GdtEntryList>(const model::GdtEntryList& data);
}

#endif //__GDT_CACHE_VIEW_VIEW_H