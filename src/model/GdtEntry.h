#ifndef __GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_H
#define __GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_H

#include <ctime>
#include <string>

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
            CVS_STAFF_WAGE = 8, // not known by gradido
            STAFF_WAGE_GLOBAL_MODIFICATOR = 9, // not known by gradido, shouldn't be also in db
            MAX
        };

        static const char* getGdtEntryTypeString(GdtEntryType type);

        GdtEntry(int id, long long amount, std::time_t date, 
                const std::string& email, const std::string& comment, 
                const std::string& source, const std::string& project, const std::string& coupon_code,
                int customer_id,
                int gdt_entry_type_id, double factor, long long amount2, double factor2, double gdt);
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
        inline int getCustomerId() const { return mCustomerId; }
        inline GdtEntryType getGdtEntryType() const {return mGdtEntryType;}
        int getGdtEntryTypeId() const;
        inline double getFactor() const {return mFactor;}
        inline long long getAmount2() const {return mAmount2;}
        inline double getAmount2Decimal() const {return static_cast<double>(mAmount2) / 100.0;}
        inline double getFactor2() const {return mFactor2;}
        inline double getGdt() const {return mGDT;}

        inline long long calculateGdt() const {
            return 
                static_cast<double>(mAmount) * mFactor * mFactor2
                + static_cast<double>(mAmount2);
        }                
        
    protected:
        std::string getFullComment(const std::string& comment, const std::string& source, const std::string& project) const;

        int					mId;
        long long			mAmount; // normal euro sum
        std::string			mDateString;
        std::time_t         mDate;
        std::string         mEmail;
        std::string		    mComment;
        std::string         mCouponCode;
        int                 mCustomerId;
        GdtEntryType		mGdtEntryType;
        double				mFactor;
        long long           mAmount2;
        double              mFactor2;
        double				mGDT; // resulting gdt

    };
}

#endif //__GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_H