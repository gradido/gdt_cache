#ifndef __GDT_CACHE_MODEL_CONFIG_H
#define __GDT_CACHE_MODEL_CONFIG_H

#include <string>
#include <lithium_mysql.hh>

namespace model 
{
    struct Config
    {
        Config(const char* configFilename);
        ~Config();
        std::string gdtServerUrl;
        //! minimal cache timeout in seconds
        int minCacheTimeout;

        li::mysql_database* database;
    };
}
extern model::Config* g_Config;

#endif //__GDT_CACHE_MODEL_CONFIG_H