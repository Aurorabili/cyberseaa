#include "Player.hpp"
#include "server/modules/player/Player.hpp"

using namespace modules;

Player::Player(const std::shared_ptr<ClientInfo> clientInfo)
    : m_clientInfo(clientInfo)
{
}