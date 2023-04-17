#include <lithium_http_server.hh>

#include "GdtEntriesCache.h"
#include "model/Config.h"
#include "main.h"
#include "boost/lexical_cast/bad_lexical_cast.hpp"

using namespace li;

int main()
{
    // load config
    g_Config = new model::Config("config.json");
    
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

    http_api api;
    api.get("/status") = [](http_request& request, http_response& response) {
        response.write("status: ok");
    };
    api.get("/listPerEmailApi/{{email}}") = [&](http_request& request, http_response& response) {
        auto params = request.url_parameters(s::email = std::string());
        response.write(ge->listPerEmailApi(params.email));
    };
    api.get("/listPerEmailApi/{{email}}/{{page}}") = [&](http_request& request, http_response& response) {
        auto params = request.url_parameters(s::email = std::string(), s::page = int());
        response.write(ge->listPerEmailApi(params.email, params.page));
    };
    api.get("/listPerEmailApi/{{email}}/{{page}}/{{count}}") = [&](http_request& request, http_response& response) {
        auto params = request.url_parameters(s::email = std::string(), s::page = int(), s::count = int());
        response.write(ge->listPerEmailApi(params.email, params.page, params.count));
    };
    api.get("/listPerEmailApi/{{email}}/{{page}}/{{count}}/{{orderDirection}}")
        = [&](http_request& request, http_response& response) {
            auto params = request.url_parameters(
                s::email = std::string(), 
                s::page = int(),
                s::count = int(),
                s::orderDirection = std::string()
            );
            response.write(
                ge->listPerEmailApi(params.email, params.page, params.count, model::GdtEntryList::orderDirectionFromString(params.orderDirection))
            );
        };

    api.post("/sumPerEmailApi") = [&](http_request& request, http_response& response) {
        auto params = request.post_parameters(s::email = std::string());
        response.write(ge->sumPerEmailApi(params.email));
    };

    // start http server
    http_serve(api, 8710);

    delete g_Config;
    g_Config = nullptr;
    return 0;
}