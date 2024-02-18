#ifndef SERVICE_HPP_
#define SERVICE_HPP_

#include <asio/awaitable.hpp>
#include <asio/io_context.hpp>

#include "common/utils/Debug.hpp"
#include "server/core/CoreMessage.hpp"
#include "server/core/ThreadPool.hpp"

using asio::awaitable;
using asio::io_context;

class Service {
public:
    Service(ThreadPool& threadPool, const std::string& name)
        : m_threadPool(threadPool)
        , m_name(name)
    {
    }
    virtual ~Service() = default;
    virtual awaitable<void> start()
    {
        logDebug() << m_name << "Service started.";
        co_return;
    }
    virtual void stop()
    {
        logDebug() << m_name << "Service stopped.";
    }

public:
    std::string getName() { return m_name; }

public:
    virtual void onMessage(std::unique_ptr<CoreMessage> message)
    {
        logDebug() << m_name << "service received message from: " << message->sender;
    };

protected:
    std::string m_name;
    ThreadPool& m_threadPool;
};

#endif /* SERVICE_HPP_ */