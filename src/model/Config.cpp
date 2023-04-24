#include "Config.h"
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>

model::Config* g_Config = nullptr;
const std::regex g_EmailValidPattern("^[^\\s@]+@([^\\s@.,]+\\.)+[^\\s@.,]{2,}$");

using namespace rapidjson;

namespace model {

    Config::Config(const char* configFilename)
    : minCacheTimeout(10)
    {
        std::ifstream ifsConfig(configFilename);
        IStreamWrapper iswConfig(ifsConfig);
    
        Document configJson;
        configJson.ParseStream(iswConfig);
        if(configJson.HasMember("gdtServerUrl") && configJson["gdtServerUrl"].IsString()) {
            gdtServerUrl = configJson["gdtServerUrl"].GetString();
        }
        if(configJson.HasMember("minCacheTimeout") && configJson["minCacheTimeout"].IsInt()) {
            minCacheTimeout = configJson["minCacheTimeout"].GetInt();
        }

        // load db config
        if(configJson.HasMember("db") && configJson["db"].IsObject()) {
            const auto& db = configJson["db"].GetObject();
            if(!db.HasMember("username") || !db.HasMember("password") || !db.HasMember("name")) {
                fprintf(stderr, "[%s] missing infos for db connection\n", __FUNCTION__);
            } else {
                database = new li::mysql_database(
                    s::host = "127.0.0.1", // Hostname or ip of the database server
                    s::database = db["name"].GetString(),  // Database name
                    s::user = db["username"].GetString(), // Username
                    s::password = db["password"].GetString(), // Password
                    s::port = 3306, // Port
                    s::charset = "utf8", // Charset
                    // Only for async connection, specify the maximum number of SQL connections per thread.
                    s::max_async_connections_per_thread = 20,
                    // Only for synchronous connection, specify the maximum number of SQL connections
                    s::max_sync_connections = 200
                );
            }
        }
        // load allowed ips
        if(configJson.HasMember("allowedIPs") && configJson["allowedIPs"].IsArray()) {
            const auto& allowedHostsJsonArray = configJson["allowedIPs"].GetArray();
            allowedHosts.reserve(allowedHostsJsonArray.Size());
            for (auto& allowedHost : allowedHostsJsonArray) {
                allowedHosts.push_back(allowedHost.GetString());
            }
        } 
    }
    Config::~Config()
    {   
        delete database;
    }
}