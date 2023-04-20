#ifndef __GDT_CACHE_MODEL_ERROR_H
#define __GDT_CACHE_MODEL_ERROR_H

#include <string>

namespace model {
    class Error
    {
    public:
        Error(const std::string& _message, const std::string& _class, const std::string& _function) noexcept;
        ~Error();

        virtual std::string getCompleteMessage() const;
        virtual std::string getCompleteMessageHtml() const noexcept;
        inline const std::string& getMessage() const {return mMessage;}
        inline const std::string& getClass() const {return mClass;}
        inline const std::string& getFunction() const {return mFunction;}
    protected:
        std::string mMessage;
        std::string mClass;
        std::string mFunction;
    };
}

#endif //__GDT_CACHE_MODEL_ERROR_H