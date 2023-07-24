#include "GdtEntry.h"
#include <math.h>
#include <iomanip>
#include <sstream>

using namespace std;

namespace model {
	
	const char* GdtEntry::getGdtEntryTypeString(GdtEntryType type)
	{
		switch (type)
		{
		case GdtEntryType::FORM: return "FORM";
		case GdtEntryType::CVS: return "CVS";
		case GdtEntryType::ELOPAGE: return "ELOPAGE";
		case GdtEntryType::ELOPAGE_PUBLISHER: return "ELOPAGE_PUBLISHER";
		case GdtEntryType::DIGISTORE: return "DIGISTORE";
		case GdtEntryType::CVS2: return "CVS2";
		case GdtEntryType::GLOBAL_MODIFICATOR: return "GLOBAL_MODIFICATOR";
		case GdtEntryType::CVS_STAFF_WAGE: return "STAFF_WAGES";
		case GdtEntryType::STAFF_WAGE_GLOBAL_MODIFICATOR: return "STAFF_WAGES_GLOBAL_MODIFICATOR";
		default: 
		  std::string message = "unknown enum value for model::GdtEntryType: ";
		  message += std::to_string(static_cast<int>(type));
		  throw std::runtime_error(message);
		}
	}

	GdtEntry::GdtEntry(int id, long long amount, std::time_t date, 
                const std::string& email, const std::string& comment, 
                const std::string& source, const std::string& project, const std::string& coupon_code,
				int customer_id,
                int gdt_entry_type_id, double factor, long long amount2, double factor2, double gdt)
	: mId(id), mAmount(amount), mDate(date), mEmail(email),
	   mComment(getFullComment(comment, source, project)), mCouponCode(coupon_code), 
	   mCustomerId(customer_id),
	   mGdtEntryType((GdtEntryType)gdt_entry_type_id), mFactor(factor), mAmount2(amount2), 
	   mFactor2(factor2), mGDT(gdt / 100.0)
	{
		std::tm tm = *std::localtime(&mDate);
		std::stringstream ss;
		//2022-09-14T13:09:24+00:00
		ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S+00:00");
		mDateString = ss.str();
	}

	GdtEntry::~GdtEntry()
	{

	}

	bool GdtEntry::operator== (const GdtEntry& b) const
	{
		return 
			mId == b.mId && 
			mAmount == b.mAmount &&
			mDateString == b.mDateString && 
			mEmail == b.mEmail &&
			mComment == b.mComment &&
			mCouponCode == b.mCouponCode &&
			mGdtEntryType == b.mGdtEntryType &&
			mFactor == b.mFactor &&
			mAmount2 == b.mAmount2 &&
			mFactor2 == b.mFactor2 &&
			mGDT == b.mGDT
			;
	}

	long long GdtEntry::getEuroAmount() const
	{
		switch(mGdtEntryType) {
			case GdtEntryType::FORM:
			case GdtEntryType::CVS:
			case GdtEntryType::ELOPAGE:
			case GdtEntryType::DIGISTORE:
			case GdtEntryType::CVS2:
			case GdtEntryType::CVS_STAFF_WAGE:
			return mAmount;
		}
		return 0;
	}
	int GdtEntry::getGdtEntryTypeId() const
	{
		// gradido doesn't know this enum type so we need to give it another
		if(mGdtEntryType == GdtEntryType::CVS_STAFF_WAGE) {
			return 2;
		}
		return static_cast<int>(mGdtEntryType);
	}

	std::string GdtEntry::getFullComment(const std::string& comment, const std::string& source, const std::string& project) const
	{
		// &#8210; = langer Gedankenstrich
		std::string fullComment = comment;
        if(source.size()) {
			if(fullComment.size()) fullComment += " – ";
			fullComment += "Quelle: " + source;
		}
		if(project.size()) {
			if(fullComment.size()) fullComment += " – ";
			fullComment += "Projekt: " + project;
		}
        return fullComment;
	}
}