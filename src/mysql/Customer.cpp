
#include "Customer.h"
#include <set>
#include "../logging/Logging.h"

using namespace li;

namespace mysql {
    namespace Customer {

        const std::string CONTACTS_SELECT_STRING("select id, LOWER(TRIM(email)), IFNULL(parent_contact_id, 0) from contacts");

        model::CustomersMap getAll(
            mysql_connection<mysql_functions_blocking> connection
        )
        {
            model::CustomersMap customers;
            std::string selectString = CONTACTS_SELECT_STRING;
            selectString += " order by id";
            auto rows = connection(selectString);
            int records = 0;
            rows.map([&](int id, std::string email, int parent_contact_id) 
            {            
                int searchId = id;            
                if(parent_contact_id) {
                    searchId = parent_contact_id;
                }
                auto it = customers.find(searchId);
                std::shared_ptr<model::Customer> customer;
                if(it == customers.end()) {
                    customer = std::make_shared<model::Customer>(searchId, email);
                    it = customers.insert({searchId, customer}).first;
                } else {
                    it->second->addEmail(email);
                }
                if(!parent_contact_id) {
                    it->second->setMainEmail(email);
                }
                records++;
            });
            
            return std::move(customers);
        }

        model::CustomerPtr getByEmail(const std::string& email,
            mysql_connection<mysql_functions_blocking> connection
        )
        {
            if(!email.size()) return nullptr;  
            model::CustomerPtr customer;
            
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
                    if(!parent_contact_id && customer) {
                        customer->setMainEmail(_email);
                    }               
                }
            };
            std::string selectMainContact = CONTACTS_SELECT_STRING;
            selectMainContact += " where LOWER(TRIM(email)) LIKE LOWER(TRIM(?)) order by id ASC";
            connection.prepare(selectMainContact)(email).map(mapFunction);        
            if(!customer) {
                std::string message = "contact ";
                message = email.substr(0, 3) + "... in db not found";
                LOG_INFORMATION(message);
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
            return std::move(customer);
        }
    }
}
