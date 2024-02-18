#include "server/core/ServerApplication.hpp"

#include <filesystem>
#include <memory>

#include "common/core/UUIDProvider.hpp"
#include "common/logger/LoggerHandler.hpp"
#include "common/utils/Debug.hpp"
#include "common/utils/Utils.hpp"
#include "server/core/MessageBus.hpp"
#include "server/core/ServerConfig.hpp"
#include "server/services/ConnectionService.hpp"
#include "server/services/EchoService.hpp"

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
    auto connectionService = std::make_shared<ConnectionService>(m_threadPool, m_messageBus);
    connectionService->init(ServerConfig::server_port);
    m_services.emplace(connectionService->getName(), connectionService);

    ///* Initialize EchoService */
    auto echoService = std::make_shared<EchoService>(m_threadPool);
    m_services.emplace(echoService->getName(), echoService);

    ///* Register Console Commands */
    registerConsoleCommand("stop", [this](const std::string&) {
        this->m_isRunning = false;
    });

    registerConsoleCommand("__debug_test_logger", [](const std::string&) {
        logDebug() << "Debug message";
        logInfo() << "Info message";
        logWarning() << "Warning message";
        logError() << "Error message";
    });
    return true;
}

void ServerApplication::run()
{
    bool isInitialized = init();

    for (auto& [name, service] : m_services) {
        co_spawn(m_threadPool.getIoContext(), service->start(), detached);
        logDebug() << name << "Service loaded.";
    }

    m_threadPool.post([this]() {
        while (m_isRunning) {
            m_messageBus.processOne();
        }
    });
    logInfo() << "Server started.";

    m_isRunning = true;
    m_threadPool.run();

    listenConsoleInput();

    if (isInitialized) {
        logInfo() << "Stopping server...";
    }

    for (auto& [name, service] : m_services) {
        service->stop();
    }

    ServerConfig::saveConfigToFile("config.json");

    logInfo() << "Server stopped.";

    m_threadPool.stop();
    return;
}

void ServerApplication::registerConsoleCommand(const std::string& command, CommandHandler handler)
{
    m_consoleCommandHandlers.emplace(command, handler);
}

void ServerApplication::listenConsoleInput()
{
    std::string line;

    while (m_isRunning) {
        std::getline(std::cin, line);
        logDebug() << "Console Input:" << line;
        auto it = m_consoleCommandHandlers.find(line);
        if (it != m_consoleCommandHandlers.end()) {
            it->second(line);
        } else {
            logError() << "Unknown command:" << line;
        }
    }
}