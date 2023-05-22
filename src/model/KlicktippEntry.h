#ifndef __GDT_CACHE_MODEL_KLICKTIPP_ENTRY_H
#define __GDT_CACHE_MODEL_KLICKTIPP_ENTRY_H

#include <string>

namespace model {
    struct KlicktippEntry
    {
        KlicktippEntry(const std::string& _email, double _euroSum, double _gdtSum)
        : email(_email), euroSum(_euroSum), gdtSum(_gdtSum) {}

        std::string     email;
        double          euroSum;
        double          gdtSum;
    };
}

#endif //__GDT_CACHE_MODEL_KLICKTIPP_ENTRY_H