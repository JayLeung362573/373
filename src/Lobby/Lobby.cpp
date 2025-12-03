#include "Lobby.h"
#include <iostream>
#include <functional>

Lobby::Lobby(const LobbyID& id, GameType type, LobbyMember host, const std::string& lobbyName)
        : m_lobbyID(id)
        , m_lobbyName(lobbyName)
        , m_gameType(type)
        , m_hostID(host.clientID)
        , m_state(LobbyState::Open)
        , m_maxPlayers(10)
{
    host.role = LobbyRole::Host;
    insertPlayer(host);
    std::cout << "[Lobby] Created lobby '" << lobbyName << "' (ID: " << id << ")\n";
}

bool
Lobby::insertPlayer(const LobbyMember& member) {
    if(isFull() || hasPlayer(member.clientID)){
        return false;
    }

    m_players.push_back(member);
    return true;
}

void Lobby::deletePlayer(uintptr_t clientID) {
    std::erase_if(m_players, [clientID](const LobbyMember& player){
        return player.clientID == clientID;
    });
}

bool
Lobby::hasPlayer(ClientID clientID) const {
    return std::any_of(m_players.begin(), m_players.end(),
                       [clientID](const LobbyMember& player){
                           return player.clientID == clientID;
                       });
}

ClientID
Lobby::getHostID() const {
    return m_hostID;
}

std::vector<LobbyMember>
Lobby::getAllPlayer() const {
    std::vector<LobbyMember> playablePlayers;
    playablePlayers.reserve(m_players.size());

    std::copy_if(m_players.begin(),
                 m_players.end(),
                 std::back_inserter(playablePlayers),
                 [](const LobbyMember& player){
                     return player.isPlayable();
                 });

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
    for(const auto& player : m_players){
        if(player.isPlayable()){
            action(player);
        }
    }
}


std::optional<LobbyRole>
Lobby::getMemberRole(uintptr_t clientID) const {
    for(const auto& player : m_players){
        if(player.clientID == clientID){
            return player.role;
        }
    }
    return std::nullopt;
}
