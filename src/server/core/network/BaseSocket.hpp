#ifndef BASESOCKET_HPP_
#define BASESOCKET_HPP_

#include <ctime>
#include <deque>
#include <memory>

#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/error_code.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read_until.hpp>
#include <asio/redirect_error.hpp>
#include <asio/strand.hpp>
#include <asio/write.hpp>

#include "common/Buffer.hpp"
#include "common/Debug.hpp"
#include "common/Types.hpp"
#include "common/utils/IntTypes.hpp"
#include "common/utils/String.hpp"
#include "server/core/Message.hpp"
#include "server/core/network/ConstBuffersHolder.hpp"
#include "server/core/network/Error.hpp"
#include "server/core/network/SocketServer.hpp"

using asio::awaitable;
using asio::co_spawn;
using asio::detached;
using asio::redirect_error;
using asio::use_awaitable;
using asio::ip::tcp;

namespace Core {
class BaseSocket : public std::enable_shared_from_this<BaseSocket> {
public:
    enum class Role {
        NONE,
        CLIENT,
        SERVER
    };

    template <typename... Args>
    explicit BaseSocket(u32 serviceId, u8 type, SocketServer* socketServer, Args&&... args)
        : m_serviceId(serviceId)
        , m_type(type)
        , m_socketServer(socketServer)
        , m_socket(std::forward<Args>(args)...)
    {
    }

    BaseSocket(const BaseSocket&) = delete;
    BaseSocket& operator=(const BaseSocket&) = delete;

    virtual ~BaseSocket() { }

    static time_t now()
    {
        return std::time(nullptr);
    }

    virtual void start(Role role)
    {
        m_role = role;
        m_recvtime = now();
    }

    virtual void read(size_t, std::string_view, int32_t)
    {
        logError("Unsupported read operation for PTYPE %d", (int)m_type);
        asio::post(m_socket.get_executor(), [this, self = shared_from_this()] {
            error(make_error_code(WebSocket::Error::INVALID_READ_OPERATION));
        });
    };

    virtual bool send(std::shared_ptr<Buffer>&& data)
    {
        if (data == nullptr || data->size() == 0) {
            return false;
        }

        if (!m_socket.is_open()) {
            return false;
        }

        if (m_wq_warn_size != 0 && m_queue.size() >= m_wq_warn_size) {
            logWarning("network send queue too long. size: %d", m_queue.size());
            if (m_wq_error_size != 0 && m_queue.size() >= m_wq_error_size) {
                co_spawn(
                    m_socket.get_executor(), [this, self = shared_from_this()]() -> awaitable<void> {
                        error(WebSocket::make_error_code(WebSocket::Error::SEND_QUEUE_TOO_BIG));
                        co_return;
                    },
                    asio::detached);
                return false;
            }
        }

        bool isWIP = !m_queue.empty();
        m_queue.emplace_back(std::move(data));

        if (!isWIP) {
            post_send();
        }

        return true;
    }

    void close()
    {
        if (m_socket.is_open()) {
            asio::error_code ec;
            m_socket.shutdown(tcp::socket::shutdown_both, ec);
            m_socket.close(ec);
        }
    }

    tcp::socket& socket()
    {
        return m_socket;
    }

    bool isOpen() const
    {
        return m_socket.is_open();
    }

    void fd(uint32_t fd)
    {
        m_fd = fd;
    }

    u32 fd() const
    {
        return m_fd;
    }

    u8 type() const
    {
        return m_type;
    }

    u32 owner() const
    {
        return m_serviceId;
    }

    Role role() const
    {
        return m_role;
    }

    void timeout(time_t now)
    {
        if ((0 != m_timeout) && (0 != m_recvtime) && (now - m_recvtime > m_timeout)) {
            asio::post(m_socket.get_executor(), [this, self = shared_from_this()]() {
                error(WebSocket::make_error_code(WebSocket::Error::READ_TIMEOUT));
            });
            return;
        }
        return;
    }

    bool set_no_delay()
    {
        asio::ip::tcp::no_delay option(true);
        asio::error_code ec;
        m_socket.set_option(option, ec);
        return !ec;
    }

    void settimeout(uint32_t v)
    {
        m_timeout = v;
        m_recvtime = now();
    }

    void set_send_queue_limit(uint32_t warnsize, uint32_t errorsize)
    {
        m_wq_warn_size = warnsize;
        m_wq_error_size = errorsize;
    }

    std::string address()
    {
        std::string address;
        asio::error_code ec;
        auto endpoint = m_socket.remote_endpoint(ec);
        if (!ec) {
            address.append(endpoint.address().to_string());
            address.append(":");
            address.append(std::to_string(endpoint.port()));
        }
        return address;
    }

protected:
    virtual void message_slice(ConstBuffersHolder& holder, const std::shared_ptr<Buffer>& buf)
    {
        (void)holder;
        (void)buf;
    }

    void post_send()
    {
        for (const auto& buf : m_queue) {
            if (buf->has_flag(BufferFlag::CHUNKED)) {
                message_slice(m_holder, buf);
            } else {
                m_holder.push_back(buf->data(), buf->size(), buf->has_flag(BufferFlag::CLOSE));
            }

            if (m_holder.size() >= ConstBuffersHolder::max_count) {
                break;
            }
        }

        asio::async_write(
            m_socket,
            make_buffers_ref(m_holder.buffers()),
            [this, self = shared_from_this()](const asio::error_code& e, std::size_t) {
                if (!e) {
                    if (m_holder.close()) {
                        if (m_socketServer != nullptr) {
                            m_socketServer->closeConnection(m_fd);
                            m_socketServer = nullptr;
                        }
                    } else {
                        for (size_t i = 0; i < m_holder.count(); ++i) {
                            m_queue.pop_front();
                        }

                        m_holder.clear();

                        if (!m_queue.empty()) {
                            post_send();
                        }
                    }
                } else {
                    error(e);
                }
            });
    }

    virtual void error(const asio::error_code& e, const std::string& additional = "")
    {
        if (nullptr == m_socketServer) {
            return;
        }

        Message msg {};
        std::string message = e.message();
        if (!additional.empty()) {
            message.append("(");
            message.append(additional);
            message.append(")");
        }
        std::string content = utils::format("{\"addr\":\"{}\",\"code\":{},\"message\":\"{}\"}", address().data(), e.value(), message.data());
        msg.writer(content);

        msg.setReceiver(static_cast<u8>(Common::SocketDataTypeEnum::SocketClose));
        msg.setSender(m_fd);
        m_socketServer->closeConnection(m_fd);
        handle_message(std::move(msg));
        m_socketServer = nullptr;
    }

    template <typename Message>
    void handle_message(Message&& m)
    {
        m_recvtime = now();
        if (nullptr != m_socketServer) {
            m.setSender(m_fd);
            if (m.type() == 0) {
                m.setType(m_type);
            }
            m_socketServer->handleMessage(m_serviceId, std::forward<Message>(m));
        }
    }

protected:
    Role m_role = Role::NONE;
    u32 m_fd = 0;
    time_t m_recvtime = 0;
    u32 m_timeout = 0;
    u32 m_wq_warn_size = 0;
    u32 m_wq_error_size = 0;
    u32 m_serviceId;
    u8 m_type;
    SocketServer* m_socketServer;
    tcp::socket m_socket;
    ConstBuffersHolder m_holder;
    std::deque<std::shared_ptr<Buffer>> m_queue;
};
};
#endif /* BASESOCKET_HPP_ */