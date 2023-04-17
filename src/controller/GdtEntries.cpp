#include "GdtEntries.h"
#include "../lib/Profiler.h"

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
        // 4 for at least one 'staff wage' gdt entry in global mod range
        // 8 for at least one 'staff wage global mod' gdt entry in global mod range
        
        for(int i = 0; i < mGlobalMods.size(); i++) {
            mGlobalModCheckMatrixBuffer[i].clear();
            mGlobalModCheckMatrixBuffer[i].insert(mGlobalModCheckMatrixBuffer[i].begin(), aEmails.size(), 0);
        }        
        for(const auto& entry: gdtEntriesList->getGdtEntries()) 
        {
            auto emailIndex = emailIndexMap.find(entry.getEmail())->second;
            // check for global mod
            for(int iGlobalMod = 0; iGlobalMod < mGlobalMods.size(); iGlobalMod++) {
                auto globalMod = mGlobalMods[iGlobalMod];
                if(mGlobalModCheckMatrixBuffer[iGlobalMod][emailIndex] < 2) {
                    if(entry.getDate() > globalMod.getStartDate() && entry.getDate() < globalMod.getEndDate()) {
                        switch(entry.getGdtEntryType()) 
                        {
                            // normal 1
                        case model::GdtEntry::GdtEntryType::FORM:
                        case model::GdtEntry::GdtEntryType::CVS:
                        case model::GdtEntry::GdtEntryType::ELOPAGE:
                        case model::GdtEntry::GdtEntryType::ELOPAGE_PUBLISHER:
                        case model::GdtEntry::GdtEntryType::DIGISTORE:
                        case model::GdtEntry::GdtEntryType::CVS2: 
                            mGlobalModCheckMatrixBuffer[iGlobalMod][emailIndex] |= 1;
                            break;
                            // normal global mod 2
                        case model::GdtEntry::GdtEntryType::GLOBAL_MODIFICATOR:
                            mGlobalModCheckMatrixBuffer[iGlobalMod][emailIndex] |= 2;
                            break;
                            // staff wage 4
                        case model::GdtEntry::GdtEntryType::CVS_STAFF_WAGE:
                            mGlobalModCheckMatrixBuffer[iGlobalMod][emailIndex] |= 4;
                            break;
                            // staff wage global mod 8
                        case model::GdtEntry::GdtEntryType::STAFF_WAGE_GLOBAL_MODIFICATOR:
                            mGlobalModCheckMatrixBuffer[iGlobalMod][emailIndex] |= 8;
                            break;
                        }
                    }
                }
            }
        }
        bool findMissingGlobalMod = false;
        if(gdtEntriesList->getTotalCount() > 0) {
            for(int iGlobalMod = 0; iGlobalMod < mGlobalMods.size(); iGlobalMod++) {
                for(int iEmail = 0; iEmail < aEmails.size(); iEmail++) {
                    auto matrixValue = mGlobalModCheckMatrixBuffer[iGlobalMod][iEmail];
                    // good
                    //if(matrixValue == 3 || matrixValue == 0 || matrixValue == 12 || matrixValue == 15) {
                    // bad
                    if(matrixValue & 1 == 1 || matrixValue & 4 == 4) {
                        findMissingGlobalMod = true;
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

        auto rows = connection("select id, IFNULL(UNIX_TIMESTAMP(start_date),946681200), UNIX_TIMESTAMP(end_date) from global_modificators where end_date < now()");
        rows.map([&](int id, std::time_t startDate, std::time_t endDate) {
            mGlobalMods.push_back(model::GlobalModificator(id, startDate, endDate));
            mGlobalModCheckMatrixBuffer.push_back(std::vector<uint8_t>());
        });
        printf("[%s] time for loading global modificators: %s\n", __FUNCTION__, timeUsed.string().data());
    }
} // namespace controller