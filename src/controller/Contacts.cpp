#include "Contacts.h"
#include "../lib/Profiler.h"
#include "../ErrorContainer.h"
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
        
        int recordCount = 0;
        // for checking if email already exist
        std::set<std::string> emailSet;
        bool emailIsMain = true;
        std::function mapFunction {
            [&](int id, const std::string& _email, int parent_contact_id) 
            {
                if(!customer) {
                    int mainId = id;
                    if(parent_contact_id) {
                        mainId = parent_contact_id;
                        emailIsMain = false;
                    }
                    customer = std::make_shared<model::Customer>(mainId, _email);
                    emailSet.insert(_email);
                } else {
                    auto it = emailSet.insert(_email);
                    // email could be added so it is unique
                    if(it.second) {
                        customer->addEmail(_email);
                    }
                }                
                recordCount++;
            }
        };
        std::string selectMainContact = CONTACTS_SELECT_STRING;
        selectMainContact += " where LOWER(TRIM(email)) LIKE LOWER(TRIM(?)) order by id ASC";
        connection.prepare(selectMainContact)(email).map(mapFunction);        
        if(!customer) {
            std::string message = "contact ";
            message = email.substr(0, 3) + "... in db not found";
            ErrorContainer::getInstance()->addError({message, "controller::Contacts", __FUNCTION__});
            return std::make_shared<model::Customer>(email);
        }

        std::string selectAdditionalContacts = CONTACTS_SELECT_STRING;
        selectAdditionalContacts += " where parent_contact_id = ?";
        if(!emailIsMain) {
            selectAdditionalContacts += " OR id = ?";
            connection.prepare(selectAdditionalContacts)(customer->getId(), customer->getId()).map(mapFunction);
        } else {
            connection.prepare(selectAdditionalContacts)(customer->getId()).map(mapFunction);
        }
        //printf("[%s] time used for %d records: %s\n", __FUNCTION__, recordCount, timeUsed.string().data());
        return customer;
    }

}