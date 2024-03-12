#ifndef SERVERCONFIG_HPP_
#define SERVERCONFIG_HPP_

#include <fstream>
#include <string>

#include <nlohmann/json.hpp>

#include "common/Exception.hpp"
#include "common/utils/IntTypes.hpp"

namespace App {

namespace fs = std::filesystem;

struct ServerConfig {
    std::string configPath = "config.json";
    std::string version = "1.0";
    u16 port = 25531;
    bool singlePlayer = true;

    void loadConfig(const std::string& configPath)
    {
        if (!fs::exists(configPath)) {
            saveConfig();
            return;
        }

        try {
            std::ifstream file(configPath);
            nlohmann::json j;
            file >> j;

            version = j["version"];
            port = j["port"];
            singlePlayer = j["singlePlayer"];
        } catch (const std::exception& e) {
            saveConfig();
            DEBUG_CHECK(false, e.what());
        }
    }

    void saveConfig()
    {
        nlohmann::json j;
        j["version"] = version;
        j["port"] = port;
        j["singlePlayer"] = singlePlayer;

        std::ofstream file(configPath);
        file << j.dump(4);
    }
};

}

#endif /* SERVERCONFIG_HPP_ */
