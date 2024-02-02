#ifndef CONNECTIONSERVICE_HPP_
#define CONNECTIONSERVICE_HPP_

#include "common/utils/IntTypes.hpp"
#include "server/network/ClientManager.hpp"
#include "server/services/Service.hpp"

class ConnectionService : public Service, public std::enable_shared_from_this<ConnectionService> {
public:
    ConnectionService(io_context& ioContext);
    void init(u16 m_port);
    bool isInitialized() { return m_port != 0; }

    awaitable<void> start();
    void stop();

private:
    u16 m_port = 0;
    bool m_isRunning = false;

    ClientManager m_clientManager;
    io_context& m_ioContext;
};

#endif /* CONNECTIONSERVICE_HPP_ */