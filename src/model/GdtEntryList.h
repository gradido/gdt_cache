#ifndef __GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_LIST_H
#define __GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_LIST_H

#include "GdtEntry.h"

#include "../utils/Profiler.h"
#include "GlobalModificator.h"
#include <lithium_mysql.hh>
#include <list>
#include <set>
#include <ctime>

namespace model {
	
	class GdtEntryList
	{
	public:
		GdtEntryList(rapidjson::Value& gdtEntryList);
		GdtEntryList();
		~GdtEntryList();


		inline void reset() {mGdtEntries.clear(); mTotalCount = 0; mTotalGDTSum = 0.0;};
		//! \param emails point to set if you like to know which email addresses are present in gdt entries
		int addGdtEntry(rapidjson::Value& gdtEntryList, std::set<std::string>* emails = nullptr);

		//! add gdt entry from db read, one after another
		void addGdtEntry(const GdtEntry& gdtEntry);

		//! compare dates and insert where it is in order
		void insertGdtEntry(const GdtEntry& gdtEntry);

		//! calculate global mod and return new gdt entry, new gdt entry is inserted 
		void calculateAndInsertGlobalModificatorEntry(
			const GlobalModificator& globalMod, 
			const std::string& email,
			li::mysql_connection<li::mysql_functions_blocking> connection
		);

		//! \return true if last update is older than Config::minCacheTimeout
		bool canUpdate();
		//! \return true if last update is older than 4h
		bool shouldUpdate();

		inline size_t getGdtEntriesCount() const { return mGdtEntries.size();}
		inline int getTotalCount() const {return mTotalCount;}
		inline const std::list<GdtEntry>& getGdtEntries() const {return mGdtEntries;}
		inline double getGdtSum() const { return mTotalGDTSum;}
		double calculateEuroSum() const;

		// todo calculate sum
		// update with new data
	protected:
		int mTotalCount;
		// TODO: use shared_ptr for GdtEntry in list
		std::list<GdtEntry> mGdtEntries;
		double mTotalGDTSum;
		std::time_t mLastUpdate;
	};
	typedef std::shared_ptr<model::GdtEntryList> GdtEntryListPtr;
	typedef std::unordered_map<std::string, model::GdtEntryListPtr> EmailGdtEntriesListMap;
}

#endif //__GRADIDO_DESKTOP_MODEL_GRAPHQL_GDT_ENTRY_LIST_H