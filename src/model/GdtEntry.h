#ifndef __GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_H
#define __GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_H

#include "Base.h"

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
            MAX
        };

        static const char* getGdtEntryTypeString(GdtEntryType type);

        GdtEntry(rapidjson::Value& gdtEntry);
        ~GdtEntry();

        rapidjson::Value toJson(rapidjson::Document::AllocatorType& alloc);
        const char* getTypename() { return "GdtEntry"; }

        bool operator== (const GdtEntry& b);
        inline bool operator!= (const GdtEntry& b) {
            return !(*this == b);
        };

        inline int getId() {return mId;}
        inline float getGdt() {return mGDT;}
        
    protected:
        int					mId;
        float				mAmount; // normal euro sum
        std::string			mDateString;
        std::string         mEmail;
        std::string			mComment;
        std::string         mCouponCode;
        GdtEntryType		mGdtEntryType;
        float				mFactor;
        float               mAmount2;
        float               mFactor2;
        float				mGDT; // resulting gdt

    };
}

#endif //__GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_H