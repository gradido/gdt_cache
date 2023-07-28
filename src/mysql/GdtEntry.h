#ifndef __GDT_CACHE_MYSQL_GDT_ENTRY_H
#define __GDT_CACHE_MYSQL_GDT_ENTRY_H

#include "../model/GdtEntryList.h"

#include <lithium_mysql.hh>

namespace mysql {
    namespace GdtEntry {
        //! load all gdt entries from one customer from db
        model::GdtEntryListPtr getByCustomer(
            int customerId,
            li::mysql_connection<li::mysql_functions_blocking> connection
        );

        //! read all gdt entries from db and group by customer, indexed by every email the customer has
        //! \param customers customer map for sorting the results by customer
        model::EmailGdtEntriesListMap getAll(
            li::mysql_connection<li::mysql_functions_blocking> connection
        );
    }
}

#endif //__GDT_CACHE_MYSQL_GDT_ENTRY_H