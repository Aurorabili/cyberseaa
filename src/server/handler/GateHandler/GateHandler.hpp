#ifndef GATEHANDLER_HPP_
#define GATEHANDLER_HPP_

#include "server/core/services/Handler.hpp"
#include "server/core/services/InternalService.hpp"
#include "server/interface/internal/Core.hpp"

using namespace Interface::Internal;

class GateHandler final : public Core::Handler {
public:
    GateHandler(Core::InternalService* _is)
        : Core::Handler(_is)
    {
    }

    void start()
    {
        auto fd = asioListen(m_service, "127.0.0.1", 28818, PTYPE_SOCKET_TCP);
        asioAccept(m_service, fd, 0, m_service->id());
    }
};

#endif // GATEHANDLER_HPP_