#ifndef __GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_H
#define __GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_H

#include <ctime>
#include <tuple>
#include <rapidjson/document.h>

namespace model {
	
    class GdtEntry
    {
    public:
        enum class GdtEntryType
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

        typedef std::tuple<int, long long, std::time_t, std::string, std::string, std::string, std::string, std::string, int, float, long long, float> Tuple;

        GdtEntry(rapidjson::Value& gdtEntry);
        GdtEntry(Tuple tuple);
        ~GdtEntry();

        const char* getTypename() const { return "GdtEntry"; }

        bool operator== (const GdtEntry& b) const;
        inline bool operator!= (const GdtEntry& b) const {
            return !(*this == b);
        };

        inline int getId() const {return mId;}
        inline long long getAmount() const {return mAmount;}
        //! \return amount value only for specific gdt entry types, else return 0
        long long getEuroAmount() const;
        inline double getAmountDecimal() const {return static_cast<double>(mAmount) / 100.0;}
        inline const std::string& getDateString() const {return mDateString;}
        inline std::time_t getDate() const {return mDate;}
        inline const std::string& getEmail() const { return mEmail;}
        inline const std::string& getComment() const { return mComment;}
        inline const std::string& getCouponCode() const { return mCouponCode;}
        inline GdtEntryType getGdtEntryType() const {return mGdtEntryType;}
        inline int getGdtEntryTypeId() const {return static_cast<int>(mGdtEntryType);}
        inline float getFactor() const {return mFactor;}
        inline long long getAmount2() const {return mAmount2;}
        inline double getAmount2Decimal() const {return static_cast<double>(mAmount2) / 100.0;}
        inline float getFactor2() const {return mFactor2;}
        inline double getGdt() const {return mGDT;}

        inline long long calculateGdt() const {return mAmount * mFactor * mFactor2 + mAmount2;}                
        
    protected:
        std::string getFullComment(Tuple tuple);

        int					mId;
        long long			mAmount; // normal euro sum
        std::string			mDateString;
        std::time_t         mDate;
        std::string         mEmail;
        std::string		    mComment;
        std::string         mCouponCode;
        GdtEntryType		mGdtEntryType;
        float				mFactor;
        long long           mAmount2;
        float               mFactor2;
        double				mGDT; // resulting gdt

    };
}

#endif //__GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_H