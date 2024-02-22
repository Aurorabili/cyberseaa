#include "PlayerList.hpp"
#include "server/modules/player/PlayerList.hpp"

#include "common/utils/Debug.hpp"
#include "server/modules/player/Player.hpp"

using namespace modules;

bool PlayerList::isPlayerOnline(const Player::PlayerID id) const
{
    return m_players.find(id) != m_players.end();
}

bool PlayerList::isPlayerExist(const Player::PlayerID id) const
{
    return m_playerSave.find(id) != m_playerSave.end();
}

Player& PlayerList::getPlayer(const Player::PlayerID id)
{
    if (isPlayerOnline(id)) {
        return m_players.at(id);
    } else {
        logError() << "Try to get player" << id << "that is not online!";
        return Player::NullPlayer();
    }
}

void PlayerList::onPlayerJoin(const Player::PlayerID id, ClientInfoPtr clientInfo)
{
    if (isPlayerOnline(id)) {
        logWarning() << "Player" << id << "is trying to join while already online!";

        clientInfo->send("You are already online!");

        clientInfo->disconnect();

        return;
    }

    if (!isPlayerExist(id)) {
        logError() << "Try to join player" << id << "that is not exist!";
        return;
    }

    Player player = Player(clientInfo);

    logInfo() << "Player" << id << "joined the game!";
}

void PlayerList::onPlayerLeave(const Player::PlayerID id)
{
    if (!isPlayerOnline(id)) {
        logWarning() << "Player" << id << "is trying to leave while not online!";
        return;
    }

    m_players.erase(id);

    co_spawn
}