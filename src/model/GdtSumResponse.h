#ifndef __GDT_CACHE_MODEL_GDT_SUM_RESPONSE_H
#define __GDT_CACHE_MODEL_GDT_SUM_RESPONSE_H

#include <string>

namespace model
{
    struct GdtSumResponse
    {
        std::string state;
        float sum;
        int count;
        float time;
    };
} // namespace model


#endif //__GDT_CACHE_MODEL_GDT_SUM_RESPONSE_H