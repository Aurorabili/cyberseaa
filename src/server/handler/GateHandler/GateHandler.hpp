#ifndef GATEHANDLER_HPP_
#define GATEHANDLER_HPP_

#include "common/Constant.hpp"
#include "server/core/services/Handler.hpp"
#include "server/core/services/InternalService.hpp"
#include "server/interface/internal/Core.hpp"
#include <asio/awaitable.hpp>

using namespace Interface::Internal;

class GateHandler final : public Core::Handler {
public:
    GateHandler(Core::InternalService* _is)
        : Core::Handler(_is)
    {
        Interface::Internal::async(_is, [this]() -> asio::awaitable<void> {
            co_await start();
        });
    }

    asio::awaitable<void> start()
    {
        auto listenfd = asioListen(m_service, "127.0.0.1", 28818, PTYPE_SOCKET_TCP);

        while (true) {
            auto conf = std::make_unique<Core::ServiceConfig>();
            conf->name = "PlayerHandler";
            conf->type = "internal";
            conf->threadid = m_service->worker()->id();
            auto serviceId = co_await createService(m_service, std::move(conf));

            auto fd = co_await asioAccept(m_service, listenfd, serviceId);
            if (fd == 0) {
                co_await removeService(m_service, serviceId);
                continue;
            }
            sendMessage(m_service, serviceId, "setFd " + std::to_string(fd), PTYPE_COMMAND);
        }
    };
};
#endif // GATEHANDLER_HPP_