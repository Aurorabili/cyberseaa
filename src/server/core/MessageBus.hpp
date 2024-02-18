#ifndef MESSAGEBUS_HPP_
#define MESSAGEBUS_HPP_

#include <memory>
#include <unordered_map>

#include <concurrentqueue.h>

#include "common/utils/Debug.hpp"
#include "common/utils/IntTypes.hpp"
#include "server/core/CoreMessage.hpp"
#include "server/services/Service.hpp"

using namespace moodycamel;

class MessageBus : public std::enable_shared_from_this<MessageBus> {
    using ServicesMap = std::unordered_map<std::string, std::shared_ptr<Service>>;

public:
    MessageBus(ServicesMap& services);
    ~MessageBus() = default;

private:
    // no copy/move semantics
    MessageBus(const MessageBus&) = delete;
    MessageBus(MessageBus&&) = delete;
    MessageBus& operator=(const MessageBus&) = delete;
    MessageBus& operator=(MessageBus&&) = delete;

public:
    void send(std::unique_ptr<CoreMessage> message);
    void processOne();

private:
    ServicesMap& m_services;
    ConcurrentQueue<std::unique_ptr<CoreMessage>> m_queue;

private:
    std::atomic<u64> m_messageCounter = 0;
};

#endif /* MESSAGEBUS_HPP_ */
