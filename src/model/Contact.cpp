#include "Contact.h"

using namespace rapidjson;

namespace model 
{
    Contact::Contact(const Value& contactJsonObj)
    : mId(0), mParentContactId(0)
    {
        if(contactJsonObj.HasMember("id")) {
            mId = contactJsonObj["id"].GetInt();
        }
        if(contactJsonObj.HasMember("email")) {
            mEmail = contactJsonObj["email"].GetString();
        }
        if(contactJsonObj.HasMember("parent_contact_id")) {
            mParentContactId = contactJsonObj["parent_contact_id"].GetInt();
        }
    }

    Contact::~Contact()
    {

    }
}
