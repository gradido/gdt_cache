#include "lithium_symbols.h"
#include "CacheServer.h"
#include "logging/Logging.h"
#include "mysql/Customer.h"
#include "mysql/GdtEntry.h"
#include "model/Config.h"
#include "view/GdtEntryList.h"
#include "utils/Profiler.h"
#include "background_tasks/UpdateGdtEntryList.h"

#include <lithium_http_client.hh>
#include <lithium_http_server.hh>
#include <lithium_metamap.hh>
#include <lithium_json.hh>

#include <thread>
#include <sstream>
#include <chrono>
#include <regex>
#include <boost/lexical_cast.hpp>
#include <typeinfo>

using namespace li;
using namespace std::chrono_literals;

CacheServer::CacheServer()
: mLastUpdateGdtEntries(0), mLastUpdateAllowedIps(0)
{
}

CacheServer::~CacheServer()
{
}

CacheServer *CacheServer::getInstance()
{
    static CacheServer one;
    return &one;
}

void CacheServer::initializeFromDb()
{
    auto dbSession = g_Config->database->connect();
    loadFromDb(dbSession);
    mLastUpdateGdtEntries = time(nullptr);
    mUpdateCacheWorker.startThread();
}

std::string CacheServer::listPerEmailApi(
    const std::string &email,
    int page /* = 1*/,
    int count /*= 25*/,
    view::OrderDirections order /* = view::OrderDirections::ASC*/
    )
{
    Profiler timeUsed;
    try {
        std::unique_lock _lock(mGdtEntriesAccessMutex, 100ms);
        if(!_lock) {
            throw http_error(504, "timeout mutex 100ms");
        }
        auto emailCustomerIdsIt = mEmailCustomerIds.find(email);
        model::GdtEntryListPtr gdtEntryList;
        if(emailCustomerIdsIt != mEmailCustomerIds.end()) {
            auto it = mGdtEntriesByEmails.find(emailCustomerIdsIt->second);
            if(it != mGdtEntriesByEmails.end()) {
                gdtEntryList = it->second;
            }
        }
        
        if(!gdtEntryList) {      
            if(std::regex_match(email, g_EmailValidPattern)) {
                mUpdateCacheWorker.pushTask(std::make_shared<task::UpdateGdtEntryList>(email));
            }
            return "{\"state\":\"success\",\"count\":0,\"gdtEntries\":[],\"gdtSum\":0,\"timeUsed\":"
                        +std::to_string(static_cast<float>(timeUsed.seconds()))+"}";
        } else {
            if(gdtEntryList->canUpdate()) {
                mUpdateCacheWorker.pushTask(std::make_shared<task::UpdateGdtEntryList>(email));
            }
            std::string resultJsonString = view::GdtEntryList::toJsonString(*gdtEntryList, timeUsed, page, count, order);
            _lock.unlock();
            
            // 50 * 1024 = hardcoded output buffer size from lithium
            if(resultJsonString.size() > 50 * 1024) {
                auto errorMetaMap = mmm(
                    s::state = "error",
                    s::error = "output to big, please retry with less count",
                    s::details = mmm(
                        s::count = count,
                        s::overflow = resultJsonString.size()-50 * 1024
                    )
                );                
                return json_encode(errorMetaMap);
            }
            return resultJsonString;
        }

    } catch(std::runtime_error& ex) {
        std::string message = "runtime exception: ";
        message += ex.what();
        LOG_ERROR(message);
        throw http_error::internal_server_error("deadlock");
    } catch(std::exception& ex) {
        std::string message = "exception: ";
        message += ex.what();
        LOG_ERROR(message);
        throw http_error::internal_server_error("unknown");
    } catch(...) {
        LOG_ERROR("unknown exception");
        throw http_error::internal_server_error("unknown 2");
    }
}

