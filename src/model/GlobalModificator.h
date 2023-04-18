#ifndef __GDT_CACHE_MODEL_GLOBAL_MODIFICATOR_H
#define __GDT_CACHE_MODEL_GLOBAL_MODIFICATOR_H

/*
 `id` int unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(255) DEFAULT '',
  `factor` float NOT NULL,
  `start_date` datetime NOT NULL,
  `end_date` datetime NOT NULL,
  `created` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
*/

#include <ctime>
#include <string>

namespace model {
    class GlobalModificator
    {
    public:
        GlobalModificator(int id, const std::string& name, float factor, std::time_t startDate, std::time_t endDate);
        inline int getId() const { return mId;}
        inline const std::string& getName() const {return mName;}
        inline float getFactor() const { return mFactor;}
        inline std::time_t getStartDate() const {return mStartDate;}
        inline std::time_t getEndDate() const {return mEndDate;}        
    protected:
        int mId;
        std::string mName;
        float       mFactor;
        std::time_t mStartDate;
        std::time_t mEndDate;

    };
}

#endif 