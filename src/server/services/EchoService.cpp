#include "server/services/EchoService.hpp"

#include <chrono>

#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>

asio::awaitable<void> EchoService::start()
{
    m_isRunning = true;
    while (m_isRunning) {
        std::lock_guard<std::mutex> lock(m_messageQueueMutex);
        if (m_messageQueue.empty()) {
            continue;
        }
        auto message = m_messageQueue.front();
        m_messageQueue.pop();

        m_threadPool.post([this, message = std::move(message)]() {
            auto wait_time = std::chrono::seconds(std::stoi(message.message));
            std::this_thread::sleep_for(wait_time);
            message.clientInfo->send("Wait and Echo: " + message.message + "s.");
        });
    }
    co_return;
}

void EchoService::stop()
{
    m_isRunning = false;
}

void EchoService::onMessage(std::unique_ptr<CoreMessage> message)
{
    try {
        auto echoMessage = dynamic_cast<EchoMessage&>(*message);

        std::lock_guard<std::mutex> lock(m_messageQueueMutex);
        m_messageQueue.push(echoMessage);
    } catch (const std::bad_cast& e) {
        logError() << "EchoService received wrong message type.";
    }
}