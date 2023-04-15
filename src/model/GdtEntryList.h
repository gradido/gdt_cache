#ifndef __GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_LIST_H
#define __GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_LIST_H

#include "GdtEntry.h"

#include "../lib/Profiler.h"
#include <list>
#include <set>
#include <ctime>

namespace model {
	
	class GdtEntryList : public Base
	{
	public:

		enum class OrderDirections 
		{
			ASC,
			DESC
		};

		GdtEntryList(rapidjson::Value& gdtEntryList);
		GdtEntryList();
		~GdtEntryList();

		//! \return added gdt entries count
		//! \param emails point to set if you like to know which email addresses are present in gdt entries
		int updateGdtEntries(rapidjson::Value& gdtEntryList, std::set<std::string>* emails = nullptr);

		rapidjson::Value toJson(rapidjson::Document::AllocatorType& alloc);
		rapidjson::Value toJson(rapidjson::Document::AllocatorType& alloc, Profiler timeUsed);
		rapidjson::Value toJson(
			rapidjson::Document::AllocatorType& alloc, 
			Profiler timeUsed, 
			int page, 
			int count, 
			OrderDirections order
		);
		const char* getTypename() { return "GdtEntryList"; }
		static OrderDirections orderDirectionFromString(const std::string& orderDirection);
		static const char* orderDirectionsToString(OrderDirections dir);

		//! \return true if last update is older than Config::minCacheTimeout
		bool canUpdate();

		inline size_t getGdtEntriesCount() { return mGdtEntries.size();}
		inline int getTotalCount() {return mTotalCount;}

		// todo calculate sum
		// update with new data
	protected:
		int mTotalCount;
		std::list<GdtEntry> mGdtEntries;
		float mTotalGDTSum;
		std::time_t mLastUpdate;
	};
}

#endif //__GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_LIST_H