#ifndef __GDT_CACHE_CONTROLLER_GDT_ENTRIES_H
#define __GDT_CACHE_CONTROLLER_GDT_ENTRIES_H

#include "../model/GdtEntryList.h"
#include "../model/Customer.h"
#include "../model/GlobalModificator.h"
#include "Contacts.h"

#include <lithium_mysql.hh>

namespace controller 
{
    class GdtEntries
    {
    public:
        GdtEntries();
        ~GdtEntries();

        //! \return true if diffs to previous dataset where found
        bool loadGlobalModificators(li::mysql_connection<li::mysql_functions_blocking> connection);
        std::shared_ptr<model::GdtEntryList> loadGdtEntriesFromDB(
            std::shared_ptr<model::Customer> customer,
            li::mysql_connection<li::mysql_functions_blocking> connection
        );

        // run only one at a time, reuse buffer 
        int checkForMissingGlobalMod(
            std::shared_ptr<model::Customer> customer,
            std::shared_ptr<model::GdtEntryList> gdtEntries,
            li::mysql_connection<li::mysql_functions_blocking> connection
        );

    protected:
        // calculate global mod, insert into gdt entry list 
        void calculateGlobalMod(model::GlobalModificator, std::shared_ptr<model::GdtEntryList> gdtEntries);

        std::vector<model::GlobalModificator> mGlobalMods;
        std::vector<std::vector<uint8_t>> mGlobalModCheckMatrixBuffer;
    };
}

#endif //__GDT_CACHE_CONTROLLER_GDT_ENTRIES_H