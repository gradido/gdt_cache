#include <lithium_http_server.hh>
#include "GdtEntriesCache.h"

#include "main.h"

using namespace li;

int main()
{
    http_api api;
    auto ge = GdtEntriesCache::getInstance();
    // GET: listPerEmailApi($email, $page = 1, $count = 25, $orderDirection = 'ASC')
    // POST: sumPerEmailApi email
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
                ge->listPerEmailApi(params.email, params.page, params.count, ge->orderDirectionFromString(params.orderDirection))
            );
        };

    api.post("/sumPerEmailApi") = [&](http_request& request, http_response& response) {
        auto params = request.post_parameters(s::email = std::string());
        response.write(ge->sumPerEmailApi(params.email));
    };

    // start http server
    http_serve(api, 8710);
    return 0;
}