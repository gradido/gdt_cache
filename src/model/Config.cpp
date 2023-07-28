#include "Config.h"
#include "../lithium_symbols.h"
#include <fstream>
#include <lithium_json.hh>
#include <lithium_metamap.hh>

model::Config* g_Config = nullptr;
const std::regex g_EmailValidPattern("^[^\\s@]+@([^\\s@.,]+\\.)+[^\\s@.,]{2,}$");

namespace model {

    Config::Config(const char* configFilename)
    : port(8710), minCacheTimeout(10), maxCacheTimeout(60 * 60 * 4), ipWhiteListing(true)
    {
        std::ifstream ifsConfig(configFilename);
        // Define a string to store the contents of the file
        std::string content((std::istreambuf_iterator<char>(ifsConfig)), std::istreambuf_iterator<char>());

        auto config = mmm(
            s::port = std::optional<int>(),
            s::db = mmm(
                s::username = std::string(),
                s::password = std::string(),
                s::name = std::string()
            ),
            s::minCacheTimeout = std::optional<int>(),
            s::maxCacheTimeout = std::optional<int>(),
            s::allowedIPs = std::vector<std::string>(),
            s::ipWhiteListening = std::optional<bool>()
        );                
        auto result = json_decode(content, config);
        if(result.bad()) {
            std::string message = "try loading config.json\n";
            message += result.what;
            throw std::runtime_error(message);
        }
        if(config.port) {
            port = config.port.value();
        }
        if(config.minCacheTimeout) {
            minCacheTimeout = config.minCacheTimeout.value();
        }
        if(config.maxCacheTimeout) {
            maxCacheTimeout = config.maxCacheTimeout.value();
        }
        if(config.ipWhiteListening) {
            ipWhiteListing = config.ipWhiteListening.value();
        }
        allowedHosts = config.allowedIPs;
    
        // load db config        
        database = new li::mysql_database(
            s::host = "127.0.0.1", // Hostname or ip of the database server
            s::database = config.db.name,  // Database name
            s::user = config.db.username, // Username
            s::password = config.db.password, // Password
            s::port = 3306, // Port
            s::charset = "utf8", // Charset
            // Only for async connection, specify the maximum number of SQL connections per thread.
            s::max_async_connections_per_thread = 20,
            // Only for synchronous connection, specify the maximum number of SQL connections
            s::max_sync_connections = 200
        );
    }
    Config::~Config() {   
        delete database;
    }
}