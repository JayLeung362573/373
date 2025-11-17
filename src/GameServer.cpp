#include <iostream>
#include "GameServer.h"
#include "Message.h"

GameServer::GameServer() = default;

struct MessageHandlerVisitor {
    uintptr_t clientID;
    GameServer* server;

    std::vector<ClientMessage> responses;

    void operator()(const JoinLobbyMessage& data) {
        responses = server->handleJoinLobbyMessages(clientID, data);
    }

    void operator()(const LeaveLobbyMessage& data) {
        responses = server->handleLeaveLobbyMessages(clientID, data);
    }

    void operator()(const GetLobbyStateMessage& data) {
        responses = server->handleGetLobbyStateMessages(clientID, data);
    }

    void operator()(const BrowseLobbiesMessage& data) {
        responses = server->handleBrowseLobbiesMessages(clientID, data);
    }

    void operator()(const StartGameMessage& data) {
        responses = server->handleStartGameMessages(clientID, data);
    }

    void operator()(const std::monostate& data) {}

    void operator()(const LobbyStateMessage& data) {}

    void operator()(const ErrorMessage& data) {}

    void operator()(const UpdateCycleMessage& data) {
        std::cout << "[GameServer]: Client should not send UpdateCycle\n";
    }
};

std::vector<ClientMessage>
GameServer::handleClientMessages(const std::vector<ClientMessage> &incomingMessages) {
    std::vector<ClientMessage> outgoingMessages;

    for(const auto& clientMsg : incomingMessages){
        MessageHandlerVisitor visitor {clientMsg.clientID, this};

        /// visit the clientMsg.message.data and call the correct handler
        std::visit(visitor, clientMsg.message.data);

        outgoingMessages.insert(outgoingMessages.end(),
                                std::make_move_iterator(visitor.responses.begin()),
                                std::make_move_iterator(visitor.responses.end()));
    }
    return outgoingMessages;
}

std::vector<ClientMessage>
GameServer::handleJoinLobbyMessages(uintptr_t clientID, const JoinLobbyMessage& joinLobbyMsg) {
    std::cout << "[GameServer] Player {" << joinLobbyMsg.playerName
              << "} attempting to join lobby (clientID = " << clientID << ")\n";

    LobbyID lobbyID;
    Lobby* lobby = nullptr;
    std::vector<ClientMessage> outgoingMessages;

    if(joinLobbyMsg.lobbyName.empty()){
        lobby = m_lobbyRegistry.createLobby(
                clientID,
                GameType::Default,
                joinLobbyMsg.playerName + "'s Lobby"
        );
    } else {
        lobbyID = joinLobbyMsg.lobbyName;
        lobby = m_lobbyRegistry.joinLobby(clientID, lobbyID);
    }

    if(!lobby){
        Message errorMsg;
        errorMsg.type = MessageType::Error;
        errorMsg.data = ErrorMessage{"[handleJoinLobby: Lobby not found or is full"};
        outgoingMessages.push_back(ClientMessage{clientID, errorMsg});
        return outgoingMessages;
    }

    if(joinLobbyMsg.lobbyName.empty()){
        lobbyID = lobby->getInfo().lobbyID;
    }

    Message lobbyStatePayload;
    lobbyStatePayload.type = MessageType::LobbyState;
    lobbyStatePayload.data = LobbyStateMessage{
            {lobby->getInfo()},
            lobbyID
    };

    auto players = lobby->getAllPlayer();
    std::cout << "[GameServer] Broadcasting new lobby state to "
              << players.size() << " clients.\n";

    for(const auto& player : players) {
        outgoingMessages.push_back(ClientMessage{player.clientID, lobbyStatePayload});
    }

    return outgoingMessages;
}

std::vector<ClientMessage>
GameServer::handleLeaveLobbyMessages(uintptr_t clientID, const LeaveLobbyMessage& leaveLobbyMsg)
{
    std::cout << "[GameServer] Player '" << leaveLobbyMsg.playerName
              << "' leaving lobby (clientID = " << clientID << ")\n";

    std::vector<ClientMessage> outgoingMessages;

    auto lobbyID = m_lobbyRegistry.findLobbyForClient(clientID);
    if (!lobbyID) {
        std::cout << "[GameServer] Player not in any lobby\n";
        Message errorMsg;
        errorMsg.type = MessageType::Error;
        errorMsg.data = ErrorMessage{"Not in any lobby"};
        outgoingMessages.push_back(ClientMessage{clientID, errorMsg});
        return outgoingMessages;
    }

    m_lobbyRegistry.leaveLobby(clientID);

    Message leaveConfirmation;
    leaveConfirmation.type = MessageType::LeaveLobby;
    leaveConfirmation.data = LeaveLobbyMessage{leaveLobbyMsg.playerName};
    outgoingMessages.push_back(ClientMessage{clientID, leaveConfirmation});

    auto lobby = m_lobbyRegistry.getLobby(*lobbyID);
    if (!lobby) {
        std::cout << "[GameServer] Lobby " << *lobbyID << " was destroyed (empty)\n";
        return outgoingMessages;
    }

    Message lobbyStatePayload;
    lobbyStatePayload.type = MessageType::LobbyState;
    lobbyStatePayload.data = LobbyStateMessage{
            {lobby->getInfo()},
            *lobbyID
    };

    auto players = lobby->getAllPlayer();
    std::cout << "[GameServer] Broadcasting lobby state to "
              << players.size() << " remaining clients\n";

    for(const auto& player : players) {
        outgoingMessages.push_back(ClientMessage{player.clientID, lobbyStatePayload});
    }

    return outgoingMessages;
}

