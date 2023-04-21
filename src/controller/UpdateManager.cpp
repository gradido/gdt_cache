#include "UpdateManager.h"
#include "../ErrorContainer.h"
#include "../lib/Profiler.h"
#include "../model/Config.h"
#include <cstring>

namespace controller
{
    UpdateManager::UpdateManager()
    {

    }

    UpdateManager::~UpdateManager()
    {

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
} // namespace controller
