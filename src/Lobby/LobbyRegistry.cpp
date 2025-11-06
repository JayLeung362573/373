#pragma once
#include "LobbyRegistry.h"
#include <iostream>

LobbyID
LobbyRegistry::createLobby(ClientID hostID, GameType gameType, const std::string &lobbyName) {
    auto lobbyID = generateLobbyID();

    auto lobby = std::make_unique<Lobby>(lobbyID, gameType, hostID, lobbyName);
    m_lobbies[lobbyID] = std::move(lobby);

    std::cout << "[Registry] Created lobby {" << lobbyName << "} (ID: "<< lobbyID << ")\n";

    return lobbyID;
}

std::vector<LobbyInfo>
LobbyRegistry::browseLobbies(GameType gameType) const {
    std::vector<LobbyInfo> result;

    for(const auto& [lobbyID, lobby] : m_lobbies){
        if(lobby->getGameType() == gameType){
            result.push_back(lobby->getInfo());
        }
    }

    std::cout << "[Registry] Found " << result.size() << " lobbies for game type "
              << static_cast<int>(gameType) << "\n";

    return result;
}

bool
LobbyRegistry::joinLobby(ClientID playerID, const LobbyID &lobbyID) {
    auto it = m_lobbies.find(lobbyID);
    if(it == m_lobbies.end()){
        std::cout << "[Registry] Lobby not found: " << lobbyID << "\n";
        return false;
    }

    std::cout << "player: " << playerID << " joined Lobby: " << lobbyID << "\n";
    return it->second->addPlayer(playerID);
}

void
LobbyRegistry::leaveLobby(ClientID playerID) {
    for(auto& [lobbyID, lobby] : m_lobbies){
        if(lobby->hasPlayer(playerID)){
            lobby->removePlayer(playerID);
        }
    }
}

LobbyID
LobbyRegistry::generateLobbyID() {
    size_t counter = 0;
    counter++;
    return "lobby_" + std::to_string(counter);
}

