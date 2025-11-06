#pragma once
#include <unordered_map>
#include <memory>
#include <optional>
#include "Lobby.h"
#include "LobbyTypes.h"

class LobbyRegistry{
public:

    LobbyID createLobby(ClientID hostID, GameType gameType, const std::string& name);
    bool destroyLobby(const LobbyID& lobbyID);

    std::vector<LobbyInfo> browseLobbies(GameType gameType) const;

    bool joinLobby(ClientID playerID, const LobbyID& lobbyID);
    void leaveLobby(ClientID playerID);
    void removeLobby(LobbyID lobbyId);

//    std::optional<Lobby*> getLobby(LobbyID lobbyID);

private:
    LobbyID generateLobbyID();
    std::unordered_map<LobbyID, std::unique_ptr<Lobby>> m_lobbies;
};