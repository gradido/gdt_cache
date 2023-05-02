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
        auto hostsCount = g_Config->allowedHosts.size();
        if(mAllowedIps.capacity() != hostsCount*2) {
            mAllowedIps.resize(hostsCount*2, "");
        }
        mAllowedIpsMutex.unlock();
        for(auto i = 0; i < hostsCount; i++) {
            if(updateAllowedIp(g_Config->allowedHosts[i])) {
                mAllowedIpsMutex.lock();
                mAllowedIps[i] = mbIp;
                mAllowedIps[i+hostsCount] = mbIpv6;
                mAllowedIpsMutex.unlock();
            }
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
        memset(mbIp, 0, sizeof mbIp);
        memset(mbIpv6, 0, sizeof mbIpv6);
        
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        if ((status = getaddrinfo(url.data(), nullptr, &hints, &res)) != 0) {
            std::cerr << "[" << __FUNCTION__ << "] getaddrinfo error: " << gai_strerror(status)
            << " for url: " << url << std::endl;
            return false;
        }
        int resultingAddressCount = 0;
            
        while (res) {
            void *addr;

            if (res->ai_family == AF_INET) { // IPv4
                struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
                addr = &(ipv4->sin_addr);
                inet_ntop(res->ai_family, addr, mbIp, sizeof mbIp);
                resultingAddressCount++;
            } else { // IPv6
                struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)res->ai_addr;
                addr = &(ipv6->sin6_addr);
                inet_ntop(res->ai_family, addr, mbIpv6, sizeof mbIpv6);
                resultingAddressCount++;
            }
            
            res = res->ai_next;
            if(resultingAddressCount > 2) {
                std::string message = "more than one returned ip for getaddrinfo for url: " + url + ", ip: "; 
                message += mbIp;
                message += ", ipv6: ";
                message += mbIpv6;
                ErrorContainer::getInstance()->addError({message, "UpdateManager", __FUNCTION__});
            }   
        }

        freeaddrinfo(res); // Aufräumen
        return true;
    }

    void UpdateManager::pushTask(std::shared_ptr<task::UpdateGdtEntryList> task, bool priority/* = false*/)
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
        std::time_t lastUpdateGlobalMods = time(nullptr);
        auto gc = GdtEntriesCache::getInstance();

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
            bool globalModUpdates = false;
            // update global mods every 1 minutes
            if(now - lastUpdateGlobalMods > 60 * 1) {
                globalModUpdates = gdtEntriesController.loadGlobalModificators(dbSession);
                lastUpdateGlobalMods = now;
            }
            // reload all gdt entries and contacts every maxCacheTimeout seconds (default 4h)
            // if global mod had an update, forced reload
            gc->reloadCacheAfterTimeout(dbSession, globalModUpdates);

            // check if we have some tasks to run
            // on every request for a customer, a task will created to reload his gdt entries, after the call was answered
            {
                Profiler timeUsedTasks;
                int runTaskCount = 0;
                while(mTasks.size() || mPriorityTasks.size()) 
                {
                    // if exit was signaled we exit and not run anymore tasks
                    if(mExitSignal) {return; }
                    std::unique_lock _lockTask(mTasksMutex);
                    
                    std::shared_ptr<task::UpdateGdtEntryList> task;
                    if(mPriorityTasks.size()) {
                        task = mPriorityTasks.front();
                        mPriorityTasks.pop();
                    } else {
                        task = mTasks.front();
                        mTasks.pop();
                        if(!gc->shouldUpdateGdtEntryList(task->getEmail())) {
                            continue;
                        }                        
                    }
                    
                    _lockTask.unlock();
                    // while we run task, unlock mutex so other can push new tasks to queue
                    if(task->isReady()) {
                        try {
                            task->run(dbSession, gdtEntriesController);
                            runTaskCount++;
                            if(ErrorContainer::getInstance()->hasErrors()) {
                                std::clog << "exit update Manager thread" << std::endl;
                                return;
                            }
                        } catch(GradidoBlockchainException& ex) {
                            ErrorContainer::getInstance()->addError({ex.getFullString(), task->getClass(), task->getName()});
                        } catch(std::runtime_error& ex) {
                            ErrorContainer::getInstance()->addError({ex.what(), task->getClass(), task->getName()});
                        } catch(...) {
                            ErrorContainer::getInstance()->addError({"unspecified exception", task->getClass(), task->getName()});
                        }
                    } else {
                        _lockTask.lock();
                        mTasks.push(task);
                    }
                }
                if(runTaskCount > 5) {
                    printf("[UpdateManager::updateThread] run %d tasks in %s\n", runTaskCount, timeUsedTasks.string().data());
                }
            }
            
        }
    }
} // namespace controller