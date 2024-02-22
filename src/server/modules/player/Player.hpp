#ifndef MODULES_PLAYER_HPP_
#define MODULES_PLAYER_HPP_

#include <chrono>
#include <ctime>
#include <memory>
#include <string>

#include <asio/ip/address.hpp>
#include <nlohmann/json.hpp>

#include "common/utils/IntTypes.hpp"
#include "common/utils/Utils.hpp"
#include "server/network/ClientInfo.hpp"

#define PLAYER_NULL_UID 0

using namespace nlohmann;

namespace modules {
class Player {
public:
    using PlayerID = u32;

public:
    Player(const std::shared_ptr<ClientInfo> clientInfo);
    ~Player() = default;

public:
    static Player& NullPlayer()
    {
        static Player nullPlayer(nullptr);
        nullPlayer.UID = PLAYER_NULL_UID;
        return nullPlayer;
    }

    bool isNull() const { return UID == PLAYER_NULL_UID; }

public:
    PlayerID UID;
    std::string Name;
    std::time_t LastLoginDatetime;
    std::time_t LastLogoutDatetime;
    std::time_t RegisterDatetime;
    std::string LastLoginIpAddress;
    std::string LoginIpAddress() const { return m_clientInfo->ipAddress().to_string(); }

public:
    json serializable() const
    {
        json j;
        j["UID"] = UID;
        j["Name"] = Name;
        j["LastLoginDatetime"] = utils::toTimeStamp<std::chrono::seconds>(LastLoginDatetime);
        j["LastLogoutDatetime"] = utils::toTimeStamp<std::chrono::seconds>(LastLogoutDatetime);
        j["RegisterDatetime"] = utils::toTimeStamp<std::chrono::seconds>(RegisterDatetime);
        j["LoginIpAddress"] = LoginIpAddress();
        return j;
    }

    void deserialize(const json& j)
    {
        UID = j.at("UID").get<PlayerID>();
        Name = j.at("Name").get<std::string>();
        LastLoginDatetime = utils::toTimeT<std::chrono::seconds>(j.at("LastLoginDatetime").get<u64>());
        LastLogoutDatetime = utils::toTimeT<std::chrono::seconds>(j.at("LastLogoutDatetime").get<u64>());
        RegisterDatetime = utils::toTimeT<std::chrono::seconds>(j.at("RegisterDatetime").get<u64>());
        LastLoginIpAddress = j.at("LoginIpAddress").get<std::string>();
    }

private:
    std::shared_ptr<ClientInfo> m_clientInfo;
};
}

#endif // MODULES_PLAYER_HPP_