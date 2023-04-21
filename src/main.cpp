#include <lithium_http_server.hh>

#include "GdtEntriesCache.h"
#include "ErrorContainer.h"
#include "controller/UpdateManager.h"
#include "model/Config.h"
#include "main.h"
#include "boost/lexical_cast/bad_lexical_cast.hpp"

#include <chrono>
#include <thread>

using namespace li;
using namespace std::chrono_literals;

void checkIpAuthorized(http_request& request)
{
    auto clientIp = request.ip_address();
    auto um = controller::UpdateManager::getInstance();
    int countTrials = 0;
    while(countTrials < 2) {        
        countTrials++;
        // compare client ip with allowed ips
        for(int i = 0; i < um->getAllowedIpsCount(); i++) {
            if(clientIp == um->getAllowedIp(i)) {
                // found match, return to request
                return;
            }
        }
        if(countTrials >= 2) break;

        // no match found, update allowed IPs just in case caller came from DNS and since last update ip was updated
        controller::UpdateStatus status;
        int updateTriesLeft = 10;
        // wait for it, if update is already running
        while((status = um->updateAllowedIps()) == controller::UpdateStatus::RUNNING && updateTriesLeft) {
            std::this_thread::sleep_for(10ms);
            updateTriesLeft--;
        }
        // we don't wait any longer for update
        if(updateTriesLeft <= 0 || status == controller::UpdateStatus::ERROR) {
            throw http_error::unauthorized("{\"state\":\"error\"}");
        }
    }    
}

int main()
{
    // load config
    g_Config = new model::Config("config.json");
    controller::UpdateManager::getInstance()->updateAllowedIps();
    
    // load data form gdt server
    auto ge = GdtEntriesCache::getInstance();
    try {
        if(!ge->initializeFromDb()) {
            fprintf(stderr, "error initializing gdt entries cache\n");
            return -1;
        }
    } catch(boost::bad_lexical_cast& ex) {
        fprintf(stderr, "exception thrown on initialize: bad lexical cast from source: %s to target: %s\n",
            ex.source_type().name(), ex.target_type().name());
        return -3;
    } catch(std::exception& ex) {
        fprintf(stderr, "exception thrown on initialize: %s\n", ex.what());
        return -2;
    }    
    auto ec = ErrorContainer::getInstance();
    http_api api;
    api.get("/status") = [&](http_request& request, http_response& response) 
    {
        response.set_header("content-type", "text/html; charset=utf-8");
        if(ec->hasErrors()) {
            response.write("<html><head><title>Errors</title></head><body>" + ec->getErrorsHtml() + "</body></html>");
        } else { 
            response.write("status: ok");
        }        
    };
    api.get("/listPerEmailApi") = [](http_request& request, http_response& response) {
        checkIpAuthorized(request);
        response.set_header("content-type", "application/json");
        response.write("{\"state\":\"error\",\"msg\":\"parameter error\"}");        
    };
    api.get("/listPerEmailApi/{{email}}") = [&](http_request& request, http_response& response) {
        checkIpAuthorized(request);
        auto params = request.url_parameters(s::email = std::string());
        response.set_header("content-type", "application/json");
        response.write(ge->listPerEmailApi(params.email));        
    };
    api.get("/listPerEmailApi/{{email}}/{{page}}") = [&](http_request& request, http_response& response) {
        checkIpAuthorized(request);
        auto params = request.url_parameters(s::email = std::string(), s::page = int());
        response.set_header("content-type", "application/json");
        response.write(ge->listPerEmailApi(params.email, params.page));        
    };
    api.get("/listPerEmailApi/{{email}}/{{page}}/{{count}}") = [&](http_request& request, http_response& response) {
        checkIpAuthorized(request);
        auto params = request.url_parameters(s::email = std::string(), s::page = int(), s::count = int());
        response.set_header("content-type", "application/json");
        response.write(ge->listPerEmailApi(params.email, params.page, params.count));        
    };
    api.get("/listPerEmailApi/{{email}}/{{page}}/{{count}}/{{orderDirection}}")
        = [&](http_request& request, http_response& response) {
            checkIpAuthorized(request);
            auto params = request.url_parameters(
                s::email = std::string(), 
                s::page = int(),
                s::count = int(),
                s::orderDirection = std::string()
            );
            response.set_header("content-type", "application/json");
            response.write(
                ge->listPerEmailApi(
                    params.email, 
                    params.page, 
                    params.count, 
                    model::GdtEntryList::orderDirectionFromString(params.orderDirection)
                )
            );            
        };

    api.get("/sumPerEmailApi") = [](http_request& request, http_response& response) {
        response.set_header("content-type", "application/json");
        response.write("{\"state\":\"error\",\"msg\":\"no post\"}");        
    };
    api.post("/sumPerEmailApi") = [&](http_request& request, http_response& response) {
        checkIpAuthorized(request);
        response.set_header("content-type", "application/json");
        auto params = request.post_parameters(s::email = std::optional<std::string>());
        if(!params.email || params.email.value() == "") {
            response.write("{\"state\":\"error\",\"msg\":\"parameter error\"}");
        } else {
            response.write(ge->sumPerEmailApi(params.email.value()));
        }        
    };

    // start http server
    http_serve(api, 8710);

    delete g_Config;
    g_Config = nullptr;
    return 0;
}