std::vector<ClientMessage>
GameServer::handleGetLobbyStateMessages(uintptr_t clientID, const GetLobbyStateMessage &getLobbyMsg) const {
    std::cout << "[GameServer] Client " << clientID
              << " requesting lobby state\n";

    auto lobbyID = m_lobbyRegistry.findLobbyForClient(clientID);
    if(!lobbyID){
        std::cout << "[GameServer] Client not in any lobby\n";
        Message errorMsg;
        errorMsg.type = MessageType::Error;
        errorMsg.data = ErrorMessage{"Client is not in a lobby"};
        return {ClientMessage{clientID, errorMsg}};
    }

    auto lobby = m_lobbyRegistry.getLobby(*lobbyID);

    if(!lobby){
        std::cout << "[GameServer] Error: Lobby " << *lobbyID << " not found after find\n";
        Message errorMsg;
        errorMsg.type = MessageType::Error;
        errorMsg.data = ErrorMessage{"handleGetLobbyState: Lobby not found"};
        return {ClientMessage{clientID, errorMsg}};
    }

    Message response;
    response.type = MessageType::LobbyState;
    response.data = LobbyStateMessage{
            {lobby->getInfo()},
            {*lobbyID}
    };
    return {ClientMessage{clientID, response}};
}

std::vector<ClientMessage>
GameServer::handleBrowseLobbiesMessages(uintptr_t clientID, const BrowseLobbiesMessage &browseLobbiesMsg) const {
    std::cout << "[GameServer] Client " << clientID
              << " browsing lobbies\n";

    auto lobbies = m_lobbyRegistry.browseLobbies(browseLobbiesMsg.gameType);

    Message response;
    response.type = MessageType::LobbyState;
    response.data = LobbyStateMessage{lobbies, ""};
    return {ClientMessage{clientID, response}};
}

std::vector<ClientMessage>
GameServer::handleStartGameMessages(uintptr_t clientID, const StartGameMessage& startGameMsg){
    std::cout << "[GameServer] Player " << startGameMsg.playerName << " (clientID: " << clientID << ")"
              << " requesting to start game\n";

    /// 1. check if this client is in a lobby
    auto lobbyID = m_lobbyRegistry.findLobbyForClient(clientID);
    if(!lobbyID){
        std::cout << "[GameServer] Error: Client not in any lobby\n";
        Message errorMsg;
        errorMsg.type = MessageType::Error;
        errorMsg.data = ErrorMessage{"You must be in a lobby to start a game"};
        return {ClientMessage{clientID, errorMsg}};
    }

    /// 2. check this lobby
    auto lobby = m_lobbyRegistry.getLobby(*lobbyID);
    if(!lobby){
        std::cout << "[GameServer] Error: Lobby: " << *lobbyID << "not found\n";
        Message errorMsg;
        errorMsg.type = MessageType::Error;
        errorMsg.data = ErrorMessage{"Lobby not found"};
        return {ClientMessage{clientID, errorMsg}};
    }

    /// 3. check if it is a host to start the game
    if(lobby->getHostID() != clientID){
        std::cout << "[GameServer] Error: Only host can start game\n";
        Message errorMsg;
        errorMsg.type = MessageType::Error;
        errorMsg.data = ErrorMessage{"Only the lobby host can start the game"};
        return {ClientMessage{clientID, errorMsg}};
    }

    /// 4. check if the game already started
    if(m_activeSessions.find(*lobbyID) != m_activeSessions.end()){
        std::cout << "[GameServer] Error: Game already started for lobby "
        << *lobbyID << "\n";

        Message errorMsg;
        errorMsg.type = MessageType::Error;
        errorMsg.data = ErrorMessage{"Game already started"};
        return {ClientMessage{clientID, errorMsg}};
    }

    /// 5. get all players (host and players, excluding audience)
    auto players = lobby->getAllPlayer();
    std::cout << "[GameServer] Starting game for lobby " << *lobbyID
              << " with " << players.size() << " players\n";

    /// 6. create game rules
    GameRules rules = createGameRules();

    /// 7. create and start session
    auto session = std::make_unique<GameSession>(*lobbyID, rules, players);
    session->start();

    /// 8. map and track this session
    m_activeSessions[*lobbyID] = std::move(session);

    /// 9. create 'startMsg' for each player and store them in 'responses',
    /// the networking will receive this 'responses' and send(notify) to each player
    std::vector<ClientMessage> responses;
    Message startMsg;
    startMsg.type = MessageType::StartGame;
    startMsg.data = JoinLobbyMessage{"Game started"};

    for(const auto& player : players){
        std::cout << "[GameServer] Notifying player " << player.clientID << "\n";
        responses.push_back(ClientMessage{player.clientID, startMsg});
    }

    return responses;
}

std::vector<ClientMessage>
GameServer::tick(const std::vector<ClientMessage> &incomingMessages) {
    return handleClientMessages(incomingMessages);
}

GameRules
GameServer::createGameRules() {
    std::vector<std::unique_ptr<ast::Statement>> statements;
    statements.clear();

    statements.push_back(
            ast::makeAssignment(
                    ast::makeVariable(Name{"winner"}),
                    ast::makeConstant(Value{String{"player1"}})
                    )
            );
    std::cout << "[GameServer] Created simple game with "
              << statements.size() << " statements\n";

    return GameRules{std::span(statements)};
}

