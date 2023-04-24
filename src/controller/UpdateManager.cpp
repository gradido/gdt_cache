#include "UpdateManager.h"
#include "../GradidoBlockchainException.h"
#include "../ErrorContainer.h"
#include "../GdtEntriesCache.h"
#include "../lib/Profiler.h"
#include "../model/Config.h"

#include <cstring>
#include <chrono>

using namespace std::chrono_literals;

namespace controller
{
    UpdateManager::UpdateManager()
    : mMainThread(&UpdateManager::updateThread, this), mExitSignal(false)
    {
        
    }   

    UpdateManager::~UpdateManager()
    {
        mMainThreadMutex.lock();
        mExitSignal = true;
        mMainThreadMutex.unlock();
        mMainThreadCondition.notify_all();
        mMainThread.join();
    }

    UpdateManager* UpdateManager::getInstance()
    {
        static UpdateManager one;
        return &one;
    }

    UpdateStatus UpdateManager::updateAllowedIps()
    {
        if(!mUpdateAllowedIpsMutex.try_lock()) {
            return UpdateStatus::RUNNING;
        }        
        //request.ip_address()
        //Profiler timeUsed;
        mAllowedIpsMutex.lock();
        if(mAllowedIps.capacity() != g_Config->allowedHosts.size()) {
            mAllowedIps.resize(g_Config->allowedHosts.size(), "");
        }
        mAllowedIpsMutex.unlock();
        for(auto i = 0; i < g_Config->allowedHosts.size(); i++) {
            updateAllowedIp(g_Config->allowedHosts[i]);
            mAllowedIpsMutex.lock();
            mAllowedIps[i] = mbIp;
            mAllowedIpsMutex.unlock();
        }
        //fprintf(stdout, "[%s] time used: %s\n", __FUNCTION__, timeUsed.string().data());
        mLastUpdateAllowedIps = std::time(nullptr);
        mUpdateAllowedIpsMutex.unlock();
        return UpdateStatus::OK;
    }

    bool UpdateManager::updateAllowedIp(const std::string& url)
    {
        struct addrinfo hints, *res;
        int status;

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        if ((status = getaddrinfo(url.data(), nullptr, &hints, &res)) != 0) {
            std::cerr << "[" << __FUNCTION__ << "] getaddrinfo error: " << gai_strerror(status)
            << " for url: " << url << std::endl;
            return false;
        }
        char ip[INET_ADDRSTRLEN];
        while (res) {
            void *addr;

            struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
            addr = &(ipv4->sin_addr);       

            // Umwandlung der IP-Adresse in einen String
            inet_ntop(res->ai_family, addr, ip, sizeof ip);
            memcpy(mbIp, ip, sizeof ip);
            res = res->ai_next;
            if(res) {
                std::string message = "more than one returned ip for getaddrinfo for url: " + url + ", ip: "; 
                message += ip;
                ErrorContainer::getInstance()->addError({message, "UpdateManager", __FUNCTION__});
            }   
        }

        freeaddrinfo(res); // Aufräumen
        return true;
    }

    void UpdateManager::pushTask(std::shared_ptr<task::Task> task, bool priority/* = false*/)
    {
        mTasksMutex.lock();
        if(priority) {
            mPriorityTasks.push(task);
        } else {
            mTasks.push(task);
        }
        mTasksMutex.unlock();
        mMainThreadCondition.notify_one();
    }

    void UpdateManager::updateThread()
    {
        std::unique_lock _lock(mMainThreadMutex);
        controller::GdtEntries gdtEntriesController;
        auto dbSession = g_Config->database->connect(); 
        gdtEntriesController.loadGlobalModificators(dbSession);

        while(true)
        {            
            // run every 1 minute or if some other thread give us something todo
            mMainThreadCondition.wait_for(_lock, 1min);            
            if(mExitSignal) {return;}
            auto now = std::time(nullptr);

            // should we update allowed ips? (maybe some dynamic ip has changed)
            // update if it is older than 1 hour
            if(now - mLastUpdateAllowedIps > 60 * 60) {
                updateAllowedIps();
            } 
            // check gdt entries for update, every gdt entry older than 4h will be scheduled for update
            // scheduled for update: create update task which can be run at the next step
            GdtEntriesCache::getInstance()->scheduleForUpdates();

            // check if we have some tasks to run
            {
                while(mTasks.size() || mPriorityTasks.size()) 
                {
                    // if exit was signaled we exit and not run anymore tasks
                    if(mExitSignal) {return; }
                    mTasksMutex.lock();
                    std::shared_ptr<task::Task> task;
                    if(mPriorityTasks.size()) {
                        task = mPriorityTasks.front();
                        mPriorityTasks.pop();
                    } else {
                        task = mTasks.front();
                        mTasks.pop();
                    }
                    
                    mTasksMutex.unlock();
                    // while we run task, unlock mutex so other can push new tasks to queue
                    if(task->isReady()) {
                        try {
                            task->run(dbSession, gdtEntriesController);
                        } catch(GradidoBlockchainException& ex) {
                            ErrorContainer::getInstance()->addError({ex.getFullString(), task->getClass(), task->getName()});
                        } catch(std::system_error& ex) {
                            ErrorContainer::getInstance()->addError({ex.what(), task->getClass(), task->getName()});
                        } catch(...) {
                            ErrorContainer::getInstance()->addError({"unspecified exception", task->getClass(), task->getName()});
                        }
                    } else {
                        std::scoped_lock _lockTask(mTasksMutex);
                        mTasks.push(task);
                    }
                }
            }
            
        }
    }
} // namespace controller