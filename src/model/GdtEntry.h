#ifndef __GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_H
#define __GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_H

#include "Base.h"
#include <ctime>

namespace model {
	
    class GdtEntry : public Base
    {
    public:
        enum class GdtEntryType : uint8_t
        {
            FORM = 1,
            CVS = 2,
            ELOPAGE = 3,
            ELOPAGE_PUBLISHER = 4,
            DIGISTORE = 5,
            CVS2 = 6,
            GLOBAL_MODIFICATOR = 7,
            CVS_STAFF_WAGE = 8,
            STAFF_WAGE_GLOBAL_MODIFICATOR = 9,
            MAX
        };

        static const char* getGdtEntryTypeString(GdtEntryType type);

        typedef std::tuple<int, long long, std::time_t, std::string, std::string, std::string, std::string, std::string, int, float, int, float> Tuple;

        GdtEntry(rapidjson::Value& gdtEntry);
        GdtEntry(Tuple tuple);
        ~GdtEntry();

        rapidjson::Value toJson(rapidjson::Document::AllocatorType& alloc);
        const char* getTypename() { return "GdtEntry"; }

        bool operator== (const GdtEntry& b) const;
        inline bool operator!= (const GdtEntry& b) const {
            return !(*this == b);
        };

        inline int getId() const {return mId;}
        inline double getGdt() const {return mGDT;}
        inline const std::string& getEmail() const { return mEmail;}
        inline std::time_t getDate() const {return mDate;}
        inline GdtEntryType getGdtEntryType() const {return mGdtEntryType;}
        
    protected:
        std::string getFullComment(Tuple tuple);

        int					mId;
        double				mAmount; // normal euro sum
        std::string			mDateString;
        std::time_t         mDate;
        std::string         mEmail;
        std::string		    mComment;
        std::string         mCouponCode;
        GdtEntryType		mGdtEntryType;
        float				mFactor;
        float               mAmount2;
        float               mFactor2;
        double				mGDT; // resulting gdt

    };
}

#endif //__GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_H