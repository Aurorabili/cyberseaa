#ifndef TCPSOCKET_HPP_
#define TCPSOCKET_HPP_

#include <asio/read.hpp>

#include "common/Buffer.hpp"
#include "common/utils/IntTypes.hpp"
#include "server/core/network/BaseSocket.hpp"
#include "server/core/network/StreamBuf.hpp"

namespace Core {
class TcpSocket : public BaseSocket {
public:
    template <typename... Args>
    explicit TcpSocket(Args&&... args)
        : BaseSocket(std::forward<Args>(args)...)
        , m_response(8192, 0)
    {
    }

    void read(size_t n, std::string_view delim, int32_t sessionid) override
    {
        if (!isOpen() || m_sessionId != 0) {
            // Undefined behavior
            logError("invalid read operation. %u", m_fd);
            asio::post(m_socket.get_executor(), [this, self = shared_from_this()]() {
                error(make_error_code(WebSocket::Error::INVALID_READ_OPERATION));
            });
            return;
        }

        Buffer* buf = m_response.buffer();
        std::size_t size = buf->size() + m_delim.size();
        buf->commit(m_revert);
        buf->consume(size);
        m_revert = 0;

        m_delim = delim;
        m_sessionId = sessionid;

        if (!m_delim.empty()) {
            read_until((n > 0 ? n : std::numeric_limits<size_t>::max()));
        } else {
            read(n);
        }
    }

protected:
    void read_until(size_t count)
    {
        asio::async_read_until(m_socket, StreamBuf(m_response.buffer(), count), m_delim,
            [this, self = shared_from_this()](const asio::error_code& e, std::size_t bytes_transferred) {
                if (e) {
                    error(e);
                    return;
                }
                response(bytes_transferred);
            });
    }

    void read(size_t count)
    {
        std::size_t size = (m_response.size() >= count ? 0 : (count - m_response.size()));
        asio::async_read(m_socket, StreamBuf(m_response.buffer(), count), asio::transfer_exactly(size),
            [this, self = shared_from_this(), count](const asio::error_code& e, std::size_t) {
                if (e) {
                    error(e);
                    return;
                }
                response(count);
            });
    }

    void error(const asio::error_code& e, const std::string& additional = "") override
    {
        (void)additional;

        if (nullptr == m_socketServer) {
            return;
        }
        m_delim.clear();
        m_response.buffer()->clear();

        if (e) {
            if (e == WebSocket::Error::READ_TIMEOUT) {
                m_response.writer(utils::format("TIMEOUT %s.(%d)", e.message().data(), e.value()));
            } else if (e == asio::error::eof) {
                m_response.writer(utils::format("EOF %s.(%d)", e.message().data(), e.value()));
            } else {
                m_response.writer(utils::format("SOCKET_ERROR %s.(%d)", e.message().data(), e.value()));
            }
        }

        m_socketServer->closeConnection(m_fd);

        if (m_sessionId != 0) {
            response(0, PTYPE_ERROR);
        }
        m_socketServer = nullptr;
    }

    void response(size_t count, u8 type = PTYPE_SOCKET_TCP)
    {
        if (m_sessionId == 0) {
            return;
        }
        auto buf = m_response.buffer();
        assert(buf->size() >= count);
        m_revert = 0;
        if (type == PTYPE_SOCKET_TCP) {
            m_revert = (buf->size() - count) + m_delim.size();
        }
        buf->revert(m_revert);
        m_response.setType(type);
        m_response.setSender(fd());
        m_response.setSessionId(m_sessionId);
        m_sessionId = 0;
        handle_message(m_response);
    }

protected:
    size_t m_revert = 0;
    s32 m_sessionId = 0;
    std::string m_delim;
    Message m_response;
};
};

#endif /* TCPSOCKET_HPP_ */