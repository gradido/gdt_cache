#include "Customer.h"

namespace model 
{
    Customer::Customer(int id, std::vector<std::string> emails)
    : mId(id)
    {
        mEmails.swap(emails);
    }

    Customer::Customer(int id, const std::string& email)
    : mId(id)
    {
        mEmails.push_back(email);
    }

    Customer::Customer(const std::string& email)
    : mId(-1)
    {
        mEmails.push_back(email);
    }

    Customer::~Customer()
    {

    }

    void Customer::addEmail(const std::string& email)
    {
        for(auto existingEmail: mEmails) {
            if(existingEmail == email) return;
        }
        mEmails.push_back(email);
    }

    std::string Customer::getEmailsString() const
    {
        std::string emailsString = "";
        for(auto email: mEmails) {
            if(emailsString.size() > 0) {
                emailsString += ",";
            }
            emailsString += email;
        }
        return emailsString;
    }
}
