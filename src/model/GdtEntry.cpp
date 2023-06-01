#include "GdtEntry.h"
#include "../GradidoBlockchainException.h"
#include "../utils/RapidjsonHelper.h"
#include <math.h>
#include <iomanip>
#include <sstream>

using namespace rapidjson;
using namespace rapidjson_helper;
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
		default: throw GradidoUnknownEnumException("unknown enum value in model::graphql::GdtEntry", "GdtEntryType", static_cast<int>(type));
		}
	}

	GdtEntry::GdtEntry(Value& gdtEntry)
		: mId(0), mAmount(0), mGdtEntryType(GdtEntryType::FORM), mFactor(0.0f), mAmount2(0), mGDT(0.0f)
	{
		checkMember(gdtEntry, "id", MemberType::INTEGER);
		checkMember(gdtEntry, "amount", MemberType::NUMBER);
		checkMember(gdtEntry, "date", MemberType::DATETIME);
		checkMember(gdtEntry, "email", MemberType::STRING);
		checkMember(gdtEntry, "comment", MemberType::STRING);
		checkMember(gdtEntry, "coupon_code", MemberType::STRING);
		checkMember(gdtEntry, "gdt_entry_type_id", MemberType::INTEGER);			
		checkMember(gdtEntry, "factor", MemberType::NUMBER);
		checkMember(gdtEntry, "amount2", MemberType::NUMBER);
		checkMember(gdtEntry, "factor2", MemberType::NUMBER);
		checkMember(gdtEntry, "gdt", MemberType::NUMBER);			

		mId = gdtEntry["id"].GetInt();
		mAmount = static_cast<long long>(round(gdtEntry["amount"].GetDouble() * 100.0));
		mDateString = gdtEntry["date"].GetString();
		mEmail = gdtEntry["email"].GetString();
		mComment = gdtEntry["comment"].GetString();
		mCouponCode = gdtEntry["coupon_code"].GetString();
		
		
		int gdtEntryTypeId = gdtEntry["gdt_entry_type_id"].GetInt();
		if (gdtEntryTypeId > static_cast<int>(GdtEntryType::MAX)) {
			throw GradidoUnknownEnumException("unknown enum value in model::graphql::GdtEntry", "GdtEntryType", static_cast<int>(gdtEntryTypeId));
		}
		mGdtEntryType = static_cast<GdtEntryType>(gdtEntryTypeId);

		mFactor = gdtEntry["factor"].GetDouble();
		mAmount2 = static_cast<long long>(round(gdtEntry["amount2"].GetDouble() * 100.0));
		mFactor2 = gdtEntry["factor2"].GetDouble();
		mGDT = gdtEntry["gdt"].GetDouble();			
	}

	GdtEntry::GdtEntry(Tuple tuple)
	 : mId(get<0>(tuple)), mAmount(get<1>(tuple)), mDate(get<2>(tuple)), mEmail(get<3>(tuple)),
	   mComment(getFullComment(tuple)), mCouponCode(get<7>(tuple)), 
	   mGdtEntryType((GdtEntryType)get<8>(tuple)), mFactor(get<9>(tuple)), mAmount2(get<10>(tuple)), 
	   mFactor2(get<11>(tuple)), mGDT(0.0)
	{
		mGDT = calculateGdt() / 100.0;
    	std::tm tm = *std::localtime(&mDate);
		std::stringstream ss;
		//2022-09-14T13:09:24+00:00
		ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S+00:00");
		mDateString = ss.str();
	}

	GdtEntry::GdtEntry(int id, long long amount, long long date, 
                const std::string& email, const std::string& comment, 
                const std::string& source, const std::string& project, const std::string& coupon_code,
                int gdt_entry_type_id, double factor, long long amount2, double factor2, double gdt)
	: mId(id), mAmount(amount), mDate(date), mEmail(email),
	   mComment(getFullComment(comment, source, project)), mCouponCode(coupon_code), 
	   mGdtEntryType((GdtEntryType)gdt_entry_type_id), mFactor(factor), mAmount2(amount2), 
	   mFactor2(factor2), mGDT(gdt)
	{

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