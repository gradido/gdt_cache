#ifndef __GDT_CACHE_TASK_UPDATE_GDT_ENTRY_LIST_H
#define __GDT_CACHE_TASK_UPDATE_GDT_ENTRY_LIST_H

#include "Task.h"
#include <string>
#include "../model/GdtEntryList.h"


namespace task
{
    class UpdateGdtEntryList: public Task
    {
    public:
        UpdateGdtEntryList(const std::string& email);
        ~UpdateGdtEntryList();

        void run(
            li::mysql_connection<li::mysql_functions_blocking> connection,
            controller::GdtEntries& gdtEntries
        );

        const char* getClass() const {return "UpdateGdtEntryList";}
        const char* getName() const {return mEmail.data();}
    protected:
        std::string mEmail;
    };
}

#endif //__GDT_CACHE_TASK_UPDATE_GDT_ENTRY_LIST_H