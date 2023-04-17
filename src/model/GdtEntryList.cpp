#include "GdtEntryList.h"
#include "../lib/RapidjsonHelper.h"
#include "../GradidoBlockchainException.h"
#include "Config.h"

#include <cmath>

using namespace rapidjson;
using namespace rapidjson_helper;

namespace model {
	
	GdtEntryList::GdtEntryList(Value& gdtEntryList)
		: mTotalCount(0), mTotalGDTSum(0.0f)
	{
		updateGdtEntries(gdtEntryList);
	}

	GdtEntryList::GdtEntryList()
		: mTotalCount(0), mTotalGDTSum(0.0f)
	{
	}

	GdtEntryList::~GdtEntryList()
	{

	}

	int GdtEntryList::updateGdtEntries(Value& gdtEntryList, std::set<std::string>* emails/*= nullptr*/)
	{
		checkMember(gdtEntryList, "count", MemberType::INTEGER);
		checkMember(gdtEntryList, "gdtEntries", MemberType::ARRAY);
		checkMember(gdtEntryList, "gdtSum", MemberType::NUMBER);
		
		mTotalCount = gdtEntryList["count"].GetInt();
		mTotalGDTSum = gdtEntryList["gdtSum"].GetFloat();
		auto it = mGdtEntries.begin();
		bool compareEntries = true;
		if(!mGdtEntries.size()) compareEntries = false;
		int addedOrUpdatedCount = 0;
		float calculateGdtSum = 0.0;
		for (auto& gdtEntry : gdtEntryList["gdtEntries"].GetArray()) 
		{
			GdtEntry entry(gdtEntry);
			if(it == mGdtEntries.end()) {
				compareEntries = false;
			}
			calculateGdtSum += entry.getGdt();
			if(emails) {
				emails->insert(entry.getEmail());
			}
			if(!compareEntries) {
				mGdtEntries.push_back(gdtEntry);
				addedOrUpdatedCount++;
			} else {
				while(it->getDate() < entry.getDate()) {
					it++;
					if(it == mGdtEntries.end()) {
						addedOrUpdatedCount++;	
						it = mGdtEntries.insert(it, entry);
					}
				}
				if(*it != entry) {
					addedOrUpdatedCount++;
					// update, replace with updated element
					if(it->getId() == entry.getId()) {
						it = mGdtEntries.erase(it);
						it = mGdtEntries.insert(it, entry);
						// new element
					} else {
						it = mGdtEntries.insert(it, entry);
					}
				}
				it++;
			}
		}
		if(mTotalCount != mGdtEntries.size()) {
			fprintf(stderr, "[%s] count mismatch total count: %d != gdtEntries list size: %ld\n",
			 	__FUNCTION__, mTotalCount, mGdtEntries.size());
		}
		if(fabs(calculateGdtSum - mTotalGDTSum) > 0.01f) {
			fprintf(stderr, "[%s] gdt sum mismatch total gdt sum: %.4f != calculated gdt sum: %.4f\n",
				__FUNCTION__, mTotalGDTSum, calculateGdtSum);
		}
		mLastUpdate = std::time(nullptr);
		return addedOrUpdatedCount;
	}
	void GdtEntryList::addGdtEntry(GdtEntry gdtEntry)
	{
		mTotalCount++;
		mTotalGDTSum += gdtEntry.getGdt();
		mGdtEntries.push_back(gdtEntry);
		mLastUpdate = std::time(nullptr);
	}

	Value GdtEntryList::toJson(rapidjson::Document::AllocatorType& alloc)
	{
		Value listGDTEntries(kObjectType);

		listGDTEntries.AddMember("count", mTotalCount, alloc);
		Value gdtEntries(kArrayType);

		for (auto& gdtEntry : mGdtEntries) {
			gdtEntries.PushBack(gdtEntry.toJson(alloc), alloc);
		}			

		listGDTEntries.AddMember("gdtEntries", gdtEntries, alloc);
		listGDTEntries.AddMember("gdtSum", mTotalGDTSum, alloc);

		return listGDTEntries;
	}

	Value GdtEntryList::toJson(Document::AllocatorType& alloc, Profiler timeUsed)
	{
		auto result = toJson(alloc);
		
		result.AddMember("state", "success", alloc);
		result.AddMember("timeUsed", timeUsed.seconds(), alloc);

		return result;
	}

	Value GdtEntryList::toJson(
		Document::AllocatorType& alloc, 
		Profiler timeUsed, 
		int page, 
		int count, 
		OrderDirections order
	)
	{
		Value listGDTEntries(kObjectType);

		listGDTEntries.AddMember("count", mTotalCount, alloc);
		Value gdtEntries(kArrayType);

		// TODO: implement pagination
		for (auto& gdtEntry : mGdtEntries) {
			gdtEntries.PushBack(gdtEntry.toJson(alloc), alloc);
		}			

		listGDTEntries.AddMember("gdtEntries", gdtEntries, alloc);
		listGDTEntries.AddMember("gdtSum", mTotalGDTSum, alloc);
		listGDTEntries.AddMember("state", "success", alloc);
		listGDTEntries.AddMember("timeUsed", timeUsed.seconds(), alloc);

		return listGDTEntries;
	}


	GdtEntryList::OrderDirections GdtEntryList::orderDirectionFromString(const std::string& orderDirection)
	{
		if(orderDirection == "ASC") return OrderDirections::ASC;
		if(orderDirection == "DESC") return OrderDirections::DESC;
		fprintf(stderr, "[%s] invalid order direction: %s, use ASC as default\n", __FUNCTION__, orderDirection.data());
		return OrderDirections::ASC;
	}
	const char* GdtEntryList::orderDirectionsToString(GdtEntryList::OrderDirections dir)
	{
		switch(dir) {
			case OrderDirections::ASC: return "ASC";
			case OrderDirections::DESC: return "DESC";
		}
		throw GradidoUnhandledEnum("GdtEntryList::orderDirectionsToString", "OrderDirections", static_cast<int>(dir));
	}

	bool GdtEntryList::canUpdate()
	{
		return (std::time(nullptr) - mLastUpdate) > g_Config->minCacheTimeout;
	}

}