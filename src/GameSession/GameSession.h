#pragma once

#include <string>
#include <vector>
#include <memory>
#include "Message.h"
#include "Lobby/Lobby.h"
#include "GameEngine/Rules.h"
#include "GameEngine/GameInterpreter.h"
#include "GameEngine/InputManager.h"

/**
 * manages a single game instance
 *
 * Jobs:
 * associate a game with a lobby (uses LobbyID)
 * track game state (finished or not)
 * track the which players are participating
 */
class GameSession{
public:
    GameSession(LobbyID lobbyID,
                ast::GameRules rules,
                std::vector<LobbyMember> players);

    std::vector<ClientMessage> start();
    std::vector<ClientMessage> tick(const std::vector<ClientMessage>& incomingMessages);
    bool isFinished() const;
    LobbyID getLobbyID() const;

private:
    LobbyID m_lobbyID;
    std::vector<LobbyMember> m_players;

    InputManager m_inputManager;
    GameInterpreter m_interpreter;

    std::optional<Program> convertRulesToProgram(ast::GameRules& rules);
    void processIncomingMessages(const std::vector<ClientMessage>& messages);

    /// Engine -> Network, server asks client
    Message convertGameMessageToMessage(const GameMessage& engineMsg) const;

    /// Network -> Engine, client answers server
    std::optional<GameMessage> convertMessageToGameMessage(const ClientMessage& clientMsg) const;

    std::vector<ClientMessage> collectOutgoingMessages();
};