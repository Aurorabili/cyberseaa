#include "server/core/network/SocketServer.hpp"

#include <memory>

#include <asio/connect.hpp>

#include "common/Buffer.hpp"
#include "common/Debug.hpp"
#include "common/Types.hpp"
#include "common/utils/IntTypes.hpp"
#include "common/utils/String.hpp"
#include "server/core/Worker.hpp"
#include "server/core/network/BaseSocket.hpp"
#include "server/core/network/TcpSocket.hpp"
#include "server/core/services/Service.hpp"

using namespace Core;

SocketServer::SocketServer(Server* s, Worker* w, asio::io_context& ioctx)
    : m_server(s)
    , m_worker(w)
    , m_ioc(ioctx)
    , m_timer(ioctx)
{
    timeout();
}

bool SocketServer::try_open(const std::string& host, uint16_t port, bool is_connect)
{
    try {
        tcp::resolver resolver { m_ioc };
        if (is_connect) {
            tcp::socket sock(m_ioc);
            asio::connect(sock, resolver.resolve(host, std::to_string(port)));
            sock.close();
            return true;
        } else {
            tcp::endpoint endpoint = *resolver.resolve(host, std::to_string(port)).begin();
            tcp::acceptor acceptor { m_ioc };
            acceptor.open(endpoint.protocol());
#if TARGET_PLATFORM != PLATFORM_WINDOWS
            acceptor.set_option(tcp::acceptor::reuse_address(true));
#endif
            acceptor.bind(endpoint);
            acceptor.close();
            return true;
        }
    } catch (const asio::system_error& e) {
        logError("%s:%d %s(%d)", host.data(), port, e.what(), e.code().value());
        return false;
    }
}

std::pair<u32, tcp::endpoint> SocketServer::listen(const std::string& host, uint16_t port, u32 owner, uint8_t type)
{
    try {
        auto ctx = std::make_shared<SocketServer::acceptor_context>(type, owner, m_ioc);
        tcp::resolver resolver(m_ioc);
        tcp::endpoint endpoint = *resolver.resolve(host, std::to_string(port)).begin();
        ctx->acceptor.open(endpoint.protocol());
#if TARGET_PLATFORM != PLATFORM_WINDOWS
        ctx->acceptor.set_option(tcp::acceptor::reuse_address(true));
#endif
        ctx->acceptor.bind(endpoint);
        ctx->acceptor.listen(std::numeric_limits<int>::max());
        ctx->reset_reserve();
        auto id = m_server->nextFd();
        ctx->fd = id;
        m_acceptors.emplace(id, ctx);
        return std::make_pair(id, ctx->acceptor.local_endpoint());
    } catch (const asio::system_error& e) {
        logError("%s:%d %s(%d)", host.data(), port, e.what(), e.code().value());
        return { 0, tcp::endpoint {} };
    }
}

u32 SocketServer::udp_open(u32 owner, std::string_view host, uint16_t port)
{
    try {
        udp_context_ptr_t ctx;
        if (host.empty()) {
            ctx = std::make_shared<SocketServer::udp_context>(owner, m_ioc);
        } else {
            udp::resolver resolver(m_ioc);
            udp::endpoint endpoint = *resolver.resolve(host, std::to_string(port)).begin();
            ctx = std::make_shared<SocketServer::udp_context>(owner, m_ioc, endpoint);
        }
        auto id = m_server->nextFd();
        ctx->fd = id;
        doReceive(ctx);
        m_udps.emplace(id, std::move(ctx));
        return id;
    } catch (const asio::system_error& e) {
        logError("%s:%d %s(%d)", host.data(), port, e.what(), e.code().value());
        return 0;
    }
}

bool SocketServer::udp_connect(u32 fd, std::string_view host, uint16_t port)
{
    try {
        if (auto iter = m_udps.find(fd); iter != m_udps.end()) {
            udp::resolver resolver(m_ioc);
            udp::endpoint endpoint = *resolver.resolve(host, std::to_string(port)).begin();
            iter->second->sock.connect(endpoint);
            return true;
        }
        return false;
    } catch (const asio::system_error& e) {
        logError("%s:%d %s(%d)", host.data(), port, e.what(), e.code().value());
        return false;
    }
}

