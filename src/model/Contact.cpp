#include "Contact.h"

using namespace rapidjson;

namespace model 
{
    Contact::Contact(const Value& contactJsonObj)
    : mId(0), mParentContactId(0), mApproved(false)
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
        if(contactJsonObj.HasMember("approved")) {
            mApproved = contactJsonObj["approved"].GetBool();
        }
    }

    Contact::Contact(int id, const std::string& email, int parentContactId, bool approved /*= false*/)
    : mId(id), mEmail(email), mParentContactId(parentContactId), mApproved(approved)
    {

    }

    Contact::~Contact()
    {

    }
}
