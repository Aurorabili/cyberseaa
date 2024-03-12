#ifndef PLAYERSERVICE_HPP_
#define PLAYERSERVICE_HPP_

#include <asio/awaitable.hpp>
#include <string>

#include "server/core/services/Handler.hpp"
#include "server/core/services/InternalService.hpp"
#include "server/interface/internal/Core.hpp"

class PlayerHandler final : public Core::Handler {
public:
    PlayerHandler(Core::InternalService* service)
        : Core::Handler(service)
    {
        Interface::Internal::async(service, [this]() -> asio::awaitable<void> {
            start();
            co_return;
        });
    }

    void start();

private:
    std::string m_playerName;
};

#endif // PLAYERSERVICE_HPP_