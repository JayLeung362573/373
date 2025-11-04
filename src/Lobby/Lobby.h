#pragma once
#include <vector>
#include <unordered_map>
#include <optional>

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
    void addPlayer(uintptr_t clientID, LobbyRole role = LobbyRole::Player);
    void removePlayer(uintptr_t clientID);

    std::optional<uintptr_t> getHostID() const;
    void setPlayerRole(uintptr_t clientID, LobbyRole role);

    std::optional<LobbyRole> getPlayerRole(uintptr_t clientID) const;
    void transferHost(uintptr_t newHostID);

    int getPlayablePlayer() const;
    bool canStartGame1(int minPlayers, int maxPlayers) const;

    std::vector<LobbyPlayer> getPlayablePlayers() const;
private:
    std::unordered_map<uintptr_t, LobbyPlayer> m_players;
};
