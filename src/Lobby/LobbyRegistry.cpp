#include "LobbyRegistry.h"
#include <iostream>
#include <ranges>

LobbyResult
LobbyRegistry::createLobby(const LobbyMember& host, GameType gameType, const std::string &lobbyName) {
    if(m_clientLobbyMap.count(host.clientID)){
        std::cout << "[Registry] Host " << host.clientID << " already in a lobby.\n";
        return {nullptr, LobbyError::AlreadyInLobby};
    }

    auto lobbyID = generateLobbyID();
    auto lobby = std::make_unique<Lobby>(lobbyID, gameType, host, lobbyName);

    auto* lobbyPtr = lobby.get();
    m_lobbies[lobbyID] = std::move(lobby);
    m_clientLobbyMap[host.clientID] = lobbyID;

    std::cout << "[Registry] Created lobby {" << lobbyName << "} (ID: "<< lobbyID << ")\n";

    return {lobbyPtr, LobbyError::None};
}

std::vector<LobbyInfo>
LobbyRegistry::browseLobbies(std::optional<GameType> gameType) const {
    std::vector<LobbyInfo> result;
    result.reserve(m_lobbies.size());

    auto matchesType = [&](const auto& entry){
        return !gameType.has_value() || entry.second->getGameType() == *gameType;
    };

    for(const auto& entry : m_lobbies | std::views::filter(matchesType)){
        result.push_back(entry.second->getInfo());
    }

    std::cout << "[Registry] Found " << result.size() << " lobbies for game type "
              << (gameType.has_value() ? std::to_string((int)*gameType) : "ALL") << ")\n";

    return result;
}

LobbyResult
LobbyRegistry::joinLobby(const LobbyMember& member, const LobbyID &lobbyID) {
    if(m_clientLobbyMap.count(member.clientID)){
        std::cout << "[LobbyRegistry] Client " << member.clientID << " already in a lobby.\n";
        return {nullptr, LobbyError::AlreadyInLobby};
    }

    auto it = m_lobbies.find(lobbyID);
    if(it == m_lobbies.end()){
        std::cout << "[Registry] Lobby not found: " << lobbyID << "\n";
        return {nullptr, LobbyError::LobbyNotFound};
    }

    bool success = it->second->insertPlayer(member);
    if(success){
        std::cout << "[Registry] player: " << member.clientID << " joined Lobby: " << lobbyID << "\n";

        m_clientLobbyMap[member.clientID] = lobbyID;

        return {it->second.get(), LobbyError::None};
    }
    return {nullptr, LobbyError::LobbyFull};
}

bool
LobbyRegistry::destroyLobby(const LobbyID &lobbyID) {
    auto it = m_lobbies.find(lobbyID);
    if(it == m_lobbies.end()){
        return false;
    }

    auto& lobby = it->second;
    for(const auto& player : lobby->getMembers()){
        m_clientLobbyMap.erase(player.clientID);
    }

    std::cout << "[Registry] Destroying lobby: " << lobbyID << "\n";
    m_lobbies.erase(it);
    return true;
}

void
LobbyRegistry::leaveLobby(ClientID clientID) {
    auto lobbyIDToLeave = findLobbyForClient(clientID);

    if(!lobbyIDToLeave){
        return;
    }

    m_clientLobbyMap.erase(clientID);

    auto it = m_lobbies.find(*lobbyIDToLeave);
    if (it == m_lobbies.end()) {
        std::cout << "[Registry] Error: Client map has issue.\n";
        return;
    }

    auto& lobby = it->second;
    lobby->deletePlayer(clientID);
    std::cout << "[Registry] Player " << clientID << " left lobby: " << *lobbyIDToLeave << "\n";

    if(lobby->getPlayerCount() == 0){
        std::cout << "[Registry] Lobby " << *lobbyIDToLeave << " is empty, destroying\n";
        m_lobbies.erase(it);
    }
}

Lobby *
LobbyRegistry::getLobby(const LobbyID &lobbyID) const {
    auto it = m_lobbies.find(lobbyID);
    if(it != m_lobbies.end()){
        return it->second.get();
    }
    return nullptr;
}

std::optional<LobbyID>
LobbyRegistry::findLobbyForClient(ClientID clientID) const {
    auto it = m_clientLobbyMap.find(clientID);
    if(it != m_clientLobbyMap.end()){
        return it->second;
    }
    return std::nullopt;
}

LobbyID
LobbyRegistry::generateLobbyID() {
    m_lobbyCounter++;
    return "lobby_" + std::to_string(m_lobbyCounter);
}
