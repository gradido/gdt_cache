#include "GlobalModificator.h"

namespace model {
    GlobalModificator::GlobalModificator(int id, const std::string& name, float factor, std::time_t startDate, std::time_t endDate)
    : mId(id), mName(name), mFactor(factor), mStartDate(startDate), mEndDate(endDate)
    {

    }

    bool GlobalModificator::operator==(const GlobalModificator& b) const
    {
        return 
            mFactor == b.mFactor &&
            mStartDate == b.mStartDate &&
            mEndDate == b.mEndDate &&
            mId == b.mId;
    }
}