bool SocketServer::accept(u32 fd, s32 sessionid, u32 owner)
{
    auto iter = m_acceptors.find(fd);
    if (iter == m_acceptors.end()) {
        return false;
    }

    auto& ctx = iter->second;
    if (!ctx->acceptor.is_open()) {
        return false;
    }

    Worker* w = m_server->getWorker(0, owner);
    if (nullptr == w)
        return false;

    auto c = w->socket_server().makeConnection(owner, ctx->type, tcp::socket(w->io_context()));

    ctx->acceptor.async_accept(c->socket(), [this, ctx, c, w, sessionid, owner](const asio::error_code& e) {
        if (!e) {
            if (!ctx->reserve.is_open()) {
                c->socket().close();
                ctx->reset_reserve();
                auto ec = asio::error::make_error_code(asio::error::no_descriptors);
                response(ctx->fd, ctx->owner, utils::format("SocketServer::accept %s(%d)", ec.message().data(), ec.value()),
                    sessionid, PTYPE_ERROR);
            } else {
                c->fd(m_server->nextFd());
                w->socket_server().addConnection(this, ctx, c, sessionid);
            }
        } else {
            if (e == asio::error::operation_aborted)
                return;

            if (e == asio::error::no_descriptors) {
                if (ctx->reserve.is_open()) {
                    ctx->reserve.close();
                }
            }

            response(ctx->fd, ctx->owner, utils::format("SocketServer::accept %s(%d)", e.message().data(), e.value()),
                sessionid, PTYPE_ERROR);
        }

        if (sessionid == 0) {
            accept(ctx->fd, sessionid, owner);
        }
    });
    return true;
}

u32 SocketServer::connect(const std::string& host, uint16_t port, u32 owner, uint8_t type, s32 sessionid, u32 millseconds)
{
    if (0 == sessionid) {
        try {
            tcp::resolver resolver(m_ioc);
            tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));
            auto c = makeConnection(owner, type, tcp::socket(m_ioc));
            asio::connect(c->socket(), endpoints);
            c->fd(m_server->nextFd());
            m_connections.emplace(c->fd(), c);
            asio::post(m_ioc, [c]() { c->start(BaseSocket::Role::CLIENT); });
            return c->fd();
        } catch (const asio::system_error& e) {
            logWarning("connect %s:%d failed: %s(%d)", host.data(), port, e.code().message().data(), e.code().value());
        }
    } else {
        std::shared_ptr<tcp::resolver> resolver = std::make_shared<tcp::resolver>(m_ioc);
        resolver->async_resolve(host, std::to_string(port),
            [this, millseconds, type, owner, sessionid, host, port, resolver](const asio::error_code& ec, tcp::resolver::results_type results) {
                if (!ec) {
                    auto c = makeConnection(owner, type, tcp::socket(m_ioc));
                    std::shared_ptr<asio::steady_timer> timer;
                    if (millseconds > 0) {
                        timer = std::make_shared<asio::steady_timer>(m_ioc);
                        timer->expires_after(std::chrono::milliseconds(millseconds));
                        timer->async_wait([this, c, owner, sessionid, host, port, timer](const asio::error_code& ec) {
                            if (!ec) {
                                // The timer may have expired, but the callback function has not yet been called(asio's complete-queue).(0 == timer->cancel()).
                                // Only trigger error code when socket not connected :
                                if (c->fd() == 0) {
                                    c->close();
                                    response(0, owner, utils::format("connect %s:%d timeout", host.data(), port), sessionid, PTYPE_ERROR);
                                }
                            }
                        });
                    }

                    asio::async_connect(c->socket(), results,
                        [this, c, host, port, owner, sessionid, timer](const asio::error_code& ec, const tcp::endpoint&) {
                            size_t cancelled_timer = 1;
                            if (timer) {
                                try {
                                    cancelled_timer = timer->cancel();
                                } catch (...) {
                                }
                            }

                            if (!ec) {
                                c->fd(m_server->nextFd());
                                m_connections.emplace(c->fd(), c);
                                c->start(BaseSocket::Role::CLIENT);
                                response(0, owner, std::to_string(c->fd()), sessionid, PTYPE_INTEGER);
                            } else {
                                if (cancelled_timer > 0)
                                    response(0, owner, utils::format("connect %s:%d %s(%d)", host.data(), port, ec.message().data(), ec.value()), sessionid, PTYPE_ERROR);
                            }
                        });
                } else {
                    response(0, owner, utils::format("resolve %s:%d %s(%d)", host.data(), port, ec.message().data(), ec.value()), sessionid, PTYPE_ERROR);
                }
            });
    }
    return 0;
}

void SocketServer::read(u32 fd, u32 owner, size_t n, std::string_view delim, s32 sessionid)
{
    if (auto iter = m_connections.find(fd); iter != m_connections.end()) {
        iter->second->read(n, delim, sessionid);
        return;
    }

    asio::post(m_ioc, [this, owner, sessionid]() {
        response(0, owner, "socket.read: closed", sessionid, PTYPE_ERROR);
    });
}

