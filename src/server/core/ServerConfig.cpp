#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

#include "common/utils/Debug.hpp"
#include "server/core/ServerConfig.hpp"

namespace fs = std::filesystem;

// init default config
u16 ServerConfig::server_port = 28818;
std::string ServerConfig::server_name = "Unnamed Server";
std::string ServerConfig::log_level = "info";
s64 ServerConfig::uuid_worker_id = 1;
s64 ServerConfig::uuid_datacenter_id = 1;
s64 ServerConfig::uuid_twepoch = 687888001020L;

void ServerConfig::loadConfigFromFile(const char* filepath)
{
    if (fs::exists(filepath)) {
        try {
            std::ifstream file(filepath);
            nlohmann::json json;
            file >> json;

            server_port = json["server_port"];
            server_name = json["server_name"];

            log_level = json["log_level"];

            uuid_worker_id = json["uuid_worker_id"];
            uuid_datacenter_id = json["uuid_datacenter_id"];
            uuid_twepoch = json["uuid_twepoch"];
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to load config file: " + std::string(e.what()));
        }
    } else {
        throw std::runtime_error("Config file not found: " + std::string(filepath));
    }
}

void ServerConfig::saveConfigToFile(const char* filepath)
{
    try {
        nlohmann::json json;

        json["server_port"] = server_port;
        json["server_name"] = server_name;

        json["log_level"] = log_level;

        json["uuid_worker_id"] = uuid_worker_id;
        json["uuid_datacenter_id"] = uuid_datacenter_id;
        json["uuid_twepoch"] = uuid_twepoch;

        std::ofstream file(filepath);
        file << json.dump(4);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to save config file: " + std::string(e.what()));
    }
}
