#ifndef SERVERAPPLICATION_HPP_
#define SERVERAPPLICATION_HPP_

#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>

#include "common/logger/LoggerHandler.hpp"
#include "common/utils/IntTypes.hpp"

#include "server/services/Service.hpp"

using asio::co_spawn;
using asio::detached;
using asio::io_context;

class ServerApplication {
public:
    bool init();

    void run();

    void setPort(u16 port) { m_port = port; }
    void setSinglePlayer(bool singlePlayer) { m_singlePlayer = singlePlayer; }

private:
    u16 m_port;
    bool m_singlePlayer;

    io_context m_ioContext;

    std::unordered_map<std::string, std::shared_ptr<Service>> m_services;
};

#endif /* SERVERAPPLICATION_HPP_ */