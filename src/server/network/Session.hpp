#ifndef SESSION_HPP_
#define SESSION_HPP_

#include <deque>
#include <memory>

#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read_until.hpp>
#include <asio/redirect_error.hpp>
#include <asio/strand.hpp>
#include <asio/write.hpp>

#include "server/network/ClientInfo.hpp"
#include "server/network/ClientManager.hpp"

using asio::awaitable;
using asio::co_spawn;
using asio::detached;
using asio::redirect_error;
using asio::use_awaitable;
using asio::ip::tcp;

class Session : public ClientInfo, public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, ClientManager& clientManager)
        : m_socket(std::move(socket))
        , m_timer(m_socket.get_executor())
        , m_clientManager(clientManager)
    {
        m_timer.expires_at(std::chrono::steady_clock::time_point::max());
    }

    void sessionStart()
    {
        m_clientManager.addClient(shared_from_this());

        co_spawn(
            m_socket.get_executor(),
            [self = shared_from_this()] { return self->reader(); },
            detached);

        co_spawn(
            m_socket.get_executor(),
            [self = shared_from_this()] { return self->writer(); },
            detached);
    }

    void send(const std::string& msg)
    {
        m_msgs.push_back(msg);
        m_timer.cancel_one();
    }

private:
    awaitable<void> reader()
    {
        try {
            for (std::string read_msg;;) {
                std::size_t n = co_await asio::async_read_until(m_socket,
                    asio::dynamic_buffer(read_msg, 1024), "\n", use_awaitable);
                m_clientManager.onMessageReceived(shared_from_this(), read_msg.substr(0, n - 1));
                read_msg.erase(0, n);
            }
        } catch (std::exception&) {
            stop();
        }
    }

    awaitable<void> writer()
    {
        try {
            while (m_socket.is_open()) {
                if (m_msgs.empty()) {
                    asio::error_code ec;
                    co_await m_timer.async_wait(redirect_error(use_awaitable, ec));
                } else {
                    co_await asio::async_write(m_socket,
                        asio::buffer(m_msgs.front()), use_awaitable);
                    m_msgs.pop_front();
                }
            }
        } catch (std::exception&) {
            stop();
        }
    }

    void stop()
    {
        m_clientManager.removeClient(shared_from_this());
        m_socket.close();
        m_timer.cancel();
    }

private:
    tcp::socket m_socket;
    asio::steady_timer m_timer;
    ClientManager& m_clientManager;
    std::deque<std::string> m_msgs;
};

#endif