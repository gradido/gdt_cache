#include "GdtEntryList.h"
#include "../lib/RapidjsonHelper.h"
#include "../GradidoBlockchainException.h"
#include "Config.h"
#include "../ErrorContainer.h"

#include <cmath>

using namespace rapidjson;
using namespace rapidjson_helper;

namespace model {
	
	GdtEntryList::GdtEntryList(Value& gdtEntryList)
		: mTotalCount(0), mTotalGDTSum(0.0f), mLastUpdate(std::time(nullptr))
	{
		addGdtEntry(gdtEntryList);
	}

	GdtEntryList::GdtEntryList()
		: mTotalCount(0), mTotalGDTSum(0.0f), mLastUpdate(std::time(nullptr))
	{
	}

	GdtEntryList::~GdtEntryList()
	{

	}

	int GdtEntryList::addGdtEntry(Value& gdtEntryList, std::set<std::string>* emails/*= nullptr*/)
	{
		checkMember(gdtEntryList, "count", MemberType::INTEGER);
		checkMember(gdtEntryList, "gdtEntries", MemberType::ARRAY);
		checkMember(gdtEntryList, "gdtSum", MemberType::NUMBER);
				
		mTotalCount = gdtEntryList["count"].GetInt();
		mTotalGDTSum = gdtEntryList["gdtSum"].GetFloat();
		int dataSetSize = gdtEntryList["gdtEntries"].GetArray().Size();
		
		double calculateGdtSum = 0.0;

		for (auto& gdtEntry : gdtEntryList["gdtEntries"].GetArray()) 
		{
			GdtEntry entry(gdtEntry);
			calculateGdtSum += entry.getGdt();
			mGdtEntries.push_back(entry);
		
			if(emails) {
				emails->insert(entry.getEmail());
			}
		}
		// TODO: replace 40 by macro or config value
		if(dataSetSize != 500) {
			if(mTotalCount != mGdtEntries.size()) {
				fprintf(stderr, "[%s] count mismatch total count: %d != gdtEntries list size: %ld\n",
					__FUNCTION__, mTotalCount, mGdtEntries.size());
			}
			if(fabs(calculateGdtSum - mTotalGDTSum) > 0.01) {
				fprintf(stderr, "[%s] gdt sum mismatch total gdt sum: %.4f != calculated gdt sum: %.4f\n",
					__FUNCTION__, mTotalGDTSum, calculateGdtSum);
			}
		}
		mLastUpdate = std::time(nullptr);
		return dataSetSize;
	}
	void GdtEntryList::addGdtEntry(const GdtEntry& gdtEntry)
	{
		mTotalCount++;
		mTotalGDTSum += gdtEntry.getGdt();
		if(mGdtEntries.size() && mGdtEntries.back().getDate() > gdtEntry.getDate()) {
			fprintf(stderr, "[%s] gdt entry arrived out of order %d < %d!\n", __FUNCTION__, mGdtEntries.back().getId(), gdtEntry.getId());
		}
		mGdtEntries.push_back(gdtEntry);
		mLastUpdate = std::time(nullptr);
	}

	void GdtEntryList::insertGdtEntry(const GdtEntry& gdtEntry)
	{
		mTotalCount++;
		mTotalGDTSum += gdtEntry.getGdt();
		if(!mGdtEntries.size() || mGdtEntries.back().getDate() <= gdtEntry.getDate()) {
			mGdtEntries.push_back(gdtEntry);
		} else {
			for(auto it = mGdtEntries.begin(); it != mGdtEntries.end(); it++) {
				if(it->getDate() >= gdtEntry.getDate()) {
					mGdtEntries.insert(it, gdtEntry);
					break;
				}
			}
		}
	}

