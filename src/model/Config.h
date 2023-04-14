#ifndef __GDT_CACHE_MODEL_CONFIG_H
#define __GDT_CACHE_MODEL_CONFIG_H

#include <string>

namespace model 
{
    struct Config
    {
        Config(const char* configFilename);
        std::string gdtServerUrl;
    };
}
extern model::Config* g_Config;

#endif //__GDT_CACHE_MODEL_CONFIG_H