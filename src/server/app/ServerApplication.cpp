#include "server/app/ServerApplication.hpp"

#include <filesystem>
#include <memory>

#include "common/Exception.hpp"
#include "server/core/Server.hpp"
#include "server/core/Service.hpp"
#include "server/core/Worker.hpp"
#include "server/services/Bootstrap/BootstrapService.hpp"
#include "server/services/Echo/EchoService.hpp"

static std::weak_ptr<Core::Server> m_wkServer;

void App::ServerApplication::loadConfig(const std::string& configPath)
{
    m_config.loadConfig(configPath);
    m_configPath = configPath;
}

void App::ServerApplication::saveConfig()
{
    m_config.saveConfig();
}

u16 App::ServerApplication::port() const
{
    return m_config.port;
}

u16 App::ServerApplication::retryCount() const
{
    return m_retryCount;
}

void App::ServerApplication::run()
{
    uint32_t thread_count = std::thread::hardware_concurrency();

    int exitcode = -1;

    if (m_isRunning) {
        return;
    }

    DEBUG_CHECK(m_retryCount >= m_maxRetryCount, "Retry count is greater than max retry count");

    if (m_config.port == 0) {
        loadConfig(m_configPath);
    }

    std::time_t t = std::time(nullptr);
    std::tm* tm = std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%d_%H-%M-%S");
    std::string str_time = oss.str();
    std::filesystem::path logPath = std::filesystem::current_path() / "logs" / (str_time + ".log");

    std::shared_ptr<Core::Server> _server = std::make_shared<Core::Server>();
    m_wkServer = _server;

    _server->registerService("BootstrapService", []() -> Core::ServicePtr {
        return std::make_unique<BootstrapService>();
    });

    _server->registerService("EchoService", []() -> Core::ServicePtr {
        return std::make_unique<EchoService>();
    });

    auto& logger = Log::instance();
    logger.set_level(LogLevel::Debug);
    logger.init(logPath);

    _server->init(thread_count);

    std::unique_ptr<Core::ServiceConfig> conf = std::make_unique<Core::ServiceConfig>();
    conf->type = "BootstrapService";
    conf->name = "BootstrapService";
    conf->unique = true;
    _server->newService(std::move(conf));
    _server->setUniqueService("BootstrapService", BOOTSTRAP_ADDR);

    exitcode = _server->run();
}