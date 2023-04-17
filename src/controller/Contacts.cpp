#include "Contacts.h"
#include "../lib/Profiler.h"

namespace controller 
{
    Contacts::Contacts()
    {

    }

    Contacts::~Contacts()
    {

    }

    bool Contacts::loadCustomersFromDb(li::mysql_connection<li::mysql_functions_blocking> connection)
    {
        Profiler startTime;
        auto rows = connection("select id, LOWER(TRIM(email)), IFNULL(parent_contact_id, 0) from contacts order by id");
        
        rows.map([&](int id, std::string email, int parent_contact_id) {
            
            int searchId = id;
            
            if(parent_contact_id) {
                searchId = parent_contact_id;
            }
            auto it = mCustomers.find(searchId);
            std::shared_ptr<model::Customer> customer;
            if(it == mCustomers.end()) {
                customer = std::make_shared<model::Customer>(searchId, email);
                mCustomers.insert({id, customer});
            } else {
                it->second->addEmail(email);
            }
        });
        printf("[%s] %s used time for reading all contacts from db\n", __FUNCTION__, startTime.string().data());        
        
        return true;
    }

}