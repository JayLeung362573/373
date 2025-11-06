#pragma once
#include <vector>
#include <unordered_map>
#include <optional>
#include <cstdint>
#include "LobbyTypes.h"

enum class LobbyRole{
    Host,
    Player,
    Audience,
};

struct LobbyPlayer{
    uintptr_t clientID = 0;
    LobbyRole role = LobbyRole::Audience;
    bool ready = false;

    bool isHost() const {return role == LobbyRole::Host;}
    bool isPlayer() const {return role == LobbyRole::Player;}
    bool isAudience() const {return role == LobbyRole::Audience;}
    bool isPlayable() const { return isHost() || isPlayer();}
};

class Lobby{
public:
    Lobby(const LobbyID& id, GameType gameType, ClientID hostID, const std::string& name);

    bool addPlayer(uintptr_t clientID, LobbyRole role = LobbyRole::Player);
    void removePlayer(uintptr_t clientID);
    bool hasPlayer(ClientID clientID) const;

    std::optional<uintptr_t> getHostID() const;

    std::optional<LobbyRole> getPlayerRole(uintptr_t clientID) const;

    std::vector<LobbyPlayer> getPlayablePlayer() const;

    LobbyInfo getInfo() const;
    GameType getGameType() const {return m_gameType;}
private:
    LobbyID m_lobbyID;
    std::string m_lobbyName;
    GameType m_gameType;
    ClientID m_hostID;
    LobbyState m_state;
    size_t m_maxPlayers;

    std::unordered_map<uintptr_t, LobbyPlayer> m_players;
};
