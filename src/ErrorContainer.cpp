#include "ErrorContainer.h"
#include <iostream>

ErrorContainer::ErrorContainer()
{

}

ErrorContainer::~ErrorContainer()
{

}

ErrorContainer* ErrorContainer::getInstance()
{
    static ErrorContainer one;
    return &one;
}

void ErrorContainer::addError(model::Error error) noexcept
{
    try {
        std::lock_guard _lock(mWorkMutex);
        mErrors.push_back(error);
    } catch(std::bad_alloc& ex) {
        std::clog << "[" << __FUNCTION__ << "] no memory left for adding error to ErrorContainer: " 
            << error.getCompleteMessage() << std::endl;
    } catch(std::system_error& ex) {
        std::clog << "[" << __FUNCTION__ << "] unable to lock work mutex, cannot add error to ErrorContainer: " 
            << error.getCompleteMessage() << std::endl;
    } catch(...) {
        std::clog << "[" << __FUNCTION__ << "] unknown exception occurred, cannot add error to ErrorContainer: "
            << error.getCompleteMessage() << std::endl;
    }
}

std::string ErrorContainer::getErrorsHtml() noexcept
{
    try {
        std::lock_guard _lock(mWorkMutex);
        std::string resultHtml = "<ul class='errors'>";
        for(auto error: mErrors){
            resultHtml += "<li>" + error.getCompleteMessageHtml() + "</li>";
        }
        resultHtml +="</ul>";
        return resultHtml;
    } catch(std::system_error& ex) {
        std::clog << "[ErrorContainer::" << __FUNCTION__ << "] unable to lock work mutex, cannot render error messages, " 
            << ex.what() << std::endl;
        return "Cannot lock work mutex!";
    } catch(...) {
        std::clog << "[" << __FUNCTION__ << "] unknown exception occurred, cannot render error messages!"
            << std::endl;
        return "unknown exception!";
    }
}

bool ErrorContainer::hasErrors() noexcept
{
    try {
        std::lock_guard _lock(mWorkMutex);
        return mErrors.size() > 0;
    } catch(std::system_error& ex) {
        std::clog << "[ErrorContainer::" << __FUNCTION__ << "] unable to lock work mutex, " 
            << ex.what() << std::endl;
    } catch(...) {
        std::clog << "[" << __FUNCTION__ << "] unknown exception occurred"
            << std::endl;
    }   
    return true;
}