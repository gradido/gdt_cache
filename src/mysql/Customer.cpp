
#include "Customer.h"
#include <set>
#include "../logging/Logging.h"

using namespace li;

namespace mysql {
    namespace Customer {

        const std::string SELECT_CONTACTS = "select LOWER(TRIM(email)), customer_id from contacts";
        const std::string SELECT_CONTACT_BY_EMAIL = "select customer_id from contacts where email LIKE LOWER(TRIM(?))";

        std::unordered_map<std::string, int> getAll(
            mysql_connection<mysql_functions_blocking> connection
        )
        {
            std::unordered_map<std::string, int> contactsMap;
            auto rows = connection(SELECT_CONTACTS);

            rows.map([&](std::string email, int customer_id) { 
                contactsMap.insert({email, customer_id});
            });
            
            return std::move(contactsMap);
        }

        int getByEmail(const std::string& email,
           mysql_connection<mysql_functions_blocking> connection
        )
        {
            if(!email.size()) return 0;  
            auto stmt = connection.prepare(SELECT_CONTACT_BY_EMAIL);
            auto optional = stmt(email).read_optional<int>();
            if(optional.has_value()) {
                return optional.value();
            } 
            return 0;
        }
    }
}
