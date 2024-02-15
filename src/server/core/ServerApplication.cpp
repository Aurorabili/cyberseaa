#include "server/core/ServerApplication.hpp"

#include <filesystem>

#include "common/core/UUIDProvider.hpp"
#include "common/logger/LoggerHandler.hpp"
#include "common/utils/Debug.hpp"
#include "common/utils/Utils.hpp"
#include "server/core/ConsoleInput.hpp"
#include "server/core/ServerConfig.hpp"
#include "server/services/ConnectionService.hpp"

namespace fs = std::filesystem;

bool ServerApplication::init()
{
    ///* Initialize Random */
    std::srand((unsigned int)std::time(nullptr));

    ///* Load Config */
    ServerConfig::loadConfigFromFile("config.json");

    ///* Initialize Logger */
    fs::path log_path = "logs";
    if (!fs::exists(log_path)) {
        fs::create_directory(log_path);
    }

    auto log_filename = log_path / (utils::getCurrentTime("%Y-%m-%d_%H-%M-%S") + ".log");

    auto log_maxLevel = []() -> LogLevel {
        if (ServerConfig::log_level == "debug") {
            return LogLevel::Debug;
        } else if (ServerConfig::log_level == "info") {
            return LogLevel::Info;
        } else if (ServerConfig::log_level == "warn") {
            return LogLevel::Warning;
        } else if (ServerConfig::log_level == "error") {
            return LogLevel::Error;
        } else {
            logWarning() << "Unknown log level: " << ServerConfig::log_level << ", using info as default.";

            return LogLevel::Info;
        }
    }();

    LoggerHandler::getInstance().init(log_maxLevel, ServerConfig::server_name, log_filename.string());

    ///* Initialize UUID Provider */
    UUIDProvider::init(ServerConfig::uuid_worker_id, ServerConfig::uuid_datacenter_id, ServerConfig::uuid_twepoch);
    logDebug() << "UUID Provider initialized. Next UUID:" << UUIDProvider::nextUUID();

    ///* Initialize Connection Service */
    auto connectionService = std::make_shared<ConnectionService>(m_ioContext);
    connectionService->init(ServerConfig::server_port);
    m_services.emplace("ConnectionService", connectionService);

    return true;
}

void ServerApplication::run()
{
    bool isInitialized = init();

    for (auto& [name, service] : m_services) {
        co_spawn(m_ioContext, service->start(), detached);
    }

    ConsoleInput consoleInput(m_ioContext);

    logInfo() << "Server started.";

    m_ioContext.run();

    if (isInitialized) {
        logInfo() << "Stopping server...";
    }

    for (auto& [name, service] : m_services) {
        service->stop();
    }

    ServerConfig::saveConfigToFile("config.json");

    logInfo() << "Server stopped.";

    m_ioContext.stop();
    return;
}