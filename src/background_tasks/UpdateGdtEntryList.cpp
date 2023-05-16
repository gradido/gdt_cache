#include "UpdateGdtEntryList.h"
#include "../CacheServer.h"

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
        li::mysql_connection<li::mysql_functions_blocking> connection
    )
    {
        if(!mEmail.size()) return;   
        CacheServer::getInstance()->reloadGdtEntry(connection, mEmail);
    }
}