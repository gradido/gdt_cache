#include "Contacts.h"
#include "../lib/Profiler.h"
#include <set>

#define CONTACTS_SELECT_STRING "select id, LOWER(TRIM(email)), IFNULL(parent_contact_id, 0) from contacts"

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
        std::string selectString = CONTACTS_SELECT_STRING;
        selectString += " order by id";
        auto rows = connection(selectString);
        
        rows.map([&](int id, std::string email, int parent_contact_id) 
        {            
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

    std::shared_ptr<model::Customer> Contacts::loadCustomerFromDb(
        li::mysql_connection<li::mysql_functions_blocking> connection, 
        const std::string& email
    )
    {
        Profiler timeUsed;
        if(!email.size()) return nullptr;
        Profiler startTime;
        std::shared_ptr<model::Customer> customer; //= std::make_shared<model::Customer>(email);
        std::string selfJoinSelectString = "";
        selfJoinSelectString += "select c1.id, LOWER(TRIM(c1.email)) as email1, LOWER(TRIM(c2.email)) as email2 ";
        selfJoinSelectString += "from contacts c1, contacts c2 ";
        selfJoinSelectString += "WHERE c1.id = c2.parent_contact_id ";
        selfJoinSelectString += "AND (c1.parent_contact_id IS NULL OR c1.parent_contact_id = 0) ";
        selfJoinSelectString += "AND (c1.email LIKE ? OR c2.email LIKE ?)";

        std::string selectString = CONTACTS_SELECT_STRING;
        selectString += " where email LIKE ?";
    
        std::set<std::string> emails;
        int recordCount = 0;
        std::function mapFunction {
            [&](int id, const std::string& email1, const std::string& email2) {
                if(!customer) customer = std::make_shared<model::Customer>(email1);
                if(email2 != email) {
                    // insert only if not already exist
                    emails.insert(email2);
                }
                recordCount++;
            }
        };
        auto query = connection.prepare(selfJoinSelectString);
        auto rows = query(email, email);
        rows.map(mapFunction);
        auto mainEmail = customer->getFirstEmail();
        if(mainEmail.size() && mainEmail != email) {            
            auto rows2 = query(mainEmail, mainEmail);
            rows2.map(mapFunction);
        }        

        for(auto& email: emails) {
            customer->addEmail(email);
        }
        printf("[%s] time used for %d records: %s\n", __FUNCTION__, recordCount, timeUsed.string().data());
        return customer;
    }

}