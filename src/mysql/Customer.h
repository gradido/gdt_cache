#ifndef __GDT_CACHE_MYSQL_CUSTOMER_H
#define __GDT_CACHE_MYSQL_CUSTOMER_H

#include <unordered_map>
#include <lithium_mysql.hh>

namespace mysql {
    namespace Customer {
        //! load all customer from db with one single mysql query
        //! \return map with email as index and customer_id as value
        std::unordered_map<std::string, int> getAll(
            li::mysql_connection<li::mysql_functions_blocking> connection
        );
        
        //! get customer_id for email from db
        //! return nullptr if customer couldn't be found
        int getByEmail(const std::string& email, 
            li::mysql_connection<li::mysql_functions_blocking> connection
        );       
    }
}

#endif //__GDT_CACHE_MYSQL_CUSTOMER_H

