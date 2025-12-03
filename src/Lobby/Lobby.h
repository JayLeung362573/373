#pragma once
#include <algorithm>
#include <optional>
#include <vector>
#include <cstdint>
#include <functional>
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
    Lobby(const LobbyID& id, GameType gameType, LobbyMember host, const std::string& lobbyName);

    /// manage player list in this lobby
    bool insertPlayer(const LobbyMember& member);
    void deletePlayer(uintptr_t clientID);
    bool hasPlayer(ClientID clientID) const;

    /// host management
    ClientID getHostID() const;
    std::optional<LobbyRole> getMemberRole(uintptr_t clientID) const;
    std::vector<LobbyMember> getAllPlayer() const;
    const std::vector<LobbyMember>& getMembers() const { return m_players; }

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

    std::vector<LobbyMember> m_players; // store in vector to expose range and other sequence algorithms
};
