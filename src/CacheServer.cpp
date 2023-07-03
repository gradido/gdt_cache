#include "main.h"
#include "CacheServer.h"
#include "GradidoBlockchainException.h"
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

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <thread>
#include <sstream>
#include <chrono>
#include <regex>
#include <boost/lexical_cast.hpp>
#include <typeinfo>

using namespace li;
using namespace rapidjson;
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

bool CacheServer::initializeFromPhp()
{
    Profiler timeUsed;
    // Get list of contacts
    std::string requestUrl = "https://" + g_Config->gdtServerUrl;
    requestUrl += "/contacts/apiGetCustomerWithGdtEntries";

    Document bodyJson;
    try
    {
        auto res = http_get(requestUrl);

        printf("[%s] used time for contacts/apiGetCustomerWithGdtEntries request: %s\n", __FUNCTION__, timeUsed.string().data());
        printf("size: %ld kByte\n", res.body.size() / 1024);

        timeUsed.reset();

        bodyJson.Parse(res.body.data());

        printf("[%s] used time parsing to json: %s\n", __FUNCTION__, timeUsed.string().data());
        timeUsed.reset();
    }
    catch (std::exception &ex)
    {
        std::string message = "call to contacts/apiGetCustomerWithGdtEntries throw exception: ";
        message += ex.what();
        LOG_ERROR(message);
        requestUrl = "https://" + g_Config->gdtServerUrl + "/contacts/apiIndex";
        auto res = http_get(requestUrl);

        printf("[%s] used time for contacts/apiIndex request: %s\n", __FUNCTION__, timeUsed.string().data());
        printf("size: %ld kByte\n", res.body.size() / 1024);

        timeUsed.reset();

        bodyJson.Parse(res.body.data());

        printf("[%s] used time parsing to json: %s\n", __FUNCTION__, timeUsed.string().data());
        timeUsed.reset();
    }

    if (!bodyJson.HasMember("emails"))
    {
        LOG_ERROR("missing emails member in apiGetCustomerWithGdtEntries result");
        return false;
    }
    if (bodyJson.HasMember("missingEmails") && bodyJson["missingEmails"].GetArray().Size() > 0)
    {
        LOG_ERROR("missing emails, please check gdt server code");
        for (auto &missingEmail : bodyJson["missingEmails"].GetArray())
        {
            fprintf(stderr, "%s", missingEmail.GetString());
        }
        fprintf(stderr, "\n");
        return false;
    }
    auto contactsCount = bodyJson["emails"].GetArray().Size();
    int currentContact = 1;
    printf("[%s] contacts count: %d\n", __FUNCTION__, bodyJson["emails"].GetArray().Size());

    Profiler breakTimer;
    for (auto &contact : bodyJson["emails"].GetArray())
    {
        Profiler timePerEmail;
        // printf("%s ", contact["email"].GetString());
        std::string email = contact.GetString();
        std::set<std::string> emails;
        emails.insert(email);
        std::shared_ptr<model::GdtEntryList> gdtEntries = std::make_shared<model::GdtEntryList>();
        int page = 1;
        do
        {
            Profiler timePerCall;
            try {
                auto json = _listPerEmailApi(email, page, 50, view::OrderDirections::ASC);
                page++;
                auto updatedCount = gdtEntries->addGdtEntry(json);
                printf("[%s] time for %d added gdt entries: %s\n", __FUNCTION__, updatedCount, timePerCall.string().data());
            }
            catch (RapidjsonParseErrorException &ex)
            {
                LOG_ERROR(ex.getFullString());
                return false;
            }
        } while (gdtEntries->getTotalCount() > gdtEntries->getGdtEntriesCount());
        printf("[%s] used time: %s for email: %s %d/%d\n",
               __FUNCTION__, timePerEmail.string().data(), email.data(), currentContact, contactsCount);

        mGdtEntriesAccessMutex.lock();
        for (auto email : emails) {
            mGdtEntriesByEmails.insert({email, gdtEntries});
        }
        mGdtEntriesAccessMutex.unlock();

        if (breakTimer.seconds() > 1.0) {
            breakTimer.reset();
            std::this_thread::sleep_for(100ms);
        }
        currentContact++;
    }
    printf("[%s] used time collecting gdt entries: %s\n", __FUNCTION__, timeUsed.string().data());

    mUpdateCacheWorker.startThread();
    return true;
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
        std::unique_lock _lock(mGdtEntriesAccessMutex, 10ms);
        if(!_lock) {
            throw http_error(504, "timeout mutex 10ms");
        }
        auto it = mGdtEntriesByEmails.find(email);
        if(it == mGdtEntriesByEmails.end()) {            
            if(std::regex_match(email, g_EmailValidPattern)) {
                mUpdateCacheWorker.pushTask(std::make_shared<task::UpdateGdtEntryList>(email));
            }
            return "{\"state\":\"success\",\"count\":0,\"gdtEntries\":[],\"gdtSum\":0,\"timeUsed\":"
                        +std::to_string(static_cast<float>(timeUsed.seconds()))+"}";
        } else {
            if(it->second->canUpdate()) {
                mUpdateCacheWorker.pushTask(std::make_shared<task::UpdateGdtEntryList>(email));
            }
            std::string resultJsonString = view::GdtEntryList::toJsonString(*it->second, timeUsed, page, count, order);
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

    } catch(std::system_error& ex) {
        std::string message = "system exception: ";
        message += ex.what();
        LOG_ERROR(message);
        throw http_error::internal_server_error("deadlock");
    } catch(...) {
        LOG_ERROR("unknown exception");
        throw http_error::internal_server_error("unknown");
    }
}

