#pragma once

#include <asio/awaitable.hpp>
#include <asio/post.hpp>
#include <memory>
#include <string>

#include "common/Buffer.hpp"
#include "common/Constant.hpp"
#include "common/utils/IntTypes.hpp"
#include "server/core/Server.hpp"
#include "server/core/Worker.hpp"
#include "server/core/services/InternalService.hpp"

namespace Interface {
namespace Internal {
    static void async(Core::InternalService* s, std::function<asio::awaitable<void>()> fn)
    {
        asio::post(s->worker()->io_context(), fn);
    }

    static void sendMessage(Core::InternalService* s, u32 receiver, s32 sessionid, const std::string& data)
    {
        auto msgBuffer = Common::Buffer::make_unique(data.size());
        msgBuffer->write_back(data.c_str(), data.size());
        s->server()->send(s->id(), receiver, msgBuffer, sessionid, Common::PTYPE_TEXT);
    }

    static u32 asioListen(Core::InternalService* s, const std::string& host, u16 port, u8 type)
    {
        auto& sock = s->worker()->socket_server();
        auto res = sock.listen(host, port, s->id(), type);

        return res.first;
    }

    static bool asioAccept(Core::InternalService* s, u32 fd, s32 sessionId, u32 owner)
    {
        auto& sock = s->worker()->socket_server();
        auto res = sock.accept(fd, sessionId, owner);
        return res;
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
        std::string address = sock.getConnectionAddress(fd);
        auto res = sock.sendTo(fd, address, std::move(data));
        return res;
    }

} // namespace internal
} // namespace Interface
