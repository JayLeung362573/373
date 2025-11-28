#include "LobbyRegistry.h"
#include <iostream>

Lobby*
LobbyRegistry::createLobby(ClientID hostID, GameType gameType, const std::string &lobbyName) {
    auto lobbyID = generateLobbyID();

    auto lobby = std::make_unique<Lobby>(lobbyID, gameType, hostID, lobbyName);

    auto* lobbyPtr = lobby.get();
    m_lobbies[lobbyID] = std::move(lobby);
    m_clientLobbyMap[hostID] = lobbyID;

    std::cout << "[Registry] Created lobby {" << lobbyName << "} (ID: "<< lobbyID << ")\n";

    return lobbyPtr;
}

std::vector<LobbyInfo>
LobbyRegistry::browseLobbies(std::optional<GameType> gameType) const {
    std::vector<LobbyInfo> result;

    for(const auto& [lobbyID, lobby] : m_lobbies){
        if(!gameType.has_value() || lobby->getGameType() == *gameType){
            result.push_back(lobby->getInfo());
        }
    }

    std::cout << "[Registry] Found " << result.size() << " lobbies for game type "
              << (gameType.has_value() ? std::to_string((int)*gameType) : "ALL") << ")\n";

    return result;
}

Lobby*
LobbyRegistry::joinLobby(ClientID clientID, const LobbyID &lobbyID) {
    if(m_clientLobbyMap.count(clientID)){
        std::cout << "[LobbyRegistry] Client " << clientID << " already in a lobby.\n";
        return nullptr;
    }

    auto it = m_lobbies.find(lobbyID);
    if(it == m_lobbies.end()){
        std::cout << "[Registry] Lobby not found: " << lobbyID << "\n";
        return nullptr;
    }

    bool success = it->second->insertPlayer(clientID);
    if(success){
        std::cout << "[Registry] player: " << clientID << " joined Lobby: " << lobbyID << "\n";

        m_clientLobbyMap[clientID] = lobbyID;

        return it->second.get(); /// return the lobby pointer
    }
    return nullptr;
}

bool
LobbyRegistry::destroyLobby(const LobbyID &lobbyID) {
    auto it = m_lobbies.find(lobbyID);
    if(it == m_lobbies.end()){
        return false;
    }

    auto& lobby = it->second;
    auto players = lobby->getAllPlayer();

    for(const auto& player : players){
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

