#ifndef __GDT_CACHE_MODEL_GDT_ENTRY_H
#define __GDT_CACHE_MODEL_GDT_ENTRY_H

#include <string>

namespace model 
{
    struct GdtEntry {
        int amount;
        int amount2;
        int gdt;
        int factor;
        int factor2;
        std::string comment;
        std::string coupon_code;
        std::string source;
        std::string project;
        int id;
        std::string date;
        std::string email;
        int gdt_entry_type_id;
    };
}

#endif //__GDT_CACHE_MODEL_GDT_ENTRY_H