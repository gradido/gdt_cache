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

    int GdtEntries::checkForMissingGlobalMod(
        std::shared_ptr<model::Customer> customer,
        std::shared_ptr<model::GdtEntryList> gdtEntriesList,
        li::mysql_connection<li::mysql_functions_blocking> connection
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
        int createMissingGlobalMod = 0;
        if(gdtEntriesList->getTotalCount() > 0) {
            for(int iGlobalMod = 0; iGlobalMod < mGlobalMods.size(); iGlobalMod++) {
                for(int iEmail = 0; iEmail < aEmails.size(); iEmail++) {
                    auto matrixValue = mGlobalModCheckMatrixBuffer[iGlobalMod][iEmail];
                    // good
                    //if(matrixValue == 3 || matrixValue == 0) {
                    // bad
                    if((matrixValue & 3) == 1) {
                        Profiler timeUsedNewMod;
                        gdtEntriesList->calculateAndInsertGlobalModificatorEntry(mGlobalMods[iGlobalMod], aEmails[iEmail], connection);
                        createMissingGlobalMod++;
                        //printf("[%s] time for adding new global mod gdt entry: %s\n", __FUNCTION__, timeUsedNewMod.string().data());
                        //findMissingGlobalMod = true;
                        //printf("[%s] find missing global mod, matrix value: %d, email: %s, global mod: %s\n", 
                          //  __FUNCTION__, matrixValue, aEmails[iEmail].data(), mGlobalMods[iGlobalMod].getName().data());
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
            }
        }
        
        // seems to be between 6 and 60 micro seconds
/*      printf("[%s] time for checking for global mod from %s: %s\n", 
            __FUNCTION__, customer->getEmails().front().data(), timeUsed.string().data()
        );*/
        return createMissingGlobalMod;
        
    }

    bool GdtEntries::loadGlobalModificators(li::mysql_connection<li::mysql_functions_blocking> connection)
    {
        Profiler timeUsed;
        std::vector<model::GlobalModificator> globalMods;
        // print 'diffFound' only if loadGlobalModificators wasn't the first call
        bool created = mGlobalMods.size() == 0;
        auto rows = connection(
            "select id, name, factor, IFNULL(UNIX_TIMESTAMP(start_date),946681200), UNIX_TIMESTAMP(end_date) \
             from global_modificators where end_date < now() order by end_date ASC");
        rows.map([&](int id, std::string name, float factor, std::time_t startDate, std::time_t endDate) {
            globalMods.push_back(model::GlobalModificator(id, name,factor, startDate, endDate));
        });
        bool diffFound = false;
        if(mGlobalMods.size() != globalMods.size()) {
            diffFound = true;
        } else {
            for(int i = 0; i < globalMods.size(); i++) {
                if(mGlobalMods[i] != globalMods[i]) {
                    diffFound = true;
                    break;
                }
            }
        }
        if(diffFound) {
            if(mGlobalMods.size() != globalMods.size()) {
                mGlobalModCheckMatrixBuffer.clear();
                mGlobalModCheckMatrixBuffer = std::vector<std::vector<uint8_t>>(globalMods.size(), std::vector<uint8_t>());
            }
            mGlobalMods.swap(globalMods);
        }

        //    mGlobalModCheckMatrixBuffer.push_back(std::vector<uint8_t>());
        if(created) {
            printf("[%s] time for loading global modificators: %s\n", __FUNCTION__, timeUsed.string().data());
        } else {
            printf("[%s] time for loading global modificators: %s, diffFound: %d\n", __FUNCTION__, timeUsed.string().data(), static_cast<int>(diffFound));
        }
        return diffFound;
    }

    std::shared_ptr<model::GdtEntryList> GdtEntries::loadGdtEntriesFromDB(
        std::shared_ptr<model::Customer> customer,
        li::mysql_connection<li::mysql_functions_blocking> connection
    )
    {
        auto gdtEntries = std::make_shared<model::GdtEntryList>();

        auto prepared = connection.prepare(
        "select id, amount, UNIX_TIMESTAMP(date), LOWER(TRIM(email)), IFNULL(comment, ''), \
        IFNULL(source, ''), IFNULL(project, ''), IFNULL(coupon_code, ''), \
        gdt_entry_type_id, factor, amount2, factor2 from gdt_entries where email IN (?) order by date ASC");
        
        auto rows = prepared(customer->getEmailsString());
        while (auto row = rows.read_optional<int, long long, int, std::string, std::string, std::string, std::string, std::string, int, float, int, float>())
        {
            model::GdtEntry entry(row.value());
            // skip gdt entries with empty emails
            if (entry.getEmail().size() == 0)
                continue;

            gdtEntries->addGdtEntry(entry);
            // printf("add %d: %ld\n", entry.getId(), entry.getDate());
        }
        return gdtEntries;
    }
} // namespace controller