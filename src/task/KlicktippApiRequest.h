#ifndef __GDT_CACHE_TASK_KLICKTIPP_API_REQUEST_H
#define __GDT_CACHE_TASK_KLICKTIPP_API_REQUEST_H

#include "ExternRequest.h"

namespace task
{
    class KlicktippApiRequest: public ExternRequest
    {
    public:
        void run();
    protected:
    };
} // namespace task


#endif //__GDT_CACHE_TASK_KLICKTIPP_API_REQUEST_H