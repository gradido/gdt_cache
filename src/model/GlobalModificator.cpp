#include "GlobalModificator.h"

namespace model {
    GlobalModificator::GlobalModificator(int id, const std::string& name, std::time_t startDate, std::time_t endDate)
    : mId(id), mName(name), mStartDate(startDate), mEndDate(endDate)
    {

    }
}