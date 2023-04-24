#ifndef __GDT_CACHE_CONTROLLER_CONTACTS_H
#define __GDT_CACHE_CONTROLLER_CONTACTS_H

#include <string>
#include <map>
#include <memory>
#include "../model/Customer.h"

#include <lithium_mysql.hh>

typedef std::map<int, std::shared_ptr<model::Customer>> CustomersMap;

namespace controller
{
    class Contacts
    {
    public:
        Contacts();
        ~Contacts();

        bool loadCustomersFromDb(li::mysql_connection<li::mysql_functions_blocking> connection);
        static std::shared_ptr<model::Customer> loadCustomerFromDb(
            li::mysql_connection<li::mysql_functions_blocking> connection, 
            const std::string& email
        );

        inline const CustomersMap& getCustomers() {return mCustomers;}
        
    protected:
        CustomersMap mCustomers;
    };
}

#endif //__GDT_CACHE_CONTROLLER_CONTACTS_H