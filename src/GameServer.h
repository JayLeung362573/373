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

    std::vector<ClientMessage> tick(const std::vector<ClientMessage>& incomingMessages);

    std::vector<ClientMessage> handleClientMessages(const std::vector<ClientMessage>& incomingMessages);

    std::vector<ClientMessage> handleJoinLobbyMessages(uintptr_t clientID, const JoinLobbyMessage& joinLobbyMsg);
    std::vector<ClientMessage> handleLeaveLobbyMessages(uintptr_t clientID, const LeaveLobbyMessage& leaveLobbyMsg);
    std::vector<ClientMessage> handleJoinGameMessages(uintptr_t clientID, const JoinGameMessage& joinGameMsg);

    std::vector<ClientMessage> handleGetLobbyStateMessages(uintptr_t clientID, const GetLobbyStateMessage& getLobbyMsg) const;
    std::vector<ClientMessage> handleBrowseLobbiesMessages(uintptr_t clientID, const BrowseLobbiesMessage& browseLobbyMsg) const;

private:
    LobbyRegistry m_lobbyRegistry;
};
