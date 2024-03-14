#ifndef INTERNALSERVICE_HPP
#define INTERNALSERVICE_HPP

#include <asio/awaitable.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "common/ConcurrentMap.hpp"
#include "common/ConcurrentQueue.hpp"
#include "common/Debug.hpp"
#include "common/utils/IntTypes.hpp"
#include "server/core/Message.hpp"
#include "server/core/Worker.hpp"
#include "server/core/services/Handler.hpp"
#include "server/core/services/Service.hpp"

namespace Core {
class InternalService : public Core::Service {
public:
    InternalService() = default;

    InternalService(const InternalService&) = delete;
    InternalService& operator=(const InternalService&) = delete;
    InternalService(const InternalService&&) = delete;
    InternalService& operator=(const InternalService&&) = delete;

public:
    bool init(const ServiceConfig& conf) override;
    void dispatch(Message* msg) override;

    s32 makeSession();
    void registerCommand(const std::string& cmd, std::function<void(const std::string&)> fn);

    template <int PTYPE>
    struct MessageType;

    template <>
    struct MessageType<Common::PTYPE_INTEGER> {
        using type = u32;
    };

    template <>
    struct MessageType<Common::PTYPE_TEXT> {
        using type = std::string;
    };

    template <>
    struct MessageType<Common::PTYPE_ERROR> {
        using type = std::string;
    };

    template <>
    struct MessageType<Common::PTYPE_COMMAND> {
        using type = std::string;
    };

    template <>
    struct MessageType<Common::PTYPE_SOCKET_TCP> {
        using type = std::unique_ptr<Buffer>;
    };

    template <>
    struct MessageType<Common::PTYPE_SOCKET_UDP> {
        using type = std::unique_ptr<Buffer>;
    };

    template <int PTYPE>
    using MessageType_t = typename MessageType<PTYPE>::type;

    template <u8 PTYPE>
    asio::awaitable<std::optional<MessageType_t<PTYPE>>> wait(s32 session)
    {
        while (m_sessionResults.find(session) == m_sessionResults.end()) {
            co_await asio::steady_timer(m_worker->io_context(), asio::chrono::milliseconds(100)).async_wait(asio::use_awaitable);
        }

        if (!m_isReady) {
            co_return std::nullopt;
        }

        auto type = m_sessionResults[session].type();
        auto size = m_sessionResults[session].size();
        auto data = m_sessionResults[session].data();
        m_sessionResults.erase(session);
        m_sessions.erase(session);

        if (type == Common::PTYPE_ERROR) {
            auto err = std::string(data, size);
            logError("session[%d] error: %s", session, err.c_str());
            co_return std::nullopt;
        }

        if (type != PTYPE) {
            logError("session[%d] message type is not expected: %d", session, type);
            co_return std::nullopt;
        }

        try {
            if constexpr (PTYPE == Common::PTYPE_INTEGER) {
                co_return std::stoi(std::string(data, size));
            } else if constexpr (PTYPE == Common::PTYPE_TEXT) {
                co_return std::string(data, size);
            } else if constexpr (PTYPE == Common::PTYPE_COMMAND) {
                co_return std::string(data, size);
            } else if constexpr (PTYPE == Common::PTYPE_SOCKET_TCP || PTYPE == Common::PTYPE_SOCKET_UDP) {
                auto buf = std::make_unique<Buffer>();
                buf->write_back(data, size);
                co_return std::move(buf);
            }
        } catch (std::exception& e) {
            logError("could not cast session[%d] message to type[%d]: ", session, type, e.what());
            co_return std::nullopt;
        }
    }

private:
    std::unique_ptr<Handler> m_handler;
    std::unordered_set<s32> m_sessions;
    std::unordered_map<s32, Message> m_sessionResults;
    std::unordered_map<std::string, std::function<void(const std::string&)>> m_commands;
    s32 m_session;
};

}
#endif // INTERNALSERVICE_HPP