std::string CacheServer::sumPerEmailApi(const std::string &email)
{
    Profiler timeUsed;
    try {
        std::unique_lock _lock(mGdtEntriesAccessMutex, 10ms);
        if(!_lock) {
            throw http_error(504, "timeout mutex 10ms");
        }
        auto it = mGdtEntriesByEmails.find(email);
        if(it == mGdtEntriesByEmails.end()) {
            if(std::regex_match(email, g_EmailValidPattern)) {
                mUpdateCacheWorker.pushTask(std::make_shared<task::UpdateGdtEntryList>(email));
            }
            return "{\"state\":\"success\",\"sum\":0,\"count\":0,\"time\":"+std::to_string(timeUsed.seconds())+"}";
        } else {
            if(it->second->canUpdate()) {
                mUpdateCacheWorker.pushTask(std::make_shared<task::UpdateGdtEntryList>(email));
            }
            std::ostringstream out;
            out.precision(2);
            out << "{\"state\":\"success\",\"sum\":" 
                <<  it->second->getGdtSum()
                << ",\"count\":"
                << it->second->getTotalCount()
                << ",\"time\":" << timeUsed.seconds() << "}";
            return out.str();
        }
    } catch(std::system_error& ex) {
        std::string message = "system exception: ";
        message += ex.what();
        LOG_ERROR(message);
        throw http_error::internal_server_error("deadlock");
    } catch(...) {
        LOG_ERROR("unknown exception");
        throw http_error::internal_server_error("unknown");
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
        auto it = mGdtEntriesByEmails.find(email);
        if(it != mGdtEntriesByEmails.end() && !it->second->canUpdate()) {
            return UpdateStatus::SKIPPED;
        }
        bool customerExist = true;
        auto customer = mysql::Customer::getByEmail(email, connection);
        if(!customer) {
            customer = std::make_shared<model::Customer>(email);
            customerExist = false;
        } 
        auto gdtEntriesList = mysql::GdtEntry::getByCustomer(customer, connection);
        
        for(auto email: customer->getEmails()) {
            auto it = mGdtEntriesByEmails.find(email);
            if(it == mGdtEntriesByEmails.end()) {
                mGdtEntriesByEmails.insert({email, gdtEntriesList});
            } else {
                it->second = gdtEntriesList;
            }
        }
        if(gdtEntriesList->getTotalCount() > 0) {
            if(!customerExist) {
                std::string message = "contact ";
                message = email.substr(0, 5) + "... in contacts not found";
                LOG_INFORMATION(message);
            }
            printf("[UpdateGdtEntryList] %s timeUsed for loading %ld contacts, %d gdt entries\n",
                timeUsed.string().data(), customer->getEmails().size(), gdtEntriesList->getTotalCount()
            );
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
    model::CustomersMap customers;

    try {
        customers = mysql::Customer::getAll(connection);
    }
    catch(const boost::bad_lexical_cast& e) {
        LOG_ERROR("boost bad lexical cast by calling mysql::Customer::getAll");
        throw;
    }

    // check that our email regex pattern is matching all emails from db
    // it is used to filter out malicious requests
    for (auto customer : customers) {
        for (auto email : customer.second->getEmails()) {
            // test regex pattern
            if(!std::regex_match(email, g_EmailValidPattern)) {
                std::string message = email;
                message += " matched false with email valid pattern, please update pattern!";
                LOG_ERROR(message);
            }            
        }
    }

    Profiler timeUsed;
    std::vector<std::string> emailsNotInCustomer;
    model::EmailGdtEntriesListMap gdtEntriesPerEmail;
    try {
        gdtEntriesPerEmail = mysql::GdtEntry::getAll(customers, connection, emailsNotInCustomer);
    } catch(const boost::bad_lexical_cast& e) {
        LOG_ERROR("boost bad lexical cast by calling mysql::GdtEntry::getAll");
        throw;
    }
    printf("[%s] time used for loading all gdt entries from db into memory: %s\n", __FUNCTION__, timeUsed.string().data());
    timeUsed.reset();

    std::lock_guard gdtEntriesAccessLock(mGdtEntriesAccessMutex);
    mGdtEntriesByEmails.clear();    
    
    // sort together
    // for every email there is a map with gdt entries belonging to the customer (not only to this email)
    for (auto customer : customers)
    {
        auto email = customer.second->getEmails().front();
        auto it = gdtEntriesPerEmail.find(email);
        if(it == gdtEntriesPerEmail.end()) {
            LOG_ERROR("cannot find email in gdt entries map");
        } else {
            for (auto email : customer.second->getEmails()) {
                mGdtEntriesByEmails.insert({email, it->second});
            }
        }
    }
    for(auto email: emailsNotInCustomer) {
        auto it = gdtEntriesPerEmail.find(email);
        if(it == gdtEntriesPerEmail.end()) {
            LOG_ERROR("cannot find email in gdt entries map");
        } else {
            mGdtEntriesByEmails.insert({email, it->second});
        }
    }
}

Document CacheServer::_listPerEmailApi(
    const std::string &email,
    int page /* = 1*/,
    int count /*= 25*/,
    view::OrderDirections order /* = model::GdtEntryList::OrderDirections::ASC */
)
{
    std::stringstream requestUrlStream;
    requestUrlStream
        << "https://" << g_Config->gdtServerUrl << "/gdt-entries/listPerEmailApi/"
        << email << "/" << std::to_string(page) << "/" << std::to_string(count) << "/"
        << view::orderDirectionsToString(order);

    auto res = http_get(requestUrlStream.str());

    Document bodyJson;
    bodyJson.Parse(res.body.data());
    if (bodyJson.HasParseError())
    {
        throw RapidjsonParseErrorException(
            "error parsing request answer",
            bodyJson.GetParseError(),
            bodyJson.GetErrorOffset())
            .setRawText(res.body);
    }
    return bodyJson;
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
