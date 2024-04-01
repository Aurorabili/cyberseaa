#ifndef SERVICE_HPP_
#define SERVICE_HPP_

#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <coroutine>
#include <memory>
#include <unordered_map>
#include <utility>

#include "common/Exception.hpp"
#include "common/utils/IntTypes.hpp"
#include "server/core/Message.hpp"
#include "server/core/Session.hpp"

namespace Core {
class Server;
class Worker;
struct ServiceConfig;

class Service {
public:
    friend class Worker;

public:
    Service() = default;
    virtual ~Service() { }

    Service(const Service&) = delete;
    Service& operator=(const Service&) = delete;
    Service(const Service&&) = delete;
    Service& operator=(const Service&&) = delete;

public:
    u32 id() const
    {
        return m_id;
    }

    const std::string& name() const
    {
        return m_name;
    }

    void setName(const std::string& name)
    {
        m_name = name;
    }

    void setContext(Server* server, Worker* worker)
    {
        m_server = server;
        m_worker = worker;
    }

    Server* server() const
    {
        return m_server;
    }

    Worker* worker() const
    {
        return m_worker;
    }

    bool unique() const
    {
        return m_isUnique;
    }

    bool ready() const
    {
        return m_isReady;
    }

    void reset()
    {
        m_isReady = false;
    }

    void setReady()
    {
        m_isReady = true;
    }

    s32 nextSessionId()
    {
        s32 sessionId = m_nowSessionId;
        while (sessions.find(sessionId) != sessions.end()) {
            sessionId++;
        }
        m_nowSessionId = sessionId;
        return sessionId;
    }

    void addSession(s32 sessionId, std::shared_ptr<Session<std::shared_ptr<Core::Message>>> session)
    {
        sessions[sessionId] = session;
    }

    void resumeSession(s32 sessionId, std::shared_ptr<Core::Message> msg)
    {
        auto it = sessions.find(sessionId);
        if (it != sessions.end()) {
            it->second->resume(msg);
            sessions.erase(it);
        }
    }

public:
    virtual bool init(const ServiceConfig& conf) = 0;

    virtual void dispatch(Message* msg) = 0;

protected:
    void setUnique(bool v)
    {
        m_isUnique = v;
    }

    void setId(u32 v)
    {
        m_id = v;
    }

protected:
    bool m_isUnique = false;
    bool m_isReady = false;
    u32 m_id = 0;
    Server* m_server = nullptr;
    Worker* m_worker = nullptr;
    std::string m_name;
    std::unordered_map<s32, std::shared_ptr<Session<std::shared_ptr<Core::Message>>>> sessions;
    s32 m_nowSessionId = 1;
};

template <typename Service, typename Message>
inline void handle_message(Service&& s, Message&& m)
{
    u32 receiver = m.receiver();
    s->dispatch(&m);
    // redirect message
    if (m.receiver() != receiver) {
        DEBUG_ASSERT(!m.broadcast(), "can not redirect broadcast message");
        if constexpr (std::is_rvalue_reference_v<decltype(m)>) {
            s->server()->sendMessage(std::forward<Message>(m));
        }
    }
}

using ServicePtr = std::unique_ptr<Service>;

struct ServiceConfig {
    bool unique = false;
    uint32_t threadid = 0;
    uint32_t creator = 0;
    int32_t session = 0;
    size_t memlimit = 0;
    std::string type;
    std::string name;
    std::string source;
    std::string params;
};
}; // namespace Cyberseea

#endif /* SERVICE_HPP_ */