#include <iostream>
#include "GameServer.h"
#include "Message.h"
#include "GameSession.h"

GameServer::GameServer() = default;

GameRules GameServer::createSimpleGameRules()
{
    // Phase 1: Create simple hardcoded game for testing
    // The vector must be static so the span remains valid
    static std::vector<std::unique_ptr<ast::Statement>> statements;
    statements.clear();

    // Create test statements: winner = "Player1", score = 100
    statements.push_back(
        ast::makeAssignment(
            ast::makeVariable(Name{"winner"}),
            ast::makeConstant(Value{String{"Player1"}})
        )
    );

    statements.push_back(
        ast::makeAssignment(
            ast::makeVariable(Name{"score"}),
            ast::makeConstant(Value{Integer{100}})
        )
    );

    std::cout << "[GameServer] Created simple game with "
              << statements.size() << " statement(s)\n";

    return GameRules{std::span(statements)};
}

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

    void operator()(const JoinGameMessage& data) {
        responses = server->handleJoinGameMessages(clientID, data);
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
GameServer::handleJoinGameMessages(uintptr_t clientID, const JoinGameMessage& joinGameMsg)
{
    std::cout << "[GameServer] Player '" << joinGameMsg.playerName
              << "' requesting to start game (clientID: " << clientID << ")\n";

    // ========== STEP 1: Find the lobby this client is in ==========
    auto lobbyID = m_lobbyRegistry.findLobbyForClient(clientID);
    if (!lobbyID) {
        std::cout << "[GameServer] Error: Client not in any lobby\n";
        Message errorMsg;
        errorMsg.type = MessageType::Error;
        errorMsg.data = ErrorMessage{"You must be in a lobby to start a game"};
        return {ClientMessage{clientID, errorMsg}};
    }

    // ========== STEP 2: Get the lobby object ==========
    auto lobby = m_lobbyRegistry.getLobby(*lobbyID);
    if (!lobby) {
        std::cout << "[GameServer] Error: Lobby '" << *lobbyID << "' not found\n";
        Message errorMsg;
        errorMsg.type = MessageType::Error;
        errorMsg.data = ErrorMessage{"Lobby not found"};
        return {ClientMessage{clientID, errorMsg}};
    }

    // ========== STEP 3: Verify only host can start game ==========
    if (lobby->getHostID() != clientID) {
        std::cout << "[GameServer] Error: Client " << clientID
                  << " is not the host (host is " << lobby->getHostID() << ")\n";
        Message errorMsg;
        errorMsg.type = MessageType::Error;
        errorMsg.data = ErrorMessage{"Only the lobby host can start the game"};
        return {ClientMessage{clientID, errorMsg}};
    }

    // ========== STEP 4: Check if game already started ==========
    if (m_activeSessions.find(*lobbyID) != m_activeSessions.end()) {
        std::cout << "[GameServer] Error: Game already in progress for lobby '"
                  << *lobbyID << "'\n";
        Message errorMsg;
        errorMsg.type = MessageType::Error;
        errorMsg.data = ErrorMessage{"Game already in progress for this lobby"};
        return {ClientMessage{clientID, errorMsg}};
    }

    // ========== STEP 5: Get all players from lobby ==========
    auto players = lobby->getAllPlayer();
    std::cout << "[GameServer] Starting game for lobby '" << *lobbyID
              << "' with " << players.size() << " player(s)\n";

    // ========== STEP 6: Create game rules ==========
    GameRules rules = createSimpleGameRules();

    // ========== STEP 7: Create and start GameSession ==========
    auto session = std::make_unique<GameSession>(*lobbyID, rules, players);
    session->start();  // Execute game immediately (Phase 1)

    // ========== STEP 8: Store session for tracking ==========
    m_activeSessions[*lobbyID] = std::move(session);

    std::cout << "[GameServer] Game session stored. Active sessions: "
              << m_activeSessions.size() << "\n";

    // ========== STEP 9: Notify all players that game started ==========
    std::vector<ClientMessage> responses;
    Message startMsg;
    startMsg.type = MessageType::JoinGame;
    startMsg.data = JoinGameMessage{"Game started and completed"};

    for (const auto& player : players) {
        std::cout << "[GameServer] Notifying client " << player.clientID << "\n";
        responses.push_back(ClientMessage{player.clientID, startMsg});
    }

    return responses;
}

std::vector<ClientMessage>
GameServer::tick(const std::vector<ClientMessage> &incomingMessages) {
    return handleClientMessages(incomingMessages);
}



