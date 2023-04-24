#include "GdtEntriesCache.h"
#include "GradidoBlockchainException.h"
#include "ErrorContainer.h"
#include "model/Config.h"
#include "controller/Contacts.h"
#include "controller/GdtEntries.h"
#include "controller/UpdateManager.h"
#include "lib/Profiler.h"
#include "task/UpdateGdtEntryList.h"

#include <lithium_http_client.hh>
#include <lithium_http_server.hh>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <thread>
#include <sstream>
#include <chrono>
#include <regex>

using namespace li;
using namespace rapidjson;
using namespace std::chrono_literals;

GdtEntriesCache::GdtEntriesCache()
{
}

GdtEntriesCache::~GdtEntriesCache()
{
}

GdtEntriesCache *GdtEntriesCache::getInstance()
{
    static GdtEntriesCache one;
    return &one;
}

bool GdtEntriesCache::initializeFromPhp()
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
        fprintf(stderr, "[%s] call to contacts/apiGetCustomerWithGdtEntries throw exception: %s\ntry apiIndex as fallback\n",
                __FUNCTION__, ex.what());
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
        fprintf(stderr, "[%s] missing emails member in apiGetCustomerWithGdtEntries result\n", __FUNCTION__);
        return false;
    }
    if (bodyJson.HasMember("missingEmails") && bodyJson["missingEmails"].GetArray().Size() > 0)
    {
        fprintf(stderr, "[%s] missing emails, please check gdt server code\n", __FUNCTION__);
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
            try
            {
                auto json = _listPerEmailApi(email, page, 50, model::GdtEntryList::OrderDirections::ASC);
                page++;
                auto updatedCount = gdtEntries->addGdtEntry(json);
                printf("[%s] time for %d added gdt entries: %s\n", __FUNCTION__, updatedCount, timePerCall.string().data());
            }
            catch (RapidjsonParseErrorException &ex)
            {
                auto errorString = ex.getFullString();
                if (errorString.size() > 100)
                {
                    std::string fileName = std::to_string(std::time(nullptr));
                    fileName += "_listPerEmailApi.html";
                    FILE *f = fopen(fileName.data(), "wt");
                    fwrite(errorString.data(), sizeof(char), errorString.size(), f);
                    fclose(f);
                    fprintf(
                        stderr, "RapidjsonParseErrorException [%s] error to long, written to file: %s\n",
                        __FUNCTION__, fileName.data());
                }
                else
                {
                    fprintf(stderr, "RapidjsonParseErrorException [%s] %s\n", __FUNCTION__, ex.getFullString().data());
                }

                return false;
            }
        } while (gdtEntries->getTotalCount() > gdtEntries->getGdtEntriesCount());
        printf("[%s] used time: %s for email: %s %d/%d\n",
               __FUNCTION__, timePerEmail.string().data(), email.data(), currentContact, contactsCount);

        mGdtEntriesByEmailMutex.lock();
        for (auto email : emails)
        {
            mGdtEntriesByEmails.insert({email, gdtEntries});
        }
        mGdtEntriesByEmailMutex.unlock();

        if (breakTimer.seconds() > 1.0)
        {
            breakTimer.reset();
            std::this_thread::sleep_for(100ms);
        }
        currentContact++;
    }
    printf("[%s] used time collecting gdt entries: %s\n", __FUNCTION__, timeUsed.string().data());

    return true;
}

