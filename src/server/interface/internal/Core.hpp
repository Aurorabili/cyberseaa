#pragma once

#include <memory>
#include <string>

#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/post.hpp>

#include "common/Buffer.hpp"
#include "common/Constant.hpp"
#include "common/Debug.hpp"
#include "common/utils/IntTypes.hpp"
#include "server/core/Message.hpp"
#include "server/core/Server.hpp"
#include "server/core/Worker.hpp"
#include "server/core/services/InternalService.hpp"
#include "server/core/services/Service.hpp"

namespace Interface {
namespace Internal {

    static asio::awaitable<void> removeService(Core::InternalService* s, u32 serviceId)
    {
        auto sessionId = s->makeSession();
        s->server()->removeService(serviceId, s->id(), sessionId);
        co_return;
    }

    static asio::awaitable<u32> createService(Core::InternalService* s, std::unique_ptr<Core::ServiceConfig> conf)
    {
        s32 sessionId = s->makeSession();
        conf->session = sessionId;
        conf->creator = s->id();
        s->server()->newService(std::move(conf));
        auto res = co_await s->wait<PTYPE_INTEGER>(sessionId);

        if (!res.has_value())
            co_return 0;
        co_return res.value();
    }

    static void async(Core::InternalService* s, std::function<asio::awaitable<void>()> fn)
    {
        asio::co_spawn(s->worker()->io_context(), fn, asio::detached);
    }

    static void sendMessage(Core::InternalService* s, u32 receiver, const std::string& data, u8 type = Common::PTYPE_TEXT)
    {
        auto msgBuffer = Common::Buffer::make_unique(data.size());
        msgBuffer->write_back(data.c_str(), data.size());
        s->server()->send(s->id(), receiver, msgBuffer, 0, type);
    }

    static u32 asioListen(Core::InternalService* s, const std::string& host, u16 port, u8 type)
    {
        auto& sock = s->worker()->socket_server();
        auto res = sock.listen(host, port, s->id(), type);

        return res.first;
    }

    static asio::awaitable<u32> asioAccept(Core::InternalService* s, u32 fd, u32 owner)
    {
        auto sessionId = s->makeSession();
        auto& sock = s->worker()->socket_server();
        auto res = sock.accept(fd, sessionId, owner);

        if (!res) {
            co_return 0;
        } else {
            auto res = co_await s->wait<PTYPE_INTEGER>(sessionId);

            if (!res.has_value())
                co_return 0;

            co_return res.value();
        }
    }

    static u32 asioConnect(Core::InternalService* s, const std::string& host, u16 port, u8 type, s32 sessionId, u32 timeout)
    {
        auto& sock = s->worker()->socket_server();
        auto fd = sock.connect(host, port, s->id(), type, sessionId, timeout);
        return fd;
    }

    static bool asioSend(Core::InternalService* s, u32 fd, std::shared_ptr<Buffer> data)
    {
        auto& sock = s->worker()->socket_server();
        auto res = sock.write(fd, std::move(data));
        return res;
    }

    static asio::awaitable<std::unique_ptr<Buffer>> asioRead(Core::InternalService* s, u32 fd)
    {
        auto sessionId = s->makeSession();
        s->worker()->socket_server().read(fd, s->id(), 0, "\n", sessionId);
        auto res = co_await s->wait<PTYPE_SOCKET_TCP>(sessionId);
        if (!res.has_value())
            co_return nullptr;
        co_return std::move(res.value());
    }

    static void asioClose(Core::InternalService* s, u32 fd)
    {
        auto& sock = s->worker()->socket_server();
        sock.closeConnection(fd);
        return;
    }

} // namespace internal
} // namespace Interface
