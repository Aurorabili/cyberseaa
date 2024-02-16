#include "server/services/ConnectionService.hpp"
#include "common/utils/Debug.hpp"
#include "server/network/Session.hpp"

ConnectionService::ConnectionService(io_context& ioContext)
    : Service(ioContext)
{
    logDebug() << "ConnectionService created.";
}

void ConnectionService::init(u16 port)
{
    m_port = port;
    logDebug() << "ConnectionService initialized.";
}

awaitable<void> ConnectionService::start()
{
    if (!isInitialized()) {
        logDebug() << "ConnectionService not initialized.";
        co_return;
    }

    logInfo() << "Start listening on port" << m_port << ".";
    m_isRunning = true;
    tcp::acceptor acceptor(m_ioContext, { tcp::v4(), m_port });
    while (m_isRunning) {
        std::make_shared<Session>(co_await acceptor.async_accept(use_awaitable),
            m_clientManager)
            ->sessionStart();
    }

    co_return;
}

void ConnectionService::stop()
{
    m_isRunning = false;
    logDebug() << "ConnectionService stopped.";
}