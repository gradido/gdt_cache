#include "Config.h"
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>

model::Config* g_Config = nullptr;
using namespace rapidjson;

namespace model {

    Config::Config(const char* configFilename)
    : minCacheTimeout(10)
    {
        std::ifstream ifsConfig(configFilename);
        IStreamWrapper iswConfig(ifsConfig);
    
        Document configJson;
        configJson.ParseStream(iswConfig);
        if(configJson.HasMember("gdtServerUrl")) {
            gdtServerUrl = configJson["gdtServerUrl"].GetString();
        }
        if(configJson.HasMember("minCacheTimeout")) {
            minCacheTimeout = configJson["minCacheTimeout"].GetInt();
        }
    }
}