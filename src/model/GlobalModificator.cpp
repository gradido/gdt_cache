#include "GlobalModificator.h"

namespace model {
    GlobalModificator::GlobalModificator(int id, const std::string& name, float factor, std::time_t startDate, std::time_t endDate)
    : mId(id), mName(name), mFactor(factor), mStartDate(startDate), mEndDate(endDate)
    {

    }
}