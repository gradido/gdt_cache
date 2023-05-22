#ifndef __GDT_CACHE_TASK_KLICKTIPP_API_REQUEST_H
#define __GDT_CACHE_TASK_KLICKTIPP_API_REQUEST_H

#include "../model/KlicktippEntry.h"
#include <vector>

namespace task
{
    class KlicktippApiRequest
    {
    public:
        KlicktippApiRequest();
        ~KlicktippApiRequest();
        void run();

        void addKlicktippEntry(const model::KlicktippEntry& klicktippEntry);
        inline size_t getCount() const { return mKlicktippEntries.size();}

    protected:
        
        std::vector<model::KlicktippEntry> mKlicktippEntries;
    };
} // namespace task


#endif //__GDT_CACHE_TASK_KLICKTIPP_API_REQUEST_H