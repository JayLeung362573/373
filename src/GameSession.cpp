#include "GameSession.h"
#include <iostream>

GameSession::GameSession(LobbyID lobbyID,
                         GameRules rules,
                         std::vector<LobbyMember> players)
    : m_lobbyID(lobbyID)
    , m_players(players)
    , m_runtime(std::make_unique<GameRuntime>(rules))
{
    std::cout << "[GameSession] Created session for lobby '" << m_lobbyID
              << "' with " << m_players.size() << " player(s)\n";
}

void GameSession::start()
{
    if (m_runtime->isFinished()) {
        std::cout << "[GameSession] Warning: Game already finished\n";
        return;
    }

    std::cout << "[GameSession] Starting game for lobby '" << m_lobbyID << "'\n";

    // Phase 1: Execute entire game in one shot
    m_runtime->run();

    std::cout << "[GameSession] Game for lobby '" << m_lobbyID << "' finished\n";
}

std::vector<ClientMessage>
GameSession::tick(const std::vector<ClientMessage>& incomingMessages)
{
    if (m_runtime->isFinished()) {
        return {};  // Game already finished
    }

    // Phase 2: Extract game messages and execute one tick
    std::vector<GameMessage> gameMsgs = extractGameMessages(incomingMessages);
    std::vector<GameMessage> outGameMsgs = m_runtime->tick(gameMsgs);

    // Convert GameMessages back to ClientMessages
    // TODO Phase 2: Implement proper message conversion
    std::vector<ClientMessage> responses;

    return responses;
}

std::vector<ClientMessage>
GameSession::getResultMessages() const
{
    // Phase 1: Return empty (no results to send yet)
    // Phase 2: Will return actual game results based on game state
    return {};
}

std::vector<GameMessage>
GameSession::extractGameMessages(const std::vector<ClientMessage>& msgs)
{
    // Phase 2: Extract relevant game messages from client messages
    // For now, return empty
    return {};
}
