#include "server/handler/PlayerHandler/PlayerHandler.hpp"

#include "common/Debug.hpp"
#include "server/interface/internal/Core.hpp"
#include <asio/awaitable.hpp>

using namespace Interface::Internal;

asio::awaitable<void> PlayerHandler::PlayerHandler::start()
{
    logInfo("PlayerHandler started: fd: %d", m_fd);

    while (true) {
        auto msg = co_await asioRead(m_service, m_fd);

        if (msg == nullptr) {
            logDebug("PlayerHandler: connection[%d] closed.", m_fd);
            asioClose(m_service, m_fd);
            co_await removeService(m_service, m_service->id());
            co_return;
        }

        auto response = Common::Buffer::make_shared();
        response->write_back(msg->data(), msg->size());
        asioSend(m_service, m_fd, response);
    }
}
