#include <lithium_http_server.hh>
#include "utils/stringUtils.h"

#include "CacheServer.h"
#include "logging/Logging.h"
#include "logging/ContainerSingleton.h"
#include "model/Config.h"
#include "lithium_symbols.h"
#include "utils/Profiler.h"
#include "boost/lexical_cast/bad_lexical_cast.hpp"

#include <chrono>
#include <thread>
#include <iostream>

using namespace li;
using namespace std::chrono_literals;

void checkIpAuthorized(http_request& request)
{
    if(!g_Config->ipWhiteListing) return;
    auto clientIp = request.ip_address();
    auto xForwardedFor = request.header("X-Forwarded-For");
    if(xForwardedFor.size()) {
        clientIp = xForwardedFor;
    }
    //printf("client ip: '%s', size: %ld\n", clientIp.data(), clientIp.size());
    std::string localhost_ipfv6 = "::";
    localhost_ipfv6.resize(INET6_ADDRSTRLEN);
    if(clientIp == localhost_ipfv6) return;
    auto cs = CacheServer::getInstance();
    int countTrials = 0;
    while(countTrials < 2) {        
        countTrials++;
        // compare client ip with allowed ips
        for(int i = 0; i < cs->getAllowedIpsCount(); i++) {
            //printf("compare %s == %s = %d\n", clientIp.data(), cs->getAllowedIp(i).data(), clientIp == cs->getAllowedIp(i));
            if(clientIp == cs->getAllowedIp(i)) {
                // found match, return to request
                return;
            }
        }
        if(countTrials >= 2) break;

        // no match found, update allowed IPs just in case caller came from DNS and since last update ip was updated
        CacheServer::UpdateStatus status;
        int updateTriesLeft = 10;
        // wait for it, if update is already running
        while((status = cs->updateAllowedIps(true)) == CacheServer::UpdateStatus::RUNNING && updateTriesLeft) {
            std::this_thread::sleep_for(10ms);
            updateTriesLeft--;
        }
        // we don't wait any longer for update
        if(updateTriesLeft <= 0 || status == CacheServer::UpdateStatus::ERROR) {
            LOG_ERROR("Waited more than 100ms for updated allowed ips, return with unauthorized.");
            throw http_error::unauthorized("{\"state\":\"error\"}");
        }
    }    
    std::string message = "request from: ";
    message += clientIp;
    message += " unauthorized";
    LOG_INFORMATION(message);
    throw http_error::unauthorized("{\"state\":\"error\"}");
}

int main(int argc, char* argv[])
{
    Profiler timeUsed;
    try {
        // load config
        // use first parameter from command line if exist
        if(argc > 1) {
            g_Config  = new model::Config(argv[1]);
        } else {        
        // else use config.json in current working dir
            g_Config = new model::Config("config.json");
        }
    } catch(std::exception& e) {
        LOG_ERROR(e.what());
        return -4;
    }
    
    // load data form gdt server
    auto cs = CacheServer::getInstance();
    cs->updateAllowedIps(true);
    try {
        cs->initializeFromDb();
    } catch(boost::bad_lexical_cast& ex) {
        std::string message = "exception thrown on initialize: bad lexical cast from source: ";
        message += ex.source_type().name();
        message += " to target: ";
        message += ex.target_type().name();
        LOG_ERROR(message);
        return -3;
    } catch(std::exception& ex) {
        std::string message = "exception thrown on initialize: ";
        message += ex.what();
        LOG_ERROR(message);
        return -2;
    }    
    auto ec = logging::ContainerSingleton::getInstance();
    http_api api;
    api.get("/status") = [&](http_request& request, http_response& response) 
    {
        response.set_header("content-type", "text/html; charset=utf-8");
        if(ec->hasErrors()) {
            response.write("<html><head><title>Errors</title></head><body>" + ec->getErrorsHtml() + "</body></html>");
        } else { 
            std::string responseString = ec->getDbLastUpdateTime();
            responseString += "<br>status: ok";
            response.write(responseString);
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
        response.write(cs->listPerEmailApi(trim(toLowercase(params.email))));        
    };
    api.get("/listPerEmailApi/{{email}}/{{page}}") = [&](http_request& request, http_response& response) {
        checkIpAuthorized(request);
        auto params = request.url_parameters(s::email = std::string(), s::page = int());
        response.set_header("content-type", "application/json");
        response.write(cs->listPerEmailApi(trim(toLowercase(params.email)), params.page));        
    };
    api.get("/listPerEmailApi/{{email}}/{{page}}/{{count}}") = [&](http_request& request, http_response& response) {
        checkIpAuthorized(request);
        auto params = request.url_parameters(s::email = std::string(), s::page = int(), s::count = int());
        response.set_header("content-type", "application/json");
        response.write(cs->listPerEmailApi(trim(toLowercase(params.email)), params.page, params.count));                
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
                cs->listPerEmailApi(
                    trim(toLowercase(params.email)), 
                    params.page, 
                    params.count, 
                    view::orderDirectionFromString(params.orderDirection)
                )
            );            
        };

    api.get("/sumPerEmailApi") = [](http_request& request, http_response& response) {
        response.set_header("content-type", "application/json");
        response.write("{\"state\":\"error\",\"msg\":\"no post\"}");        
    };
    api.get("/sumPerEmailApi/{{email}}") = [&](http_request& request, http_response& response) {
        checkIpAuthorized(request);
        response.set_header("content-type", "application/json");
        auto params = request.url_parameters(s::email = std::string());
        if(params.email == "") {
            response.write("{\"state\":\"error\",\"msg\":\"parameter error\"}");
        } else {
            response.write(cs->sumPerEmailApi(trim(toLowercase(params.email))));
        }        
    };
    api.post("/sumPerEmailApi") = [&](http_request& request, http_response& response) {
        checkIpAuthorized(request);
        response.set_header("content-type", "application/json");
        auto params = request.post_parameters(s::email = std::optional<std::string>());
        if(!params.email || params.email.value() == "") {
            response.write("{\"state\":\"error\",\"msg\":\"parameter error\"}");
        } else {
            response.write(cs->sumPerEmailApi(trim(toLowercase(params.email.value()))));
        }        
    };
    std::string info = "time for startup preparations: ";
    info += timeUsed.string();
    LOG_INFORMATION(info);
    // start http server
    http_serve(api, g_Config->port);

    delete g_Config;
    g_Config = nullptr;
    return 0;
}