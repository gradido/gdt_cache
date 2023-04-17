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
}
