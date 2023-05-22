#ifndef __GDT_CACHE_VIEW_VIEW_H
#define __GDT_CACHE_VIEW_VIEW_H

#include "rapidjson/document.h"
#include "../model/GdtEntry.h"
#include "../model/GdtEntryList.h"

namespace view {
    enum class OrderDirections 
    {
        ASC,
        DESC
    };

    OrderDirections orderDirectionFromString(const std::string& orderDirection);
    const char* orderDirectionsToString(OrderDirections dir);

    template<class T> rapidjson::Value toJson(const T& data, rapidjson::Document::AllocatorType& alloc);
    template<> rapidjson::Value toJson<model::GdtEntry>(const model::GdtEntry& data, rapidjson::Document::AllocatorType& alloc);
    template<> rapidjson::Value toJson<model::GdtEntryList>(const model::GdtEntryList& data, rapidjson::Document::AllocatorType& alloc);
}

#endif //__GDT_CACHE_VIEW_VIEW_H