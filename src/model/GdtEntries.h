#ifndef __GDT_CACHE_MODEL_GDT_ENTRIES_H
#define __GDT_CACHE_MODEL_GDT_ENTRIES_H

#include "GdtEntry.h"
#include <vector>

namespace model {
    struct GdtEntries
    {
        std::string state;
        int count;
        std::vector<GdtEntry> gdt_entries;
        double gdt_sum;
        double time_used;
    };
}

#endif //__GDT_CACHE_MODEL_GDT_ENTRIES_H