	void GdtEntryList::calculateAndInsertGlobalModificatorEntry(
		const GlobalModificator& globalMod, 
		const std::string& email,
		li::mysql_connection<li::mysql_functions_blocking> connection
	)
	{
		if(!mGdtEntries.size()) return;
		long long gdtSum = 0;
		auto it = mGdtEntries.begin();
		for(; it != mGdtEntries.end(); it++) {
			if(it->getEmail() != email) continue;
			if(it->getDate() >= globalMod.getStartDate() && it->getDate() <= globalMod.getEndDate()) {
				gdtSum += it->calculateGdt();
			}
			if(it->getDate() > globalMod.getEndDate()) {
				break;
			}
		}

		auto prepared = connection.prepare(
			"INSERT INTO gdt_entries(email, gdt_entry_type_id, amount, factor, date, project) \
			 VALUES(?,?,?,?,FROM_UNIXTIME(?),?)"
		);
		long long integerGdtSum = static_cast<long long>(round(gdtSum* 100.0));
		prepared(
			email, 7, gdtSum, 
			globalMod.getFactor(), static_cast<long long>(globalMod.getEndDate()), 
			globalMod.getName()
		);
		auto id = connection.last_insert_rowid();
		auto prepared2 = connection.prepare(
			"INSERT INTO gdt_modificator_entries(gdt_entry_id, global_modificator_id, email) \
			 VALUES(?,?,?)"
		);
		prepared2(id, globalMod.getId(), email);

		// typedef std::tuple<int, long long, std::time_t, std::string, std::string, std::string, std::string, std::string, int, float, long long, float> Tuple;
		// id, amount, date, email, comment, source, project, coupon_code, gdt_entry_type_id, factor, amount2, factor2
		GdtEntry globalModGdtEntry({
			id, gdtSum, globalMod.getEndDate(), email,"", "", globalMod.getName(), "", 7, globalMod.getFactor(),0, 1.0
		});
		// todo: maybe optimize in future to save memory using reference instead of copy
		insertGdtEntry(globalModGdtEntry);
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

		// only go into loop if enough gdt entries exist for return at least one
		if(mGdtEntries.size() > (page-1)*count) 
		{
			// start at the beginning of list (smallest date) by order == ASC
			auto it = mGdtEntries.begin();
			// start at the end of list (largest date) by order == DESC
			if(order == OrderDirections::DESC) {
				it = mGdtEntries.end();
				// end is actually after the last entry so we need to move on step back
				it--;
			}
			
			// move iterator to position where to start collecting gdt entries
			// if client request at least page 2
			if(page > 1) {					
				// for every entry to skip
				for(int i = 0; i < (page-1) * count; i++) {
					if(order == OrderDirections::ASC) {
						// increment iterator by order == ASC
						it++;
					} else {
						// decrement iterator by order == DESC
						it--;
					}
				}					
			}
			// collect max count gdt entries
			for(int i = 0; i < count; i++) 
			{
				gdtEntries.PushBack(it->toJson(alloc), alloc);	
				if(order == OrderDirections::ASC) {
					// increment iterator by order == ASC
					it++;
					// end is already past the last entry so we exit here
					if(it == mGdtEntries.end()) break;
				} else {
					// begin is the first entry, we cannot further decrement here
					if(it == mGdtEntries.begin()) break;
					// decrement iterator by order == DESC
					it--;					
				}
			}
		}

		listGDTEntries.AddMember("gdtEntries", gdtEntries, alloc);
		listGDTEntries.AddMember("gdtSum", mTotalGDTSum, alloc);
		listGDTEntries.AddMember("state", "success", alloc);
		listGDTEntries.AddMember("timeUsed", static_cast<float>(timeUsed.seconds()), alloc);

		return listGDTEntries;
	}


	GdtEntryList::OrderDirections GdtEntryList::orderDirectionFromString(const std::string& orderDirection)
	{
		if(orderDirection == "ASC") return OrderDirections::ASC;
		if(orderDirection == "DESC") return OrderDirections::DESC;
		//fprintf(stderr, "[%s] invalid order direction: %s, use ASC as default\n", __FUNCTION__, orderDirection.data());
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

	bool GdtEntryList::shouldUpdate()
	{
		return (std::time(nullptr) - mLastUpdate) > 60 * 1;//60 * 60 * 4;
	}

}