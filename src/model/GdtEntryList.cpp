#include "GdtEntryList.h"
#include "Config.h"
#include "../logging/Logging.h"

#include <cmath>

namespace model {
	
	GdtEntryList::GdtEntryList()
		: mTotalCount(0), mTotalGDTSum(0.0f), mLastUpdate(std::time(nullptr))
	{
	}

	GdtEntryList::~GdtEntryList()
	{

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