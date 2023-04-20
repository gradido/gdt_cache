#include "Error.h"

namespace model 
{
    Error::Error(const std::string& _message, const std::string& _class, const std::string& _function) noexcept
    : mMessage(_message), mClass(_class), mFunction(_function)
    {

    }

    Error::~Error()
    {

    }

    std::string Error::getCompleteMessage() const
    {
        return mMessage + " from " + mClass + "::" + mFunction;
    }
    std::string Error::getCompleteMessageHtml() const noexcept
    {
        return "<message>" + mMessage + "</message> from <origin>" + mClass + "::" + mFunction + "</origin>";
    }
}