std::string CacheServer::sumPerEmailApi(const std::string &email)
{
    Profiler timeUsed;
    try {
        std::unique_lock _lock(mGdtEntriesAccessMutex, 100ms);
        if(!_lock) {
            throw http_error(504, "timeout mutex 100ms");
        }
        auto emailCustomerIdsIt = mEmailCustomerIds.find(email);
        model::GdtEntryListPtr gdtEntryList;
        if(emailCustomerIdsIt != mEmailCustomerIds.end()) {
            auto it = mGdtEntriesByEmails.find(emailCustomerIdsIt->second);
            if(it != mGdtEntriesByEmails.end()) {
                gdtEntryList = it->second;
            }
        }
        
        if(!gdtEntryList) {
            if(std::regex_match(email, g_EmailValidPattern)) {
                mUpdateCacheWorker.pushTask(std::make_shared<task::UpdateGdtEntryList>(email));
            }
            return "{\"state\":\"success\",\"sum\":0,\"time\":"+std::to_string((static_cast<float>(timeUsed.seconds())))+"}";
        } else {
            if(gdtEntryList->canUpdate()) {
                mUpdateCacheWorker.pushTask(std::make_shared<task::UpdateGdtEntryList>(email));
            }
            std::ostringstream out;
            out << "{\"state\":\"success\",\"sum\":" 
                << view::stringWithoutTrailingZeros(gdtEntryList->getGdtSum())
                << ",\"time\":" << timeUsed.seconds() << "}";
            return out.str();
        }
    } catch(std::runtime_error& ex) {
        std::string message = "runtime exception: ";
        message += ex.what();
        LOG_ERROR(message);
        throw http_error::internal_server_error("deadlock");
    } catch(std::exception& ex) {
        std::string message = "exception: ";
        message += ex.what();
        LOG_ERROR(message);
        throw http_error::internal_server_error("unknown");
    } catch(http_error& ex) {
        std::string message = "http_error: ";
        message += ex.what();
        LOG_ERROR(message);
        throw ex;
    } catch(...) {
        LOG_ERROR("unknown exception");
        throw http_error::internal_server_error("unknown 2");
    } 
}

CacheServer::UpdateStatus CacheServer::reloadCacheAfterTimeout(mysql_connection<mysql_functions_blocking> connection, bool ignoreTimeout/* = false*/)
noexcept
{
    auto now = time(nullptr);
    if(now - mLastUpdateGdtEntries > g_Config->maxCacheTimeout || ignoreTimeout) {
        try {
            std::unique_lock<std::mutex> _lock(mGdtEntriesUpdateMutex, std::try_to_lock);
            if(!_lock.owns_lock()) {
                return UpdateStatus::RUNNING;
            }   
            loadFromDb(connection);
            return UpdateStatus::OK;
        } catch(const boost::bad_lexical_cast& e) {
            std::string message = "boost bad lexical cast with source type: ";
            auto& sourceType = typeid(e.source_type());
            auto& targetType = typeid(e.target_type());
            message += sourceType.name();
            message += ", and with target type: ";
            message += targetType.name();
            LOG_ERROR(message);

        } catch(std::runtime_error& ex) {
            std::string message = "runtime error: ";
            message += ex.what();
            LOG_ERROR(message);
        } catch(std::exception& ex) {
            std::string message = "library exception: ";
            message += ex.what();
            LOG_ERROR(message);
        }
        catch(...) {
            LOG_ERROR("unknown exception");
        } 
        mLastUpdateGdtEntries = now;
    }
    return UpdateStatus::SKIPPED;
}

