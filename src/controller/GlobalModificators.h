#ifndef __GDT_CACHE_CONTROLLER_GDT_ENTRIES_H
#define __GDT_CACHE_CONTROLLER_GDT_ENTRIES_H

#include "../model/GdtEntryList.h"
#include "../model/Customer.h"
#include "../mysql/GlobalModificator.h"

namespace controller 
{
    class GlobalModificators
    {
    public:
        GlobalModificators();
        ~GlobalModificators();

        //! update global modificators with freshly loaded datas from db
        //! \return true if diffs to previous dataset where found
        //! if new data set differ from old one, swap them
        bool updateGlobalModificators(model::GlobalModificators& globalMods);

        //! check if some global modificator is missing in gdt entry list, if one is missing calculate and save it into db
        int checkForMissingGlobalMod(
            model::CustomerPtr customer,
            model::GdtEntryListPtr gdtEntries,
            li::mysql_connection<li::mysql_functions_blocking> connection
        );

    protected:

        model::GlobalModificators mGlobalMods;
        std::vector<std::vector<uint8_t>> mGlobalModCheckMatrixBuffer;
    };
}

#endif //__GDT_CACHE_CONTROLLER_GDT_ENTRIES_H