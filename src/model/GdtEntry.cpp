#include "GdtEntry.h"
#include "../GradidoBlockchainException.h"
#include "../lib/RapidjsonHelper.h"

using namespace rapidjson;
using namespace rapidjson_helper;

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
}