CacheServer::UpdateStatus CacheServer::reloadGdtEntry(mysql_connection<mysql_functions_blocking> connection, const std::string& email) noexcept
{
    try {
        Profiler timeUsed;
        std::lock_guard _lock(mGdtEntriesAccessMutex);
        auto emailCustomerIdsIt = mEmailCustomerIds.find(email);
        if(emailCustomerIdsIt != mEmailCustomerIds.end()) {
            auto it = mGdtEntriesByEmails.find(emailCustomerIdsIt->second);
            if(it != mGdtEntriesByEmails.end() && !it->second->canUpdate()) {
                return UpdateStatus::SKIPPED;
            }
        }
        // get customer id for email fresh from db
        auto customerId = mysql::Customer::getByEmail(email, connection);
        if(!customerId) {
            return UpdateStatus::OK;
        }
        // update or insert customer id in email customer map
        if(emailCustomerIdsIt == mEmailCustomerIds.end()) {
            mEmailCustomerIds.insert({email, customerId});
        } else {
            emailCustomerIdsIt->second = customerId;
        }        
        
        // read gdt entry list for customer fresh from db
        auto gdtEntriesList = mysql::GdtEntry::getByCustomer(customerId, connection);
        // update gdt entry list
        auto it = mGdtEntriesByEmails.find(customerId);
        if(it == mGdtEntriesByEmails.end()) {
            mGdtEntriesByEmails.insert({customerId, gdtEntriesList});
        } else {
            it->second = gdtEntriesList;
        }

        return UpdateStatus::OK;
    }
    catch(std::runtime_error& ex) {
        std::string message = "runtime error: ";
        message += ex.what();
        LOG_ERROR(message);
    } catch(std::exception& ex) {
        std::string message = "library exception: ";
        message += ex.what();
        LOG_ERROR(message);
    }
    catch(...) {
        LOG_ERROR("unknown exception");
    } 
    return UpdateStatus::ERROR;
}

CacheServer::UpdateStatus CacheServer::updateAllowedIps(bool ignoreTimeout) noexcept
{
    if(!g_Config->ipWhiteListing) return UpdateStatus::OK;

    auto now = time(nullptr);
    // update ips at least one per hour
    if(now - mLastUpdateAllowedIps > 60 * 60 || ignoreTimeout) 
    {
        try {
            std::unique_lock<std::mutex> _lock(mGdtEntriesUpdateMutex, std::try_to_lock);
            if(!_lock.owns_lock()) {
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
        } catch(std::runtime_error& ex) {
            std::string message = "runtime error: ";
            message += ex.what();
            LOG_ERROR(message);
        } catch(std::exception& ex) {
            std::string message = "library exception: ";
            message += ex.what();
            LOG_ERROR(message);
        }
        catch(...) {
            LOG_ERROR("unknown exception");
        } 
    }
    return UpdateStatus::SKIPPED;
}

void CacheServer::loadFromDb(li::mysql_connection<li::mysql_functions_blocking> connection)
{
    try {
        mEmailCustomerIds = mysql::Customer::getAll(connection);
    }
    catch(const boost::bad_lexical_cast& e) {
        LOG_ERROR("boost bad lexical cast by calling mysql::Customer::getAll");
        throw;
    }

    // check that our email regex pattern is matching all emails from db
    // it is used to filter out malicious requests
    for (auto customer : mEmailCustomerIds) {
        auto email = customer.first;
        // test regex pattern
        if(!std::regex_match(email, g_EmailValidPattern)) {
            std::string message = email;
            message += " matched false with email valid pattern, please update pattern!";
            LOG_ERROR(message);
        }                    
    }

    Profiler timeUsed;
    model::EmailGdtEntriesListMap gdtEntriesByEmail;
    try {
        gdtEntriesByEmail = mysql::GdtEntry::getAll(connection);
    } catch(const boost::bad_lexical_cast& e) {
        LOG_ERROR("boost bad lexical cast by calling mysql::GdtEntry::getAll");
        throw;
    }
    // lock gdt entries only for move results and delete old
    {
        std::lock_guard gdtEntriesAccessLock(mGdtEntriesAccessMutex);    
        mGdtEntriesByEmails = std::move(gdtEntriesByEmail);
    }
    // printf("[%s] time used for loading all gdt entries from db into memory: %s\n", __FUNCTION__, timeUsed.string().data());
    timeUsed.reset();    
}

bool CacheServer::updateAllowedIp(const std::string& url)
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
            LOG_ERROR(message);
        }   
    }

    freeaddrinfo(res); // Aufräumen
    return true;
}
