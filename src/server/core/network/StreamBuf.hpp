#ifndef STREAMBUF_HPP_
#define STREAMBUF_HPP_

#include <asio/buffer.hpp>

#include "common/Buffer.hpp"

namespace Core {
class StreamBuf {

    using Buffer = Common::Buffer;

public:
    typedef asio::const_buffer const_buffers_type;
    typedef asio::mutable_buffer mutable_buffers_type;

    StreamBuf(Buffer* buf, std::size_t maxsize = (std::numeric_limits<std::size_t>::max)())
        : m_buffer(buf)
        , m_maxSize(maxsize)
    {
    }

    StreamBuf(const StreamBuf&) = delete;

    StreamBuf& operator=(const StreamBuf&) = delete;

    StreamBuf(StreamBuf&& other) noexcept
        : m_buffer(other.m_buffer)
        , m_maxSize(other.m_maxSize)
    {
        other.m_buffer = nullptr;
        other.m_maxSize = 0;
    }

    StreamBuf& operator=(StreamBuf&& other) noexcept
    {
        m_buffer = other.m_buffer;
        m_maxSize = other.m_maxSize;
        other.m_buffer = nullptr;
        other.m_maxSize = 0;
        return *this;
    }

    std::size_t size() const noexcept
    {
        if (nullptr == m_buffer)
            return 0;
        return m_buffer->size();
    }

    std::size_t max_size() const noexcept
    {
        return m_maxSize;
    }

    std::size_t capacity() const noexcept
    {
        if (nullptr == m_buffer)
            return 0;
        return m_buffer->capacity() - m_buffer->reserved();
    }

    const_buffers_type data() const noexcept
    {
        if (nullptr == m_buffer)
            return asio::const_buffer { nullptr, 0 };
        return asio::const_buffer { m_buffer->data(), m_buffer->size() };
    }

    mutable_buffers_type prepare(std::size_t n)
    {
        if (nullptr == m_buffer)
            return asio::mutable_buffer { nullptr, 0 };
        auto space = m_buffer->prepare(n);
        return asio::mutable_buffer { space.first, space.second };
    }

    void commit(std::size_t n)
    {
        if (nullptr == m_buffer)
            return;
        m_buffer->commit(n);
    }

    void consume(std::size_t n)
    {
        if (nullptr == m_buffer)
            return;
        m_buffer->consume(n);
    }

private:
    Buffer* m_buffer;
    std::size_t m_maxSize;
};
}

#endif // STREAMBUF_HPP_