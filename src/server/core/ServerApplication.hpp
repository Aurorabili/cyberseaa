#ifndef SERVERAPPLICATION_HPP_
#define SERVERAPPLICATION_HPP_

#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>

#include "common/utils/IntTypes.hpp"

#include "server/core/ThreadPool.hpp"
#include "server/services/Service.hpp"

using asio::co_spawn;
using asio::detached;
using asio::io_context;

class ServerApplication {
public:
    using CommandHandler = std::function<void(const std::string&)>;

public:
    bool init();
    void run();

public:
    void setPort(u16 port) { m_port = port; }
    void setSinglePlayer(bool singlePlayer) { m_singlePlayer = singlePlayer; }

public:
    void registerConsoleCommand(const std::string& command, CommandHandler handler);
    void listenConsoleInput();

private:
    u16 m_port;
    bool m_singlePlayer;
    bool m_isRunning = false;
    io_context m_ioContext;

    ThreadPool m_threadPool;

    std::unordered_map<std::string, std::shared_ptr<Service>> m_services;

private:
    std::unordered_map<std::string, CommandHandler> m_consoleCommandHandlers;
};

#endif /* SERVERAPPLICATION_HPP_ */