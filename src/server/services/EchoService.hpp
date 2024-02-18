#ifndef ECHOSERVICE_HPP_
#define ECHOSERVICE_HPP_

#include <memory>
#include <queue>

#include "server/network/ClientInfo.hpp"
#include "server/services/Service.hpp"

#define _SERVICE_NAME "EchoService"

class EchoMessage : public CoreMessage {
public:
    EchoMessage(std::shared_ptr<ClientInfo> _clientInfo, std::string _message, std::string _sender)
        : message(_message)
        , clientInfo(_clientInfo)
        , CoreMessage(_sender, _SERVICE_NAME)
    {
    }
    std::string message;
    std::shared_ptr<ClientInfo> clientInfo;
};

class EchoService : public Service {
public:
    EchoService(ThreadPool& _threadPool)
        : Service(_threadPool, _SERVICE_NAME) {};

    awaitable<void> start() override;
    void stop() override;

public:
    awaitable<EchoMessage> receiveMessage();

public:
    void onMessage(std::unique_ptr<CoreMessage> message) override;

private:
    bool m_isRunning = false;
    std::queue<EchoMessage> m_messageQueue;
    std::mutex m_messageQueueMutex;
};

#endif /* ECHOSERVICE_HPP_ */