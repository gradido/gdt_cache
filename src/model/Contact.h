#ifndef __GDT_CACHE_MODEL_CONTACT_H
#define __GDT_CACHE_MODEL_CONTACT_H

#include "rapidjson/document.h"

namespace model 
{
    class Contact
    {
    public:
        //! \param contactJsonObj: {"id":1,"email":"email@email.com","parent_contact_id":0}
        Contact(const rapidjson::Value& contactJsonObj);
        Contact(int id, const std::string& email, int parentContactId, bool approved = false);
        ~Contact();
    protected:
        int mId;
        std::string mEmail;
        int mParentContactId;
        bool mApproved;

    };
}

#endif //__GDT_CACHE_MODEL_CONTACT_H