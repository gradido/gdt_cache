#ifndef __GDT_CACHE_BACKGROUND_TASKS_UPDATE_GDT_ENTRY_LIST_H
#define __GDT_CACHE_BACKGROUND_TASKS_UPDATE_GDT_ENTRY_LIST_H

#include <string>
#include "../model/GdtEntryList.h"
#include <lithium_mysql.hh>

namespace task
{
    class UpdateGdtEntryList
    {
    public:
        UpdateGdtEntryList(const std::string& email);
        ~UpdateGdtEntryList();

        void run(
            li::mysql_connection<li::mysql_functions_blocking> connection
        );

        const std::string getEmail() const { return mEmail;}
    protected:
        // email of user
        std::string mEmail;
        // only first two and last two character of email, other characters replaced with x
        std::string mName;
    };
}

#endif //__GDT_CACHE_BACKGROUND_TASKS_UPDATE_GDT_ENTRY_LIST_H