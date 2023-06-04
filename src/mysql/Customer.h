#ifndef __GDT_CACHE_MYSQL_CUSTOMER_H
#define __GDT_CACHE_MYSQL_CUSTOMER_H

#include <map>
#include "../model/Customer.h"

#include <lithium_mysql.hh>

namespace mysql {
    namespace Customer {
        //! load all customer from db with one single mysql query
        model::CustomersMap getAll(
            li::mysql_connection<li::mysql_functions_blocking> connection
        );
        
        //! load a single customer from db
        //! return nullptr if customer couldn't be found
        model::CustomerPtr getByEmail(const std::string& email, 
            li::mysql_connection<li::mysql_functions_blocking> connection
        );       
    }
}

#endif //__GDT_CACHE_MYSQL_CUSTOMER_H

