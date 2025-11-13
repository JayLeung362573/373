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
GameServer::handleJoinGameMessages(uintptr_t clientID, const JoinGameMessage& joinGameMsg){
    std::cout << "[GameServer] JoinGame: " << joinGameMsg.playerName << "\n";

    // TODO start the game

    Message response;
    response.type = MessageType::JoinGame;
    response.data = JoinGameMessage{joinGameMsg.playerName};
    return {ClientMessage{clientID, response}};
}

std::vector<ClientMessage>
GameServer::tick(const std::vector<ClientMessage> &incomingMessages) {
    return handleClientMessages(incomingMessages);
}



