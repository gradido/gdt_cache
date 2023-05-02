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

        std::string gdtServerUrl;
        int port;
        //! minimal cache timeout in seconds
        int minCacheTimeout;
        //! maximal cache timeout in seconds
        int maxCacheTimeout;

        li::mysql_database* database;
        std::vector<std::string> allowedHosts;
        
    };
}
extern model::Config* g_Config;
extern const std::regex g_EmailValidPattern;

#endif //__GDT_CACHE_MODEL_CONFIG_H