bool GdtEntriesCache::initializeFromDb()
{
    auto dbSession = g_Config->database->connect();
    controller::Contacts contacts;
    contacts.loadCustomersFromDb(dbSession);
    const auto &customers = contacts.getCustomers();
    controller::GdtEntries gdtEntries;
    gdtEntries.loadGlobalModificators(dbSession);
    std::list<std::string> missingEmailsInContactsTable;

    Profiler timeUsed;

    std::map<std::string, std::shared_ptr<model::GdtEntryList>> gdtEntriesPerEmail;
    for (auto customer : customers)
    {
        auto gdtEntriesList = std::make_shared<model::GdtEntryList>();
        for (auto email : customer.second->getEmails())
        {
            // test regex pattern
            if(!std::regex_match(email, g_EmailValidPattern)) {
                fprintf(stderr, "[%s] %s matched false with email valid pattern, please update pattern!\n", __FUNCTION__, email.data());
            }            
            gdtEntriesPerEmail.insert({email, gdtEntriesList});
        }
    }

    auto rows = dbSession(
        "select id, amount, UNIX_TIMESTAMP(date), LOWER(TRIM(email)), IFNULL(comment, ''), \
            IFNULL(source, ''), IFNULL(project, ''), IFNULL(coupon_code, ''), \
            gdt_entry_type_id, factor, amount2, factor2 from gdt_entries order by date ASC");
    while (auto row = rows.read_optional<int, long long, int, std::string, std::string, std::string, std::string, std::string, int, float, int, float>())
    {
        model::GdtEntry entry(row.value());
        // skip gdt entries with empty emails
        if (entry.getEmail().size() == 0)
            continue;

        auto it = gdtEntriesPerEmail.find(entry.getEmail());
        if (it == gdtEntriesPerEmail.end())
        {
            missingEmailsInContactsTable.push_back(entry.getEmail());
            it = gdtEntriesPerEmail.insert({entry.getEmail(), std::make_shared<model::GdtEntryList>()}).first;
            fprintf(stderr, "[%s] missing %s email in contacts table, cannot check global mods for them\n", __FUNCTION__, entry.getEmail().data());
        }
        it->second->addGdtEntry(entry);
        // printf("add %d: %ld\n", entry.getId(), entry.getDate());
    }
    printf("[%s] time used for loading all gdt entries from db into memory: %s\n", __FUNCTION__, timeUsed.string().data());
    timeUsed.reset();

    int addedGlobalMods = 0;
    // sort together and check for global mod updates
    for (auto customer : customers)
    {
        auto email = customer.second->getEmails().front();
        auto gdtEntriesList = gdtEntriesPerEmail.find(email)->second;
        while (auto count = gdtEntries.checkForMissingGlobalMod(customer.second, gdtEntriesPerEmail.find(email)->second, dbSession))
        {
            addedGlobalMods += count;
            printf("\rcalculated globalMods: %d", addedGlobalMods);
            /*// if global mod is missing, let gdt server calculate new one with request
            gdtEntriesList->reset();
            int page = 1;
            do {
                Profiler timePerCall;
                try {
                    auto json = _listPerEmailApi(email, page, 500, model::GdtEntryList::OrderDirections::ASC);
                    page++;
                    auto dataSetSize = gdtEntriesList->addGdtEntry(json);
                    printf("[%s] time for %d updated gdt entries (global mod): %s\n",
                        __FUNCTION__, dataSetSize, timePerCall.string().data());
                    if(timePerCall.seconds() > 5) {
                        std::this_thread::sleep_for(1s);
                    }
                } catch(RapidjsonParseErrorException& ex) {
                    auto errorString = ex.getFullString();
                    if(errorString.size() > 100) {
                        std::string fileName = std::to_string(std::time(nullptr));
                        fileName += "_listPerEmailApi.html";
                        FILE* f = fopen(fileName.data(), "wt");
                        fwrite(errorString.data(), sizeof(char), errorString.size(), f);
                        fclose(f);
                        fprintf(
                            stderr, "RapidjsonParseErrorException [%s] error to long, written to file: %s\n",
                            __FUNCTION__, fileName.data()
                        );
                    } else {
                        fprintf(stderr, "RapidjsonParseErrorException [%s] %s\n", __FUNCTION__, ex.getFullString().data());
                    }

                    return false;
                }
            } while(gdtEntriesList->getTotalCount() > gdtEntriesList->getGdtEntriesCount());
            */
        }

        mGdtEntriesByEmailMutex.lock();
        for (auto email : customer.second->getEmails())
        {
            mGdtEntriesByEmails.insert({email, gdtEntriesPerEmail.find(email)->second});
        }
        mGdtEntriesByEmailMutex.unlock();
    }
    if (addedGlobalMods)
    {
        printf("\n");
        printf("[%s] time used for calculate %d missing global mods: %s\n",
               __FUNCTION__, addedGlobalMods, timeUsed.string().data());
    }
    else
    {
        printf("[%s] time used for checking for missing global mods: %s\n",
               __FUNCTION__, timeUsed.string().data());
    }

    return true;
}

