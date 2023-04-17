#include "GdtEntry.h"
#include "../GradidoBlockchainException.h"
#include "../lib/RapidjsonHelper.h"

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
		default: throw GradidoUnknownEnumException("unknown enum value in model::graphql::GdtEntry", "GdtEntryType", static_cast<int>(type));
		}
	}

	GdtEntry::GdtEntry(Value& gdtEntry)
		: mId(0), mAmount(0.0f), mGdtEntryType(GdtEntryType::FORM), mFactor(0.0f), mGDT(0.0f)
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
		mAmount = gdtEntry["amount"].GetFloat();
		mDateString = gdtEntry["date"].GetString();
		mEmail = gdtEntry["email"].GetString();
		mComment = gdtEntry["comment"].GetString();
		mCouponCode = gdtEntry["coupon_code"].GetString();
		
		
		int gdtEntryTypeId = gdtEntry["gdt_entry_type_id"].GetInt();
		if (gdtEntryTypeId > static_cast<int>(GdtEntryType::MAX)) {
			throw GradidoUnknownEnumException("unknown enum value in model::graphql::GdtEntry", "GdtEntryType", static_cast<int>(gdtEntryTypeId));
		}
		mGdtEntryType = static_cast<GdtEntryType>(gdtEntryTypeId);

		mFactor = gdtEntry["factor"].GetFloat();
		mAmount2 = gdtEntry["amount2"].GetFloat();
		mFactor2 = gdtEntry["factor2"].GetFloat();
		mGDT = gdtEntry["gdt"].GetFloat();			
	}

	GdtEntry::GdtEntry(Tuple tuple)
	 : mId(get<0>(tuple)), mAmount((double)get<1>(tuple)/100.0), mDate(get<2>(tuple)), mEmail(get<3>(tuple)),
	   mComment(getFullComment(tuple)), mCouponCode(get<7>(tuple)), 
	   mGdtEntryType((GdtEntryType)get<8>(tuple)), mFactor(get<9>(tuple)), mAmount2((float)get<10>(tuple)/100.0f), 
	   mFactor2(get<11>(tuple)), mGDT(0.0)
	{
		mGDT = mAmount * mFactor * mFactor2 + mAmount2;
	}

	GdtEntry::~GdtEntry()
	{

	}

	Value GdtEntry::toJson(Document::AllocatorType& alloc)
	{
		Value gdtEntry(kObjectType);

		gdtEntry.AddMember("id", mId, alloc);
		gdtEntry.AddMember("amount", mAmount, alloc);
		gdtEntry.AddMember("date", Value(mDateString.data(), alloc), alloc);
		gdtEntry.AddMember("email", Value(mEmail.data(), alloc), alloc);
		gdtEntry.AddMember("comment", Value(mComment.data(), alloc), alloc);
		gdtEntry.AddMember("coupon_code", Value(mCouponCode.data(), alloc), alloc);
		gdtEntry.AddMember("gdt_entry_type_id", Value(getGdtEntryTypeString(mGdtEntryType), alloc), alloc);
		gdtEntry.AddMember("factor", mFactor, alloc);
		gdtEntry.AddMember("amount2", mAmount2, alloc);
		gdtEntry.AddMember("factor2", mFactor2, alloc);
		gdtEntry.AddMember("gdt", mGDT, alloc);

		return gdtEntry;			
	}

	/*
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
	*/
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

	std::string GdtEntry::getFullComment(Tuple tuple)
	{
		auto comment = get<4>(tuple), source = get<5>(tuple), project = get<6>(tuple);
		// &#8210; = langer Gedankenstrich
		std::string fullComment = comment;
        if(source.size()) {
			if(fullComment.size()) fullComment += " – ";
			fullComment += source;
		}
		if(project.size()) {
			if(fullComment.size()) fullComment += " – ";
			fullComment += project;
		}
        
        return fullComment;
	}
}