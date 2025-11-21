#include "GameSession.h"
#include <iostream>
#include <utility>

GameSession::GameSession(LobbyID lobbyID, ast::GameRules rules, std::vector<LobbyMember> players)
    : m_lobbyID(std::move(lobbyID))
    , m_players(std::move(players))
    , m_interpreter(m_inputManager, convertRulesToProgram(rules))
    {

    std::cout << "[GameSession] Created session for lobby " << m_lobbyID
              << " with " << m_players.size() << " players\n";}

void
GameSession::start() {
    m_interpreter.execute();
    std::cout << "[GameSession] Game started\n";
}

// TODO: Revisit once IO implemented in GameInterpreter
 std::vector<ClientMessage>
 GameSession::tick(const std::vector<ClientMessage>& incomingMessages){
    processIncomingMessages(incomingMessages);

    m_interpreter.execute();

    std::vector<ClientMessage> outgoing;

    return outgoing;
 }

bool
GameSession::isFinished() const {
    return false;
}

LobbyID
GameSession::getLobbyID() const {
    return m_lobbyID;
}

std::optional<Program>
GameSession::convertRulesToProgram(ast::GameRules& rules) {
    Program program;
    program.statements = std::move(rules.statements);
    return std::make_optional(std::move(program));
}

void
GameSession::processIncomingMessages(const std::vector<ClientMessage> &messages) {
    for(const auto& msg : messages){

    }
}