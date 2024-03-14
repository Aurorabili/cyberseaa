#include "server/core/services/InternalService.hpp"

#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/dispatch.hpp>
#include <memory>
#include <string>
#include <thread>

#include "common/Buffer.hpp"
#include "common/Constant.hpp"
#include "common/Debug.hpp"
#include "server/handler/GateHandler/GateHandler.hpp"
#include "server/handler/PlayerHandler/PlayerHandler.hpp"

const std::unordered_map<std::string, std::function<std::unique_ptr<Core::Handler>(Core::InternalService*)>> InternalHandlerMap = {
    { "PlayerHandler", [](Core::InternalService* is) { return std::make_unique<PlayerHandler>(is); } },
    { "GateHandler", [](Core::InternalService* is) { return std::make_unique<GateHandler>(is); } }
};

void Core::InternalService::dispatch(Message* msg)
{
    if (!ready()) {
        return;
    }

    logDebug("Dispatching message: type: %d; data: %s; receiver: %d; session: %d", msg->type(), msg->data(), msg->receiver(), msg->sessionId());

    if (msg->sessionId() != 0) {
        m_sessionResults.emplace(msg->sessionId(), msg->clone());
    }

    if (msg->sessionId() == 0 && msg->type() == Common::PTYPE_COMMAND) {
        auto cmd = std::string(msg->data(), msg->size());

        std::unordered_map<std::string, std::string> args;
        std::string_view sv(cmd);
        while (!sv.empty()) {
            auto pos = sv.find_first_of(' ');
            if (pos == std::string_view::npos) {
                args[std::string(sv)] = "";
                break;
            }

            auto key = sv.substr(0, pos);
            sv.remove_prefix(pos + 1);

            pos = sv.find_first_of(' ');
            if (pos == std::string_view::npos) {
                args[std::string(key)] = std::string(sv);
                break;
            }

            auto value = sv.substr(0, pos);
            sv.remove_prefix(pos + 1);

            args[std::string(key)] = std::string(value);
        }

        if (m_commands.find(args.begin()->first) != m_commands.end()) {
            m_commands[args.begin()->first](args.begin()->second);
        } else {
            logError("Command not found: %s", args.begin()->first.c_str());
        }
    }
}

bool Core::InternalService::init(const ServiceConfig& conf)
{
    m_name = conf.name;

    logInfo("[WORKER %u] new service [%s]", m_worker->id(), m_name.c_str());

    if (InternalHandlerMap.find(m_name) == InternalHandlerMap.end()) {
        logError("Service [%s] not found", m_name.c_str());
        return false;
    }

    m_handler = InternalHandlerMap.at(m_name)(this);

    return true;
}

s32 Core::InternalService::makeSession()
{
    while (m_sessions.find(m_session++) != m_sessions.end() && m_session <= 0x7FFFFFFF) { }
    m_sessions.insert(m_session);
    return m_session;
}

void Core::InternalService::registerCommand(const std::string& cmd, std::function<void(const std::string&)> fn)
{
    m_commands[cmd] = fn;
}