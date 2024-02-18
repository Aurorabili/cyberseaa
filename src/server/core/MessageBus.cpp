#include "server/core/MessageBus.hpp"

#include "common/utils/Debug.hpp"
#include "server/core/CoreMessage.hpp"

MessageBus::MessageBus(ServicesMap& services)
    : m_services(services)
{
}

void MessageBus::send(std::unique_ptr<CoreMessage> message)
{
    m_queue.enqueue(std::move(message));
    m_messageCounter.fetch_add(1);
}

void MessageBus::processOne()
{
    if (m_messageCounter.load() == 0) {
        return;
    }

    std::unique_ptr<CoreMessage> message;
    if (m_queue.try_dequeue(message)) {
        auto it = m_services.find(message->receiver);
        if (it != m_services.end()) {
            logDebug() << "MessageBus: sending message to " << message->receiver;
            it->second->onMessage(std::move(message));
        }
        m_messageCounter.fetch_sub(1);
    }
}
