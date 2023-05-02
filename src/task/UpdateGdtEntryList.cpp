#include "UpdateGdtEntryList.h"
#include "../controller/Contacts.h"
#include "../GdtEntriesCache.h"

namespace task 
{
    UpdateGdtEntryList::UpdateGdtEntryList(const std::string& email)
    : mEmail(email), mName(email)
    {
        for(int i = 2; i < email.size() - 2; i++) {
            mName.data()[i] = 'x';
        }
    }   


    UpdateGdtEntryList::~UpdateGdtEntryList()
    {

    }

    void UpdateGdtEntryList::run(
        li::mysql_connection<li::mysql_functions_blocking> connection,
        controller::GdtEntries& gdtEntries
    )
    {
        Profiler timeUsed;
        auto gc = GdtEntriesCache::getInstance();

        
        if(!mEmail.size()) return;        
        if((!gc->canUpdateGdtEntryList(mEmail))) return;
        
        auto customer = controller::Contacts::loadCustomerFromDb(connection, mEmail);
        if(!customer) customer = std::make_shared<model::Customer>(mEmail);
        auto gdtEntriesList = gdtEntries.loadGdtEntriesFromDB(customer, connection);
        auto addedMissingGlobCount = gdtEntries.checkForMissingGlobalMod(customer, gdtEntriesList, connection);
        gc->swapGdtEntryList(customer, gdtEntriesList);
        if(gdtEntriesList->getTotalCount() > 0) {
            printf("[UpdateGdtEntryList] %s timeUsed for loading %ld contacts, %d gdt entries and %d global mod\n",
                timeUsed.string().data(), customer->getEmails().size(), gdtEntriesList->getTotalCount(), addedMissingGlobCount
            );
        }
    }
}