#ifndef __GDT_CACHE_MODEL_CUSTOMER_H
#define __GDT_CACHE_MODEL_CUSTOMER_H

#include <string>
#include <vector>


namespace model 
{
    class Customer
    {
    public:
        //! swap vector, will be left empty afterwards
        Customer(int id, std::vector<std::string> emails);
        Customer(int id, const std::string& email);
        ~Customer();

        void addEmail(const std::string& email);
        inline const std::vector<std::string>& getEmails() {return mEmails;}
        inline int getId() {return mId;}
    protected:
        int mId;
        std::vector<std::string> mEmails;
    };
}

#endif //__GDT_CACHE_MODEL_CUSTOMER_H