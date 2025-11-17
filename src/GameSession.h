#pragma once

#include <memory>
#include <vector>
#include "Message.h"
#include "GameEngine/GameMessage.h"
#include "GameEngine/GameRuntime.h"
#include "GameEngine/Rules.h"
#include "Lobby.h"

using GameRules = ast::GameRules;

/**
 * Manages a single game session for a lobby.
 * Acts as a translation layer between network messages (ClientMessage) and game messages (GameMessage).
 *
 * Phase 1: Single-shot execution - entire game runs to completion in start()
 * Phase 2: Tick-based execution - game runs incrementally via tick() calls
 */
class GameSession {
public:
    /**
     * Create a new game session for the given lobby.
     * @param lobbyID The lobby identifier
     * @param rules The game rules to execute
     * @param players The players participating in this session
     */
    GameSession(LobbyID lobbyID,
                GameRules rules,
                std::vector<LobbyMember> players);

    /**
     * Start the game (Phase 1: runs entire game to completion).
     */
    void start();

    /**
     * Execute one tick of the game (Phase 2: not yet implemented).
     * @param incomingMessages Messages from clients
     * @return Messages to send back to clients
     */
    std::vector<ClientMessage> tick(const std::vector<ClientMessage>& incomingMessages);

    /**
     * Get game result messages to send to players (Phase 2: not yet implemented).
     * @return Messages containing game results
     */
    std::vector<ClientMessage> getResultMessages() const;

    /**
     * Check if the game has finished execution.
     * @return true if game is done
     */
    bool isFinished() const { return m_runtime->isFinished(); }

    /**
     * Get the lobby ID for this session.
     * @return The lobby identifier
     */
    LobbyID getLobbyID() const { return m_lobbyID; }

private:
    /**
     * Extract game messages from client messages (Phase 2).
     * @param msgs Client messages
     * @return Game messages for the interpreter
     */
    std::vector<GameMessage> extractGameMessages(const std::vector<ClientMessage>& msgs);

    LobbyID m_lobbyID;
    std::vector<LobbyMember> m_players;
    GameRules m_rules;  // Store rules so GameRuntime can reference them
    std::unique_ptr<GameRuntime> m_runtime;
};
