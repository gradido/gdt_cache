#ifndef __GDT_CACHE_MODEL_CONFIG_H
#define __GDT_CACHE_MODEL_CONFIG_H

#include <string>
#include <vector>
#include <regex>
#include <lithium_mysql.hh>

namespace model 
{
    struct Config
    {
        Config(const char* configFilename);
        ~Config();

        int port;
        //! minimal cache timeout in seconds
        int minCacheTimeout;
        //! maximal cache timeout in seconds
        int maxCacheTimeout;

        li::mysql_database* database;
        std::vector<std::string> allowedHosts;
        
        // if ip white listening is enable or not
        // true (default) if it is enabled
        bool ipWhiteListing;        
    };
}
extern model::Config* g_Config;
extern const std::regex g_EmailValidPattern;

#endif //__GDT_CACHE_MODEL_CONFIG_H