std::string GdtEntriesCache::listPerEmailApi(
    const std::string &email,
    int page /* = 1*/,
    int count /*= 25*/,
    model::GdtEntryList::OrderDirections order /* = model::GdtEntryList::OrderDirections::ASC*/
    )
{
    Profiler timeUsed;
    try {
        std::unique_lock _lock(mGdtEntriesByEmailMutex);
        auto um = controller::UpdateManager::getInstance();
        auto it = mGdtEntriesByEmails.find(email);
        if(it == mGdtEntriesByEmails.end()) {            
            if(std::regex_match(email, g_EmailValidPattern)) {
                um->pushTask(std::make_shared<task::UpdateGdtEntryList>(email));
            }
            return "{\"state\":\"success\",\"count\":0,\"gdtEntries\":[],\"gdtSum\":0,\"timeUsed\":"
                        +std::to_string(static_cast<float>(timeUsed.seconds()))+"}";
        } else {
            if(it->second->canUpdate()) {
                um->pushTask(std::make_shared<task::UpdateGdtEntryList>(email), true);
            }
            // needed because of memory allocator
            Document baseJson(kObjectType);
            auto resultJson = it->second->toJson(baseJson.GetAllocator(), timeUsed, page, count, order);
            _lock.unlock();
            //printf("result allocator size: %d %d kByte\n", baseJson.GetAllocator().Size(), baseJson.GetAllocator().Size() / 1024);
            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);
            resultJson.Accept(writer);
            //printf("string buffer size: %d %d kByte\n", buffer.GetSize(), buffer.GetSize() / 1024);
            // 50 * 1024 = hardcoded output buffer size from lithium
            if(buffer.GetSize() > 50 * 1024) {
                auto alloc = baseJson.GetAllocator();
                Value errorJson(kObjectType);
                errorJson.AddMember("state", "error", alloc);
                errorJson.AddMember("error", "output to big, please retry with less count", alloc);
                Value details(kObjectType);
                details.AddMember("count", count, alloc);
                details.AddMember("overflow", buffer.GetSize()-50 * 1024, alloc);
                errorJson.AddMember("details", details, alloc);
                StringBuffer buffer2;
                Writer<StringBuffer> writer2(buffer2);
                errorJson.Accept(writer2);
                return buffer2.GetString();
            }
            return buffer.GetString();
        }

    } catch(std::system_error& ex) {
        std::string message = "system exception: ";
        message += ex.what();
        ErrorContainer::getInstance()->addError({message, "GdtEntriesCache", __FUNCTION__});
        throw li::http_error::internal_server_error("deadlock");
    } catch(...) {
        ErrorContainer::getInstance()->addError({"unknown exception", "GdtEntriesCache", __FUNCTION__});
        throw li::http_error::internal_server_error("unknown");
    }
}

std::string GdtEntriesCache::sumPerEmailApi(const std::string &email)
{
    Profiler timeUsed;
    try {
        std::lock_guard _lock(mGdtEntriesByEmailMutex);
        auto um = controller::UpdateManager::getInstance();
        auto it = mGdtEntriesByEmails.find(email);
        if(it == mGdtEntriesByEmails.end()) {
            if(std::regex_match(email, g_EmailValidPattern)) {
                um->pushTask(std::make_shared<task::UpdateGdtEntryList>(email));
            }
            return "{\"state\":\"success\",\"sum\":0,\"count\":0,\"time\":"+std::to_string(timeUsed.seconds())+"}";
        } else {
            if(it->second->canUpdate()) {
                um->pushTask(std::make_shared<task::UpdateGdtEntryList>(email), true);
            }
            return "{\"state\":\"success\",\"sum\":"
            + std::to_string(it->second->getGdtSum())
            +",\"count\":"
            + std::to_string(it->second->getTotalCount())
            +",\"time\":" + timeUsed.string() + "}";
        }
    } catch(std::system_error& ex) {
        std::string message = "system exception: ";
        message += ex.what();
        ErrorContainer::getInstance()->addError({message, "GdtEntriesCache", __FUNCTION__});
        throw li::http_error::internal_server_error("deadlock");
    } catch(...) {
        ErrorContainer::getInstance()->addError({"unknown exception", "GdtEntriesCache", __FUNCTION__});
        throw li::http_error::internal_server_error("unknown");
    } 
}

