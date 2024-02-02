#include "server/network/ClientManager.hpp"
#include "common/core/UUIDProvider.hpp"
#include "common/utils/Debug.hpp"

void ClientManager::addClient(ClientInfoPtr client)
{
    client->setId(UUIDProvider::nextUUID());
    m_clients.insert(std::pair<s64, ClientInfoPtr>(client->getId(), client));
    logInfo() << LOG_PREFIX << "Client " << client->getId() << " connected.";
}

void ClientManager::removeClient(ClientInfoPtr client)
{
    m_clients.erase(client->getId());
    logInfo() << LOG_PREFIX << "Client " << client->getId() << " disconnected.";
}

void ClientManager::broadcast(const std::string& msg)
{
    for (auto& client : m_clients) {
        client.second->send(msg);
    }
}

std::set<ClientInfoPtr> ClientManager::getClients() const
{
    std::set<ClientInfoPtr> clients;
    for (auto& client : m_clients) {
        clients.insert(client.second);
    }
    return clients;
}

ClientInfoPtr ClientManager::getClientById(s64 id) const
{
    auto client = m_clients.find(id);
    if (client != m_clients.end()) {
        return client->second;
    }
    return nullptr;
}
