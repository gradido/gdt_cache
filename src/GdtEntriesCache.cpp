#include "GdtEntriesCache.h"
#include "model/Config.h"
#include "lib/Profiler.h"
#include <lithium_http_client.hh>
#include <rapidjson/document.h>

using namespace li;
using namespace rapidjson;

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

bool GdtEntriesCache::initialize()
{
    Profiler timeUsed;
    // Get list of contacts
    std::string requestUrl = "https://" + g_Config->gdtServerUrl;
    requestUrl += "/contacts/apiIndex";
    auto res = http_get(requestUrl);
    Document bodyJson;
    bodyJson.Parse(res.body.data()); 
    printf("get contacts result: %s\n", res.body.data());   

    printf("[%s] used time: %s\n", __FUNCTION__, timeUsed.string().data());

    return false;

    return true;
}

std::string GdtEntriesCache::listPerEmailApi(
    const std::string& email, 
    int page/* = 1*/, 
    int count /*= 25*/, 
    OrderDirections order/* = OrderDirections::ASC*/
)
{
    return "listPerEmailApi";
}

std::string GdtEntriesCache::sumPerEmailApi(const std::string& email)
{
    return "sumPerEmailApi";
}

GdtEntriesCache::OrderDirections GdtEntriesCache::orderDirectionFromString(const std::string& orderDirection)
{
    if(orderDirection == "ASC") return OrderDirections::ASC;
    if(orderDirection == "DESC") return OrderDirections::DESC;
    fprintf(stderr, "[%s] invalid order direction: %s, use ASC as default\n", __FUNCTION__, orderDirection);
    return OrderDirections::ASC;
}