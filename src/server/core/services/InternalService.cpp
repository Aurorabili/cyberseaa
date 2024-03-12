#include "server/core/services/InternalService.hpp"

#include <memory>

#include "common/Debug.hpp"
#include "server/handler/PlayerHandler/PlayerHandler.hpp"

const std::unordered_map<std::string, std::function<std::unique_ptr<Core::Handler>(Core::InternalService*)>> InternalHandlerMap = {
    { "PlayerHandler", [](Core::InternalService* is) { return std::make_unique<PlayerHandler>(is); } }
};

void Core::InternalService::dispatch(Message* msg)
{
    if (!ready()) {
        return;
    }

    logDebug("Dispatching message: type: %d; data: %s; receiver: %d", msg->type(), msg->data(), msg->receiver());
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