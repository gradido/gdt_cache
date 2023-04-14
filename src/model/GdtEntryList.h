#ifndef __GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_LIST_H
#define __GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_LIST_H

#include "GdtEntry.h"
#include "../lib/Profiler.h"
#include <vector>

namespace model {
	
	class GdtEntryList : public Base
	{
	public:
		GdtEntryList(rapidjson::Value& gdtEntryList);
		~GdtEntryList();

		rapidjson::Value toJson(rapidjson::Document::AllocatorType& alloc, Profiler timeUsed);
		const char* getTypename() { return "GdtEntryList"; }
	protected:
		int mTotalCount;
		std::vector<GdtEntry> mGdtEntries;
		float mTotalGDTSum;

	};
}

#endif //__GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_LIST_H