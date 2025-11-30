#pragma once

#include <memory.h>
#include <unordered_map>
#include <vector>
#include "Message.h"
#include "Lobby.h"
#include "LobbyRegistry.h"
#include "GameSession.h"


class NetworkingInterface;

// Handles game application logic server-side
// Communicates with clients via NetworkingInterface

class GameServer
{
public:
    GameServer();

    std::vector<ClientMessage> tick(const std::vector<ClientMessage>& incomingMessages);

    std::vector<ClientMessage> handleClientMessages(const std::vector<ClientMessage>& incomingMessages);

    std::vector<ClientMessage> handleCreateLobbyMessages(uintptr_t clientID);
    std::vector<ClientMessage> handleJoinLobbyMessages(uintptr_t clientID, const JoinLobbyMessage& joinLobbyMsg);
    std::vector<ClientMessage> handleLeaveLobbyMessages(uintptr_t clientID, const LeaveLobbyMessage& leaveLobbyMsg);

    std::vector<ClientMessage> handleStartGameMessages(uintptr_t clientID, const StartGameMessage& joinGameMsg);
    std::vector<ClientMessage> handleGetLobbyStateMessages(uintptr_t clientID, const GetLobbyStateMessage& getLobbyMsg) const;

    std::vector<ClientMessage> handleBrowseLobbiesMessages(uintptr_t clientID, const BrowseLobbiesMessage& browseLobbyMsg) const;
    std::vector<ClientMessage> showCurrentLobbies(uintptr_t clientID);

    std::vector<ClientMessage> handleCreationInput(uintptr_t clientID, const Message& creationInput);

    std::vector<ClientMessage> handleStartJoinLobbyMessages(uintptr_t clientID);
    std::vector<ClientMessage> handleJoinInput(uintptr_t clientID, const Message& joinInput);
private:
    LobbyRegistry m_lobbyRegistry;
    std::unordered_map<LobbyID, std::unique_ptr<GameSession>> m_activeSessions;

    ast::GameRules createGameRules(GameType type);
    bool isGameInputMessage(const Message& msg) const;

    ast::GameRules createNumberBattleRules();
    ast::GameRules createChoiceBattleRules();

    /// helper functions
    Message createLobbyStateMessage(const Lobby* lobby) const;
    Lobby* getLobbyForClient(uintptr_t clientID) const;

    struct LobbyCreationState{
        enum class Step{
            AskingForName,
            AskingForType
        };
        Step currentStep;
        std::string lobbyName;
    };

    struct LobbyJoinState {
        enum class Step {
            AskingForPlayerName,
            AskingForLobbyName
        };
        Step currentStep;
        std::string playerName;
    };

    std::unordered_map<uintptr_t, LobbyCreationState> m_pendingCreations;
    std::unordered_map<uintptr_t, LobbyJoinState> m_pendingJoins;
};
