#ifndef CLIENTMANAGER_HPP_
#define CLIENTMANAGER_HPP_

#include <memory>
#include <set>
#include <unordered_map>

#include "common/utils/IntTypes.hpp"
#include "server/core/MessageBus.hpp"
#include "server/network/ClientInfo.hpp"

#define LOG_PREFIX "[ClientManager]"

using ClientInfoPtr = std::shared_ptr<ClientInfo>;

class ClientManager {
public:
    ClientManager(MessageBus& messageBus)
        : m_messageBus(messageBus) {};
    void addClient(ClientInfoPtr client);
    void removeClient(ClientInfoPtr client);

    void broadcast(const std::string& msg);

    std::set<ClientInfoPtr> getClients() const;
    ClientInfoPtr getClientById(s64 id) const;

public:
    void onMessageReceived(ClientInfoPtr client, const std::string& msg);

private:
    std::unordered_map<s64, ClientInfoPtr> m_clients;

private:
    MessageBus& m_messageBus;
};

#endif /* CLIENTMANAGER_HPP_ */