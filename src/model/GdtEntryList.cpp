#include "GdtEntryList.h"
#include "../lib/RapidjsonHelper.h"

using namespace rapidjson;
using namespace rapidjson_helper;

namespace model {
	
	GdtEntryList::GdtEntryList(Value& gdtEntryList)
		: mTotalCount(0), mTotalGDTSum(0.0f)
	{
		checkMember(gdtEntryList, "count", MemberType::INTEGER);
		checkMember(gdtEntryList, "gdtEntries", MemberType::ARRAY);
		checkMember(gdtEntryList, "gdtSum", MemberType::NUMBER);

		mTotalCount = gdtEntryList["count"].GetInt();
		mTotalGDTSum = gdtEntryList["gdtSum"].GetFloat();
		for (auto& gdtEntry : gdtEntryList["gdtEntries"].GetArray()) {
			mGdtEntries.push_back(gdtEntry);
		}
	}

	GdtEntryList::~GdtEntryList()
	{

	}

	Value GdtEntryList::toJson(Document::AllocatorType& alloc, Profiler timeUsed)
	{
		Value listGDTEntries(kObjectType);

		listGDTEntries.AddMember("count", mTotalCount, alloc);
		Value gdtEntries(kArrayType);

		for (auto& gdtEntry : mGdtEntries) {
			gdtEntries.PushBack(gdtEntry.toJson(alloc), alloc);
		}			

		listGDTEntries.AddMember("gdtEntries", gdtEntries, alloc);
		listGDTEntries.AddMember("gdtSum", mTotalGDTSum, alloc);
		listGDTEntries.AddMember("state", "success", alloc);
		listGDTEntries.AddMember("timeUsed", timeUsed.seconds(), alloc);

		return listGDTEntries;
	}
}