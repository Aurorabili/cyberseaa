#ifndef SERVERAPPLICATION_HPP_
#define SERVERAPPLICATION_HPP_

#include <memory>

#include "common/utils/IntTypes.hpp"
#include "server/app/ServerConfig.hpp"
#include "server/core/Server.hpp"

namespace App {

class ServerApplication {
public:
    ServerApplication() = default;
    ~ServerApplication() = default;

    ServerApplication(const ServerApplication&) = delete;
    ServerApplication& operator=(const ServerApplication&) = delete;
    ServerApplication(ServerApplication&&) = delete;
    ServerApplication& operator=(ServerApplication&&) = delete;

public:
    void loadConfig(const std::string& configPath);
    void saveConfig();
    u16 port() const;
    u16 retryCount() const;

public:
    void run();

private:
    bool m_isRunning = false;
    std::string m_configPath = "config.json";
    u16 m_retryCount = 0;
    u16 m_maxRetryCount = 0;
    ServerConfig m_config;
};
}; // namespace App
#endif /* SERVERAPPLICATION_HPP_ */