#pragma once

#include <memory.h>
#include <unordered_map>
#include <vector>
#include "Message.h"
#include "Lobby.h"
#include "LobbyRegistry.h"


class NetworkingInterface;

// Handles game application logic server-side
// Communicates with clients via NetworkingInterface

class GameServer
{
public:
    GameServer();

    enum class ServerState{
        InLobby,
        InGame,
        GameOver,
    };

    std::vector<ClientMessage> tick(const std::vector<ClientMessage>& incomingMessages);

    ServerState getState() const;
private:
    std::vector<ClientMessage> handleLobbyMessages(const std::vector<ClientMessage>& incomingMessages);
    std::vector<ClientMessage> handleGameMessages(const std::vector<ClientMessage>& incomingMessages);

    std::vector<ClientMessage> broadcastLobbyState();
    LobbyRegistry m_lobbyRegistry;
    bool m_gameStarted;
    bool m_gameOver;
    std::unordered_map<ClientID, LobbyID> m_clientToLobby;
};
