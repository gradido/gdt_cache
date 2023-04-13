#include "GdtEntriesCache.h"

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
    //
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