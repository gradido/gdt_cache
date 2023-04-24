#include "UpdateGdtEntryList.h"
#include "../controller/Contacts.h"
#include "../GdtEntriesCache.h"

namespace task 
{
    UpdateGdtEntryList::UpdateGdtEntryList(const std::string& email)
    : mEmail(email)
    {

    }


    UpdateGdtEntryList::~UpdateGdtEntryList()
    {

    }

    void UpdateGdtEntryList::run(
        li::mysql_connection<li::mysql_functions_blocking> connection,
        controller::GdtEntries& gdtEntries
    )
    {
        auto gc = GdtEntriesCache::getInstance();

        printf("[%s] check\n", __FUNCTION__);
        if(!mEmail.size()) return;        
        if((!gc->shouldUpdateGdtEntryList(mEmail) || !gc->canUpdateGdtEntryList(mEmail))) return;
        printf("[%s] start\n", __FUNCTION__);
        auto customer = controller::Contacts::loadCustomerFromDb(connection, mEmail);
        if(!customer) customer = std::make_shared<model::Customer>(mEmail);
        printf("[%s] loaded customers: %s\n", __FUNCTION__, customer->getEmailsString().data());
        auto gdtEntriesList = gdtEntries.loadGdtEntriesFromDB(customer, connection);
        printf("[%s] load gdt entries from db: %d", __FUNCTION__, gdtEntriesList->getTotalCount());
        auto addedMissingGlobCount = gdtEntries.checkForMissingGlobalMod(customer, gdtEntriesList, connection);
        printf("[%s] added missing glob count: %d", __FUNCTION__, addedMissingGlobCount);
        gc->swapGdtEntryList(customer, gdtEntriesList);
        printf("[%s] end\n", __FUNCTION__);
    }
}