#include "GdtEntryList.h"
#include "../utils/RapidjsonHelper.h"
#include "../GradidoBlockchainException.h"
#include "Config.h"
#include "../logging/Logging.h"

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
	
	bool GdtEntryList::canUpdate()
	{
		return (std::time(nullptr) - mLastUpdate) > g_Config->minCacheTimeout;
	}

	bool GdtEntryList::shouldUpdate()
	{
		return (std::time(nullptr) - mLastUpdate) > 60 * 1;//60 * 60 * 4;
	}

	double GdtEntryList::calculateEuroSum() const
	{
		long long euroSumCent = 0;
		for(auto& gdtEntry: mGdtEntries) {
			euroSumCent += gdtEntry.getEuroAmount();
		}
		return static_cast<double>(euroSumCent) / 100.0;
	}

}