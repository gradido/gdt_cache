#include "GdtEntries.h"
#include "../lib/Profiler.h"

#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

namespace controller
{
    GdtEntries::GdtEntries()
    {

    }    

    GdtEntries::~GdtEntries()
    {

    }

    bool GdtEntries::checkForMissingGlobalMod(
        std::shared_ptr<model::Customer> customer,
        std::shared_ptr<model::GdtEntryList> gdtEntriesList
    )
    {
        Profiler timeUsed;
       
        auto aEmails = customer->getEmails();
        std::map<std::string, int> emailIndexMap;
        for(int i = 0; i < aEmails.size(); i++) {
            emailIndexMap.insert({aEmails[i], i});
        }
        
        // 1 for at least one 'normal' gdt entry in global mod range
        // 2 for at least one 'normal global mod' gdt entry in global mod range
        
        for(int i = 0; i < mGlobalMods.size(); i++) {
            mGlobalModCheckMatrixBuffer[i].reserve(aEmails.size());
            memset(mGlobalModCheckMatrixBuffer[i].data(), 0, aEmails.size() * sizeof(uint8_t));
        }        
        for(const auto& entry: gdtEntriesList->getGdtEntries()) 
        {
            auto it = emailIndexMap.find(entry.getEmail());
            if(it == emailIndexMap.end()) continue;
            auto emailIndex = it->second;
            //printf("%d: \n", entry.getId());
            // check for global mod
            for(int iGlobalMod = 0; iGlobalMod < mGlobalMods.size(); iGlobalMod++) {
                auto globalMod = mGlobalMods[iGlobalMod];
                
                if(entry.getDate() > globalMod.getStartDate() && entry.getDate() <= globalMod.getEndDate()) {
              /*      printf("%d.%d (%s) %s %d\n", 
                        iGlobalMod, emailIndex, entry.getEmail().data(), 
                        entry.getGdtEntryTypeString(entry.getGdtEntryType()), entry.getDate());*/
                    switch(entry.getGdtEntryType()) 
                    {                        
                        // normal 1
                    case model::GdtEntry::GdtEntryType::FORM:
                    case model::GdtEntry::GdtEntryType::CVS:
                    case model::GdtEntry::GdtEntryType::ELOPAGE:
                    case model::GdtEntry::GdtEntryType::ELOPAGE_PUBLISHER:
                    case model::GdtEntry::GdtEntryType::DIGISTORE:
                    case model::GdtEntry::GdtEntryType::CVS2: 
                    case model::GdtEntry::GdtEntryType::CVS_STAFF_WAGE:
                        mGlobalModCheckMatrixBuffer[iGlobalMod][emailIndex] |= 1;
                        break;
                        // normal global mod 2
                    case model::GdtEntry::GdtEntryType::GLOBAL_MODIFICATOR:
                        mGlobalModCheckMatrixBuffer[iGlobalMod][emailIndex] |= 2;
                        break;
                    }
                } else {
                    //printf("%d < %d <  %d\n", globalMod.getStartDate(), globalMod.getEndDate(), entry.getDate());
                }
            }
            //printf("\n");
        }
        bool findMissingGlobalMod = false;
        if(gdtEntriesList->getTotalCount() > 0) {
            for(int iGlobalMod = 0; iGlobalMod < mGlobalMods.size(); iGlobalMod++) {
                for(int iEmail = 0; iEmail < aEmails.size(); iEmail++) {
                    auto matrixValue = mGlobalModCheckMatrixBuffer[iGlobalMod][iEmail];
                    // good
                    //if(matrixValue == 3 || matrixValue == 0) {
                    // bad
                    if((matrixValue & 3) == 1) {
                        findMissingGlobalMod = true;
                        printf("[%s] find missing global mod, matrix value: %d, email: %s, global mod: %s\n", 
                            __FUNCTION__, matrixValue, aEmails[iEmail].data(), mGlobalMods[iGlobalMod].getName().data());
                        /*rapidjson::Document base;
                        auto json = gdtEntriesList->toJson(base.GetAllocator());
                        rapidjson::StringBuffer s;
                        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);    
                        json.Accept(writer);

                        printf(s.GetString());
*/
                        break;
                    }
                }
                if(findMissingGlobalMod) break;
            }
        }
        
        // seems to be between 6 and 60 micro seconds
/*      printf("[%s] time for checking for global mod from %s: %s\n", 
            __FUNCTION__, customer->getEmails().front().data(), timeUsed.string().data()
        );*/
        return findMissingGlobalMod;
        
    }

    void GdtEntries::loadGlobalModificators(li::mysql_connection<li::mysql_functions_blocking> connection)
    {
        Profiler timeUsed;
        mGlobalMods.clear();
        mGlobalModCheckMatrixBuffer.clear();

        auto rows = connection("select id, name, IFNULL(UNIX_TIMESTAMP(start_date),946681200), UNIX_TIMESTAMP(end_date) from global_modificators where end_date < now()");
        rows.map([&](int id, std::string name, std::time_t startDate, std::time_t endDate) {
            mGlobalMods.push_back(model::GlobalModificator(id, name, startDate, endDate));
            mGlobalModCheckMatrixBuffer.push_back(std::vector<uint8_t>());
        });
        printf("[%s] time for loading global modificators: %s\n", __FUNCTION__, timeUsed.string().data());

       /* timeUsed.reset();
        auto rows = connection("select gdt_entry_id, global_modificator_id, email from gdt_modificator_entries");
        int count = 0;
        rows.map([&](int gdtEntryId, int globalModificatorId, std::string email) {
            count++;
        });
        printf("[%s] time for loading %d global modificator entries: %s\n",
            __FUNCTION__, count, timeUsed.string().data());
            */
    }
} // namespace controller