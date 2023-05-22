#ifndef __GDT_CACHE_VIEW_GDT_ENTRY_LIST_H
#define __GDT_CACHE_VIEW_GDT_ENTRY_LIST_H

#include "rapidjson/document.h"
#include "View.h"
#include "../model/GdtEntryList.h"

namespace view {

    namespace GdtEntryList {
        rapidjson::Value toJson(const model::GdtEntryList& data, rapidjson::Document::AllocatorType& alloc, Profiler timeUsed);
		rapidjson::Value toJson(
            const model::GdtEntryList& data,
			rapidjson::Document::AllocatorType& alloc, 
			Profiler timeUsed, 
			int page, 
			int count, 
			OrderDirections order
		);
    }
}

#endif //__GDT_CACHE_VIEW_GDT_ENTRY_LIST_H