bool SocketServer::write(u32 fd, std::shared_ptr<Buffer>&& data, BufferFlag flag)
{
    if (nullptr == data || 0 == data->size())
        return false;

    if (auto iter = m_connections.find(fd); iter != m_connections.end()) {
        data->set_flag(flag);
        return iter->second->send(std::move(data));
    }

    if (auto iter = m_udps.find(fd); iter != m_udps.end()) {
        iter->second->sock.async_send(
            asio::buffer(data->data(), data->size()),
            [data, ctx = iter->second](std::error_code /*ec*/, std::size_t /*bytes_sent*/) {
            });
        return true;
    }
    return false;
}

bool SocketServer::closeConnection(u32 fd)
{
    if (auto iter = m_connections.find(fd); iter != m_connections.end()) {
        iter->second->close();
        m_connections.erase(iter);
        m_server->unlockFd(fd);
        return true;
    }

    if (auto iter = m_udps.find(fd); iter != m_udps.end()) {
        iter->second->closed = true;
        iter->second->sock.close();
        m_udps.erase(iter);
        m_server->unlockFd(fd);
        return true;
    }

    if (auto iter = m_acceptors.find(fd); iter != m_acceptors.end()) {
        iter->second->close();
        m_acceptors.erase(iter);
        m_server->unlockFd(fd);
        return true;
    }
    return false;
}

void SocketServer::closeAllConnections()
{
    for (auto& s : m_connections) {
        s.second->close();
    }

    for (auto& s : m_udps) {
        s.second->sock.close();
    }

    for (auto& ac : m_acceptors) {
        ac.second->close();
    }
}

bool SocketServer::setTimeout(u32 fd, u32 seconds)
{
    if (auto iter = m_connections.find(fd); iter != m_connections.end()) {
        iter->second->settimeout(seconds);
        return true;
    }
    return false;
}

bool SocketServer::setNodelay(u32 fd)
{
    if (auto iter = m_connections.find(fd); iter != m_connections.end()) {
        return iter->second->set_no_delay();
    }
    return false;
}

bool SocketServer::setEnableChunked(u32 fd, std::string_view flag)
{
    auto v = EnableChunkedEnum::NONE;
    for (const auto& c : flag) {
        switch (c) {
        case 'r':
        case 'R':
            v = v | EnableChunkedEnum::RECEIVE;
            break;
        case 'w':
        case 'W':
            v = v | EnableChunkedEnum::SEND;
            break;
        default:
            logWarning("tcp::set_enable_chunked Unsupported enable chunked flag %s.Support: 'r' 'w'.", flag.data());
            return false;
        }
    }

    // if (auto iter = m_connections.find(fd); iter != m_connections.end()) {
    //     auto c = std::dynamic_pointer_cast<utils_connection>(iter->second);
    //     if (c) {
    //         c->set_enable_chunked(v);
    //         return true;
    //     }
    // }
    return false;
}

bool SocketServer::setSendQueueLimit(u32 fd, u32 warnsize, u32 errorsize)
{
    if (auto iter = m_connections.find(fd); iter != m_connections.end()) {
        iter->second->set_send_queue_limit(warnsize, errorsize);
        return true;
    }
    return false;
}

static bool decode_endpoint(std::string_view address, udp::endpoint& ep)
{
    if (address[0] != '4' && address[0] != '6')
        return false;
    asio::ip::port_type port = 0;
    if (address[0] == '4') {
        address = address.substr(1);
        asio::ip::address_v4::bytes_type bytes;
        if ((address.size()) != bytes.size() + sizeof(port))
            return false;
        memcpy(bytes.data(), address.data(), bytes.size());
        memcpy(&port, address.data() + bytes.size(), sizeof(port));
        ep = udp::endpoint(asio::ip::address(asio::ip::make_address_v4(bytes)), port);
    } else {
        address = address.substr(1);
        asio::ip::address_v6::bytes_type bytes;
        if ((address.size()) != bytes.size() + sizeof(port))
            return false;
        memcpy(bytes.data(), address.data(), bytes.size());
        memcpy(&port, address.data() + bytes.size(), sizeof(port));
        ep = udp::endpoint(asio::ip::address(asio::ip::make_address_v6(bytes)), port);
    }
    return true;
}

bool SocketServer::sendTo(u32 host, std::string_view address, std::shared_ptr<Buffer>&& data)
{
    if (address.empty())
        return false;
    if (auto iter = m_udps.find(host); iter != m_udps.end()) {
        udp::endpoint ep;
        if (!decode_endpoint(address, ep))
            return false;
        iter->second->sock.async_send_to(
            asio::buffer(data->data(), data->size()), ep,
            [data, ctx = iter->second](std::error_code /*ec*/, std::size_t /*bytes_sent*/) {
            });
        return true;
    }
    return false;
}

