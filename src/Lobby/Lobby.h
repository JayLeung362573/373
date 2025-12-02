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

struct LobbyMember{
    uintptr_t clientID = 0;
    std::string name;
    LobbyRole role = LobbyRole::Audience;
    bool ready = false;

    bool isHost() const {return role == LobbyRole::Host;}
    bool isPlayer() const {return role == LobbyRole::Player;}
    bool isAudience() const {return role == LobbyRole::Audience;}
    bool isPlayable() const { return isHost() || isPlayer();}
};

/// Individual lobby instance:
/// 1) store lobby related data: name, id, type, members
/// 2) manage players in THIS individual lobby
/// 3) track ready states
class Lobby{
public:
    Lobby(const LobbyID& id, GameType gameType, ClientID hostID, const std::string& lobbyName, const std::string& hostPlayerName);

    /// manage player list in this lobby
    bool insertPlayer(uintptr_t clientID, const std::string& playerName, LobbyRole role = LobbyRole::Player);
    void deletePlayer(uintptr_t clientID);
    bool hasPlayer(ClientID clientID) const;

    /// host management
    ClientID getHostID() const;
    std::optional<LobbyRole> getMemberRole(uintptr_t clientID) const;
    std::vector<LobbyMember> getAllPlayer() const;

    /// getters for Info
    LobbyInfo getInfo() const;
    GameType getGameType() const {return m_gameType;}
    size_t getPlayerCount() const {return m_players.size();}
    bool isFull() const {return m_players.size() >= m_maxPlayers;}

    /// helper function
    void forEachPlayer(const std::function<void(const LobbyMember&)>& action) const;

private:
    LobbyID m_lobbyID;
    std::string m_lobbyName;
    GameType m_gameType;
    ClientID m_hostID;
    LobbyState m_state;
    size_t m_maxPlayers;

    std::unordered_map<uintptr_t, LobbyMember> m_players;
};
