#include "GameSession.h"
#include <iostream>
#include <utility>

GameSession::GameSession(LobbyID lobbyID, GameRules rules, std::vector<LobbyMember> players)
    : m_lobbyID(std::move(lobbyID))
    , m_players(std::move(players))
    , m_runtime(std::make_unique<GameRuntime>(rules))
    {

    std::cout << "[GameSession] Created session for lobby " << m_lobbyID
              << " with " << m_players.size() << " players\n";}

void
GameSession::start() {
    if(m_runtime->isFinished()){
        std::cout << "[GameSession] Cannot start: already finished or inactive\n";
        return;
    }
    // TODO: simulate simple game without player input first

    // TODO: then use tick() with player input
    m_runtime->run();

    std::cout << "[GameSession] Game for lobby " << m_lobbyID << " finished\n";
}

std::vector<ClientMessage>
GameSession::tick(const std::vector<ClientMessage>& incomingMessages){
    if(m_runtime->isFinished()){
        return {};
    }

    std::vector<GameMessage> gameMsgs = extractGameMessages(incomingMessages);

    auto outGameMsgs = m_runtime->tick(gameMsgs);

    if(m_runtime->isFinished()){
        return {};
    }

    return convertToClientMessages(outGameMsgs);
}

std::vector<GameMessage>
GameSession::extractGameMessages(const std::vector<ClientMessage> &clientMsgs) {
    return {};
}

std::vector<ClientMessage>
GameSession::convertToClientMessages(const std::vector<GameMessage> &gameMsgs) {
    return {};
}