std::string SocketServer::getConnectionAddress(u32 fd)
{
    if (auto iter = m_connections.find(fd); iter != m_connections.end()) {
        return iter->second->address();
    }
    return std::string();
}

bool SocketServer::switchType(u32 fd, uint8_t new_type)
{
    if (auto iter = m_connections.find(fd); iter != m_connections.end()) {
        auto c = iter->second;
        if (c->type() != PTYPE_SOCKET_TCP)
            return false;
        auto new_session = makeConnection(c->owner(), new_type, std::move(c->socket()));
        new_session->fd(fd);
        iter->second = new_session;
        new_session->start(c->role());
        return true;
    }
    return false;
}

size_t SocketServer::encodeEndpoint(char* buf, const asio::ip::address& addr, asio::ip::port_type port)
{
    size_t size = 0;
    if (addr.is_v4()) {
        buf[0] = '4';
        size += 1;
        auto bytes = addr.to_v4().to_bytes();
        memcpy(buf + size, bytes.data(), bytes.size());
        size += bytes.size();
    } else if (addr.is_v6()) {
        buf[0] = '6';
        size += 1;
        auto bytes = addr.to_v6().to_bytes();
        memcpy(buf + size, bytes.data(), bytes.size());
        size += bytes.size();
    }
    memcpy(buf + size, &port, sizeof(port));
    size += sizeof(port);
    return size;
}

ConnectionPtr SocketServer::makeConnection(u32 serviceid, u8 type, tcp::socket&& sock)
{
    ConnectionPtr session;
    switch (type) {
    case PTYPE_SOCKET_TCP: {
        session = std::make_shared<TcpSocket>(serviceid, type, this, std::move(sock));
        break;
    }
    case PTYPE_SOCKET_WS: {
        session = std::make_shared<BaseSocket>(serviceid, type, this, std::move(sock));
        break;
    }
    default:
        DEBUG_ASSERT(false, "Unknown socket protocol");
        break;
    }
    return session;
}

void SocketServer::response(u32 sender, u32 receiver, std::string_view content, s32 sessionid, uint8_t type)
{
    if (0 == sessionid) {
        logError("%s", std::string(content).data());
        return;
    }
    m_response.setSender(sender);
    m_response.setReceiver(0);
    m_response.buffer()->clear();
    m_response.writer(content);
    m_response.setSessionId(sessionid);
    m_response.setType(type);

    handleMessage(receiver, m_response);
}

void SocketServer::addConnection(SocketServer* from, const acceptor_context_ptr_t& ctx, const std::shared_ptr<BaseSocket>& c, s32 sessionid)
{
    asio::dispatch(m_ioc, [this, from, ctx, c, sessionid] {
        m_connections.emplace(c->fd(), c);
        c->start(BaseSocket::Role::SERVER);

        if (sessionid != 0) {
            asio::dispatch(from->m_ioc, [from, ctx, sessionid, fd = c->fd()] {
                from->response(ctx->fd, ctx->owner, std::to_string(fd), sessionid, PTYPE_INTEGER);
            });
        }
    });
}

Service* SocketServer::findService(u32 serviceid)
{
    return m_worker->find_service(serviceid);
    ;
}

void SocketServer::timeout()
{
    m_timer.expires_after(std::chrono::seconds(10));
    m_timer.async_wait([this](const asio::error_code& e) {
        if (e) {
            return;
        }

        auto now = BaseSocket::now();
        for (auto& connection : m_connections) {
            connection.second->timeout(now);
        }
        timeout();
    });
}

void SocketServer::doReceive(const udp_context_ptr_t& ctx)
{
    if (ctx->closed)
        return;

    auto buf = ctx->msg.buffer();
    buf->clear();
    auto space = buf->prepare(udp_context::READ_BUFFER_SIZE);
    ctx->sock.async_receive_from(
        asio::buffer(space.first, space.second), ctx->from_ep,
        [this, ctx](std::error_code ec, std::size_t bytes_recvd) {
            if (!ec && bytes_recvd > 0) {
                auto buf = ctx->msg.buffer();
                buf->commit(bytes_recvd);
                char arr[32];
                size_t size = encodeEndpoint(arr, ctx->from_ep.address(), ctx->from_ep.port());
                buf->write_front(arr, size);
                ctx->msg.setSender(ctx->fd);
                ctx->msg.setReceiver(0);
                ctx->msg.setType(PTYPE_SOCKET_UDP);
                handleMessage(ctx->owner, ctx->msg);
            }
            doReceive(ctx);
        });
}
