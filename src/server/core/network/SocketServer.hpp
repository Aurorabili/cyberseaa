#ifndef SOCKETSERVER_HPP_
#define SOCKETSERVER_HPP_

#include <memory>

#include <asio/io_context.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/ip/udp.hpp>

#include "common/Buffer.hpp"
#include "server/core/Message.hpp"
#include "server/core/Server.hpp"
#include "server/core/services/Service.hpp"

using asio::io_context;
using asio::ip::tcp;
using asio::ip::udp;

namespace Core {
class Worker;
class Service;
class BaseSocket;

using ConnectionPtr = std::shared_ptr<BaseSocket>;

class SocketServer {
public:
    struct acceptor_context {
        acceptor_context(uint8_t t, uint32_t o, asio::io_context& ioc)
            : type(t)
            , owner(o)
            , reserve(ioc)
            , acceptor(ioc)
        {
        }

        void close()
        {
            asio::error_code ec;
            reserve.close(ec);
            acceptor.cancel(ec);
            acceptor.close(ec);
        }

        void reset_reserve()
        {
            asio::error_code ec;
            reserve.open(tcp::v4(), ec);
        }

        uint8_t type;
        uint32_t owner;
        uint32_t fd = 0;
        tcp::socket reserve;
        tcp::acceptor acceptor;
    };

    static constexpr size_t addr_v4_size = 1 + sizeof(asio::ip::address_v4::bytes_type) + sizeof(asio::ip::port_type);
    static constexpr size_t addr_v6_size = 1 + sizeof(asio::ip::address_v6::bytes_type) + sizeof(asio::ip::port_type);

    struct udp_context {
        static constexpr size_t READ_BUFFER_SIZE = size_t { 2048 } - addr_v6_size;
        udp_context(uint32_t o, asio::io_context& ioc, udp::endpoint ep)
            : owner(o)
            , msg(READ_BUFFER_SIZE, static_cast<uint32_t>(addr_v6_size))
            , sock(ioc, ep)
        {
        }

        udp_context(uint32_t o, asio::io_context& ioc)
            : owner(o)
            , msg(READ_BUFFER_SIZE, static_cast<uint32_t>(addr_v6_size))
            , sock(ioc, udp::endpoint(udp::v4(), 0))
        {
        }

        bool closed = false;
        uint32_t owner;
        uint32_t fd = 0;
        Message msg;
        udp::socket sock;
        udp::endpoint from_ep;
    };

    using acceptor_context_ptr_t = std::shared_ptr<acceptor_context>;
    using udp_context_ptr_t = std::shared_ptr<udp_context>;

public:
    friend class BaseSocket;

    SocketServer(Server* s, Worker* w, asio::io_context& ioctx);

    SocketServer(const SocketServer&) = delete;

    SocketServer& operator=(const SocketServer&) = delete;

    bool try_open(const std::string& host, uint16_t port, bool is_connect = false);

    std::pair<uint32_t, tcp::endpoint> listen(const std::string& host, uint16_t port, uint32_t owner, uint8_t type);

    uint32_t udp_open(uint32_t owner, std::string_view host, uint16_t port);

    bool udp_connect(uint32_t fd, std::string_view host, uint16_t port);

    bool accept(uint32_t fd, int32_t sessionid, uint32_t owner);

    uint32_t connect(const std::string& host, uint16_t port, uint32_t owner, uint8_t type, int32_t sessionid, uint32_t millseconds = 0);

    void read(uint32_t fd, uint32_t owner, size_t n, std::string_view delim, int32_t sessionid);

    bool write(uint32_t fd, std::shared_ptr<Buffer>&& data, BufferFlag flag = BufferFlag::NONE);

    bool closeConnection(uint32_t fd);

    void closeAllConnections();

    bool setTimeout(uint32_t fd, uint32_t seconds);

    bool setNodelay(uint32_t fd);

    bool setEnableChunked(uint32_t fd, std::string_view flag);

    bool setSendQueueLimit(uint32_t fd, uint32_t warnsize, uint32_t errorsize);

    bool sendTo(uint32_t host, std::string_view address, std::shared_ptr<Buffer>&& data);

    std::string getConnectionAddress(uint32_t fd);

    bool switchType(uint32_t fd, uint8_t new_type);

    static size_t encodeEndpoint(char* buf, const asio::ip::address& addr, asio::ip::port_type port);

private:
    ConnectionPtr makeConnection(uint32_t serviceid, uint8_t type, tcp::socket&& sock);

    void response(uint32_t sender, uint32_t receiver, std::string_view data, int32_t sessionid, uint8_t type);

    void addConnection(SocketServer* from, const acceptor_context_ptr_t& ctx, const ConnectionPtr& c, int32_t sessionid);

    template <typename Message>
    void handleMessage(uint32_t serviceid, Message&& m);

    Service* findService(uint32_t serviceid);

    void timeout();

    void doReceive(const udp_context_ptr_t& ctx);

private:
    Server* m_server;
    Worker* m_worker;
    asio::io_context& m_ioc;
    asio::steady_timer m_timer;
    Message m_response;

    std::unordered_map<uint32_t, acceptor_context_ptr_t> m_acceptors;
    std::unordered_map<uint32_t, ConnectionPtr> m_connections;
    std::unordered_map<uint32_t, udp_context_ptr_t> m_udps;
};

template <typename Message>
void SocketServer::handleMessage(uint32_t serviceId, Message&& msg)
{
    auto s = findService(serviceId);
    if (nullptr == s) {
        closeConnection(msg.sender());
        return;
    }
    Core::handle_message(s, std::forward<Message>(msg));
}

};

#endif /* SOCKETSERVER_HPP_ */