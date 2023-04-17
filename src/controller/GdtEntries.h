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

        void loadGlobalModificators(li::mysql_connection<li::mysql_functions_blocking> connection);

        // run only one at a time, reuse buffer 
        bool checkForMissingGlobalMod(
            std::shared_ptr<model::Customer> customer,
            std::shared_ptr<model::GdtEntryList> gdtEntries
        );

    protected:
        std::vector<model::GlobalModificator> mGlobalMods;
        std::vector<std::vector<uint8_t>> mGlobalModCheckMatrixBuffer;
    };
}

#endif //__GDT_CACHE_CONTROLLER_GDT_ENTRIES_H