#ifndef CONNECTIONSERVICE_HPP_
#define CONNECTIONSERVICE_HPP_

#include <memory>

#include "common/utils/IntTypes.hpp"
#include "server/core/MessageBus.hpp"
#include "server/network/ClientManager.hpp"
#include "server/services/Service.hpp"

#define _SERVICE_NAME "ConnectionService"

class ConnectionService : public Service, public std::enable_shared_from_this<ConnectionService> {
public:
    ConnectionService(ThreadPool& threadPool, MessageBus& messageBus)
        : Service(threadPool, _SERVICE_NAME)
        , m_clientManager(messageBus) {};
    void init(u16 m_port);
    bool isInitialized() { return m_port != 0; }

    awaitable<void> start() override;
    void stop() override;

private:
    u16 m_port = 0;
    bool m_isRunning = false;

    ClientManager m_clientManager;
};

#endif /* CONNECTIONSERVICE_HPP_ */