#include "ExternRequestsManager.h"

namespace controller 
{
    ExternRequestsManager::ExternRequestsManager()
    : ThreadLoop(120)
    {

    }

    ExternRequestsManager::~ExternRequestsManager()
    {

    }

    ExternRequestsManager* ExternRequestsManager::getInstance()
    {
        static ExternRequestsManager one;
        return &one;
    }

    void ExternRequestsManager::run()
    {
        std::unique_lock _lock(mRequestsMutex);
        while(mRequests.size()) {
            auto request = mRequests.front();
            mRequests.pop();
            request->run();
        }
    }

    void ExternRequestsManager::addTask(std::shared_ptr<task::ExternRequest> request)
    {
        mRequestsMutex.lock();
        mRequests.push(request);
        mRequestsMutex.unlock();
        notify();
    }
}