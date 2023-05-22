#ifndef __GDT_CACHE_MODEL_CUSTOMER_H
#define __GDT_CACHE_MODEL_CUSTOMER_H

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace model 
{
    class Customer
    {
    public:
        //! swap vector, will be left empty afterwards
        Customer(int id, std::vector<std::string> emails);
        Customer(int id, const std::string& email);
        Customer(const std::string& email);
        ~Customer();

        void addEmail(const std::string& email);
        inline const std::vector<std::string>& getEmails() {return mEmails;}
        std::string getEmailsString() const;
        inline std::string getFirstEmail() { if(!mEmails.size()) return ""; return mEmails[0];}
        inline int getId() {return mId;}
        inline void setMainEmail(const std::string& email) {mMainEmail = email;}
        inline const std::string& getMainEmail() const {return mMainEmail;}
    protected:
        int mId;
        std::vector<std::string> mEmails;
        std::string mMainEmail;
    };

    typedef std::map<int, std::shared_ptr<Customer>> CustomersMap;
    typedef std::shared_ptr<Customer> CustomerPtr;
}

#endif //__GDT_CACHE_MODEL_CUSTOMER_H