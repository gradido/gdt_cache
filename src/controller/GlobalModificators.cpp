#include "GlobalModificators.h"

namespace controller
{
    GlobalModificators::GlobalModificators()
    {

    }    

    GlobalModificators::~GlobalModificators()
    {

    }

    int GlobalModificators::checkForMissingGlobalMod(
        model::CustomerPtr customer,
        model::GdtEntryListPtr gdtEntriesList,
        li::mysql_connection<li::mysql_functions_blocking> connection
    )
    {       
        auto aEmails = customer->getEmails();
        std::map<std::string, int> emailIndexMap;
        for(int i = 0; i < aEmails.size(); i++) {
            emailIndexMap.insert({aEmails[i], i});
        }
        
        int addedGlobalModGdtEntrySum = 0;
        int createMissingGlobalMod = 0;
        do {
            createMissingGlobalMod = 0;
            for(int i = 0; i < mGlobalMods.size(); i++) {
                mGlobalModCheckMatrixBuffer[i].reserve(aEmails.size());
                memset(mGlobalModCheckMatrixBuffer[i].data(), 0, aEmails.size() * sizeof(uint8_t));
            }        
            for(const auto& entry: gdtEntriesList->getGdtEntries()) 
            {
                auto it = emailIndexMap.find(entry.getEmail());
                if(it == emailIndexMap.end()) continue;
                auto emailIndex = it->second;
                
                // check for global mod
                for(int iGlobalMod = 0; iGlobalMod < mGlobalMods.size(); iGlobalMod++) {
                    auto globalMod = mGlobalMods[iGlobalMod];
                    
                    if(entry.getDate() > globalMod.getStartDate() && entry.getDate() <= globalMod.getEndDate()) {
                
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
                    } 
                }
            }
            
            if(gdtEntriesList->getTotalCount() > 0) {
                for(int iGlobalMod = 0; iGlobalMod < mGlobalMods.size(); iGlobalMod++) {
                    for(int iEmail = 0; iEmail < aEmails.size(); iEmail++) {
                        auto matrixValue = mGlobalModCheckMatrixBuffer[iGlobalMod][iEmail];

                        if((matrixValue & 3) == 1) {
                            Profiler timeUsedNewMod;
                            gdtEntriesList->calculateAndInsertGlobalModificatorEntry(mGlobalMods[iGlobalMod], aEmails[iEmail], connection);
                            createMissingGlobalMod++;
                            break;
                        }
                    }
                }
            }
            addedGlobalModGdtEntrySum += createMissingGlobalMod;
        } while(createMissingGlobalMod);
        
        return addedGlobalModGdtEntrySum;
        
    }

    bool GlobalModificators::updateGlobalModificators(model::GlobalModificators& globalMods)
    {
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

        return diffFound;
    }

} // namespace controller