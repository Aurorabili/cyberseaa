#ifndef INTERFACE_CORE_HPP_
#define INTERFACE_CORE_HPP_

#include <memory>

#include <asio/awaitable.hpp>

#include "common/Buffer.hpp"
#include "common/Constant.hpp"
#include "common/Debug.hpp"
#include "server/core/Message.hpp"
#include "server/core/Service.hpp"
#include "server/core/Session.hpp"
#include "server/core/Worker.hpp"

static std::shared_ptr<Session<std::shared_ptr<Core::Message>>> makeSession(Core::Service* service)
{
    s32 sessionId = service->nextSessionId();
    auto& ctx = service->worker()->io_context();
    auto newSession = std::make_shared<Session<std::shared_ptr<Core::Message>>>(sessionId, ctx);
    service->addSession(sessionId, newSession);
    return newSession;
}

static asio::awaitable<std::shared_ptr<Core::Message>> callService(Core::Service* s, u32 receiverId, const std::string& cmd)
{
    auto session = makeSession(s);
    auto buf = Buffer::make_unique(cmd.size());
    buf->write_back(cmd.data(), cmd.size());
    s->server()->send(s->id(), receiverId, buf, session->id(), PTYPE_COMMAND);
    co_return co_await session->wait();
}

static void sendMessageToService(Core::Service* s, u32 receiverId, std::unique_ptr<Buffer>&& data, u32 sessionId, u8 type)
{
    s->server()->send(s->id(), receiverId, data, sessionId, type);
    return;
}

static asio::awaitable<u32> newService(Core::Service* service, std::unique_ptr<Core::ServiceConfig>&& conf)
{
    auto session = makeSession(service);
    conf->session = session->id();
    conf->creator = service->id();
    service->server()->newService(std::move(conf));
    auto msg = co_await session->wait();
    if (msg->type() == PTYPE_INTEGER) {
        co_return std::stoul(msg->data());
    } else {
        logError("Failed to create new service: %s", msg->data());
        co_return 0;
    }
}

static void closeService(Core::Service* service, u32 id)
{
    service->server()->removeService(id, service->id(), 0);
    auto buf = Buffer::make_unique();
    sendMessageToService(service, id, std::move(buf), 0, PTYPE_SHUTDOWN);
    return;
}

static u32 asioListen(Core::Service* service, const std::string& host, u16 port, u8 type)
{
    auto& ss = service->worker()->socket_server();
    auto rst = ss.listen(host, port, service->id(), type);
    return rst.first;
}

static asio::awaitable<u32> asioAccept(Core::Service* s, u32 fd, u32 owner)
{
    auto session = makeSession(s);
    auto& sock = s->worker()->socket_server();
    sock.accept(fd, s->id(), session->id(), owner);
    auto msg = co_await session->wait();
    if (msg->type() == PTYPE_INTEGER) {
        co_return std::stoul(msg->data());
    } else {
        logError("Failed to accept new connection: %s", msg->data());
        co_return 0;
    }
}

static void asioSend(Core::Service* s, u32 fd, std::shared_ptr<Buffer>&& data)
{
    auto& sock = s->worker()->socket_server();
    sock.write(fd, std::move(data));
    return;
}

static asio::awaitable<std::unique_ptr<Buffer>> asioRecv(Core::Service* s, u32 owner, u32 fd, size_t n, std::string_view delim)
{
    auto session = makeSession(s);
    auto& sock = s->worker()->socket_server();
    sock.read(fd, owner, n, delim, session->id());

    auto msg = co_await session->wait();
    if (msg->type() == PTYPE_SOCKET_TCP) {
        auto buf = Buffer::make_unique(msg->size());
        buf->write_back(msg->data(), msg->size());
        co_return std::move(buf);
    } else {
        logError("Failed to receive data: %s", msg->data());
        co_return nullptr;
    }
}

#endif /* INTERFACE_CORE_HPP_ */