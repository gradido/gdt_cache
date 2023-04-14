#ifndef __GDT_CACHE_CONTROLLER_CONTACTS_H
#define __GDT_CACHE_CONTROLLER_CONTACTS_H

#include <string>
#include <map>

#include "../model/Contact.h"

namespace controller
{
    class Contacts
    {
    public:
        Contacts();
        ~Contacts();

        /*! \params jsonString
         * expected format
         * [{"id":1,"email":"email@email.com","parent_contact_id":0}]
        */
        bool loadFromJsonString(const std::string& jsonString);
    protected:
    };
}

#endif //__GDT_CACHE_CONTROLLER_CONTACTS_H