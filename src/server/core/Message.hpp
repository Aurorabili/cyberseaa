#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_

#include <cstddef>
#include <memory>

#include "common/Buffer.hpp"
#include "common/utils/IntTypes.hpp"

using namespace Common;

namespace Core {
class Message final {
public:
    static Message EmptyMessage()
    {
        return Message { BufferPtr {} };
    }

    Message()
        : m_data(Buffer::make_unique())
    {
    }

    Message(size_t reserve)
        : m_data(Buffer::make_unique(reserve))
    {
    }

    Message(size_t reserve, u16 head_reserve)
        : m_data(Buffer::make_unique(reserve, head_reserve))
    {
    }

    explicit Message(BufferPtr&& v)
        : m_data(std::move(v))
    {
    }

    ~Message() = default;

    Message(const Message&) = delete;

    Message& operator=(const Message&) = delete;

    Message(Message&& other) noexcept
        : m_type(std::exchange(other.m_type, u8 {}))
        , m_sender(std::exchange(other.m_sender, 0))
        , m_receiver(std::exchange(other.m_receiver, 0))
        , m_sessionid(std::exchange(other.m_sessionid, 0))
        , m_data(std::move(other.m_data))
    {
    }

    Message& operator=(Message&& other) noexcept
    {
        if (this != std::addressof(other)) {
            m_type = std::exchange(other.m_type, uint8_t {});
            m_sender = std::exchange(other.m_sender, 0);
            m_receiver = std::exchange(other.m_receiver, 0);
            m_sessionid = std::exchange(other.m_sessionid, 0);
            m_data = std::move(other.m_data);
        }
        return *this;
    }

public:
    void setSender(u32 serviceid)
    {
        m_sender = serviceid;
    }

    u32 sender() const
    {
        return m_sender;
    }

    void setReceiver(u32 serviceid)
    {
        m_receiver = serviceid;
    }

    u32 receiver() const
    {
        return m_receiver;
    }

    void setSessionId(s32 v)
    {
        m_sessionid = v;
    }

    s32 sessionId() const
    {
        return m_sessionid;
    }

    void setType(u8 v)
    {
        m_type = v;
    }

    u8 type() const
    {
        return m_type;
    }

    bool broadcast() const
    {
        return (m_data && m_data->has_flag(BufferFlag::BROADCAST));
    }

    void setBroadcast(bool v)
    {
        if (!m_data) {
            return;
        }
        v ? m_data->set_flag(BufferFlag::BROADCAST) : m_data->clear_flag(BufferFlag::BROADCAST);
    }

    void writer(std::string_view s)
    {
        assert(m_data);
        m_data->write_back(s.data(), s.size());
    }

    const char* data() const
    {
        return m_data ? m_data->data() : nullptr;
    }

    size_t size() const
    {
        return m_data ? m_data->size() : 0;
    }

    BufferPtr moveBuffer()
    {
        return std::move(m_data);
    }

    Buffer* buffer()
    {
        return m_data ? m_data.get() : nullptr;
    }


private:
    u8 m_type = 0;
    u32 m_sender = 0;
    u32 m_receiver = 0;
    s32 m_sessionid = 0;
    BufferPtr m_data;
};
};

#endif /* MESSAGE_HPP_ */