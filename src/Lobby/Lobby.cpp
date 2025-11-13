#include "Lobby.h"
#include <iostream>

Lobby::Lobby(const LobbyID& id, GameType type, ClientID hostID, const std::string& name)
        : m_lobbyID(id)
        , m_lobbyName(name)
        , m_gameType(type)
        , m_hostID(hostID)
        , m_state(LobbyState::Open)
        , m_maxPlayers(10)
{
    std::cout << "[Lobby] Created lobby '" << name << "' (ID: " << id << ")\n";
}

bool
Lobby::insertPlayer(uintptr_t clientID, LobbyRole role) {
    if(isFull()){
        return false;
    }

    if(hasPlayer(clientID)){
        return false;
    }

    LobbyMember player;
    player.clientID = clientID;
    player.role = role;
    player.ready = false;

    m_players[clientID] = player;
    return true;
}

void Lobby::deletePlayer(uintptr_t clientID) {
    m_players.erase(clientID);
}

bool
Lobby::hasPlayer(ClientID clientID) const {
    return m_players.find(clientID) != m_players.end();
}

std::optional<uintptr_t>
Lobby::getHostID() const {
    for(const auto& [clientID, player] : m_players){
        if(player.isHost()){
            return clientID;
        }
    }
    return std::nullopt;
}

std::vector<LobbyMember>
Lobby::getAllPlayer() const {
    std::vector<LobbyMember> playablePlayers;

    for(const auto& [clientID, player] : m_players){
        if(player.isPlayable()){
            playablePlayers.push_back(player);
        }
    }

    return playablePlayers;
}

LobbyInfo
Lobby::getInfo() const {
    LobbyInfo info;
    info.lobbyID = m_lobbyID;
    info.lobbyName = m_lobbyName;
    info.gameType = m_gameType;
    info.hostID = m_hostID;
    info.currentPlayers = m_players.size();
    info.maxPlayers = m_maxPlayers;
    info.state = m_state;

    return info;
}

std::optional<LobbyRole>
Lobby::getMemberRole(uintptr_t clientID) const {
    auto it = m_players.find(clientID);
    if(it != m_players.end()){
        return it->second.role;
    }
    return std::nullopt;
}