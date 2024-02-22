#ifndef MODULES_PLAYERLIST_HPP_
#define MODULES_PLAYERLIST_HPP_

#include <filesystem>
#include <unordered_map>

#include <asio/awaitable.hpp>

#include "server/modules/player/Player.hpp"
#include "server/network/ClientManager.hpp"

namespace fs = std::filesystem;

namespace modules {
class PlayerList {
public:
    using PlayerMap = std::unordered_map<Player::PlayerID, Player>;
    using PlayerSaveMap = std::unordered_map<Player::PlayerID, fs::path>;

public:
    bool isPlayerOnline(const Player::PlayerID id) const;
    bool isPlayerExist(const Player::PlayerID id) const;
    Player& getPlayer(const Player::PlayerID id);
    void onPlayerJoin(const Player::PlayerID id, ClientInfoPtr clientInfo);
    void onPlayerLeave(const Player::PlayerID id);

private:
    asio::awaitable<void> loadPlayer(const Player::PlayerID id);
    asio::awaitable<void> savePlayer(const Player::PlayerID id);

private:
    PlayerMap m_players;
    PlayerSaveMap m_playerSave;
};
}

#endif /* MODULES_PLAYERLIST_HPP_ */