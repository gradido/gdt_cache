#include "GdtEntriesCache.h"
#include "GradidoBlockchainException.h"
#include "model/Config.h"
#include "controller/Contacts.h"
#include "controller/GdtEntries.h"
#include "lib/Profiler.h"
#include <lithium_http_client.hh>
#include <rapidjson/document.h>

#include <thread>
#include <sstream>
#include <chrono>

using namespace li;
using namespace rapidjson;
using namespace std::chrono_literals;

GdtEntriesCache::GdtEntriesCache()
{

}

GdtEntriesCache::~GdtEntriesCache()
{

}

GdtEntriesCache* GdtEntriesCache::getInstance()
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
    try {
        auto res = http_get(requestUrl);

        printf("[%s] used time for contacts/apiGetCustomerWithGdtEntries request: %s\n", __FUNCTION__, timeUsed.string().data());
        printf("size: %ld kByte\n", res.body.size() / 1024);    

        timeUsed.reset();
    
        bodyJson.Parse(res.body.data()); 

        printf("[%s] used time parsing to json: %s\n", __FUNCTION__, timeUsed.string().data());
        timeUsed.reset();
    } catch(std::exception& ex) {
        fprintf(stderr, "[%s] call to contacts/apiGetCustomerWithGdtEntries throw exception: %s\ntry apiIndex as fallback\n",
            __FUNCTION__, ex.what()
        );
        requestUrl = "https://" + g_Config->gdtServerUrl + "/contacts/apiIndex";
        auto res = http_get(requestUrl);

        printf("[%s] used time for contacts/apiIndex request: %s\n", __FUNCTION__, timeUsed.string().data());
        printf("size: %ld kByte\n", res.body.size() / 1024);    

        timeUsed.reset();
    
        bodyJson.Parse(res.body.data()); 

        printf("[%s] used time parsing to json: %s\n", __FUNCTION__, timeUsed.string().data());
        timeUsed.reset();
    }

    if(!bodyJson.HasMember("emails")) {
        fprintf(stderr, "[%s] missing emails member in apiGetCustomerWithGdtEntries result\n", __FUNCTION__);
        return false;
    }
    if(bodyJson.HasMember("missingEmails") && bodyJson["missingEmails"].GetArray().Size() > 0) {
        fprintf(stderr, "[%s] missing emails, please check gdt server code\n", __FUNCTION__);
        for(auto& missingEmail: bodyJson["missingEmails"].GetArray()) {
            fprintf(stderr, "%s", missingEmail.GetString());
        }
        fprintf(stderr, "\n");
        return false;
    }
    auto contactsCount = bodyJson["emails"].GetArray().Size();
    int currentContact = 1;
    printf("[%s] contacts count: %d\n", __FUNCTION__, bodyJson["emails"].GetArray().Size());
        
    Profiler breakTimer;
    for (auto& contact : bodyJson["emails"].GetArray()){
        Profiler timePerEmail;
        //printf("%s ", contact["email"].GetString());  
        std::string email = contact.GetString();
        std::set<std::string> emails;
        emails.insert(email);
        std::shared_ptr<model::GdtEntryList> gdtEntries = std::make_shared<model::GdtEntryList>();
        int page = 1;
        do {
            Profiler timePerCall;
            try {
                auto json = _listPerEmailApi(email, page, 50, model::GdtEntryList::OrderDirections::ASC);
                page++;
                auto updatedCount = gdtEntries->addGdtEntry(json);
                printf("[%s] time for %d added gdt entries: %s\n", __FUNCTION__, updatedCount, timePerCall.string().data());
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
        } while(gdtEntries->getTotalCount() > gdtEntries->getGdtEntriesCount());
        printf("[%s] used time: %s for email: %s %d/%d\n",
            __FUNCTION__, timePerEmail.string().data(), email.data(), currentContact, contactsCount
        );

        mGdtEntriesByEmailMutex.lock();
        for(auto email: emails) {
            mGdtEntriesByEmails.insert({email, gdtEntries});
        }
        mGdtEntriesByEmailMutex.unlock();

        if(breakTimer.seconds() > 1.0) {
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
    const auto& customers = contacts.getCustomers();
    controller::GdtEntries gdtEntries;
    gdtEntries.loadGlobalModificators(dbSession);
    std::list<std::string> missingEmailsInContactsTable;

    Profiler timeUsed;

    std::map<std::string, std::shared_ptr<model::GdtEntryList>> gdtEntriesPerEmail;
    for(auto customer: customers) {
        auto gdtEntriesList = std::make_shared<model::GdtEntryList>();
        for(auto email: customer.second->getEmails()) {
            gdtEntriesPerEmail.insert({email, gdtEntriesList});
        }
    }
    
    auto rows = dbSession(
        "select id, amount, UNIX_TIMESTAMP(date), LOWER(TRIM(email)), IFNULL(comment, ''), \
            IFNULL(source, ''), IFNULL(project, ''), IFNULL(coupon_code, ''), \
            gdt_entry_type_id, factor, amount2, factor2 from gdt_entries order by date ASC"
    );
    while(auto row = rows.read_optional
    <int, long long, int, std::string, std::string, std::string, std::string, std::string, int, float, int, float>()) 
    {   
        model::GdtEntry entry(row.value());
        // skip gdt entries with empty emails
        if(entry.getEmail().size() == 0) continue;
        
        auto it = gdtEntriesPerEmail.find(entry.getEmail());
        if(it == gdtEntriesPerEmail.end()) {
            missingEmailsInContactsTable.push_back(entry.getEmail());
            it = gdtEntriesPerEmail.insert({entry.getEmail(), std::make_shared<model::GdtEntryList>()}).first;
            fprintf(stderr, "[%s] missing %s email in contacts table, cannot check global mods for them\n", __FUNCTION__, entry.getEmail().data());
        }
        it->second->addGdtEntry(entry);
        //printf("add %d: %ld\n", entry.getId(), entry.getDate());
    }
    printf("[%s] time used for loading all gdt entries from db into memory: %s\n", __FUNCTION__, timeUsed.string().data());

    // sort together and check for global mod updates
    for(auto customer: customers) {
        auto email = customer.second->getEmails().front();
        auto gdtEntriesList = gdtEntriesPerEmail.find(email)->second;
        if(gdtEntries.checkForMissingGlobalMod(customer.second, gdtEntriesPerEmail.find(email)->second)) {
            // if global mod is missing, let gdt server calculate new one with request
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
        }

        mGdtEntriesByEmailMutex.lock();
        for(auto email: customer.second->getEmails()) {
            mGdtEntriesByEmails.insert({email, gdtEntriesPerEmail.find(email)->second});
        }
        mGdtEntriesByEmailMutex.unlock();
    }

    printf("[%s] time used for loading %ld gdt entries from db: %s\n", 
        __FUNCTION__, customers.size(), timeUsed.string().data()
    );

    return false;
}

std::string GdtEntriesCache::listPerEmailApi(
    const std::string& email, 
    int page/* = 1*/, 
    int count /*= 25*/, 
    model::GdtEntryList::OrderDirections order/* = model::GdtEntryList::OrderDirections::ASC*/
)
{
    return "listPerEmailApi";
}

std::string GdtEntriesCache::sumPerEmailApi(const std::string& email)
{
    return "sumPerEmailApi";
}

Document GdtEntriesCache::_listPerEmailApi(
    const std::string& email, 
    int page/* = 1*/, 
    int count /*= 25*/, 
    model::GdtEntryList::OrderDirections order /* = model::GdtEntryList::OrderDirections::ASC */
)
{
    std::stringstream requestUrlStream; 
    requestUrlStream
        << "https://" << g_Config->gdtServerUrl << "/gdt-entries/listPerEmailApi/"
        << email << "/" << std::to_string(page) << "/" << std::to_string(count) << "/" 
        << model::GdtEntryList::orderDirectionsToString(order)
    ;
    
    auto res = http_get(requestUrlStream.str());

    Document bodyJson;
    bodyJson.Parse(res.body.data()); 
    if (bodyJson.HasParseError()) {
		throw RapidjsonParseErrorException(
            "error parsing request answer", 
            bodyJson.GetParseError(),
            bodyJson.GetErrorOffset()
        ).setRawText(res.body);
	}
    return bodyJson;
}
