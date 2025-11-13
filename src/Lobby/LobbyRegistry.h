#pragma once
#include <unordered_map>
#include <memory>
#include <optional>
#include "Lobby.h"
#include "LobbyTypes.h"

/// Lobby manager:
/// 1) create and destroy lobbies
/// 2)
class LobbyRegistry{
public:

    LobbyID createLobby(ClientID hostID, GameType gameType, const std::string& name);
    bool destroyLobby(const LobbyID& lobbyID);

    std::vector<LobbyInfo> browseLobbies(GameType gameType) const;

    bool joinLobby(ClientID playerID, const LobbyID& lobbyID);
    void leaveLobby(ClientID playerID);

    Lobby * getLobby(const LobbyID& lobbyID) const;
    std::optional<LobbyID> findLobbyForClient(ClientID playerID) const;
private:
    LobbyID generateLobbyID();

    std::unordered_map<LobbyID, std::unique_ptr<Lobby>> m_lobbies;
    size_t m_lobbyCounter = 0;
};