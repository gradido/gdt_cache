#ifndef __GDT_CACHE_VIEW_GDT_ENTRY_LIST_H
#define __GDT_CACHE_VIEW_GDT_ENTRY_LIST_H

#include "View.h"
#include "../model/GdtEntryList.h"

namespace view {

    namespace GdtEntryList {
		std::string toJsonString(const model::GdtEntryList& data, Profiler timeUsed);
		std::string toJsonString(
            const model::GdtEntryList& data,
			Profiler timeUsed, 
			int page, 
			int count, 
			OrderDirections order
		);
    }
}

#endif //__GDT_CACHE_VIEW_GDT_ENTRY_LIST_H