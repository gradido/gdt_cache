#ifndef __GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_LIST_H
#define __GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_LIST_H

#include "GdtEntry.h"

#include "../lib/Profiler.h"
#include <list>

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
		~GdtEntryList();

		//! \return added gdt entries count
		int updateGdtEntries(rapidjson::Value& gdtEntryList);

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
		// todo calculate sum
		// update with new data
	protected:
		int mTotalCount;
		std::list<GdtEntry> mGdtEntries;
		float mTotalGDTSum;

	};
}

#endif //__GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_LIST_H