#include "Lobby.h"
#include <iostream>

Lobby::Lobby(const LobbyID& id, GameType type, ClientID hostID, const std::string& lobbyName, const std::string& hostPlayerName)
        : m_lobbyID(id)
        , m_lobbyName(lobbyName)
        , m_gameType(type)
        , m_hostID(hostID)
        , m_state(LobbyState::Open)
        , m_maxPlayers(10)
{
    insertPlayer(hostID, hostPlayerName, LobbyRole::Host);
    std::cout << "[Lobby] Created lobby '" << lobbyName << "' (ID: " << id << ")\n";
}

bool
Lobby::insertPlayer(uintptr_t clientID, const std::string& playerName, LobbyRole role) {
    if(isFull()){
        return false;
    }
    auto [it, inserted] = m_players.try_emplace(clientID);

    if(!inserted){
        return false;
    }

    LobbyMember& player = it->second;
    player.clientID = clientID;
    player.name = playerName;
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

ClientID
Lobby::getHostID() const {
    return m_hostID;
}

std::vector<LobbyMember>
Lobby::getAllPlayer() const {
    std::vector<LobbyMember> playablePlayers;
    playablePlayers.reserve(m_players.size());

    auto hostIt = m_players.find(m_hostID);
    if (hostIt != m_players.end() && hostIt->second.isPlayable()) {
        playablePlayers.push_back(hostIt->second);
    }

    for(const auto& [clientID, player] : m_players){
        if(clientID == m_hostID) {
            continue;
        }

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
    info.minPlayers = 1;
    info.state = m_state;

    return info;
}

void
Lobby::forEachPlayer(const std::function<void(const LobbyMember&)>& action) const{
    auto hostIt = m_players.find(m_hostID);
    if (hostIt != m_players.end() && hostIt->second.isPlayable()) {
        action(hostIt->second);
    }

    for(const auto& [clientID, player] : m_players){
        if(clientID == m_hostID) {
            continue;
        }

        if(player.isPlayable()){
            action(player);
        }
    }
}


std::optional<LobbyRole>
Lobby::getMemberRole(uintptr_t clientID) const {
    auto it = m_players.find(clientID);
    if(it != m_players.end()){
        return it->second.role;
    }
    return std::nullopt;
}