#ifndef __GDT_CACHE_CONTROLLER_EXTERN_REQUEST_MANAGER_H
#define __GDT_CACHE_CONTROLLER_EXTERN_REQUEST_MANAGER_H

#include "../lib/ThreadLoop.h"
#include "../task/ExternRequest.h"
#include <queue>

namespace controller {
    /*!
     * @author einhornimmond 
     * @date 27.04.2023 
     * run extern requests in extra thread  
    */
    class ExternRequestsManager: public ThreadLoop
    {
    public: 
        ~ExternRequestsManager();
        static ExternRequestsManager* getInstance();

        void run();
        const char* getName() const {return "Singleton";}
        const char* getType() const {return "ExternRequestsManager";}

        void addTask(std::shared_ptr<task::ExternRequest> task);
    protected:
        ExternRequestsManager();
        std::queue<std::shared_ptr<task::ExternRequest>> mRequests;
        std::mutex mRequestsMutex;
    };
}


#endif //__GDT_CACHE_CONTROLLER_EXTERN_REQUEST_MANAGER_H