void GdtEntriesCache::scheduleForUpdates()
{
    try {
        auto um = controller::UpdateManager::getInstance();
        std::lock_guard _lock(mGdtEntriesByEmailMutex);
        for(auto gdtEntriesList: mGdtEntriesByEmails) {
            if(gdtEntriesList.second->shouldUpdate()) {
                um->pushTask(std::make_shared<task::UpdateGdtEntryList>(gdtEntriesList.first));
            }
        }
    } catch(std::system_error& ex) {
        std::string message = "system exception: ";
        message += ex.what();
        ErrorContainer::getInstance()->addError({message, "GdtEntriesCache", __FUNCTION__});
    } catch(...) {
        ErrorContainer::getInstance()->addError({"unknown exception", "GdtEntriesCache", __FUNCTION__});
    } 
}

bool GdtEntriesCache::canUpdateGdtEntryList(const std::string& email) const noexcept
{
    try {
        std::lock_guard _lock(mGdtEntriesByEmailMutex);
        auto it = mGdtEntriesByEmails.find(email);
        if(it == mGdtEntriesByEmails.end()) {
            return true;
        }
        return it->second->canUpdate();
    } catch(std::system_error& ex) {
        std::string message = "system exception: ";
        message += ex.what();
        ErrorContainer::getInstance()->addError({message, "GdtEntriesCache", __FUNCTION__});
    } catch(...) {
        ErrorContainer::getInstance()->addError({"unknown exception", "GdtEntriesCache", __FUNCTION__});
    } 
    return false;
}

bool GdtEntriesCache::shouldUpdateGdtEntryList(const std::string& email) const noexcept
{
    try {
        std::lock_guard _lock(mGdtEntriesByEmailMutex);
        auto it = mGdtEntriesByEmails.find(email);
        if(it == mGdtEntriesByEmails.end()) {
            return true;
        }
        return it->second->shouldUpdate();
    } catch(std::system_error& ex) {
        std::string message = "system exception: ";
        message += ex.what();
        ErrorContainer::getInstance()->addError({message, "GdtEntriesCache", __FUNCTION__});
    } catch(...) {
        ErrorContainer::getInstance()->addError({"unknown exception", "GdtEntriesCache", __FUNCTION__});
    } 
    return false;
}

void GdtEntriesCache::swapGdtEntryList(
    std::shared_ptr<model::Customer> customer,
    std::shared_ptr<model::GdtEntryList> updatedGdtEntryList
) noexcept
{
    try {
        std::lock_guard _lock(mGdtEntriesByEmailMutex);
        for(auto email: customer->getEmails()) 
        {
            auto it = mGdtEntriesByEmails.find(email);
            if(it == mGdtEntriesByEmails.end()) {
                mGdtEntriesByEmails.insert({email, updatedGdtEntryList});
            } else {
                it->second = updatedGdtEntryList;
            }
        }
        
    } catch(std::system_error& ex) {
        std::string message = "system exception: ";
        message += ex.what();
        ErrorContainer::getInstance()->addError({message, "GdtEntriesCache", __FUNCTION__});
    } catch(...) {
        ErrorContainer::getInstance()->addError({"unknown exception", "GdtEntriesCache", __FUNCTION__});
    } 
}

Document GdtEntriesCache::_listPerEmailApi(
    const std::string &email,
    int page /* = 1*/,
    int count /*= 25*/,
    model::GdtEntryList::OrderDirections order /* = model::GdtEntryList::OrderDirections::ASC */
)
{
    std::stringstream requestUrlStream;
    requestUrlStream
        << "https://" << g_Config->gdtServerUrl << "/gdt-entries/listPerEmailApi/"
        << email << "/" << std::to_string(page) << "/" << std::to_string(count) << "/"
        << model::GdtEntryList::orderDirectionsToString(order);

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
