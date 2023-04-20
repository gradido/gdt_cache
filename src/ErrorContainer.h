#ifndef __GDT_CACHE_ERROR_CONTAINER_H
#define __GDT_CACHE_ERROR_CONTAINER_H

#include "model/Error.h"
#include <mutex>
#include <list>

class ErrorContainer
{
public:
    ~ErrorContainer();
    static ErrorContainer* getInstance();
    void addError(model::Error error) noexcept;

    std::string getErrorsHtml() noexcept;
    bool hasErrors() noexcept;

protected:
    ErrorContainer();
    std::list<model::Error> mErrors;
    std::recursive_mutex mWorkMutex;
};

#endif //__GDT_CACHE_ERROR_CONTAINER_H