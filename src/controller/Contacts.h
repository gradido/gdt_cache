#ifndef __GDT_CACHE_CONTROLLER_CONTACTS_H
#define __GDT_CACHE_CONTROLLER_CONTACTS_H

#include <string>
#include <map>

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
        //std::map<int, model::Contact
    };
}

#endif //__GDT_CACHE_CONTROLLER_CONTACTS_H