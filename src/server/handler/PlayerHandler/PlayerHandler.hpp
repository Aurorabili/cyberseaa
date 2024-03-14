#ifndef PLAYERSERVICE_HPP_
#define PLAYERSERVICE_HPP_

#include <asio/awaitable.hpp>
#include <string>

#include "common/Debug.hpp"
#include "server/core/services/Handler.hpp"
#include "server/core/services/InternalService.hpp"
#include "server/interface/internal/Core.hpp"

class PlayerHandler final : public Core::Handler {
public:
    PlayerHandler(Core::InternalService* service)
        : Core::Handler(service)
    {
        m_service->registerCommand("setFd", [this](const std::string& data) {
            logDebug("PlayerHandler: setFd: %s", data.c_str());
            m_fd = std::stoi(data);
            Interface::Internal::async(m_service, [this]() -> asio::awaitable<void> {
                co_await start();
            });
        });
    }

    asio::awaitable<void> start();

private:
    std::string m_playerName;
    s32 m_fd;
};

#endif // PLAYERSERVICE_HPP_